#ifndef tcpH
#define tcpH

#include "node.h"
#include "../container/vectorList.h"
#include "../io/byte_stream.h"
#include "../container/production_pipe.h"

#include <thread>
#include <sstream>
#include <mutex>

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

#undef StartService

namespace DeltaWorks
{
	/**
	@brief TCP/IP related constructs
	*/
	namespace TCP
	{
		#if SYSTEM==WINDOWS
			typedef int	socklen_t;
		#endif


		#undef TCP_TRY
		#undef TCP_CATCH

		#define TCP_TRY  try
		#define TCP_CATCH\
					catch (const std::exception&ex)\
					{\
						::DeltaWorks::TCP::onIgnoredException(CLOCATION,ex);\
					}\
					catch (...)\
					{\
						::DeltaWorks::TCP::onIgnoredArbitraryException(CLOCATION);\
					}



		extern std::function<void(const TCodeLocation&,const std::exception&)>	onIgnoredException;
		extern std::function<void(const TCodeLocation&)>	onIgnoredArbitraryException;


		void	SwapCloseSocket(volatile SOCKET&socket);
		String	ToString(const addrinfo&address);
		String	ToString(const sockaddr_storage&address, socklen_t addrLen, bool includePort = true);
		String	StripPort(const String&host);


		class Dispatchable
		{
		public:
			virtual ~Dispatchable(){}
		};
	
		typedef std::shared_ptr<Dispatchable>	PDispatchable;


		/**
		Atomic storage for time stamps
		*/
		class AtomicTimeStamp
		{
		public:
			/**/				AtomicTimeStamp():time(Time::Stamp().ToTicks())
			{}

			/**
			Casts the locally stored time to a stamp
			*/
			Time::Stamp			Cast() const {return Time::Stamp::FromTicks(time);}
			/**
			Updates the internally stored time to now
			*/
			void				Update() {time = Time::Stamp().ToTicks();}

		private:
			std::atomic<UINT64>	time;
		};

		/**
		Special mutex for events that allows to completely kick out threads before they enter a critical section
		*/
		class EventLock
		{
			struct TimedLock
			{
				std::recursive_timed_mutex	mutex;
				TCodeLocation		lastLockedBy[0x10];
				index_t				lockDepth=0;

				bool			TryLock(const TCodeLocation&locker)
				{
					#ifdef _DEBUG
						static const size_t toleranceMS = 30000;
					#else
						static const size_t toleranceMS = 5000;
					#endif
					if (!mutex.try_lock_for(std::chrono::milliseconds(toleranceMS)))
					{
						if (lockDepth == 0)
						{
							FATAL__(String(locker)+": Failed to lock thread after "+String(toleranceMS)+"ms. Deadlock assumed. No lockers. This should not happen. Like, ever!");
						}
						else
							if (lockDepth <= ARRAYSIZE(lastLockedBy)) 
							{
								FATAL__(String(locker)+": Failed to lock thread after "+String(toleranceMS)+"ms. Deadlock assumed. Last locker was "+String(lastLockedBy[lockDepth-1]));
							}
							else
								FATAL__(String(locker)+": Failed to lock thread after "+String(toleranceMS)+"ms. Deadlock assumed. Too many lockers");
						return false;
					}
					if (lockDepth < ARRAYSIZE(lastLockedBy))
						lastLockedBy[lockDepth] = locker;
					lockDepth++;
					return true;
				}

				void			Unlock()
				{
					lockDepth--;
					mutex.unlock();
				}

			};


			TimedLock			innerLock;//,permissiveLock;
			volatile count_t	blockLevel;//,permissiveCount;
		



		public:
			/**/		EventLock():blockLevel(0)	{}


			bool		IsBlocked() const {return blockLevel > 0;}

			void		Block(const TCodeLocation&locker)
			{
				if (!innerLock.TryLock(locker))
					return;
				blockLevel++;
				innerLock.Unlock();
			}

			bool		Unblock(const TCodeLocation&locker)
			{
				if (!innerLock.TryLock(locker))
					return false;
				ASSERT__(blockLevel > 0);
				blockLevel--;
				bool rs = blockLevel == 0;
				innerLock.Unlock();
				return rs;
			}
	/*
			bool		PermissiveLock(const TCodeLocation&locker)
			{
				if (!permissiveLock.TryLock(locker))
					return false;
				if (permissiveCount == 0)
				{
					if (!innerLock.TryLock(locker))
					{
						permissiveLock.Unlock();
						return false;
					}
					if (blockLevel != 0)
					{
						innerLock.Unlock();
						permissiveLock.Unlock();
						return false;
					}
				}
				permissiveCount++;
				permissiveLock.Unlock();
				return true;
			}

			void		PermissiveUnlock(const TCodeLocation&locker)
			{
				if (!permissiveLock.TryLock(locker))
					return;
				ASSERT__(permissiveCount > 0);
				permissiveCount--;
				if (permissiveCount == 0)
				{
					innerLock.Unlock();
				}
				permissiveLock.Unlock();
			}*/

			bool		ExclusiveLock(const TCodeLocation&loc)
			{
				if (!innerLock.TryLock(loc))
					return false;
				if (blockLevel != 0)
				{
					innerLock.Unlock();
					return false;
				}
				return true;
			}


			void		ExclusiveUnlock()
			{
				innerLock.Unlock();
			}


		};


		class SocketAccess
		{
		public:
			virtual					~SocketAccess()	{};
			virtual void			SetSocket(SOCKET socket) = 0;
			virtual void			CloseSocket() = 0;
			virtual bool			IsClosed() const = 0;
			virtual SocketAccess*	CloneClass() const = 0;

