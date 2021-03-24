#ifndef systemMonitoredProcessH
#define systemMonitoredProcessH

#ifdef WIN32

#include "../general/timer.h"
#include "../global_string.h"
#include "../general/thread_system.h"
#include <functional>

namespace DeltaWorks
{

	/**
	Generalized process supervisor.
	Allows the creation of arbitrary process, and potentially automatic restarting in case they fail.
	A separate thread is localy created to monitor the running process.
	The process is NOT terminated upon object destruction, but all observation will come to an end (obviously)
	*/
	class MonitoredProcess : private Sys::ThreadObject
	{
	public:
		struct Paths
		{
			PathString	executablePath,
						workingDirectory;
		};
	private:
		PROCESS_INFORMATION	infoOut;
		Time::Stamp		lastCheck;

		Paths			paths;
		PathString		executableName;
		std::function<PathString()>	asyncFetchParameters;

		bool			createWindow = true;

		volatile bool	quitThread=false,terminateOnQuit=false,autoRestart;
		
		mutable Sys::SpinLock	faultLock;
		String			lastFault;
		DWORD			lastExitCode=0;

		PathString		CompileFullExecutable()const;


		void			Restart();
		void			Flush();
		void			QuitThread();


		virtual void	ThreadMain() override;
		void			ThreadedStartProcess(const Paths&);
	public:

		/**
		Called if the monitored process has ended, and the local object is not configured to automatically restart.
		Note that the function is called asynchronously from a separate thread
		*/
		std::function<void()>	onEndAsync;

		/**
		Constructs a new process monitor.
		@param autoRestart If set, any started or resumed process will be automatically restarted if found to have ended due to whatever cause
		*/
		/**/			MonitoredProcess(bool autoRestart):autoRestart(autoRestart)	{}

		virtual			~MonitoredProcess() {QuitThread();}

		/**
		Updates whether any monitored process should automatically be restarted.
		Affects any subsequent time, a process monitored by this object is detected to have ended.
		No reset anywhere else
		*/
		void			SetAutoRestart(bool autoRestart) {this->autoRestart = autoRestart;}
		bool			DoesAutoRestart() const {return autoRestart;}

		/**
		Terminates any running process and starts the local process anew using the specified configuration.
		If auto-restart has been configured, then the started process will be kept alive
		@param workingDirectory The folder that the process should run in
		@param executablePath Path to the executable to be executed, including file name
		@param parametersWithoutExecutableName Additional parameters. May be empty
		@param createWindow If false, any window/console created by the started process will not appear (Windows feature), otherwise operates as normal
		*/
		void			Start(const PathString&workingDirectory, const PathString&executablePath, const PathString&parametersWithoutExecutableName, bool createWindow=true);
		/**
		@copydoc Start();
		@param asyncGetParametersWithoutExecutableName Function to retrieve additional parameters from. May be empty. Called by a separate thread, therefor must be thread safe.
		*/
		void			Start(const PathString&workingDirectory, const PathString&executablePath, const std::function<PathString()>&asyncGetParametersWithoutExecutableName, bool createWindow=true);

		/**
		Checks whether a process with the specified executablePath is already running. Parameters cannot be checked reliably, so they are ignored.
		If a process is found, then the found process is copied to the local configuration.
		@return True if a running process was found, false otherwise
		*/
		bool			TryResume(const PathString&workingDirectory, const PathString&executablePath);
		/**
		First checks whether a process with the specified executablePath is already running. Parameters cannot be checked reliably, so it is assumed they match.
		If a process is found, then the found process is copied to the local configuration.
		Otherwise a new process is started with the given parameters
		@param workingDirectory The folder that the process should run in
		@param executablePath Path to the executable to be executed, including file name
		@param parametersWithoutExecutableName Additional parameters. May be empty
		@param createWindow If false, any window/console created by the started process will not appear (Windows feature), otherwise operates as normal
		*/
		void			ResumeOrStart(const PathString&workingDirectory, const PathString&executablePath, const PathString&parametersWithoutExecutableName, bool createWindow=true);
		/**
		@copydoc ResumeOrStart()
		@param asyncGetParametersWithoutExecutableName Function to retrieve additional parameters from. May be empty. Called by a separate thread, therefor must be thread safe.
		*/
		void			ResumeOrStart(const PathString&workingDirectory, const PathString&executablePath, const std::function<PathString()>&asyncGetParametersWithoutExecutableName, bool createWindow=true);
		/**
		Terminates the process if currently active
		*/
		void			Terminate();
		/**
		Ends any running observation thread without terminating the monitored application
		*/
		void			EndObservation();
		/**
		Terminates and restarts the monitored process
		*/
		void			ForceRestart();
		bool			IsRunning() const;
		bool			IsFaulted() const;
		String			GetLastFault() const;
		bool			HasAnyVisibleWindows() const;
		/**
		Gets all windows associated with the monitored process
		@param[out] outWindows Container to hold all encountered windows. Emptied if no process is currently monitored
		*/
		void			GetWindows(Array<HWND>&outWindows) const;
		DWORD			GetLastExitCode() const {return lastExitCode;}

		Sys::ThreadObject::AwaitCompletion;


		/**
		Adjusts paths if the target is found to point to a managed application and the path contains UNC markers
		*/
		static void		Sanitize(Paths&paths);
	};
}


#endif






#endif
