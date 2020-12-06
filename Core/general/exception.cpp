#include "../global_root.h"
#include "exception.h"
#include "thread_system.h"
#ifdef __CUDACC__
	#include <cstdlib>
#endif

namespace DeltaWorks
{
	namespace Except
	{
		static void defaultFatal(const FatalDescriptor&discriptor)
		{
			DISPLAY__(discriptor.ToString());
		}

		void (*onFatal)(const FatalDescriptor&exception) = defaultFatal;


		static System::RecursiveMutex	fatal_mutex;
		//static volatile	bool	fatal_occured = false;


		void	TerminateApplication()
		{
			#ifdef __CUDACC__
				exit(-1);
			#else
				#ifdef _DEBUG
					std::abort();
				#else
					#ifdef WIN32
						if (IsDebuggerPresent())
							DebugBreak();
						TerminateProcess(GetCurrentProcess(),0);	//self-terminate
					#else
						exit(-1);
					#endif
				#endif
			#endif
			throw Except::Fatal(CLOCATION,"terminate");
				/*
			#elif SYSTEM == WINDOWS
				TerminateProcess(GetCurrentProcess(), 0);
			#elif SYSTEM==UNIX
				raise(SIGKILL);
			#else
				exit(-1);
			#endif*/
		}

		void	EndFatal()
		{
			fatal_mutex.Release();
		}

		void	BeginFatal()
		{
			static volatile bool doBreak = true;
			#ifdef _DEBUG
				if (IsDebuggerPresent() && doBreak)
				{
					doBreak = false;
					DebugBreak();
				}
			#endif

			fatal_mutex.Lock();
		}


		void	HolyShit(const char*message)
		{
			static System::Mutex	mutex;

			mutex.lock();	//no unlock. display message. end program. now.

				if (message != NULL)
					DISPLAY__(message);
			
				TerminateApplication();

		}



	}




	void		FatalDescriptor::Copy(const char*message, size_t length)	throw()
	{
		if (length > sizeof(this->message)-1)
			length = sizeof(this->message)-1;
		memcpy(this->message,message,length);
		this->message[length] = 0;
		Reformat();
	}

	
	void			TCodeLocation::Format(TCodeLocation::TOutString&rs)	const	throw()
	{
		//std::cout << "determining code location for "<<file<<"/"<<method<<" ["<<line<<"]"<<std::endl;
		const char	*in = file,
					*last = file;

		TOutString	filename;
		FormatFilename(filename);
						
		sprintf_s(rs.buffer,sizeof(rs.buffer),"line %i of %s::%s()",line,filename.begin,method);
		rs.begin = rs.buffer;
	};

	void		FatalDescriptor::Reformat() throw()
	{
		TCodeLocation::TOutString filename;
		TCodeLocation::FormatFilename(filename);
		sprintf_s(formatted,"Fatal exception in line %i of %s::%s\n\"%s\"",TCodeLocation::line,filename.begin,TCodeLocation::method,message);
	}


	void			TCodeLocation::FormatFilename(TOutString&rs)	const	throw()
	{
		const char	*in = file,
					*last = file;

		char		*out = rs.buffer;
							
						
		while (*in)
		{
			if (*in == '/' || *in == '\\')
			{
				if (in-last == 2)
				{
					if (last[0] == '.' && last[1] == '.' && out-rs.buffer>=2)
					{
						out -= 2;
						while (out > rs.buffer && *out != '/' && *out != '\\')
							out--;
					}
				}
				last = in+1;
			}
			if (out-rs.buffer >= sizeof(rs.buffer)-2)
				break;
			(*out++) = (*in++);
		}
		(*out++) = 0;
		unsigned cnt = 0;
		while (out > rs.buffer && cnt < 2)
		{
			out--;
			if (*out == '/' || *out == '\\')
				cnt++;
		}
		if (cnt == 2)
			out++;
		rs.begin = out;
	};
}
