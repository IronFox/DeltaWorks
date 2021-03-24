#include "../global_root.h"
#include "tcp.h"



namespace DeltaWorks
{

	namespace TCP
	{
		bool			verbose=false;

		template class Channel<String,0,0>;
		template class ObjectSender<String,0>;
		template class SignalSender<0>;




		void	NoExceptionHandler(const TCodeLocation&loc,const std::exception&ex)
		{

		}
		void	NoArbExceptionHandler(const TCodeLocation&loc)
		{

		}

		std::function<void(const TCodeLocation&,const std::exception&)>	onIgnoredException = NoExceptionHandler;
		std::function<void(const TCodeLocation&)>	onIgnoredArbitraryException = NoArbExceptionHandler;



		void	SwapCloseSocket(volatile SOCKET&socket)
		{
			if (socket == INVALID_SOCKET)
				return;
			SOCKET socket_to_close = socket;
			socket = INVALID_SOCKET;
			closesocket(socket_to_close);
		}

		String	StripPort(const String&host)
		{
			index_t at = host.Find(':');
			if (at != InvalidIndex)
				return host.subString(0,at);
			return host;
		}


		String	ToString(const addrinfo&address)
		{
			sockaddr_storage temp;
			memcpy(&temp, address.ai_addr, address.ai_addrlen);
			return ToString(temp,(socklen_t)address.ai_addrlen);
		}

		String	ToString(const sockaddr_storage&address, socklen_t addrLen, bool includePort /*= true*/)
		{
			if (addrLen == 0)
				return "<No Address>";
			char hoststr[NI_MAXHOST];
			char portstr[NI_MAXSERV];

			//in case hosts file contains localhost redirects, which produces funny results, let's do a manual localhost comparison:

			int rc;
			rc = getnameinfo((struct sockaddr *)&address, addrLen, hoststr, sizeof(hoststr), portstr, sizeof(portstr), NI_NUMERICHOST | NI_NUMERICSERV);
			if (rc != 0)
				return "<No Address>";
			String result = hoststr;
			if (result == "127.0.0.1" || result == "::1")
			{
				if (includePort)
					return "localhost:"+String(portstr);
				return "localhost";
			}


			//rc = getnameinfo((struct sockaddr *)&address, addrLen, hoststr, sizeof(hoststr), portstr, sizeof(portstr), /*NI_NUMERICHOST |*/ NI_NUMERICSERV);
			//if (rc != 0)
			//{
			//	if (includePort)
			//		result += portstr;
			//	return result;
			//}
			if (!includePort)
				return hoststr;
			String hstr = hoststr;
			if (hstr.Contains(':') && !hstr.BeginsWith('['))
				hstr = '[' + hstr + ']';
			return hstr+":"+String(portstr);

			//char ipbuff[INET_ADDRSTRLEN];
			//inet_ntop(address.ss_family, &(((struct sockaddr_in *)&address)->sin_addr), ipbuff, INET_ADDRSTRLEN);
			//return String(ipbuff);
		}


		static const char* lastSocketError()
		{
			return Net::GetLastError();
		}

		const char*	event2str(event_t event)
		{
			#undef ECASE
			#define ECASE(symbol)	case Event::symbol: return "Event::"#symbol;
			switch (event)
			{
				ECASE(None);
				ECASE(ConnectionFailed);
				ECASE(ConnectionEstablished);
				ECASE(ConnectionLost);
			}
			return "Unknown event";
			#undef ECASE
		
	
	
		}
		/*
		void	closesocket(SOCKET socket)
		{
			std::cout << "closing socket "<<socket<<std::endl;
			if (::closesocket(socket))
				std::cout << "failed to close socket"<<std::endl;
		}*/



		//void RootChannel::SendObject(Destination&target, Package&object, unsigned minUserLevel)
		//{
		//	target.SendObject(id,object,minUserLevel);
		//}

		//void RootChannel::SendPackage(Destination&target, const PPeer&exclude, const ISerializable&object, unsigned minUserLevel)
		//{
		//	target.SendObject(id,exclude,object,minUserLevel);
		//}


		void	Dispatcher::OnDoneResolving(const std::function<void()>&f)
		{
			if (IsResolving())
			{
				postResolutionCallbackLock.lock();
				postResolutionCallbacks << f;
				postResolutionCallbackLock.unlock();
			}
			else
				f();
		}


		void	Dispatcher::HandleObject(RootChannel*receiver, Peer&sender, const PDispatchable&object)
		{
			if (!object)
				return;
			if (relayTo)
			{
				relayTo->HandleObject(receiver,sender,object);
				return;
			}
			if (async)
			{
				receiver->Handle(object,sender.LinkFromThis());

			}
			else
			{
				TCommonEvent inbound;
				inbound.type = TCommonEvent::Object;
				inbound.object = object;
				inbound.sender = sender.LinkFromThis();
				inbound.receiver = receiver;
				if (eventLock.ExclusiveLock(CLOCATION))
				{
					queue << inbound;
					anyEventsPending = true;
					eventLock.ExclusiveUnlock();
				}
			}
		}
	
		void	Dispatcher::HandlePeerDeletion(const PPeer&peer)
		{
			if (!peer || !peer->Destroy())
				return;
			//if (async)
			//{
			//	peer->Join();
			//}
			//else
			{
				wasteMutex.lock();
					wasteBucket.Append(peer);
				wasteMutex.release();
			}
		}
	
		/*virtual*/ void	Dispatcher::HandleEvent(event_t ev, Peer&sender)
		{
			if (relayTo)
			{
				relayTo->HandleEvent(ev,sender);
				return;
			}
			if (ev == Event::None)
				return;
			if (async)
			{
			//	if (eventLock.PermissiveLock(CLOCATION))
				if (!eventLock.IsBlocked())	//not thread-safe but works if the event is caused by the same thread as the one that blocked
				{
					if (onEvent)
					{
						onEvent(ev,sender);
					}
				//	eventLock.PermissiveUnlock(CLOCATION);
				}
			}
			else
			{
				TCommonEvent  sevent;
				sevent.type = TCommonEvent::NetworkEvent;
				sevent.event = ev;
				sevent.sender = sender.LinkFromThis();
				if (eventLock.ExclusiveLock(CLOCATION))
				{
					queue << sevent;
					anyEventsPending = true;
					eventLock.ExclusiveUnlock();
				}
			}
		}

