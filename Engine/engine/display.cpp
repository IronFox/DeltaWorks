#include "display.h"

#include "renderer/opengl.h"
#include "renderer/dummygl.h"

template class Engine::Display < Engine::OpenGL > ;
template class Engine::Display < Engine::DummyGL > ;




#if SYSTEM==WINDOWS

#undef NEAR
#ifndef NEAR
#define NEAR
#endif

//#include <ole2.h> 
//#include <shlobj.h>

bool operator==(const RECT&a, const RECT&b)
{
	return a.left == b.left && a.right == b.right && a.top == b.top && a.bottom == b.bottom;
}


#define WM_OLEDROP WM_USER + 1 

using namespace DeltaWorks;

class CtxDropTarget : public IDropTarget 
{ 
private: 
	unsigned long references; 
	bool acceptFormat; 
	Array<String>	files;

	// helper function 
	static void	DropToFiles(HDROP hDrop, Array<String>&files)
	{
		UINT cnt = DragQueryFile(hDrop,0xFFFFFFFF,NULL,0);
		files.SetSize(cnt);
		if (cnt)
		{
			for (unsigned i = 0; i < cnt; i++)
			{
				unsigned len = DragQueryFile(hDrop,i,NULL,0);
				files[i].Resize(len);
				DragQueryFileA(hDrop,i,files[i].mutablePointer(),len+1);
				if (strlen(files[i].c_str()) != files[i].GetLength())
					FATAL__("File drop read error");
			}
		}
		//DragFinish(hDrop);
	}

	static bool DataToFiles(LPDATAOBJECT obj, Array<String>&files)
	{
		FORMATETC fmtetc; 
		fmtetc.cfFormat = CF_HDROP; 
		fmtetc.ptd = NULL; 
		fmtetc.dwAspect = DVASPECT_CONTENT; 
		fmtetc.lindex = -1; 
		fmtetc.tymed = TYMED_HGLOBAL;

	   // user has dropped on us -- get the CF_HDROP data from drag source 
		STGMEDIUM medium; 
		HRESULT hr = obj->GetData(&fmtetc, &medium);

		if (!FAILED(hr)) 
		{ 
			HGLOBAL HFiles = medium.hGlobal; 
			HDROP HDrop = (HDROP)GlobalLock(HFiles);

		   // call the helper routine which will notify the Form 
			// of the drop 
			DropToFiles(HDrop,files);

		   // release the pointer to the memory 
			GlobalUnlock(HFiles); 
			ReleaseStgMedium(&medium);
			return true;
		}
		return false;
	}

protected: 
	// IUnknown methods 
	STDMETHOD(QueryInterface)(REFIID riid, void * * ppvObj); 
	STDMETHOD_(ULONG, AddRef)(); 
	STDMETHOD_(ULONG, Release)();

	// IDropTarget methods 
	STDMETHOD(DragEnter)(LPDATAOBJECT pDataObj, DWORD grfKeyState, 
											POINTL pt, LPDWORD pdwEffect); 
	STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect); 
	STDMETHOD(DragLeave)(); 
	STDMETHOD(Drop)(LPDATAOBJECT pDataObj, DWORD grfKeyState, 
					POINTL pt, LPDWORD pdwEffect);

public: 
	Engine::DragEventHandler		onDrag;
	Engine::DropEventHandler		onDrop;
	CtxDropTarget(); 
	~CtxDropTarget();
	bool							IsDragging()	const	{return acceptFormat;}
}; 



CtxDropTarget::CtxDropTarget() 
        : IDropTarget() 
{ 
    references = 1; 
    acceptFormat = false; 
} 
//---------------------------------------------------------------------------
CtxDropTarget::~CtxDropTarget() 
{ 
} 
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------

// IUnknown Interface has three member functions: 
// QueryInterface, AddRef, and Release.

STDMETHODIMP CtxDropTarget::QueryInterface(REFIID iid, void * * ppv) 
{ 
   // tell other objects about our capabilities 
   if (iid == IID_IUnknown || iid == IID_IDropTarget) 
   { 
       *ppv = this; 
       AddRef(); 
       return NOERROR; 
   } 
   *ppv = NULL; 
   return ResultFromScode(E_NOINTERFACE); 
} 
//---------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CtxDropTarget::AddRef() 
{ 
   return ++references; 
} 
//---------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CtxDropTarget::Release() 
{ 
   if (--references == 0) 
   { 
       delete this; 
       return 0; 
   } 
   return references; 
} 
//---------------------------------------------------------------------------

// IDropTarget Interface handles the Drag and Drop 
// implementation

// Drag Enter is called first 
STDMETHODIMP CtxDropTarget::DragEnter(LPDATAOBJECT pDataObj, DWORD grfKeyState, 
    POINTL pt, LPDWORD pdwEffect) 
{ 
	FORMATETC fmtetc;

	fmtetc.cfFormat = CF_HDROP; 
	fmtetc.ptd      = NULL; 
	fmtetc.dwAspect = DVASPECT_CONTENT; 
	fmtetc.lindex   = -1; 
	fmtetc.tymed    = TYMED_HGLOBAL;

   // does the drag source provide CF_HDROP, 
    // which is the only format we accept 
	if (onDrop && pDataObj->QueryGetData(&fmtetc) == NOERROR) 
	{
		if (!onDrag || (DataToFiles(pDataObj,files) && onDrag(files)))
		{
			(*pdwEffect) = DROPEFFECT_COPY;
			acceptFormat = true;
			return NOERROR;
		}
	}
	(*pdwEffect) = DROPEFFECT_NONE;
	acceptFormat = false;
	return E_UNEXPECTED;
	//return NOERROR; 
} 
//---------------------------------------------------------------------------

// implement visual feedback if required 
STDMETHODIMP CtxDropTarget::DragOver(DWORD grfKeyState, POINTL pt,  
    LPDWORD pdwEffect) 
{ 
   return NOERROR; 
} 
//---------------------------------------------------------------------------

// remove visual feedback 
STDMETHODIMP CtxDropTarget::DragLeave() 
{ 
	acceptFormat = false; 
	return NOERROR; 
} 
//---------------------------------------------------------------------------

// source has sent the DRAGDROP_DROP message indicating 
// a drop has a occurred 
STDMETHODIMP CtxDropTarget::Drop(LPDATAOBJECT pDataObj, DWORD grfKeyState, 
    POINTL pt, LPDWORD pdwEffect) 
{
	if (acceptFormat && onDrop && DataToFiles(pDataObj,files))
	{
		onDrop(files);
	}
	else
    { 
		(*pdwEffect) = DROPEFFECT_NONE; 
    }
	acceptFormat = false;
	return NOERROR; 
} 
//--------------------------------------------------------------------------- 
  




















#elif SYSTEM==UNIX

	//private include to prevent name violations
	#include <X11/extensions/Xrandr.h>
#endif


namespace Engine
{

	Context			context;
	LogFile			 *log_file(NULL);

	#if SYSTEM==UNIX
	static XRRScreenSize				*res_map=NULL;
	static int							res_count=0;
	static XRRScreenConfiguration		*config=NULL;
	static DEVMODE					 	_current=0,
										_initial=0,
										_target=0;
	#else
	static DEVMODE					 	_current,
										_initial,
										_target;
	
	#endif

	static bool							_applied=false,
										_focused=false;
	
	/*static*/ UINT32				Context::displayConfigFlags = 0;


	#define ENGINE_CLASS_NAME L"Engine Display Class"

	
	#if SYSTEM==WINDOWS
		#ifndef DISPLAY_NO_MOUSE_HOOKS
			HHOOK	Context::hHook(NULL);
			unsigned	Context::hook_counter(0);
		
		
			void	Context::installHook()
			{
				if (!hHook)
				{
					hHook = SetWindowsHookEx(WH_MOUSE_LL,mouseHook,GetInstance(),0);
				}
				hook_counter++;
			}
		
			void	Context::uninstallHook(bool force)
			{
				hook_counter--;
				if (force)
					hook_counter = 0;
				if (!hook_counter && hHook)
				{
					UnhookWindowsHookEx(hHook);
					hHook = NULL;
				}
			}
		#endif

