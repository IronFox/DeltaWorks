#ifndef engine_displayH
#define engine_displayH

/******************************************************************

Engine Display

******************************************************************/

#include <global_root.h>
#include "renderer/renderer.h"
#include "interface/keyboard.h"
#include "interface/mouse.h"
#include "interface/joystick.h"
#include "interface/screen.h"
#include "timing.h"
#include "aspect.h"
#include "textout.h"
#include <math/resolution.h>

#if SYSTEM==WINDOWS

#elif SYSTEM_VARIANCE==LINUX
	#include "../X11/displayConnection.h"
	#include <X11/Xlib.h>
	#include <X11/Xutil.h>
	#include <X11/Xos.h>
	typedef int DEVMODE;
#endif

#include <io/log.h>

/*!
	\brief Game engine modules

	The Engine namespace consists of modules directly relating to graphical realtime applications.
*/
namespace Engine
{

	typedef std::function<void()>	FCallback;


#if SYSTEM==WINDOWS

#elif SYSTEM==UNIX
	void				ExecuteEvent(XEvent&event);
#endif


	void logOperations(LogFile*);

	#define ENGINE_LOG(msg)	{if (Engine::log_file) (*Engine::log_file)<<__FILE__<<"("<<(int)__LINE__<<") <"<<__func__<<">: "<<String(msg)<<nl; }


	typedef bool (*pEngineExec)();


	template <class> class  Display;
	class				   Context;


	#define ERR_NO_ERROR					0
	#define ERR_CLASS_REGISTRATION_FAILED   1
	#define ERR_WINDOW_CREATION_FAILED	 	2
	#define ERR_X_CONNECTION_FAILED			3

	class DisplayConfig
	{
	public:
		enum flag_t
		{
			ResizeDragHasEnded = 0x1,
			IsFullscreen = 0x2,
			IsMaximized = 0x4,
			IsMinimzed = 0x8
		};


		typedef std::function<void(const Resolution&, UINT32)>	FOnResize;
		typedef void (*f_on_resize)(const Resolution&newRes, UINT32 flags);

		enum border_style_t
		{
			NoBorder,
			FixedBorder,
			ResizableBorder
		};


		struct Icon
		{
			String		fileName;
			int			idiResource = -1;

			/**/		Icon()	{}
			/**/		Icon(int idiResource): idiResource(idiResource)	{}
		};


		String			window_name;
		Icon			icon;
		border_style_t	border_style;
		FOnResize		onResize;



		DisplayConfig():border_style(FixedBorder)
		{}
		DisplayConfig(border_style_t border_style_):border_style(border_style_)
		{}
		DisplayConfig(const char*window_name_):window_name(window_name_),border_style(FixedBorder)
		{}
		DisplayConfig(const String&window_name_):window_name(window_name_),border_style(FixedBorder)
		{}
		DisplayConfig(const String&window_name_, bool hide_border_):window_name(window_name_),border_style(hide_border_ ? NoBorder : FixedBorder)
		{}
		DisplayConfig(border_style_t style, const FOnResize&resize):border_style(style),onResize(resize)
		{}
		DisplayConfig(const char*window_name_, const FOnResize&resize):window_name(window_name_),border_style(ResizableBorder),onResize(resize)
		{}
		DisplayConfig(const String&window_name_, const FOnResize&resize):window_name(window_name_),border_style(ResizableBorder),onResize(resize)
		{}
		DisplayConfig(const String&window_name_, border_style_t style, const FOnResize&resize):window_name(window_name_),border_style(style),onResize(resize)
		{}
		DisplayConfig(border_style_t style, f_on_resize resize):border_style(style),onResize(resize)
		{}
		DisplayConfig(const char*window_name_, f_on_resize resize):window_name(window_name_),border_style(ResizableBorder),onResize(resize)
		{}
		DisplayConfig(const String&window_name_, f_on_resize resize):window_name(window_name_),border_style(ResizableBorder),onResize(resize)
		{}
		DisplayConfig(const String&window_name_, border_style_t style, f_on_resize resize):window_name(window_name_),border_style(style),onResize(resize)
		{}

	};

	typedef std::function<bool(const Array<String>&)>	DragEventHandler;
	typedef std::function<void(const Array<String>&)>	DropEventHandler;

	/*!
		\brief	Display root interface

		The class requires a visual interface (i.e. OpenGL) as template parameter

	*/