		/*virtual*/ void	Dispatcher::HandleSignal(UINT32 signal, Peer&peer)
		{
			if (relayTo)
			{
				relayTo->HandleSignal(signal,peer);
				return;
			}

			if (async)
			{
	//			if (eventLock.PermissiveLock(CLOCATION))
		//		if (!eventLock.IsBlocked())	//not thread-safe but works if the event is caused by the same thread as the one that blocked. this is not an event and should not be blocked in permissive mode
				{
					if (onSignal)
						onSignal(signal,peer);
			//		eventLock.PermissiveUnlock(CLOCATION);
				}
			}
			else
			{
				TCommonEvent tsignal;
				tsignal.type = TCommonEvent::Signal;
				tsignal.channel = signal;
				tsignal.sender = peer.LinkFromThis();

				if (eventLock.ExclusiveLock(CLOCATION))
				{
					queue << tsignal;
					anyEventsPending = true;
					eventLock.ExclusiveUnlock();
				}
			
			}
		}


		void Dispatcher::Resolve()
		{
			if (!anyEventsPending)
				return;
			if (eventLock.ExclusiveLock(CLOCATION))
			{
				resolutionBuffer.Clear();
				TCP_TRY
				{
					TCommonEvent ev;
					//TSignal signal;
					while (queue >> ev)
						resolutionBuffer << ev;
		
				}
				TCP_CATCH
				anyEventsPending = false;
				eventLock.ExclusiveUnlock();


				terminateAfterResolve = false;
				resolving = true;
				TCP_TRY
				{
					foreach (resolutionBuffer,_ev)
					{
						if (terminateAfterResolve)
							break;
						const TCommonEvent&ev = *_ev;
						TCP_TRY
						{
							switch (ev.type)
							{
								case TCommonEvent::NetworkEvent:
									if (onEvent)
									{
										PPeer p = ev.sender.s;
										Peer*ptr = p ? p.get() : ev.sender.p;
										if (ptr)
											onEvent(ev.event,*ptr);
										else
										{
											//this should now be impossible with the use of shared_ptr instead of weak_ptr
											throw Exception(CLOCATION,"peer reference lost in transit for event "+String(event2str(ev.event)));
										}
									}
								break;
								case TCommonEvent::Signal:
									if (onSignal)
									{
										PPeer p = ev.sender.s;
										Peer*ptr = p ? p.get() : ev.sender.p;
										if (ptr)
											onSignal(ev.channel,*ptr);
										else
										{
											throw Exception(CLOCATION,"peer reference lost in transit for signal on channel "+String(ev.channel));
										}
									}
								break;
								case TCommonEvent::Object:
									ev.receiver->Handle(ev.object,ev.sender);
								break;
							}
						}
						TCP_CATCH
					}
					resolutionBuffer.Clear();
				}
				TCP_CATCH
				resolving = false;
				if (terminateAfterResolve)
				{
					PostResolutionTermination();
					terminateAfterResolve = false;
				}
				if (postResolutionCallbacks.IsNotEmpty())
				{
					postResolutionCallbackLock.lock();
					foreach (postResolutionCallbacks,f)
						(*f)();
					postResolutionCallbacks.Clear();
					postResolutionCallbackLock.unlock();
				}
			}
			//mutex.release();
			FlushWaste();
		}

		void	Dispatcher::SignalPostResolutionTermination()
		{
			ASSERT__(resolving);
			terminateAfterResolve = true;
		}


		void Dispatcher::FlushWaste()
		{
			wasteMutex.lock();
				foreach(wasteBucket, w)
				{
					TCP_TRY
					{
						(*w)->Join(/*1000*/);
					}
					TCP_CATCH
					TCP_TRY
					{
						(*w)->Dissolve();
					}
					TCP_CATCH
				}
				wasteBucket.Clear();
			wasteMutex.release();
		}
	
		void Dispatcher::FlushPendingEvents()
		{
			FlushWaste();
			if (eventLock.ExclusiveLock(CLOCATION))
			{
				queue.Clear();
				eventLock.ExclusiveUnlock();
			}
		}
	
		RootChannel*	Dispatcher::getReceiver(UINT32 channelID, unsigned user_level)
		{
			if (relayTo)
				return relayTo->getReceiver(channelID,user_level);
			RootChannel*channel;
			if (!channel_map.Query(channelID,channel))	//not protecting this against invalid operations because netRead operations don't interfere with each other
			{
				#ifdef _DEBUG
					String dbg;
					bool found = false;
					channel_map.visitAllEntries([&dbg,&found,channelID](index_t index, RootChannel*channel)
					{
						dbg += String(index)+", ";
						if (index == channelID)
							found = true;
					});
					ASSERT__(!channel_map.IsSet(channelID));
					ASSERT__(!found);
					FATAL__("channel is not known: "+String(channelID)+" (known channels: "+dbg+")");	//for now, this is appropriate
				#endif
				if (verbose)
					std::cout << "Peer::ThreadMain(): no receiver available (nothing installed on this channel). ignoring package"<<std::endl;

				return NULL;
			}
			return channel;
		}
	
		Dispatcher::Dispatcher():async(true),is_locked(false),onEvent(NULL),onSignal(NULL),onIgnorePackage(NULL),onDeserializationFailed(NULL),resolving(false),terminateAfterResolve(false),anyEventsPending(false)
		{}
	
		Dispatcher::~Dispatcher()
		{
			//synchronized(mutex)
			//{
			//	TInbound inbound;
			//	while (object_queue >> inbound)
			//		Discard(inbound.object);
			//}
		}
	
	
		void	Dispatcher::OpenSignalChannel(UINT32 id, unsigned min_user_level)
		{
			ASSERT_IS_NULL__(relayTo);
			signal_map.Set(id,min_user_level);
		}
	
