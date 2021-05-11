#include "../global_root.h"
#include "node.h"

/******************************************************************

Network-interface.

******************************************************************/

namespace DeltaWorks
{

	namespace Net
	{
		static bool					net_initialized(false);

		static SynchronizedLogFile			log_file( StringType::Cast("net.log"),true);

			/* initNet *
		Used to initialize network and check versions.
		This method is called automatically by any created NetItems, though its result is ignored.
		True result is implicted.
		*/

		bool DOMAIN_A initNet()
		{
			#if SYSTEM==WINDOWS
				if (net_initialized)
					return true;
				WSADATA data;
				if (WSAStartup(MAKEWORD(2, 2), &data))
					return false;
				if (LOBYTE(data.wVersion) != 2 || HIBYTE(data.wVersion) != 2)
					return false;
				net_initialized = true;
				return true;
			#else
				return true;
			#endif
		}


	#if 0
		String Name::ToString() const
		{
			char str[INET6_ADDRSTRLEN];
			switch (ss_family)
			{
				case AF_INET:
				{
					sockaddr_in copy = (const sockaddr_in&)*this;
					inet_ntop(ss_family,&copy.sin_addr,str,sizeof(str));
					return String(str) + ":"+String(((const sockaddr_in&)*this).sin_port);
				}
				break;
				case AF_INET6:
				{
					sockaddr_in6 copy = (const sockaddr_in6&)*this;
					inet_ntop(ss_family,&copy.sin6_addr,str,sizeof(str));
					return "[" + String(str) + "]:"+String(((const sockaddr_in6&)*this).sin6_port);
				}
				break;
				default:
				break;
			}
			return "NA";
		}
	#endif /*0*/
	

		void sysmessage(Node*node, const String&str)
		{
			log_file << str<<nl;
		}


		const char*msg2str(UINT16 message)
		{
			#define ECASE(token) case token: return #token
			switch (message)
			{
				ECASE(NET_RECOVERY_FAILED);
				ECASE(NET_CL_REQUEST_CONNECT);
				ECASE(NET_SV_CONFIRM_CONNECT);
				ECASE(NET_SV_FORCE_DISCONNECT);
				ECASE(NET_SV_SERVER_FULL);
				ECASE(NET_SV_SERVER_SHUTDOWN);
				ECASE(NET_SV_DENY_CONNECT);
				ECASE(NET_DISCONNECT);
				ECASE(NET_SYNC_REQUEST);
				ECASE(NET_SYNC_REPLY);
				ECASE(NET_TICK);
				ECASE(NET_USER);
				ECASE(NET_TIMEOUT);
				ECASE(NET_CONNECTION_DIED);
			};
			return "undefined message";
		}

		const char*	GetLastError()
		{
			#if SYSTEM==WINDOWS
				return Net::err2str(WSAGetLastError());
			#else
				return Net::err2str(errno);
			#endif
		}

		const char*err2str(unsigned code)
		{
			#if SYSTEM!=WINDOWS
			switch (code)
			{
				ECASE(EADDRINUSE);
				ECASE(EADDRNOTAVAIL);
				ECASE(EAFNOSUPPORT);
				ECASE(EBADF);
				ECASE(EINVAL);
				ECASE(ENOTSOCK);
				ECASE(EOPNOTSUPP);
				ECASE(EACCES);
				ECASE(EDESTADDRREQ);
				ECASE(EISDIR);
				ECASE(EIO);
				ECASE(ELOOP);
				ECASE(ENAMETOOLONG);
				ECASE(ENOENT);
				ECASE(ENOTDIR);
				ECASE(EROFS);
				ECASE(EISCONN);
				ECASE(ENOBUFS);
			}
			#else
			switch (code)
			{
				ECASE(WSA_INVALID_HANDLE);
				ECASE(WSA_NOT_ENOUGH_MEMORY);
				ECASE(WSA_INVALID_PARAMETER);
				ECASE(WSA_OPERATION_ABORTED);
				ECASE(WSA_IO_INCOMPLETE);
				ECASE(WSA_IO_PENDING);
				ECASE(WSAEINTR);
				ECASE(WSAEBADF);
				ECASE(WSAEACCES);
				ECASE(WSAEFAULT);
				ECASE(WSAEINVAL);
				ECASE(WSAEMFILE);
				ECASE(WSAEWOULDBLOCK);
				ECASE(WSAEINPROGRESS);
				ECASE(WSAEALREADY);
				ECASE(WSAENOTSOCK);
				ECASE(WSAEDESTADDRREQ);
				ECASE(WSAEMSGSIZE);
				ECASE(WSAEPROTOTYPE);
				ECASE(WSAENOPROTOOPT);
				ECASE(WSAEPROTONOSUPPORT);
				ECASE(WSAESOCKTNOSUPPORT);
				ECASE(WSAEOPNOTSUPP);
				ECASE(WSAEPFNOSUPPORT);
				ECASE(WSAEAFNOSUPPORT);
				ECASE(WSAEADDRINUSE);
				ECASE(WSAEADDRNOTAVAIL);
				ECASE(WSAENETDOWN);
				ECASE(WSAENETUNREACH);
				ECASE(WSAENETRESET);
				ECASE(WSAECONNABORTED);
				ECASE(WSAECONNRESET);
				ECASE(WSAENOBUFS);
				ECASE(WSAEISCONN);
				ECASE(WSAENOTCONN);
				ECASE(WSAESHUTDOWN);
				ECASE(WSAETOOMANYREFS);
				ECASE(WSAETIMEDOUT);
				ECASE(WSAECONNREFUSED);
				ECASE(WSAELOOP);
				ECASE(WSAENAMETOOLONG);
				ECASE(WSAEHOSTDOWN);
				ECASE(WSAEHOSTUNREACH);
				ECASE(WSAENOTEMPTY);
				ECASE(WSAEPROCLIM);
				ECASE(WSAEUSERS);
				ECASE(WSAEDQUOT);
				ECASE(WSAESTALE);
				ECASE(WSAEREMOTE);
				ECASE(WSAEDISCON);
				ECASE(WSASYSNOTREADY);
				ECASE(WSAVERNOTSUPPORTED);
				ECASE(WSANOTINITIALISED);
				ECASE(WSAHOST_NOT_FOUND);
				ECASE(WSATRY_AGAIN);
				ECASE(WSANO_RECOVERY);
				ECASE(WSANO_DATA);
			}
			#endif
			#undef ECASE
			return code == 0 ? "" : "Unknown Error";
		}


