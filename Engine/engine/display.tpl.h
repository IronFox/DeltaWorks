#ifndef eveTplH
#define eveTplH


namespace Engine
{

	template <class GL>Display<GL>::Display():exec_target(NULL),context_error(false),region_locked(false),framebuffer_bound(false),resolution_overridden(false),config(default_buffer_config)
	{
		GL::globalInstance = this;
	}


	template <class GL> inline void Display<GL>::RenderToken()
	{
		RenderSomething();
	}

	template <class GL> inline void Display<GL>::RenderPivot()
	{
		RenderSomething();
	}
	
	template <class GL> void Display<GL>::RenderSomething()
	{
	    if (!pivot_defined)
	    {
	        pivot_defined = true;
	        pivot.select(SimpleGeometry::Triangles);
	        pivot.Resize(8);

	        pivot.color(1,0,0); pivot.vertex(0,0,1);
	        pivot.color(0,1,0); pivot.vertex(1,0,0);
	        pivot.color(0,0,1); pivot.vertex(0,1,0);

	        pivot.color(0,1,0); pivot.vertex(1,0,0);
	        pivot.color(1,1,0); pivot.vertex(0,0,-1);
	        pivot.color(0,0,1); pivot.vertex(0,1,0);

	        pivot.color(1,1,0); pivot.vertex(0,0,-1);
	        pivot.color(0,1,1); pivot.vertex(-1,0,0);
	        pivot.color(0,0,1); pivot.vertex(0,1,0);

	        pivot.color(0,1,1); pivot.vertex(-1,0,0);
	        pivot.color(1,0,0); pivot.vertex(0,0,1);
	        pivot.color(0,0,1); pivot.vertex(0,1,0);

	        pivot.color(0,1,0); pivot.vertex(1,0,0);
	        pivot.color(1,0,0); pivot.vertex(0,0,1);
	        pivot.color(0,0,1); pivot.vertex(0,-1,0);

	        pivot.color(1,1,0); pivot.vertex(0,0,-1);
	        pivot.color(0,1,0); pivot.vertex(1,0,0);
	        pivot.color(0,0,1); pivot.vertex(0,-1,0);

	        pivot.color(0,1,1); pivot.vertex(-1,0,0);
	        pivot.color(1,1,0); pivot.vertex(0,0,-1);
	        pivot.color(0,0,1); pivot.vertex(0,-1,0);

	        pivot.color(1,0,0); pivot.vertex(0,0,1);
	        pivot.color(0,1,1); pivot.vertex(-1,0,0);
	        pivot.color(0,0,1); pivot.vertex(0,-1,0);
	    }
	    GL::render(pivot);
	}



	
	#if SYSTEM==WINDOWS
	template <class GL>bool Display<GL>::Create(const DisplayConfig&dconfig)
	{

	    framebuffer_bound = false;
		resolution_overridden = false;
		while (true)
	    {
			HWND hWnd = context.createWindow(dconfig.window_name,dconfig.border_style, dconfig.onResize,dconfig.icon);
	        if (!hWnd)
	        {
	            context_error = true;
	            return false;
	        }
			if (!GL::CreateContext(hWnd,config,context.GetClientSize()))
	        {
	            context.destroyWindow();
	            if (GL::GetErrorCode() == ERR_RETRY)
	                continue;
	            context_error = false;
	            return false;
	        }
			Resolution res = context.GetClientSize();
	        GL::SetViewport(rect(0,res.height,res.width,0),res);
	        return true;
	    }

	}
	#elif SYSTEM==LINUX
	template <class GL>bool Display<GL>::Create(const DisplayConfig&dconfig)
	{
	    framebuffer_bound = false;
	    ::Display*connection = context.connect();
	    if (!connection)
	    {
	        context_error = true;
	        return false;
	    }
	    while (true)
	    {
	        TWindowAttributes attributes;
	        if (!GL::CreateContext(connection,config,attributes))
	        {
	            if (GL::GetErrorCode() == ERR_RETRY)
	                continue;
	            context.disconnect();
	            context_error = false;
	            return false;
	        }
			Window wnd = context.createWindow(dconfig.window_name,attributes,dconfig.border_style, dconfig.onResize,dconfig.icon);
	        //Window wnd = context.createWindow(dconfig.window_name,attributes,dconfig.hide_border,dconfig.icon_filename);
	        if (!wnd)
	        {
	            GL::DestroyContext();
	            context_error = true;
	            context.disconnect();
	            return false;
	        }
	        if (!GL::BindContext(wnd))
	        {
	            context_error = false;
	            context.destroyWindow();
	            GL::DestroyContext();
	            context.disconnect();
	            context_error = false;
	            return false;
	        }
			Resolution res = context.GetClientSize();
	        GL::SetViewport(rect(0,res.height,res.width,0),res);

	        //current_target_resolution = 
			//window_client_resolution = res;
			//pixel_aspect = window_client_resolution.GetPixelAspect();

	        return true;
	    }
	}
	#endif