		void	Dispatcher::CloseSignalChannel(UINT32 id)
		{
			ASSERT_IS_NULL__(relayTo);
			signal_map.Unset(id);
		}
	
		void	Dispatcher::InstallChannel(RootChannel&channel)
		{
			ASSERT_IS_NULL__(relayTo);
			channel_map.Set(channel.id,&channel);
		}
	
		void	Dispatcher::UninstallChannel(RootChannel&channel)
		{
			ASSERT_IS_NULL__(relayTo);
			channel_map.Unset(channel.id);
		}


		bool	Dispatcher::QuerySignalMap(UINT32 channelID, unsigned&outMinChannelID) const
		{
			if (relayTo)
				return relayTo->QuerySignalMap(channelID,outMinChannelID);
			return signal_map.Query(channelID,outMinChannelID);
		}

		String	Dispatcher::DebugGetOpenSignalChannels() const
		{
			if (relayTo)
				return relayTo->DebugGetOpenSignalChannels();
			String result;
			signal_map.VisitAllKeys([&result](index_t index)
			{
				if (result.IsNotEmpty())
					result+=',';
				result += String(index);
			});
			return result;
		}

		String	Dispatcher::DebugGetOpenPackageChannels() const
		{
			if (relayTo)
				return relayTo->DebugGetOpenPackageChannels();
			String result;
			channel_map.VisitAllKeys([&result](index_t index)
			{
				if (result.IsNotEmpty())
					result+=',';
				result += String(index);
			});
			return result;
		}
		void Connection::HandleIncomingPackage(UINT32 channelID, Peer&sender, IReadStream&stream)
		{
			RootChannel*receiver = getReceiver(channelID,sender.userLevel);
			if (receiver)
			{
				if (verbose)
					std::cout << "Peer::ThreadMain(): deserializing"<<std::endl;
				
				try
				{
					PDispatchable object = receiver->Deserialize(stream,sender);
					if (stream.GetRemainingBytes()!=0)
						throw Except::Memory::SerializationFault(CLOCATION,"Object deserialization did not consume all available data. "+String(stream.GetRemainingBytes())+" byte(s) left in stream");
					if (verbose)
						std::cout << "Peer::ThreadMain(): deserialization succeeded, dispatching object"<<std::endl;
					HandleObject(receiver,sender,object);
				}
				catch (const std::exception&ex)
				{
					if (onDeserializationFailed)
						onDeserializationFailed(channelID,sender,ex.what());
					//#ifdef _DEBUG
					//	else if (!sender.destroyed)
					//		FATAL__("deserialization failed: "+String(ex.what()));	//for now, this is appropriate
					//#endif
				}
				//elif (verbose)
				//{
				//	std::cout << "Peer::ThreadMain(): deserialization failed, or refuses to return an object"<<std::endl;
				//}

			}
			else
			{
				#ifdef _DEBUG
					if (!stream.GetRemainingBytes())	//signal
					{
						unsigned min_user_level = 0;
						bool mapped = QuerySignalMap(channelID,min_user_level);
						ASSERT__(!mapped);
						ASSERT__(min_user_level == 0);
						FATAL__("channel/signal channel is not known: "+String(channelID)+" (known channels: "+DebugGetOpenPackageChannels()+", known signal channels: "+DebugGetOpenSignalChannels()+")");	//for now, this is appropriate
					}
				#endif
				if (onIgnorePackage)
				{
					onIgnorePackage(channelID,UINT32(stream.GetRemainingBytes()),sender);
				}
			}

		}

		bool Connection::HandleIncomingSignal(UINT32 channelID, Peer&sender)
		{
			unsigned min_user_level=0;

			if (QuerySignalMap(channelID,min_user_level))
			{
				if (sender.userLevel >= min_user_level)
				{
					HandleSignal(channelID,sender);
				}
				else
				{
					if (verbose)
						std::cout << "Peer::ThreadMain(): insufficient user level. ignoring package"<<std::endl;
					if (onIgnorePackage)
					{
						onIgnorePackage(channelID,0,sender);
					}
				}
				return true;
			}
			return false;
		}

	
	