			virtual	int				Read(void*data, size_t dataSize) = 0;
			virtual int				Write(const void*data, size_t dataSize) = 0;
		};


		class Exception : public std::exception
		{
		public:
			String message;
			TCodeLocation loc;


			Exception(const TCodeLocation&loc):loc(loc)	{}
			Exception(const TCodeLocation&loc,const String&message):loc(loc),message(message)	{}



			const char* what() const override {return (String(loc)+": "+message).c_str();}
		};


		class DefaultSocketAccess : public SocketAccess
		{
		public:
			SOCKET			socketHandle;

			/**/			DefaultSocketAccess():socketHandle(INVALID_SOCKET){}

			virtual SocketAccess*	CloneClass() const
			{
				return new DefaultSocketAccess();
			}

			virtual bool	IsClosed() const override
			{
				return socketHandle == INVALID_SOCKET;
			}

			virtual void	CloseSocket() override
			{
				SwapCloseSocket(socketHandle);
			}
		
			virtual void	SetSocket(SOCKET socket)	override
			{
				socketHandle = socket;
			}

			virtual	int		Read(void*data, size_t dataSize) override
			{
				return recv(socketHandle,(char*)data,(int)dataSize,0);
			}

			virtual int		Write(const void*data, size_t dataSize) override
			{
				const char* at = (const char*)data,
						* end = at + dataSize;
				while (at < end)
				{
					#if 0
						//this works ... a little too well:
						timeval tv;
						tv.tv_sec = 0;
						tv.tv_usec = 100000;
						FD_SET set;
						FD_ZERO(&set);
						FD_SET(socketHandle,&set);
						select(1,nullptr,&set,nullptr,&tv);
						if (!FD_ISSET(socketHandle,&set))
						{
							return -1;
						}
					#endif /*0*/

					int sent = send(socketHandle,at,end-at,0);
					if (sent <= 0)
						return sent;
					at += sent;
				}
				return (int)dataSize;
			}
		};


		extern bool			verbose;	//!< Set true to automatically print events to the console. false by default

		namespace User	//! User level definitions for use by the client application
		{
			enum user_t	//! User level definitions for use by the client application
			{
				Anonymous,		//!< Non-authenticated user
				Authenticated,	//!< General authenticated user
				Privileged,		//!< Administrator grade authenticated user
				Root,			//!< Authenticated super user
			
				Any=Anonymous
			};
		}
	
		typedef User::user_t	user_t;


		class Peer;
		typedef std::shared_ptr<Peer>	PPeer;
		typedef std::weak_ptr<Peer>	WPeer;

		struct TDualLink
		{
			Peer					*p;	//cannot avoid pointer sometimes
			PPeer					s;

			/**/					TDualLink():p(NULL){}
			explicit				TDualLink(Peer*_p):p(_p)	{}
			explicit				TDualLink(const PPeer&_s):p(NULL),s(_s)	{}
		};


		class Package
		{
		public:
			/**/		Package():dataIncludingHeaderSpace(8)
			{
				Header()[0] = 0;
				Header()[1] = 0;
			}

			template <typename Serializable>
				explicit	Package(const Serializable&s)
				{
					Update(s);
				}

			template <typename Serializable>
				/**/		Package(const Serializable&s, UINT32 channelID)
				{
					Update(s);
					UpdateChannelID(channelID);
				}

			const Array<BYTE>& GetData() const
			{
				return dataIncludingHeaderSpace;
			}
			UINT32	GetChannel() const
			{
				return Header()[0];
			}

			void	UpdateChannelID(UINT32 id)
			{
				Header()[0] = id;
			}


			template <typename Serializable>
				void	Update(const Serializable&s)
				{
					using Serialization::Serialize;

					SerialSizeScanner scanner;
					Serialize(scanner,s);

					dataIncludingHeaderSpace.SetSize(scanner.GetTotalSize()+8);
					SerializeToCompactMemory(s,dataIncludingHeaderSpace+8,scanner.GetTotalSize());
					Header()[0] = 0;
					Header()[1] = (UINT32)scanner.GetTotalSize();
				}
		private:

			UINT32*	Header()
			{
				return (UINT32*)dataIncludingHeaderSpace.pointer();
			}
			const UINT32*	Header()const
			{
				return (const UINT32*)dataIncludingHeaderSpace.pointer();
			}
			Array<BYTE>	dataIncludingHeaderSpace;
		};
	

		template <typename T>
			class Serial
			{
			private:
				bool		sealed;
				T			element;
				Package		serialized;
			public:
				/**/		Serial():sealed(false)
				{}

				T&			Ref()
				{
					ASSERT__(!sealed);
					return element;
				}

				void		Seal()
				{
					if (sealed)
						return;
					sealed = true;
					serialized.Update(element);
				}

				const Package&	IncludeChannelID(UINT32 channelID)
				{
					ASSERT__(sealed);
					serialized.UpdateChannelID(channelID);
					return serialized;
				}

				const Package&	Get() const
				{
					ASSERT__(sealed);
					return serialized;
				}

			};


		///**
		//@brief Signal serializable that does not hold any actual data.
		//
		//VoidSerializable is always of size 0 and ommits any decoding or encoding but returns success upon doing so
		//*/
		//class VoidSerializable:public SerializableObject
		//{
		//public:
		//	virtual	void			Serialize(IWriteStream&stream) const		override	{}
		//	virtual	void			Deserialize(IReadStream&stream)	override	{}
		//};
	



