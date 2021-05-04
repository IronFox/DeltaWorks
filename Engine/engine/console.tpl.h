#ifndef engine_consoleTplH
#define engine_consoleTplH

/******************************************************************

Language/Font-independed console.

******************************************************************/

namespace Engine
{

	template <class GL, class Font>
		VisualConsole<GL,Font>::Section::Section():relative(0,0,1,1),absolute(0,0,1,1),current(0,0,1,1)
		{
		    M::Vec::clear(trans0);
		    M::Vec::clear(trans1);
		    M::Vec::clear(translation);
		}

	template <class GL, class Font>
		void VisualConsole<GL,Font>::Section::setPrimaryTranslation(float x, float y)
		{
		    M::Vec::def(trans0,-x,-y);
		}

	template <class GL, class Font>
		void VisualConsole<GL,Font>::Section::setSecondaryTranslation(float x, float y)
		{
		    M::Vec::def(trans1,-x,-y);
		}

	template <class GL, class Font>
		void VisualConsole<GL,Font>::Section::updateState(float state)
		{
		    if (state < 0.5)
		        M::Vec::mad(trans1,trans0,1-state*2,translation);
		    else
		        M::Vec::mult(trans1,1-(state-0.5)*2,translation);
			absolute.Translate(translation);
		    //M::Vec::add(absolute.min(),translation,current.dim);
		    //M::Vec::add(absolute.dim+2,translation,current.dim+2);
		}

	template <class GL, class Font>
		void VisualConsole<GL,Font>::Section::updateAbsolutePosition(const M::TFloatRect&parentLocation)
		{
			parentLocation.MakeAbsolute(relative,absolute);
		    //absolute.x0 = x + width*relative.x0;
		    //absolute.y0 = y + height*relative.y0;
		    //absolute.x1 = x + width*relative.x1;
		    //absolute.y1 = y + height*relative.y1;
		}

	template <class GL, class Font>
		void VisualConsole<GL,Font>::Section::render(GL*renderer)
		{
		    if (primary.IsEmpty())
		        return;
		    Engine::TColorTextureVertex v0,v1,v2,v3;
		    M::Vec::def(v0,current.x.min,current.y.min,0,1);
		    M::Vec::def(v1,current.x.max,current.y.min,0,1);
		    M::Vec::def(v2,current.x.max,current.y.max,0,1);
		    M::Vec::def(v3,current.x.min,current.y.max,0,1);
		    M::Vec::set(v0.color,1);
		    M::Vec::set(v1.color,1);
		    M::Vec::set(v2.color,1);
		    M::Vec::set(v3.color,1);
		    M::Vec::def(v0.coord,0,0);
		    M::Vec::def(v1.coord,1,0);
		    M::Vec::def(v2.coord,1,1);
		    M::Vec::def(v3.coord,0,1);
		    if (!shiny.IsEmpty())
		    {
		        v0.color.a = 0.3-cos(timing.now)*0.25;
		        v1.color.a = 0.3-cos(timing.now+2)*0.25;
		        v2.color.a = 0.3-cos(timing.now+3)*0.25;
		        v3.color.a = 0.3-cos(timing.now+1)*0.25;
		    }

		    renderer->useTexture(primary,true);
		    renderer->face(v0,v1,v2,v3);
		    if (!renderer->useTexture(shiny,true))
		        return;
		    v0.color.a = 0.5+cos(timing.now)*0.5;
		    v1.color.a = 0.5+cos(timing.now+2)*0.5;
		    v2.color.a = 0.5+cos(timing.now+3)*0.5;
		    v3.color.a = 0.5+cos(timing.now+1)*0.5;
		    renderer->face(v0,v1,v2,v3);
		}


	template <class GL, class Font>
		void VisualConsole<GL,Font>::updateSectionPositions()
		{
		    body.updateAbsolutePosition(location);
		    body.updateState(status);
		    head.updateAbsolutePosition(location);
		    head.updateState(status);
		    foot.updateAbsolutePosition(location);
		    foot.updateState(status);
		    if (text_target)
		    {
				text_target->current.MakeAbsolute(relative_text,absolute_text);
		        // absolute_text.x.min = text_target->current.x0 + (text_target->current.x1-text_target->current.x0)*relative_text.left;
		        // absolute_text.right = text_target->current.x0 + (text_target->current.x1-text_target->current.x0)*relative_text.right;
		        // absolute_text.top = text_target->current.y0 + (text_target->current.y1-text_target->current.y0)*relative_text.top;
		        // absolute_text.y.min = text_target->current.y0 + (text_target->current.y1-text_target->current.y0)*relative_text.bottom;
		    }
		}




