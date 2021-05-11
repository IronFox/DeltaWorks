#include "../global_root.h"
#include "monitoredProcess.h"
#include "../io/file_system.h"
#include "../container/buffer.h"

#ifdef WIN32

#include <cstdio>
#include <windows.h>
#include <tlhelp32.h>
#include <winternl.h>
#include "../container/hashtable.h"


//#include <iostream>

namespace DeltaWorks
{
	namespace MonitorDetails
	{


		//https://github.com/apache/reef/blob/master/lang/cs/Org.Apache.REEF.Bridge/BinaryUtil.cpp
		//Apache License
		static DWORD GetActualAddressFromRVA(IMAGE_SECTION_HEADER* pSectionHeader, IMAGE_NT_HEADERS* pNTHeaders, DWORD dwRVA)
		{
			DWORD dwRet = 0;

			for (int j = 0; j < pNTHeaders->FileHeader.NumberOfSections; j++, pSectionHeader++)
			{
				DWORD cbMaxOnDisk = std::min( pSectionHeader->Misc.VirtualSize, pSectionHeader->SizeOfRawData );

				DWORD startSectRVA, endSectRVA;

				startSectRVA = pSectionHeader->VirtualAddress;
				endSectRVA = startSectRVA + cbMaxOnDisk;

				if ( (dwRVA >= startSectRVA) && (dwRVA < endSectRVA))
				{
					dwRet =  (pSectionHeader->PointerToRawData ) + (dwRVA - startSectRVA);
					break;
				}

			}

			return dwRet;
		}



		static bool _IsManaged(const PathString&path)
		{
			bool bIsManaged = false;    //variable that indicates whether
										//managed or not.
 
			HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ,
										FILE_SHARE_READ,NULL,OPEN_EXISTING,
										FILE_ATTRIBUTE_NORMAL,NULL);
 
			//attempt the standard paths (Windows dir and system dir) if
			//CreateFile failed in the first place.
			if(INVALID_HANDLE_VALUE == hFile)
				throw Except::IO::DriveAccess::FileOpenFault(CLOCATION,"Unable to open "+FileSystem::PathToString(path));

 
			//succeeded
			HANDLE hOpenFileMapping = CreateFileMappingW(hFile,NULL,
														PAGE_READONLY,0,
														0,NULL);
			if(hOpenFileMapping)
			{
				BYTE* lpBaseAddress = NULL;
 
				//Map the file, so it can be simply be acted on as a
				//contiguous array of bytes
				lpBaseAddress = (BYTE*)MapViewOfFile(hOpenFileMapping,
													FILE_MAP_READ,0,0,0);
 
				if(lpBaseAddress)
				{
					//having mapped the executable, now start navigating
					//through the sections
 
					//DOS header is straightforward. It is the topmost
					//structure in the PE file
					//i.e. the one at the lowest offset into the file
					IMAGE_DOS_HEADER* pDOSHeader =
						(IMAGE_DOS_HEADER*)lpBaseAddress;
 
					//the only important data in the DOS header is the
					//e_lfanew
					//the e_lfanew points to the offset of the beginning
					//of NT Headers data
					IMAGE_NT_HEADERS* pNTHeaders =
						(IMAGE_NT_HEADERS*)((BYTE*)pDOSHeader +
						pDOSHeader->e_lfanew);
 
					//store the section header for future use. This will
					//later be need to check to see if metadata lies within
					//the area as indicated by the section headers
					IMAGE_SECTION_HEADER* pSectionHeader =
						(IMAGE_SECTION_HEADER*)((BYTE*)pNTHeaders +
						sizeof(IMAGE_NT_HEADERS));
 
					//Now, start parsing
					//First of all check if it is a PE file. All assemblies
					//are PE files.
					if(pNTHeaders->Signature == IMAGE_NT_SIGNATURE)
					{
						//start parsing COM table (this is what points to
						//the metadata and other information)
						DWORD dwNETHeaderTableLocation =
							pNTHeaders->OptionalHeader.DataDirectory
							[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].
							VirtualAddress;
 
						if(dwNETHeaderTableLocation)
						{
							//.NET header data does exist for this module;
							//find its location in one of the sections
							IMAGE_COR20_HEADER* pNETHeader =
								(IMAGE_COR20_HEADER*)((BYTE*)pDOSHeader +
								GetActualAddressFromRVA(pSectionHeader,
								pNTHeaders,dwNETHeaderTableLocation));
 
							if(pNETHeader)
							{
								//valid address obtained. Suffice it to say,
								//this is good enough to identify this as a
								//valid managed component
								bIsManaged = true;
							}
						}
					}
					else
					{
					}
					//cleanup
					UnmapViewOfFile(lpBaseAddress);
				}
				//cleanup
				CloseHandle(hOpenFileMapping);
			}
			//cleanup
			CloseHandle(hFile);
			return bIsManaged;
		}