		/**
		@brief Abstract data destination
		
		Peer (client) or Server (all clients) are of this type and implement the abstract SendPackage() method
		*/
		class Destination
		{
		public:
		virtual					~Destination()	{};
			/**
			@brief Abstract data send method
				
			SendPackage() sends a serializable data structure on the specified channel to the remote destination described by the local structure
			@param channel Channel to send the data on
			@param serialized Data to send on the specified channel
			@return true on success, false otherwise
			*/
			virtual	void		SendPackage(const Package&, unsigned minUserLevel)=0;
			virtual	void		SendPackage(const PPeer&exclude, const Package&p, unsigned minUserLevel)	{SendPackage(p,minUserLevel);};
		};
		typedef std::shared_ptr<Destination>	PDestination;
		/**
			@brief Abstract channel installation to handle outgoing and incoming packages
		
			This structure is for inheritance only and does not provide public members.
		*/
		class RootChannel 
		{
		protected:
			const UINT32		id;			//!< Channel id. Unmutable
			friend class 		Peer;
			friend class 		Connection;
			friend class		Dispatcher;
			System::Mutex				sendMutex;	//!< Mutex used to shield outgoing serializations to the tcp socket
			
								RootChannel(UINT32 id_):id(id_)
								{}
			
							
			/**
			@brief Deserializes the stream into a new object

			This method is called asynchronously by the respective connection's worker thread.
			*/
			virtual	PDispatchable	Deserialize(IReadStream&stream,Peer&sender)
			{
				DBG_FATAL__("TCP: Trying to deserialize on a pure root channel");
				return PDispatchable();
			}
		public:
			/**
			@brief Dispatches a successfully deserialized object (returned by Deserialize() )

			The method is invoked only, if Deserialize() returned a new non-NULL object.
			Depending on whether the connection is synchronous or asynchronous, this method is called my the main thread, or the connection's worker thread.
			*/
			virtual	void				Handle(const PDispatchable&serializable,const TDualLink&sender)	{FATAL__("Channel failed to override handle()");};
		};


		/**
			@brief Dedicated send-only channel
		*/
		template <class SerializableObject, unsigned Channel>
			class ObjectSender:public RootChannel
			{
			protected:
				virtual	PDispatchable	Deserialize(IReadStream&stream,Peer&sender)	override
				{
					DBG_FATAL__("TCP: Trying to deserialize on a pure object sender channel");
					return PDispatchable();
				}
			public:
				/**/					ObjectSender():RootChannel(Channel)
										{}

				
				void					SendTo(Destination&destination, const SerializableObject&object, unsigned minUserLevel=0)
				{
					destination.SendPackage(Package(object,RootChannel::id),minUserLevel);
				}
				void					SendTo(const PDestination&destination, const SerializableObject&object, unsigned minUserLevel=0)
				{
					if (!destination)
						return;	//silently ignore this
					SendTo(*destination,object,minUserLevel);
				}
				/**
				@brief Special overload form of sendTo for servers only
					
				Allows to send objects to all clients except a specific client
					
				@param destination Transfer destination. The method will fail if @a destination is NULL
				@param exclude Peer to specifically exclude
				@param object Serializable object to send
				*/
				void				SendTo(const PDestination&destination, const PPeer&exclude, const SerializableObject&object, unsigned minUserLevel=0)
				{
					if (!destination)
						return;	//silently ignore this
					SendTo(*destination,exclude,object,minUserLevel);
				}
				void				SendTo(Destination&destination, const PPeer&exclude, const SerializableObject&object, unsigned minUserLevel=0)
				{
					destination.SendPackage(exclude,Package(object,RootChannel::id), minUserLevel);
				}


				void					SendTo(Destination&destination, Serial<SerializableObject>&object, unsigned minUserLevel=0)
				{
					destination.SendPackage(object.IncludeChannelID(RootChannel::id), minUserLevel);
				}
				void					SendTo(const PDestination&destination, Serial<SerializableObject>&object, unsigned minUserLevel=0)
				{
					if (!destination)
						return;	//silently ignore this
					SendTo(*destination,object,minUserLevel);
				}
				/**
				@brief Special overload form of sendTo for servers only
					
				Allows to send objects to all clients except a specific client
					
				@param destination Transfer destination. The method will fail if @a destination is NULL
				@param exclude Peer to specifically exclude
				@param object Serializable object to send
				*/
				void				SendTo(const PDestination&destination, const PPeer&exclude, Serial<SerializableObject>&object, unsigned minUserLevel=0)
				{
					if (!destination)
						return;	//silently ignore this
					SendTo(*destination,exclude,object,minUserLevel);
				}
				void				SendTo(Destination&destination, const PPeer&exclude, Serial<SerializableObject>&object, unsigned minUserLevel=0)
				{
					destination.SendPackage(exclude,object.IncludeChannelID(RootChannel::id), minUserLevel);
				}
												
			};
	
		/**
			@brief Dedicated send-only data-less channel
		*/
		template <unsigned ChannelID>
			class SignalSender:public RootChannel
			{
			protected:
				virtual	PDispatchable	Deserialize(IReadStream&stream,Peer&sender)	override
				{
					DBG_FATAL__("TCP: Trying to deserialize on a pure signal sender");
					return PDispatchable();
				}
			public:
				/**/				SignalSender():RootChannel(ChannelID)
									{}
				/**
				@brief Sends a signal (data-less package) to the specified destination
				*/
				void				SendTo(Destination&destination, unsigned minUserLevel=0)
				{
					destination.SendPackage(Package(),minUserLevel);
				}
				void				SendTo(const PDestination&destination, unsigned minUserLevel=0)
				{
					if (!destination)
						return;
					destination->SendPackage(Package(),minUserLevel);
				}
				/**
				@brief Special overload form of sendTo for servers only
					
				Allows to send objects to all clients except a specific client
					
				@param destination Transfer destination. The method will fail if @a destination is NULL
				@param exclude Peer to specifically exclude
				@param object Serializable object to send
				*/
				void				SendTo(const PDestination&destination, const PPeer&exclude, unsigned minUserLevel=0)
				{
					if (!destination)
						return;
					destination->SendPackage(exclude,Package(),minUserLevel);
				}
				void				SendTo(Destination&destination, const PPeer&exclude, unsigned minUserLevel=0)
				{
					destination.SendPackage(exclude,Package(),minUserLevel);
				}
												
			};