	template <class GL, class Font>
	VisualConsole<GL,Font>::VisualConsole():renderer(GL::global_instance),
	                                location(0,0,1,1),font_x(0.05),font_y(0.1),
	                                history_state(0),text_target(0),relative_text(0,0,1,1),absolute_text(0,0,1,1)
		{
		    textout.setScale(font_x,font_y);
		    textout.MoveTo(location.x.min,location.y.max+EVE_PANEL_SIZE*location.GetHeight());
		}

	template <class GL, class Font>
		VisualConsole<GL,Font>::VisualConsole(GL*renderer_):renderer(renderer_),
	                                location(0,0,1,1),font_x(0.05),font_y(0.1),
	                                history_state(0),text_target(0),relative_text(0,0,1,1),absolute_text(0,0,1,1)
		{
		    textout.setScale(font_x,font_y);
		    textout.MoveTo(location.x.min,location.y.max+EVE_PANEL_SIZE*location.GetHeight());
		}

	template <class GL, class Font>
		VisualConsole<GL,Font>::VisualConsole(GL&renderer_):renderer(&renderer_),
	                                location(0,0,1,1),font_x(0.05),font_y(0.1),
	                                history_state(0),text_target(0),relative_text(0,0,1,1),absolute_text(0,0,1,1)
		{
		    textout.setScale(font_x,font_y);
		    textout.MoveTo(location.x.min,location.y.max+EVE_PANEL_SIZE*location.GetHeight());
		}

	template <class GL, class Font>
		void		VisualConsole<GL,Font>::setRenderer(GL*renderer_)
		{
			renderer = renderer_;
		}
		
	template <class GL, class Font>
		void		VisualConsole<GL,Font>::setRenderer(GL&renderer_)
		{
			renderer = &renderer_;
		}
		
	template <class GL, class Font>
		GL*		VisualConsole<GL,Font>::getRenderer()	const
		{
			return renderer;
		}


	template <class GL, class Font>
		void VisualConsole<GL,Font>::setFinalRegion(float x_, float y_, float width_, float height_)
		{
			location.Set(x_,y_,width_,height_);
		    // x = x_;
		    // y = y_;
		    // width = width_;
		    // height = height_;
		    
		    updateSectionPositions();
		}

	template <class GL, class Font>
		void VisualConsole<GL,Font>::setFontSize(float fx, float fy)
		{
		    font_x = fx;
		    font_y = fy;
		    textout.setScale(fx,fy);
		}

	template <class GL, class Font>
		void VisualConsole<GL,Font>::setTexture(eConsoleSection section, const Texture&texture)
		{
		    getSec(section).primary = texture;
		}

	template <class GL, class Font>
		void VisualConsole<GL,Font>::setTextures(eConsoleSection section, const Texture&primary, const Texture&shiny)
		{
		    Section&sec = getSec(section);
		    sec.primary = primary;
		    sec.shiny = shiny;
			if (primary.IsEmpty())
				ErrMessage("Warning: Primary section texture is empty for Console");
			if (shiny.IsEmpty())
				ErrMessage("Warning: Secondary section texture is empty for Console");
		}

	template <class GL, class Font>
		void VisualConsole<GL,Font>::setPrimaryTranslation(float x, float y, unsigned section_mask)
		{
		    if (section_mask&Head)
		        head.setPrimaryTranslation(x,y);
		    if (section_mask&Body)
		        body.setPrimaryTranslation(x,y);
		    if (section_mask&Foot)
		        foot.setPrimaryTranslation(x,y);
		}

	template <class GL, class Font>
		void VisualConsole<GL,Font>::setSecondaryTranslation(float x, float y, unsigned section_mask)
		{
		    if (section_mask&Head)
		        head.setSecondaryTranslation(x,y);
		    if (section_mask&Body)
		        body.setSecondaryTranslation(x,y);
		    if (section_mask&Foot)
		        foot.setSecondaryTranslation(x,y);
		}

