#include "components.h"
#include <string/encoding.h>


namespace Engine
{
	namespace GUI
	{
	
		Layout								Button::globalLayout,
											Panel::globalLayout,
											Edit::globalLayout,
											ComboBox::globalLayout,
											Menu::globalLayout;
		ScrollBarLayout						ScrollBarLayout::global;
		CheckBox::TStyle					CheckBox::globalStyle;
		SliderLayout						SliderLayout::global;

	
	
	
		bool		Panel::GetChildSpace(M::Rect<float>&outRect)	const
		{
			if (children.IsEmpty())
				return false;
			const PComponent&first = children.First();
			outRect = first->currentRegion;
				
			for (index_t i = 1; i < children.Count(); i++)
			{
				const PComponent&child = children[i];
				outRect.Include(child->currentRegion);
			}
			return true;
		}
		
		void			Panel::Append(const PComponent&component)
		{
			DBG_ASSERT__(!children.Contains(component));
			component->anchored.Set(true,false,false,true);
			if (children.IsNotEmpty())
				component->offset.top = children.Last()->offset.top-children.Last()->height;
			else
				component->offset.top = 0;
			component->offset.left = 0;
			component->SetWindow(windowLink);
			children << component;

			SignalLayoutChange();
		}
		
		void			Panel::AppendRight(const PComponent&component)
		{
			DBG_ASSERT__(!children.Contains(component));
			if (children.IsNotEmpty())
			{
				component->anchored = children.Last()->anchored;
				component->offset.top = children.Last()->offset.top;
				component->offset.left = children.Last()->offset.left+children.Last()->width;
			}
			else
			{
				component->anchored.Set(true,false,false,true);
				component->offset.top = 0;
				component->offset.left = 0;
			}
			component->SetWindow(windowLink);
			children << component;

			SignalLayoutChange();
		}
		
		bool					Panel::Add(const PComponent&component)
		{
			if (!component || children.Contains(component))
				return false;
			children << component;
			component->SetWindow(windowLink);
			SignalLayoutChange();
			return true;
		}
		
		bool					Panel::Erase(const PComponent&component)
		{
			if (children.findAndErase(component))
			{
				component->SetWindow(PWindow());
				SignalLayoutChange();
				return true;
			}
			return false;
		}
		
		bool					Panel::Erase(index_t index)
		{
			if (index < children.Count())
			{
				children[index]->SetWindow(PWindow());
				children.Erase(index);
				SignalLayoutChange();
				return true;
			}
			return false;
		}
		
		
		bool					Panel::MoveChildUp(const PComponent&component)
		{
			index_t index;
			if ((index = children.GetIndexOf(component)) != -1)
				return MoveChildUp(index);
			return false;
		}
		
		bool					Panel::MoveChildUp(index_t index)
		{
			if (index+1 >= children.Count())
				return false;
			Buffer<PComponent,4>::AppliedStrategy::swap(children[index],children[index+1]);
			SignalVisualChange();
			return true;
		}
		

		
		bool					Panel::MoveChildDown(const PComponent&component)
		{
			index_t index;
			if ((index = children.GetIndexOf(component)) != -1)
				return MoveChildDown(index);
			return false;
		}
		
		bool					Panel::MoveChildDown(index_t index)
		{
			if (!index || index >= children.Count())
				return false;
			Buffer<PComponent,4>::AppliedStrategy::swap(children[index-1],children[index]);
			SignalVisualChange();
			return true;
		}

		bool					Panel::MoveChildToTop(const PComponent&component)
		{
			index_t index;
			if ((index = children.GetIndexOf(component))!=-1)
				return MoveChildToTop(index);
			return false;
		}
		
		bool					Panel::MoveChildToTop(index_t index)
		{
			if (index+1 < children.Count())
			{
				PComponent cmp = children[index];
				children.Erase(index);
				children << cmp;
				SignalVisualChange();
				return true;
			}
			return false;
		}
		
		bool					Panel::MoveChildToBottom(const PComponent&component)
		{
			index_t index;
			if ((index = children.GetIndexOf(component)) != -1)
				return MoveChildToBottom(index);
			return false;
		}
		
		bool					Panel::MoveChildToBottom(index_t index)
		{
			if (index && index < children.Count())
			{
				PComponent cmp = children[index];
				children.Erase(index);
				children.Insert(0,cmp);
				SignalVisualChange();
				return true;
			}
			return false;
		}
		
		void		Panel::UpdateLayout(const M::Rect<float>&parent_region)
		{
			Component::UpdateLayout(parent_region);
			for (index_t i = 0; i < children.Count(); i++)
				children[i]->UpdateLayout(cellLayout.client);
		}
		
		float		Panel::GetClientMinWidth()	const
		{
			float rs = 0;
			
			for (index_t i = 0; i < children.Count(); i++)
			{
				float c = children[i]->GetMinWidth(true);
				if (c > rs)
					rs = c;
			}
			return rs;
		}
		
		float		Panel::GetClientMinHeight()	const
		{
			float rs = 0;
			
			for (index_t i = 0; i < children.Count(); i++)
			{
				float c = children[i]->GetMinHeight(true);
				if (c > rs)
					rs = c;
			}
			return rs;
		}
		
		/*virtual override*/ void		Panel::OnColorPaint(ColorRenderer&renderer, bool parentIsEnabled)
		{
			renderer.PushColor();
				Component::OnColorPaint(renderer,parentIsEnabled);
				bool subEnabled = parentIsEnabled && IsEnabled();
				renderer.Clip(cellLayout.client);
					for (index_t i = 0; i < children.Count(); i++)
						if (children[i]->IsVisible())
						{
							renderer.PeekColor();
							children[i]->OnColorPaint(renderer,subEnabled);
						}
				renderer.Unclip();
			renderer.PopColor();
		}
		
		/*virtual override*/ void		Panel::OnNormalPaint(NormalRenderer&renderer, bool parentIsEnabled)
		{
			renderer.PushNormalMatrix();
				Component::OnNormalPaint(renderer,parentIsEnabled);
				bool subEnabled = parentIsEnabled && IsEnabled();
			
				renderer.Clip(cellLayout.client);
					for (index_t i = 0; i < children.Count(); i++)
						if (children[i]->IsVisible())
						{
							renderer.PeekNormalMatrix();
							children[i]->OnNormalPaint(renderer,subEnabled);
						}
				renderer.Unclip();
			renderer.PopNormalMatrix();
		}

		/*virtual override*/ PComponent			Panel::GetComponent(float x, float y, ePurpose purpose, bool&outIsEnabled)
		{
			outIsEnabled &= IsEnabled();
			for (index_t i = children.Count()-1; i < children.Count(); i--)
			{
				const PComponent&child = children[i];
				if (child->IsVisible() && child->cellLayout.border.Contains(x,y))
					return child->GetComponent(x,y,purpose,outIsEnabled);
			}
			return shared_from_this();
		}

		
		

		
		void	ScrollBarLayout::LoadFromFile(const PathString&filename, float scale)
		{
			/*
				TTexture				backCenter,
										backBottom,
										cursorCenter,
										cursorBottom,
										bottomButton;
				float					buttonIndent;
			*/
			
			FileSystem::Folder	folder(FileSystem::ExtractFileDir(filename));
			
			XML::Container	xml;
			xml.LoadFromFile(filename);
				

			String attrib;
			
			XML::Node	*xback = xml.Find("scrollbar/background"),
						*xcursor = xml.Find("scrollbar/cursor"),
						*xbutton = xml.Find("scrollbar/button");
			if (!xback || !xcursor || !xbutton)
				throw Except::IO::DriveAccess::FileFormatFault(String(__func__)+ ": Required XML nodes scrollbar/background, scrollbar/cursor, and/or scrollbar/button not found");

			TTexture::load(xback,"center",folder,backCenter,scale);

			TTexture::load(xback,"bottom",folder,backBottom,scale);
			TTexture::load(xcursor,"center",folder,cursorCenter,scale);
			TTexture::load(xcursor,"bottom",folder,cursorBottom,scale);
			TTexture::load(xbutton,"background",folder,bottomButton,scale);

			if (!xbutton->Query("indent",attrib) || !convert(attrib.c_str(),buttonIndent))
				throw Except::IO::DriveAccess::FileFormatFault(String(__func__)+  ": Failed to find or parse scrollbar/button attribute 'indent'");
			
			
			buttonIndent *= scale;
			minWidth = M::Max(M::Max(M::Max(backCenter.width,backBottom.width),M::Max(cursorCenter.width,cursorBottom.width)),bottomButton.width);
			minHeight = 2*(bottomButton.height-buttonIndent+backBottom.height);
		}
		
		
		
		void	SliderLayout::LoadFromFile(const PathString&filename, float scale)
		{
			/*
				TTexture				backCenter,
										backBottom,
										cursorCenter,
										cursorBottom,
										bottomButton;
				float					buttonIndent;
			*/
			
			FileSystem::Folder	folder(FileSystem::ExtractFileDir(filename));
			
			XML::Container	xml;
			xml.LoadFromFile(filename);
				

			String attrib;
			
			XML::Node	*xbar = xml.Find("slider/bar"),
						*xslider = xml.Find("slider/slider");
			if (!xbar || !xslider)
				throw Except::IO::DriveAccess::FileFormatFault(String(__func__)+ ": Required XML nodes slider/bar and/or slider/slider not found");

			TTexture::load(xbar,"left",folder,barLeft,scale);
			TTexture::load(xbar,"center",folder,barCenter,scale);
			TTexture::load(xslider,"background",folder,slider,scale);

			minWidth = M::Max(2*barLeft.width+barCenter.width,slider.width);
			minHeight = M::Max(M::Max(barLeft.height,barCenter.height),slider.height);
		}
		
		
		
		
		
		
		
		
				
		void		Slider::_Setup()
		{
			cursorGrabbed = false;
			sliderLayout = &SliderLayout::global;
			current = 0;
			max = 1;
		}
		

		void		Slider::OnSlide()
		{
			onSlide();
		}
		
		
		
		
		void		Slider::_UpdateCursorRegion()
		{
			if (!sliderLayout)
				return;
			float width = sliderLayout->slider.width;
			slideFrom = cellLayout.client.left()+width/2;
			slideRange = cellLayout.client.x.max-cellLayout.client.left()-width;
			float center = slideFrom+current/max*slideRange;
			cursorRegion.left() = center-width/2;
			cursorRegion.x.max = center+width/2;
			cursorRegion.top() = cellLayout.client.y.center()+sliderLayout->slider.height/2;
			cursorRegion.bottom() = cursorRegion.top()-sliderLayout->slider.height;
			slider.region = cursorRegion;			
		}
		
		
		
		void		Slider::UpdateLayout(const M::Rect<float>&parent_region)
		{
			Component::UpdateLayout(parent_region);
			_UpdateCursorRegion();
			if (sliderLayout)
			{
				barLeft.region = cellLayout.client;
				barLeft.region.x.max = barLeft.region.left()+sliderLayout->barLeft.width;
				barLeft.region.top() = cellLayout.client.y.center()+sliderLayout->barLeft.height/2;
				barLeft.region.bottom() = barLeft.region.top()-sliderLayout->barLeft.height;
				barLeft.color = &sliderLayout->barLeft.color;
				barLeft.normal = &sliderLayout->barLeft.normal;
				barLeft.orientation = 0;
				
				barRight.region = cellLayout.client;
				barRight.region.left() = barRight.region.x.max-sliderLayout->barLeft.width;
				barRight.region.top() = cellLayout.client.y.center()+sliderLayout->barLeft.height/2;
				barRight.region.bottom() = barRight.region.top()-sliderLayout->barLeft.height;
				barRight.color = &sliderLayout->barLeft.color;
				barRight.normal = &sliderLayout->barLeft.normal;
				barRight.orientation = 2;
				
				
				barCenter.region = cellLayout.client;
				barCenter.region.left() = barLeft.region.x.max;
				barCenter.region.x.max = barRight.region.left();
				barCenter.region.top() = cellLayout.client.y.center()+sliderLayout->barCenter.height/2;
				barCenter.region.bottom() = barCenter.region.top()-sliderLayout->barCenter.height;
				barCenter.color = &sliderLayout->barCenter.color;
				barCenter.normal = &sliderLayout->barCenter.normal;
				barCenter.orientation = 0;
				
				//ShowMessage(barCenter.region.ToString());
				
				slider.region = cursorRegion;
				slider.color = &sliderLayout->slider.color;
				slider.normal = &sliderLayout->slider.normal;
				slider.orientation = 0;
			}
		}
		