		namespace Event	//! Socket event
		{
			#undef None
			enum event_t	//! Socket event
			{
				None,					//!< Void
				ConnectionFailed,		//!< The local connection attempt has failed
				ConnectionEstablished,	//!< The local connection attempt has succeeded
				ConnectionLost			//!< Connection closed unexpectedly
			};
		}
	
		typedef Event::event_t	event_t;
	
		const char*	event2str(event_t);	//!< Retrieves a string representation of the specified event (for logging or other debug output)
	
		/**
			@brief Package handler
		
			The dispatcher handles incoming objects and events. Objects and events are enqueued if @b async is false. Otherwise they are directly executed and deleted if appropriate.
			If @b async is false then the client application must invoke Resolve() from time to time to handle incoming events and objects
		*/
		class Dispatcher
		{
		private:
			struct TCommonEvent
			{
				enum type_t
				{
					NetworkEvent,
					Signal,
					Object
				};

				type_t					type;

				event_t					event;	//only if type==NetworkEvent

				UINT32					channel;	//only if type==Signal

				PDispatchable			object;		//only if type==Object
				RootChannel				*receiver;	//only if type==Object
			
				TDualLink				sender;
			};

			
			//Queue<TInbound>				object_queue;
			//Queue<TEvent>				event_queue;
			//Queue<TSignal>				signal_queue;
			Ctr::Queue<TCommonEvent>	queue;
			volatile bool				anyEventsPending;	//low impact if not initialized properly. Self-managed after the first frame or so. Only used in synchronous mode. must be set true, if anything is filled into queue.

			Ctr::Vector<PPeer>			wasteBucket;


			Sys::Mutex					wasteMutex;
			bool						async;
			volatile bool				is_locked;
			Ctr::IndexTable<RootChannel*>	channel_map;
			Ctr::IndexTable<unsigned>	signal_map;
			Dispatcher*					relayTo=nullptr;

			bool						resolving,terminateAfterResolve;
			Ctr::Vector<TCommonEvent>	resolutionBuffer;	//used as temporary storage by Resolve() for events taken out of queue prior to resolution.
			Sys::SpinLock					postResolutionCallbackLock;
			Ctr::Vector<std::function<void()>,Swap>	postResolutionCallbacks;

			
			//friend class Peer;
		protected:
			bool						PostResolveTerminationIsImminent() const {return resolving && terminateAfterResolve;}
			virtual void				HandleSignal(UINT32 signal, Peer&sender);
			virtual void				HandleObject(RootChannel*receiver, Peer&sender, const PDispatchable&object);
			RootChannel*				getReceiver(UINT32 channel_id, unsigned user_level);
			bool						QuerySignalMap(UINT32 channelID, unsigned&outMinChannelID) const;
			String						DebugGetOpenSignalChannels() const;
			String						DebugGetOpenPackageChannels() const;

			friend class ConnectionAttempt;
		public:
			void (*onEvent)(event_t event, Peer&origin);	//!< Callback hook for events (connection, disconnection, etc). NULL by default. @param event Event that occured @param origin Event origin
			void (*onSignal)(UINT32 signal, Peer&origin);	//!< Callback hook for signals (data-less packages). NULL by default.  @param signal Channel that the data-less package was received on @param origin Origin of the signal
			void (*onIgnorePackage)(UINT32 channel,UINT32 size,Peer&origin);	//!< Callback hook for ignored packages. Always async. NULL by default. @param channel Channel the package or signal was received on @param size Package size in bytes @param origin Receiving peer
			void (*onDeserializationFailed)(UINT32 channel, Peer&origin, const String&exceptionMessage);	//!< Callback hook for deserialization failures. NULL by default. If not set, such errors cause a fatal exception in debug mode, and are ignored in release mode.
	
			/**/						Dispatcher();
			virtual						~Dispatcher();
			bool						IsResolving() const {return resolving;}
			/**
			Attaches the specified function to the post-resolution queue, if currently resolving.
			Executed directly otherwise
			*/
			void						OnDoneResolving(const std::function<void()>&);
			/**
			Resolves all events that occured since the last Resolve() invokation.
			The client application must call this method frequently from the same thread, if the local dispatcher is set to synchronous. Never otherwise.
			This method is not thread-safe. It interacts savely with the internal TCP mechanics but calling this method from multiple threads without some form of synchronization will cause problems.
			*/
			void						Resolve();
			void						FlushPendingEvents();	//!< Dumps all remaining, pending events without executing them. The client application may call this method if the local dispatcher is set to synchronous. Nothing happens otherwise.
			/**
			Installs a channel in/out processor.
			If the used channel id is already in use then it will be overwritten with the provided channel handler.
			Object/signal relaying must be disabled, or the method will issue a critical assertion.
			*/
			void						InstallChannel(RootChannel&channel);
			/**
			Uninstalls a channel in/out processor. The used channel id will be closed.
			Object/signal relaying must be disabled, or the method will issue a critical assertion.
			*/
			void						UninstallChannel(RootChannel&channel);
			/**
			Opens a signal channel for receiving.
			Object/signal relaying must be disabled, or the method will issue a critical assertion.
			@param id Channel id to listen for signals on @param min_user_level Minimum required user level for this signal to be accepted
			*/
			void						OpenSignalChannel(UINT32 id, unsigned minUserLevel);
			/**
			Closes a signal channel.
			Object/signal relaying must be disabled, or the method will issue a critical assertion.
			*/
			void						CloseSignalChannel(UINT32 id);
			
