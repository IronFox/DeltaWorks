#ifndef nodeH
#define nodeH

/******************************************************************

Network-interface.

******************************************************************/

#include "../global_root.h"

#pragma comment(lib,"ws2_32.lib")

#if SYSTEM==WINDOWS
	#include <winsock2.h>
	#include <io.h>
	#include <ws2tcpip.h>
	
#elif SYSTEM==UNIX
	#include <sys/socket.h>
	#include <sys/types.h>
	#include <sys/errno.h>
	
	#include <netinet/in.h>
	#include <netinet/tcp.h>

	#include <netdb.h>
	#include <arpa/inet.h>
	
	typedef int	SOCKET;
	#define INVALID_SOCKET	-1
	
	#define closesocket close
#endif


#include <fcntl.h>

#include "../global_root.h"
#include "../global_string.h"
#include "../general/timer.h"
#include "../general/thread_system.h"
#include "../io/delta_crc32.h"
#include "../io/mutex_log.h"
#include "../container/hashtable.h"
#include "../general/ref.h"

#include <iostream>



/* * necessary libs *

gcc: libws2_32.a (recommend) or libwsock32.a

*/

namespace DeltaWorks
{

	/**
		\brief Network namespace
	*/
	namespace Net
	{

		struct TPackageHead;
		struct TPackageInfo;
		struct TChannel;
		class Package;
		class Name;
		class Task;
		class TaskList;
		class QueueItem;
		class EventItem;
		class TransferItem;
		template <class> class Queue;
		class Node;
		class Connection;
		class Server;
		class Client;
		class ConnectionAttachment;


		typedef QueueItem			QueueItem;





		//typedef sockaddr_storage									NetIdent;
		typedef Container::IndexTable<TChannel>							ChannelList;

		typedef void (*_netReceive)(Connection*connection, UINT16 channel, const void*pntr,UINT32 size);
		typedef void (*_netEvent)(Connection*);
		typedef void (*_netClientEvent)(const Name&name,UINT32);
		typedef void (*_netClientDrop)(Connection*connection, UINT32 reason);
		typedef bool (*_netAccept4)(const sockaddr_in&name);			//must the thread-safe!!!!
		typedef bool (*_netAccept6)(const sockaddr_in6&name);			//must the thread-safe!!!!

	

		#define TMPL						template <class C> inline
		#define DOMAIN_M					//main-domain
		#define DOMAIN_A					//application-domain
		#define DOMAIN_T					//timer-domain
		#define NET_DOT						200
		#define NET_REQUEST_TIMEOUT			(2.0f)
		#define NET_RECEIVE_TIMEOUT			(1.0f)
		#define NET_SEND_TIMEOUT			(0.5f)
		#define NET_PING_INTERVAL			2	//one ping every 2 seconds
		#define NET_CONNECTION_TIMEOUT		10	//no message in 10 seconds? disconnect
		#define NET_SYNC_INTERVAL			5.0
		#define NET_SYNC_STEP				0.1
		#define NET_SYNC_STEPS				46		//46 sync requests sent in 4.6 seconds
		#define NET_SYNC_STEPS_NEEDED		36		//36 sync replies needed to continue
		#define NET_DELAY_BALANCE			0.5
		#define NET_SYNC_ADOPT_INTERVAL		4.0 //4 seconds to adopt to new server delta - that's full adopt one second before resync

		#define NET_DEFAULT_PORT			19788


		//package flags
		#define NET_FLOOD					0x0
		#define NET_CONFIRM					0x1
		#define NET_SERIAL					0x2
		#define NET_SECURE					(NET_CONFIRM|NET_SERIAL)
		#define NET_PIPE					NET_SECURE

		//internal flags
		#define NET_DATA					0x4
		#define NET_FIRST					0x8

		//messages
		#define NET_CL_REQUEST_CONNECT		0x1
		#define NET_SV_CONFIRM_CONNECT		0x2
		#define NET_SV_FORCE_DISCONNECT		0x3
		#define NET_SV_SERVER_FULL			0x4
		#define NET_SV_SERVER_SHUTDOWN		0x5
		#define NET_SV_DENY_CONNECT			0x6