		void ConnectionAttempt::ThreadMain()
		{

			if (verbose)
				std::cout << "ConnectionAttempt::ThreadMain() enter"<<std::endl;
			client->DisconnectPeer();
			client->Join();
			if (!Net::initNet())
			{
				client->SetError("Failed to initialize network ("+String(lastSocketError())+")");
				dispatcher->HandleEvent(Event::ConnectionFailed,*client);
				if (verbose)
					std::cout << "ConnectionAttempt::ThreadMain() exit: failed to initialize network"<<std::endl;
				return;
			}
			const String&url = connect_target;
			index_t separator = url.Find(':');
			if (separator == InvalidIndex)
			{
				client->SetError("Missing port in address line '"+url+"'");
				dispatcher->HandleEvent(Event::ConnectionFailed,*client);
				if (verbose)
					std::cout << "ConnectionAttempt::ThreadMain() exit: provided URL lacks port"<<std::endl;
				return;
			}
			String addr = url.subString(0,separator),
					s_port = url.subString(separator+1);
			USHORT port;
			if (!convert(s_port.c_str(),port))
			{
				client->SetError("Failed to parse port number '"+s_port+"'");
				dispatcher->HandleEvent(Event::ConnectionFailed,*client);
				if (verbose)
					std::cout << "ConnectionAttempt::ThreadMain() exit: provided port is not parsable"<<std::endl;
				return;
			}



			addrinfo hints,*remote_address;
			memset(&hints,0,sizeof(hints));
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;

			if (getaddrinfo(addr.c_str(),s_port.c_str(),&hints,&remote_address) != 0)
			{
				client->SetError("Unable to resolve address '"+String(addr)+"'");
				dispatcher->HandleEvent(Event::ConnectionFailed,*client);
				if (verbose)
					std::cout << "ConnectionAttempt::ThreadMain() exit: unable to decode IP address"<<std::endl;
				return;
			}

			addrinfo*actual_address = remote_address;

			String host = "";

			SOCKET socketHandle = INVALID_SOCKET;

			while (actual_address != NULL)
			{
				socketHandle = socket(actual_address->ai_family,actual_address->ai_socktype,actual_address->ai_protocol); //and create a new socket
				if (socketHandle != INVALID_SOCKET)
				{
					int i = 1;
					if (setsockopt( socketHandle, IPPROTO_TCP, TCP_NODELAY, (const char *)&i, sizeof(i)) == SOCKET_ERROR )
					{
						closesocket(socketHandle);
						socketHandle = INVALID_SOCKET;
					}
				}


				if (socketHandle == INVALID_SOCKET)
				{
					client->Fail("Socket creation failed");
					if (verbose)
						std::cout << "ConnectionAttempt::ThreadMain() exit: unable to create socket"<<std::endl;
					return;
				}

				if (connect(socketHandle,actual_address->ai_addr,(int)actual_address->ai_addrlen) == 0)
					break;
				if (host.IsNotEmpty())
					host += '/';
				host += ToString(*actual_address);
				actual_address = actual_address->ai_next;
				SwapCloseSocket(socketHandle);
			}

			if (actual_address == NULL)
			{
				freeaddrinfo(remote_address);
				client->SetError("'"+host+"' does not answer on port "+s_port);
				dispatcher->HandleEvent(Event::ConnectionFailed,*client);
				if (verbose)
					std::cout << "ConnectionAttempt::ThreadMain() exit: connection failed"<<std::endl;
				return;
			}
			memcpy(&client->address, actual_address->ai_addr, actual_address->ai_addrlen);
			client->addressLength = (socklen_t)actual_address->ai_addrlen;
			freeaddrinfo(remote_address);
			try
			{
				client->socketAccess->SetSocket(socketHandle);
				client->lastReceivedPackage.Update();
			}
			catch (const std::exception&exception)
			{
				client->SetError("Socket set operation to '"+host+"' failed: "+exception.what());
				dispatcher->HandleEvent(Event::ConnectionFailed,*client);
				if (verbose)
					std::cout << "ConnectionAttempt::ThreadMain() exit: connection failed"<<std::endl;
				return;
			}
			catch (...)
			{
				client->SetError("Socket set operation to '"+host+"' failed (no compatible exception given)");
				dispatcher->HandleEvent(Event::ConnectionFailed,*client);
				if (verbose)
					std::cout << "ConnectionAttempt::ThreadMain() exit: connection failed"<<std::endl;
				return;
			}
			client->writer.Begin(client->socketAccess);

			if (verbose)
				std::cout << "ConnectionAttempt::ThreadMain(): starting client thread"<<std::endl;
			client->Start();
			if (verbose)
				std::cout << "ConnectionAttempt::ThreadMain() exit: connection established"<<std::endl;
			if (verbose)
				std::cout << "ConnectionAttempt::ThreadMain(): sending 'connection established' event"<<std::endl;
			dispatcher->HandleEvent(Event::ConnectionEstablished,*client);

		}


		/*virtual override*/ String		Client::AddressToString(bool includePort) const
		{
			if (this->addressLength == 0)
			{
				String copy;
				attemptHostCopyLock.lock();
				copy = attemptHostCopy;
				attemptHostCopyLock.unlock();
				if (!includePort)
				{
					index_t at = copy.Find(':');
					if (at != InvalidIndex)
						copy.Erase(at);
				}
				return copy;
			}
			return TCP::ToString(address,addressLength,includePort);
		}


		bool	Client::Connect(const String&url)
		{
			connectionLock.lock();
			if (attempt.IsRunning())
			{
				attempt.Join();
				connectionLock.unlock();
				return !socketAccess->IsClosed();
			}
			ConnectAsync(url);
			attempt.Join();
			ASSERT__(!attempt.IsRunning());
			connectionLock.unlock();
			return !socketAccess->IsClosed();
		}
	
		void	Client::ConnectAsync(const String&url)
		{
			connectionLock.lock();
			ASSERT__(!IsSelf());
			attemptHostCopyLock.lock();
			attemptHostCopy = url;
			attemptHostCopyLock.unlock();
			if (attempt.IsRunning())
			{
				if (!attempt.IsDone())
				{
					connectionLock.unlock();
					return;
				}
				attempt.Join();
			}
			
			attempt.connect_target = url;
	//		attempt.client = this;
			attempt.Start();
			connectionLock.unlock();
		}

		void	Peer::handleUnexpectedSendResult(int result)
		{
			writer.Terminate();
			if (result <= 0)
			{
				if (socketAccess->IsClosed())
				{
					if (verbose)
						std::cout << "Peer::succeeded() exit: socket handle reset by remote operation"<<std::endl;
					return;
				}
				SetError("");
				owner->HandleEvent(Event::ConnectionLost,*this);
				socketAccess->CloseSocket();
				owner->OnDisconnect(this,Event::ConnectionLost);
				if (verbose)
					std::cout << "Peer::succeeded() exit: result is "<<result<<std::endl;
				return;
			}
			if (socketAccess->IsClosed())
			{
				if (verbose)
					std::cout << "Peer::succeeded() exit: socket handle reset by remote operation"<<std::endl;
				return;
			}
			SetError("Connection lost to "+ToString()+" ("+lastSocketError()+")");
			socketAccess->CloseSocket();
			owner->HandleEvent(Event::ConnectionLost,*this);
			owner->OnDisconnect(this, Event::ConnectionLost);
			if (verbose)
				std::cout << "Peer::succeeded() exit: result was unexpected. socket closed"<<std::endl;
		}
	
		bool	Peer::succeeded(int result, size_t desired)
		{
			if (verbose)
				std::cout << "Peer::succeed() enter: result="<<result<<", desired="<<desired<<std::endl;
			if (result == (int)desired)
			{
				if (verbose)
					std::cout << "Peer::succeed() exit: match"<<std::endl;
				return true;
			}
			handleUnexpectedSendResult(result);
			return false;
		}
	