			void						SetAsync(bool is_async)		//! Changes the socket asynchronous status. Dispatchers run in async mode by default. @param is_async Set true if the dispatcher should perform operations asynchronously from this point on, false otherwise
										{
											ASSERT_IS_NULL__(relayTo);
											async = is_async;
										}
			bool						IsAsync()	const			//! Queries whether or not this dispatcher is currently asynchronously. Dispatchers run in async mode by default.
										{
											return relayTo ? relayTo->IsAsync() : async;
										}
			/**
			Defines a relay dispatcher to get channels from and to receive incoming objects, events and signals instead of the local instance.
			Set NULL to disable.
			If set, local channel declarations and async settings are completely ignored
			*/
			void						RelayTo(Dispatcher*relay)	{ASSERT__(!relay || channel_map.IsEmpty()); relayTo = relay;}

			virtual void				HandleEvent(event_t ev, Peer&sender);
		protected:
			/**
			Can only be called from inside Resolve(). Sets an interal flag to call PostResolutionTermination() once resolution is done.
			Subsequent events during this resolution execution are discarded.
			*/
			void						SignalPostResolutionTermination();

			/**
			Signals that Resolve() has finished, and some callback function triggered an execution of SignalPostResolutionTermination() in the meantime.
			*/
			virtual void				PostResolutionTermination()=0;


			void						FlushWaste();
			EventLock					eventLock;
			void						HandlePeerDeletion(const PPeer&);
		};
	
	
		/**
		@brief Abstract connection prototype
		
		A connection represents the largest most abstract network structure. Only server/client objects qualify as Connection derivatives.
		*/
		class Connection:public Dispatcher
		{
		protected:
			serial_size_t				safe_package_size;	//!< Maximum allowed package size. ~64MB by default
			
										Connection():safe_package_size(64000000)
										{}

		public:
			virtual	void				OnDisconnect(const Peer*, event_t event)	{};	//!< Abstract disconnection even called if the local connection has been lost or closed

			serial_size_t				GetSafePackageSize() const {return safe_package_size;}
			bool						HandleIncomingSignal(UINT32 channelID, Peer&sender);
			/**
			Handles an incoming package on a specific channel.
			Automatically looks up the appropriate receiver and dispatches the message
			@param channelID Channel the package was received on
			@param sender Sender peer
			@param stream Source stream. Must be EOF after deserialization or package will be ignored
			@param dataSize Total available space
			*/
			void						HandleIncomingPackage(UINT32 channelID, Peer&sender, IReadStream&stream);
		};

		typedef std::shared_ptr<Connection>	PConnection;


		/**
		Setable/getable error state
		*/
		class ErrorState
		{
			Sys::Mutex					error_mutex;
			String						last_error,
										current_error;
		protected:
			void						SetError(const String&message)	//!< Updates the local error string. The local structure maintains two string objects to reduce the risk of invalid pointer access (???)
										{
											error_mutex.lock();
												last_error.adoptData(current_error);
												current_error = message;
											error_mutex.release();
										}
		public:
			String						GetError(bool flushError)	//! Queries the last occured error
										{
											error_mutex.lock();
												String copy = current_error;
												if (flushError)
													current_error = "";
											error_mutex.release();
											return copy;
										}
		};

		class TCPThreadObject
		{
		private:
			std::thread			thread;
			volatile bool		done = true;
			Sys::Semaphore		sem;
			Sys::SpinLock		joinLock;


			static void			_Start(TCPThreadObject*self, Sys::Semaphore*sem)
			{
				sem->enter();
				self->ThreadMain();
				self->done = true;
			}
		public:



			virtual void		ThreadMain()=0;



			void				Start()
			{
				if (IsRunning())
					return;
				done = false;
				//Semaphore			sem;	//perhaps better not to have this in stack
				auto s = std::thread(_Start, this, &sem);
				thread.swap(s);
				sem.leave();
			}

			bool				IsDone() const
			{
				return done;
			}

			bool				IsRunning() const
			{
				return thread.joinable();
			}

			bool				IsSelf() const
			{
				return std::this_thread::get_id() == thread.get_id();
			}

			void				AssertIsSelf()
			{
				std::thread::id self = std::this_thread::get_id();
				std::thread::id local = thread.get_id();
				std::stringstream ss;
				ss << self<<"!="<<local;
				//string mystring = ;
				ASSERT1__(self == local,ss.str().c_str());
		
			}

			void				Join()
			{
				if (IsSelf())
					return;
				joinLock.lock();
				TCP_TRY
				{
					if (thread.joinable())
						thread.join();
					auto s = std::thread();
					thread.swap(s);
				}
				TCP_CATCH
				joinLock.unlock();
			}
		};



		class PeerWriter : public TCPThreadObject
		{
			Ctr::WorkPipe<Ctr::Array<BYTE> >		pipe;
			SocketAccess				*volatile accessPointer;
			Sys::SpinLock				accessPointerLock;
		public:
			volatile bool				connectionLost;
			Peer						*const parent;
		
			/**/						PeerWriter(Peer*parent):accessPointer(nullptr),connectionLost(true),parent(parent){}

			void						Begin(SocketAccess*access);
			void						Update(SocketAccess*access);
			void						Terminate();

			void						ThreadMain() override;			//!< Socket write thread main