		float		Slider::GetClientMinWidth()	const
		{
			if (!sliderLayout)
				return 0;
			return sliderLayout->minWidth;
		}
		
		float		Slider::GetClientMinHeight()	const
		{
			if (!sliderLayout)
				return 0;
			return sliderLayout->minHeight;
		}
		
		

		void		Slider::OnColorPaint(ColorRenderer&renderer, bool parentIsEnabled)
		{
			Component::OnColorPaint(renderer,parentIsEnabled);

			renderer.Paint(barLeft);
			renderer.Paint(barCenter);
			renderer.Paint(barRight);
			renderer.MarkNewLayer();
			renderer.Paint(slider);
		}
		
		void		Slider::OnNormalPaint(NormalRenderer&renderer, bool parentIsEnabled)
		{
			Component::OnNormalPaint(renderer,parentIsEnabled);

			//Display<OpenGL>&display = requireOperator()->getDisplay();
			renderer.Paint(barLeft,false);
			renderer.Paint(barCenter,false);
			renderer.Paint(barRight,false);
			renderer.MarkNewLayer();
			renderer.Paint(slider,cursorGrabbed);
		}
		
		Component::eEventResult	Slider::OnMouseDrag(float x, float y)
		{
			if (cursorGrabbed)
			{
				float	relative = x-cursorRegion.left(),
						delta = (relative-cursorHook[0])*max/slideRange;
				current = M::clamped(current+delta,0,max);
				
				_UpdateCursorRegion();
				
				OnSlide();
				return RequestingRepaint;
			}
			return Handled;
		}
		
		Component::eEventResult	Slider::OnMouseDown(float x, float y, TExtEventResult&ext)
		{
			if (!enabled)
				return Unsupported;
			if (cursorRegion.Contains(x,y) && max > 0)
			{
				cursorHook[0] = x-cursorRegion.left();
				cursorHook[1] = y-cursorRegion.bottom();
				cursorGrabbed = true;
				return RequestingRepaint;
			}
			return Unsupported;
				
		}
		
		Component::eEventResult		Slider::OnMouseUp(float x, float y)
		{
			bool changed =  cursorGrabbed;
			cursorGrabbed = false;
			return changed?RequestingRepaint:Handled;
		}
		
		Component::eEventResult		Slider::OnMouseWheel(float x, float y, short delta)
		{
			float new_current = M::Clamp01(current + delta/max);

			if (current != new_current)
			{
				current = new_current;
				_UpdateCursorRegion();
				OnSlide();
				return RequestingRepaint;
			}
			return Handled;
		}
		
		
		
		
		
		
		
		
		
		
				
		
		void		ScrollBar::_Setup()
		{
			scrollable.reset();
			horizontal = false;
			upPressed = false;
			downPressed = false;
			cursorGrabbed = false;
			scrollLayout = &ScrollBarLayout::global;
			autoVisibility = false;
		}

		void		ScrollBar::ScrollTo(float v)
		{
			scrollData.current = v;
			OnScroll();
		}
		
		void		ScrollBar::OnScroll()
		{
			if (std::shared_ptr<Scrollable> scrollable_ = scrollable.lock())
			{
				if (horizontal)
				{
					scrollable_->horizontal = scrollData;
					scrollable_->OnHorizontalScroll();
				}
				else
				{
					scrollable_->vertical = scrollData;
					scrollable_->OnVerticalScroll();
				}
			}
			onScroll();
		}
		
		
		
		void		ScrollBar::UpdateLayout(const M::Rect<float>&parent_region)
		{
			Component::UpdateLayout(parent_region);
			if (scrollLayout)
			{
				float 	current = M::Clamp01(scrollData.current);
				if (!horizontal)
				{
					upButton.region = cellLayout.client;
					upButton.region.bottom() = upButton.region.top()-scrollLayout->bottomButton.height;
					upButton.color = &scrollLayout->bottomButton.color;
					upButton.normal = &scrollLayout->bottomButton.normal;
					upButton.orientation = 2;
					
					downButton.region = cellLayout.client;
					downButton.region.top() = downButton.region.bottom()+scrollLayout->bottomButton.height;
					downButton.color = &scrollLayout->bottomButton.color;
					downButton.normal = &scrollLayout->bottomButton.normal;
					downButton.orientation = 0;
				
					backgroundTop.region = cellLayout.client;
					backgroundTop.region.bottom() = backgroundTop.region.top()-scrollLayout->backBottom.height;
					backgroundTop.color = &scrollLayout->backBottom.color;
					backgroundTop.normal = &scrollLayout->backBottom.normal;
					backgroundTop.orientation = 2;
					
					
					backgroundBottom.region = cellLayout.client;
					backgroundBottom.region.top() = backgroundBottom.region.bottom()+scrollLayout->backBottom.height;
					backgroundBottom.color = &scrollLayout->backBottom.color;
					backgroundBottom.normal = &scrollLayout->backBottom.normal;
					backgroundBottom.orientation = 0;
					
					backgroundCenter.region = cellLayout.client;
					backgroundCenter.region.top() = backgroundTop.region.bottom();
					backgroundCenter.region.bottom() = backgroundBottom.region.top();
					backgroundCenter.color = &scrollLayout->backCenter.color;
					backgroundCenter.normal = &scrollLayout->backCenter.normal;
					backgroundCenter.orientation = 0;

					
					cursorRange = cellLayout.client.GetHeight()-2*scrollLayout->bottomButton.height+2*scrollLayout->buttonIndent;
					float	display_range = scrollData.max-scrollData.min,
							cursorLength = display_range>scrollData.window?cursorRange*scrollData.window/display_range:cursorRange;
					
					if (cursorLength < 2*scrollLayout->cursorBottom.height)
						cursorLength = 2*scrollLayout->cursorBottom.height;
					cursorRange -= cursorLength;
					cursorTop.region.left() = cellLayout.client.left();
					cursorTop.region.x.max = cellLayout.client.x.max;
					cursorTop.region.top() = cellLayout.client.top()-scrollLayout->bottomButton.height+scrollLayout->buttonIndent-cursorRange*current;
					cursorTop.region.bottom() = cursorTop.region.top()-scrollLayout->cursorBottom.height;
					cursorTop.color = &scrollLayout->cursorBottom.color;
					cursorTop.normal = &scrollLayout->cursorBottom.normal;
					cursorTop.orientation = 2;
					
					cursorCenter.region.left() = cellLayout.client.left();
					cursorCenter.region.x.max = cellLayout.client.x.max;
					cursorCenter.region.top() = cursorTop.region.bottom();
					cursorCenter.region.bottom() = cursorCenter.region.top()-cursorLength+2*scrollLayout->cursorBottom.height;
					cursorCenter.color = &scrollLayout->cursorCenter.color;
					cursorCenter.normal = &scrollLayout->cursorCenter.normal;
					cursorCenter.orientation = 0;
					
					cursorBottom.region.left() = cellLayout.client.left();
					cursorBottom.region.x.max = cellLayout.client.x.max;
					cursorBottom.region.top() = cursorCenter.region.bottom();
					cursorBottom.region.bottom() = cursorBottom.region.top()-scrollLayout->cursorBottom.height;
					cursorBottom.color = &scrollLayout->cursorBottom.color;
					cursorBottom.normal = &scrollLayout->cursorBottom.normal;
					cursorBottom.orientation = 0;
					
					cursorRegion.Set(cellLayout.client.left(),cursorBottom.region.bottom(),cellLayout.client.x.max,cursorTop.region.top());
				}
				else
				{
					current = 1.0f-current;
					downButton.region = cellLayout.client;
					downButton.region.left() = downButton.region.x.max-scrollLayout->bottomButton.height;
					downButton.color = &scrollLayout->bottomButton.color;
					downButton.normal = &scrollLayout->bottomButton.normal;
					downButton.orientation = 3;
					
					upButton.region = cellLayout.client;
					upButton.region.x.max = upButton.region.left()+scrollLayout->bottomButton.height;
					upButton.color = &scrollLayout->bottomButton.color;
					upButton.normal = &scrollLayout->bottomButton.normal;
					upButton.orientation = 1;
				
					backgroundTop.region = cellLayout.client;
					backgroundTop.region.left() = backgroundTop.region.x.max-scrollLayout->backBottom.height;
					backgroundTop.color = &scrollLayout->backBottom.color;
					backgroundTop.normal = &scrollLayout->backBottom.normal;
					backgroundTop.orientation = 3;
					
					
					backgroundBottom.region = cellLayout.client;
					backgroundBottom.region.x.max = backgroundBottom.region.left()+scrollLayout->backBottom.height;
					backgroundBottom.color = &scrollLayout->backBottom.color;
					backgroundBottom.normal = &scrollLayout->backBottom.normal;
					backgroundBottom.orientation = 1;
					
					backgroundCenter.region = cellLayout.client;
					backgroundCenter.region.x.max = backgroundTop.region.left();
					backgroundCenter.region.left() = backgroundBottom.region.x.max;
					backgroundCenter.color = &scrollLayout->backCenter.color;
					backgroundCenter.normal = &scrollLayout->backCenter.normal;
					backgroundCenter.orientation = 1;
					
					
					cursorRange = cellLayout.client.GetWidth()-2*scrollLayout->bottomButton.height+2*scrollLayout->buttonIndent;
					float	display_range = scrollData.max-scrollData.min,
							cursorLength = display_range>scrollData.window?cursorRange*scrollData.window/display_range:cursorRange;
					
					if (cursorLength < 2*scrollLayout->cursorBottom.height)
						cursorLength = 2*scrollLayout->cursorBottom.height;
					cursorRange -= cursorLength;
					cursorTop.region.top() = cellLayout.client.top();
					cursorTop.region.bottom() = cellLayout.client.bottom();
					cursorTop.region.x.max = cellLayout.client.x.max-scrollLayout->bottomButton.height+scrollLayout->buttonIndent-cursorRange*current;
					cursorTop.region.left() = cursorTop.region.x.max-scrollLayout->cursorBottom.height;
					cursorTop.color = &scrollLayout->cursorBottom.color;
					cursorTop.normal = &scrollLayout->cursorBottom.normal;
					cursorTop.orientation = 3;
					
					cursorCenter.region.bottom() = cellLayout.client.bottom();
					cursorCenter.region.top() = cellLayout.client.top();
					cursorCenter.region.x.max = cursorTop.region.left();
					cursorCenter.region.left() = cursorCenter.region.x.max-cursorLength+2*scrollLayout->cursorBottom.height;
					cursorCenter.color = &scrollLayout->cursorCenter.color;
					cursorCenter.normal = &scrollLayout->cursorCenter.normal;
					cursorCenter.orientation = 1;
					
					cursorBottom.region.bottom() = cellLayout.client.bottom();
					cursorBottom.region.top() = cellLayout.client.top();
					cursorBottom.region.x.max = cursorCenter.region.left();
					cursorBottom.region.left() = cursorBottom.region.x.max-scrollLayout->cursorBottom.height;
					cursorBottom.color = &scrollLayout->cursorBottom.color;
					cursorBottom.normal = &scrollLayout->cursorBottom.normal;
					cursorBottom.orientation = 1;
					
					cursorRegion.Set(cursorBottom.region.left(),cellLayout.client.bottom(),cursorTop.region.x.max,cellLayout.client.top());
				}
			
			}
		}
		