	template <class GL, class Font>
		void VisualConsole<GL,Font>::setRelativeTextureArea(eConsoleSection section, float left, float bottom, float right, float top)
		{
		    Section&sec = getSec(section);
		    sec.relative = M::TFloatRect(left,bottom,right,top);
		    sec.updateAbsolutePosition(location);
		    sec.updateState(status);
		}

	template <class GL, class Font>
		void VisualConsole<GL,Font>::setRelativeTextArea(eConsoleSection section, float left, float bottom, float right, float top)
		{
		    text_target = &getSec(section);
		    relative_text = M::TFloatRect(left,bottom,right,top);
			text_target->current.MakeAbsolute(relative_text,absolute_text);
		}



	template <class GL, class Font>
		Font& VisualConsole<GL,Font>::getFont()
		{
		    return textout.GetFont();
		}


	template <class GL, class Font>
		String VisualConsole<GL,Font>::Truncate(const String&origin)
		{
		    float len = absolute_text.GetWidth();
		    char buffer[0x100];
		    memcpy(buffer,origin.c_str(),M::Min(origin.GetLength(),sizeof(buffer)));
		    BYTE at = M::Min(origin.GetLength()+1,sizeof(buffer))-1;
		    buffer[at] = 0;

		    while (at && textout.GetUnscaledWidth(buffer)*font_x > len)
		        buffer[--at] = 0;

		    return buffer;
		}

	template <class GL, class Font>
		void VisualConsole<GL,Font>::printLine(const String&line)
		{
		    StringList list(line);
		    float len = absolute_text.GetWidth();
		    while (list.Count())
		    {
				bool split = false;
		        for (unsigned i = list.Count(); i; i--)
		        {
		            String comb = list.fuse(0,i,' ');
		            if (textout.GetUnscaledWidth(comb)*font_x <= len)
		            {
		                lines.add(comb);
		                list.erase(0,i);
						split = true;
		                break;
		            }
		        }
				if (!split)
				{
					String&el = list.First();
					while (el.GetLength())
					{
						bool printed=false;
						for (unsigned i = 1; i <= el.GetLength(); i++)
							if (textout.GetUnscaledWidth(el.c_str(),i)*font_x > len)
							{
								lines.add(el.subString(0,i));
								el.Erase(0,i);
								printed = true;
								break;
							}
						if (!printed)
						{
							lines.add(el);
							el = "";
						}
					}
					list.erase(0,1);
				}
		    }
		}

	template <class GL, class Font>
		void VisualConsole<GL,Font>::print(const String&line, bool word_wrap)
		{
		    String local = line;
		    while (local.GetLength())
		    {
		        unsigned nl = local.GetIndexOf('\n');
		        if (nl)
		        {
		            if (!word_wrap)
		                lines.add(Truncate(local.subString(0,nl-1)));
		            else
		                printLine(local.subString(0,nl-1));
		            local.Erase(0,nl);
		        }
		        else
		        {
		            if (!word_wrap)
		                lines.add(Truncate(local));
		            else
		                printLine(local);
		            return;
		        }
		    }
		}