		String	 ip2str(UINT32 ip)
		{
			BYTE*field = (BYTE*)&ip;
			return IntToStr(field[0])+"."+IntToStr(field[1])+"."+IntToStr(field[2])+"."+IntToStr(field[3]);
		}

	#if 0
		void DOMAIN_M Node::ThreadMain()//thread to manage read-recall
		{
			while (read());
		}

		void Node::netTimeThread()//thread to keep the connection busy
		{
			TPackageHead tick;
			tick.lo = NET_TICK;
			tick.hi = 0;
			tick.config = 0;
			while (true)
			{
				#if SYSTEM==WINDOWS
					Sleep(NET_DOT);
				#elif SYSTEM==UNIX
					usleep(NET_DOT*1000);
				#endif
				sendto(node,(char*)&tick,sizeof(tick),0,(sockaddr*)&self,sizeof(self));
				//since package is send to self tick will be executed on receive. This way the two main threads
				//won't overlap.
			}
		}


		bool netAccept(const NetIdent&name)
		{
			return true;
		}



		ClientEventItem::ClientEventItem(_netClientEvent event, const Name&name, UINT32 para):exec(event),target(name),parameter(para)
		{}

		ClientEventItem::~ClientEventItem()
		{
			exec(target,parameter);
		}

		ClientDropItem::ClientDropItem(_netClientDrop event, Connection*connection, UINT32 para):exec(event),target(connection),parameter(para)
		{}

		ClientDropItem::~ClientDropItem()
		{
			exec(target,parameter);
		}


		EventItem::EventItem(_netEvent event, Connection*connection):exec(event),target(connection)
		{}

		EventItem::~EventItem()
		{
			exec(target);
		}

		TransferItem::TransferItem():data(NULL),data_len(0),channel(0),sender(NULL),exec(NULL)
		{}

		TransferItem::~TransferItem()
		{
			if (exec)
				exec(sender,channel,data,data_len);
			dealloc(data);
		}


		void Package::setTransferSize(UINT32 size)
		{
			head.custom[0] = (size>>16);
			head.custom[1] = (size>>8);
			head.custom[2] = size;
		//	info->transfer_size = size;
		}

		UINT32 Package::getTransferSize()  const
		{
			return ((UINT32)head.custom[0]<<16)|((UINT32)head.custom[1]<<8)|head.custom[2];
		//	return info->transfer_size;
		}



		Package::Package(BYTE flags, UINT16 lo, UINT16 hi, UINT32 transfer_size):head(*(TPackageHead*)field)
		{
			setTransferSize(transfer_size);
			head.flags = flags;
			head.lo = lo;
			head.hi = hi;
		}

		Package::Package(BYTE flags, UINT16 lo, UINT16 hi):head(*(TPackageHead*)field)
		{
			head.config = 0;
			head.flags = flags;
			head.lo = lo;
			head.hi = hi;
		}



		void* Package::data()
		{
			return field+sizeof(head);
		}




		Task::Task(UINT16 transfer_id):data_size(0),data(NULL),package(NULL),completed(0),packages(0),out(NULL),
											  closed(false),undef(true),transfer(transfer_id),exec(NULL)
		{
			in = timer.netTime();
		}

		Task::Task(UINT16 transfer_id, UINT32 size):data_size(size),data(alloc<BYTE>(size)),completed(0),
												out(NULL),closed(false),undef(false),transfer(transfer_id),exec(NULL)
		{
			packages = data_size/max_package_size;
			if (data_size%max_package_size || !packages)
				packages++;
			package = alloc<bool>(packages);
			memset(package,0,packages);
			in = timer.netTime();
		}

		Task::Task(UINT16 transfer_id, UINT16 channel_id, const void*pntr, UINT32 size):data_size(size),data(alloc<BYTE>(size)),
												completed(0),channel(channel_id),closed(false),undef(false),transfer(transfer_id)
		{
			packages = data_size/max_package_size;
			if ((data_size%max_package_size) || !packages)
				packages++;
			package = alloc<bool>(packages);
			out = alloc<Timer::Time>(packages);
			__int64 now = timer.netTime();
			for (UINT16 i = 0; i < packages; i++)
				out[i] = now;
			memset(package,0,packages);
			memcpy(data,pntr,data_size);
		}

		Task::~Task()
		{
			dealloc(data);
			dealloc(package);
			dealloc(out);
		}

		void DOMAIN_M Task::Insert(UINT16 id, const void*pntr, UINT16 size)
		{
			if (id >= packages)
			{
				sysmessage(NULL,"CONN: package id ("+IntToStr(id)+") exceeds package count ("+IntToStr(packages)+")");
				return;
			}
			if (id < packages-1 && size != max_package_size)
			{
				sysmessage(NULL,"CONN: package ("+IntToStr(id)+"/"+IntToStr(packages)+") is not of max package size but "+IntToStr(size));
				return;
			}
			UINT32 offset = max_package_size*id;
			if (offset+size > data_size)
			{
				sysmessage(NULL,"CONN: target region ("+IntToStr(offset)+", "+IntToStr(offset+size)+") exceeds data size ("+IntToStr(data_size)+")");
				return;
			}
			memcpy(&data[offset],pntr,size);
			completed+=!package[id];
			package[id] = true;
			in = timer.netTime();
		}

		void DOMAIN_M Task::confirm(UINT16 id)
		{
			if (id >= packages)
				return;
			completed+=!package[id];
			package[id] = true;
		}

		bool DOMAIN_M Task::finished()
		{
			return completed == packages;
		}


		TransferItem* DOMAIN_M Task::get()
		{
			if (!finished() || closed || undef)
				return NULL;
			TransferItem*result = SignalNew(new TransferItem());
			result->data = data;
			result->data_len = data_size;
			result->sender = NULL;
			result->exec = exec;
			result->channel = channel;
			sysmessage(NULL,"Task: creating transfer item for channel "+IntToStr(channel));
			data = NULL;
			closed = true;
			return result;
		}