	template <class GL>String Display<GL>::GetErrorStr()
	{
	    if (context_error)
	        return "Context returns ("+IntToStr(context.errorCode())+"): "+String(context.errorStr());
	    return "System "+String(GL::GetName())+" returns ("+IntToStr(GL::GetErrorCode())+"): "+String(GL::GetErrorStr());
	}


	template <class GL> inline bool Display<GL>::HideCursor()
	{
	    return mouse.HideCursor();
	}

	template <class GL> inline void Display<GL>::ShowCursor()
	{
	    mouse.ShowCursor();
	}

	template <class GL> inline  void  Display<GL>::PositionWindow(unsigned left, unsigned top, unsigned width, unsigned height)
	{
		SignalWindowResize(context.PositionWindow(left,top,width,height));
	}



	template <class GL>	inline	void	Display<GL>::OverrideSetClientResolution(const Resolution&region)
	{
		overridden_client_resolution = region;
		resolution_overridden = true;
	}

	template <class GL> inline  void Display<GL>::PositionWindow(const RECT&rect)
	{
		SignalWindowResize(context.PositionWindow(rect));
	}

	template <class GL> inline /*static*/ void Display<GL>::SetBorderStyle(DisplayConfig::border_style_t style)
	{
		context.SetBorderStyle(style);
	}

	template <class GL> inline /*static*/ DisplayConfig::border_style_t Display<GL>::GetBorderStyle()
	{
		return context.GetBorderStyle();
	}

	template <class GL> inline  void Display<GL>::ResizeWindow(unsigned width, unsigned height)
	{
		SignalWindowResize(context.ResizeWindow(width,height));
	}

	template <class GL> inline void Display<GL>::SignalWindowResize(UINT32 flags)
	{
		//OverrideSetClientResolution(context.GetClientSize());
		if (flags & DisplayConfig::ResizeDragHasEnded)
		{
			Resolution res = GetTargetResolution();
			GL::SignalWindowResize(res);
			GL::SetViewport(rect(0,res.height,res.width,0),res);
		}
	}



	template <class GL>
		inline	void		Display<GL>::SetSize(unsigned width, unsigned height)
		{
			ResizeWindow(width,height);
		}

	template <class GL>
		/*static*/ inline  const RECT&			Display<GL>::GetPreMaxiMinimizeLocation()
		{
			return context.GetPreMaxiMinimizeLocation();
		}
	template <class GL>
		/*static*/ inline  const Resolution&	Display<GL>::GetPreMaxiMinimizeSize()
		{
			return context.GetPreMaxiMinimizeSize();
		}


	template <class GL>
		/*static*/ inline  const RECT& Display<GL>::GetWindowLocation()
		{
			return context.GetWindowLocation();
		}

	template <class GL> inline
		/*static*/ unsigned			Display<GL>::GetWidth()
		{
			const RECT&	dim = context.GetWindowLocation();
			return dim.right-dim.left;
		}

	template <class GL> inline
		UINT			Display<GL>::GetClientWidth()	const
		{
			return resolution_overridden ? overridden_client_resolution.width : context.GetClientWidth();
		}
	
	template <class GL> inline
		UINT			Display<GL>::GetClientHeight()	const
		{
			return resolution_overridden ? overridden_client_resolution.height : context.GetClientHeight();
		}
	
		