		float		ScrollBar::GetClientMinWidth()	const
		{
			if (!scrollLayout)
				return 0;
			if (horizontal)
				return scrollLayout->minHeight;
			return scrollLayout->minWidth;
		}
		
		float		ScrollBar::GetClientMinHeight()	const
		{
			if (!scrollLayout)
				return 0;
			if (horizontal)
				return scrollLayout->minWidth;
			return scrollLayout->minHeight;
		}
		
		

		/*virtual override*/	void		ScrollBar::OnColorPaint(ColorRenderer&renderer, bool parentIsEnabled)
		{
			Component::OnColorPaint(renderer,parentIsEnabled);
			renderer.Paint(backgroundTop);
			renderer.Paint(backgroundCenter);
			renderer.Paint(backgroundBottom);
			
			renderer.MarkNewLayer();
			renderer.PushColor();
			if (cursorGrabbed)
				renderer.ModulateColor(0.7f);
			renderer.Paint(cursorTop);
			renderer.Paint(cursorCenter);
			renderer.Paint(cursorBottom);
			renderer.PopColor();
			renderer.MarkNewLayer();
			
			renderer.Paint(upButton);
			renderer.Paint(downButton);
		}
		
		/*virtual override*/	void		ScrollBar::OnNormalPaint(NormalRenderer&renderer, bool parentIsEnabled)
		{
			Component::OnNormalPaint(renderer,parentIsEnabled);

			renderer.Paint(backgroundTop,false);
			renderer.Paint(backgroundCenter,false);
			renderer.Paint(backgroundBottom,false);
			renderer.MarkNewLayer();
			
			renderer.Paint(cursorTop,cursorGrabbed);
			renderer.Paint(cursorCenter,cursorGrabbed);
			renderer.Paint(cursorBottom,cursorGrabbed);
			
			renderer.MarkNewLayer();
			renderer.Paint(upButton,false);
			renderer.Paint(downButton,false);
		}
		
		/*virtual override*/	Component::eEventResult	ScrollBar::OnMouseDrag(float x, float y)
		{
			if (cursorGrabbed && cursorRange > 0)
			{
				if (horizontal)
				{
					float	relative = x-cursorRegion.left(),
							delta = (relative-cursorHook[0])/cursorRange;
					scrollData.current+=delta;
					scrollData.current = M::Clamp01(scrollData.current);
				}
				else
				{
					float	relative = y-cursorRegion.bottom(),
							delta = (relative-cursorHook[1])/cursorRange;
					scrollData.current-=delta;
					scrollData.current = M::Clamp01(scrollData.current);
				}
				OnScroll();
				return RequestingReshape;
			}
			return Handled;
		}
		
		/*virtual override*/	Component::eEventResult	ScrollBar::OnMouseDown(float x, float y, TExtEventResult&ext)
		{
			if (cursorRegion.Contains(x,y) && cursorRange > 0)
			{
				cursorHook[0] = x-cursorRegion.left();
				cursorHook[1] = y-cursorRegion.bottom();
				cursorGrabbed = true;
			}
			elif (upButton.region.Contains(x,y))
			{
				upPressed = true;
				if ((scrollData.max-scrollData.min) > 0)
				{
					scrollData.current = M::Clamp01(scrollData.current - scrollData.window);
					OnScroll();
				}
			}
			elif (downButton.region.Contains(x,y))
			{
				downPressed = true;
				if ((scrollData.max-scrollData.min) > 0)
				{
					scrollData.current = M::Clamp01(scrollData.current + scrollData.window);
					OnScroll();
				}
			}
				
			return RequestingReshape;
		}
		
		/*virtual override*/	Component::eEventResult		ScrollBar::OnMouseUp(float x, float y)
		{
			bool changed =  cursorGrabbed;
			upPressed = false;
			downPressed = false;
			cursorGrabbed = false;
			return changed?RequestingRepaint:Handled;
		}
		
		/*virtual override*/	Component::eEventResult		ScrollBar::OnMouseWheel(float x, float y, short delta)
		{
			float new_current = horizontal?
									M::Clamp01(scrollData.current + delta/(scrollData.max-scrollData.min)):
									M::Clamp01(scrollData.current - delta/(scrollData.max-scrollData.min));
			if (scrollData.current != new_current)
			{
				scrollData.current = new_current;
				OnScroll();
				return RequestingReshape;
			}
			return Handled;
		}
		
		
			/*
					if (!enabled)
				return NULL;
			bool new_pressed = down && currentRegion.Contains(x,y);
			requests_update |= new_pressed != pressed;
			pressed = new_pressed;
			return down?this:NULL;
		}
		
		Component*	Button::onMouseDown(float x, float y)
		{
			if (!enabled)
				return NULL;
			requests_update |= !down;
			down = true;
			pressed = true;
			return this;
		}
		
		void	Button::onMouseUp(float x, float y)
		{
			requests_update |= pressed;
			pressed = false;
			down = false;
*/

		void					ScrollBox::ScrollTo(float x, float y)
		{
			horizontalBar->ScrollTo(x);
			verticalBar->ScrollTo(y);
		}
		
		void					ScrollBox::scrollToX(float x)
		{
			horizontalBar->ScrollTo(x);
		}

		void					ScrollBox::scrollToY(float y)
		{
			verticalBar->ScrollTo(y);
		}

		
		/*virtual override*/	bool					ScrollBox::Erase(const PComponent&component)
		{
			if (children.findAndErase(component))
			{
				visible_children.findAndErase(component);
				SignalLayoutChange();
				component->SetWindow(PWindow());
				return true;
			}
			return false;
		}
		
		/*virtual override*/	bool					ScrollBox::Erase(index_t index)
		{
			if (index >= children.Count())
				return false;
			visible_children.findAndErase(children[index]);
			children[index]->SetWindow(PWindow());
			children.Erase(index);
			SignalLayoutChange();
			return true;
		}
		
		/*virtual override*/	PConstComponent		ScrollBox::GetChild(index_t index) const
		{
			if (index < children.Count())
				return children[index];
			index -= children.Count();
			if (index == 0)
				return horizontalBar;
			if (index == 1)
				return verticalBar;
			return PConstComponent();
		}
		
		/*virtual override*/	PComponent		ScrollBox::GetChild(index_t index)
		{
			if (index < children.Count())
				return children[index];
			index -= children.Count();
			if (index == 0)
				return horizontalBar;
			if (index == 1)
				return verticalBar;
			return PComponent();
		}

		/*virtual override*/	count_t	ScrollBox::CountChildren() const
		{
			return children.Count()+2;
		}
		

		
		/*virtual override*/	void		ScrollBox::UpdateLayout(const M::Rect<float>&parent_region)
		{
			Component::UpdateLayout(parent_region);
			
			if (horizontalBar->scrollable.expired())
				horizontalBar->scrollable = toScrollable();
			if (verticalBar->scrollable.expired())
				verticalBar->scrollable = toScrollable();

			M::Rect<float>	current;
			if (children.IsEmpty())
			{
				current.Set(0,0,1,1);
				horizontal.current = 0.5;
				vertical.current = 0.5;
			}
			else
			{
				float	w = cellLayout.client.GetWidth(),
						h = cellLayout.client.GetHeight();
				M::Rect<float>	region(0,0,w,h);
				for (index_t i = 0; i < children.Count(); i++)
				{
					//children[i]->UpdateLayout(cellLayout.client);
					M::Rect<float>	child_region;
					const PComponent&child = children[i];
					if (verticalBar->IsVisible())
					{
						child->anchored.top = true;
						child->anchored.bottom = false;
					}
					if (horizontalBar->IsVisible())
					{
						child->anchored.left = true;
						child->anchored.right = false;
					}
					
					child->Locate(region,child_region);
					

					if (!i)
						current = child_region;
					else
						current.Include(child_region);
				}
			}
			
			horizontal.min = 0;
			horizontal.max = current.GetWidth();
			vertical.min = 0;
			vertical.max = current.GetHeight();
			
			
			/*	horizontal.min -= cellLayout.client.left();
				horizontal.max -= cellLayout.client.left();
				vertical.min -= cellLayout.client.top();
				vertical.max -= cellLayout.client.top();*/
			
			horizontal.window = cellLayout.client.GetWidth();
			vertical.window = cellLayout.client.GetHeight();
			
			if (horizontalBar->autoVisibility)
				horizontalBar->SetVisible(horizontal.max>horizontal.window);
			if (verticalBar->autoVisibility)
				verticalBar->SetVisible(vertical.max>vertical.window);
			
			if (verticalBar->IsVisible())
				horizontal.window-=verticalBar->width;
			if (horizontalBar->IsVisible())
				vertical.window-=horizontalBar->height;
			
			if (!horizontalBar->IsVisible() && horizontalBar->autoVisibility)
			{
				horizontalBar->SetVisible(horizontal.max>horizontal.window);
				if (horizontalBar->IsVisible())
					vertical.window-=horizontalBar->height;
			}
			if (!verticalBar->IsVisible() && verticalBar->autoVisibility)
			{
				verticalBar->SetVisible(vertical.max>vertical.window);
				if (verticalBar->IsVisible())
				{
					horizontal.window-=verticalBar->height;
					if (!horizontalBar->IsVisible() && horizontalBar->autoVisibility)
					{
						horizontalBar->SetVisible(horizontal.max>horizontal.window);
						if (horizontalBar->IsVisible())
							vertical.window-=horizontalBar->height;
					}
				}
			}

			horizontalBar->scrollData = horizontal;
			verticalBar->scrollData = vertical;
			horizontalBar->anchored.Set(true,true,true,false);
			horizontalBar->offset.Set(0,0,verticalBar->IsVisible()?-verticalBar->GetMinWidth(false):0,0);
			verticalBar->anchored.Set(false,true,true,true);
			verticalBar->offset.Set(0,horizontalBar->IsVisible()?horizontalBar->GetMinHeight(false):0,0,0);
			horizontalBar->UpdateLayout(cellLayout.client);
			verticalBar->UpdateLayout(cellLayout.client);
			effectiveClientRegion.Set(cellLayout.client.left(),
										horizontalBar->IsVisible()?/*floor*/(horizontalBar->currentRegion.top()):cellLayout.client.bottom(),
										verticalBar->IsVisible()?/*ceil*/(verticalBar->currentRegion.left()):cellLayout.client.x.max,
										cellLayout.client.top());
			//effectiveClientRegion = cellLayout.client;
			
			float	hrange = (horizontal.max-effectiveClientRegion.GetWidth()),
					vrange = (vertical.max-effectiveClientRegion.GetHeight());
			if (hrange < 0)
			{
				hrange = 0;
				horizontal.current = horizontalBar->scrollData.current = 0;
			}
			if (vrange < 0)
			{
				vrange = 0;
				vertical.current = verticalBar->scrollData.current = 0;
			}

			float	offset_x = horizontalBar->IsVisible()?-(horizontal.current * hrange):0,
					offset_y = verticalBar->IsVisible()?(vertical.current * vrange):0;
			
			M::Rect<float>	space = effectiveClientRegion;
			space.Translate(offset_x,offset_y);
			
			visible_children.reset();
			for (index_t i = 0; i < children.Count(); i++)
			{
				const PComponent&child = children[i];
				if (!child->IsVisible())
					continue;
				child->UpdateLayout(space);
				if (effectiveClientRegion.Intersects(child->currentRegion))
					visible_children << child;
			}
		}

		/*virtual override*/	bool		ScrollBox::CanHandleMouseWheel()	const
		{
			return (horizontalBar->IsVisible() && horizontalBar->IsEnabled()) || (verticalBar->IsVisible() && horizontalBar->IsEnabled());
		}

