#ifndef systemH
#define systemH

/*****************************************************************************

Classes for platform-independent handling of platform-specific system aspects.
Supported aspects are Pipes, and Shared-Libraries.

*****************************************************************************/

#include "../global_root.h"
#include "../container/array.h"
#include <stdlib.h>

#pragma comment(lib,"psapi.lib")


#if SYSTEM == WINDOWS
	#include <winbase.h>
	#include <Psapi.h>
#elif SYSTEM == UNIX
	#include <unistd.h>
	#include <pthread.h>
	#include <sys/sem.h>
	#include <sys/ipc.h>
	#include <sys/types.h>
	#include <dlfcn.h>
	#include <string.h>
	#include <fcntl.h>
	#include <stdarg.h>
	#include <signal.h>
	#include <sched.h>
	#include <sys/ioctl.h>
	//#include <signal.h>
	#include <termios.h>
	#include <sys/time.h>
	#include <sys/resource.h>

	#ifndef SEM_R
		#define SEM_A		0200	/* alter permission */
		#define SEM_R	   0400	/* read permission */	
	#endif
#endif

namespace DeltaWorks
{


	namespace System //! Translation namespace for common system tasks
	{

	


		class Pipe //! System independent (non blocking) pipe handler
		{
		private:
			#if SYSTEM==WINDOWS
				HANDLE	  		read_handle,write_handle;
			#elif SYSTEM==UNIX
				int		 		handle[2];
			#endif

		public:
								Pipe(DWORD size=0);						//!< Constructs a new pipe. \param size Pipe size in bytes. Only applicable when using windows. Set 0 for default.
		virtual					~Pipe();

				bool			write(const void*data, size_t bytes);	 //!< Writes data to the pipe \param data Pointer to the data to read from \param bytes Number of bytes to write to the pipe
				bool			read(void*target, size_t bytes);		  //!< Reads data from the stream \param target Pointer to the data to write to \param bytes Number of bytes to read from the pipe
			template <class C>
				bool			write(const C&data)						 //!< Writes one unit of POD data \param data Reference to the object to read from
								{
									return write(&data,sizeof(C));
								}
			template <class C>
				Pipe&	  		operator<<(const C&data)					//!< Streaming operator: Writes one unit of POD data \param data Reference to the object to read from
								{
									write(&data,sizeof(C));
									return *this;
								}
			template <class C>
				bool			read(C&data)								//!< Reads one unit of POD data \param data Reference to the object to write to \return true on success
								{
									return read(&data,sizeof(C));
								}
			template <class C>
				bool			operator>>(C&data)						  //!< Streaming operator: Reads one unit of POD data \param data Reference to the object to write to \return true on success
								{
									return read(&data,sizeof(C));
								}
		};

		class BlockingPipe //! System independent (blocking) pipe handler
		{
		protected:
			#if SYSTEM==WINDOWS
				HANDLE			read_handle,write_handle;
			#elif SYSTEM==UNIX
				int				handle[2];
			#endif

								BlockingPipe(bool);
		public:
			static const size_t	BadCall = (size_t)-1;	//!< Returned by PeekReadBytes() if something is wrong with the pipe

								BlockingPipe(DWORD size=0);			   //!< Constructs a new (blocking) pipe. \param size Pipe size in bytes. Only applicable when using windows. Set 0 for default.
		virtual					~BlockingPipe();
			#if SYSTEM==WINDOWS
				HANDLE			writeHandle()	const	{return write_handle;}
				size_t			PeekReadBytes(void *target, size_t bytes);
				size_t			GetFillLevel() const;
			#elif SYSTEM==UNIX
				int				writeHandle()	const	{return handle[1];}
			#endif


				bool			Write(const void*data, size_t bytes)	{return write(data,bytes);}

				bool			write(const void*data, size_t bytes);	  //!< Writes data to the pipe \param data Pointer to the data to read from \param bytes Number of bytes to write to the pipe
				/*!
					\brief Reads data from the stream
					\param target Pointer to the data to write to
					\param bytes Number of bytes to read from the pipe
				
					The function blocks until the specified number of bytes were read from the pipe
					or an error occured.
				*/
				bool			read(void*target, size_t bytes);
			template <class C>
				bool			write(const C&data)						 //!< Writes one unit of POD data \param data Reference to the object to read from
								{
									return write(&data,sizeof(C));
								}
			template <class C>
				bool			writeArray(const Ctr::ArrayRef<C>&data)						 //!< Writes one unit of POD data \param data Reference to the object to read from
								{
									return write(data.pointer(),data.GetContentSize());
								}
							
			template <class C>
				BlockingPipe&	operator<<(const C&data)				  //!< Streaming operator: Writes one unit of POD data \param data Reference to the object to read from
								{
									write(&data,sizeof(C));
									return *this;
								}
				/*!
					\brief Reads one unit of POD data
					\param data Reference to the data to write to
					\return true on success

					The function blocks until the entire object was read from the pipe
					or an error occured.
				*/
			template <class C>
				bool			read(C&data)
								{
									return read(&data,sizeof(C));
								}
			template <class C>
				C				read()
								{
									C rval;
									read(&rval,sizeof(C));
									return rval;
								}
			template <class C>
				bool			readArray(Ctr::ArrayRef<C>&data)
								{
									return read(data.pointer(),data.GetContentSize());
								}
				/*!
					\brief Streaming operator: Reads one unit of POD data
					\param data Reference to the data to write to
					\return true on success

					The function blocks until the entire object was read from the pipe
					or an error occured.
				*/
			template <class C>
				bool			operator>>(C&data)
								{
									return read(&data,sizeof(C));
								}
			void				Close();
		};