	template <class GL> inline
		/*static*/ unsigned			Display<GL>::GetHeight()
		{
			const RECT& dim = context.GetWindowLocation();
			return dim.bottom-dim.top;
		}

	template <class GL> inline
		Resolution				Display<GL>::GetClientSize()	const
		{
			return resolution_overridden ? overridden_client_resolution : context.GetClientSize();
		}
		
	template <class GL>
		inline Resolution					Display<GL>::GetTargetResolution()	const
		{
			if (framebuffer_bound)
				return target_buffer_resolution;
			if (resolution_overridden)
				return overridden_client_resolution;
			return context.GetClientSize();
		}
		
	template <class GL> inline
		/*static*/ Resolution				Display<GL>::GetSize()
		{
			return context.GetWindowSize();
		}

	template <class GL> inline
		float				Display<GL>::GetPixelAspect()	const
		{
			if (framebuffer_bound)
				return target_buffer_resolution.GetPixelAspect();
			if (resolution_overridden)
				return overridden_client_resolution.GetPixelAspect();
			return context.pixelAspectf();
		}


	template <class GL> inline
		/*static*/ Resolution				Display<GL>::GetDimension()
		{
			return context.GetWindowSize();
		}

	template <class GL> inline
		/*static*/ Resolution				Display<GL>::GetDimensions()
		{
			return context.GetWindowSize();
		}





	template <class GL> inline void Display<GL>::SetScreen(const TDisplayMode&mode)
	{
	    context.SetScreen(mode);
		OverrideSetClientResolution(context.GetClientSize());
	}

	template <class GL> inline void Display<GL>::QueryScreen(ResolutionList*r_list, FrequencyList*f_list, DWORD w_min, DWORD h_min, DWORD f_min)
	{
	    context.QueryScreen(r_list,f_list,w_min,h_min,f_min);
	}

	template <class GL>
		/*static*/ inline Resolution		Display<GL>::GetScreenSize()
		{
			return context.GetScreenSize();
		}

	
	#if SYSTEM==WINDOWS

	template <class GL>
		/*static*/ inline bool Display<GL>::GetScreen(DEVMODE&mode)
		{
			return context.GetScreen(mode);
		}

	template <class GL>
		/*static*/ inline bool Display<GL>::GetScreen(DEVMODE*mode)
		{
			return context.GetScreen(mode);
		}

	template <class GL>
		/*static*/ inline bool Display<GL>::IsCurrent(const DEVMODE&screen)
		{
			return context.IsCurrent(screen);
		}

	template <class GL>
		/*static*/ inline void						Display<GL>::AcceptFileDrop()
		{
			context.AcceptFileDrop();
		}
	template <class GL>
		/*static*/ inline void						Display<GL>::AcceptFileDrop(const DragEventHandler&dragHandler, const DropEventHandler&dropHandler)
		{
			context.AcceptFileDrop(dragHandler, dropHandler);
		}
	template <class GL>
		/*static*/ inline void						Display<GL>::BlockFileDrop()
		{
			context.BlockFileDrop();
		}


	#elif SYSTEM==UNIX

	template <class GL> inline  int Display<GL>::FindScreen(DWORD width, DWORD height, DWORD&refresh_rate)
	{
	    return context.FindScreen(width,height,refresh_rate);
	}

/*
	template <class GL> inline const XRRScreenSize&   Display<GL>::GetScreen(unsigned index)
	{
	    return context.GetScreen(index);
	}

	template <class GL> inline bool Display<GL>::GetScreen(XRRScreenSize*size)
	{
	    return context.GetScreen(size);
	}

	template <class GL> inline bool Display<GL>::GetScreen(XRRScreenSize&size)
	{
	    return context.GetScreen(size);
	}

	template <class GL> inline bool Display<GL>::IsCurrent(const XRRScreenSize&size)
	{
	    return context.IsCurrent(size);
	}
*/


	#endif

	template <class GL> inline short Display<GL>::GetRefreshRate()
	{
	    return context.GetRefreshRate();
	}