		void	Peer::SendData(const Package&p)
		{
			if (socketAccess->IsClosed())
				throw Except::IO::Network::ConnectionLost(CLOCATION,"Socket access is closed");
			writer.Write(p);
		}
	
	
		void	Peer::NetRead(BYTE*current, size_t size)
		{
			if (verbose)
				std::cout << "Peer::netRead() enter: size="<<size<<std::endl;
			BYTE*end = current+size;
			while (current < end)
			{
				int size = socketAccess->Read(current,end-current);
				if (size <= 0)
					throw Except::IO::Network::ConnectionLost(CLOCATION,"SocketAccess::Read() returned "+String(size));
				current += size;
				lastReceivedPackage.Update();
			}
			if (verbose)
				std::cout << "Peer::netRead() exit"<<std::endl;
		}

		/*virtual override*/ void	Peer::Read(void*target, serial_size_t size)
		{
			if (!size)
				return;
			if (size > remaining_size)
				throw Except::IO::Network::ReadLogicFault(CLOCATION,"Trying to read more data from stream ("+String(size)+"), than provided by remaining package data ("+String(remaining_size)+")");
			NetRead((BYTE*)target,size);
			remaining_size -= size;
		}

		/*virtual override*/ serial_size_t				Peer::GetRemainingBytes() const
		{
			return remaining_size;
		}

	
		///*virtual override*/ bool	Peer::Write(const void*target, serial_size_t size)
		//{
		//	if (verbose)
		//		std::cout << "Peer::write() enter"<<std::endl;
		//	if (!size)
		//		return true;
		//	if (verbose)
		//		std::cout << "Peer::write(): sending "<<size<<" byte(s)"<<std::endl;
		//	if (size > remaining_write_size)
		//	{
		//		FATAL__("Send-size ("+String(size)+" byte(s)) exceeds remaining available write size ("+String(remaining_write_size)+" byte(s))");
		//	}
		//	if (!succeeded(socketAccess->Write(target,size),size))
		//	{
		//		if (verbose)
		//			std::cout << "Peer::write(): failed to write data to socket"<<std::endl;
		//		return false;
		//	}
		//	remaining_write_size -= size;
		//	if (verbose)
		//		std::cout << "Peer::write() exit: "<<remaining_write_size<<" byte(s) remaining to write"<<std::endl;
		//	return true;
		//}
	
		static BYTE	dump_buffer[2048];
		void	Peer::SendPackage(const Package&object, unsigned minUserLevel)
		{
			if (this->userLevel < minUserLevel)
				return;
			if (writer.connectionLost)
				return;
			writer.Write(object);
		}


		void						PeerWriter::Begin(SocketAccess*access)
		{
			Terminate();
			connectionLost = false;
			pipe.clear();
			accessPointerLock.lock();
			this->accessPointer = access;
			accessPointerLock.unlock();
			this->Start();
		}

		void						PeerWriter::Update(SocketAccess*access)
		{
			accessPointerLock.lock();

			this->accessPointer = access;
			accessPointerLock.unlock();
		}
		void						PeerWriter::Terminate()
		{
			TCP_TRY
			{
				accessPointerLock.lock();
					accessPointer = nullptr;
				accessPointerLock.unlock();
			}
			TCP_CATCH

			connectionLost = true;
			TCP_TRY
			{
				pipe.clear();
			}
			TCP_CATCH
			TCP_TRY
			{
				pipe.OverrideSignalNow();
			}
			TCP_CATCH
			this->Join();
		}


		void	PeerWriter::ThreadMain()
		{
			Ctr::Vector0<Ctr::Array<BYTE>,Swap>	storage;
			for (;;)
			{

				if (accessPointer && !connectionLost)
					pipe.WaitForContent();

				accessPointerLock.lock();
				if (!accessPointer || connectionLost /*|| pipe.IsEmpty()*/)	//through weird synchronization events, the pipe might be empty
				{
					connectionLost = true;
					accessPointer = nullptr;
					if (verbose)
						std::cout << __func__<<" exit"<<std::endl;
					accessPointerLock.unlock();
					return;
				}
				storage.Clear();
				pipe.SignalRead();
				if (verbose)
					std::cout << __func__<<" loop "<<std::endl;
	
				storage.MoveAppend(pipe.begin(),pipe.Count());
				pipe.ExitRead();


				for (auto package : storage)
				{
					bool did_write = false;
					{
						UINT32*data = (UINT32*)package.pointer();
						int rs = accessPointer->Write(package.pointer(),package.size());
						if ((size_t)rs != package.size())
						{
							if (verbose)
								std::cout << __func__ << " exit: failed to send package chunk"<<std::endl;
							connectionLost = true;
							accessPointer = nullptr;
							accessPointerLock.unlock();
							parent->handleUnexpectedSendResult(rs);
							return;
						}
					}
			
				}
				accessPointerLock.unlock();

				storage.Clear();
				if (verbose)
					std::cout << __func__ << " success "<<std::endl;
			}
		}
	
		bool	Peer::SendSignal(UINT32 channel_id)
		{
			if (verbose)
				std::cout << "Peer::sendSignal() enter: channel_id="<<channel_id<<std::endl;
			//synchronized(write_mutex)
			{
				if (!writer.WriteSignal(channel_id))
				{
					if (verbose)
						std::cout << __func__<<" exit: failed to send channel id"<<std::endl;
					return false;
				}
				if (verbose)
					std::cout << __func__ << " exit"<<std::endl;
				return true;
			}
		}

		void	Peer::CloseWriterDown(Event::event_t ev, const String&errorMessage)
		{
			writer.Terminate();
			if (!socketAccess->IsClosed())
			{
				socketAccess->CloseSocket();
				SetError(errorMessage);
				owner->HandleEvent(ev,*this);
				if (!destroyed)
					owner->OnDisconnect(this, ev);
			}
		}
	
