#include "../global_root.h"
#include "system.h"



/*
Calling a DLL with C# (C Sharp)
		The following example demonstrates how to call a (C++) DLL from C Sharp. The code for the DLL is here. 
call_dll.cs
// http://www.c-sharpcenter.com/Tutorial/UnManaged.htm


using System.Runtime.InteropServices;
using System;

class call_dll {

  [StructLayout(LayoutKind.Sequential, Pack=1)]
  private struct STRUCT_DLL {
    public Int32  count_int;
    public IntPtr ints;
  }

  [DllImport("mingw_dll.dll")]
  private static extern int func_dll(
      int an_int,
      [MarshalAs(UnmanagedType.LPArray)] byte[] string_filled_in_dll,
      ref STRUCT_DLL s
   );
  
  public static void Main() {

    byte[] string_filled_in_dll = new byte[21];


    STRUCT_DLL struct_dll = new STRUCT_DLL();
    struct_dll.count_int = 5;
    int[]  ia = new int[5];
    ia[0] = 2; ia[1] = 3; ia[2] = 5; ia[3] = 8; ia[4] = 13;

    GCHandle gch    = GCHandle.Alloc(ia);
    struct_dll.ints = Marshal.UnsafeAddrOfPinnedArrayElement(ia, 0);

    int ret=func_dll(5,string_filled_in_dll, ref struct_dll);
    
    Console.WriteLine("Return Value: " + ret);
    Console.WriteLine("String filled in DLL: " + System.Text.Encoding.ASCII.GetString(string_filled_in_dll));

  }
}
*/

namespace DeltaWorks
{

	namespace System
	{




		
		#if SYSTEM==WINDOWS

			void	WindowsErrorToString(DWORD lastError, char*outMsg, size_t outSize)
			{
				LPVOID lpMsgBuf;
				FormatMessageA(
					FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					lastError,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(char*) &lpMsgBuf,
					0,
					NULL
					);
				size_t len = strlen((char*)lpMsgBuf);
				if (len >= outSize)
					len = outSize-1;
				memcpy(outMsg,lpMsgBuf,len);
				outMsg[len] = 0;
				LocalFree( lpMsgBuf );
			}
			const char*			getLastError()
			{
				static char		error_buffer[512];
				WindowsErrorToString(GetLastError(),error_buffer,sizeof(error_buffer));
				return error_buffer;
			}
		#endif


		Pipe::Pipe(DWORD size)
		{
			#if SYSTEM==WINDOWS
				if (!CreatePipe(&read_handle,&write_handle,NULL,size))
					FATAL__("pipe-creation failed!");
			#elif SYSTEM==UNIX
				if (pipe(handle))
					FATAL__("pipe-creation failed!");
				fcntl(handle[0], F_SETFL, O_NONBLOCK); //O_NOBLOCK)
			#endif
		}

		Pipe::~Pipe()
		{
			#if SYSTEM==WINDOWS
				CloseHandle(read_handle);
				CloseHandle(write_handle);
			#elif SYSTEM==UNIX
				close(handle[0]);
				close(handle[1]);
			#endif
		}

		bool Pipe::write(const void*data, size_t bytes)
		{
			#if SYSTEM==WINDOWS
				DWORD written;
				return WriteFile(write_handle,data,(DWORD)bytes,&written,NULL) && (size_t)written == bytes;
			#elif SYSTEM==UNIX
			   return (:: write(handle[1],data,bytes) == bytes);
			#endif
		}

		bool    Pipe::read(void*target, size_t bytes)
		{
			#if SYSTEM==WINDOWS
				DWORD size;
				if (!PeekNamedPipe(read_handle,NULL,0,NULL,&size,NULL) || size < bytes)
					return false;
				ReadFile(read_handle,target,(DWORD)bytes,&size,NULL);
				return true;
			#elif SYSTEM==UNIX
				return (::read(handle[0],target,bytes) == bytes);
			#endif

		}

		BlockingPipe::BlockingPipe(DWORD size)
		{
			#if SYSTEM==WINDOWS
				if (!CreatePipe(&read_handle,&write_handle,NULL,size))
					FATAL__("pipe-creation failed!");
			#elif SYSTEM==UNIX
				if (pipe(handle))
					FATAL__("pipe-creation failed!");
				//fcntl(handle[0], F_SETFL, O_BLOCK); //O_NOBLOCK)
			#endif
		}
	
		BlockingPipe::BlockingPipe(bool)
		{
			#if SYSTEM==WINDOWS
				read_handle = write_handle = 0;
			#elif SYSTEM==UNIX
				handle[0] = 0;
				handle[1] = 0;
			#endif
		}