	template <class GL> inline bool Display<GL>::IsMaximized()	{return context.WindowIsMaximized();}
	template <class GL> inline void Display<GL>::Maximize()		{context.MaximizeWindow();}
	template <class GL> inline bool	Display<GL>::IsMinimized()	{return context.WindowIsMinimized();}
	template <class GL> inline void Display<GL>::Minimize()		{context.MinimizeWindow();}
	template <class GL> inline void Display<GL>::Restore()		{context.RestoreWindow();}




	template <class GL> inline bool Display<GL>::ApplyScreen()
	{
	    bool result = context.ApplyScreen();
		OverrideSetClientResolution(context.GetClientSize());
	    return result;
	}


	template <class GL> inline  bool Display<GL>::ApplyWindowScreen(DWORD refresh_rate)
	{
	    bool result = context.ApplyWindowScreen(refresh_rate);
		OverrideSetClientResolution(context.GetClientSize());
	    return result;
	}

	template <class GL> inline  void Display<GL>::ResetScreen()
	{
	    context.ResetScreen();
		OverrideSetClientResolution(context.GetClientSize());
	}

	template <class GL> inline	void	Display<GL>::Capture(Image&target)
	{
		if (!framebuffer_bound)
		{
			if (resolution_overridden)
				target.SetSize(overridden_client_resolution.width,overridden_client_resolution.height,config.alpha_buffer_bits?4:3);
			else
				target.SetSize(context.GetClientWidth(),context.GetClientHeight(),config.alpha_buffer_bits?4:3);
		}
		else
			target.SetSize(target_buffer_resolution.width,target_buffer_resolution.height,framebuffer_alpha?4:3);
		GL::Capture(target);
	}
	template <class GL> inline	void	Display<GL>::Capture(FloatImage&target)
	{
		if (!framebuffer_bound)
		{
			if (resolution_overridden)
				target.SetSize(overridden_client_resolution.width,overridden_client_resolution.height,config.alpha_buffer_bits?4:3);
			else
				target.SetSize(context.GetClientWidth(),context.GetClientHeight(),config.alpha_buffer_bits?4:3);
		}
		else
			target.SetSize(target_buffer_resolution.width,target_buffer_resolution.height,framebuffer_alpha?4:3);
		GL::Capture(target);
	}

	template <class GL> inline	void	Display<GL>::Capture(typename GL::Texture&target)
	{
		Resolution res = GetTargetResolution();
		GL::Capture(target,res.width,res.height);
	}

	template <class GL> inline	void	Display<GL>::CaptureDepth(typename GL::Texture&target)
	{
		Resolution res = GetTargetResolution();
		GL::CaptureDepth(target,res.width,res.height);
	}


	template <class GL>void Display<GL>::Assign(pEngineExec target)
	{
	    exec_target = target;
	//    context.Assign(target);
	}