		void	Peer::ThreadMain()
		{
			if (verbose)
				std::cout << "Peer::ThreadMain() enter"<<std::endl;
			AssertIsSelf();	//this should really be implied. as it turns out due to whatnot kind of errors, sometimes it hicks up
			try
			{
				while (socketAccess && !socketAccess->IsClosed())
				{
					UINT32	header[2];
					NetRead((BYTE*)header,sizeof(header));
					if (verbose)
						std::cout << "Peer::ThreadMain(): received header: channel="<<header[0]<<" size="<<header[1]<<std::endl;
					//lastReceivedPackage = timer.Now();
					remaining_size = (serial_size_t)header[1];
					if (remaining_size > owner->GetSafePackageSize())
					{
						throw Except::IO::Network::ProtocolViolation("Maximum safe package size ("+String(owner->GetSafePackageSize()/1024)+"KB) exceeded by "+String((remaining_size-owner->GetSafePackageSize())/1024)+"KB");
					}
					UINT32	channel_index = header[0];
					//std::cout << "has package "<<channel_index<<"/"<<remaining_size<<std::endl;

					if (remaining_size || !owner->HandleIncomingSignal(channel_index,*this))
					{
						owner->HandleIncomingPackage(channel_index,*this,*this);
						while (remaining_size > sizeof(dump_buffer))
							NetRead(dump_buffer,sizeof(dump_buffer));
						NetRead(dump_buffer,remaining_size);
					}
				}
			}
			catch (const Except::IO::Network::ConnectionLost&ex)
			{
				CloseWriterDown(Event::ConnectionLost,"Connection lost to "+ToString()+" ("+lastSocketError()+")");
			}
			catch (const std::exception&ex)
			{
				CloseWriterDown(Event::ConnectionLost,ex.what());
			}
			catch (...)
			{
				CloseWriterDown(Event::ConnectionLost,"...");
			}

			if (verbose)
				std::cout << "Peer::ThreadMain() exit: socket handle reset by remote operation"<<std::endl;
		}
	
		void Peer::DisconnectPeer()
		{
			if (verbose)
				std::cout << __func__ <<" enter"<<std::endl;
			writer.Terminate();
			if (socketAccess && !socketAccess->IsClosed())
			{
				if (verbose)
					std::cout << __func__ << ": graceful shutdown: invoking handlers and closing socket"<<std::endl;
				//SetError("");
				owner->HandleEvent(Event::ConnectionLost,*this);
				socketAccess->CloseSocket();
				owner->OnDisconnect(this,Event::ConnectionLost);
			}
			elif (verbose)
				std::cout << __func__ << ": socket handle reset by remote operation"<<std::endl;
			if (verbose)
				std::cout << __func__ << " exit"<<std::endl;
		}

		void	Peer::SetSelf(const PPeer&p)
		{
			ASSERT__(p.get()==this);
			self = p;
		}

		void		Server::ThreadMain()
		{
	#if 1
			if (verbose)
				std::cout << "Server::ThreadMain() enter"<<std::endl;
			sockaddr_storage	addr;
			while (socket_handle != INVALID_SOCKET)
			{
				if (IsAsync())
				{
					FlushWaste();
				}


				SOCKET handle;
				socklen_t size = (socklen_t)sizeof(addr);
				handle = accept( socket_handle, (sockaddr*)&addr,&size);

				if (handle != INVALID_SOCKET)
				{
					int i = 1;
					if (setsockopt( handle, IPPROTO_TCP, TCP_NODELAY, (const char *)&i, sizeof(i)) == SOCKET_ERROR )
					{
						closesocket(handle);
						handle = INVALID_SOCKET;
					}
				}

				if (handle == INVALID_SOCKET)
				{
					if (socket_handle == INVALID_SOCKET)
					{
						if (verbose)
							std::cout << "Server::ThreadMain() exit: socket handle reset by remote operation"<<std::endl;
						return;
					}
					Fail("accept() call failed");
					if (verbose)
						std::cout << "Server::ThreadMain() exit: accept() operation failed"<<std::endl;
					return;
				}
				PPeer peer(new Peer(this));
				peer->SetSelf(peer);
				peer->address = addr;
				peer->addressLength = size;

				peer->SetCloneOfSocketAccess(socketAccess);
				peer->socketAccess->SetSocket(handle);

				if (verbose)
					std::cout << "Server::ThreadMain(): acquiring write lock for client create"<<std::endl;
				if (clientMutex.BeginWrite())
				{
					if (this->is_shutting_down)
					{
						clientMutex.EndWrite();
						continue;
					}
					clientList << peer;
					clientMutex.EndWrite();

					if (verbose)
						std::cout << "Server::ThreadMain(): released write lock"<<std::endl;
					SetError("");
					peer->writer.Begin(peer->socketAccess);

					if (verbose)
						std::cout << "Server::ThreadMain():	starting peer thread"<<std::endl;
					peer->Start();
					HandleEvent(Event::ConnectionEstablished,*peer);
				}
				else
					if (verbose)
						std::cout << "Server::ThreadMain():	ignoring client"<<std::endl;
			}
			if (verbose)
				std::cout << "Server::ThreadMain() exit: socket handle reset by remote operation"<<std::endl;
	#endif /*0*/
		}
	
		void		Client::Disconnect()
		{
			if (IsResolving())
			{
				Dispatcher::SignalPostResolutionTermination();
				return;
			}
			if (!IsAttemptingToConnect() && !IsConnected())
				return;

			connectionLock.lock();
			eventLock.Block(CLOCATION);
			attempt.Join();
			DisconnectPeer();
			Join();
			eventLock.Unblock(CLOCATION);

			Dispatcher::FlushPendingEvents();
			HandleEvent(TCP::Event::ConnectionLost,*this);
			connectionLock.unlock();
		}


		void		Peer::Fail(const String&message)
		{
			if (verbose)
				std::cout << "Client::fail() invoked: "<<message<<std::endl;
			SetError(message+" ("+lastSocketError()+")");
			DisconnectPeer();
		}
	
		void		Server::Fail(const String&message)
		{
			if (verbose)
				std::cout << "Server::fail() enter: "<<message<<std::endl;
			if (is_shutting_down)
			{
				if (verbose)
					std::cout << "Server::fail(): status: shutting down. message ignored"<<std::endl;
				return;
			}
			SetError(message+" ("+lastSocketError()+")");
			if (socket_handle != INVALID_SOCKET)
			{
				if (verbose)
					std::cout << "Server::fail(): closing listen socket"<<std::endl;
				HandleEvent(Event::ConnectionLost,centralPeer);
				SwapCloseSocket(socket_handle);
			}
			if (verbose)
				std::cout << "Server::fail(): terminating service"<<std::endl;
			eventLock.Block(CLOCATION);
				EndService();
			eventLock.Unblock(CLOCATION);
			if (verbose)
				std::cout << "Server::fail() exit"<<std::endl;
		}