		void DOMAIN_M Task::timeOutSend(Timer::Time now, BYTE flags, Connection*connection)
		{
			if (closed)
				return;
			for (UINT16 i = 0; i < packages; i++)
				if (!package[i] && timer.netToSecondsf(now-out[i]) > NET_SEND_TIMEOUT)
				{
					sysmessage(connection->parent,"Task: resending "+IntToStr(transfer)+" - "+IntToStr(i)+" ("+IntToStr(data_size)+" bytes)");
					USHORT size = max_package_size;
					UINT32 offset = max_package_size*i;
					if (offset+size > data_size)
						size = data_size-offset;

					out[i] = now;
					BYTE flg = flags|NET_DATA;
					UINT16 lo;
					if (!i)
					{
						flg |= NET_FIRST;
						lo = channel;
					}
					else
						lo = i;
					Package package(flg,lo,transfer,data_size);
					memcpy(package.data(),&data[offset],size);
					connection->parent->sendPackage(connection->ident,&package,size+sizeof(TPackageHead));
				}
		}

		void DOMAIN_M Task::redef(UINT32 size)
		{
			exec = NULL;
			if (size == data_size)
				return;
			data_size = size;
			re_alloc(data,data_size);
			packages = data_size/max_package_size;
			if (data_size%max_package_size || !packages)
				packages++;
			re_alloc(package,packages);
			memset(package,0,packages);
			in = timer.netTime();
			undef = false;
		}


		TaskList::TaskList():first(0)
		{}

		TransferItem* DOMAIN_M TaskList::getFinished(Node*sender, bool serial)
		{
			if (serial)
			{
				if (Task*element = peek())
					do
					{
						TransferItem*result = element->get();
						if (result || element->closed)
						{
							sysmessage(sender, "TaskList: Task closed (transfer "+IntToStr(element->transfer)+"). erasing...");
							Discard(get());
							first++;					 
						}
						else
						{
							sysmessage(sender, "TaskList: Task incomplete ("+IntToStr(element->transfer)+"). returning NULL");
							return NULL;
						}
						if (result)
						{
							sysmessage(sender, "TaskList: finished, returning");
							return result;
						}
					}
					while (element = peek());
		//		sysmessage(sender, "TaskList: empty, returning");
				return NULL;
			}
			for (UINT16 i = 0; i < elements(); i++)
			{
				Task*element = peek(i);
				TransferItem*result = element->get();
				if ((result || element->closed) && !i)
				{
					Discard(get());
					first++;
					i--;
				}
				if (result)
					return result;
			}
			return NULL;
		}

		void DOMAIN_M TaskList::timeOutClose(Node*sender, Timer::Time now, bool direct_erase_undef)
		{
			for (UINT16 i = 0; i < elements(); i++)
			{
				Task*task = peek(i);
				bool timeout = timer.netToSecondsf(now-task->in) > NET_RECEIVE_TIMEOUT;
				if ((!task->finished()||task->undef) && (task->closed || timeout || (task->undef && direct_erase_undef)))
				{
					if (!task->closed)
						sysmessage(sender, "TaskList: closing task "+IntToStr(task->transfer)+" (timeout)");
					if (!i)
					{
						sysmessage(sender, "TaskList: erasing task "+IntToStr(task->transfer)+" (timeout)");
						Discard(get());
						i--;
						first++;
					}
					else
						task->closed = timeout;
				}
			}
		}

		void DOMAIN_M TaskList::timeOutSend(Timer::Time now, BYTE flags, Connection*connection)
		{
			for (UINT16 i = 0; i < elements(); i++)
				peek(i)->timeOutSend(now,flags,connection);
		}


		Task* DOMAIN_A TaskList::append(UINT16 channel, const void*data, UINT32 size)
		{
			access.lock();
			Task*result = SignalNew(new Task(first+elements(),channel,data,size));
			Insert(result);
			access.release();
			return result;
		}

		Task* DOMAIN_M TaskList::locate(Node*sender, UINT16 transfer, UINT32 size)
		{
			sysmessage(sender,"TaskList: locating task "+IntToStr(transfer));
			UINT16 local = transfer-first,over = local-elements();
			if (local < elements())
			{
				sysmessage(sender,"TaskList: found task");
				Task*rs = peek(local);
				if (rs->closed)
					return NULL;
				if (rs->undef)
					rs->redef(size);
				return rs;
			}
			UINT16 el = elements();
			if (over > 0x2000 || over > 0xFFFF-el)
				return NULL;
			sysmessage(sender, "TaskList: appending task ("+IntToStr(over)+")");
			for (UINT16 i = 0; i < over; i++)
				Insert(SignalNew(new Task(first+el+i)));
			Task*rs = SignalNew(new Task(first+elements(),size));
			Insert(rs);
			return rs;
		}

		void DOMAIN_M TaskList::confirm(UINT16 transfer, UINT16 package)
		{
			UINT16 local = transfer-first;
			if (local >= elements())
				return;
			Task*task = peek(local);
			task->confirm(package);
			if (task->finished())
				task->closed = true;
			while (peek() && peek()->closed)
			{
				Discard(get());
				first++;
			}
		}




		void DOMAIN_M Node::confirm(const NetIdent&target,UINT16 transfer_id,BYTE package_id, bool serial)
		{
			UINT16 flags = NET_CONFIRM;
			if (serial)
				flags |= NET_SERIAL;
			Package package(flags,package_id,transfer_id);
			sendPackage(target,&package,sizeof(TPackageHead)); //and off you go
		}

		void  Node::sendMessage(const NetIdent&target,UINT16 message_id)
		{
			TPackageHead head;
			head.config = 0;
			head.lo = 0;
			head.hi = message_id;
		
		/*	Package package(0,0,message_id);
			sendPackage(target,&package,sizeof(TPackageHead));*/
		
			sendPackage(target,&head);
		}

		void Node::sendPackage(const NetIdent&target, Package*package, UINT32 size)
		{
			sendto(node,(char*)package->field,size,0,(sockaddr*)&target,sizeof(target)); //then push the package out

			#ifdef NET_ALLOW_LOW_EVENT_CATCH  //if this is set OnWrite and OnRead events will be called
				if (OnWrite)
					event(OnWrite,target,size);
			#endif
		}

		void Node::sendPackage(const NetIdent&target, TPackageHead*package)
		{
			sendto(node,(char*)package,sizeof(TPackageHead),0,(sockaddr*)&target,sizeof(target)); //then push the package out

			#ifdef NET_ALLOW_LOW_EVENT_CATCH  //if this is set OnWrite and OnRead events will be called
				if (OnWrite)
					event(OnWrite,target,sizeof(TPackageHead));
			#endif
		}