	#endif
	
	
	static void initAnalogs()
	{
		static bool initialized = false;
		if (!initialized)
		{
			mouse.RegAnalogInputs();
			#if SYSTEM==WINDOWS
				joystick.RegAnalogInputs();
			#endif
			initialized = true;
		}
	}

	void logOperations(LogFile*target)
	{
		log_file = target;
	}

	Context::Context():exec_target(NULL),
	#if SYSTEM==WINDOWS
		eventHook(NULL),hInstance(NULL),hWnd(NULL),class_created(false),
	#elif SYSTEM==UNIX
		display(NULL),window(0),colormap(0),
	#endif
		shutting_down(false)
	{
		_location.left = 0;
		_location.top = 0;
		_location.right = 800;
		_location.bottom = 600;

		client_area.left = 0;
		client_area.top = 0;
		client_area.right = 800;
		client_area.bottom = 600;

	#if SYSTEM==WINDOWS
		OleInitialize(NULL);
		_initial.dmSize = sizeof(_initial);
		_initial.dmDriverExtra = 0;
		if (!EnumDisplaySettings(NULL,ENUM_REGISTRY_SETTINGS,&_initial))
		{
			ErrMessage("Context_Error: unable to retrieve initial screen setting!");
			_initial.dmPelsWidth = 800;
			_initial.dmPelsHeight = 600;
			_initial.dmFields = DM_PELSWIDTH|DM_PELSHEIGHT;
		}
	#elif SYSTEM==UNIX
		memset(function_key_mask,false,sizeof(function_key_mask));
	 	function_key_mask[Key::Left] = true;
	 	function_key_mask[Key::Right] = true;
	 	function_key_mask[Key::Up] = true;
	 	function_key_mask[Key::Down] = true;
	 	function_key_mask[Key::PageUp] = true;
	 	function_key_mask[Key::PageDown] = true;
	 	function_key_mask[Key::End] = true;
	 	function_key_mask[Key::Home] = true;
	 	function_key_mask[Key::Insert] = true;
	 	function_key_mask[Key::Delete] = true;
	 	function_key_mask[Key::Enter] = true;
	 	function_key_mask[Key::Return] = true;
	 	function_key_mask[Key::Backspace] = true;
	 	function_key_mask[Key::Shift] = true;
	 	function_key_mask[Key::NumLock] = true;
	 	function_key_mask[Key::Tab] = true;
	 	function_key_mask[Key::Pause] = true;
	 	function_key_mask[Key::Escape] = true;
	 	function_key_mask[Key::Down] = true;

	 	function_key_mask[Key::F1] = true;
	 	function_key_mask[Key::F2] = true;
	 	function_key_mask[Key::F3] = true;
	 	function_key_mask[Key::F4] = true;
	 	function_key_mask[Key::F5] = true;
	 	function_key_mask[Key::F6] = true;
	 	function_key_mask[Key::F7] = true;
	 	function_key_mask[Key::F8] = true;
	 	function_key_mask[Key::F9] = true;
	 	function_key_mask[Key::F10] = true;
	 	function_key_mask[Key::F11] = true;
	 	function_key_mask[Key::F12] = true;
	 	function_key_mask[Key::F13] = true;
	 	function_key_mask[Key::F14] = true;
	 	function_key_mask[Key::F15] = true;
	 	function_key_mask[Key::F16] = true;
	 	function_key_mask[Key::F17] = true;
	 	function_key_mask[Key::F18] = true;
	 	function_key_mask[Key::F19] = true;
	 	function_key_mask[Key::F20] = true;
	#endif
		_current = _initial;
	}


	void	Context::MaximizeWindow()
	{
		#if SYSTEM==WINDOWS
			if (hWnd)
				ShowWindow(hWnd,SW_MAXIMIZE);
		#elif SYSTEM_VARIANCE==LINUX
			if (display)
			{
				XEvent xev;
				Atom wm_state  =  XInternAtom(display, "_NET_WM_STATE", False);
				Atom max_horz  =  XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
				Atom max_vert  =  XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", False);

				memset(&xev, 0, sizeof(xev));
				xev.type = ClientMessage;
				xev.xclient.window = window;
				xev.xclient.message_type = wm_state;
				xev.xclient.format = 32;
				xev.xclient.data.l[0] = 1;//_NET_WM_STATE_ADD;
				xev.xclient.data.l[1] = max_horz;
				xev.xclient.data.l[2] = max_vert;

				XSendEvent(display, DefaultRootWindow(display), False, SubstructureNotifyMask, &xev);			
			}
		#else
			#error sgub
		#endif	
	}

	void	Context::MinimizeWindow()
	{
		#if SYSTEM==WINDOWS
			if (hWnd)
				ShowWindow(hWnd,SW_MINIMIZE);
		#elif SYSTEM_VARIANCE==LINUX
			if (display)
			{
				XEvent xev;
				Atom wm_state  =  XInternAtom(display, "_NET_WM_STATE_HIDDEN", False);
				//Atom max_horz  =  XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
				//Atom max_vert  =  XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_VERT", False);

				memset(&xev, 0, sizeof(xev));
				xev.type = ClientMessage;
				xev.xclient.window = window;
				xev.xclient.message_type = wm_state;
				xev.xclient.format = 32;
				xev.xclient.data.l[0] = 1;//_NET_WM_STATE_ADD;
				//xev.xclient.data.l[1] = max_horz;
				//xev.xclient.data.l[2] = max_vert;

				XSendEvent(display, DefaultRootWindow(display), False, SubstructureNotifyMask, &xev);			
			}
		#else
			#error stub
		#endif	

	}

	void	Context::RestoreWindow()
	{
		#if SYSTEM==WINDOWS
			if (hWnd)
				ShowWindow(hWnd,SW_RESTORE);
		#elif SYSTEM_VARIANCE==LINUX
			if (display)
			{
				XEvent xev;
				Atom wm_state  =  XInternAtom(display, "_NET_WM_STATE", False);	//i hope this is correct
				//Atom max_horz  =  XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
				//Atom max_vert  =  XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", False);

				memset(&xev, 0, sizeof(xev));
				xev.type = ClientMessage;
				xev.xclient.window = window;
				xev.xclient.message_type = wm_state;
				xev.xclient.format = 32;
				xev.xclient.data.l[0] = 1;//_NET_WM_STATE_ADD;
				//xev.xclient.data.l[1] = max_horz;
				//xev.xclient.data.l[2] = max_vert;

				XSendEvent(display, DefaultRootWindow(display), False, SubstructureNotifyMask, &xev);			
			}
		#else
			#error stub
		#endif	

	}


	#if SYSTEM==WINDOWS

	static CtxDropTarget	dropHandler;

	void Context::AcceptFileDrop()
	{
		//dropHandler.enabled = true;
		if (hWnd)
		{
			RegisterDragDrop(hWnd,&dropHandler);
			//DragAcceptFiles(hWnd,TRUE);
		}
	}
	
	void Context::AcceptFileDrop(const DragEventHandler&onDrag, const DropEventHandler&onDrop)
	{
		dropHandler.onDrag = onDrag;
		dropHandler.onDrop = onDrop;
		//dropHandler.enabled = true;
		if (hWnd)
		{
			HRESULT rs = RegisterDragDrop(hWnd,&dropHandler);
			if (rs != S_OK)
			{
				ShowMessage(rs == DRAGDROP_E_INVALIDHWND);
				ShowMessage(rs == DRAGDROP_E_ALREADYREGISTERED);
				ShowMessage(rs == E_OUTOFMEMORY);
			}
			//ShowMessage();

			//DragAcceptFiles(hWnd,acceptDroppedFiles);
		}
	}

	void Context::BlockFileDrop()
	{
		//dropHandler.enabled = false;
		if (hWnd)
		{
			RevokeDragDrop(hWnd);
			//DragAcceptFiles(hWnd,FALSE);
		}
	}
	