	template <class GL> class Display:public GL
	{
	private:
			pEngineExec					exec_target;
			bool						context_error,
										region_locked,
										depth_test_locked,
										framebuffer_bound,
										resolution_overridden;
			Resolution					target_buffer_resolution,
										overridden_client_resolution;
			bool						framebuffer_alpha;
			M::TVec3<float>				cameraLocation;
	static	SimpleGeometry				pivot;
	static  bool						pivot_defined;

	#if SYSTEM==WINDOWS
	inline  void						Process(const MSG&msg);
	#elif SYSTEM==UNIX
	inline  void						Process(XEvent&event);
	#endif
			RECT						Transform(const M::TFloatRect&rect);
			RECT						Transform(const M::TFloatRect&rect, const Resolution&res);

	public:

		#undef FORWARD
		/*!
		  \def FORWARD
		  Methods that use this simply foward to the underlieing context class
		*/
		#define FORWARD static inline

		TVisualConfig				config;			 	//!< startup configuration

		/**/						Display();
		/*!
		\brief Creates the eve interface
		\return true on success

		create() creates a window and binds the GL rendering context to the window.
		Successive calls to create() will have no effect until the Display context
		has been destroyed via close(). The window and rendering context will
		be destroyed automatically if the Display object instance is deleted.
		*/
		inline	bool				Create(const DisplayConfig&config = DisplayConfig());	//!< @overload
		FORWARD bool				HideCursor();   				//!< Hides the mouse cursor while above the active window. A created context is required. \return true if cursor is hidden
		FORWARD void				ShowCursor();   				//!< Shows cursor again if it has previously been hidden.
		/*!
		\brief Changes the location and size of the associated window
		\param left New left coordinate of the window in pixels (relative to the left screen border)
		\param top New top coordinate of the window in pixels (relative to the top screen border - facing downwards)
		\param width New window width in pixels
		\param height New window height in pixels

		localWindow() redefines the location and size of the active window. If no window is active then
		the specified window location will be applied during the next create() call.
		Depending on the underlying system, the window size either specifies the size
		of the inner client area or the windoe including its border (if any).
		*/
		inline  void				PositionWindow(unsigned left, unsigned top, unsigned width, unsigned height);
		inline  void				PositionWindow(const RECT&rect);	//!< Identical to the above localeWindow() using a RECT struct.
		/*!
		\brief Changes the size of the associated window
		\param width New window width in pixels
		\param height New window height in pixels

		resizeWindow() redefines the size of the active window. If no window is active then
		the specified window dimension will be applied during the next create() call.
		*/
		inline  void				ResizeWindow(unsigned width, unsigned height);
		inline	void				SetSize(unsigned width, unsigned height);	//!< @copydoc resizeWindow()
		FORWARD	void				SetBorderStyle(DisplayConfig::border_style_t style);
		FORWARD DisplayConfig::border_style_t	GetBorderStyle();

		void						SignalWindowResize(UINT32 displayConfigFlags);

		FORWARD const RECT&			GetPreMaxiMinimizeLocation();
		FORWARD const Resolution&	GetPreMaxiMinimizeSize();

		FORWARD const RECT&			GetWindowLocation();	//!< Retrieves the current window location and size \return RECT Struct containing the current window location.
		FORWARD	unsigned			GetWidth();			//!< Queries the current window width
		FORWARD	unsigned			GetHeight();				//!< Queries the current window height
		inline	UINT				GetClientWidth()			const;	//!< Queries the current window's inner width
		inline	UINT				GetClientHeight()			const;	//!< Queries the current window's inner height
		inline	float				GetPixelAspect()	const;	   //!< window pixel-aspect
		FORWARD	Resolution			GetSize();							//!< Queries the current window size
		inline	Resolution			GetClientSize()			const;	//!< Queries the current window's inner size
		inline	Resolution			GetTargetResolution()				const;
		FORWARD	Resolution			GetDimension();					//!< Identical to size()
		FORWARD	Resolution			GetDimensions();					//!< Identical to size()
		FORWARD void				QueryScreen(ResolutionList*r_list, FrequencyList*f_list=NULL, DWORD w_min=0, DWORD h_min=0, DWORD f_min=0);
		FORWARD	Resolution			GetScreenSize();
		RECT						TransformViewport(const M::TFloatRect&rect, const Resolution&clientSize);
		inline  void				SetScreen(const TDisplayMode&mode);
		FORWARD short				GetRefreshRate();
		FORWARD bool				IsMaximized();
		FORWARD void				Maximize();
		FORWARD bool				IsMinimized();
		FORWARD void				Minimize();
		FORWARD void				Restore();
		FORWARD void				RegisterFocusCallbacks(const FCallback&onFocusLost, const FCallback&onFocusRestored);
		#if SYSTEM==WINDOWS
			FORWARD bool			GetScreen(DEVMODE&mode);
			FORWARD bool			GetScreen(DEVMODE*mode);
			FORWARD bool			IsCurrent(const DEVMODE&screen);
			/**
			Signals that file drops are allowed and specified handler functions to check whether or not a set of files is accepted, and what to do if they are actually dropped.
			@param onDrag Function to determine whether or not a group of files being dragged atop the window is accepted for dropping. The function must either accept all files, or none.
			The function may be empty in which case all files are accepted.
			@param onDrop Function to be called if a set of accepted files are dropped on the window. The function is not executed if @a onDrag returned false on them.
			*/
			FORWARD void			AcceptFileDrop(const DragEventHandler&onDrag, const DropEventHandler&onDrop);
			/**
			Signals that file drops are allowed. Use this method to quickly en/disable file dropping without altering the handler functions.
			When first called use AcceptFileDrop(const DragEventHandler&, const DropEventHandler&) instead,
			to define handler functions.
			If no handler functions are specified, no files are accepted since they have nowhere to go.
			*/
			FORWARD void			AcceptFileDrop();
			/**
			Signals that file drops are disallowed. Does not affect the defined handler functions. Until AcceptFileDrop() is called again, all file drops are rejected globally,
			and no handler functions are executed.
			*/
			FORWARD void			BlockFileDrop();

		#elif SYSTEM==UNIX
			FORWARD int							FindScreen(DWORD width, DWORD height, DWORD&refresh_rate);
			/*FORWARD const XRRScreenSize&		GetScreen(unsigned index);
			FORWARD bool						GetScreen(XRRScreenSize*size);
			FORWARD bool						GetScreen(XRRScreenSize&size);
			FORWARD bool						IsCurrent(const XRRScreenSize&size);*/
		#endif

		void						RenderSomething();
		inline	void				RenderPivot();		//!< Identical to RenderSomething()
		inline	void				RenderToken();		//!< Identical to RenderSomething()

		inline  bool				ApplyScreen();
		inline  bool				ApplyWindowScreen(DWORD refresh_rate=0);
		inline  void				ResetScreen();
		inline	void				Capture(FloatImage&target);
		inline	void				Capture(Image&target);
		inline	void				Capture(typename GL::Texture&target);
		inline	void				CaptureDepth(typename GL::Texture&target);

		void						InterruptCheckEvents();
		void						Assign(pEngineExec target);		//!< Assigns a new loop-function \param target Callback function of the form bool func(). The function may return false causing Execute() to return
		void						Execute();					  	//!< Executes the assigned loop-function
		void						ExecuteNoClr();				 //!< Executes the assigned loop-function without clearing the screen per loop
		void						Destroy();					//!< Terminates engine execution
		void						Terminate();				//!< \deprecated {use close() instead}
		String						GetErrorStr();					 //!< Returns a string representation of the last occured error. \return String representation of the last occured error

		void						LockRegion();					   //!< Prevent subsequent Pick(...) or PickCentered(...) calls from affecting the render region
		void						UnlockRegion();					 //!< Allow subsequent Pick(...) or PickCentered(...) calls to affect the render region
		void						PickRegion(const M::TFloatRect&rect);   //!< Pick rendering region \param rect New render region
		template <class C>
			void					Pick(const Aspect<C>&aspect);	//!< Picks aspect (i.e. a camera) for rendering \param aspect Aspect that should be used from now on
		template <class C>
			void					PickCentered(const Aspect<C>&aspect);				//!< Picks aspect (i.e. a camera) as if it were located in the point of origin \param aspect Aspect that should be used from now on
		/**
		Returns the last picked camera's location. If picked centered, returns 0-vector
		*/
		const M::TVec3<>&			GetCameraLocation() const {return cameraLocation;}
		bool						TargetFBO(const typename GL::FBO&pobj);	 //!< Binds a pixel buffer object for rendering \param pobj Pixel buffer object that should be rendered to
		void						TargetBackbuffer();										//!< Unbinds bound pixel buffer object
		void						OverrideSetClientResolution(const Resolution&res);
	};