		void DOMAIN_M Node::checkTimeouts()
		{}

		void DOMAIN_M Node::event(_netClientEvent exec, const Name&target, UINT32 para)
		{
			if (_direct)
				exec(target,para);
			else
				_queue.Insert(SignalNew(new ClientEventItem(exec,target,para)));
		}

		void DOMAIN_M Node::event(_netClientDrop exec, Connection*connection, UINT32 para)
		{
			if (_direct)
			{
				exec(connection,para);
			}
			else
			{
				_queue.Insert(SignalNew(new ClientDropItem(exec,connection,para)));
			}
		}

		void DOMAIN_M Node::event(_netEvent exec, Connection*connection)
		{
			if (_direct)
				exec(connection);
			else
				_queue.Insert(SignalNew(new EventItem(exec,connection)));
		}


		void DOMAIN_M Node::event(QueueItem*item)
		{
			if (_direct)
				Discard(item)
			else
				_queue.Insert(item);
		}

		void DOMAIN_M Node::trash(Connection*connection)
		{
			if (_direct)
				Discard(connection)
			else
				_queue.Insert(connection);
		}




		Node::Node():   _online(false),_defined(false),_recovering(false),read_buffer(0,0,0,0),core_thread(this,false),
								_direct(false),port(NET_DEFAULT_PORT),node(0),
								OnRead(NULL),OnWrite(NULL),OnPing(NULL),OnTick(NULL)
		{
			time_thread.create(this,(pThreadMethod)&Net::Node::netTimeThread,false);
			initNet();
		}


		Node::~Node()
		{
			disconnect();  //unbind
			if (_defined)		//and
				closesocket(node); //delete the node if defined
		}

		void DOMAIN_A Node::makeAutonomous()
		{
			if (_direct)
				return;
			resolve();
			_direct = true;
		}

		void DOMAIN_A Node::makeResolvable()
		{
			_direct = false;
		}

		void DOMAIN_A Node::openChannel(UINT16 channel, _netReceive exec)
		{
			TChannel c = {0,exec};
			synchronized(channel_mutex)
			{
				channels.Set(channel,c);
			}
		}

		void DOMAIN_A Node::openChannel(UINT16 channel, _netReceive exec, UINT32 package_size)
		{
			TChannel c = {package_size,exec};
			channels.Set(channel,c);
		}

		void DOMAIN_A Node::closeChannel(UINT16 channel)
		{
			synchronized(channel_mutex)
			{
				channels.unSet(channel);
			}
		}
	
		bool DOMAIN_A Node::channelIsOpen(UINT16 channel)
		{
			bool is_open;
			synchronized(channel_mutex)
			{
				is_open = channels.IsSet(channel);
			}
			return is_open;
		}


		bool DOMAIN_A Node::bindService()
		{
	#if 0
			error = "";
			if (_online) //already active?
				return true;
			if (_defined) //whups~ socket already defined?
				closesocket(node); //then erase it
			node = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP); //and create a new udp datagram socket
			_defined = true; //now it's defined
			if (node == INVALID_SOCKET) //or maybe not?
			{
				error = "socket() call failed";
				return false; //then return a failed-signal
			}
			char optval = 1; //I don't really know if this is necessary. It's been in the original code. I'll leave it for the time being.
			if (setsockopt(node, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) || optval!=1)
			{
				/*error = "setsockopt() call failed";
				closesocket(node);
				return false;*/
			}
			memset(&self,0,sizeof(self)); //redefine the self.
			self.sin_family			= AF_INET;				 //internet-address (well LAN,WEB, who cares?)
			self.sin_addr.s_addr	= htonl(INADDR_ANY);	   //will be zero in the end. And must be zero when calling bind.
			self.sin_port		   = htons(port);
			if (bind(node,(sockaddr*)&self,sizeof(self))) //bind the self to the new socket.
			{
				error = "bind() call failed";
				//or not
				closesocket(node);
				return false; //then leave it.
			}
			self.sin_addr.s_addr = 0x0100007F; // 127.0.0.1 (localhost)
			_online = true;  //up and running
			core_thread.start();
			time_thread.start();
			return true;
	#endif
			return false;
		}