		bool		Server::StartService(USHORT port, bool limitToLocalhost/*=false*/, USHORT*outPort/*=nullptr*/)
		{
			return ExtStartService(port,Protocol::IPv4,limitToLocalhost,outPort);
		}

		bool		Server::ExtStartService(USHORT port, Protocol proto, bool limitToLocalhost/*=false*/, USHORT*outPort/*=nullptr*/)
		{
			ASSERT__(!IsResolving());
			if (verbose)
				std::cout << "Server::startService() enter"<<std::endl;
			if (IsRunning())
			{
				if (!IsDone())
				{
					SetError("connection already active");
					if (verbose)
						std::cout << "Server::startService() exit: service is already online"<<std::endl;
					return false;
				}
				Join();
			}
			is_shutting_down = false;
			if (!Net::initNet())
			{
				SetError("Net failed to initialize");
				if (verbose)
					std::cout << "Server::startService() exit: failed to initialize the network"<<std::endl;
				return false;
			}

			struct sockaddr_storage self;
			socklen_t               selflen;
			memset(&self,0,sizeof(self));

			switch (proto)
			{
				case Protocol::IPv4:
				{
					socket_handle = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP); //and create a new socket
					sockaddr_in&self4 = (sockaddr_in&)self;
					self4.sin_family = AF_INET;
					self4.sin_addr.s_addr = htonl(limitToLocalhost ? INADDR_LOOPBACK : INADDR_ANY);
					self4.sin_port = htons(port);
					selflen = sizeof(self4);
				}
				break;
				case Protocol::IPv6:
					socket_handle = socket(AF_INET6,SOCK_STREAM,IPPROTO_TCP); //and create a new socket
					sockaddr_in6&self6 = (sockaddr_in6&)self;
					self6.sin6_family = AF_INET6;
					self6.sin6_addr = limitToLocalhost ? in6addr_loopback : in6addr_any;
					self6.sin6_port = htons(port);
					selflen = sizeof(self6);
				break;
			}

			if (socket_handle == INVALID_SOCKET)
			{
				Fail("socket creation failed");
				if (verbose)
					std::cout << "Server::startService() exit: failed to create listen socket"<<std::endl;
				return false;
			}
			this->port = port;
		
	
			if (bind(socket_handle, (sockaddr*)&self,selflen))
			{
				Fail("socket bind failed");
				if (verbose)
					std::cout << "Server::startService() exit: failed to bind listen socket"<<std::endl;
				return false;
			}
			if (listen(socket_handle,0))
			{
				Fail("socket listen failed");
				if (verbose)
					std::cout << "Server::startService() exit: failed to start listen operation"<<std::endl;
				return false;
			}

			if (port == 0)
			{
				struct sockaddr_in sin;
				socklen_t len = sizeof(sin);
				if (getsockname(socket_handle, (struct sockaddr *)&sin, &len) == -1)
				{
					Fail("Unable to fetch sock-name on new socket");
					return false;
				}
				else
				{
					UINT16 p = ntohs(sin.sin_port);
					//if (port != 0 && port != p)
					//{
					//	Fail("Read port does not match expected port");
					//	return false;
					//}
					port = p;
					if (outPort)
						(*outPort) = p;
				}
			}
			else
				if (outPort)
					(*outPort) = port;

			if (verbose)
				std::cout << "Server::startService(): socket created, bound, and now listening. starting thread"<<std::endl;
			Start();
			if (verbose)
				std::cout << "Server::startService() exit: service is online"<<std::endl;
			return true;
		}

		PPeer		Server::GetSharedPointerOfClient(Peer*peer)
		{
			if (verbose)
				std::cout << __func__ " enter" << std::endl;
			if (peer)
			{
				if (clients_locked)
				{
					if (verbose)
						std::cout << __func__ " exit: clients are locked" << std::endl;
					return PPeer();
				}
				if (verbose)
					std::cout << __func__": acquiring read lock for client lookup" << std::endl;
				clientMutex.BeginRead();
				if (verbose)
					std::cout << __func__ ": lock acquired. searching" << std::endl;
				index_t at = InvalidIndex;
				foreach(clientList, cl)
					if (cl->get() == peer)
					{
						at = cl - clientList.pointer();
						break;
					}
				PPeer p;
				if (at != InvalidIndex)
				{
					if (verbose)
						std::cout << __func__ " client found" << std::endl;
					p = clientList[at];
				}
				clientMutex.EndRead();
				if (verbose)
					std::cout << __func__": released read lock. returning result" << std::endl;
				return p;
			}
			if (verbose)
				std::cout << __func__ " exit" << std::endl;
			return PPeer();
		}

	
		void		Server::OnDisconnect(const Peer*peer, event_t event)
		{
			if (verbose)
				std::cout << "Server::onDisconnect() enter"<<std::endl;
			if (peer)
			{
				if (clients_locked)
				{
					if (verbose)
						std::cout << "Server::onDisconnect() exit: clients are locked" << std::endl;
					return;
				}
				if (verbose)
					std::cout << "Server::onDisconnect(): acquiring write lock for client disconnect" << std::endl;
				if (!clientMutex.BeginWrite())
				{
					if (verbose)
						std::cout << "Server::onDisconnect(): Server locked down for termination. Aborting event handling" << std::endl;
					return;
				}
				if (verbose)
					std::cout << "Server::onDisconnect(): lock acquired. dropping peer" << std::endl;
				index_t at = InvalidIndex;
				foreach(clientList, cl)
					if (cl->get() == peer)
					{
						at = cl - clientList.pointer();
						break;
					}
				PPeer p;
				if (at != InvalidIndex)
				{
					p = clientList[at];
					clientList.Erase(at);
				}
				clientMutex.EndWrite();
				if (verbose)
					std::cout << "Server::onDisconnect(): released write lock. discarding peer" << std::endl;
				if (p)
					HandlePeerDeletion(p);
			}
			if (verbose)
				std::cout << "Server::onDisconnect() exit"<<std::endl;
		}