		/*virtual override*/	Component::eEventResult		ScrollBox::OnMouseWheel(float x, float y, short delta)
		{
			if (horizontalBar->IsVisible() && horizontalBar->IsEnabled() && horizontalBar->currentRegion.Contains(x,y))
			{
				if (horizontalBar->scrollable.expired())
					horizontalBar->scrollable = toScrollable();
				return horizontalBar->OnMouseWheel(x,y,delta);
			}
			if (verticalBar->IsVisible() && horizontalBar->IsEnabled() && verticalBar->currentRegion.Contains(x,y))
			{
				if (verticalBar->scrollable.expired())
					verticalBar->scrollable = toScrollable();
				return verticalBar->OnMouseWheel(x,y,delta);
			}
			if (effectiveClientRegion.Contains(x,y))
			{
				
				for (index_t i = visible_children.Count()-1; i < visible_children.Count(); i--)
				{
					const PComponent&child = visible_children[i];
					if (child->currentRegion.Contains(x,y))
					{
						if (!child->IsEnabled())
							break;
						eEventResult rs = child->OnMouseWheel(x,y,delta);
						if (rs == Unsupported)
							break;
						return rs;
					}
				}
			}
			if (verticalBar->IsVisible() && verticalBar->IsEnabled())
			{
				if (verticalBar->scrollable.expired())
					verticalBar->scrollable = toScrollable();
				return verticalBar->OnMouseWheel(x,y,delta);
			}
			if (horizontalBar->IsVisible() && horizontalBar->IsEnabled())
			{
				if (horizontalBar->scrollable.expired())
					horizontalBar->scrollable = toScrollable();
				return horizontalBar->OnMouseWheel(x,y,delta);
			}
			return Unsupported;
		}
		
		/*virtual override*/	float		ScrollBox::GetClientMinWidth()	const
		{
			float w = horizontalBar->IsVisible() || horizontalBar->autoVisibility?horizontalBar->GetMinWidth(false):Panel::GetClientMinWidth();
			if (verticalBar->IsVisible())
				w += verticalBar->GetMinWidth(false);
			return w;
		}
		
		/*virtual override*/	float		ScrollBox::GetClientMinHeight()	const
		{
			float h;
			if (verticalBar->IsVisible() || verticalBar->autoVisibility)
				h = verticalBar->GetMinHeight(false);
			else
			{
				h = 0;
				for (index_t i = 0; i < children.Count(); i++)
					h = std::max(children[i]->GetMinHeight(true),h);
			}
			if (horizontalBar->IsVisible())
				h += horizontalBar->GetMinHeight(false);
			return h;
		}
		
		/*virtual override*/ void		ScrollBox::OnColorPaint(ColorRenderer&renderer, bool parentIsEnabled)
		{
			renderer.PushColor();
			Component::OnColorPaint(renderer,parentIsEnabled);

			renderer.Clip(effectiveClientRegion);
			
			bool subEnabled = parentIsEnabled && IsEnabled();

			for (index_t i = 0; i < visible_children.Count(); i++)
			{
				renderer.PeekColor();
				visible_children[i]->OnColorPaint(renderer,subEnabled);
			}
			renderer.Unclip();
			//renderer.MarkNewLayer();
			
			if (horizontalBar->IsVisible())
			{
				renderer.PeekColor();
				horizontalBar->OnColorPaint(renderer,subEnabled);
			}
			if (verticalBar->IsVisible())
			{
				renderer.PeekColor();
				verticalBar->OnColorPaint(renderer,subEnabled);
			}
			renderer.PopColor();
		}
		
		/*virtual override*/ void		ScrollBox::OnNormalPaint(NormalRenderer&renderer, bool parentIsEnabled)
		{
			renderer.PushNormalMatrix();
			Component::OnNormalPaint(renderer,parentIsEnabled);
			
			renderer.Clip(effectiveClientRegion);
			
			bool subEnabled = parentIsEnabled && IsEnabled();
			
			for (index_t i = 0; i < visible_children.Count(); i++)
			{
				renderer.PeekNormalMatrix();
				visible_children[i]->OnNormalPaint(renderer,subEnabled);
			}
			renderer.Unclip();
			//renderer.MarkNewLayer();

			if (horizontalBar->IsVisible())
			{
				renderer.PeekNormalMatrix();
				horizontalBar->OnNormalPaint(renderer,subEnabled);
			}
			if (verticalBar->IsVisible())
			{
				renderer.PeekNormalMatrix();
				verticalBar->OnNormalPaint(renderer,subEnabled);
			}
			renderer.PopNormalMatrix();

		}

		/*virtual override*/ PComponent			ScrollBox::GetComponent(float x, float y, ePurpose purpose, bool&outIsEnabled)
		{
			outIsEnabled &= IsEnabled();
		
			PComponent result;
			if (horizontalBar->IsVisible() && horizontalBar->IsEnabled() && horizontalBar->currentRegion.Contains(x,y) && (result = horizontalBar->GetComponent(x,y,purpose,outIsEnabled)))
			{
				if (horizontalBar->scrollable.expired())
					horizontalBar->scrollable = toScrollable();
				return result;
			}
			if (verticalBar->IsVisible() && verticalBar->IsEnabled() && verticalBar->currentRegion.Contains(x,y) && (result = verticalBar->GetComponent(x,y,purpose,outIsEnabled)))
			{
				if (verticalBar->scrollable.expired())
					verticalBar->scrollable = toScrollable();
				return result;
			}
			if (effectiveClientRegion.Contains(x,y))
			{
				for (index_t i = visible_children.Count()-1; i < visible_children.Count(); i--)
				{
					const PComponent&child = visible_children[i];
					if (child->cellLayout.border.Contains(x,y))
					{
						bool subEnabled = outIsEnabled;
						result = child->GetComponent(x,y,purpose,subEnabled);
						if (purpose == MouseWheelRequest && CanHandleMouseWheel() && outIsEnabled && (!subEnabled || !result || !result->CanHandleMouseWheel()))
							return shared_from_this();	//override child if it cannot or will not handle mouse wheel
						outIsEnabled = subEnabled;
						return result;
					}
				}
			}
			return shared_from_this();
		}
		
		void			ScrollBox::Append(const PComponent&component)
		{
			component->anchored.Set(true,false,false,true);
			if (children.IsNotEmpty())
				component->offset.top = children.Last()->offset.top-children.Last()->height;
			else
				component->offset.top = 0;
			component->offset.left = 0;
			component->SetWindow(windowLink);
			children << component;
			SignalLayoutChange();
		}
		
		
		void	Edit::_Setup()
		{
			goLeft = 0;
			goRight = 0;
			tickInterval = 0.1;
			readonly = false;
			maskInput = false;
			cursor = 0;
			selectionStart = 0;
			viewBegin = viewEnd = 0;
			layout = globalLayout.Refer();
			width = GetMinWidth(false);
			height = GetMinHeight(false);
		}

		void Edit::SetText(const String&new_text)
		{
			text.Clear();
			{
				auto begin = new_text.c_str();
				const auto end = begin + new_text.GetLength();
				while (begin < end)
				{
					char32_t c32;
					if (!StringEncoding::UTF8::ToUTF32(begin,end,c32))
						break;
					text.Append(c32);
				}
			}
			selectionStart = 0;
			cursor = 0;
			viewBegin = viewEnd = 0;
			_UpdateView();
			SignalVisualChange();
		}
		void Edit::SetText(const StringRef&new_text)
		{
			text.Clear();
			StringEncoding::UTF8::Scanner scanner(new_text);
			while (scanner.HasNext())
			{
				char32_t c32 = scanner.Next();
				if (c32)
					text.Append(c32);
			}
			selectionStart = 0;
			viewBegin = viewEnd = 0;
			_UpdateView();
			SignalVisualChange();
		}

		void						Edit::SetText(const char*new_text)
		{
			SetText(StringRef(new_text));
		}


		/*virtual override*/ void			Edit::OnColorPaint(ColorRenderer&renderer, bool parentIsEnabled)
		{
			const float fontHeight = renderer.GetFont().GetHeight();
			renderer.PushColor();
			if (readonly && enabled && parentIsEnabled)
				renderer.ModulateColor(0.5f);	//if not enabled, will be toned by Component::OnColorPaint
			
			Component::OnColorPaint(renderer,parentIsEnabled);

			renderer.PeekColor();

			size_t end = M::Min(viewEnd,text.GetLength());
			if (end>viewBegin)
			{
				renderer.Clip(cellLayout.client);
				float	bottom = cellLayout.client.y.center()-fontHeight/2+font_offset,
						top = cellLayout.client.y.center()+fontHeight/2;
				if (maskInput)
				{
					float csize = fontHeight*0.75;
					renderer.SetPointSize(csize*0.8);
					for (size_t i = viewBegin; i < end; i++)
						renderer.PaintPoint(cellLayout.client.left()+csize*(0.5+i),cellLayout.client.y.center());
				}
				else
				{
					renderer.SetTextPosition(cellLayout.client.left(),bottom);
					renderer.WriteText(text.pointer()+viewBegin,end-viewBegin);
				}
				if (selectionStart != cursor)
				{
					renderer.ModulateColor(0.2,0.3,0.5,0.7);
					size_t	sel_begin = selectionStart,
							sel_end = cursor;
					if (sel_begin > sel_end)
						swp(sel_begin,sel_end);
					if (sel_begin < viewBegin)
						sel_begin = viewBegin;
					if (sel_end < viewBegin)
						sel_end = viewBegin;
					float	left = cellLayout.client.left()+_GetTextWidth(text.pointer()+viewBegin,sel_begin-viewBegin),
							right = cellLayout.client.left()+_GetTextWidth(text.pointer()+viewBegin,sel_end-viewBegin);
						//glColor4f(0.4,0.6,1,0.7);
					renderer.FillRect(M::Rect<>(left-2,bottom,right+2,top));
					renderer.MarkNewLayer();
					renderer.PeekColor();
					
					if (maskInput)
					{
						float csize = fontHeight*0.75;
						renderer.SetPointSize(csize*0.8);
						for (index_t i = sel_begin; i < sel_end; i++)
							renderer.PaintPoint(cellLayout.client.left()+csize*(0.5+i),cellLayout.client.y.center());
					}
					else
					{
						renderer.SetTextPosition(cellLayout.client.left()+renderer.GetUnscaledWidth(text.pointer()+viewBegin,sel_begin-viewBegin),bottom);
						renderer.WriteText(text.pointer()+sel_begin,sel_end-sel_begin);
					}
				}
					
				renderer.Unclip();
			}
			if (IsFocused() && enabled && parentIsEnabled && cursorTicks < 5)
			{
				renderer.MarkNewLayer();
				renderer.RenderLine(cursorOffset,cellLayout.client.y.center()-fontHeight/2,
									cursorOffset,cellLayout.client.y.center()+fontHeight/2+font_offset);
			}
			renderer.PopColor();
		}
		
		Component::eEventResult		Edit::OnTick()
		{
			count_t new_ticks = (cursorTicks+1)%10;
			bool refresh = new_ticks < 5 != cursorTicks < 5;
			cursorTicks  = new_ticks;

			
			if (goRight && !viewRightMost)
			{
				viewBegin++;
				if (cursor < text.GetLength())
					cursor++;
				_UpdateView();
				return RequestingRepaint;
			}
			if (goLeft && viewBegin)
			{
				viewBegin--;
				if (cursor)
					cursor--;
				_UpdateView();
				return RequestingRepaint;
			}
			return refresh?RequestingRepaint:Handled;
		}

		
		Component::eEventResult		Edit::OnMouseHover(float x, float y, TExtEventResult&ext)
		{
			if (cellLayout.client.Contains(x,y))
				ext.customCursor = Mouse::CursorType::EditText;
			return Handled;
		}
		
		Component::eEventResult		Edit::OnMouseDown(float x, float y, TExtEventResult&ext)
		{
			if (cellLayout.client.Contains(x,y))
			{
				//ShowMessage(String(x)+", "+String(y)+" is in "+cellLayout.client.ToString());
				float rx = x - cellLayout.client.left();
				size_t end = M::Min(viewEnd-1+viewRightMost,text.GetLength());
				index_t index=viewBegin;
				for (; index < end; index++)
				{
					float width = _GetCharWidth(text[index]);
					if (rx < width/2)
						break;
					rx -= width;
				}
				cursor = index;
				if (!input.pressed[Key::Shift])
					selectionStart = cursor;
				ext.customCursor = Mouse::CursorType::EditText;
				
				cursorOffset = cellLayout.client.left()+_GetTextWidth(text.pointer()+viewBegin,cursor-viewBegin);			
				return RequestingRepaint;
			}
			return Handled;
		}
		