			template <typename Serializable>
			void						Write(UINT32 channel, const Serializable&s)
			{
				if (connectionLost)
					throw Except::IO::Network::ConnectionLost(CLOCATION,"Cannot write data on channel "+String(channel));
				using Serialization::Serialize;
				SerialSizeScanner scanner;
				Serialize(scanner,s);

				const serial_size_t size = scanner.GetTotalSize();
				Array<BYTE>	data(8 + size);
				UINT32*header = (UINT32*)data.pointer();
				header[0] = channel;
				header[1] = (UINT32)size;

				SerializeToCompactMemory(s,data.pointer()+8,size);
				pipe.MoveAppend(data);
			}
			void						Write(UINT32 channel, const void*rawData, size_t numBytes)
			{
				if (connectionLost)
					throw Except::IO::Network::ConnectionLost(CLOCATION,"Cannot write data on channel "+String(channel));
				Array<BYTE>	data(8 + numBytes);
				UINT32*header = (UINT32*)data.pointer();
				header[0] = channel;
				header[1] = (UINT32)numBytes;
				memcpy(data.pointer()+8,rawData,numBytes);
				pipe.MoveAppend(data);
			}

			bool						Write(const Package&packet)
			{
				if (connectionLost)
					return false;
				pipe << packet.GetData();
				return true;
			}

			bool						WriteSignal(UINT32 signal)
			{
				if (connectionLost)
					return false;
				Array<BYTE>	data(8);
				(*(UINT32*)data.pointer()) = signal;
				(*(((UINT32*)data.pointer())+1)) = (UINT32)0;
				pipe.MoveAppend(data);
				return true;
			}

		};

	
		/**
		@brief TCP socket handler
		
		A peer handles incoming and outgoing packages from/to a specific IP address over a specific socket handle. A server would manage one peer structure per client, a client would inherit from Peer.
	
		*/
		class Peer : protected TCPThreadObject, protected IReadStream, /*protected IWriteStream, */public Destination, public ErrorState
		{
		private:
			friend class Dispatcher;
			friend class ConnectionAttempt;
			AtomicTimeStamp				lastReceivedPackage;

			void						CloseWriterDown(Event::event_t ev, const String&errorMessage);
		protected:
			PeerWriter					writer;
			Connection					*owner;			//!< Pointer to the owning connection to handle incoming packages and report errors to
			Sys::Mutex					//write_mutex,	//!< Mutex to synchronize socket write operations
										counter_mutex;	//!< Currently not in use
			//volatile SOCKET				socket_handle;	//!< Handle to the occupied socket
			serial_size_t				remaining_size,			//!< Size remaining for reading streams 
										remaining_write_size;	//!< Size remaining for writing streams in the announced memory frame
			ByteStreamBuffer			serial_buffer;		//!< Buffer used to serialize package data
			SocketAccess				*socketAccess;
			std::weak_ptr<Peer>			self;
			friend class Server;
			friend class RootChannel;
			friend class PeerWriter;
			
			void						ThreadMain() override;			//!< Socket read thread main
			void						SendData(const Package&p);		//!< Sends a finished package to the TCP stream. Used by server
			bool						succeeded(int result, size_t desired);							//!< Handles the result of a TCP socket send operation. @param result Actual value returned by send() @param desired Valued expected to be returned by send() @return true if both values match, false otherwise. The connection is automatically closed, events triggered and error values set if the operation failed.
			void						handleUnexpectedSendResult(int result);
			void						Read(void*target, serial_size_t size) override;								//!< IInStream override for direct TCP stream input
			//bool						Write(const void*target, serial_size_t size) override;						//!< IOutStream override for direct TCP stream output
			serial_size_t				GetRemainingBytes() const override;
			void						NetRead(BYTE*current, size_t size);							//!< Continuously reads a sequence of bytes from the TCP stream. The method does not return until either the requested amount of bytes was received or an error occured
			

			virtual String				AddressToString(bool includePort) const
										{
											return TCP::ToString(address,addressLength,includePort);
										}
			virtual bool				AddressIsLocalhost() const
										{
											return addressLength && AddressToString(false) == "localhost";
										}

		public:
			/**
				@brief Abstract peer attachment class
			*/
			class Attachment
			{
			public:
			virtual						~Attachment()	{};
			};
			/**
				@brief Peer attachment
				
				Useful to attach client data to a peer. If @a delete_attachment_on_disconnect is set true then the attached data structure will be erased automatically before this peer object is deleted.
				
			*/
			std::shared_ptr<Attachment>	attachment;
			sockaddr_storage			address;
			socklen_t					addressLength;
			unsigned					userLevel;							//!< Current user level. Anonymous (0) by default
			volatile bool				destroyed;
		
				
			/**/						Peer(Connection*connection):owner(connection),socketAccess(new DefaultSocketAccess()),
										userLevel(User::Anonymous),addressLength(0), destroyed(false),
										writer(this)
										{
											memset(&address,0,sizeof(address));
											ASSERT_NOT_NULL__(owner);
											//writer.Begin(socketAccess);
										}
			virtual						~Peer()
										{}
			/**
			Basically the destructor, except not a destructor, so the destruction won't throw exceptions.
			*/
			void						Dissolve()
										{
											writer.Terminate();
											ASSERT__(!socketAccess || socketAccess->IsClosed());
											SocketAccess*a = socketAccess;
											socketAccess = nullptr;
											if (a)
												delete a;
										}

			/**
			Pre-desructor shut-down
			*/
			bool						Destroy()
										{
											if (!destroyed)
											{
												writer.Terminate();
												destroyed = true;
												DisconnectPeer();
												return true;
											}
											return false;
											//if (root_address)
											//	freeaddrinfo(root_address);
										}