	template <class GL, class Font>
		void VisualConsole<GL,Font>::render()
		{
			if (!renderer)
			{
				renderer = GL::global_instance;
				if (!renderer)
					FATAL__("renderer not specified");
			}
		    bool changed = false;
		    if (target_open && status < 1)
		    {
		        status += Engine::timing.delta*2.0;
		        if (status > 1)
		            status = 1;
		        changed = true;
		    }
		    if (!target_open && status > 0)
		    {
		        status -= Engine::timing.delta*2.0;
		        if (status < 0)
		            status = 0;
		        changed = true;
		    }
		    if (changed)
		    {
		        head.updateState(status);
		        body.updateState(status);
		        foot.updateState(status);
		        if (text_target)
		        {
					text_target->current.MakeAbsolute(relative_text,absolute_text);
		        }
		    }
		    
		    if (status > 0)
		    {
		        body.render(renderer);

		            unsigned    //block = status*height/line_height,
		                        frame_size = M::Clamp((int)((absolute_text.GetHeight())/font_y)-1,0,1000),
		                        offset = lines - M::Clamp(history_state*frame_size+frame_size,0,lines),
		                        count = lines - offset;
		            textout.MoveTo(absolute_text.x.min,absolute_text.y.min+font_y*(count+1));
		            for (unsigned i = 0; i < count; i++)
		            {
		                textout.line(i);
		                textout.print(lines[offset+i],'$');
		            }
		        float displace = clamped(textout.GetUnscaledWidth(keyboard.GetRawInput())*font_x-(absolute_text.GetWidth()),0,100000);

		        textout.MoveTo(absolute_text.x.min-displace,absolute_text.y.min+font_y);
		        textout.SetLine(0);
		        textout.Print(keyboard.GetRawInput());

		        if (Console::is_focused)
		        {
		            renderer->useTexture(NULL);
		            TColorVertex  v0,v1;
		            float x = absolute_text.x.min-displace+textout.GetUnscaledWidth(keyboard.GetRawInput())*font_x,
		                  y = absolute_text.y.min,
		                  light = cos(Engine::timing.now);
		            M::Vec::def(v0,x,y+font_y*0.1,0,1);
		            M::Vec::def(v1,x,y+font_y*0.9,0,1);
		            M::Vec::def(v0.color,1,1,1,0.6+0.2*light);
		            M::Vec::def(v1.color,1,1,1,0.6+0.2*light);
		            renderer->segment(v0,v1);
		        }
		    }
		    foot.render(renderer);
		    head.render(renderer);
		}



	template <class GL, class Font>
		Console::Action VisualConsole<GL,Font>::resolve(const float pointer[2])
		{
		    bool in_head = head.current.Contains(pointer),
		         in_body = body.current.Contains(pointer),
		         in_foot = foot.current.Contains(pointer);
		    if (!in_head && !in_body && !in_foot)
		        return Unfocus;
		    if (in_head)
		        return Toggle;
			if (is_focused && target_open)
				return NoAction;
		    return Focus;
		}

	template <class GL, class Font>
		void VisualConsole<GL,Font>::execute(Console::Action action)
		{
		    switch (action)
		    {
		        case Open:
		        case Close:
		            setTarget(action==Open);
		        break;
		        case Toggle:
		            setTarget(!target_open);
		        break;
		        case Focus:
		            focus();
		            setTarget(true);
		        break;
				case Unfocus:
		            killFocus();
		        break;
		    }
		}

	template <class GL, class Font>
		Console::Action VisualConsole<GL,Font>::resolveAndExecute(const float pointer[2])
		{
		    Action action = resolve(pointer);
		    execute(action);
		    return action;
		}

	template <class GL, class Font>
		void VisualConsole<GL,Font>::setState(bool open)
		{
			Console::setState(open);
		    updateSectionPositions();
		}

	//template <class GL, class Font>
	//	void	VisualConsole<GL,Font>::loadDefaults(TextureResource<GL>&archive, int window_width, int window_height, Key::Name close_key, const String&head_name, const String&head_light,const String&body,const String&foot, const String&foot_light)
	//	{
	//		float   wscale = 1024.0f/((float)window_width),
	//	            hscale = vabs(768.0f/((float)window_height));
	//	    setFinalRegion(0,1.0-0.5*hscale,0.4*wscale,0.5*hscale);
	//	    setFontSize(0.015*wscale,0.02*hscale);
	//	    setTextures(Console::Head,   archive.getTexture(head_name),
	//	                                    archive.getTexture(head_light));
	//	    setTexture(Console::Body,    archive.getTexture(body));
	//	    setTextures(Console::Foot,   archive.getTexture(foot),
	//	                                    archive.getTexture(foot_light));
	//	    setPrimaryTranslation(0.38*wscale,0,Console::Head|Console::Body|Console::Foot);
	//	    setSecondaryTranslation(0,-0.46*hscale,Console::Body|Console::Foot);
	//	    setRelativeTextureArea(Console::Head,0,0.85,1,1);
	//	    setRelativeTextureArea(Console::Body,0,0,1,1);
	//	    setRelativeTextureArea(Console::Foot,0,0,1,0.15);
	//	    setRelativeTextArea(Console::Body,0.01,0.05,0.94,0.95);
	//		bindCloseKey(close_key);
	//	}

}

#endif