	StringW Context::createClass()
	{
		if (class_created)
			return ENGINE_CLASS_NAME;

		//hInstance = getInstance();
		hInstance = GetModuleHandle(NULL);
        WNDCLASSW wc;
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = (WNDPROC) WndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInstance;
        wc.hIcon = NULL;//LoadIcon(NULL, IDI_APPLICATION);;
        wc.hCursor = LoadCursor(NULL,IDC_ARROW);
        wc.hbrBackground = NULL;
        wc.lpszMenuName = NULL;
        wc.lpszClassName = ENGINE_CLASS_NAME;
        if (!RegisterClassW(&wc))
        {
            _error = ERR_CLASS_REGISTRATION_FAILED;
            return "";
        }
		class_created = true;
		return ENGINE_CLASS_NAME;
	}

	void Context::destroyClass()
	{
		if (!class_created)
			return;
		hInstance = GetInstance();
		UnregisterClassW(ENGINE_CLASS_NAME,hInstance);
		class_created = false;
	}

	void	Context::setWindow(HWND window)
	{
		hWnd = window;

	}

	HWND	Context::window()	const
	{
		return hWnd;
	}
	


	
	HWND Context::createWindow(const String&window_name, DisplayConfig::border_style_t border_style, const DisplayConfig::FOnResize&onResize, const DisplayConfig::Icon&icon)
	{
		displayConfigFlags = 0;
		this->border_style = border_style;
		this->onResize = onResize;
	/*
	XCreateWindow(WFDisplay,RootWindow(WFDisplay,VisualInfo->screen),30,40,Width,Height,0,VisualInfo->depth,InputOutput,VisualInfo->visual,CWBorderPixel|CWColormap|CWEventMask,&WindowAttributes);
	*/

	
		if (hWnd)
			return hWnd;
		mouse.LooseFocus();
		DWORD style = WS_POPUP|WS_CLIPCHILDREN;//|WS_CLIPSIBLINGS;
		switch (border_style)
		{
			case DisplayConfig::NoBorder:
				style |= WS_EX_TOPMOST;
			break;
			case DisplayConfig::FixedBorder:
				style |=   WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
			break;
			case DisplayConfig::ResizableBorder:
				style |=   WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME;
			break;
		}
		_error = ERR_NO_ERROR;
		hInstance = GetInstance();

		WNDCLASSEXW wc;
		wc.cbSize = sizeof (wc);
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc = (WNDPROC) WndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance;
		wc.hCursor = LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName = NULL;
		wc.lpszClassName = ENGINE_CLASS_NAME;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0; 
		if (icon.fileName.IsNotEmpty())
		{
			wc.hIconSm = wc.hIcon = (HICON)LoadImageA(hInstance,icon.fileName.c_str(),IMAGE_ICON,0,0,LR_LOADFROMFILE);
		}
		else
			if (icon.idiResource >= 0)
				wc.hIconSm = wc.hIcon = LoadIcon (GetModuleHandle(NULL), MAKEINTRESOURCE(icon.idiResource));
			else
				wc.hIconSm = wc.hIcon = LoadIcon (GetModuleHandle(NULL), IDI_APPLICATION);
		if (!RegisterClassExW(&wc))
		{
			_error = ERR_CLASS_REGISTRATION_FAILED;
			return NULL;
		}


		//#ifdef _UNICODE
			Array<wchar_t>	w_name;
			w_name.SetSize(window_name.GetLength()+1);
			for (index_t i = 0; i < window_name.GetLength(); i++)
				w_name[i] = (wchar_t)window_name.GetChar(i);
			w_name.Last() = 0;
				
			
			hWnd = CreateWindowExW(
				0,
				ENGINE_CLASS_NAME,w_name.pointer(),
				style,
				_location.left,_location.top, //Position
				_location.right-_location.left,_location.bottom-_location.top, //Dimensionen
				/*HWND_DESKTOP*/NULL,
				NULL,
				hInstance, //Application
				NULL);
		//#else
		//	hWnd = CreateWindowExA(
		//		0,
		//		ENGINE_CLASS_NAME,window_name.c_str(),
		//		style,
		//		_location.left,_location.top, //Position
		//		_location.right-_location.left,_location.bottom-_location.top, //Dimensionen
		//		/*HWND_DESKTOP*/NULL,
		//		NULL,
		//		hInstance, //Application
		//		NULL);
		//#endif




		if (!hWnd)
		{
			_error = ERR_WINDOW_CREATION_FAILED;
			UnregisterClassW(ENGINE_CLASS_NAME,hInstance);
			return NULL;
		}
		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);
		ApplyWindowPosition();
		SetForegroundWindow(hWnd);
		SetFocus(hWnd);
		timing.initialize();
		
		initAnalogs();
		
		mouse.cursor_visible = true;
		//UnregisterClassA(ENGINE_CLASS_NAME,hInstance);
		
		SignalResize(0);
		//this->_restoredClientSize = 
		