	#ifdef _DEBUG
		#define DISPLAY_NO_MOUSE_HOOKS
	#endif

	/**
	Singleton non-template base of display. All window operations are actually executed by the context, and not the template display helper
	*/
	class Context
	{
	public:
		#if SYSTEM==WINDOWS
			typedef LRESULT (*EventHook)(HWND window, UINT Msg, WPARAM wParam,LPARAM lParam);
		#endif
	private:
			pEngineExec			exec_target;
	#if SYSTEM==WINDOWS

			EventHook			eventHook;
			HINSTANCE			hInstance;
			HWND				hWnd;
	#ifndef DISPLAY_NO_MOUSE_HOOKS
		static	HHOOK			hHook;
		static	unsigned		hook_counter;
	#endif
		bool					class_created;

		FCallback				onRestoreFocus,onLoseFocus;

		static  LRESULT CALLBACK	WndProc(HWND hWnd, UINT Msg, WPARAM wParam,LPARAM lParam);

	#ifndef DISPLAY_NO_MOUSE_HOOKS
		static 	LRESULT CALLBACK 	mouseHook(int nCode, WPARAM wParam,   LPARAM lParam);
		static	void				installHook();
		static	void				uninstallHook(bool force=false);
	#else
		static	void				installHook()	{};
		static	void				uninstallHook(bool force=false)	{};