			template <class SocketAccessClass>
				void					SetSocketAccess()
										{
											ASSERT__(socketAccess->IsClosed());
											delete socketAccess;
											socketAccess = new SocketAccessClass();
											writer.Update(socketAccess);
										}
			void						SetCloneOfSocketAccess(SocketAccess*access)
										{
											ASSERT__(socketAccess->IsClosed());
											delete socketAccess;
											socketAccess = access->CloneClass();
											writer.Update(socketAccess);
										}

			/**
			Updates the local weak pointer reference to the given shared_ptr.
			A critical assertion is triggered if the passed pointer does not match the local object's address.
			Must be called by a server instance for each of its clients upon connect.
			Self serving client instances do not need to call this method since they are rarely shared pointers
			*/
			void						SetSelf(const PPeer&);
			TDualLink					LinkFromThis()
			{
				PPeer p = self.lock();
				if (p)
					return TDualLink(p);
				return TDualLink(this);
			}


			virtual void				DisconnectPeer();			//!< Disconnects the local peer. If this peer is element of a peer collection (ie. a Server instance) then the owner is automatically notified that the client on this peer is no longer available. The local data is erased immediately if the respective dispatcher is set to @b async, or when its resolve() method is next executed.
			virtual bool				IsConnected()	const	//! Queries whether or not the local peer is currently connected @return true if the local peer is currently connected, false otherwise
										{
											return !socketAccess->IsClosed();
										}

			/**
			Logs the specified error message (including the last socked error) and disconnects the local peer
			*/
			void						Fail(const String&message);


			String						ToString(bool includePort=true)	const	//! Converts the local address into a string. If the local object is NULL then the string "NULL" is returned instead.
										{
											return this?AddressToString(includePort):"NULL";
										}
			bool						IsLocalhost() const
										{
											return this && AddressIsLocalhost();
										}
			virtual bool				SendSignal(UINT32 channel);		//!< Sends a data-less package to the other end of this peer
			
			bool						HandleIsValid()	const	{return !socketAccess->IsClosed();}
			void						SendPackage(const Package&object, unsigned minUserLevel=0) override;		//!< Sends a serializable object to the TCP stream on the specified channel
			/**
			Retrieves the high-resolution time of last receiving of any package from the remote end. Even partial packages count
			*/
			Time::Stamp					GetLastPackageTime()	const	{return lastReceivedPackage.Cast();}
			void						SignalPackage()					{lastReceivedPackage.Update();}
		};

		typedef std::shared_ptr<Peer::Attachment>	PAttachment;

		class Client;
	
		/**
			@brief Asynchronous connection attempt
		*/
		class ConnectionAttempt:public TCPThreadObject
		{
		protected:
			friend class Client;
			void				ThreadMain();
		public:
			String				connect_target;	//!< Target address
			Peer				*const client;		//!< Client to connect
			Dispatcher			*const dispatcher;

			/**/				ConnectionAttempt(Peer*parent,Dispatcher*state):client(parent),dispatcher(state)	{}
		};

		/**
			@brief Single socket TCP connection client
		
			The Client object establishes and maintains a connection to a server. Multiple connections to different servers require different instances of Client.
		*/
		class Client : public Connection, public Peer
		{
		protected:
			ConnectionAttempt	attempt;
			bool				is_connected;	//!< True if the client has an active connection to a server
			mutable std::recursive_mutex	connectionLock;

			mutable Sys::SpinLock	attemptHostCopyLock;
			String				attemptHostCopy;	//!< Host passed during the last connection attempt. Protected by connectionLock
		public:
					
			/**/				Client():Peer(this),is_connected(false),attempt(this,this)
								{}
			virtual				~Client()
								{
									attempt.Join();
									DisconnectPeer();
									Join();
								}
			bool				Connect(const String&url);		//!< Attempts to connect to a remote server and waits until a connection was established (or not establishable). Depending on the local connection this may lag for a few seconds. @return true if a connection could be established, false otherwise.
			void				ConnectAsync(const String&url);	//!< Starts the process of connecting to a remote host. Due to the asynchronous nature of this method, No immediate result is available. Hook a function into the local object's inherited @a onEvent callback pointer to receive information about success or failure
			
			//bool				IsConnected() const {return is_connected;}	//implemented by Peer

			bool				IsAttemptingToConnect()	const	//!< Queries whether or not the client is currently attempting to connect
								{
								
									return attempt.IsRunning() && !attempt.IsDone();
								}
			void				Disconnect();

		protected:
			virtual void		PostResolutionTermination() override {Disconnect();}
			virtual String		AddressToString(bool includePort) const override;
			virtual void		HandleEvent(event_t ev, Peer&sender) override
			{
				if (ev != Event::ConnectionEstablished)
					this->addressLength = 0;
				Connection::HandleEvent(ev,sender);
			}

		};
	
		/**
		@brief Multi socket TCP connection server
		
		The Server object sets up a local listen port and automatically accepts incoming connection requests.
		Dispatcher for channel handling is inherited via Connection
		*/
		class Server : public Connection, private TCPThreadObject, public Destination, public ErrorState
		{
		private:
			volatile SOCKET		socket_handle;
			volatile bool		clients_locked,
								is_shutting_down;

			Peer				centralPeer;	//used for server-events to not pass null

			SocketAccess		*socketAccess;

			//Ctr::Array<BYTE>		out_buffer;
			
			void				ThreadMain() override;
		protected:
			void				OnDisconnect(const Peer*, event_t) override;	//!< Executed by a peer if its connection died
			void				Fail(const String&message);					//!< Executed if an operation failed
			void				SendPackage(const Package&object, unsigned minUserLevel) override;
			void				SendPackage(const PPeer&exclude, const Package&object, unsigned minUserLevel) override;
		