		Component::eEventResult	Edit::OnMouseDrag(float x, float y)
		{
			float rx = x - cellLayout.client.left();
			goLeft = rx < 0;
			size_t end = M::Min(viewEnd-1+viewRightMost,text.GetLength());
			index_t index=viewBegin;
			for (; index < end; index++)
			{
				float width = _GetCharWidth(text[index]);
				if (rx < width/2)
					break;
				rx -= width;
			}
			goRight = index == end && rx > 0;
			cursor = index;
			cursorOffset = cellLayout.client.left()+_GetTextWidth(text.pointer()+viewBegin,cursor-viewBegin);			
			return RequestingRepaint;
		}
		
		Component::eEventResult	Edit::OnMouseUp(float x, float y)
		{
			goLeft = false;
			goRight = false;
		
			return Handled;
		}

		
		Component::eEventResult	Edit::OnKeyDown(Key::Name key)
		{
			switch (key)
			{
				case Key::Left:
					if (cursor && text.GetLength())
					{
						if (!input.pressed[Key::Ctrl])
							cursor--;
						else
						{
							if (cursor >= text.GetLength())
								cursor = text.GetLength()-1;
							while (cursor && IsWhitespace(text[cursor-1]))
								cursor--;
							while (cursor && !IsWhitespace(text[cursor-1]))
								cursor--;
							if (IsWhitespace(text[cursor]))
								cursor++;
						}
						if (!input.pressed[Key::Shift])
							selectionStart = cursor;
						_UpdateView();
						return RequestingRepaint;
					}
				break;
				case Key::Right:
					if (cursor<text.GetLength())
					{
						if (!input.pressed[Key::Ctrl])
							cursor++;
						else
						{
							while (cursor<text.GetLength() && !IsWhitespace(text[cursor]))
								cursor++;
							while (cursor<text.GetLength() && IsWhitespace(text[cursor]))
								cursor++;
							//if (cursor && !isWhitespace(text[cursor-1])
						}
						
						if (!input.pressed[Key::Shift])
							selectionStart = cursor;
						_UpdateView();
						return RequestingRepaint;
					}
				break;
				case Key::Home:
					if (cursor)
					{
						cursor = 0;
						if (!input.pressed[Key::Shift])
							selectionStart = cursor;
						_UpdateView();
						return RequestingRepaint;
					}
				break;
				case Key::End:
					if (cursor<text.GetLength() && !readOnly)
					{
						cursor = text.GetLength();
						if (!input.pressed[Key::Shift])
							selectionStart = cursor;
						_UpdateView();
						return RequestingRepaint;
					}
				break;
				case Key::BackSpace:
					if ((cursor || selectionStart)  && !readOnly)
					{
						if (selectionStart == cursor)
						{
							selectionStart--;
							text.Erase(--cursor);
							{
								_UpdateView();
								onChange();
								return RequestingRepaint;
							}
							
						}
						else
							if (selectionStart > cursor)
							{
								text.Erase(cursor,selectionStart-cursor);
								{
									selectionStart = cursor;
									_UpdateView();
									onChange();
									return RequestingRepaint;
								}
							}
							else
								text.Erase(selectionStart,cursor-selectionStart);
								{
									cursor = selectionStart;
									_UpdateView();
									onChange();
									return RequestingRepaint;
								}
					}
				break;
				case Key::Delete:
					if (!readOnly)
					{
						if (selectionStart == cursor)
						{
							text.Erase(cursor);
							{
								_UpdateView();
								onChange();
								return RequestingRepaint;
							}
							
						}
						else
							if (selectionStart > cursor)
							{
								text.Erase(cursor,selectionStart-cursor);
								{
									selectionStart = cursor;
									_UpdateView();
									onChange();
									return RequestingRepaint;
								}
							}
							else
								text.Erase(selectionStart,cursor-selectionStart);
								{
									cursor = selectionStart;
									_UpdateView();
									onChange();
									return RequestingRepaint;
								}
					}
				break;
				case Key::C:
					if (input.pressed[Key::Ctrl] && selectionStart != cursor && !maskedView)
					{
						size_t	begin = selectionStart,
								end = cursor;
						if (begin > end)
							swp(begin,end);
						String sub = String(text.pointer()+begin,end-begin);
						if (!System::copyToClipboard(NULL,sub.c_str()))
							ErrMessage("failed to copy");
					}
				break;
				case Key::X:
					if (input.pressed[Key::Ctrl] && selectionStart != cursor && !maskedView)
					{
						size_t	begin = selectionStart,
								end = cursor;
						if (begin > end)
							swp(begin,end);
						String sub = String(text.pointer()+begin,end-begin);
						if (System::copyToClipboard(NULL,sub.c_str()) && !readOnly)
						{
							text.Erase(begin,end-begin);
							if (cursor > begin)
								cursor = begin;
							selectionStart = cursor;
							_UpdateView();
							onChange();
							return RequestingRepaint;
						}
					}
				break;
				case Key::V:
					if (input.pressed[Key::Ctrl] && !readOnly)
					{
						size_t len = System::GetClipboardTextLength(NULL);
						String clipboard{TStringLength(len)};
						if (System::GetFromClipboardIfText(NULL,clipboard.mutablePointer(),len))
						{

							if (cursor!=selectionStart)
							{
								if (cursor > selectionStart)
								{
									text.Erase(selectionStart,cursor-selectionStart);
									cursor = selectionStart;
								}
								else
									text.Erase(cursor,selectionStart-cursor);
							}

							text.Insert(cursor,StringEncoding::UTF8::ToUTF32(clipboard));
							cursor+=len;
							selectionStart = cursor;
							_UpdateView();
							onChange();
							return RequestingRepaint;
						}
					}
				break;
				case Key::Enter:
				case Key::Return:
					onEnter();
				return Handled;
				default:
					{
						Key::Name k = key;
						if ((k < Key::A || k > Key::Z) && (k < Key::N0 || k > Key::N9) && k != Key::SZ && k != Key::AE && k != Key::UE && k != Key::OE && k != Key::Space && k != Key::Period && k != Key::Comma && k != Key::Minus && k != Key::Mesh)
							return Unsupported;	//really have no use for these. Cascade away.
						//else goto Handled...
					}
			}
			return Handled;
		}
		
		Component::eEventResult	Edit::OnKeyUp(Key::Name key)
		{
		
			return Unsupported;
		}
		
		void	Edit::_UpdateView()
		{
			if (cursor > text.GetLength())
				cursor = text.GetLength();
			if (viewBegin >= text.GetLength())
				viewBegin = text.GetLength()?text.GetLength()-1:0;
			if (cursor < viewBegin)
				viewBegin = cursor;
			else
				while (_GetTextWidth(text.pointer()+viewBegin,cursor-viewBegin)>cellLayout.client.GetWidth())
					viewBegin++;
			viewEnd = viewBegin+1;
			while (viewEnd < text.GetLength() && _GetTextWidth(text.pointer()+viewBegin,viewEnd-viewBegin)<cellLayout.client.GetWidth())
				viewEnd++;
			viewRightMost = _GetTextWidth(text.pointer()+viewBegin,viewEnd-viewBegin)<cellLayout.client.GetWidth();
			//if (textout.unscaledLength(text.root()+viewBegin,viewEnd-viewBegin)>=cellLayout.client.GetWidth())
				//viewEnd--;
			cursorOffset = cellLayout.client.left()+_GetTextWidth(text.pointer()+viewBegin,cursor-viewBegin);
		}
		
		void	Edit::UpdateLayout(const M::Rect<float>&parent_region)
		{
			Component::UpdateLayout(parent_region);
			_UpdateView();
		}
		
		
		
		Component::eEventResult	Edit::OnChar(char32_t c)
		{
			if (readOnly || (acceptChar && !acceptChar(c)))
				return Unsupported;
			if (cursor!=selectionStart)
			{
				if (cursor > selectionStart)
				{
					text.Erase(selectionStart,cursor-selectionStart);
					cursor = selectionStart;
				}
				else
					text.Erase(cursor,selectionStart-cursor);
			}

			text.Insert(cursor++,c);
			selectionStart = cursor;
			_UpdateView();
			onChange();
			return RequestingRepaint;
		}
		
		
		
		
		

		/*virtual override*/ void		Button::OnNormalPaint(NormalRenderer&renderer, bool parentIsEnabled)
		{
			renderer.PushNormalMatrix();
				if (AppearsPressed())
					renderer.ScaleNormals(-1,-1,1);
			
				Component::OnNormalPaint(renderer,parentIsEnabled);

			renderer.PopNormalMatrix();
		}
		
		/*virtual override*/ void		Button::OnColorPaint(ColorRenderer&renderer, bool parentIsEnabled)
		{
			bool pressed = AppearsPressed();
			renderer.PushColor();
				if (pressed && enabled && parentIsEnabled)
					renderer.ModulateColor(0.5f);
			Component::OnColorPaint(renderer,parentIsEnabled);
			renderer.PopColor();

			if (caption.IsNotEmpty())
			{
				const M::Rect<float>&rect=cellLayout.client;
				renderer.SetTextPosition(rect.x.center()-ColorRenderer::textout.GetUnscaledWidth(caption)*0.5+pressed,rect.y.center()-ColorRenderer::textout.GetFont().GetHeight()/2+font_offset);
				renderer.PushColor();
				renderer.ModulateColor(1.0-0.2*(pressed||!enabled));
				renderer.WriteText(caption);
				renderer.PopColor();
			}		
		}
		
		/*virtual override*/ Component::eEventResult	Button::OnMouseDrag(float x, float y)
		{
			bool new_pressed = down && currentRegion.Contains(x,y);
			bool changed = pressed != new_pressed;
			pressed = new_pressed;
			return changed?RequestingRepaint:Handled;
		}
		
		Component::eEventResult	Button::OnMouseDown(float x, float y, TExtEventResult&ext)
		{
			bool changed = !down;
			down = true;
			pressed = true;
			return changed?RequestingRepaint:Handled;
		}
		
		Component::eEventResult	Button::OnMouseUp(float x, float y)
		{
			if (pressed)
				OnExecute();
			bool changed = pressed;
			pressed = false;
			down = false;
			return changed?RequestingRepaint:Handled;
		}
		
		Component::eEventResult	Button::OnKeyDown(Key::Name key)
		{
			switch (key)
			{
				case Key::Space:
				case Key::Enter:
				case Key::Return:
					onExecute();
				return Handled;
			}
			return Unsupported;
		}
		
		Component::eEventResult	Button::OnKeyUp(Key::Name key)
		{
			return Unsupported;
		}
		
		
		
		float		Button::GetClientMinWidth()	const
		{
			return ColorRenderer::textout.GetUnscaledWidth(caption);
		}
		
		float		Button::GetClientMinHeight()	const
		{

			return ColorRenderer::textout.GetFont().GetHeight();
		}
		
		/*virtual override*/void		CheckBox::OnNormalPaint(NormalRenderer&renderer, bool parentIsEnabled)
		{
			Component::OnNormalPaint(renderer,parentIsEnabled);
			if (style && !style->boxNormal.IsEmpty())
			{
				renderer.TextureRect(GetBoxRect(),style->boxNormal);
			}
		}
		