		bool DOMAIN_M Node::resetConnection()
		{
	#if 0
			if (_online)  //but only if online
				shutdown(node,0); //shutdown node (I didn't find the macros for SD_...)
			_online = false;
			if (_defined) //whups~ socket already defined?
				closesocket(node);
			node = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP); //and create a new socket
			_defined = true; //now it's defined
			if (node == INVALID_SOCKET) //or maybe not?
				return false; //then return a failed-signal
			char optval = 1; //I don't really know if this is necessary. It's been in the original code. I'll leave it for the time being.
			if (setsockopt(node, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) || optval!=1)
			{
				closesocket(node);
				return false;
			}
			memset(&self,0,sizeof(self)); //redefine the self.
			self.sin_family		= AF_INET;				 //internet-address (well LAN,WEB, who cares?)
			self.sin_addr.s_addr	= htonl(INADDR_ANY);	   //will be zero in the end. And must be zero when calling bind.
			self.sin_port			= htons(port);
			if (bind(node,(sockaddr*)&self,sizeof(self))) //bind the self to the new socket.
			{
				closesocket(node);
				return false; //then leave it.
			}
			self.sin_addr.s_addr = 0x100007F; // 127.0.0.1 (localhost)
			_online = true;  //up and running
			return true;
	#endif
			return false;
		}


		void DOMAIN_A Node::disconnect()
		{
			if (_online)  //but only if online
			{
				time_thread.terminate();
				core_thread.terminate();
				shutdown(node,0); //shutdown node (I didn't find the macros for SD_...)
			}
			_online = false; //not online anymore
		}

		void DOMAIN_M Node::disconnectM()
		{
			if (_online)  //but only if online
			{
				_online = false;
				disconnect();
				shutdown(node,0); //shutdown node (I didn't find the macros for SD_...)
				time_thread.terminate();
			}
		}

		bool DOMAIN_A Node::isOnline()
		{
			return online();
		}

		bool DOMAIN_A Node::online()
		{
			/*if (_online != core_thread.isActive() || _online != time_thread.isActive())
			{
				if (_online)
				{
					if (!core_thread.isActive())
						FATAL__("online but core thread is not active");
					FATAL__("online but time thread is not active");
				}
				if (core_thread.isActive())
					FATAL__("offline but core thread is active");
				FATAL__("offline but time thread is active");
				
			}*/
			return _online&&core_thread.isActive();
		}

		bool DOMAIN_M Node::read()
		{
			Name origin;  //used to store the address the package is from
			#if SYSTEM==WINDOWS
				int size = sizeof(origin); //size of Name
			#else
				socklen_t size = sizeof(origin);
			#endif
			int received = recvfrom(node,(char*)read_buffer.field,sizeof(read_buffer.field),0,(sockaddr*)&origin,&size); //let's get the data
			if (received < 0)
			{
				if (!_online)
					return false;
				time_thread.terminate();
				if (!_recovering)
					_recovering = resetConnection();
				else
					_recovering = false;
				if (!_recovering)
				{
					fetchDisconnect(origin,NET_RECOVERY_FAILED,true);
			 //	   ShowMessage(err2str(WSAGetLastError()));

					_online = false;
					shutdown(node,0); //shutdown node (I didn't find the macros for SD_...)
					return false;
				}
				else
					fetchDisconnect(origin,NET_CONNECTION_DIED,false);
				time_thread.start();
			}
		//	if (_recovering)
				_recovering = false;
			

			if (origin == self) //if data is from localhost
			{
				#ifdef NET_ALLOW_LOW_EVENT_CATCH  //if this is set OnWrite and OnRead events will be called
				if (OnTick)
					event(OnTick,origin,0);
				#endif

				checkTimeouts(); //Interpret as tick
				return true; //return because no other data is present (localhost only sends ticks)
			}
			if ((size_t)received < sizeof(TPackageHead))
				return _online;
			#ifdef NET_ALLOW_LOW_EVENT_CATCH
				if (OnRead)
					event(OnRead,origin,received);
			#endif
			if (!channel(origin))
				return _online;
			read(&read_buffer,received);
			return _online;
		}


		bool DOMAIN_M Node::channel(const Name&) //channel incoming package to client or whatever
		{
			return false;  //not channeled. If this function is called, package will be ignored.
		}

		void DOMAIN_M Node::read(Package*, UINT32) //NetItem does not handle packages...
		{}

		void DOMAIN_M Node::fetchDisconnect(Connection*, UINT16,bool)//...or whatever
		{}

		void DOMAIN_M Node::fetchDisconnect(const Name&, UINT16, bool)
		{}

		void DOMAIN_A Node::resolve()
		{
			_queue.clear();
			if (!core_thread.isActive() && time_thread.isActive())
				time_thread.terminate();
		}

		const String& DOMAIN_A	Node::GetError()
		{
			return error;
		}









		Connection::Connection(const Name&n, Node*node):last_recv(timer.netTime()),parent(node),
																		non_confirm_serial(0),non_confirm_id(0),synchronized_(false),ident(n)
		{}

		Connection::Connection(Node*node):parent(node),non_confirm_serial(0),non_confirm_id(0),synchronized_(false)
		{}

		Node*	Connection::getParent()
		{
			return parent;
		}
	
		int Connection::getDelta(const Timer::Time&now, bool external_access)
		{
			if (!synchronized_)
				return 0;
			if (external_access)
				sync.mutex.lock();

				double seconds_passed = timer.netToSecondsd(now - sync.last_sync);
				if (seconds_passed > NET_SYNC_ADOPT_INTERVAL)
					seconds_passed = NET_SYNC_ADOPT_INTERVAL;
				int rs = sync.last_delta+(int)(sync.delta_change*seconds_passed);
			
			if (external_access)
				sync.mutex.release();
			
			return rs;
		}



		UINT32	Connection::getTimeStamp()
		{
			UINT32	base = (UINT32)timer.netTime();
			return base + getDelta(base,true);
		}

		void DOMAIN_M   Connection::sendSyncRequest(const Timer::Time&now)
		{
			if (sync.sent >= NET_SYNC_STEPS)
				return;
			sysmessage(parent,"sending sync request("+IntToStr(sync.sent)+")");
			TPackageHead request;
			sync.last_action = sync.time[sync.sent+1] = sync.time[sync.sent] = now;
			request.custom[0] = sync.sent++;
			request.custom[1] = 0;
			request.custom[2] = 0;
			request.flags = NET_FLOOD;
			request.lo = 0;
			request.hi = NET_SYNC_REQUEST;
			parent->sendPackage(ident,&request);
			request.custom[0] = sync.sent++;
			parent->sendPackage(ident,&request);
		}


		void DOMAIN_M   Connection::initSync(const Timer::Time&now)
		{
			sysmessage(parent,"initiating sync");
			sync.busy = true;
			sync.sent = 0;
			sync.received = 0;
			sync.delta_sum = 0;
			sendSyncRequest(now);
		}

		void DOMAIN_M   Connection::checkSync(const Timer::Time&now)
		{
			if (!synchronized_ && !sync.busy)
			{
				initSync(now);
				return;
			}
			float seconds_passed = timer.netToSecondsf(now - sync.last_action);
			if (sync.busy)
			{
				if (seconds_passed > NET_SYNC_STEP)
					sendSyncRequest(now);
			}
			else
				if (seconds_passed > NET_SYNC_INTERVAL)
					initSync(now);
		}

		void DOMAIN_M   Connection::closeSync()
		{
			sync.mutex.lock();
				Timer::Time	now = timer.netTime();
				int target_delta = sync.delta_sum / sync.received;
		//		if (synchronized_)
				{
					sync.last_delta = getDelta(now,false);
					sync.delta_change = (double)(target_delta-sync.last_delta)/NET_SYNC_ADOPT_INTERVAL;
				}
		/*		else
				{
					sync.last_delta = target_delta;
					sync.delta_change = 0;
				}*/
				sync.last_sync = now;
				sysmessage(parent,"closing sync - delta is now "+IntToStr(sync.last_delta)+" change is "+FloatToStr(sync.delta_change));
				sync.busy = false;
				sync.last_action = now;
				synchronized_ = true;
			sync.mutex.release();
		}

		void DOMAIN_M Connection::checkTimeouts(Timer::Time now)
		{
			if (timer.netToSecondsf(now-last_recv) > NET_CONNECTION_TIMEOUT) //no ping?
			{
				parent->fetchDisconnect(this,NET_TIMEOUT,false);//disconnect myself
				return;
			}
			if (timer.netToSecondsf(now-last_ping) > NET_PING_INTERVAL)
			{
				parent->sendMessage(ident,NET_TICK);
				last_ping = now;
			}
			out_serial.timeOutSend(now,NET_SERIAL|NET_CONFIRM,this); //only resend confirm-transfers
			out_parallel.timeOutSend(now,NET_CONFIRM,this);
			in[NET_FLOOD].timeOutClose(parent,now,false); //only erase non-confirm-transfers
			in[NET_SERIAL].timeOutClose(parent,now,true); //same here
			while (TransferItem*item = in[NET_SERIAL].getFinished(parent,true))
			{
				item->sender = this;
				//item->flags = NET_SERIAL;
				sysmessage(parent,"CONN: finishing transfer");
				parent->event(item);
			}


		}

		void DOMAIN_A Connection::message(UINT16 channel, BYTE flags)
		{
			write(channel,flags,NULL,0);
		}

		void DOMAIN_A Connection::sendString(UINT16 channel, const String&string)
		{
			write(channel,NET_SERIAL|NET_CONFIRM,string.c_str(),UINT32(string.GetLength()));
		}

		void DOMAIN_A Connection::write(UINT16 channel, const void*data, UINT32 len)
		{
			write(channel,NET_SERIAL|NET_CONFIRM,data,len);
		}
	
		void DOMAIN_A Connection::write(UINT16 channel, BYTE flags, const void*data, UINT32 len)
		{
			if (len > max_package_size*0x1000)
				return;
			
			sysmessage(parent,"CONN: writing data ("+IntToStr(channel)+") / flags: "+IntToStr(flags)+" len: "+IntToStr(len));

			flags|=NET_DATA;


			const BYTE*pntr = (const BYTE*)data;
			UINT32 transfer_id;
			if (flags&NET_CONFIRM)
			{
				Task*out_task;
				if (flags&NET_SERIAL)
					out_task = out_serial.append(channel,data,len);
				else
					out_task = out_parallel.append(channel,data,len);
				transfer_id = out_task->transfer;
				sysmessage(parent,"CONN: added task ("+IntToStr(out_task->transfer)+")");
			}
			else
				if (flags&NET_SERIAL)
					transfer_id = non_confirm_serial++;
				else
					transfer_id = non_confirm_id++;
			UINT16 packages = len/max_package_size,
				   rest = len%max_package_size;
			if (rest||!packages)
				packages++;
			else
				rest = max_package_size;
			sysmessage(parent,"CONN: transfer needs "+IntToStr(packages)+" packages");
			for (UINT16 i = 0; i < packages-1; i++)
			{
				BYTE flg = flags;
				UINT16  lo;
				if (!i)
				{
					flg |= NET_FIRST;
					lo = channel;
				}
				else
					lo = i;
				
				Package package(flg,lo,transfer_id,len);
				memcpy(package.data(),pntr,max_package_size);
				parent->sendPackage(ident,&package,const_buffer_size);
				pntr+=max_package_size;
			}
			sysmessage(parent,"CONN: sending last package ("+IntToStr(rest)+" bytes)");
			Package package(packages>1?flags:flags|NET_FIRST,packages>1?packages-1:channel,transfer_id,len);
			memcpy(package.data(),pntr,rest);
			parent->sendPackage(ident,&package,rest+sizeof(TPackageHead));
		}

		void DOMAIN_A Connection::reset(const Name&target)
		{
			synchronized_ = false;
			out_serial.clear();
			out_serial.first = 0;
			out_serial.access.release();
			out_parallel.clear();
			out_parallel.first = 0;
			out_parallel.access.release();
			in[0].clear();
			in[0].first = 0;
			in[1].clear();
			in[1].first = 0;
			in[2].clear();
			in[2].first = 0;
			in[3].clear();
			in[3].first = 0;
		
		
			ident = target;
			last_recv = timer.netTime();
		}

		void DOMAIN_M Connection::handlePackage(Package*package, UINT32 package_size)
		{
			last_recv = timer.netTime();
			if (package->head.flags&NET_DATA)
			{
				UINT16 package_id=package->head.lo;
				if (package->head.flags&NET_FIRST)
					package_id = 0;

				UINT32 transfer_size;
				sysmessage(parent,"CONN: incoming package (flags: "+IntToStr(package->head.flags)+") "+IntToStr(package->head.hi)+" "+IntToStr(package_id)+" size: "+IntToStr(package_size));
				if (package->head.flags&NET_CONFIRM)
					parent->confirm(ident,package->head.hi,package_id,!!(package->head.flags&NET_SERIAL));

				transfer_size = package->getTransferSize();
				BYTE bucket = (package->head.flags)&0x3;
				TChannel receive;
				receive.exec = NULL;
				if (package->head.flags&NET_FIRST)
				{
					synchronized(parent->channel_mutex)
					{
						if (!parent->channels.Query(package->head.lo,receive))
						{
							sysmessage(parent,"CONN: channel ("+IntToStr(package->head.lo)+") not found");
							return;
						}
					}
					if (receive.package_size && transfer_size != receive.package_size)
					{
						sysmessage(parent,"CONN: channel "+IntToStr(package->head.lo)+ " enforces package size "+IntToStr(receive.package_size)+" but package is of size "+IntToStr(transfer_size));
						return;
					}
					sysmessage(parent,"CONN: forwarding package to channel "+IntToStr(package->head.lo));
				}
				Task*task = in[bucket].locate(parent,package->head.hi,transfer_size);
				if (task)
				{
					if (package->head.flags&NET_FIRST)
						task->channel = package->head.lo;
					if (receive.exec)
						task->exec = receive.exec;
					sysmessage(parent,"CONN: task located (bucket "+IntToStr(bucket)+")");
					task->Insert(package_id,package->data(),UINT16(package_size-sizeof(TPackageHead)));
				}
				else
					sysmessage(parent,"CONN: could not find task for transfer "+IntToStr(package->head.hi)+" (bucket "+IntToStr(bucket)+")");
				if (!(bucket & NET_CONFIRM))
					in[bucket].timeOutClose(parent,timer.netTime(),!!(bucket & NET_SERIAL));
				while (TransferItem*item = in[bucket].getFinished(parent,!!(bucket&NET_SERIAL)))
				{
					item->sender = this;
					//item->flags = bucket;
					sysmessage(parent,"CONN: finishing transfer");
					parent->event(item);
				}
			}
			else
			{
				sysmessage(parent,"CONN: incoming message");
				if (package->head.flags & NET_CONFIRM)
				{
					sysmessage(parent,"CONN: confirmation for "+IntToStr(package->head.hi)+" "+IntToStr(package->head.lo));
					if (package->head.flags & NET_SERIAL)
						out_serial.confirm(package->head.hi,package->head.lo);
					else
						out_parallel.confirm(package->head.hi,package->head.lo);
					return;
				}
				switch(package->head.hi)
				{
					case NET_CL_REQUEST_CONNECT:
						sysmessage(parent,"CONN: responding to connect-request");
						parent->sendMessage(ident,NET_SV_CONFIRM_CONNECT); //tell the client that the connection has allready been accepted
					break;
					case NET_SV_DENY_CONNECT:
					case NET_DISCONNECT:
					case NET_SV_FORCE_DISCONNECT:
					case NET_SV_SERVER_SHUTDOWN:
						sysmessage(parent,"CONN: ShutdownMessage: hanging up");
						parent->fetchDisconnect(this,package->head.hi,false);
					break;
					case NET_SYNC_REQUEST:
					{
						sysmessage(parent,"CONN: responding to sync request");
						TPackageHead reply;
						Timer::Time t = timer.netTime();
					
						reply.custom[0] = package->head.custom[0];
						reply.custom[1] = t;
						reply.custom[2] = (t>>8);
						reply.lo = (t>>16);
						reply.hi = NET_SYNC_REPLY;
						reply.flags = NET_FLOOD;

						parent->sendPackage(ident,&reply);
					}
					break;
					case NET_SYNC_REPLY:
					{
						BYTE			sync_id = package->head.custom[0];
						if (sync_id < NET_SYNC_STEPS && sync.busy)
						{
							Timer::Time	t = timer.netTime(),
											delay = t-sync.time[sync_id];
							int			 rmt_time = ((int)package->head.custom[1]|((int)package->head.custom[2]<<8)|((int)package->head.lo<<16))+(int)(NET_DELAY_BALANCE*delay),
											lcl_time = (int)t,
											delta = (rmt_time-lcl_time);
							sync.delta_sum += delta;
							sync.received++;
							sysmessage(parent,"CONN: got sync response ("+IntToStr(sync.received)+"/"+IntToStr(NET_SYNC_STEPS_NEEDED)+") after "+IntToStr(delay)+" tick(s)");
							if (sync.received > NET_SYNC_STEPS_NEEDED)
								closeSync();
						
							ping = (UINT32)delta/2;
							#ifdef NET_ALLOW_LOW_EVENT_CATCH
								if (parent->OnSync)
									parent->event(parent->OnSync,ident,ping);
							#endif
						}
					}
					break;

				}
			}
		}
	
		UINT32		DOMAIN_A Server::getTimeStamp()
		{
			return (UINT32)timer.netTime();
		}

		void DOMAIN_M Server::fetchDisconnect(const Name&name, UINT16 reason, bool closing)
		{
			if (closing)
			{
				if (OnServerCrash)
					event(OnServerCrash,name,NET_RECOVERY_FAILED);
				return;
			}
			list_access.lock();
			index_t index = lookupIndex(name);
			if (!index)
				return;
			index--;
			if (OnClientDisconnect) //call event
				event(OnClientDisconnect,GetChar(index),reason);
			trash(drop(index));
			list_access.release();
		}


		void DOMAIN_M Server::fetchDisconnect(Connection*connection, UINT16 reason, bool closing)
		{
			if (closing)
			{
				if (OnServerCrash)
					event(OnServerCrash,connection->ident,NET_RECOVERY_FAILED);
				return;
			}
			list_access.lock();
			index_t index = getIndexOf(connection);
			if (!index)
				return;
			index--;
			if (OnClientDisconnect) //call event
				event(OnClientDisconnect,connection,reason);
			trash(drop(index));
			list_access.release();
		}

		bool DOMAIN_M Server::channel(const Name&name)
		{
			incoming_name = name;
			list_access.lock();
			incoming_target = lookup(name);
			list_access.release();
			return true;
		}

		void DOMAIN_M Server::read(Package*package, UINT32 size)
		{
			if (incoming_target)
				incoming_target->handlePackage(package,size);
			else
			{
				if (package->head.lo || package->head.hi != NET_CL_REQUEST_CONNECT)
					return;
				sysmessage(this,"connect_request");
				if (!AcceptClient(incoming_name))
				{
					Node::sendMessage(incoming_name,NET_SV_DENY_CONNECT);
					return;
				}

				incoming_target = SignalNew(new Connection(incoming_name,this));
				list_access.lock();
					Insert(incoming_target);
				list_access.release();
				Node::sendMessage(incoming_name,NET_SV_CONFIRM_CONNECT); //tell the client that the connection was accepted
		//		ShowMessage("got new client!");
				if (OnClientConnect) //and call the event
					event(OnClientConnect,incoming_target);
			}
		}

		void DOMAIN_M Server::checkTimeouts()
		{

			__int64 now = timer.netTime(); //only get time once
		/*	bool do_ping = (timer.toSecondsf(now-_last_ping) > NET_PING_INTERVAL);
			if (do_ping) //if ping
			{
				reset();
				while (Connection*connection = each())
					Node::sendMessage(connection->ident,NET_PING);
				_last_ping = now; //and remember that we pinged
			}*/
			reset();
			while (Connection*connection = each())
				connection->checkTimeouts(now);
		}







		Server::Server():OnClientConnect(NULL),OnServerCrash(NULL),OnClientDisconnect(NULL),AcceptClient(netAccept)
		{}

		bool DOMAIN_A Server::startService()
		{
			return bindService();
		}

		void DOMAIN_A Server::disconnect()
		{
			list_access.lock();
				reset();
				while (Connection*connection = each())
					Node::sendMessage(connection->ident,NET_SV_SERVER_SHUTDOWN);
				clear();
			list_access.release();
			Node::disconnect();
		}

		count_t DOMAIN_A Server::clients()
		{
			return Count();
		}

		void			Server::lockClients()
		{
			list_access.lock();
		}

		Connection* Server::getDirect(UINT32 index)
		{
			return GetChar(index);
		}

		void			Server::releaseClients()
		{
			list_access.release();
		}


		Connection* DOMAIN_A Server::getClient(UINT32 index)
		{
			list_access.lock();
			Connection*connection = GetChar(index);
			list_access.release();
			return connection;
		}

		Connection* DOMAIN_A Server::findClient(const Name&name)
		{
			list_access.lock();
			Connection*connection = lookup(name);
			list_access.release();
			return connection;
		}

		void DOMAIN_A Server::message(UINT16 topic, BYTE flags)//empty package
		{
			list_access.lock();
			for (UINT32 i = 0; i < Count(); i++)
			{
				Connection*connection = GetChar(i);
				if (connection) //connection might be dropped this second
					connection->write(topic,flags,NULL,0);
			}
			list_access.release();
		}
		void DOMAIN_A Server::sendString(UINT16 channel, const String&string)
		{
			write(channel,NET_SERIAL|NET_CONFIRM,string.c_str(),UINT32(string.GetLength()));
		}

		void DOMAIN_A Server::write(UINT16 channel, const void*data, UINT32 len)
		{
			write(channel,NET_SERIAL|NET_CONFIRM,data,len);
		}
	
		void DOMAIN_A Server::write(UINT16 topic, BYTE flags, const void*data, UINT32 size)
		{
			list_access.lock();
			for (UINT32 i = 0; i < Count(); i++)
			{
				Connection*connection = GetChar(i);
				if (connection) //connection might be dropped this second
					connection->write(topic,flags,data,size);
			}
			list_access.release();
		}

		bool DOMAIN_A Server::disconnectClient(Connection*client)
		{
			list_access.lock();
			index_t index = getIndexOf(client);
			if (!index)
				return false;
			index--;
			Node::sendMessage(client->ident,NET_SV_FORCE_DISCONNECT);
			Erase(index);
			list_access.release();
			return true;
		}

		void Client::fetchDisconnect(const Name&name, UINT16 reason, bool closing)
		{
			if (reason == NET_CONNECTION_DIED)
				return;
			if (OnDisconnect)
				event(OnDisconnect,this,reason);
			if (!closing)
				disconnectM();
		}

		void Client::fetchDisconnect(Connection*connection, UINT16 reason, bool closing)
		{
			if (reason == NET_CONNECTION_DIED)
				return;
			if (OnDisconnect)
				event(OnDisconnect,this,reason);
			if (!closing)
				disconnectM();
		}

		bool Client::channel(const Name&target)
		{
			return target==ident;
		}

		void Client::read(Package*package, UINT32 package_size)
		{
			if (requesting && !package->head.lo && package->head.hi == NET_SV_CONFIRM_CONNECT)
			{
				requesting = false;
				if (OnConnect)
					event(OnConnect,this);
			}
			else
			{
				sysmessage(this,"handling package");
				handlePackage(package,package_size);
			}
		}

		void Client::checkTimeouts()
		{
			Timer::Time now = timer.netTime();
			checkSync(now);
			Connection::checkTimeouts(now);
			if (requesting) //with one difference. We have to check what happend to our request.
				if (timer.netToSecondsf(now-requested)>NET_REQUEST_TIMEOUT)
				{
					if (OnConnectFail)
						event(OnConnectFail,this,NET_TIMEOUT);
					disconnectM(); //if connect-attempt timed out, then abort.
				}
				else
				{
					sysmessage(this,"rerequesting");
					Node::sendMessage(ident,NET_CL_REQUEST_CONNECT); //otherwise resend the request.
				}
		}

		Client::Client():Connection(this),OnConnect(NULL),OnDisconnect(NULL),OnConnectFail(NULL)
		{
			//port = 0;	//dynamic port - doesn't work
		}


		bool Client::connectTo(const char*addr, UINT16 port)
		{
	#if 0
			if (!port)
				port = this->port;
			sockaddr_in target_addr;
			memset(&target_addr,0,sizeof(target_addr)); //clear the address
			target_addr.sin_port = htons(port); //set the port
			target_addr.sin_family		= AF_INET;//internet - what else?
			hostent*server;
			if (isalpha(addr[0]))
				server = gethostbyname(addr); //next we look up the hostname - this is actually a bit laggy
			else
			{
				UINT32 ip = inet_addr(addr);
				if (ip == INADDR_NONE)
				{
					Node::error = "unable to resolve address '"+String(addr)+"'";
					return false;
				}
				server = gethostbyaddr((const char*)&ip,sizeof(ip),AF_INET);
			}
			#if SYSTEM==WINDOWS
				if (!server || WSAGetLastError()-WSABASEERR>0) //if we got an error we just abort. Usually it's a "host not found"-error
				{
					Node::error = "unable to resolve address '"+String(addr)+"'";
					return false;
				}
			#else
				if (!server)
				{
					Node::error = "unable to resolve address '"+String(addr)+"'";
					return false;
				}
			#endif
			target_addr.sin_addr.s_addr = 0;											  //reset all 4 bytes, so we don't get funny things later because...
			memcpy(&target_addr.sin_addr.s_addr,server->h_addr_list[0],server->h_length); //I don't know why, but addresses returned by getHostByName are not necessarily 4-byte addresses. They can be of lesser length, so we use memcpy, to transfer the address
		//	ShowMessage(String(addr)+" resolved to "+Name(target_addr).ToString());
			return connectTo(target_addr);												//and connect to the target.
	#endif
			return false;
		}

		bool Client::connectTo(const Name&target)
		{
			disconnect(); //first disconnect any old connection
			reset(target);
			port++;
			if (!Node::bindService()) //then rebind the socket
				return false;
			Node::sendMessage(ident,NET_CL_REQUEST_CONNECT);//and send a message to the new server
			requesting = true; //requesting means the client is waiting for a valid answer from the server
			requested = timer.netTime();//to check at what time the connection failed
			return true; //at least the request is out. For the server to react it will take some time
		}

		void Client::disconnect()
		{
			if (_online)
				Node::sendMessage(ident,NET_DISCONNECT); //send a message that we are going offline. Even if the message doesn't reach its destination it won't matter.
			Node::disconnect(); //unbind the service

		}
		#endif /*0*/

	}

}