		class PipeFeed
		{
		public:
			#if SYSTEM==WINDOWS
				typedef HANDLE	handle_t;
			#elif SYSTEM==UNIX
				typedef int		handle_t;
				#define INVALID_HANDLE_VALUE -1
			#endif
		private:
			handle_t			write_handle;
		public:
			PipeFeed()			:write_handle(INVALID_HANDLE_VALUE)	{};

			void				Set(handle_t handle)	{write_handle = handle;}
			bool				IsSet()	const	{return write_handle != INVALID_HANDLE_VALUE;}
			bool				write(const void*data, size_t bytes);	  //!< Writes data to the pipe \param data Pointer to the data to read from \param bytes Number of bytes to write to the pipe
			template <class C>
			bool				write(const C&data)						 //!< Writes one unit of POD data \param data Reference to the object to read from
								{
									return write(&data,sizeof(C));
								}
			template <class C>
			bool				writeArray(const Ctr::ArrayRef<C>&data)						 //!< Writes one unit of POD data \param data Reference to the object to read from
								{
									return write(data.pointer(),data.GetContentSize());
								}
							
			template <class C>
			PipeFeed&			operator<<(const C&data)				  //!< Streaming operator: Writes one unit of POD data \param data Reference to the object to read from
								{
									write(&data,sizeof(C));
									return *this;
								}

		};
	
		class NamedPipeClient:public BlockingPipe //! System independent (blocking) named pipe handler
		{
		public:
			/**/				NamedPipeClient();			   			//!< Constructs a new (blocking) named pipe client. The pipe is not automatically connected.
	
			bool				ConnectTo(const char*pipe_name, unsigned timeout=500);
	
			bool				IsActive()			const;					//!< Queries the current status of the named pipe
			bool				IsConnected()		const;					//!< Queries the current status of the named pipe
		};
		class NamedPipeServer:public BlockingPipe //! System independent (blocking) named pipe handler
		{
		private:
			DWORD				timeout;
		public:
			/**/				NamedPipeServer();			   			//!< Constructs a new (blocking) named pipe client. The pipe is not automatically connected.
	
			bool				Start(const char*pipe_name, DWORD timeoutMS);
			bool				AcceptClient();								//!< Checks and connects any pending connection request. Blocks until a client has connected	
			bool				IsActive()			const;					//!< Queries the current status of the named pipe
			bool				IsConnected()		const;					//!< Queries the current status of the named pipe
		};

		const char*				getFirstEnv(const char**field, unsigned len, const char*except);	//!< Returns the first non-NULL environment variable or \b except if no such could be found \param field Field containing key-names to check for \param len Number of entries in the name field \param except String to return if no matching key was found \return First non-NULL result of getenv(...) or \b except if none was found
	


		/**
		Queries the number of logical processors available on the local system 
		@return number of logical processors
		*/
		count_t					QueryLogicalProcessorsCount();
		inline count_t			getProcessorCount()		/**@copydoc QueryLogicalProcessorsCount()*/ {return QueryLogicalProcessorsCount();}
		const char*				getSystemName();		//!< Queries the name of the currently used operating system \return System name or 'unknown' if the name was not determinable
		const char*				getUserName();			//!< Queries the name of the currently logged in user \return User name or 'unknown' if the name was not determinable
		const char*				getUserDomain();		//!< Queries the domain, the currently logged in user is member of \return User domain name or '' if the domain was not determinable
		const char*				getUserHomeFolder();	//!< Queries the home folder of the currently logged in user \return User home folder or '' if the home folder was not determinable
		const char*				getTempFolder();		//!< Queries the system temp folder \return System temp folder
		const char*				getHostName();			//!< Queries the resolved address of the local computer or at least its name \return computer addr/name

		size_t					getPhysicalMemory();	//!< Queries the total available physical memory (in bytes)
		size_t					getMemoryConsumption();	//!< Memory currently consumed by this application (in bytes)
	
		bool					copyToClipboard(HWND window, const char*line);
		bool					GetFromClipboardIfText(HWND window, char*buffer, size_t buffer_size);
		count_t					GetClipboardTextLength(HWND window);
	
		int						getConsoleWidth();		//!< Retrieves the visual width of the console in characters

		/**
		Effective only on windows, and on Windows versions where QuickEdit is enabled by default.
		This function disables QuickEdit on the currently active console (if any).

		QuickEdit can freeze the application if the user decides to click anywhere in the window.
		Unfortunately, it can freeze some console applications permanently.
		The standard mark feature does the same, but is better hidden.
		*/
		void					DisableConsoleQuickEdit();
	
		#if SYSTEM==WINDOWS
			inline void sleep(DWORD milsec)   {::Sleep(milsec);}
			const char*			getLastError();
			inline const char*	GetLastErrorString()	{return getLastError();}
			/**
			Formats a windows error code to string
			*/
			void				WindowsErrorToString(DWORD lastError, char*outMsg, size_t outSize);


		#elif SYSTEM==LINUX
			inline void sleep(unsigned milsec)   {usleep(milsec*1000);}
		#endif
	
	

	//	void					setEnv(const String&name, const String&value);



	}


	namespace Sys = System;
}

#endif