		HashTable<PathString,bool>	managedMap;
		Sys::Mutex	managedMutex;

		static bool IsManaged(const PathString&path)
		{
			managedMutex.lock();
			try
			{
				bool*v = managedMap.QueryPointer(path);
				if (v)
				{
					bool rs = *v;
					managedMutex.unlock();
					return rs;
				}
				bool managed = _IsManaged(path);
				managedMap.Set(path,managed);
				managedMutex.unlock();
				return managed;
			}
			catch (...)
			{
				managedMutex.unlock();
				throw;
			}
		}






		static DWORD		MapPathToProcess(const FileSystem::File&file)
		{
			FileSystem::Folder folder;
			PROCESSENTRY32 entry;
			entry.dwSize = sizeof(PROCESSENTRY32);

			HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

			//StringW out;

			if (Process32First(snapshot, &entry) == TRUE)
			{
				do
				{
					if (file.GetName().EqualsIgnoreCase(entry.szExeFile))
					{
						MODULEENTRY32 mentry;
						mentry.dwSize = sizeof(MODULEENTRY32);
						HANDLE msnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,entry.th32ProcessID);
						if (Module32First(msnapshot,&mentry) == TRUE)
						{
							FileSystem::File mfile;
							if (folder.FindFile(mentry.szExePath,mfile))
							{
								if (mfile.GetLocation().EqualsIgnoreCase(file.GetLocation()))
								{
									CloseHandle(msnapshot);
									CloseHandle(snapshot);
									return mentry.th32ProcessID;
								}
							}
							else
								ShowMessage(FileSystem::PathToString(mentry.szExePath));
						}
						CloseHandle(msnapshot);
					}
					//out += entry.szExeFile;
					//out += L'\n';
					//std::wcout << entry.szExeFile<<std::endl;
				}
				while  (Process32Next(snapshot, &entry) == TRUE);
			}
			CloseHandle(snapshot);
			//ShowMessage("not found: "+String(file.GetName()));
			//ShowMessage(String(out));
			return NULL;
		}


		struct TEnumParameter
		{
			DWORD	process;
			Container::Vector0<HWND>	*outWindows;
		};

		static BOOL CALLBACK Enumerate(HWND hWnd, LPARAM lParam)
		{
			TEnumParameter*p = (TEnumParameter*)lParam;
			DWORD process = 0x0;
			GetWindowThreadProcessId( hWnd, &process );
			if (p->process == process)
			{
				p->outWindows->Append(hWnd);
			}
			return TRUE;
		}

		static void EnumerateWindows(DWORD process, Container::Vector0<HWND>&outWindows)
		{
			TEnumParameter p;
			p.process = process;
			p.outWindows = &outWindows;
			EnumWindows( Enumerate, (LPARAM)&p );
		}

	}

	void		MonitoredProcess::QuitThread()
	{
		quitThread = true;
		if (!application_shutting_down)
			AwaitCompletion();
	}


	bool		MonitoredProcess::TryResume(const PathString&workingDirectory, const PathString&executablePath)
	{
		FileSystem::Folder folder;
		FileSystem::File file;
		if (!folder.FindFile(executablePath,file))
		{
			//DBG_FATAL__("Unable to find executable "+String(executablePath));
			return false;
		}
		DWORD process = MonitorDetails::MapPathToProcess(file);
		if (process == 0)
			return false;

		HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process);
		if (processHandle == INVALID_HANDLE_VALUE)
			return false;
		Terminate();
		Flush();
		infoOut.dwProcessId = process;
		infoOut.hProcess = processHandle;
		paths.executablePath = file.GetLocation();
		paths.workingDirectory = file.GetFolder();
		this->createWindow = HasAnyVisibleWindows();
		terminateOnQuit = false;
		quitThread = false;
		ThreadObject::Start();

		return true;
	}



	void		MonitoredProcess::ResumeOrStart(const PathString&workingDirectory, const PathString&executablePath, const PathString&parametersWithoutExecutableName, bool createWindow/*=true*/)
	{
		ResumeOrStart(workingDirectory,executablePath,[parametersWithoutExecutableName](){return parametersWithoutExecutableName;},createWindow);
	}
	void		MonitoredProcess::ResumeOrStart(const PathString&workingDirectory, const PathString&executablePath, const std::function<PathString()>&parametersWithoutExecutableName, bool createWindow/*=true*/)
	{
		if (TryResume(workingDirectory,executablePath))
			return;
		Start(workingDirectory,executablePath,parametersWithoutExecutableName, createWindow);
	}


	void		MonitoredProcess::Start(const PathString&_workingDirectory, const PathString&_executablePath, const PathString&parametersWithoutExecutableName, bool createWindow)
	{
		Start(_workingDirectory,_executablePath,[parametersWithoutExecutableName](){return parametersWithoutExecutableName;},createWindow);
	}


	void		MonitoredProcess::Start(const PathString&_workingDirectory, const PathString&_executablePath, const std::function<PathString()>&parametersWithoutExecutableName, bool createWindow)
	{
		Terminate();
		FileSystem::Folder f(_workingDirectory);
		if (!f.IsValidLocation())
			throw Except::IO::DriveAccess("Process Start: Chosen working directory '"+FileSystem::PathToString(_workingDirectory)+"' is invalid");
		FileSystem::File found;
		if (!f.FindFile(_executablePath,found,false))
			throw Except::IO::DriveAccess("Process Start: Chosen executable '"+FileSystem::PathToString(_executablePath)+"' does not exist");
		if (!found.DoesExist())
			throw Except::IO::DriveAccess("Process Start: Chosen executable '"+FileSystem::PathToString(found.GetLocation())+"' does not exist");
		this->asyncFetchParameters = parametersWithoutExecutableName;
		executableName = ((PathString::char_t)'"')+FileSystem::ExtractFileNameExt(_executablePath)+'\"';
		paths.executablePath = found.GetLocation();
		paths.workingDirectory = f.GetLocation();


		this->createWindow = createWindow;
		Restart();
	}

	PathString		MonitoredProcess::CompileFullExecutable()const
	{
		if (!asyncFetchParameters)
			return executableName;
		return executableName+' '+asyncFetchParameters();
	}



	void	MonitoredProcess::Restart()
	{
		ASSERT__(!IsActive());
		
		faultLock.lock();
		lastFault = "";
		faultLock.unlock();

		quitThread = false;
		terminateOnQuit = false;

		ThreadObject::Start();
	}

	bool			MonitoredProcess::IsFaulted() const
	{
		return lastFault.IsNotEmpty();
	}

	String			MonitoredProcess::GetLastFault() const
	{
		String copy;
		faultLock.lock();
		copy = lastFault;
		faultLock.unlock();
		return copy;
	}

	void			MonitoredProcess::ThreadedStartProcess(const Paths&paths)
	{
		if (infoOut.dwProcessId == 0)
		{
			STARTUPINFOW	infoIn;
			//PROCESS_INFORMATION	infoOut;
			ZeroMemory(&infoIn,sizeof(infoIn));
			infoIn.cb = sizeof(infoIn);

			if (!createWindow)
			{
				infoIn.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
				infoIn.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
				infoIn.hStdOutput =  GetStdHandle(STD_OUTPUT_HANDLE);
				infoIn.hStdError = GetStdHandle(STD_ERROR_HANDLE);
				infoIn.wShowWindow = SW_HIDE;
			}

			//infoIn.wShowWindow = SW_MINIMIZE;
			//infoIn.dwFlags |= STARTF_USESHOWWINDOW;	//dx11 doesn't start well minimized

			infoIn.dwXSize = 100;
			infoIn.dwYSize = 100;
			//infoIn.dwFlags |= STARTF_USESIZE;

			DWORD flags = 0;
			#ifdef _DEBUG
				//flags |= DEBUG_PROCESS;
			#endif
			flags |= CREATE_NEW_CONSOLE;
			if (!createWindow)
				flags |= CREATE_NO_WINDOW;

			auto parameters = this->CompileFullExecutable();
			BOOL rs = CreateProcessW(paths.executablePath.c_str(),parameters.mutablePointer(),NULL,NULL,FALSE,flags,NULL,paths.workingDirectory.c_str(),&infoIn,&infoOut);
			if (rs == FALSE)
				throw Except::IO::ParameterFault("Process Start: Windows refused to start process '"+FileSystem::PathToString(paths.executablePath)+"': "+System::GetLastErrorString());
		}
	}

	/*static*/ void		MonitoredProcess::Sanitize(Paths&local)
	{
		if (MonitorDetails::IsManaged(local.executablePath))
		{
			/*
			There is some ongoing issue with .NET programs and UNC paths.
			
			Setting the executable working directory to an UNC path will correctly start the application,
			but any retrieval of the active working directory by that application will throw an exception.
			Unfortunately, many IO constructors will do so implicitly even if unnecessary.
			See https://blogs.msdn.microsoft.com/bclteam/2007/02/13/long-paths-in-net-part-1-of-3-kim-hamilton/ for reference.
			Apparently, this is never going to be 'fixed'.

			Therefor, at the risk of paths ending up too long, we strip any found UNC markers before executing
			if the application is detected to be managed.
			We could test string length and potentially throw an exception, but recent changes in Windows 10
			imply that the 260 character limitation could be entirely removed eventually.
			E.g. https://blogs.msdn.microsoft.com/jeremykuhne/2016/07/30/net-4-6-2-and-long-paths-on-windows-10/
			*/
			const PathString uncMarker = StringType::Cast("\\\\?\\");
			if (local.executablePath.BeginsWith(uncMarker))
				local.executablePath.EraseLeft(uncMarker.GetLength());
			if (local.workingDirectory.BeginsWith(uncMarker))
				local.workingDirectory.EraseLeft(uncMarker.GetLength());
		}
	}



	/*virtual override*/ void	MonitoredProcess::ThreadMain()
	{
		try
		{
			Paths local = paths;
			Sanitize(local);


			ThreadedStartProcess(local);

			while (!quitThread)
			{
				bool running = (GetExitCodeProcess(infoOut.hProcess,&lastExitCode)!=0 && lastExitCode == STILL_RUNNING);
				if (!running)
				{
					Flush();
					if (autoRestart)
						ThreadedStartProcess(local);
					else
					{
						if (onEndAsync)
							onEndAsync();
						return; //already flushed, no point terminating, no exception
					}
				}
				Sleep(1000);
			}
			if (terminateOnQuit)
			{
				Container::Vector0<HWND>	windows;
				MonitorDetails::EnumerateWindows(infoOut.dwProcessId,windows);
				foreach (windows,w)
				{
					//DestroyWindow(*w);
					//WINDOWINFO info;
					//info.cbSize = sizeof(info);
					//GetWindowInfo(*w,&info);
					//wchar_t str[0x100];
					//memset(str,0,sizeof(str));
					//GetWindowText(*w,str,sizeof(str)-1);
					//
					//DWORD threadID = GetWindowThreadProcessId(*w,NULL);
					//if (threadID == 0)
						//continue;
					PostMessage(*w,WM_CLOSE,0,0);
					//PostThreadMessageA(threadID,WM_CLOSE,0,0);
				}
				count_t numTries = 0;
				while (WaitForSingleObject(infoOut.hProcess,2000) != 0)
				{
					if (++numTries > 3)
						throw Except::Program::GeneralFault(CLOCATION,"Unable to terminate monitored process");
					TerminateProcess(infoOut.hProcess,0);
				}
			}

		}
		catch (const std::exception&ex)
		{
			faultLock.lock();
			lastFault = ex.what();
			faultLock.unlock();
		}
		Flush();
	}


	bool		MonitoredProcess::IsRunning()	const
	{
		return IsActive();
		//	return false;
		//DWORD code;
		//return (GetExitCodeProcess(infoOut.hProcess,&code)!=0 &&  code == STILL_RUNNING);
	}

	void	MonitoredProcess::ForceRestart()
	{
		Terminate();
		Restart();
	}


	void MonitoredProcess::Flush()
	{
		CloseHandle(infoOut.hProcess);
		CloseHandle(infoOut.hThread);
		infoOut.hProcess = INVALID_HANDLE_VALUE;
		infoOut.hThread = INVALID_HANDLE_VALUE;
		infoOut.dwThreadId = 0;
		infoOut.dwProcessId = 0;
	}

	void		MonitoredProcess::Terminate()
	{
		terminateOnQuit = true;
		EndObservation();
	}

	void		MonitoredProcess::EndObservation()
	{
		QuitThread();
	}

	void		MonitoredProcess::GetWindows(Array<HWND>&outWindows) const
	{
		const DWORD pid = infoOut.dwProcessId;
		if (pid)
		{
			Vector0<HWND>	windows;
			MonitorDetails::EnumerateWindows(pid,windows);
			windows.CopyToArray(outWindows);
		}
		else
			outWindows.SetSize(0);
	}


	bool		MonitoredProcess::HasAnyVisibleWindows() const
	{
		const DWORD pid = infoOut.dwProcessId;
		if (pid)
		{
			Container::Vector0<HWND>	windows;
			MonitorDetails::EnumerateWindows(pid,windows);
			foreach (windows,w)
			{
				if (IsWindowVisible(*w))
					return true;
			}
		}
		return false;
	}
}


#endif