		#define NET_SYNC_REQUEST			0x7
		#define NET_SYNC_REPLY				0x8
		#define NET_DISCONNECT				0x9
		#define NET_TICK					0xA

		//non-message-reasons
		#define NET_USER					0xB
		#define NET_TIMEOUT					0xC
		#define NET_CONNECTION_DIED			0xD
		#define NET_RECOVERY_FAILED			0xE



		//#define NET_ALLOW_LOW_EVENT_CATCH
		//#define NET_UNSTABLE_NET_TEST
		//#define NET_SYS_MESSAGE

		const char* msg2str(UINT16 message);
		String		ip2str(UINT32 ip);
		bool		initNet();
	
		const char*	err2str(unsigned code);
		const char*	GetLastError();


		void sysmessage(Node*node, const String&str);
		//#define sysmessage(node,str) {}

		//#define sysmessage(node,str) {cout << node->self_.ToString().c_str()<<": "<<String(str).c_str()<<endl;}


		/* package v2.0


		[flags:8][custom:24][lo:16][hi:16]

		lo:
			package id
			channel id

		hi:
			transfer id
			message id

		*/

		#if 0

		struct TPackageHead	//!	Package header
		{
			union
			{
				struct
				{
					BYTE			flags,		//!< Package flags
									custom[3];	//!< Custom package header content
				};
				UINT32				config;		//!< Package configuration
			};
			union
			{
				struct
				{
					UINT16			lo,hi;
				};
				UINT32				ident;	//!< Package ident
			};
		};

		static const UINT32				const_buffer_size=0x2000,
										max_package_size=const_buffer_size-sizeof(TPackageHead);

		struct TChannel	//! Net Channel
		{
				UINT32				package_size;	//!< Fixed package size if non-zero
				_netReceive			exec;	//!< Channel handler
		};

		class ConnectionAttachment	//! Attachment base class to attach data to a connection
		{
		public:
		virtual						~ConnectionAttachment()	{}
		};

		class QueueItem	//! Net Queue item
		{
		public:
		virtual				~QueueItem(){}
		};




		//class Name//! Network address identifier
		//{
		//public:


		//						Name()
		//						{}
		//						Name(const sockaddr_storage&ident):NetIdent(ident)
		//						{}
		//		int				CompareTo(const Name&other)	const
		//						{
		//							addr->
		//							if (this->ss_family < other.ss_family)
		//								return -1;
		//							if (ss_family > other.ss_family)
		//								return 1;
		//							if (ss_family == AF_INET6)
		//							{
		//								const sockaddr_in6&a = (const sockaddr_in6&)*this,
		//													&b = (const sockaddr_in6&)other;
		//								for (BYTE k = 0; k < 8; k++)
		//									if (a.sin6_addr.u.Word[k] < b.sin6_addr.u.Word[k])
		//										return -1;
		//									elif (a.sin6_addr.u.Word[k] > b.sin6_addr.u.Word[k])
		//										return 1;
		//							}
		//							else
		//							{
		//								const sockaddr_in	&a = (const sockaddr_in&)*this,
		//													&b = (const sockaddr_in&)other;
		//								if (a.sin_addr.S_un.S_addr < b.sin_addr.S_un.S_addr)
		//									return -1;
		//								elif (a.sin_addr.S_un.S_addr > b.sin_addr.S_un.S_addr)
		//									return 1;
		//							}
		//							return 0;
		//						}
		//		bool			operator<(const Name&other) const
		//						{
		//							return CompareTo(other) < 0;
		//						}
		//		bool			operator>(const Name&other) const
		//						{
		//							return CompareTo(other) > 0;
		//						}
		//		bool			operator==(const Name&other) const
		//						{
		//							return CompareTo(other) == 0;
		//						}
		//		String			ToString() const;		//!< Converts a network address into a string of the form 192.168.0.1:2048 \return String representation of the address
		//};