		BlockingPipe::~BlockingPipe()
		{
			Close();
		}

		void BlockingPipe::Close()
		{
			#if SYSTEM==WINDOWS
				if (read_handle)
				{
					CloseHandle(read_handle);
				}
				if (write_handle && read_handle != write_handle)
					CloseHandle(write_handle);
				write_handle = NULL;
				read_handle = NULL;
			#elif SYSTEM==UNIX
				if (handle[0])
				{
					close(handle[0]);
				}
				if (handle[1] && handle[0] != handle[1])
					close(handle[1]);
				handle[0] = 0;
				handle[1] = 0;
			#endif
		}

		bool BlockingPipe::write(const void*data, size_t bytes)
		{
			#if SYSTEM==WINDOWS
				DWORD written;
				return WriteFile(write_handle,data,(DWORD)bytes,&written,NULL) && (size_t)written == bytes;
			#elif SYSTEM==UNIX
			   return (:: write(handle[1],data,bytes) == bytes);
			#endif
		}

		#if SYSTEM==WINDOWS
			size_t			BlockingPipe::GetFillLevel() const
			{
				DWORD rs = 0;
				BOOL rc = PeekNamedPipe(read_handle,NULL,0,NULL,&rs,NULL);
				if (!rc)
					return BadCall;
				return rs;
			}

			size_t	BlockingPipe::PeekReadBytes(void *target, size_t bytes)
			{
				DWORD rs = 0;
				BOOL rc = PeekNamedPipe(read_handle,target,(DWORD)bytes,&rs,NULL,NULL);
				if (!rc)
					return BadCall;
				return rs;
			}
		#endif

		bool    BlockingPipe::read(void*target, size_t bytes)
		{
			#if SYSTEM==WINDOWS
				DWORD size;
				return ReadFile(read_handle,target,(DWORD)bytes,&size,NULL) && (size_t)size == bytes;
			#elif SYSTEM==UNIX
				return (::read(handle[0],target,bytes) == bytes);
			#endif
		}

		bool PipeFeed::write(const void*data, size_t bytes)
		{
			#if SYSTEM==WINDOWS
				DWORD written;
				return WriteFile(write_handle,data,(DWORD)bytes,&written,NULL) && (size_t)written == bytes;
			#elif SYSTEM==UNIX
			   return (:: write(write_handle,data,bytes) == bytes);
			#endif
		}


		NamedPipeClient::NamedPipeClient():BlockingPipe(true)
		{
	
		}

	
		bool		NamedPipeClient::ConnectTo(const char*pipe_name, unsigned timeout)
		{
			#if SYSTEM==WINDOWS
				if (!WaitNamedPipeA(pipe_name , timeout))
					return false;
				read_handle = write_handle = CreateFileA(pipe_name , GENERIC_READ|GENERIC_WRITE ,  0 , NULL , OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL, NULL); 
				if (read_handle == INVALID_HANDLE_VALUE)
				{
					return false;
				}
				return true;
			#else
				return false;
			#endif
		}
	
	
		bool		NamedPipeClient::IsActive()			const
		{
			#if SYSTEM==WINDOWS
				return read_handle!=INVALID_HANDLE_VALUE;
			#else
				return false;
			#endif
		}
	
		bool		NamedPipeClient::IsConnected()		const
		{
			return IsActive();
		}
	