			bool				IsShuttingDown() const {return is_shutting_down;}

		private:
			Sys::ReadWriteMutex	clientMutex;	//!< Mutex to shield operations on the client list. This mutex should be locked before clients are queried
			Ctr::Vector<PPeer>	clientList;		//!< List of all currently connected clients
			USHORT				port;			//!< Read only variable which is updated during StartService()
		public:

	
								Server():socket_handle(INVALID_SOCKET),clients_locked(false),is_shutting_down(false),socketAccess(new DefaultSocketAccess()),centralPeer(this)
								{}
			virtual				~Server()
								{
									EndService();
									//clientMutex.BeginWrite();
									//	clientList.Clear();
									//clientMutex.EndWrite();
									delete socketAccess;
								}
			bool				HasAnyClients() const
			{
				return clientList.Count() != 0;
			}
			template <typename F>
				void			VisitAllPeerAttachments(const F&f)
								{
									clientMutex.BeginRead();
									{
										foreach (clientList,client)
										{
											const PPeer&p = *client;
											if (p->attachment)
												f(p->attachment);
										}
									}
									clientMutex.EndRead();
								}
			template <typename F>
				void			VisitAllPeers(const F&f)
								{
									clientMutex.BeginRead();
									{
										foreach (clientList,client)
										{
											f(*client);
										}
									}
									clientMutex.EndRead();
								}
			template <class SocketAccessClass>
				void			SetSocketAccess()
								{
									ASSERT__(socketAccess->IsClosed());
									delete socketAccess;
									socketAccess = new SocketAccessClass();
								}

			UINT16				GetPort() const {return port;}
			/**
			@brief Starts the local service on the specified port
				
			This method attempts to bind the specified port to the local object and listen for any incoming
			connections. Query the result of error() to retrieve a more detailed error discription in case
			this method returns false.
			@param port Port to bind this service to
			@param limitToLocalhost If true, only clients connecting from localhost will be permitted
			@param[out] outPort Actually used port. Useful only if @a port is set to 0 such that a dynamic port is assigned
			@return true on success, false otherwise.
			*/
			bool				StartService(USHORT port, bool limitToLocalhost=false, USHORT*outPort=nullptr);

			enum class Protocol
			{
				IPv4,
				IPv6
			};

			bool				ExtStartService(USHORT port, Protocol proto, bool limitToLocalhost=false, USHORT*outPort=nullptr);


			/**
			@brief Terminates the service.
				
			Any currently connected clients are automatically disconnected.
			This operation executes the ConnectionClosed event once for each remaining client and once for itself
			passing NULL as @b peer parameter.
			*/
			void				EndService();
			void				StopService()	{EndService();};	//!< @overload
			void				Shutdown()		{EndService();};		//!< @overload
			bool				IsOnline()	const	//!< Queries the current execution status
								{
									return IsRunning() && !IsDone() && !PostResolveTerminationIsImminent();
								}

			PPeer				GetSharedPointerOfClient(Peer*);
		protected:
			virtual void		PostResolutionTermination() override {EndService();}

		};





		/**
			@brief Abstract template channel structure to handle all traffic on a specific channel
		
			Instances must override the OnReceive() method or specify an onObjectReceive function pointer.
			Otherwise incoming packages are ignored.
		
			@param Object the type of object being sent and received on this channel. Must be a derivative of @a ISerializable
			@param MinUserLevel Minimum remote user level required for an incoming package to be decoded on this channel. If the remote user level does not satisfy this level then the incoming package will be ignored
			@param Channel Channel that this channel structure should operate on
		*/
		template <class Object, unsigned MinUserLevel, UINT32 ChannelID>
			class Channel:public ObjectSender<Object,ChannelID>
			{
			public:
				typedef ObjectSender<Object,ChannelID>	Super;
				void					(*onObjectReceive)(Object&object, Peer&sender);
				void					(*onSimpleObjectReceive)(Object&object);
				
				
				virtual	void			Handle(const PDispatchable&obj,const TDualLink&sender)	override
				{
					PPeer p = sender.s;
					Peer*ptr = p ? p.get() : sender.p;
					if (ptr && ptr->userLevel >= MinUserLevel)
					{
						OnReceive(*(Object*)obj.get(),*ptr);
					}
					else if (!ptr)
					{
						//DBG_FATAL__("peer reference lost in transit for data on channel "+String(ChannelID));
					}
					//discard((Object*)serializable);
				}
			protected:

				static void MyDelete(Dispatchable*p)
				{
					delete (Object*)p;
				}
								
				virtual	PDispatchable	Deserialize(IReadStream&stream,Peer&sender)	override
				{
					Object*rsObject = new Object();
					PDispatchable result;
					try
					{
						using Serialization::Deserialize;
						Deserialize(stream,*rsObject);
						result.reset((Dispatchable*)rsObject,MyDelete);
					}
					catch (const std::exception&ex)
					{
						delete rsObject;
						throw;
					}
					return result;
				}
								
				virtual	void			OnReceive(Object&object, Peer&sender)
				{
					if (onObjectReceive)
						onObjectReceive(object,sender);
					elif (onSimpleObjectReceive)
						onSimpleObjectReceive(object);
				}
		
			public:
					
				/**/			Channel():onObjectReceive(NULL),onSimpleObjectReceive(NULL)
								{}
								
				/**/			Channel(void(*onObjectReceive_)(Object&object, Peer&sender)):onObjectReceive(onObjectReceive_),onSimpleObjectReceive(NULL)
								{}
				/**/			Channel(void(*onObjectReceive_)(Object&object)):onObjectReceive(NULL),onSimpleObjectReceive(onObjectReceive_)
								{}
	
			};



	}




}

#endif