	#endif

	#elif SYSTEM==UNIX
			::Display			*display;
			Window				window; //is long unsigned int
			int					screen;
			Colormap			colormap;   //is long unsigned int
			TWindowAttributes	window_attributes;
			bool				function_key_mask[0x100];

	friend  void				ExecuteEvent(XEvent&event);

	#endif

		void					looseFocus();
		void					restoreFocus();
		UINT32					ApplyWindowPosition();
		void					eventClose();

		static UINT32			displayConfigFlags;



	protected:
		int								_error;
		DWORD							_trate;
		RECT					 		_location,
										client_area;
		DisplayConfig::border_style_t	border_style;
		DisplayConfig::FOnResize		onResize;
	public:
		bool					shutting_down;

		/**/					Context();

		template <typename T>
			void				GetAbsoluteClientRegion(M::Rect<T>&rect)		const
			{
				rect.x.min = client_area.left;
				rect.y.min = client_area.top;
				rect.x.max = client_area.right;
				rect.y.max = client_area.bottom;
			}


		UINT32					GetDisplayConfigFlags() const;
			
		void					RegisterFocusCallbacks(const FCallback&onFocusLost, const FCallback&onFocusRestored)
		{
			onLoseFocus = onFocusLost;
			onRestoreFocus = onFocusRestored;
		}

	#if SYSTEM==WINDOWS
			void				RegisterEventHook(EventHook hook)	{eventHook = hook;}
			HWND				window()	const;
			HWND				getWindow()	const {return window();}
			HWND				createWindow(const String&window_name, DisplayConfig::border_style_t border_style, const DisplayConfig::FOnResize&onResize, const DisplayConfig::Icon&icon);
			HWND 				createChildWindow(HWND parent, bool enabled);
			void				setWindow(HWND);
			StringW				createClass();
			void				destroyClass();

			bool				CheckFullscreen(const WINDOWINFO&) const;
	#elif SYSTEM==UNIX
			::Display*			connect();
			::Display*			connection();
			void				disconnect();
			Window				createWindow(const String&window_name, const TWindowAttributes&attributes, DisplayConfig::border_style_t border_style, const DisplayConfig::FOnResize&onResize, const DisplayConfig::Icon&icon);
			void				setWindow(Window);
	#endif
			DisplayConfig::border_style_t	GetBorderStyle()		const	{return border_style;}
			void				SetBorderStyle(DisplayConfig::border_style_t style);

			bool				hideCursor();
			void				showCursor();
			UINT32				ResizeWindow(unsigned width, unsigned height);
			UINT32				PositionWindow(unsigned left, unsigned top, unsigned width, unsigned height);
			UINT32				PositionWindow(const RECT&dimensions);
			const RECT&			GetWindowLocation()						const;
			const RECT&			windowClientArea()						const;
			Resolution			GetWindowSize()							const;
			Resolution			GetClientSize()							const;
			UINT				GetClientWidth()							const;
			UINT				GetClientHeight()							const;
			RECT				Transform(const M::TFloatRect&field)		const;
			static RECT			Transform(const M::TFloatRect&field, const Resolution&);
			void				destroyWindow();

			void				SignalResize(UINT32 flags);

			bool				isTopWindow()							const;
			void				focus();
			bool				IsFocused() const {return isFocused();}
			bool				isFocused()								const;
			bool				isMinimized()							const;


			static UINT32		GetLastSignalledDisplayFlags();

			/**
			Replaces the on window resize callback function
			@param newCallback Callback to replace the existing callback function
			@return previous (replaced) callback function
			*/
			DisplayConfig::FOnResize	ReplaceOnResize(const DisplayConfig::FOnResize&newCallback);