		NamedPipeServer::NamedPipeServer():BlockingPipe(true)
		{
	
		}

	
		bool		NamedPipeServer::Start(const char*pipe_name, DWORD timeoutMS)
		{
			Close();
			#if SYSTEM==WINDOWS
				timeout = timeoutMS;
				//read_handle = write_handle = CreateFileA(pipe_name , GENERIC_READ|GENERIC_WRITE ,  FILE_SHARE_WRITE|FILE_SHARE_READ , NULL , CREATE_NEW, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_DELETE_ON_CLOSE, NULL); 
				//read_handle = write_handle = CreateNamedPipeA(pipe_name,PIPE_ACCESS_DUPLEX,PIPE_TYPE_BYTE|PIPE_READMODE_BYTE| PIPE_WAIT|FILE_FLAG_OVERLAPPED,PIPE_UNLIMITED_INSTANCES,1024,1024,500,NULL);
				read_handle = write_handle = CreateNamedPipeA(
				  pipe_name,                    // pipe name
				  PIPE_ACCESS_DUPLEX |      // read/write access
				  FILE_FLAG_OVERLAPPED,
				  PIPE_TYPE_BYTE |              // byte type pipe
				  PIPE_READMODE_BYTE |          // message-read mode
				  PIPE_WAIT,                // blocking mode
				  PIPE_UNLIMITED_INSTANCES,     // max. instances
				  1024,                      // output buffer size
				  1024,                      // input buffer size
				  timeout,         // client time-out
				  NULL);                        // default security attribute
				if (read_handle == INVALID_HANDLE_VALUE)
				{
					return false;
				}
				return true;
			#else
				return false;
			#endif
		}
		bool		NamedPipeServer::AcceptClient()
		{
			#if SYSTEM==WINDOWS
				//https://memset.wordpress.com/2010/10/08/timeout-on-named-pipes/
				OVERLAPPED ol = {0,0,0,0,NULL};
				BOOL ret = 0;
				ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
				ret = ConnectNamedPipe(write_handle, &ol);
				if( ret == 0 )
				{
					switch( GetLastError() ) {
						case ERROR_PIPE_CONNECTED:
							/* client is connected */
							ret = TRUE;
						break;
						case ERROR_IO_PENDING:
							/* if pending i wait PIPE_TIMEOUT_CONNECT ms */
							if( WaitForSingleObject(ol.hEvent, timeout) == WAIT_OBJECT_0 )
							{
								DWORD dwIgnore;
								ret = GetOverlappedResult(write_handle, &ol, &dwIgnore, FALSE);
							}
							else
							{
								CancelIo(write_handle);
							}
						break;
					}
				}
				CloseHandle(ol.hEvent);
				return ret != 0;
	//			return ConnectNamedPipe(read_handle,NULL) != 0;
			#else
				return false;
			#endif	
		}
	
		bool		NamedPipeServer::IsActive()			const
		{
			#if SYSTEM==WINDOWS
				return read_handle!=INVALID_HANDLE_VALUE;
			#else
				return false;
			#endif
		}
	
		bool		NamedPipeServer::IsConnected()		const
		{
			return IsActive();
		}





		const char*				getFirstEnv(const char**field, unsigned len, const char*except)
		{
			for (unsigned i = 0; i < len; i++)
				if (const char*rs = getenv(field[i]))
					return rs;
			return except;
		}

		const char*				getSystemName()
		{
			static const char*	key_names[] = {"OSTYPE","OS"};
			return getFirstEnv(key_names,ARRAYSIZE(key_names),"unknown");
		}

		const char*				getUserName()
		{
			static const char*	key_names[] = {"USER","LOGNAME","USERNAME"};
			return getFirstEnv(key_names,ARRAYSIZE(key_names),"unknown");
		}

		const char*				getUserDomain()
		{
			static const char*	key_names[] = {"GROUP","USERDOMAIN"};
			return getFirstEnv(key_names,ARRAYSIZE(key_names),"");
		}


		const char*				getUserHomeFolder()
		{
			static const char*	key_names[] = {"HOME","PWD","USERPROFILE"};
			return getFirstEnv(key_names,ARRAYSIZE(key_names),"");
		}

		const char*				getTempFolder()
		{
			static const char*	key_names[] = {"TEMP","TMP","TMPDIR"};
			return getFirstEnv(key_names,ARRAYSIZE(key_names),FOLDER_SLASH_STR"tmp");
		}

		const char*				getHostName()
		{
			static const char*	key_names[] = {"HOST","LOGONSERVER","COMPUTERNAME"};	//ok, these are not identical but wtf
			return getFirstEnv(key_names,ARRAYSIZE(key_names),"unknown");
		}


		size_t					getPhysicalMemory()
		{
			#if SYSTEM==WINDOWS
				MEMORYSTATUS status;
				GlobalMemoryStatus(&status);
				return status.dwTotalPhys;
			#elif SYSTEM==LINUX
				//check /proc/meminfo
				FILE*f = fopen("/proc/meminfo","rb");
				unsigned long long kb = 0;
				int rc = fscanf ( f, "MemTotal: %llu kB", &kb );
				fclose(f);
				ASSERT__(rc == 1);
				return kb*1024;
			#else
				#error find some solution
			#endif
		}

		size_t					getMemoryConsumption()
		{
			#if SYSTEM==WINDOWS
				PROCESS_MEMORY_COUNTERS	information;

				if (!GetProcessMemoryInfo(GetCurrentProcess(),&information,sizeof(information)))
					return 0;
				return information.WorkingSetSize;
  			#elif SYSTEM==LINUX
				rusage	usage;
				if (getrusage(RUSAGE_SELF,&usage))
					return 0;
				return usage.ru_ixrss*4096;
			#else
				#error find some solution
			#endif
		}