		class ClientEventItem:public QueueItem	//! Net client event item
		{
		public:
				_netClientEvent exec;				//!< Event handler
				const addrinfo*	target;				//!< Target address
				UINT32			parameter;			//!< Event parameter

								ClientEventItem(_netClientEvent event, const addrinfo*name, UINT32 para);
		virtual					~ClientEventItem();
		};

		class ClientDropItem:public QueueItem		//! Net client drop item
		{
		public:
				_netClientDrop	exec;				//!< Event handler
				Connection 	*target;				//!< Target connection
				UINT32			parameter;			//!< Event parameter
			
								ClientDropItem(_netClientDrop event, Connection*connection, UINT32 para);
		virtual					~ClientDropItem();
		};

		class EventItem:public QueueItem			//! Net event item
		{
		public:
				_netEvent		exec;				//!< Event handler
				Connection 	*target;				//!< Target connection

								EventItem(_netEvent event, Connection*connection);
		virtual					~EventItem();	//event executes on deletion
		};

		class TransferItem:public QueueItem		//! Net transfer finished item
		{
		public:
				BYTE			*data;				//!< Data pointer
				UINT32			data_len;			//!< Data length
				//BYTE			flags;				//!< Transfer flags
				UINT16			channel;			//!< Transfer channel
				Connection 	*sender;				//!< Transfer origin
				_netReceive		exec;				//!< Transfer handler

								TransferItem();
		virtual					~TransferItem();	//event executes on erase
		};


		template <class Item> class Queue		//! Net item queue
		{
		private:
				Item			*loop[0x10000];
				UINT16			first,last;
		public:
								Queue();
				Item*			peek();					//!< Queries the oldest element in the queue without removing it \return Pointer to the oldest element or NULL if the queue is empty
				Item*			get();					//!< Retrieves the oldest element in the queue by removing it from the queue \return Pointer to the oldest element or NULL if the queue is empty
				Item*			peek(UINT16 n);			//!< Queries the nth oldest element in the queue (with 0 being the oldest) without removing it \return Pointer to the nth oldest element or NULL if no such exists
				void			Insert(Item*item);		//!< Inserts a new item to the end of the queue \param item Pointer to the item that should be inserted
				void			clear();				//!< Sequentially erases all elements in the queue
				UINT16			elements();				//!< Counts the elements in the queue \return Number of elements in the queue
		};

		class Package	//! Full package (including data)
		{
		public:
				BYTE				field[const_buffer_size];		//!< Package data buffer (including header)
				TPackageHead		&head;						//!< Reference to the head of the package

									Package(BYTE flags, UINT16 lo, UINT16 hi);							//!< Constructs a new net message package (with custom set to 0)
									Package(BYTE flags, UINT16 lo, UINT16 hi, UINT32 transfer_size);	//!< Constructs a new net transfer package (with custom set to transfer_size)
				void*				data();																	//!< Returns pointer to the first byte of the package data (behind the header)
				void				setTransferSize(UINT32 size);											//!< Updates transfer size (updates \b custom )
				UINT32				getTransferSize()					const;								//!< Retrieves transfer size from the \b custom header field
		};


		class Task		//! Transfer task
		{
		private:
				UINT32			data_size;
				BYTE			*data;
				bool			*package;
				UINT16			completed,packages;
				Time::Stamp		in,*out;
				bool			closed,undef;
		friend	class			TaskList;
		public:
				UINT16			transfer;	//!< Transfer id
				_netReceive		exec;		//!< Transfer handler
				UINT16			channel;	//!< Transfer channel