		/*virtual override*/void		CheckBox::OnColorPaint(ColorRenderer&renderer, bool parentIsEnabled)
		{
			renderer.PushColor();
			Component::OnColorPaint(renderer,parentIsEnabled);
			
			const M::Rect<> rect=GetBoxRect();
			if (style && !style->boxColor.IsEmpty())
			{
				renderer.TextureRect(rect,style->boxColor);
				renderer.MarkNewLayer();
			}
			if (checked && style && !style->checkMark.IsEmpty())
			{
				renderer.TextureRect(rect,style->checkMark);
				renderer.MarkNewLayer();
			}
			
			if (pressed && style && !style->highlightMark.IsEmpty())
			{
				renderer.ModulateColor(1,0.6,0);
				renderer.TextureRect(rect,style->highlightMark);
				renderer.PeekColor();
				renderer.MarkNewLayer();
			}
		
			if (caption.IsNotEmpty())
			{
				const float h= ColorRenderer::textout.GetFont().GetHeight();
				const float size = GetBoxSize();
				renderer.SetTextPosition(rect.left()+size+h*0.2f,rect.y.center()-h/2.f+font_offset);//textout.GetFont().GetHeight()*0.5);
				renderer.ModulateColor(1.0-0.2*(!enabled));
				renderer.WriteText(caption);
			}
			renderer.PopColor();
		}


		


		Component::eEventResult	CheckBox::OnMouseDrag(float x, float y)
		{
			bool new_pressed = down && currentRegion.Contains(x,y);
			bool changed = new_pressed != pressed;
			pressed = new_pressed;
			return changed?RequestingRepaint:Handled;
		}
		
		Component::eEventResult	CheckBox::OnMouseDown(float x, float y, TExtEventResult&ext)
		{
			bool changed = !down;
			down = true;
			pressed = true;
			return changed?RequestingRepaint:Handled;
		}
		
		Component::eEventResult	CheckBox::OnMouseUp(float x, float y)
		{
			if (pressed)
			{
				checked = !checked;
				onChange();
			}
			bool changed = pressed;
			pressed = false;
			down = false;
			return changed?RequestingRepaint:Handled;
		}
		
		
		float		CheckBox::GetMinWidth(bool includeOffsets)	const
		{
			float rs;
			{
				rs = ColorRenderer::textout.GetUnscaledWidth(caption)+0.2*ColorRenderer::textout.GetFont().GetHeight()+GetBoxSize();
				if (anchored.right && includeOffsets)
					rs -= offset.right;
				if (anchored.left && includeOffsets)
					rs += offset.left;
			}
			if (layout)
			{
				rs += layout->clientEdge.left+layout->clientEdge.right;
				if (rs < layout->minWidth)
					rs = layout->minWidth;
			}
			return rs;
		}
		
		float		CheckBox::GetMinHeight(bool includeOffsets)	const
		{
			float rs;
			{
				rs = ColorRenderer::textout.GetFont().GetHeight();
				if (anchored.top && includeOffsets)
					rs -= offset.top;
				if (anchored.bottom && includeOffsets)
					rs += offset.bottom;
			}
			if (layout)
			{
				rs += layout->clientEdge.bottom+layout->clientEdge.top;
				if (rs < layout->minHeight)
					rs = layout->minHeight;
			}
			return rs;
		}
		
		
		
		
		
		void			Label::UpdateLayout(const M::Rect<float>&parent_space)
		{
			Component::UpdateLayout(parent_space);
			if (wrapText)
			{
				float w = cellLayout.client.GetWidth();
				if (w != lastWidth || textChanged)
				{
					textChanged = false;
					lastWidth = w;
					Wrap(caption,w,_CharLen,lines);
					//for (index_t i = 0; i < lines.Count(); i++)
					//	cout << "'"<<lines[i]<<"'"<<endl;
					height = GetMinHeight(false);
				}
			}
					
			
		}
		
		
		
		void			Label::_Setup()
		{
			lastWidth = -1;
			caption = "Label";
			wrapText = false;
			textChanged = true;
			fillBackground = false;
			M::Vec::set(backgroundColor,1);
			M::Vec::set(textColor,1);
			textMargin.SetAll(0.f);
			height = GetMinHeight(false);
			width = GetMinWidth(false);
		}
		
		float			Label::GetClientMinWidth()	const
		{
			if (!wrapText)
				return ColorRenderer::textout.GetUnscaledWidth(caption) + textMargin.left + textMargin.right;
			return 30;
		}
		
		float			Label::GetClientMinHeight()	const
		{
			if (!wrapText)
				return ColorRenderer::textout.GetFont().GetHeight() + textMargin.bottom + textMargin.top;
			return ColorRenderer::textout.GetFont().GetHeight()*lines.Count() + textMargin.bottom + textMargin.top;
		
		}
		
		Label*			Label::SetText(const String&text)
		{
			caption = text;
			lines.Clear();
			textChanged = true;
			if (!wrapText)
				width = GetMinWidth(false);
			SignalLayoutChange();
			return this;
		}
		Label*			Label::SetColor(const M::TVec4<>&color)
		{
			textColor = color;
			SignalVisualChange();
			return this;
		}

		float		Label::_CharLen(char c)
		{
			return ColorRenderer::textout.GetUnscaledWidth(StringRef(&c,1));
		}
		
		void			Label::OnColorPaint(ColorRenderer&renderer, bool parentIsEnabled)
		{
			renderer.PushColor();
			
			Component::OnColorPaint(renderer, parentIsEnabled);

			if (!fillBackground && caption.IsEmpty())
			{
				renderer.PopColor();
				return;
			}
			
			if (fillBackground)
			{
				renderer.ModulateColor(backgroundColor);
				renderer.FillRect(cellLayout.client);
				renderer.PeekColor();
				renderer.MarkNewLayer();
			}
			if (caption.IsNotEmpty())
			{
				renderer.Clip(cellLayout.client);
				renderer.ModulateColor(textColor);
				if (!wrapText)
				{
					float	bottom = cellLayout.client.y.center()-ColorRenderer::textout.GetFont().GetHeight()/2+font_offset;
					renderer.SetTextPosition(cellLayout.client.left() + textMargin.left,bottom + textMargin.bottom);
					renderer.WriteText(caption);
				}
				else
				{
					for (index_t i = 0; i < lines.Count(); i++)
					{
						renderer.SetTextPosition(cellLayout.client.left() + textMargin.left,cellLayout.client.top()-(ColorRenderer::textout.GetFont().GetHeight()*(i+1))-textMargin.top);
						renderer.WriteText(lines[i]);
					}
				}
				renderer.Unclip();
			}
			renderer.PopColor();
		}

		void	Label::SetTextMargin(const M::Quad<float>&margin)
		{
			ASSERT_GREATER_OR_EQUAL__(margin.right,0.f);
			ASSERT_GREATER_OR_EQUAL__(margin.top,0.f);
			textMargin = margin;
			if (!wrapText)
				width = GetMinWidth(false);
			SignalLayoutChange();
		}
		
		void	ComboBox::_Setup()
		{
			//MenuEntry::GetMenu();	//don't create menu (called from constructor)
			selectedObject.reset();
			selectedEntry = InvalidIndex;
			openDown = true;
			layout = globalLayout.Refer();
			width = GetMinWidth(false);
			height = GetMinHeight(false);
			//ShowMessage(width);
		}
		
		void	ComboBox::UpdateLayout(const M::Rect<float>&parent_space)
		{
			if (!selectedObject && GetMenu()->CountChildren()>2)
			{
				selectedEntry = 0;
				selectedObject = ((MenuEntry*)(GetMenu()->GetChild(selectedEntry).get()))->GetObject();
				if (selectedObject)
					SetText(selectedObject->ConvertToString());
				else
					SetText("<error>");
			}
			height = Label::GetMinHeight(false);
			MenuEntry::UpdateLayout(parent_space);
		}
		
		
		void	MenuEntry::CorrectMenuWindowSize()	const
		{
			bool changed = false;
			//float mw = menuWindow->GetMinWidth();
			Menu*	menu = (Menu*)menuWindow->rootComponent.get();
			M::TVec2<> size = menu->GetIdealSize();
			if (menuWindow->GetLayout())	//this is NOT included in the result of GetIdealSize(), since it cannot know the window layout (only component layouts)
			{
				size.y += menuWindow->GetLayout()->clientEdge.top+menuWindow->GetLayout()->clientEdge.bottom;
				size.x += menuWindow->GetLayout()->clientEdge.left+menuWindow->GetLayout()->clientEdge.right;
			}
			if (menuWindow->fsize.x != size.x)
			{
				menuWindow->SetWidth(size.x);
				changed = true;
			}
			float h = M::Min(150,size.y);
			if (menuWindow->fsize.y != h)
			{
				menuWindow->SetHeight(h);
				changed = true;
			}
					
			if (menuWindow->layoutChanged || changed)
			{
				menuWindow->UpdateLayout();
			}
		}

		
		float	ComboBox::GetMinWidth(bool includeOffsets)	const
		{
			PWindow menuWindow = GetMenuWindow();
			if (menuWindow)
			{
				if (menuWindow->layoutChanged)
					menuWindow->UpdateLayout();
				CorrectMenuWindowSize();
				return M::Max(MenuEntry::GetMinWidth(includeOffsets),menuWindow->GetMinWidth());
			}
			return MenuEntry::GetMinWidth(includeOffsets);
		}

		
		void	ComboBox::OnMenuClose(const PMenuEntry&child)
		{
			selectedEntry = GetMenu()->GetIndexOfChild(child);
			if (selectedEntry)
			{
				selectedEntry--;
				selectedObject = child->GetObject();
			}
			else
			{
				selectedEntry = InvalidIndex;
				selectedObject.reset();
			}
			if (selectedObject)
				SetText(selectedObject->ConvertToString());
			else
				SetText("");
			SignalLayoutChange();
			onChange();
		}
		
		void	ComboBox::Select(index_t index)
		{
			selectedEntry = index;
			if (selectedEntry >= GetMenu()->CountChildren())
			{
				DBG_ASSERT_EQUAL__(selectedEntry,InvalidIndex);
				selectedEntry = InvalidIndex;
				selectedObject.reset();
				SetText("");
			}
			else
			{
				selectedObject = ((MenuEntry*)GetMenu()->GetChild(selectedEntry).get())->GetObject();
				if (selectedObject)
					SetText(selectedObject->ConvertToString());
				else
					SetText("<error>");
			}
			SignalLayoutChange();
		}

		
		void	MenuEntry::UpdateLayout(const M::Rect<float>&parent_space)
		{
			Label::UpdateLayout(parent_space);
			
		}
		
		Component::eEventResult			MenuEntry::OnMouseDown(float x, float y, TExtEventResult&ext)
		{
			onExecute();
			
			if (menuWindow)
			{
				POperator op = RequireOperator();
				auto delta = (timing.now64-menuWindow->hidden);
				if (delta.IsInRange(0_s,0.1_s))
				{
					op->HideMenus();
					//Window::hideMenus();
				}
				else
				{
					ASSERT_NOT_NULL__(menuWindow->rootComponent);
					
					M::Rect<float>	absolute = cellLayout.border;
					PWindow parent = GetWindow();
					ASSERT__(parent);
					
					#ifdef DEEP_GUI
						absolute.Translate(parent->current_center.x-parent->fsize.x/2,parent->current_center.y-parent->fsize.y/2);
					#else
						absolute.Translate(parent->x-parent->fsize.x/2,parent->y-parent->fsize.y/2);
					#endif
						

					CorrectMenuWindowSize();
					
					#ifdef DEEP_GUI
						if (openDown)
						{
							menuWindow->current_center.x = absolute.left()+menuWindow->fsize.x/2;
							menuWindow->current_center.y = absolute.bottom()-menuWindow->fsize.y/2;
						}
						else
						{
							menuWindow->current_center.x = absolute.x.max+menuWindow->fsize.x/2;
							menuWindow->current_center.y = absolute.top()-menuWindow->fsize.y/2;
						}
						menuWindow->current_center.x -= (menuWindow->cellLayout.border.left());
						menuWindow->current_center.y += (menuWindow->fsize.y-menuWindow->cellLayout.border.top());
						
						menuWindow->current_center.shellRadius = 1;
						
						menuWindow->origin = menuWindow->destination = menuWindow->current_center;
					#else
						if (openDown)
						{
							menuWindow->x = absolute.left()+menuWindow->fsize.x/2;
							menuWindow->y = absolute.bottom()-menuWindow->fsize.y/2;
						}
						else
						{
							menuWindow->x = absolute.x.max+menuWindow->fsize.x/2;
							menuWindow->y = absolute.top()-menuWindow->fsize.y/2;
						}
						menuWindow->x -= (menuWindow->GetCellLayout().border.left());
						menuWindow->y += (menuWindow->fsize.y-menuWindow->GetCellLayout().border.top());
					#endif
						
					op->ShowMenu(menuWindow);
					menuIsOpen = true;
				}
			}
			else
			{
				RequireOperator()->HideMenus();
				OnMenuClose(std::static_pointer_cast<MenuEntry,Component>(shared_from_this()));
			}
			
			return Handled;
		}
		
		
		Component::eEventResult			MenuEntry::OnMouseExit()
		{
			//bool changed = Label::fillBackground;
			//Label::fillBackground = false;
			if (IsFocused())
			{
				PMenu menu = parent.lock();
				if (menu)
					menu->_SelectMenuEntry(PComponent());
				ResetFocused();
				return RequestingRepaint;
			}
			return Handled;
		}
		