		void		Server::EndService()
		{
			if (!IsOnline())
				return;
			if (IsResolving())
			{
				Dispatcher::SignalPostResolutionTermination();
				return;
			}

			if (verbose)
				std::cout << "Server::endService() enter"<<std::endl;
			if (is_shutting_down)
			{
				if (verbose)
					std::cout << "Server::endService() exit: already shutting down"<<std::endl;
				return;
			}
			is_shutting_down = true;
			eventLock.Block(CLOCATION);
			TCP_TRY
			{
				if (verbose)
					std::cout << "Server::endService(): acquiring write lock for service termination"<<std::endl;
				ASSERT__(clientMutex.BeginWrite(true));
					if (verbose)
						std::cout << "Server::endService(): lock acquired. erasing client peers"<<std::endl;
					clients_locked = true;
					foreach (clientList,cl)
					{
						(*cl)->Destroy();
						TCP_TRY
						{
							(*cl)->Join(/*1000*/);
						}
						TCP_CATCH
						TCP_TRY
						{
							(*cl)->Dissolve();
						}
						TCP_CATCH
					}
					clientList.Clear();
					clients_locked = false;
				clientMutex.EndWrite();
				if (verbose)
					std::cout << "Server::endService(): released write lock"<<std::endl;
			
				if (socket_handle != INVALID_SOCKET)
				{
					if (verbose)
						std::cout << "Server::endService(): closing listen socket"<<std::endl;
					SwapCloseSocket(socket_handle);
				}
				if (verbose)
					std::cout << "Server::endService(): awaiting listen thread termination"<<std::endl;
				FlushPendingEvents();
				Join(/*1000*/);
			}
			TCP_CATCH
			if (eventLock.Unblock(CLOCATION))	//Server::Fail() blocks events and calls this method, thus generating recursion. Let's not clear error messages then
				SetError("");
			if (verbose)
				std::cout << "Server::endService(): sending connection closed event"<<std::endl;
			HandleEvent(Event::ConnectionLost,centralPeer);
			if (verbose)
				std::cout << "Server::endService() exit: service is offline"<<std::endl;
			ASSERT__(!IsOnline());
		}
	

		void		Server::SendPackage(const Package&object, unsigned minUserLevel)
		{
			if (verbose)
				std::cout << "Server::sendObject(): acquiring read lock for message send"<<std::endl;
			clientMutex.BeginRead();
				if (verbose)
					std::cout << "Server::sendObject(): lock acquired"<<std::endl;
				for (index_t i = 0; i < clientList.Count(); i++)
				{
					const PPeer peer = clientList[i];	//should be copy? let's assume so for now
					if (peer->socketAccess->IsClosed())
						continue;	//we assume this case is already handled
					if (peer->userLevel < minUserLevel)
						continue;
					try
					{
						peer->SendData(object);
					}
					catch (const std::exception&ex)
					{
						clientMutex.EndRead();
						if (verbose)
						{
							std::cout << "Server::sendObject(): released read lock"<<std::endl;
							std::cout << "Server::sendObject(): acquiring write lock for peer termination"<<std::endl;
						}
						if (clientMutex.BeginWrite())
						{
							clients_locked = true;
							if (verbose)
								std::cout << "Server::sendObject(): erasing peer "<<peer->ToString()<<std::endl;
							clientList.FindAndErase(peer);
							clients_locked = false;
							clientMutex.EndWrite();
						}
						else
							FATAL__("TCP Server: Failed to acquire write-lock. This should not happen");

						if (verbose)
						{
							std::cout << "Server::sendObject(): released write lock"<<std::endl;
							std::cout << "Server::sendObject(): acquiring read lock for continued message send"<<std::endl;
						}
						HandlePeerDeletion(peer);
						clientMutex.BeginRead();
						i--;
					}
				}
			clientMutex.EndRead();
			if (verbose)
			{
				std::cout << "Server::sendObject(): released read lock"<<std::endl;
				std::cout << "Server::sendObject() exit"<<std::endl;
			}
		}


		void		Server::SendPackage(const PPeer&exclude, const Package&p, unsigned minUserLevel)
		{
			if (verbose)
				std::cout << "Server::sendObject(): acquiring read lock for message send"<<std::endl;
			clientMutex.BeginRead();
				for (index_t i = 0; i < clientList.Count(); i++)
				{
					const PPeer peer = clientList[i];
					if (peer == exclude)
						continue;
					if (peer->socketAccess->IsClosed())
						continue;	//we assume this case is already handled
					if (peer->userLevel < minUserLevel)
						continue;
					try
					{
						peer->SendData(p);
					}
					catch (const std::exception&ex)
					{
						clientMutex.EndRead();
						if (verbose)
						{
							std::cout << "Server::sendObject(): released read lock"<<std::endl;
							std::cout << "Server::sendObject(): acquiring write lock for peer termination"<<std::endl;
						}
						if (clientMutex.BeginWrite())
						{
							clients_locked = true;
							if (verbose)
								std::cout << "Server::sendObject(): erasing peer "<<peer->ToString()<<std::endl;
							clientList.FindAndErase(peer);
							clients_locked = false;
							clientMutex.EndWrite();
						}
						else
							FATAL__("TCP Server: Failed to acquire write-lock. This should not happen");

						if (verbose)
						{
							std::cout << "Server::sendObject(): released write lock"<<std::endl;
							std::cout << "Server::sendObject(): acquiring read lock for continued message send"<<std::endl;
						}
						clientMutex.BeginRead();
						i--;
					}
				}
			clientMutex.EndRead();
			if (verbose)
			{
				std::cout << "Server::sendObject(): released read lock"<<std::endl;
				std::cout << "Server::sendObject() exit"<<std::endl;
			}
		}
	
		
	


	 }
 }