								Task(UINT16 transfer);
								Task(UINT16 transfer, UINT32 size);
								Task(UINT16 transfer, UINT16 channel, const void*data, UINT32 size);
		virtual					~Task();
				void			Insert(UINT16 package, const void*data, UINT16 size);	//!< Inserts package into the transfer field \param package Index of the package \param data Pointer to the data field \param size Size of the data field in bytes
				void			confirm(UINT16 package);								//!< Confirms that the specified package was received
				bool			finished();																//!< Queries status \return true if the transfer is finished, false otherwise
				void			timeOutSend(Time::Stamp now, BYTE flags, Connection*connection);	//!< Resends all packages that timed out to the specified connection \param now Current system time in microseconds \param flags Package flags to use for resent packages
				void			redef(UINT32 size);														//!< Redefines the local task to match the new size
				TransferItem*	get();																	//!< Returns a pointer to a new TransferItem object if the local transfer is finished or NULL otherwise
		};

		class TaskList:private Queue<Task>	//! Net task list
		{
		public:
				UINT16			first;				//!< Index of the first existing task
				System::Mutex	access;				//!< Access mutex

								TaskList();
				TransferItem*	getFinished(Node*sender, bool serial);		//!< Returns the next finished inbound transfer that is not empty \param sender Transfer origin for debug output \param serial Force serial finishing \return Pointer to a new TransferItem object containing the finished transfer
				void			timeOutClose(Node*sender, Time::Stamp now, bool direct_erase_undef);	//!< Closes timed out inbound transfers \param sender Transfer origin for debug output \param now Current time \param direct_erase_undef Causes the method to erase undefined (implied) transfers
				void			timeOutSend(Time::Stamp now, BYTE flags, Connection*connection);		//!< Resends timed out outbound transfers via the specified connection using the specified flags \param now Current time \param flags Package flags \param connection Out connection
				Task*			append(UINT16 channel, const void*data, UINT32 size);						//!< Appends a new outbound transfer \param channel Transfer channel \param data Pointer to the resource data \param size Size of the resource data in bytes \return Inserted task object
				Task*			locate(Node*sender, UINT16 transfer, UINT32 size);						//!< Checks if the respective transfer exists. Potentially creates new transfers \param sender Transfer origin for debug output \param transfer Transfer ID \param size Transfer size in bytes
				void			confirm(UINT16 transfer, UINT16 package);									//!< Confirms inbound package receive
			
				Queue<Task>::clear;
		};

		class Node:public ThreadObject		//! Root network node
		{
		protected:
				bool						_online,					//!< Specifies whether or not the node is currently active
											_defined,					//!< Specifies whether or not the node is initialized
											_recovering;				//!< Set true if the connection previously died and is currently recovering
				Package					read_buffer;				//!< Inbound message buffer
				Thread						core_thread,				//!< Thread responsible for handling messages
											time_thread;				//!< Thread responsible for sending pings
				Queue<QueueItem>			_queue;						//!< Out queue for events
				bool						_direct;					//!< Set true if events should directly be executed (autonomous connections)
				String						error;						//!< Error description for last occured error


				void			confirm(const NetIdent&target,UINT16 transfer_id,BYTE package_id, bool serial); //!< Sends a confirm message to the specified target
				void			sendMessage(const NetIdent&target,UINT16 message_id);							//!< Sends a message to the specified target
				void			sendPackage(const NetIdent&target, Package*package, UINT32 size);			//!< Sends a package to the specified target
				void			sendPackage(const NetIdent&target, TPackageHead*head);						//!< Sends a package to the specified target
				bool			resetConnection();																//!< Reinstalls connection (on error)
				void			event(_netClientEvent event, const Name&target, UINT32 para);				//!< Enqueues event (or directly executes if running in autonomous mode)
				void			event(_netClientDrop event, Connection*connection, UINT32 para);			//!< Enqueues event (or directly executes if running in autonomous mode)
				void			event(_netEvent event, Connection*connection);								//!< Enqueues event (or directly executes if running in autonomous mode)
				void			event(QueueItem*);																//!< Enqueues event (or directly executes if running in autonomous mode)
				void			trash(Connection*);															//!< Discards a connection
				bool			bindService();							//!< Binds the socket to the port \return true on success
				void			disconnectM();							//!< Shuts the connection down (called by the read thread)
				bool			read();									//!< Internal read method (called by the read thread)	\return true on success, false on disconnect