			/**
			Checks whether or not the managed window exists and is currently the top-most
			*/
			bool				WindowIsTop()							const	{return isTopWindow();}
			/**
			Checks whether or not the managed window exists and is currently minimized
			*/
			bool				WindowIsMinimized()						const	{return isMinimized();}
			/**
			Checks whether ot not the mangaged window exists and is currently maximized
			*/
			bool				WindowIsMaximized()						const;

			void				MaximizeWindow();
			void				MinimizeWindow();
			void				RestoreWindow();


			void				checkFocus();	//sends isFocused() to eveMouse.setFocused()

			float				pixelAspectf()							const;
			double				pixelAspectd()							const;
			float				windowAspectf()							const;
			double				windowAspectd()							const;

			unsigned			countScreenModes();
			short				GetRefreshRate();
	#if SYSTEM==WINDOWS
			
			void				AcceptFileDrop(const DragEventHandler&, const DropEventHandler&);
			void				AcceptFileDrop();
			void				BlockFileDrop();
			Resolution			GetScreenSize()							const;


			DEVMODE*			GetScreen(unsigned index)				const;
			bool				IsCurrent(const DEVMODE&screen)			const;
			bool				GetScreen(DEVMODE*mode)					const;
			bool				GetScreen(DEVMODE&mode)					const;
	#elif SYSTEM==UNIX

			Resolution			GetScreenSize();
			int							FindScreen(DWORD width, DWORD height, DWORD&refresh_rate);
	#endif


		void						SetScreen(DEVMODE&screen);
		void						SetScreen(const TDisplayMode&mode);
		void						SetScreen(DEVMODE*screen);
		bool						ApplyScreen();
		bool						ApplyWindowScreen(DWORD refresh_rate=0);
		void						ResetScreen();

		void						QueryScreen(ResolutionList*r_list, FrequencyList*f_list=NULL, DWORD w_min=0, DWORD h_min=0, DWORD f_min=0);

		void						Assign(pEngineExec target);
		void						Execute();
		void						handleEvents();
		void						close();

			

		unsigned				errorCode();
		const char*				errorStr();

		const RECT&				GetPreMaxiMinimizeLocation()	{return _restoredLocation;}
		const Resolution&		GetPreMaxiMinimizeSize()	{return _restoredClientSize;}
	private:
		RECT				_restoredLocation;
		Resolution			_restoredClientSize;
	};


	/*!


		\fn void Display::QueryScreen(ResolutionList*r_list, FrequencyList*f_list=NULL, DWORD w_min=0, DWORD h_min=0, DWORD f_min=0)
		\brief Queries possible screen resolutions
		\param r_list Pointer to output resolution list
		\param f_list Pointer to output frequency list or NULL
		\param w_min  Minimum screen width (for resolution to be listed)
		\param h_min  Minimum screen height (for resolution to be listed)
		\param f_min  Minimum refresh rate (for resolution to be listed)

		QueryScreen() queries the main screen for all possible resolutions and writes them to \a r_list. If \a f_list is not \a NULL then all possible
		refresh rates are listed separatly to \a f_list. \a w_min, \a h_min and \a f_min can be used to exclude smaller screen resolutions.


		\fn void Display::SetScreen(const DisplayMode&mode)
		\brief Sets the active screen resolution target
		\param mode Const reference to the target screen mode

		The method merely sets the resolution target. To change the resolution to the specified target call \a ApplyScreen()


		\fn short Display::GetRefreshRate()
		\brief Returns the current screen refresh rate
		\return Current screen refresh rate


		\fn void Display::RenderSomething()
		\brief Renders a 1x1x1 colored non-textured token to the point of origin

		RenderSomething() (renderPivot()/renderToken()) can be very useful when setting up a new application where cameras may be misplaced.
		The rendering process of this token is fairly rubust and can cope with most aversive rendering setups.

		\fn bool Display::ApplyScreen()
		\brief Applies the target screen resolution to the screen
		\return true on success

		This method applies the target screen resolution (set by \a SetScreen(...) ) to the main screen.

		\fn bool  Display::ApplyWindowScreen(DWORD refresh_rate=0)
		\brief Uses the active window dimensions as screen resolution
		\param refresh_rate Target refresh rate. 0 implies highest possible.
		\return true on success

		\fn void  Display::ResetScreen()
		\brief Restores the native screen resolution


	*/



	extern Context				  	context;
	extern LogFile					*log_file;

}


#include "display.tpl.h"
#endif