		return hWnd;
	}
	
	HWND Context::createChildWindow(HWND parent, bool enabled)
	{

	/*
	XCreateWindow(WFDisplay,RootWindow(WFDisplay,VisualInfo->screen),30,40,Width,Height,0,VisualInfo->depth,InputOutput,VisualInfo->visual,CWBorderPixel|CWColormap|CWEventMask,&WindowAttributes);
	*/
		if (hWnd)
			return hWnd;

		displayConfigFlags = 0;


		mouse.LooseFocus();
		DWORD style = WS_CHILD |WS_CLIPCHILDREN;//|WS_CLIPSIBLINGS;
		if (!enabled)
			style |= WS_DISABLED;
		
		_error = ERR_NO_ERROR;
		hInstance = GetInstance();

		WNDCLASSEXW wc;
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc = (WNDPROC) WndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance;
		wc.hCursor = LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName = NULL;
		wc.lpszClassName = ENGINE_CLASS_NAME;
		wc.cbSize = sizeof (wc);
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0; 
		wc.hIconSm = wc.hIcon = LoadIcon (NULL, IDI_APPLICATION);
		if (!RegisterClassExW(&wc))
		{
			_error = ERR_CLASS_REGISTRATION_FAILED;
			return NULL;
		}



		hWnd = CreateWindowExW(
			0,
			ENGINE_CLASS_NAME,L"",
			style,
			_location.left,_location.top, //Position
			_location.right-_location.left,_location.bottom-_location.top, //Dimensionen
			parent,
			NULL,
			hInstance, //Application
			NULL);

		if (!hWnd)
		{
			_error = ERR_WINDOW_CREATION_FAILED;
			UnregisterClassW(ENGINE_CLASS_NAME,hInstance);
			return NULL;
		}
		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);
		ApplyWindowPosition();
		SetForegroundWindow(hWnd);
		SetFocus(hWnd);
		timing.initialize();
		
		initAnalogs();
		
		mouse.cursor_visible = true;
		UnregisterClassW(ENGINE_CLASS_NAME,hInstance);
		
				
		
		
		return hWnd;
	}
	#elif SYSTEM==UNIX

	::Display*Context::connect()
	{
		display = getDisplay();
		if (!display)
			_error = ERR_X_CONNECTION_FAILED;
		return display;
	}

	::Display*Context::connection()
	{
		return display;
	}

	void Context::disconnect()
	{
		res_map = NULL;
		res_count = 0;
		display = NULL;
		closeDisplay();
	}


	Window Context::createWindow(const String&window_name, const TWindowAttributes&attributes,DisplayConfig::border_style_t border_style, const DisplayConfig::FOnResize&onResize, const DisplayConfig::Icon&icon)
	{
		displayConfigFlags = 0;

		this->border_style = border_style;
		this->onResize = onResize;
		if (!display)
			return 0;

		if (window)
			return window;

		screen = attributes.screen_number;
		res_map = XRRSizes(display, screen, &res_count);

		colormap=XCreateColormap(display,RootWindow(display,attributes.screen_number),attributes.visual,AllocNone);
		if (!colormap)
			return 0;
		XSetWindowAttributes attrib;
		attrib.colormap = colormap;
		attrib.border_pixel = border_style == DisplayConfig::NoBorder ?0:CopyFromParent;
		attrib.event_mask = ExposureMask|ButtonPressMask|ButtonReleaseMask|StructureNotifyMask|KeyPressMask|KeyReleaseMask|SubstructureRedirectMask;
		attrib.override_redirect = border_style == DisplayConfig::NoBorder;



		window = XCreateWindow(display,RootWindow(display,attributes.screen_number),_location.left,_location.top,_location.right-_location.left,_location.bottom-_location.top,
		 border_style != DisplayConfig::NoBorder,
		 attributes.depth,InputOutput,attributes.visual,CWBorderPixel|CWColormap|CWEventMask|CWOverrideRedirect,&attrib);
		if (!window)
			return 0;
		//Atom atom = XInternAtom (display, "WM_DELETE_WINDOW", False);
		//XSetWMProtocols(display, window, &atom , 1);

		if (config)
			XRRFreeScreenConfigInfo(config);
		config = XRRGetScreenInfo(display,window);
		XSetStandardProperties(display,window,window_name.c_str(),"",None,0,0,0);
		//XSelectInput(display, window, ButtonPressMask | ButtonReleaseMask | FocusChangeMask);
		XMapWindow(display,window);
		if (border_style == DisplayConfig::NoBorder)
			XGrabKeyboard(display, window, True, GrabModeAsync, GrabModeAsync, CurrentTime);
		//XGrabPointer(display, window, True, ButtonPressMask|ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None /*window*/, None, CurrentTime);
		XMoveResizeWindow(display,window,_location.left,_location.top,_location.right-_location.left,_location.bottom-_location.top);
		//XStoreName(display,window,window_name);
		timing.initialize();
		mouse.Assign(display,window,screen);
		mouse.RedefineWindow(_location,window);
		mouse.SetRegion(res_map[_current].width, res_map[_current].height);
		initAnalogs();
		window_attributes = attributes;
		return window;
	}


	#endif



	bool Context::hideCursor()
	{
		return  mouse.HideCursor();
	}

	void Context::showCursor()
	{
		mouse.ShowCursor();
	}
	

	#if SYSTEM==WINDOWS


		bool		Context::CheckFullscreen(const WINDOWINFO&info) const
		{
			Resolution screen = GetScreenSize();
			return info.rcClient == info.rcWindow
					 && info.rcClient.left == 0 && info.rcClient.top == 0 && info.rcClient.right == screen.width && info.rcClient.bottom == screen.height;
		}

	#endif	


	UINT32 Context::GetDisplayConfigFlags() const
	{
		#if SYSTEM==WINDOWS
			if (!hWnd)
				return 0;
			UINT32 result = DisplayConfig::ResizeDragHasEnded;
			WINDOWINFO	info;
			if (GetWindowInfo(hWnd,&info))
			{
				if (info.dwStyle & WS_ICONIC)
					result |= DisplayConfig::IsMinimzed;
				if (info.dwStyle & WS_MAXIMIZE)
					result |= DisplayConfig::IsMaximized;
				if (CheckFullscreen(info))
					result |= DisplayConfig::IsFullscreen;
			}
			return result;
		#else
			//#error stub
			return DisplayConfig::ResizeDragHasEnded;
		#endif
	}

	UINT32 Context::ApplyWindowPosition()
	{
		UINT32 result = 0;
		#if SYSTEM==WINDOWS
			if (!hWnd)
				return result;
			result |= DisplayConfig::ResizeDragHasEnded;
			SetWindowPos(hWnd,NULL,_location.left,_location.top,_location.right-_location.left,_location.bottom-_location.top,0);
			WINDOWINFO	info;
			if (GetWindowInfo(hWnd,&info))
			{
				if (info.dwStyle & WS_ICONIC)
					result |= DisplayConfig::IsMinimzed;
				if (info.dwStyle & WS_MAXIMIZE)
					result |= DisplayConfig::IsMaximized;
				if (CheckFullscreen(info))
					result |= DisplayConfig::IsFullscreen;
				_location = info.rcWindow;
				client_area = info.rcClient;
				mouse.RedefineWindow(client_area,hWnd);
			}
		#elif SYSTEM==UNIX
			client_area = _location;
			if (!window || !display)
				return result;
			result |= DisplayConfig::ResizeDragHasEnded;
			XMoveResizeWindow(display,window,_location.left,_location.top,_location.right-_location.left,_location.bottom-_location.top);
			//XGetGeometry
			mouse.redefineWindow(_location,window);
		#endif
		return result;
	}

	/*static*/ UINT32		Context::GetLastSignalledDisplayFlags()
	{
		return displayConfigFlags;
	}


	DisplayConfig::FOnResize	Context::ReplaceOnResize(const DisplayConfig::FOnResize&newCallback)
	{
		DisplayConfig::FOnResize old = onResize;
		onResize = newCallback;

		return old;
	}


	void Context::SignalResize(UINT32 flags)
	{
		#if SYSTEM==WINDOWS
			if (!hWnd)
				return;
			WINDOWINFO	info;
			if (GetWindowInfo(hWnd,&info))
			{
				_location = info.rcWindow;
				client_area = info.rcClient;
				mouse.RedefineWindow(client_area,hWnd);

				const Resolution newRes = Resolution(client_area.right - client_area.left,client_area.bottom - client_area.top);

				if (!(flags & (Engine::DisplayConfig::IsMaximized | Engine::DisplayConfig::IsMinimzed | Engine::DisplayConfig::IsFullscreen)))
				{
					_restoredLocation = _location;
					_restoredClientSize = newRes;
				}


				if (onResize)
					onResize(newRes,flags);
			}
		#elif SYSTEM_VARIANCE==LINUX
			if (!display)
				return;
			XWindowAttributes info;
			XGetWindowAttributes(display, window, &info);
			_location.left = info.x;
			_location.top = info.y;
			_location.right = info.x + info.width;
			_location.bottom = info.y + info.height;
			client_area = _location;
			mouse.RedefineWindow(client_area,window);
			if (onResize)
				onResize(Resolution(client_area.right - client_area.left,client_area.bottom - client_area.top),flags);
			
			
		#else
			#error not defined
		#endif
	}

	void	Context::SetBorderStyle(DisplayConfig::border_style_t style)
	{
		if (style != border_style)
		{
			#if SYSTEM==WINDOWS
				LONG lStyle = GetWindowLong(window(), GWL_STYLE);
				switch (style)
				{
					case DisplayConfig::NoBorder:
						lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
					break;
					case DisplayConfig::FixedBorder:
						lStyle |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
						lStyle &= ~(WS_MAXIMIZEBOX | WS_THICKFRAME);
					break;
					case DisplayConfig::ResizableBorder:
						lStyle |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME;
					break;
				}
				SetWindowLong(window(), GWL_STYLE, lStyle);
				SetWindowPos(window(), NULL, 0,0,0,0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
			#elif SYSTEM==LINUX
				if (display)
				{
					XSetWindowAttributes attrib;
					attrib.colormap = colormap;
					attrib.border_pixel = style == DisplayConfig::NoBorder ?0:CopyFromParent;
					attrib.event_mask = ExposureMask|ButtonPressMask|ButtonReleaseMask|StructureNotifyMask|KeyPressMask|KeyReleaseMask|SubstructureRedirectMask;
					attrib.override_redirect = style == DisplayConfig::NoBorder;
					XChangeWindowAttributes(display,window,CWBorderPixel|CWColormap|CWEventMask|CWOverrideRedirect,&attrib);
					if (style == DisplayConfig::NoBorder)
						XGrabKeyboard(display, window, True, GrabModeAsync, GrabModeAsync, CurrentTime);
					else
						XUngrabKeyboard(display,CurrentTime);
				}
			#else
				#error stub
			#endif
			border_style = style;
		}

	}


	UINT32 Context::ResizeWindow(unsigned width, unsigned height)
	{
		_location.right = _location.left + width;
		_location.bottom = _location.top + height;
		return ApplyWindowPosition();
	}

	UINT32 Context::PositionWindow(unsigned left, unsigned top, unsigned width, unsigned height)
	{
		_location.left = left;
		_location.top = top;
		_location.right = left+width;
		_location.bottom = top+height;
		return ApplyWindowPosition();
	}

	UINT32 Context::PositionWindow(const RECT&dimensions)
	{
		_location = dimensions;
		return ApplyWindowPosition();
	}

	const RECT& Context::GetWindowLocation()	const
	{
		return _location;
	}

	const RECT& Context::windowClientArea()	const
	{
		return client_area;
	}

	Resolution Context::GetWindowSize()	const
	{
		return Resolution(
				_location.right-_location.left,
				_location.bottom-_location.top);
	}

	Resolution	Context::GetClientSize()	const
	{
		return Resolution(
			client_area.right-client_area.left,
			client_area.bottom-client_area.top);
	}

	UINT	Context::GetClientWidth()	const
	{
		return client_area.right-client_area.left;
	}
	UINT	Context::GetClientHeight()	const
	{
		return client_area.bottom-client_area.top;
	}

	bool	Context::isTopWindow()	const
	{
		#if SYSTEM==WINDOWS
			return GetForegroundWindow() == hWnd;
		#elif SYSTEM==UNIX
			return isFocused();	//not quite true but i can't be arsed to scan the window tree
		#else
			#error stub
		#endif
	}
	void	Context::focus()
	{
		#if SYSTEM == WINDOWS
			SetForegroundWindow(hWnd);
			SetFocus(hWnd);
		#elif SYSTEM==UNIX
			XSetInputFocus(display, window, RevertToParent, CurrentTime);
		#else
			#error stub
		#endif
	}


	bool Context::isMinimized() const
	{
		#if SYSTEM==WINDOWS
			return hWnd && IsIconic(hWnd);
		#elif SYSTEM_VARIANCE==LINUX
			return false;	//don't know
		#else
			#error stub
		#endif

	}

	bool Context::WindowIsMaximized() const
	{
		#if SYSTEM==WINDOWS
			return hWnd && IsZoomed(hWnd);
		#elif SYSTEM==UNIX
			return false;
		#else
			#error stub
		#endif

	}

	bool Context::isFocused()	const
	{

		#if SYSTEM==WINDOWS
			return hWnd && GetForegroundWindow() == hWnd;
		#elif SYSTEM==UNIX
			if (!window)
				return false;
			Window focus;
			int revert_to;
			XGetInputFocus(display, &focus, &revert_to);
			return focus == window;
		#else
			return false;
		#endif
	}

	void Context::checkFocus()
	{
		mouse.SetFocused(isFocused());
	}

	void Context::destroyWindow()
	{
		shutting_down = true;
		#if SYSTEM==WINDOWS
			if (!hWnd)
				return;
			ShowWindow(hWnd,SW_HIDE);
			DestroyWindow(hWnd);
			hWnd = NULL;
			UnregisterClassW(ENGINE_CLASS_NAME,hInstance);
		#elif SYSTEM==UNIX
			if (!window || !display)
				return;
			if (config)
				XRRFreeScreenConfigInfo(config);
			config = NULL;
			XUngrabPointer(display,CurrentTime);
			XUngrabKeyboard(display,CurrentTime);
			XDestroyWindow(display,window);
			XFreeColormap(display,colormap);
			res_count = 0;
			window = 0;

		#endif
		shutting_down = false;
	}


	void Context::SetScreen(DEVMODE&screen)
	{
		_target = screen;
	}

	void Context::SetScreen(const TDisplayMode&mode)
	{
	#if SYSTEM==WINDOWS
		_target.dmPelsWidth = mode.width;
		_target.dmPelsHeight = mode.height;
		_target.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
		_target.dmSize = sizeof(_target);
		_target.dmDriverExtra = 0;

		if (mode.frequency)
		{
			_target.dmDisplayFrequency = mode.frequency;
			_target.dmFields |= DM_DISPLAYFREQUENCY;
		}
	#elif SYSTEM==UNIX
		_trate = mode.frequency;
		_target = FindScreen(mode.width,mode.height,_trate);
	#endif
	}


	void Context::SetScreen(DEVMODE*screen)
	{
		_target = (*screen);
	}

	bool Context::ApplyScreen()
	{
	#if SYSTEM==WINDOWS
		_applied = (ChangeDisplaySettings(&_target,CDS_FULLSCREEN)==DISP_CHANGE_SUCCESSFUL);
		if (_applied)
		{
			EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&_current);
			mouse.SetRegion(_current.dmPelsWidth,_current.dmPelsHeight);
		}
	#elif SYSTEM==UNIX
	/*	Time stamp;
		XRRConfigTimes(config,&stamp);
		ShowMessage("stamp retrieved: "+IntToStr(stamp));
		Rotation rotation;
		XRRRotations(display,screen,&rotation);
		ShowMessage("rotation retrieves: "+IntToStr(rotation));
		ShowMessage("trying to set config: (display=0x"+IntToHex((int)display,8)+", config=0x"+IntToHex((int)config,8)+", window="+IntToStr(window)+", target="+IntToStr(_target)+", target_rate="+IntToStr(_trate)+")");*/
		_applied = !XRRSetScreenConfigAndRate(display, config, window, _target, 1, _trate, CurrentTime);
		if (!_applied)
		{
			config = XRRGetScreenInfo(display,window);
			_applied = !XRRSetScreenConfigAndRate(display, config, window, _target, 1, _trate, CurrentTime);
		}
	#endif
		return _applied;
	}

	bool Context::ApplyWindowScreen(DWORD refresh_rate)
	{
	#if SYSTEM==WINDOWS
		if (!hWnd)
			return false;
		DEVMODE mode = _target;
		mode.dmPelsWidth = _location.right;
		mode.dmPelsHeight = _location.bottom;
		mode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
		mode.dmSize = sizeof(mode);
		mode.dmDriverExtra = 0;

		if (refresh_rate)
		{
			mode.dmDisplayFrequency = refresh_rate;
			mode.dmFields |= DM_DISPLAYFREQUENCY;
		}
		if (ChangeDisplaySettings(&mode,CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL)
		{
			EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&_current);
			mouse.SetRegion(_current.dmPelsWidth,_current.dmPelsHeight);
			_target = _current;
			_applied = true;
			ApplyWindowPosition();
			return true;
		}
		else
			if (_applied)
				ApplyScreen();
			else
				ResetScreen();
		return false;
	#elif SYSTEM==UNIX
		if (!window || !display)
			return false;
		int size = FindScreen(_location.right,_location.bottom,refresh_rate);
		if (size==-1)
			return false;
		_target = size;
		_trate = refresh_rate;
		_applied = ApplyScreen();
		LocateWindow();
		return _applied;
	#endif
	}

	void Context::ResetScreen()
	{
		if (_applied)
		#if SYSTEM==WINDOWS
			ChangeDisplaySettings(NULL,0);
		#elif SYSTEM==UNIX
			XRRSetScreenConfigAndRate(display, config, window, 0, 1, 0, CurrentTime);
		#endif
		mouse.ResetRegion();
		_current = _initial;
		_applied = false;
	}




	RECT Context::Transform(const M::TFloatRect&rect)	const
	{
		DWORD   width = client_area.right-client_area.left,
				height = client_area.bottom-client_area.top;
		//ShowMessage(IntToStr(width)+" x "+IntToStr(height));
		RECT result;
		result.left	 = (LONG)(rect.x.min	   *width);
		result.right	= (LONG)(rect.x.max	  *width);
		result.top	  = (LONG)((rect.y.max)	  *height);
		result.bottom   = (LONG)((rect.y.min)   *height);
		return result;
	}


	unsigned Context::countScreenModes()
	{
	#if SYSTEM==WINDOWS
		DEVMODE dummy;
		dummy.dmSize = sizeof(dummy);
		dummy.dmDriverExtra = 0;
		unsigned cnt(0);
		while (EnumDisplaySettings(NULL,cnt++,&dummy));
		return cnt;
	#elif SYSTEM==UNIX
		 return res_count;
	#endif
	}

	#if SYSTEM==WINDOWS
	DEVMODE*Context::GetScreen(unsigned index)	const
	{
		DEVMODE rs;
		rs.dmSize = sizeof(rs);
		rs.dmDriverExtra = 0;
		if (EnumDisplaySettings(NULL,index,&rs))
		{
			rs.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
			return new DEVMODE(rs);
		}
		return NULL;
	}

	bool Context::IsCurrent(const DEVMODE&screen)	const
	{
		return _current.dmPelsWidth == screen.dmPelsWidth && _current.dmPelsHeight == screen.dmPelsHeight && _current.dmDisplayFrequency == screen.dmDisplayFrequency;
	}

	bool Context::GetScreen(DEVMODE*mode)	const
	{
		EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,mode);
		return _applied;
	}

	bool Context::GetScreen(DEVMODE&mode)	const
	{
		return GetScreen(&mode);
	}

	short Context::GetRefreshRate()
	{
		DEVMODE mode;
		GetScreen(&mode);
		return mode.dmDisplayFrequency;
	}
	
	Resolution	Context::GetScreenSize()	const
	{
		DEVMODE mode;
		GetScreen(&mode);
		return Resolution(mode.dmPelsWidth,mode.dmPelsHeight);
	}

	#elif SYSTEM==UNIX

	static const XRRScreenSize& GetScreen(unsigned index)
	{
		 return res_map[index];
	}

	static bool GetScreen(XRRScreenSize*size)
	{
		context.connect();
		_current = XRRConfigCurrentConfiguration(config,0);
		 (*size) = res_map[_current];
		 return _applied;
	}
	
	Resolution Context::GetScreenSize()
	{
		if (!res_map)
		{
			connect();
			//screen = attributes.screen_number;
			/*cout << "Retreiving XRR screen resolutions..."<<endl;
			res_map = XRRSizes(display, DefaultScreen (display), &res_count);
			cout << "Retreived "<<res_count<<" possible screen resolution(s) of screen 0. Now retrieving configuration..."<<endl;*/
			Window root = RootWindow (display, DefaultScreen (display));
			ASSERT__(root!=None);
			
			XWindowAttributes	attributes;
			XGetWindowAttributes(display, root, &attributes);
			return Resolution(attributes.width,attributes.height);
			
			/*

			Window window;
			int dummy;
			XGetInputFocus(display, &window, &dummy);
			config = XRRGetScreenInfo(display,window);
			
			cout << " done."<<endl;
			*/
			
		}
		if (!connect())
		{
			return Resolution();
		}
		if (!config)
		{
			int screen = DefaultScreen(display);
			Window root = RootWindow(display, screen),*dummy;
			int x, y;
			unsigned border, depth, width, height;
			XGetGeometry(display, root, dummy,  &x,&y,&width,&height,&border,&depth);
			return Resolution(width,height);
		}
			

			
		std::cout << "retrieving current configuration ("<<config<<")"<<std::endl;

		_current = XRRConfigCurrentConfiguration(config,0);
		std::cout << "Current screen is "<<_current<<std::endl;
		return Resolution(res_map[_current].width,res_map[_current].height);
	}

	static bool GetScreen(XRRScreenSize&size)
	{
		 return GetScreen(&size);
	}

	static bool IsCurrent(const XRRScreenSize&size)
	{
		 return res_map[_current].width == size.width && res_map[_current].height == size.height;
	}

	int Context::FindScreen(DWORD width, DWORD height, DWORD&refresh_rate)
	{
		 for (int i = 0; i < res_count; i++)
			  if (res_map[i].width == width && res_map[i].height == height)
			  {
				   int rates;
				   short*rate = XRRRates(display,screen, i, &rates),
						max = 0;

				   for (int j = 0; j < rates; j++)
						if (rate[j] == refresh_rate)
							 return i;
						else
							 if (rate[j] > max)
								  max = rate[j];
	//			   XFree(rate);
				   refresh_rate = max;
				   return i;
			  }
		 return 0;
	}


	short Context::GetRefreshRate()
	{
		return XRRConfigCurrentRate(config);
	}

	#endif



	void Context::restoreFocus()
	{
		#if SYSTEM==UNIX
			XGrabKeyboard(display, window, True, GrabModeAsync, GrabModeAsync, CurrentTime);
		#endif
		_focused = true;
		mouse.RestoreFocus();
		if (onRestoreFocus)
			onRestoreFocus();
	}

	void Context::looseFocus()
	{
		#if SYSTEM==UNIX
			XUngrabKeyboard(display, CurrentTime);
		#endif
		_focused = false;
		mouse.LooseFocus();
		input.ReleasePressedKeys();
		if (onLoseFocus)
			onLoseFocus();
	}

	void Context::Assign(pEngineExec target)
	{
		exec_target = target;
	}


	void Context::handleEvents()
	{
		#if SYSTEM==WINDOWS
			MSG msg;
			while (PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
			{
				if (GetMessage(&msg, NULL,0,0))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
					return;
			}
		#elif SYSTEM==UNIX
			XEvent event;
			while (XPending(display))
			{
				XNextEvent(display,&event);
				Engine::ExecuteEvent(event);
			}
		#endif
	}


	void Context::Execute()
	{
		if (!exec_target
		#if SYSTEM==WINDOWS
			|| !hWnd
		#elif SYSTEM==UNIX
			|| !window
		#endif
			)
			return;
		bool exec_loop = true;
		BYTE focus_check = 5;
	    while (exec_loop)
	    {
			if (!--focus_check)
			{
				focus_check = 5;
				if (mouse.IsLocked())
					mouse.SetFocused(context.isFocused());
			}
			#if SYSTEM==WINDOWS
				MSG msg;
				while (PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
				{
					if (GetMessage(&msg, NULL,0,0))
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
					else
						return;
				}
			#elif SYSTEM==UNIX
				XEvent event;
				while (XPending(display))
				{
					XNextEvent(display,&event);
					Engine::ExecuteEvent(event);
				}
			#endif
			timing.update();
			exec_loop = exec_target() && !context.shutting_down;
		}
	}

	float Context::windowAspectf()	const
	{
		return (float)(client_area.right-client_area.left)/(client_area.bottom-client_area.top);
	}

	double Context::windowAspectd()	const
	{
		return (double)(client_area.right-client_area.left)/(client_area.bottom-client_area.top);
	}

	float Context::pixelAspectf()	const
	{
		#if SYSTEM==WINDOWS
			float frag = ((float)(client_area.right-client_area.left)) / ((float)(client_area.bottom-client_area.top));
			return frag;//*(float)_current.dmPelsWidth / _current.dmPelsHeight;
		#elif SYSTEM==UNIX
			if (!res_map)
				return windowAspectf();
			unsigned width = res_map[_current].width,
					 height = res_map[_current].height;
			float frag = ((float)(client_area.right-client_area.left)) / ((float)(client_area.bottom-client_area.top));
			return frag;//*(float)width / height;
		#endif
	}

	double Context::pixelAspectd()	const
	{
		#if SYSTEM==WINDOWS
			double frag = ((double)(client_area.right-client_area.left)/_current.dmPelsWidth) / ((double)(client_area.bottom-client_area.top)/_current.dmPelsHeight);
			return frag*(double)_current.dmPelsWidth / _current.dmPelsHeight;
		#elif SYSTEM==UNIX
			if (!res_map)
				return windowAspectd();
			unsigned width = res_map[_current].width,
					 height = res_map[_current].height;
			double frag = ((double)(client_area.right-client_area.left)/width) / ((double)(client_area.bottom-client_area.top)/height);
			return frag*(double)width / height;
		#endif
	}


	void Context::eventClose()
	{
		if (!shutting_down)
			close();
		#if SYSTEM==WINDOWS
		else
			ResetScreen();
		#endif
	}

	void Context::close()
	{
		ResetScreen();
		destroyWindow();
		#if SYSTEM==UNIX
			disconnect();
		#elif SYSTEM==WINDOWS
			#ifndef DISPLAY_NO_MOUSE_HOOKS
				uninstallHook(true);
			#endif
		#endif
		shutting_down = true;
	}


	unsigned Context::errorCode()
	{
		return _error;
	}

	const char*Context::errorStr()
	{
		#define ecase(token) case token: return #token;
		switch (_error)
		{
			ecase(ERR_NO_ERROR)
			ecase(ERR_CLASS_REGISTRATION_FAILED)
			ecase(ERR_WINDOW_CREATION_FAILED)
			ecase(ERR_X_CONNECTION_FAILED)
		}
		#undef ecase
		return "ERR_UNDEFINED_ERROR_CODE";
	}


	void Context::QueryScreen(ResolutionList*r_list, FrequencyList*f_list, DWORD w_min, DWORD h_min, DWORD f_min)
	{
		unsigned cnt(0);
		#if SYSTEM==WINDOWS

			DEVMODE out_info;
			out_info.dmSize = sizeof(out_info);
			out_info.dmDriverExtra = 0;
			while (EnumDisplaySettings(NULL,cnt++,&out_info))
				if (out_info.dmPelsWidth>=w_min && out_info.dmPelsHeight>=h_min && out_info.dmDisplayFrequency>=f_min)
				{
					r_list->insert(out_info);
					if (f_list)
						f_list->insert(out_info.dmDisplayFrequency);
				}
		#elif SYSTEM==UNIX
			for (int i = 0; i < res_count; i++)
			{
				ScreenResolution*res = SignalNew(new ScreenResolution());
				res->width = res_map[i].width;
				res->height = res_map[i].height;
				int rates;
				short*rate = XRRRates(display,screen, i, &rates);
				for (int j = 0; j < rates; j++)
				{
					res->insertFrequency(rate[j]);
					if (f_list)
						f_list->insert(rate[j]);
			}
			r_list->insert(res);
	//			deloc(rate);
			}
		#endif
		if (f_list)
		{
			for (ResolutionList::iterator res = r_list->begin(); res != r_list->end(); ++res)
			{
				TDisplayFrequency*f = res->frequency;
				while (f)
				{
					f->index = f_list->find(f->rate);
					if (f->index == UNSIGNED_UNDEF)
						FATAL__("Given frequency("+IntToStr(f->rate)+" hz) not found in frequency_list");
					f = f->next;
				}
			}
		}
	}




	#if SYSTEM==WINDOWS

	










	#ifndef DISPLAY_NO_MOUSE_HOOKS

		LRESULT CALLBACK Context::mouseHook(
				int nCode,
				WPARAM wParam,
				LPARAM lParam
				)
		{
			#ifndef WM_XBUTTONDOWN
				#define WM_XBUTTONDOWN 523
			#endif
			#ifndef WM_XBUTTONUP
				#define WM_XBUTTONUP 524
			#endif

			if (nCode < 0)
				return CallNextHookEx(hHook,nCode,wParam,lParam);
			if (nCode == HC_ACTION)
			{
				bool uninstall = false;
				switch (wParam)
				{
					case WM_LBUTTONUP:	uninstall=true;	mouse.SignalButtonUp(0);				break;
					case WM_MBUTTONUP:  uninstall=true;	mouse.SignalButtonUp(1);				break;
					case WM_RBUTTONUP:  uninstall=true;	mouse.SignalButtonUp(2);				break;
					case WM_XBUTTONUP:	uninstall=true;	mouse.SignalButtonUp(HIWORD (wParam)==1?3:4);	break;
				}
				LRESULT rs = CallNextHookEx(hHook,nCode,wParam,lParam);

				if (uninstall)
					uninstallHook();

				return rs;
		
			}
			else
				return CallNextHookEx(hHook,nCode,wParam,lParam);

		
		}
	#endif


	LRESULT CALLBACK Context::WndProc(HWND hWnd, UINT Msg, WPARAM wParam,LPARAM lParam)
	{


		#ifndef VK_XBUTTON1
			#define VK_XBUTTON1 5
		#endif
		#ifndef VK_XBUTTON2
			#define VK_XBUTTON2 6
		#endif



		switch (Msg)
		{
			case WM_SETCURSOR:
				return (LOWORD(lParam) == HTCLIENT) && mouse.CursorIsNotDefault() ? 1 : DefWindowProcW(hWnd, Msg, wParam, lParam);
			case WM_SIZING:		context.SignalResize(0);									return 0;
			case WM_SIZE:	
			{
				if (wParam != SIZE_MINIMIZED)
				{
					UINT32 newFlags = DisplayConfig::ResizeDragHasEnded;

					if (wParam == SIZE_MAXIMIZED)
						newFlags |= DisplayConfig::IsMaximized;
					if (wParam == SIZE_MINIMIZED)
						newFlags |= DisplayConfig::IsMinimzed;
					WINDOWINFO	info;
					if (GetWindowInfo(hWnd,&info))
					{
						if (context.CheckFullscreen(info))
							newFlags |= DisplayConfig::IsFullscreen;
					}

					context.SignalResize(newFlags);
					displayConfigFlags = newFlags;
				}
			}
			return 0;
			case WM_EXITSIZEMOVE:context.SignalResize(displayConfigFlags | DisplayConfig::ResizeDragHasEnded);								return 0;
			case WM_ERASEBKGND: 															return 1;
			case WM_SETFOCUS:   context.restoreFocus();										return 0;
			case WM_KILLFOCUS:  context.looseFocus();										return 0;
			case WM_SYSKEYDOWN:
			case WM_KEYDOWN:	keyboard.keyDown((Key::Name)wParam);						return 0;
			case WM_SYSKEYUP:
			case WM_KEYUP:		keyboard.keyUp((Key::Name)wParam);							return 0;
			case WM_CHAR:		keyboard.input(wParam);										return 0;
			case WM_LBUTTONDOWN:mouse.SignalButtonDown(0); installHook();							return 0;
			case WM_LBUTTONUP:  mouse.SignalButtonUp(0);											return 0;
			case WM_MBUTTONDOWN:mouse.SignalButtonDown(1); installHook();							return 0;
			case WM_MBUTTONUP:  mouse.SignalButtonUp(1);											return 0;
			case WM_RBUTTONDOWN:mouse.SignalButtonDown(2); installHook();							return 0;
			case WM_RBUTTONUP:  mouse.SignalButtonUp(2);											return 0;
			case WM_XBUTTONDOWN:
				//cout << wParam<<" / "<<HIWORD (wParam)<<" == "<<VK_XBUTTON1<<endl;
				mouse.SignalButtonDown(HIWORD (wParam)==1?3:4); installHook();	return 0;
			case WM_XBUTTONUP:	mouse.SignalButtonUp(HIWORD (wParam)==1?3:4);			return 0;
			case WM_MOUSEWHEEL: mouse.HandleMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam)/ WHEEL_DELTA);	return 0;
			case WM_MOVE:
			{
				WINDOWINFO	info;
				if (GetWindowInfo(context.hWnd,&info))
				{
					context._location = info.rcWindow;
					context.client_area = info.rcClient;
					mouse.RedefineWindow(context.client_area,context.hWnd);
				}
			}
			break;
			case WM_MENUCHAR:
				if( LOWORD(wParam) & VK_RETURN )
					return MAKELRESULT(0, MNC_CLOSE);//MNC_CLOSE (close the menu), is going fullscreen anyway..
				return MAKELRESULT(0, MNC_IGNORE);
			case WM_QUERYENDSESSION:
				return TRUE;
			//case WM_DROPFILES:
			//if (context.acceptDroppedFiles)
			//{
			//	HDROP hDrop = (HDROP)wParam;
			//	UINT cnt = DragQueryFile(hDrop,0xFFFFFFFF,NULL,0);
			//	if (cnt)
			//	{
			//		Array<String>	files(cnt);
			//		for (unsigned i = 0; i < cnt; i++)
			//		{
			//			unsigned len = DragQueryFile(hDrop,i,NULL,0);
			//			files[i].Resize(len);
			//			DragQueryFileA(hDrop,i,files[i].mutablePointer(),len+1);
			//			if (strlen(files[i].c_str()) != files[i].GetLength())
			//				FATAL__("File drop read error");
			//		}
			//		context.OnFileDrop(files);
			//	}
			//	DragFinish(hDrop);
			//}
			//break;
			//case WM_PAINT:	DefWindowProc(hWnd, Msg, wParam, lParam);						return 0;
			case WM_DESTROY:
			case WM_CLOSE:	  
			case WM_ENDSESSION:
				context.eventClose();								   			
			return 0;
			case WM_INPUT:
			{
				RAWINPUT input;
				UINT dwSize = sizeof(input);
			    if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &input, &dwSize, sizeof(RAWINPUTHEADER)))
				{
					if (input.header.dwType == RIM_TYPEMOUSE) 
					{
						if (input.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
							mouse.RecordAbsoluteMouseMovement(input.data.mouse.lLastX, input.data.mouse.lLastY);
						else
							mouse.RecordRelativeMouseMovement(input.data.mouse.lLastX, input.data.mouse.lLastY);
					}
					PRAWINPUT pinput = &input;
					return DefRawInputProc(&pinput,1,sizeof(RAWINPUTHEADER));
				}

			}
			break;
			default:
				return context.eventHook ? context.eventHook(hWnd,Msg,wParam,lParam) : DefWindowProcW(hWnd, Msg, wParam, lParam);
		}
		return context.eventHook ? context.eventHook(hWnd,Msg,wParam,lParam) : DefWindowProcW(hWnd, Msg, wParam, lParam);
		//return DefWindowProc(hWnd,Msg,wParam,lParam);
	}

	#elif SYSTEM==UNIX

	static Key::Name	symbolToCode(KeySym symbol, Key::Name fallback)
	{
		switch (symbol)
		{
			case XK_Home:
			case XK_KP_Home:
				return Key::Home;
			case XK_Left:
			case XK_KP_Left:
				return Key::Left;
			case XK_Up:
			case XK_KP_Up:
				return Key::Up;
			case XK_Right:
			case XK_KP_Right:
				return Key::Right;
			case XK_Down:
			case XK_KP_Down:
				return Key::Down;
			case XK_Page_Up:
			case XK_KP_Page_Up:
				return Key::PageUp;
			case XK_Page_Down:
			case XK_KP_Page_Down:
				return Key::PageDown;
			case XK_End:
			case XK_KP_End:
				return Key::End;
			case XK_Insert:
			case XK_KP_Insert:
				return Key::Insert;
			case XK_Print:
				return Key::Print;
			case XK_BackSpace:
				return Key::BackSpace;
			case XK_KP_Tab:
			case XK_Tab:
				return Key::Tab;
			case XK_Return:
				return Key::Return;
			case XK_Pause:
				return Key::Pause;
			case XK_Escape:
				return Key::Escape;
			case XK_Delete:
			case XK_KP_Delete:
				return Key::Delete;
			case XK_KP_Space:
				return Key::Space;
			case XK_KP_Enter:
				return Key::Enter;
			case XK_KP_Add:
				return Key::PadPlus;
			case XK_KP_Subtract:
				return Key::PadMinus;
			case XK_KP_Multiply:
				return Key::PadMult;
			case XK_KP_Decimal:
				return Key::PadDiv;
			case XK_KP_0:
			case XK_0:
				return Key::N0;
			case XK_KP_1:
			case XK_1:
				return Key::N1;
			case XK_KP_2:
			case XK_2:
				return Key::N2;
			case XK_KP_3:
			case XK_3:
				return Key::N3;
			case XK_KP_4:
			case XK_4:
				return Key::N4;
			case XK_KP_5:
			case XK_5:
				return Key::N5;
			case XK_KP_6:
			case XK_6:
				return Key::N6;
			case XK_KP_7:
			case XK_7:
				return Key::N7;
			case XK_KP_8:
			case XK_8:
				return Key::N8;
			case XK_KP_9:
			case XK_9:
				return Key::N9;
			case XK_F1:
				return Key::F1;
			case XK_F2:
				return Key::F2;
			case XK_F3:
				return Key::F3;
			case XK_F4:
				return Key::F4;
			case XK_F5:
				return Key::F5;
			case XK_F6:
				return Key::F6;
			case XK_F7:
				return Key::F7;
			case XK_F8:
				return Key::F8;
			case XK_F9:
				return Key::F9;
			case XK_F10:
				return Key::F10;
			case XK_F11:
				return Key::F11;
			case XK_F12:
				return Key::F12;
			case XK_Shift_L:
			case XK_Shift_R:
				return Key::Shift;
			case XK_Caps_Lock:
			case XK_Shift_Lock:
				return Key::Capital;
			case XK_Control_L:
			case XK_Control_R:
				return Key::Ctrl;
			case XK_Alt_L:
			case XK_Alt_R:
				return Key::Alt;
				
			case XK_space:
				return Key::Space;
			case XK_plus:
				return Key::Plus;
			case XK_comma:
				return Key::Comma;
			case XK_minus:
				return Key::Minus;
			case XK_period:
				return Key::Period;
			case XK_A:
			case XK_a:
				return Key::A;
			case XK_Adiaeresis:
			case XK_adiaeresis:
				return Key::AE;
			case XK_B:
			case XK_b:
				return Key::B;
			case XK_C:
			case XK_c:
				return Key::C;
			case XK_D:
			case XK_d:
				return Key::D;
			case XK_E:
			case XK_e:
				return Key::E;
			case XK_F:
			case XK_f:
				return Key::F;
			case XK_G:
			case XK_g:
				return Key::G;
			case XK_H:
			case XK_h:
				return Key::H;
			case XK_I:
			case XK_i:
				return Key::I;
			case XK_J:
			case XK_j:
				return Key::J;
			case XK_K:
			case XK_k:
				return Key::K;
			case XK_L:
			case XK_l:
				return Key::L;
			case XK_M:
			case XK_m:
				return Key::M;
			case XK_N:
			case XK_n:
				return Key::N;
			case XK_O:
			case XK_o:
				return Key::O;
			case XK_Odiaeresis:
			case XK_odiaeresis:
				return Key::OE;
			case XK_P:
			case XK_p:
				return Key::P;
			case XK_Q:
			case XK_q:
				return Key::Q;
			case XK_R:
			case XK_r:
				return Key::R;
			case XK_S:
			case XK_s:
				return Key::S;
			case XK_T:
			case XK_t:
				return Key::T;
			case XK_U:
			case XK_u:
				return Key::U;
			case XK_Udiaeresis:
			case XK_udiaeresis:
				return Key::UE;
			case XK_V:
			case XK_v:
				return Key::V;
			case XK_W:
			case XK_w:
				return Key::W;
			case XK_X:
			case XK_x:
				return Key::X;
			case XK_Y:
			case XK_y:
				return Key::Y;
			case XK_Z:
			case XK_z:
				return Key::Z;
			case XK_ssharp:
				return Key::SZ;
			case XK_dead_circumflex:
				return Key::Caret;
		}
		return fallback;			
	}

	void ExecuteEvent(XEvent&event)
	{
		switch (event.type)
		{
			case KeyPress:
			{
				Key::Name code = (Key::Name)event.xkey.keycode;
				KeySym symbol = XLookupKeysym(&event.xkey,input.pressed[Key::Shift]);
				code = symbolToCode(symbol,code);
				keyboard.keyDown(code);
				if (event.xkey.keycode >= 0x100 || !context.function_key_mask[(BYTE)code])
				{
					if (symbol != NoSymbol)
						keyboard.input(symbol);
				}

//				cout << "down: "<<event.xkey.keycode<<endl;
			}
			break;
			case KeyRelease:
			{
				Key::Name code = (Key::Name)event.xkey.keycode;
				KeySym symbol = XLookupKeysym(&event.xkey,input.pressed[Key::Shift]);
				code = symbolToCode(symbol,code);
				keyboard.keyUp(code);
	//			cout << "up: "<<event.xkey.keycode<<endl;
			}
			break;
			case ButtonPress:
			{
				unsigned id = event.xbutton.button-Button1;
				if (id < 3)
				{
					mouse.SignalButtonDown(id);
				}
				elif (id >= 5)
				{
					mouse.SignalButtonDown(id-2);
				}
				else
					mouse.HandleMouseWheel((1-(id-3)*2));
			}
			break;
			case ButtonRelease:
			{
				unsigned id = event.xbutton.button-Button1;
				if (id < 3)
				{
					mouse.SignalButtonUp(id);
				}
				elif (id >= 5)
				{
					mouse.SignalButtonUp(id-2);
				}
			}
			break;
			case FocusIn:	   context.restoreFocus();								 break;
			case FocusOut:	  context.looseFocus();								   break;
			case 0x00000021:
			case DestroyNotify: context.eventClose();								   break;
		}
	}
	#endif

}