		virtual bool			channel(const Name&);
		virtual void			read(Package*package, UINT32 size);
		virtual void			fetchDisconnect(Connection*client, UINT16 reason, bool closing);
		virtual void			fetchDisconnect(const Name&name, UINT16 reason, bool closing);
		virtual void			checkTimeouts();

				void			netTimeThread();
				void			ThreadMain();

		friend	class			Connection;
		friend	class			Task;

		public:
				Name		self;									//!< Own address
			
				Mutex					channel_mutex;				//!< Shields modifications on the channel map
				IndexTable<TChannel>	channels;					//!< Open channels
				UINT16			port;									//!< Own port
				SOCKET			node;									//!< Network socket

				_netClientEvent OnRead,									//!< Low level on read event
								OnWrite,								//!< Low level on write event
								OnPing,									//!< Low level on ping event
								OnTick;									//!< Low level on tick event

								Node();
		virtual					~Node();
				void			makeAutonomous();						//!< Enters autonomous mode (events will directly be executed)
				void			makeResolvable();								//!< Enters resolvable mode (events have to be resolved)
				void			openChannel(UINT16 channel, _netReceive exec);	//!< Opens the specified channel and binds it \param channel Channel to open \param exec Channel handler
				void			openChannel(UINT16 channel, _netReceive exec, UINT32 package_size);	//!< Opens the specified channel and binds it \param channel Channel to open \param exec Channel handler \param package_size Any non zero package size will enforce the specified package size. Packages of different size will be ignored.
				void			closeChannel(UINT16 channel);					//!< Closes the specified channel
				bool			channelIsOpen(UINT16 channel);
		virtual void			disconnect();							//!< Shuts the connection down
				bool			online();								//!< Queries status \return true if the service is currently active/connected to a server
				bool			isOnline();								//!< Queries status \return true if the service is currently active/connected to a server
				void			resolve();								//!< Resolves outbound events (must be called by the client application each frame if in resolvable mode)
				const String&	GetError();								//!< Retrieves an error description of the last occured error
		};



		class Connection:public QueueItem		//! Root connection
		{
		private:
				bool			connected;
				Timer::Time		last_recv,
								last_ping;
				Node			*parent;
				TaskList		out_serial, out_parallel, in[4];
				UINT16			non_confirm_serial,non_confirm_id;
			
			struct
			{
				BYTE			sent,
								received;
				Timer::Time		time[NET_SYNC_STEPS];
				bool			busy;
				__int64			delta_sum;
				int				last_delta;
				double			delta_change;
				Timer::Time		last_action,
								last_sync;
				Mutex			mutex;
			}					sync;
			bool				synchronized_;


		friend	class			Task;



		protected:
				void			sendSyncRequest(const Timer::Time&);
				void			initSync(const Timer::Time&);
				void			closeSync();
				void			checkSync(const Timer::Time&);

		public:
				Name			ident;					//!< Target address
				Reference<ConnectionAttachment>	attachment;	//!< Custom data hook per connection (client)
				UINT32			ping;					//!< Message delay in micro seconds

								Connection(Node*parent);
								Connection(const Name&name, Node*parent);