		Component::eEventResult			MenuEntry::OnFocusGained()
		{
			Label::fillBackground = true;
			return RequestingRepaint;
		}
		
		Component::eEventResult			MenuEntry::OnFocusLost()
		{
			Label::fillBackground = menuIsOpen;
			return RequestingRepaint;
		}
		
		Component::eEventResult			MenuEntry::OnKeyDown(Key::Name key)
		{
			return parent.expired()?Unsupported:parent.lock()->OnKeyDown(key);
		}
		
		
		const PWindow&	MenuEntry::RetrieveMenuWindow()
		{
			if (!menuWindow)
			{
				menuWindow.reset(new Window(false, &Window::menuStyle));
				PMenu menu = PMenu(new Menu());
				menuWindow->SetComponent(menu);
				menuWindow->onHide += bind(&MenuEntry::OnMenuHide,this);
				menu->parent = std::static_pointer_cast<MenuEntry,Component>(shared_from_this());
				//menu->level = level+1;

			}
			return menuWindow;
		}
		
		PMenu		MenuEntry::GetMenu()
		{
			return std::static_pointer_cast<Menu, Component>(RetrieveMenuWindow()->rootComponent);
		}

		PConstMenu	MenuEntry::GetMenu()	const
		{
			return menuWindow?std::static_pointer_cast<const Menu, const Component>(menuWindow->rootComponent):PConstMenu();
		}
		
		void		MenuEntry::DiscardMenu()
		{
			if (menuWindow)
			{
				POperator op = GetOperator();
				if (op)	//might, in fact, be null
					op->HideMenus();
				menuWindow.reset();
			}
		}
		
		void		MenuEntry::_Setup()
		{
			parent.reset();
			menuWindow.reset();
			object.reset();
			openDown = false;
			menuIsOpen = false;
			Label::wrapText = false;
			//level = 0;
			height = GetMinHeight(false);
			width = GetMinWidth(false);
		}
		
		void	MenuEntry::SetObject(const std::shared_ptr<IToString>&object_)
		{
			if (object == object_)
				return;
			object = object_;
			if (object && object.get() != this)
				Label::SetText(object->ConvertToString());
			//SignalVisualChange();
		}
		

		void	MenuEntry::OnColorPaint(ColorRenderer&renderer, bool parentIsEnabled)
		{
			//Label::fillBackground = IsFocused();
			renderer.PushColor();
			if (!IsEnabled())
				renderer.ModulateColor(1.f,0.5f);
			Label::OnColorPaint(renderer,parentIsEnabled);
			renderer.PopColor();
		}

		void	MenuEntry::OnMenuHide()
		{
			menuIsOpen = false;
			Label::fillBackground = IsFocused();
			SignalVisualChange();
		}
		
		void	MenuEntry::OnMenuClose(const PMenuEntry&child)
		{
			menuIsOpen = false;
			Label::fillBackground = IsFocused();
			if (!parent.expired())
			{
				PMenu	p = parent.lock();
				if (!p->parent.expired())
					p->parent.lock()->OnMenuClose(std::static_pointer_cast<MenuEntry, Component>(shared_from_this()));
			}
			SignalVisualChange();
		}
		
		MenuEntry::~MenuEntry()
		{
			//DiscardMenu();	//only hides menu in the operator, but i suspect even if the case happens where a menu is still visible when the root is erased, the operator should now be able to handle that
		}
		
		/*virtual override*/	void				Menu::OnColorPaint(ColorRenderer&renderer, bool parentIsEnabled)
		{
			if (leftBackgroundColor.a != 0 || rightBackgroundColor.a != 0)
			{
				//renderer.PushColor();
					//renderer.ModulateColor(backgroundColor);
				M::TVec2<>	p0 = {cellLayout.client.x.min,cellLayout.client.y.min},
						p1 = {cellLayout.client.x.max,cellLayout.client.y.min},
						p2 = {cellLayout.client.x.max,cellLayout.client.y.max},
						p3 = {cellLayout.client.x.min,cellLayout.client.y.max};
				
				renderer.FillQuad(p0,leftBackgroundColor,p1,rightBackgroundColor,p2,rightBackgroundColor,p3,leftBackgroundColor);
				//renderer.PopColor();
			}
			Super::OnColorPaint(renderer,parentIsEnabled);
		}

		
		M::TVec2<>		Menu::GetIdealSize()	const
		{
			M::TVec2<> res = {0,0};
			if (horizontal)
			{
				for (index_t i = 0; i < children.Count(); i++)
					res.x += children[i]->width;
				if (layout)
					res.x += layout->clientEdge.left+layout->clientEdge.right;
				res.y = GetMinHeight(true);
			}
			else
			{
				for (index_t i = 0; i < children.Count(); i++)
					res.y += children[i]->height;
				if (layout)
					res.y += layout->clientEdge.top+layout->clientEdge.bottom;
				res.x = GetMinWidth(true);
			}
			return res;
		}

		void					Menu::SetHorizontal(bool b)
		{
			if (horizontal == b)
				return;
			horizontal = b; 
			ScrollBox::horizontalBar->SetVisible(false);
			ScrollBox::verticalBar->SetVisible(false);
			ScrollBox::verticalBar->autoVisibility = !b;
			ScrollBox::horizontalBar->autoVisibility = b;
			foreach(children,c)
			{
				PMenuEntry entry = std::dynamic_pointer_cast<MenuEntry,Component>(*c);
				if (entry)
					entry->openDown = horizontal;
			}
			_ArrangeItems();
		}
						
		
		PMenuEntry		Menu::Add(const String&caption)
		{
			PMenuEntry entry = PMenuEntry(new MenuEntry());
			entry->SetText(caption);
			entry->object = entry;
			entry->parent = std::static_pointer_cast<Menu,Component>(shared_from_this());
			ASSERT__(Add(entry));
			return entry;
		}
		
		bool		Menu::Add(const PComponent&component)
		{
			PMenuEntry entry = std::dynamic_pointer_cast<MenuEntry,Component>(component);
			if (!entry || !ScrollBox::Add(component))
				return false;
			entry->parent = std::static_pointer_cast<Menu,Component>(shared_from_this());
			entry->SetTextColor(entryTextColor);
			entry->backgroundColor = entryBackgroundColor;
			entry->SetTextMargin(M::Quad<float>(5,0,5,0));
			entry->openDown = horizontal;
			//((MenuEntry*)component)->level = level;
			component->SetWindow(windowLink);		
			_ArrangeItems();
			PMenuEntry parent = this->parent.lock();
			//if (parent)
			//	parent->CorrectMenuWindowSize();
			return true;
		}

		void		Menu::SetEntryTextColor(const M::TVec4<>&color)
		{
			entryTextColor = color;
			foreach(children,c)
			{
				PMenuEntry entry = std::dynamic_pointer_cast<MenuEntry,Component>(*c);
				if (entry)
					entry->SetTextColor(color);
			}
		}

		void		Menu::SetEntryBackgroundColor(const M::TVec3<>&color)
		{
			entryBackgroundColor = color;
			foreach(children,c)
			{
				PMenuEntry entry = std::dynamic_pointer_cast<MenuEntry,Component>(*c);
				if (entry)
					entry->backgroundColor = entryBackgroundColor;
			}
		}
		
		bool		Menu::Erase(const PComponent&component)
		{
			if (ScrollBox::Erase(component))
			{
				_ArrangeItems();
				return true;
			}
			return false;
		}
		
		bool		Menu::Erase(index_t index)
		{
			if (ScrollBox::Erase(index))
			{
				_ArrangeItems();
				return true;
			}
			return false;
		}
		
		bool		Menu::MoveChildUp(const PComponent&component)
		{
			if (ScrollBox::MoveChildUp(component))
			{
				_ArrangeItems();
				return true;
			}
			return false;
		}

		bool		Menu::MoveChildUp(index_t index)
		{
			if (ScrollBox::MoveChildUp(index))
			{
				_ArrangeItems();
				return true;
			}
			return false;
		}

		bool		Menu::MoveChildDown(const PComponent&component)
		{
			if (ScrollBox::MoveChildDown(component))
			{
				_ArrangeItems();
				return true;
			}
			return false;
		}
		
		bool		Menu::MoveChildDown(index_t index)
		{
			if (ScrollBox::MoveChildDown(index))
			{
				_ArrangeItems();
				return true;
			}
			return false;
		}
		
		bool		Menu::MoveChildToTop(const PComponent&component)
		{
			if (ScrollBox::MoveChildToTop(component))
			{
				_ArrangeItems();
				return true;
			}
			return false;
		}

		bool		Menu::MoveChildToTop(index_t index)
		{
			if (ScrollBox::MoveChildToTop(index))
			{
				_ArrangeItems();
				return true;
			}
			return false;
		}

		bool		Menu::MoveChildToBottom(const PComponent&component)
		{
			if (ScrollBox::MoveChildToBottom(component))
			{
				_ArrangeItems();
				return true;
			}
			return false;
		}

		bool		Menu::MoveChildToBottom(index_t index)
		{
			if (ScrollBox::MoveChildToBottom(index))
			{
				_ArrangeItems();
				return true;
			}
			return false;
		}
		
		void		Menu::_ArrangeItems()
		{
			float current=0;
			for (index_t i = 0; i < children.Count(); i++)
			{
				const PComponent&component = children[i];
				
				if (horizontal)
				{
					component->anchored.Set(true,true,false,true);

					component->offset.left = current;
					current += component->width;
					component->offset.top = 0;
					component->offset.bottom = 0;
				}
				else
				{
					component->anchored.Set(true,false,true,true);
					component->offset.top = current;
					current -= component->height;
					component->offset.left = 0;
					component->offset.right = 0;
				}
			}
			if (autoResize)
			{
				M::TVec2<> size = GetIdealSize();
				width = size.x;
				height = size.y;
			}
			SignalLayoutChange();
		}

		void	Menu::_Setup()
		{
			ScrollBox::horizontalBar->SetVisible(false);
			ScrollBox::verticalBar->autoVisibility = true;
			parent.reset();
			//level = 0;
			horizontal = false;
			autoResize = true;
			M::Vec::clear(leftBackgroundColor);
			M::Vec::clear(rightBackgroundColor);
			M::Vec::set(entryTextColor,1);
			M::Vec::def(entryBackgroundColor,0.5,0.5,1);

			selectedEntry = InvalidIndex;
			layout = globalLayout.Refer();
		}
		
		
		Component::eEventResult			Menu::OnKeyDown(Key::Name key)
		{
			switch (key)
			{
				case Key::Up:
					if (selectedEntry > 0 && selectedEntry != InvalidIndex)
					{
						selectedEntry--;
						SetFocused(children[selectedEntry]);
						return RequestingRepaint;
					}
					return Handled;
				case Key::Down:
					if (selectedEntry+1<children.Count())
					{
						selectedEntry++;
						SetFocused(children[selectedEntry]);
						return RequestingRepaint;
					}
				
			}
			return Unsupported;
		}
		
		
		Component::eEventResult		MenuEntry::OnMouseHover(float x, float y, TExtEventResult&)
		{
			bool changed = !IsFocused();
			//Label::fillBackground = true;
			if (changed)
			{
				PMenu menu = parent.lock();
				if (menu)
					menu->_SelectMenuEntry(shared_from_this());
				return RequestingRepaint;
			}
			return Handled;
		}