		count_t					QueryLogicalProcessorsCount()
		{
			#if SYSTEM==LINUX
				//return (unsigned)sysconf(_SC_NPROCESSORS_CONF);
				return (count_t)get_nprocs();
			#elif SYSTEM==WINDOWS
				const char*str = getenv("NUMBER_OF_PROCESSORS");
				if (!str)
					return 1;
				auto rs = strtoul(str,NULL,10);
				if (rs == 0 || rs == ULONG_MAX)
					return 1;
				return (count_t)rs;
			#else
				#error not supported
			#endif
		}

		bool					copyToClipboard(HWND window, const char*line)
		{
			#if SYSTEM==WINDOWS
				if (!OpenClipboard(window))
					return false;
				EmptyClipboard();
				size_t len = strlen(line);
				HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, len+1);
				if (hglbCopy == NULL)
				{
					CloseClipboard();
					return false;
				}


				char*lptstrCopy = (char*)GlobalLock(hglbCopy);
					memcpy(lptstrCopy, line, len+1);
				GlobalUnlock(hglbCopy);
				SetClipboardData(CF_TEXT, hglbCopy);


				CloseClipboard();

				//GlobalFree(hglbCopy);

				return true;

			#elif SYSTEM==LINUX

				return false;

			#else
				#error stub
			#endif
		}

		count_t				GetClipboardTextLength(HWND window)
		{
			#if SYSTEM==WINDOWS
				size_t len;
				if (!IsClipboardFormatAvailable(CF_TEXT))
					return false;
				if (!OpenClipboard(window))
					return false;

				bool success = false;
				HANDLE hglb = GetClipboardData(CF_TEXT);
				if (hglb != NULL)
				{
					len = GlobalSize(hglb);
					success = true;
					/*
					const char*lptstr = (const char*)GlobalLock(hglb);
					if (lptstr)
					{
						len = strlen(lptstr);
						GlobalUnlock(hglb);
						success = true;
					}*/
				}
				CloseClipboard();

				return success ? len : 0;
			#elif SYSTEM==LINUX


				return 0;
			#else
				#error stub
			#endif
		}

		bool				GetFromClipboardIfText(HWND window, char*buffer, size_t buffer_size)
		{
			#if SYSTEM==WINDOWS
				if (!IsClipboardFormatAvailable(CF_TEXT))
					return false;
				if (!OpenClipboard(window))
					return false;

				bool success = false;
				HANDLE hglb = GetClipboardData(CF_TEXT);
				if (hglb != NULL)
				{
					const char*lptstr = (const char*)GlobalLock(hglb);
					if (lptstr)
					{
						size_t len = strlen(lptstr);
						if (len >= buffer_size)
							len = buffer_size-1;
						memcpy(buffer,lptstr,len);
						buffer[len] = 0;
						GlobalUnlock(hglb);
						success = true;
					}
				}
				CloseClipboard();
				return success;
			#elif SYSTEM==LINUX


				return false;
			#else
				#error stub
			#endif
		}

		int getConsoleWidth()
		{
			#if SYSTEM==UNIX
				int fd;
				winsize wsz;

				fd = fileno (stderr);
				if (ioctl (fd, TIOCGWINSZ, &wsz) < 0)
					return 0;			/* most likely ENOTTY */

				return wsz.ws_col;
			#elif SYSTEM==WINDOWS
				CONSOLE_SCREEN_BUFFER_INFO	info;
				if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),&info))
					return 0;
				int w = info.srWindow.Right-info.srWindow.Left;
				if (w > info.dwSize.X)
					w = info.dwSize.X;
				return w;
			#endif
		}

	/*    bool                    setEnv(const String&name, const String&value);
		{
			#if SYSTEM==WINDOWS
				return !_putenv((name+"="+value).c_str());
			#elif SYSTEM==UNIX
				return !putenv(name.c_str(),value.c_str(),1);
			#else
				#error not supported
			#endif
	   }*/

		void DisableConsoleQuickEdit()
		{
			#if SYSTEM==WINDOWS	
				const HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
				if (handle != INVALID_HANDLE_VALUE && handle != NULL)
				{
					DWORD current = 0;
					GetConsoleMode(handle, &current);

					current &= ~ENABLE_QUICK_EDIT_MODE;	//strip flag
					current &= ~ENABLE_INSERT_MODE;
					current |= ENABLE_EXTENDED_FLAGS;	//make sure it's not ignored

					SetConsoleMode(handle, current);
				}
			#endif

		}
   
	}
}