				void			reset(const Name&name);					//!< Resets connection and assigns a new address (for connection reuse)
				void			checkTimeouts(Timer::Time now);			//!< Checks for message timeouts
				void			handlePackage(Package*package, UINT32 size);	//!< Invokes package handling
				void			message(UINT16 channel, BYTE flags=NET_CONFIRM|NET_SERIAL);			//!< Sends an empty package \param channel Target channel to send on \param flags Package flags
				void			write(UINT16 channel, BYTE flags, const void*data, UINT32 len);		//!< Starts a new transfer \param channel Target channel to send on \param flags Package flags \param data Pointer to the first byte of the transfer data \param len Length of transfer data in bytes
		TMPL	void			send(UINT16 channel, BYTE flags, const C*data, UINT32 elements=1);	//!< Starts a new transfer \param channel Target channel to send on \param flags Package flags \param data Pointer to the first element of the transfer data \param elements Length of transfer data in elements
		TMPL	void			send(UINT16 channel, BYTE flags, const C&data);							//!< Starts a new transfer \param channel Target channel to send on \param flags Package flags \param data Reference to the transfer data
		TMPL	void			sendArray(UINT16 channel, BYTE flags, const Ctr::Array<C>&data);			//!< Starts a new transfer \param channel Target channel to send on \param flags Package flags \param data Const reference to a data array
				void			write(UINT16 channel, const void*data, UINT32 len);		//!< Starts a new transfer. Implies NET_CONFIRM|NET_SERIAL \param channel Target channel to send on \param data Pointer to the first byte of the transfer data \param len Length of transfer data in bytes
		TMPL	void			send(UINT16 channel, const C*data, UINT32 elements=1);	//!< Starts a new transfer. Implies NET_CONFIRM|NET_SERIAL \param channel Target channel to send on \param data Pointer to the first element of the transfer data \param elements Length of transfer data in elements
		TMPL	void			send(UINT16 channel, const C&data);							//!< Starts a new transfer. Implies NET_CONFIRM|NET_SERIAL \param channel Target channel to send on \param data Reference to the transfer data
				void			sendString(UINT16 channel, const String&string);		//!< Starts a new transfer. Implies NET_CONFIRM|NET_SERIAL \param channel Target channel to send on \param string String object to send the data of. The sent string data will not include the trailing zero.
		TMPL	void			sendArray(UINT16 channel, const Ctr::Array<C>&data);			//!< Starts a new transfer. Implies NET_CONFIRM|NET_SERIAL  \param channel Target channel to send on \param data Const reference to a data array
				UINT32			getTimeStamp();											//!< Queries connection time \return Nettime in micro seconds
				int				getDelta(const Timer::Time&now, bool secure=true);
				Node*			getParent();


		};

		class Server:private Sorted<List::M::Vector3<Connection>,IdentSort>,public Node	//! Server
		{
		private:
				Name			incoming_name;
				Connection		*incoming_target;
				Mutex			list_access;


		virtual void			fetchDisconnect(Connection*connection, UINT16 reason, bool closing);
		virtual void			fetchDisconnect(const Name&name, UINT16 reason, bool closing);
		virtual bool			channel(const Name&);
		virtual void			read(Package*package, UINT32 size);
		virtual void			checkTimeouts();

		public:
				_netEvent		OnClientConnect;		//!< Evoked if a new client connects
				_netClientEvent OnServerCrash;			//!< Evoked if the server suffers a fatal socket crash
				_netClientDrop	OnClientDisconnect;		//!< Evoked if a client disconnects
				_netAccept		AcceptClient;			//!< Queried if a new client attempts connection (internal thread domain)


								Server();
				bool			startService();			//!< Binds socket and starts threads to handle incoming client requests \return true if the service could be started
				void			disconnect();			//!< Stops the service
			
				count_t			clients();						//!< Counts connected clients
				Connection* 	getClient(UINT32 index);		//!< Retrieves the specified client (mutex protected) \param index Index of the requested client (0 = first client) \return Pointer to the respective connection or NULL if \b index is invalid
				Connection* 	findClient(const Name&name);	//!< Finds a connection via the respective address (mutex protected) \param name IP Address of the respective client \return Pointer to the connection or NULL if the respective connection could not be located
			
				void			lockClients();						//!< Locks client mutex for direct client access
				Connection* 	getDirect(UINT32 index);			//!< Retrieves the specified client (direct - no mutex protection) \param index Index of the requested client (0 = first client) \return Pointer to the respective connection or NULL if \b index is invalid
				void			releaseClients();					//!< Releases client mutex