		void			Menu::_SelectMenuEntry(const PComponent&c)
		{
			if (selectedComponent == c)
				return;
			index_t index = Panel::children.GetIndexOf(c);
			selectedEntry = index;
			if (index != InvalidIndex)
			{
				selectedComponent = c;
				SetFocused(children[selectedEntry]);
			}
			else
			{
				if (selectedComponent->IsFocused())
					ResetFocused();
				selectedComponent.reset();
			}
			SignalVisualChange();
		}


		static void	LoadLayout(XML::Node*xtheme, FileSystem::Folder&folder, const String&path, Layout&layout, float outer_scale)
		{
			XML::Node*node = xtheme->Find(path);
			if (!node)
				return;	//graceful ignore
			String string;
			float scale = 1.0f;
			if (node->Query("scale",string))
				convert(string.c_str(),scale);
			layout.override = NULL;
			FileSystem::File	file;
			if (node->Query("file",string))
				if (folder.FindFile(PathString(string),file))
				{
					layout.LoadFromFile(file.GetLocation(),scale*outer_scale);
					return;
				}
			if (node->Query("copy",string))
			{
				if (string == "button")
					layout.override = &Button::globalLayout;
				elif (string == "panel")
					layout.override = &Panel::globalLayout;
				elif (string == "edit")
					layout.override = &Edit::globalLayout;
				elif (string == "scrollbox")
					layout.override = &ScrollBox::globalLayout;
				elif (string == "window/common")
					layout.override = &Window::commonStyle;
				elif (string == "window/menu")
					layout.override = &Window::menuStyle;
				elif (string == "window/hint")
					layout.override = &Window::hintStyle;
				else
					throw Except::Program::UnsupportedRequest("Unknown resource layout '"+string+"'");
			}
		}
		
		static void LoadLayout(XML::Node*xtheme, FileSystem::Folder&folder, const String&path, ScrollBarLayout&layout, float outer_scale)
		{
			XML::Node*node = xtheme->Find(path);
			if (!node)
				return;	//graceful ignore
			String string;
			float scale = 1.0f;
			if (node->Query("scale",string))
				convert(string.c_str(),scale);
			
			FileSystem::File	file;
			if (node->Query("file",string))
				if (folder.FindFile(PathString(string),file))
					layout.LoadFromFile(file.GetLocation(),scale*outer_scale);
		}
		
		static void LoadLayout(XML::Node*xtheme, FileSystem::Folder&folder, const String&path, SliderLayout&layout, float outer_scale)
		{
			XML::Node*node = xtheme->Find(path);
			if (!node)
				return;	//graceful ignore
			String string;
			float scale = 1.0f;
			if (node->Query("scale",string))
				convert(string.c_str(),scale);
			
			FileSystem::File	file;
			if (node->Query("file",string))
				if (folder.FindFile(PathString(string),file))
					layout.LoadFromFile(file.GetLocation(),scale*outer_scale);
		}
		
		static void LoadLayout(XML::Node*xtheme, FileSystem::Folder&folder, const String&path, CheckBox::TStyle&layout, float outer_scale)
		{
			XML::Node*node = xtheme->Find(path);
			if (!node)
				return;	//graceful ignore
			String string;

			FileSystem::File	file;
			
			if (node->Query("color",string) && folder.FindFile(PathString(string),file))
				LoadColor(file.GetLocation(),layout.boxColor);
			if (node->Query("bump",string) && folder.FindFile(PathString(string),file))
				LoadBump(file.GetLocation(),layout.boxNormal);
			if (node->Query("check",string) && folder.FindFile(PathString(string),file))
				LoadColor(file.GetLocation(),layout.checkMark);
			if (node->Query("highlight",string) && folder.FindFile(PathString(string),file))
				LoadColor(file.GetLocation(),layout.highlightMark);
		}
		
		
		void		LoadTheme(const PathString&filename,float outer_scale)
		{
			XML::Container	xml;
			xml.LoadFromFile(filename);
			FileSystem::Folder	folder(FileSystem::ExtractFileDir(filename));
			String	string;
			XML::Node*xtheme = xml.Find("theme");
			if (!xtheme)
				throw Except::IO::DriveAccess::FileFormatFault(Except::globalString("XML theme file lacks theme XML root node"));
				
			FileSystem::File	file;
			if (XML::Node*node = xtheme->Find("font"))
			{
				float scale = 1.0f;
				if (node->Query("scale",string))
					convert(string.c_str(),scale);
				if (!node->Query("file",string))
					throw Except::IO::DriveAccess::FileFormatFault("XML Font node lacks file attribute");
				if (folder.FindFile(PathString(string),file))
					ColorRenderer::textout.GetFont().LoadFromFile(file.GetLocation(),scale*outer_scale);
						//FAIL("Failed to load font from font file '"+file.GetLocation()+"'");
			}
			else
				throw Except::IO::DriveAccess::FileFormatFault(Except::globalString("XML theme file lacks theme/font node"));
			
			LoadLayout(xtheme,folder,"button",Button::globalLayout,outer_scale);
			LoadLayout(xtheme,folder,"panel",Panel::globalLayout,outer_scale);
			LoadLayout(xtheme,folder,"combobox",ComboBox::globalLayout,outer_scale);
			LoadLayout(xtheme,folder,"edit",Edit::globalLayout,outer_scale);
			LoadLayout(xtheme,folder,"scrollbox",ScrollBox::globalLayout,outer_scale);
			LoadLayout(xtheme,folder,"scrollbar/style",ScrollBarLayout::global,outer_scale);
			LoadLayout(xtheme,folder,"window/common",Window::commonStyle,outer_scale);
			LoadLayout(xtheme,folder,"window/menu",Window::menuStyle,outer_scale);
			LoadLayout(xtheme,folder,"window/hint",Window::hintStyle,outer_scale);
			LoadLayout(xtheme,folder,"checkbox/style",CheckBox::globalStyle,outer_scale);
			LoadLayout(xtheme,folder,"slider/style",SliderLayout::global,outer_scale);
			
			/*
			if (Button::globalLayout.copy_from)
				Button::globalLayout = *Button::globalLayout.copy_from;
			if (Panel::globalLayout.copy_from)
				Panel::globalLayout = *Panel::globalLayout.copy_from;
			if (ComboBox::globalLayout.copy_from)
				ComboBox::globalLayout = *ComboBox::globalLayout.copy_from;
			if (Edit::globalLayout.copy_from)
				Edit::globalLayout = *Edit::globalLayout.copy_from;
			if (ScrollBox::globalLayout.copy_from)
				ScrollBox::globalLayout = *ScrollBox::globalLayout.copy_from;
			if (Window::commonStyle.copy_from)
				Window::commonStyle = *Window::commonStyle.copy_from;
			if (Window::menuStyle.copy_from)
				Window::menuStyle = *Window::menuStyle.copy_from;
			if (Window::hintStyle.copy_from)
				Window::hintStyle = *Window::hintStyle.copy_from;
			*/
			

		}
		
		
		void			ShowChoice(Operator&op, const String&title, const String&Query, const Array<String>&choices, const std::function<void(index_t)>&onSelect)
		{
			if (choices.IsEmpty())
				return;
			PLabel	message_label = PLabel(new Label());
			PPanel	panel = PPanel(new Panel());
			Array<PButton >	buttons(choices.Count());
			for (index_t i = 0; i < buttons.Count(); i++)
			{
				buttons[i] = PButton(new Button());
				buttons[i]->anchored.Set(true,true,false,false);
				buttons[i]->SetCaption(choices[i]);
				buttons[i]->width = std::max(buttons[i]->width,100.f);
			}
				//message_button->on_execute += HideMessage;
			
			message_label->wrapText = true;
			message_label->SetText(Query);
			message_label->anchored.Set(true,true,true,true);
			panel->Add(message_label);
			panel->Add(buttons.First());
			buttons.First()->anchored.Set(true,true,false,false);

			for (index_t i = 1; i < buttons.Count(); i++)
				panel->AppendRight(buttons[i]);

			message_label->offset.bottom = buttons.First()->height;

			PWindow	window = Window::CreateNew(NewWindowConfig(title,WindowPosition(0,0,400,200,SizeChange::Fixed),true),panel);
			window->size.height = (size_t)(window->fsize.y = window->GetMinHeight());
			window->layoutChanged = true;
			window->visualChanged = true;
			window->UpdateLayout();
			window->size.height = (size_t)(window->fsize.y = window->GetMinHeight());
			message_label->SetText(Query);
			window->UpdateLayout();
			window->size.height = (size_t)(window->fsize.y = window->GetMinHeight());

			op.ShowWindow(window);
			Component::SetFocused(buttons[0]);
			
			std::weak_ptr<GUI::Window>	weak_window = window;
			for (index_t i = 0; i < buttons.Count(); i++)
				buttons[i]->onExecute += [weak_window,i,onSelect]()
				{
					PWindow	window = weak_window.lock();
					if (window)
						window->Hide();
					onSelect(i);
				};
		}


		static PWindow	messageWindow;
		static PLabel	messageLabel;
		static PButton	messageButton;
		static void createMessageWindow(Operator&op)
		{
			if (messageWindow)
				return;
			
			messageLabel = PLabel(new Label());
			PPanel	panel = PPanel(new Panel());
			messageButton = PButton(new Button());
			
			messageButton->anchored.Set(false,true,false,false);
			messageButton->SetCaption("OK");
			messageButton->width = 100;
			messageButton->onExecute += HideMessage;
			
			messageLabel->SetText("message");
			messageLabel->wrapText = true;
			messageLabel->anchored.Set(true,true,true,true);
			panel->Add(messageLabel);
			panel->Add(messageButton);
			messageLabel->offset.bottom = messageButton->height;

			//float	mx = op.getDisplay().GetClientWidth()/2,
			//		my = op.getDisplay().GetClientHeight()/2;
			messageWindow = Window::CreateNew(NewWindowConfig("Message",WindowPosition(0,0,400,200,SizeChange::Fixed),true),panel);
		}
		
		
		void	ShowMessage(Operator&op, const String&title, const String&message)
		{
			createMessageWindow(op);
			messageWindow->title = title;
			
			//message_window->UpdateLayout();
			messageLabel->SetText(message);
			
			messageWindow->UpdateLayout();
			messageWindow->size.height = (size_t)(messageWindow->fsize.y = messageWindow->GetMinHeight());
			messageWindow->layoutChanged = true;
			messageWindow->visualChanged = true;
			
			#ifdef DEEP_GUI
				message_window->current_center.x = message_window->origin.x = message_window->destination.x = 0;
				message_window->current_center.y = message_window->origin.y = message_window->destination.y = 0;
				message_window->current_center.shellRadius = 1;
				message_window->origin.shellRadius = 1;
				message_window->destination.shellRadius = 1;
			#else
				messageWindow->x = 0;
				messageWindow->y = 0;
			#endif
			//ShowMessage("inserting window");
			op.ShowWindow(messageWindow);
			Component::SetFocused(messageButton);
		}
		
		void	ShowMessage(Operator&op, const String&message)
		{
			ShowMessage(op,"Message",message);
		}
		
		bool	ShowingMessage()
		{
			if (!messageWindow)
				return false;
			POperator op = messageWindow->operatorLink.lock();
			return op && op->WindowIsVisible(messageWindow);
		}
		
		void	HideMessage()
		{
			if (messageWindow)
				messageWindow->Hide();
		}
		
	}
}