	template <class GL>void Display<GL>::InterruptCheckEvents()
	{
        #if SYSTEM==WINDOWS
            MSG msg;
            while (PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
            {
                if (!GetMessage(&msg, NULL,0,0))
                    FATAL__("event-handling error");
                TranslateMessage(&msg);
                Process(msg);
//                    DispatchMessage(&msg);
            }
        #elif SYSTEM==UNIX
            XEvent event;
            while (!context.shutting_down && XPending(context.connection()))
            {
                if (XNextEvent(context.connection(),&event))
                    FATAL__("event-handling error");
                Process(event);
            }
        #endif

	}

	template <class GL>void Display<GL>::Execute()
	{
	    if (!exec_target)
	        return;
	    bool exec_loop = true;
	    #if SYSTEM==UNIX
	        ::Display*connection = context.connection();
	        if (!connection)
	            return;
	    #endif
		BYTE focus_check = 1;
		BYTE callAnyway = 0;
	    while (exec_loop)
	    {
			if (!--focus_check)
			{
				focus_check = 5;
				if (mouse.IsLocked())
					context.checkFocus();
			}
			if (context.isMinimized())
			{
				#if SYSTEM==UNIX
					usleep(50000);
				#else
					Sleep(50.f);
				#endif
				if (++callAnyway > 10)
				{
					callAnyway = 0;
					timing.Update();
					exec_loop = !context.shutting_down && exec_target();
				}
			}
			else
			{
				timing.Update();
				exec_loop = !context.shutting_down && exec_target();
			}

	        mouse.Update();	//set here to reset motion
			InterruptCheckEvents();
	        GL::NextFrame();
	    }
	}

	template <class GL>void Display<GL>::ExecuteNoClr()
	{
	    if (!exec_target)
	        return;
	    bool exec_loop = true;
	    #if SYSTEM==UNIX
	        ::Display*connection = context.connection();
	        if (!connection)
	            return;
	    #endif
		BYTE focus_check = 1;
		BYTE callAnyway = 0;

	    while (exec_loop)
	    {
			if (!--focus_check)
			{
				focus_check = 5;
				if (mouse.IsLocked())
					context.checkFocus();
			}
			if (context.isMinimized())
			{
				#if SYSTEM==UNIX
					usleep(50000);
				#else
					Sleep(50.f);
				#endif
				if (++callAnyway > 10)
				{
					callAnyway = 0;
					timing.Update();
					exec_loop = !context.shutting_down && exec_target();
				}
			}
			else
			{
				timing.Update();
				exec_loop = !context.shutting_down && exec_target();
			}


	        mouse.Update();

	        #if SYSTEM==WINDOWS
	            MSG msg;
	            while (PeekMessageW(&msg,NULL,0,0,PM_NOREMOVE))
	            {
	                if (!GetMessageW(&msg, NULL,0,0))
	                    FATAL__("event-handling error");
	                TranslateMessage(&msg);
	                Process(msg);
	//                    DispatchMessage(&msg);
	            }
	        #elif SYSTEM==UNIX
	            XEvent event;
	            while (XPending(connection))
	            {
	                if (!XNextEvent(connection,&event))
	                    FATAL__("event-handling error");
	                Process(event);
	            }
	        #endif

	        GL::NextFrameNoClr();
	    }
	}

	#if SYSTEM==WINDOWS
	    template <class GL>inline void Display<GL>::Process(const MSG&msg)
	    {
	        if ((msg.message == WM_DESTROY || msg.message == WM_CLOSE || msg.message == WM_QUIT) && !context.shutting_down)
	            Destroy();
	        else
	            DispatchMessageW(&msg);
	    }

	#elif SYSTEM==UNIX

	    template <class GL>inline void Display<GL>::Process(XEvent&event)
	    {
	        if (event.type == DestroyNotify && !context.shutting_down)
	            Destroy();
	        else
	            Engine::ExecuteEvent(event);
	    }

	#endif

	template <class GL>void Display<GL>::Destroy()
	{
	    TargetBackbuffer();
	    GL::DestroyContext();
	    context.close();
		mouse.Release();
	}

	template <class GL>
		void Display<GL>::Terminate()
		{
			Destroy();
		}

	template <class GL> void Display<GL>::LockRegion()
	{
	    region_locked = true;
	}

	template <class GL> void Display<GL>::UnlockRegion()
	{
	    region_locked = false;
	}


	template <class GL> /*static*/ void Display<GL>::RegisterFocusCallbacks(const FCallback&onFocusLost, const FCallback&onFocusRestored)
	{
		context.RegisterFocusCallbacks(onFocusLost,onFocusRestored);
	}


	/**
		@brief Multi-window specific method. Allows to transform a float viewport to the corresponding pixel-region.

		Uses the locally stored dimensions if a frame buffer object is currently bound, the specified client size otherwise

		@param rect Viewport to transform
		@param clientSize Size of the client viewport (in pixels) to use, if no frame buffer object is currently bound
	*/
	template <class GL> RECT Display<GL>::TransformViewport(const M::TFloatRect&rect, const Resolution&client_size)
	{
		if (framebuffer_bound)
			return Transform(rect);

	    RECT result;
	    result.left     = (LONG)(rect.x.min       *client_size.width);
	    result.right    = (LONG)(rect.x.max      *client_size.width);
	    result.top      = (LONG)((rect.y.max)      *client_size.height);
	    result.bottom   = (LONG)((rect.y.min)   *client_size.height);
	    return result;

	}

	template <class GL> RECT Display<GL>::Transform(const M::TFloatRect&rect)
	{
		Resolution res = GetTargetResolution();
		return Transform(rect,res);
	}

	template <class GL> RECT Display<GL>::Transform(const M::TFloatRect&rect, const Resolution&res)
	{
	    RECT result;
	    result.left     = (LONG)(rect.x.min		*res.width);
	    result.right    = (LONG)(rect.x.max		*res.width);
	    result.top      = (LONG)((rect.y.max)	*res.height);
	    result.bottom   = (LONG)((rect.y.min)	*res.height);
	    return result;
	}


	template <class GL> void Display<GL>::PickRegion(const M::TFloatRect&rect)
	{
		Resolution res = GetTargetResolution();
	    GL::SetViewport(Transform(rect,res),res);
	}


	template <class GL>
	template <class C>void Display<GL>::Pick(const Aspect<C>&aspect)
	{
	    if (!region_locked)
		{
			Resolution res = GetTargetResolution();
	        GL::SetViewport(Transform(aspect.region,res),res);
		}
		if (GlobalAspectConfiguration::loadAsProjection)
		{
			M::TMatrix4<>	view_projection;
			M::Mat::Mult(aspect.projection,aspect.view,view_projection);
			GL::SetCameraMatrices(M::Matrix<>::eye4,view_projection,M::Matrix<>::eye4);
		}
		else
		{
			GL::SetCameraMatrices(aspect.view,aspect.projection,aspect.viewInvert);
		}
		GL::setDepthTest(aspect.depthTest);
		environment_matrix.x.xyz = aspect.viewInvert.x.xyz;
		environment_matrix.y.xyz = aspect.viewInvert.y.xyz;
		environment_matrix.z.xyz = aspect.viewInvert.z.xyz;
		cameraLocation = aspect.viewInvert.w.xyz;
	}

	template <class GL>
	template <class C>void Display<GL>::PickCentered(const Aspect<C>&aspect)
	{
	    if (!region_locked)
		{
			Resolution res = GetTargetResolution();
	        GL::SetViewport(Transform(aspect.region,res),res);
		}
	    M::TMatrix4<C>   view;
		view.x = aspect.view.x;
		view.y = aspect.view.y;
		view.z = aspect.view.z;
	    M::Vec::def(view.w,0,0,0,1);

		if (GlobalAspectConfiguration::loadAsProjection)
		{
			M::TMatrix4<>	view_projection;
			M::Mat::Mult(aspect.projection,view,view_projection);
			GL::SetCameraMatrices(M::Matrix<>::eye4,view_projection,M::Matrix<>::eye4);
		}
		else
		{
			GL::SetCameraMatrices(view,aspect.projection,aspect.viewInvert);
		}
		GL::setDepthTest(aspect.depthTest);

		environment_matrix.x.xyz = aspect.viewInvert.x.xyz;
		environment_matrix.y.xyz = aspect.viewInvert.y.xyz;
		environment_matrix.z.xyz = aspect.viewInvert.z.xyz;
		cameraLocation = M::Vector3<>::zero;
	}

	template <class GL>
	bool Display<GL>::TargetFBO(const typename GL::FBO&pobj)
	{
	    if (pobj.IsEmpty())
		{
			TargetBackbuffer();
	        return false;
		}
	    if (GL::TargetFBO(pobj))
	    {
			framebuffer_bound = true;
	        target_buffer_resolution = pobj.GetResolution();
			framebuffer_alpha = pobj.PrimaryHasAlpha();
	        //pixelAspect = current_target_resolution.GetPixelAspect();
			//ShowMessage("pbuffer bound. region is "+String(region_size.x)+", "+String(region_size.y)+". aspect is "+String(pixelAspect));

			GL::SetViewport(rect(0,target_buffer_resolution.height,target_buffer_resolution.width,0),target_buffer_resolution);	        
			return true;
	    }
	    TargetBackbuffer();
	    return false;
	}


	template <class GL>
	void Display<GL>::TargetBackbuffer()
	{
	    if (!framebuffer_bound)
			return;
		
	    framebuffer_bound = false;
	    GL::TargetBackbuffer();
		Resolution res = GetClientSize();
	    GL::SetViewport(rect(0,res.height,res.width,0),res);
	}


	template <class GL> SimpleGeometry					Display<GL>::pivot;
	template <class GL> bool                        Display<GL>::pivot_defined=false;

}

#endif