				void			message(UINT16 channel, BYTE flags=NET_CONFIRM|NET_SERIAL);				//!< Sends an empty package to \b all clients (mutex protected) \param channel Target channel to send on \param flags Package flags
				void			write(UINT16 channel, BYTE flags, const void*data, UINT32 size);		//!< Starts a new transfer to \b all clients (mutex protected) \param channel Target channel to send on \param flags Package flags \param data Pointer to the first byte of the transfer data \param len Length of transfer data in bytes
		TMPL	void			send(UINT16 channel, BYTE flags, const C*data, UINT32 elements=1);		//!< Starts a new transfer to \b all clients (mutex protected) \param channel Target channel to send on \param flags Package flags \param data Pointer to the first element of the transfer data \param elements Length of transfer data in elements
		TMPL	void			send(UINT16 channel, BYTE flags, const C&data);							//!< Starts a new transfer to \b all clients (mutex protected) \param channel Target channel to send on \param flags Package flags \param data Reference to the transfer data
		TMPL	void			sendArray(UINT16 channel, BYTE flags, const Ctr::Array<C>&data);			//!< Starts a new transfer to \b all clients (mutex protected) \param channel Target channel to send on \param flags Package flags \param data Reference to the transfer data
				void			write(UINT16 channel, const void*data, UINT32 size);					//!< Starts a new transfer to \b all clients (mutex protected). Implies NET_CONFIRM|NET_SERIAL \param channel Target channel to send on \param data Pointer to the first byte of the transfer data \param len Length of transfer data in bytes
		TMPL	void			send(UINT16 channel, const C*data, UINT32 elements=1);					//!< Starts a new transfer to \b all clients (mutex protected). Implies NET_CONFIRM|NET_SERIAL \param channel Target channel to send on \param data Pointer to the first element of the transfer data \param elements Length of transfer data in elements
		TMPL	void			send(UINT16 channel, const C&data);										//!< Starts a new transfer to \b all clients (mutex protected). Implies NET_CONFIRM|NET_SERIAL \param channel Target channel to send on \param data Reference to the transfer data
				void			sendString(UINT16 channel, const String&string);						//!< Starts a new transfer to \b all clients (mutex protected). Implies NET_CONFIRM|NET_SERIAL \param channel Target channel to send on \param string String object to send the data of. The sent string data will not include the trailing zero.
		TMPL	void			sendArray(UINT16 channel, const Ctr::Array<C>&data);						//!< Starts a new transfer to \b all clients (mutex protected). Implies NET_CONFIRM|NET_SERIAL  \param channel Target channel to send on \param flags Package flags \param data Reference to the transfer data
				bool			disconnectClient(Connection*connection);								//!< Forces disconnection of the specified client (mutex protected). Does not evoke OnClientDisconnect. \return true if the respective client existed
				UINT32			getTimeStamp();															//!< Queries connection time \return Nettime in micro seconds
		};


		class Client:public Connection, public Node //! Client
		{
		private:
				Timer::Time	requested;
				bool			requesting;
			
		virtual void			fetchDisconnect(const Name&name, UINT16 reason, bool closing);
		virtual void			fetchDisconnect(Connection*, UINT16 reason, bool closing);
		virtual bool			channel(const Name&);
		virtual void			read(Package*package, UINT32 size);
		virtual void			checkTimeouts();

		public:
				_netEvent		OnConnect;						//!< Evoked if the connection to a server could be established
				_netClientDrop	OnDisconnect,					//!< Evoked if the connection to a server was lost
								OnConnectFail;					//!< Evoked if the connection attempt to a server failed

								Client();
				bool			connectTo(const char*server, UINT16 port=0);	//!< Connects to a server via the specified port \param server Address of the server to connect to (IP address or DNS name) \param port Port number to use or 0 for default port \return true if a connection could be attempted
				bool			connectTo(const Name&name);					//!< Connects to a resolved remote address \return true if a connection could be attempted
				void			disconnect();									//!< Disconnects from the server
		};

	#endif /*0*/
	}


	#include "node.tpl.h"
}

#endif
