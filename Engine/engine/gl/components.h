#ifndef gl_gui_componentsH
#define gl_gui_componentsH

#include <string/encoding.h>
#include "gui.h"
#undef GetObject

namespace Engine
{
	namespace GUI
	{

		/**
		@brief Simple label component
			
		A label simply displays a string and provides no further functionality. The label may be multi-lined
		*/
		class Label:public Component, public IToString
		{
			typedef Component			Parent;
			String						caption;	//!< Single line (complete) caption of the label
			Array<StringRef>			lines;			//!< Split (wrapped) caption
			bool						textChanged;	//!< Indicates that the text changed and the text should be re-wrapped before printing it the next time
			M::TVec4<GLfloat>			textColor;			//!< Label text color (solid white by default)
			float						lastWidth;
			M::Quad<float>				textMargin;	//!< Distance between text and the respective edge. All positive values (even up and right)
				
			void						_Setup();
			static	float				_CharLen(char32_t c);
		protected:
			/**/						Label(const String&type):Component("Label/"+type){_Setup();}		
		public:
			bool						wrapText,			//!< Indicates that the caption should be wrapped at t
										fillBackground;	//!< Fills the label background before writing the label caption (false by default)
			M::TVec3<GLfloat>				backgroundColor;	//!< Label background used if @b fillBackground is true (light blueish by default)
			TIcon						icon;				//!< Label icon (empty by default)
				
			/**/						Label():Component("Label")	{_Setup();}
			virtual	float				GetClientMinWidth()	const	override;
			virtual	float				GetClientMinHeight()	const	override;
			virtual	void				OnColorPaint(ColorRenderer&, bool parentIsEnabled)	override;
			const String&				GetText() const	{return caption;}	//!< Retrieves the current caption
			Label*						SetText(const String&text);		//!< Updates label caption
			inline Label*				SetTextColor(const M::TVec4<>&color)	{return SetColor(color);}
			Label*						SetColor(const M::TVec4<>&color);	//!< Updates label text color
			void						SetTextMargin(const M::Quad<float>&margin);
			const M::Quad<float>&		GetTextMargin() const	{return textMargin;}
			String						ConvertToString()	const override	{return caption;}	//!< Simple IToString::ConvertToString() override
			virtual	void				UpdateLayout(const M::Rect<float>&parent_region)	override;
		};
		
		typedef std::shared_ptr<Label>		PLabel;
		typedef std::shared_ptr<const Label>PConstLabel;

		class SliderLayout
		{
		public:
			TTexture					barLeft,
										barCenter,
										slider;
			float						minWidth,
										minHeight;
			static  SliderLayout		global;	//!< Global slider layout, applied by default to the appearance of a new slider

			void						LoadFromFile(const PathString&filename, float scale=1.0f);
		};
		
		/**
		@brief Scrollbar layout
			
		A scrollbar layout defines the shapes and textures used by a vertical scrollbar. Horizontal scrollbars are supposed to simply rotate the textures/shapes.
		The visual scrollbar is defined by each the center (stretching) part and the bottom end of both the background (back) and the cursor. The top end is supposed to be
		simply the mirrored bottom end of those two.
		Of the two scrollbuttons only the bottom one is defined as one solid texture/shape. Again the top button is expected to be the reflected bottom button.
		*/
		class ScrollBarLayout
		{
		public:
			TTexture					backCenter,	//!< Center fragment of the background
										backBottom,	//!< Bottom fragment of the background
										cursorCenter,	//!< Center fragment of the cursor
										cursorBottom,	//!< Bottom fragment of the cursor
										bottomButton;	//!< Down button
			float						buttonIndent,	//!< Upper space of the button that is both used by the cursor and the button. Sort of like an overlapping if the shape between button and cursor is not straight
										minWidth,		//!< Minimum width of the scrollbar. This is in fact the same as the maximum width since the width of a vertical scrollbar is usually static.
										minHeight;		//!< Minimum height of the scrollbar
			/**
			@brief Loads the local scrollbar layout from an xml file
			@param filename Path to the xml file
			@param scale Scale to apply to the loaded layout
			@param error_out String to store an error description in (in case of an error) or NULL if the error output should be ignored.
			@return true on success
			*/
			void						LoadFromFile(const PathString&filename, float scale=1.0f);
				
			static  ScrollBarLayout		global;	//!< Global scrollbar layout, applied by default to the appearance of a new scrollbar
		};
		
		
		/**
		@brief Data structure used by scrollbar to specify the current scrollbar status
		*/
		struct TScrollData
		{
			float						min,		//!< Smallest scrollbar value
										max,		//!< Greatest scrollbar value
										window,		//!< Window covered by the scrollbar view (effectivly defines the length of the bar)
										current;	//!< Scroll window position ranging 0 to 1
										
			/**/						TScrollData():min(0),max(0),window(100),current(0){}
		};

		/**
		@brief Abstract scrollable object
			
		Scrollable defines an abstract object that can be scrolled on up to two directions
		*/
		class Scrollable
		{
		public:
			TScrollData					horizontal,	//!< Horizontal scrolldata
										vertical;	//!< Vertical scrolldata
				
			virtual						~Scrollable()	{};
			virtual	void				OnScroll()				{}
			virtual	void				OnHorizontalScroll()	{OnScroll();}	//!< Triggered if a horizontal scrollbar - that this scrollable component has been attached to - was scrolled
			virtual	void				OnVerticalScroll()		{OnScroll();}	//!< Triggered if a vertical scrollbar - that this scrollable component has been attached to - was scrolled
		};
		
		
		
		/**
			@brief Scrollbar component
			
			The scrollbar is vertical by default but can be horizontal too
		*/
		class ScrollBar: public Component
		{
			typedef Component			Parent;
			float						cursorHook[2],
										cursorRange;
			bool						upPressed,
										downPressed,
										cursorGrabbed;
			TFreeCell					backgroundTop,
										backgroundCenter,
										backgroundBottom,
										cursorTop,
										cursorCenter,
										cursorBottom,
										upButton,
										downButton;
			M::Rect<float>					cursorRegion;
				
			void						_Setup();
			static void					_PaintColor(const TCell&);
			static void					_PaintNormal(const TCell&, bool);
		public:
			std::weak_ptr<Scrollable>	scrollable;
			bool						horizontal,		//!< Indicates that the scrollbar is horizontal rather than vertical
										autoVisibility;	//!< Indicates that the governing structure (ie a scrollbox) should attempt to determine and update the visibility of this scrollbar automatically
			FunctionalEvent				onScroll;		//!< Event that is fired if the scrollbar changes
			TScrollData					scrollData;		//!< Current scrollbar state
			ScrollBarLayout*			scrollLayout;		//!< Inner scrollbar layout. Functionality is undetermined if the scrollbar has no inner layout
										
										
				
			/**/						ScrollBar():Component("ScrollBar")
										{
											_Setup();
											anchored.Set(false,true,true,true);
											width = GetMinWidth(false);
											height = GetMinHeight(false);
											scrollData.window = height;
										}
			/**/						ScrollBar(bool horizontal_):Component("ScrollBar")
										{
											_Setup();
											horizontal = horizontal_;
											width = GetMinWidth(false);
											height = GetMinHeight(false);
											if (horizontal)
											{
												anchored.Set(true,true,true,false);
												scrollData.window = width;
											}
											else
											{
												anchored.Set(false,true,true,true);
												scrollData.window = height;
											}
										}
			void						ScrollTo(float v);
			virtual	void				OnScroll();	//!< Triggered if the scrollbar changes. Invokes the local event container and notifies the linked scrollable by default
			virtual	void				UpdateLayout(const M::Rect<float>&parent_region) override;
			virtual	float				GetClientMinWidth()	const override;
			virtual	float				GetClientMinHeight()	const override;
			virtual	void				OnColorPaint(ColorRenderer&, bool parentIsEnabled) override;
			virtual	void				OnNormalPaint(NormalRenderer&, bool parentIsEnabled) override;
			virtual	eEventResult		OnMouseDrag(float x, float y) override;
			virtual	eEventResult		OnMouseDown(float x, float y, TExtEventResult&) override;
			virtual	eEventResult		OnMouseUp(float x, float y) override;
			virtual	eEventResult		OnMouseWheel(float x, float y, short delta) override;
			virtual bool				CanHandleMouseWheel()	const override {return true;}
		};
		typedef std::shared_ptr<ScrollBar>		PScrollBar;
		typedef std::shared_ptr<const ScrollBar>PConstScrollBar;
		


		/**
			@brief Horizontal slider component
			
			Strictly horizontal slider component
		*/
		class Slider: public Component
		{
			typedef Component			Parent;
			float						cursorHook[2];
			bool						cursorGrabbed;
			TFreeCell					barLeft,
										barCenter,
										barRight,
										slider;
			float						slideFrom,
										slideRange;
			float						current,
										max;
			M::Rect<float>					cursorRegion;
				
			void						_Setup();
			void						_UpdateCursorRegion();
				
		public:

			FunctionalEvent				onSlide;		//!< Event that is fired if the slider changes
			SliderLayout*				sliderLayout;		//!< Inner slider layout. Functionality is undetermined if the slider has no inner layout
										
										
				
			/**/						Slider():Component("Slider")
										{
											_Setup();
											anchored.Set(true,false,true,true);
											width = GetMinWidth(false);
											height = GetMinHeight(false);
										}
			virtual	void				OnSlide();	//!< Triggered if the slider changes. Invokes the local event container by default
			virtual	void				UpdateLayout(const M::Rect<float>&parent_region) override;
			virtual	float				GetClientMinWidth()	const override;
			virtual	float				GetClientMinHeight()	const override;
			virtual	void				OnColorPaint(ColorRenderer&, bool parentIsEnabled) override;
			virtual	void				OnNormalPaint(NormalRenderer&, bool parentIsEnabled) override;
			virtual	eEventResult		OnMouseDrag(float x, float y) override;
			virtual	eEventResult		OnMouseDown(float x, float y, TExtEventResult&) override;
			virtual	eEventResult		OnMouseUp(float x, float y) override;
			virtual	eEventResult		OnMouseWheel(float x, float y, short delta) override;
			virtual bool				CanHandleMouseWheel()	const override {return true;}
		
			inline float				GetMax()const {return max;}
			inline float				GetCurrent() const {return current;}
			void						SetMax(float max_)	{if (max_ != max) {max = max_; SignalLayoutChange();}}
			void						SetCurrent(float current_)	{if (current_ != current) {current = current_; SignalLayoutChange();}}
		};
		typedef std::shared_ptr<Slider>		PSlider;
		typedef std::shared_ptr<const Slider>PConstSlider;
	


		/**
			@brief Panel component
			
			Panels store a collection of child GUI
		*/
		class Panel:public Component
		{
			typedef Component			Parent;
		protected:
			PreAllocVector<PComponent,4>		children;

			/**/						Panel(const String&typeName):Component("Panel/"+typeName)	//!< Creates a derivative panel
										{
											layout = globalLayout.Refer();
											anchored.SetAll(true);
										}
		public:
			static Layout				globalLayout;	//!< Default panel layout
				
			/**/						Panel():Component("Panel")	//!< Creates a plain panel
										{
											layout = globalLayout.Refer();
											anchored.SetAll(true);
										}
			virtual	void				UpdateLayout(const M::Rect<float>&parent_region) override;
			virtual	float				GetClientMinWidth()	const override;
			virtual	float				GetClientMinHeight()	const override;
			virtual	void				OnColorPaint(ColorRenderer&, bool parentIsEnabled) override;
			virtual	void				OnNormalPaint(NormalRenderer&, bool parentIsEnabled) override;
			virtual PComponent			GetComponent(float x, float y, ePurpose purpose, bool&outIsEnabled) override;
			bool						GetChildSpace(M::Rect<float>&out_rect)	const;	//!< Retrieves the space currently occupied by the children of this panel @param out_rect Rectangle container to store the child space in @return true if the local panel has at least one child, false otherwise
		
			void						Append(const PComponent&component);	//!< Appends  the specified component beneath the last member component of the local panel
			void						AppendRight(const PComponent&component);	//!< Appends the specified component to the right of the last member component of the local panel
			/**
			@brief Adds a row in the form "[caption][tab][component]" to the end of the panel
					
			AddRow() adds a new label with the specified caption as well as the specified GUI to the bottom end of the local panel's elements
			@param caption Caption to assign to the created label object. The label object is located at the left edge underneath the last child component
			@param caption_width Minimal width to allocate for the caption label
			@param component Component object to Add. The component is located to the right of the newly created caption, at least @a caption_width to right of the left edge of the label.
			@return @a component
			*/
			template <class Component>
				std::shared_ptr<Component>	AddRow(const String&caption, float caption_width, const std::shared_ptr<Component>&component)
				{
					if (!component)
						return std::shared_ptr<Component>();
					std::shared_ptr<GUI::Component> last = children.IsNotEmpty()?children.Last():std::shared_ptr<GUI::Component>();
					float	y_offset = (last?last->offset.top-last->height:0),
							x_offset = 0;

					std::shared_ptr<GUI::Label> label(new GUI::Label());
					label->SetText(caption);
					
					float h = vmax(label->height,component->height);

					
					label->anchored.Set(true,false,false,true);
					label->offset.left = x_offset;
					label->offset.top = y_offset-h/2+label->height/2;
					this->Add(label);
					x_offset = M::Max(caption_width,label->width);
					
					component->anchored.Set(true,false,true,true);
					component->offset.left = x_offset;
					component->offset.top = y_offset-h/2+component->height/2;
					this->Add(component);
					return component;
				}
				
				
			virtual	bool				Add(const PComponent&component);		//!< Adds a component to the panel. The added panel is drawn last 	@return true on success
			virtual	bool				Erase(const PComponent&component);	//!< Erases a component from the panel.		@return true on success
			virtual	bool				Erase(index_t index);			//!< Erases a component by index from the panel		@return true on success
			virtual	bool				MoveChildUp(const PComponent&component);	//!< Moves a component further up in the order of child GUI			@return true on success
			virtual	bool				MoveChildUp(index_t index);			//!< Moves a component by index further up in the order of child GUI	@return true on success
			virtual	bool				MoveChildDown(const PComponent&component);	//!< Moves a component further down in the order of child GUI		@return true on success
			virtual	bool				MoveChildDown(index_t index);		//!< Moves a component by index further down in the order of child GUI	@return true on success
			virtual	bool				MoveChildToTop(const PComponent&component);	//!< Moves a component to the top of the order of child GUI			@return true on success
			virtual	bool				MoveChildToTop(index_t index);		//!< Moves a component by index to the top of the order of child GUI	@return true on success
			virtual	bool				MoveChildToBottom(const PComponent&component);	//!< Moves a component to the bottom of the order of child GUI	@return true on success
			virtual	bool				MoveChildToBottom(index_t index);			//!< Moves a component by index to the bottom of the order of child GUI	@return true on success
			virtual	PConstComponent		GetChild(index_t index) const override	{return children[index];}
			virtual	PComponent			GetChild(index_t index) override	{return children[index];}
			virtual	count_t				CountChildren() const override {return children.Count();}
			virtual	index_t				GetIndexOfChild(const PComponent&child)const	override {return children.GetIndexOf(child)+1;}	//!< Queries the index of a child +1. @return Index+1 or 0 if the child could not be found
			virtual	void				clear() {children.clear();}
		};
		typedef std::shared_ptr<Panel>		PPanel;
		typedef std::shared_ptr<const Panel>	PConstPanel;
		
		/**
			@brief Scrollable panel
			
			A scrollbox automatically provides a horizontal and vertical scrollbar. Both can be set invisible individually.
		*/
		class ScrollBox: public Panel, public Scrollable
		{
			typedef Panel				Super;

			M::Rect<float>						effectiveClientRegion;
			PreAllocVector<PComponent,4>	visible_children;
				

			void						createBars()
										{
											horizontalBar.reset(new ScrollBar(true));
											verticalBar.reset(new ScrollBar(false));
										}
		protected:
			/**/						ScrollBox(const String&sub_type_name):Panel("ScrollBox/"+sub_type_name)
										{
											createBars();
										}
		public:
			PScrollBar					horizontalBar,	//!< Horizontal scrollbar
										verticalBar;	//!< Vertical scrollbar
		
			/**/						ScrollBox():Panel("ScrollBox")
										{
											createBars();
										}
			std::shared_ptr<Scrollable>	toScrollable()
										{
											return std::static_pointer_cast<Scrollable, ScrollBox>(std::static_pointer_cast<ScrollBox, Component>(shared_from_this()));
										}
		//virtual	void				OnScroll();
			virtual	void				UpdateLayout(const M::Rect<float>&parent_region) override;
			virtual	float				GetClientMinWidth()	const override;
			virtual	float				GetClientMinHeight()	const override;
			virtual	void				OnColorPaint(ColorRenderer&, bool parentIsEnabled) override;
			virtual	void				OnNormalPaint(NormalRenderer&, bool parentIsEnabled) override;
			virtual PComponent			GetComponent(float x, float y, ePurpose, bool&outIsEnabled) override;
			virtual	eEventResult		OnMouseWheel(float x, float y, short delta) override;
			virtual bool				CanHandleMouseWheel()	const override;
			void						Append(const PComponent&component);
			virtual	PConstComponent		GetChild(index_t index) const override;
			virtual	PComponent			GetChild(index_t index) override;
			virtual	size_t				CountChildren() const override;
			virtual	bool				Erase(const PComponent&component) override;
			virtual	bool				Erase(index_t index) override;
			virtual	void				clear() override	{children.reset();visible_children.reset();}

			void						ScrollTo(float x, float y);
			void						scrollToX(float x);
			void						scrollToY(float y);

		};
		typedef std::shared_ptr<ScrollBox>		PScrollBox;
		typedef std::shared_ptr<const ScrollBox>PConstScrollBox;
		
		/**
			@brief Button component
		*/
		class Button:public Component
		{
			typedef Component			Super;
			bool						down,
										pressed;
			String						caption;		//!< Button caption
		protected:
			bool						constantlyDown;
		public:
			static	Layout				globalLayout;	//!< Global default button layout
			FunctionalEvent				onExecute;		//!< Event that is fired if the button is pressed

			/**/						Button():Component("Button"),down(false),pressed(false),constantlyDown(false),caption("Button")
										{
											layout = globalLayout.Refer();
											width = GetMinWidth(false);
											height = GetMinHeight(false);
										}
			/**/						Button(const String&caption_):Component("Button"),down(false),pressed(false),constantlyDown(false),caption(caption_)
										{
											layout = globalLayout.Refer();
											width = GetMinWidth(false);
											height = GetMinHeight(false);
										}
			void						SetConstantlyDown(bool b)	{constantlyDown = b; SignalVisualChange();}
			bool						IsConstantlyDown()	const	{return constantlyDown;}
			virtual	void				OnColorPaint(ColorRenderer&, bool parentIsEnabled)	override;
			virtual	void				OnNormalPaint(NormalRenderer&, bool parentIsEnabled)	override;
			virtual	float				GetClientMinWidth()	const	override;
			virtual	float				GetClientMinHeight()	const	override;
			virtual	eEventResult		OnMouseDrag(float x, float y)	override;
			virtual	eEventResult		OnMouseDown(float x, float y, TExtEventResult&)	override;
			virtual	eEventResult		OnMouseUp(float x, float y)	override;
			virtual	eEventResult		OnKeyDown(Key::Name key)	override;
			virtual	eEventResult		OnKeyUp(Key::Name key)	override;
		
			void						SetCaption(const String&caption_)	//!< Updates the caption of the local string and automatically adjusts the local button width to match the minimum required size
										{
											if (caption == caption_)
												return;
											caption = caption_;
											float newWidth = GetMinWidth(false);
											if (!M::Similar(newWidth,width))
											{
												SignalLayoutChange();
												width = newWidth;
											}
											else
												SignalVisualChange();
										}
			virtual	void				OnExecute()
										{
											onExecute();
										}
			inline bool					AppearsPressed()	const	{return pressed || constantlyDown;}
		};
		typedef std::shared_ptr<Button>		PButton;
		typedef std::shared_ptr<const Button>PConstButton;
		
		
		/**
			@brief Input component
			
			Alles editing of a string including the most common operations. Also supports copy&paste
		*/
		class Edit:public Component
		{
		public:
			typedef GenericStringBuffer<char32_t> TextBuffer;
		private:
			typedef Component			Super;
			bool						goLeft,	//!< Indicates that during the next onTick() invocation the view should move to the left
										goRight,	//!< Indicates that during the next onTick() invocation the view should move to the right
										viewRightMost;	//!< Indicates that the view cannot move any further to the right
			count_t						cursorTicks;		//!< Tick counter for cursor visibility
			float						cursorOffset,
										cursorLength;
			TextBuffer					text;			//!< Edit buffer
			size_t						cursor,			//!< Current cursor location
										selectionStart,		//!< Selection start. This can be before or after the cursor location
										viewBegin,		//!< First visible character
										viewEnd;		//!< One past the last visible character

			void						_Setup();
			void						_UpdateView();
			inline float				_GetCharWidth(char c)
										{
											if (maskInput)
												return ColorRenderer::textout.GetFont().GetHeight()*0.75f;
											return ColorRenderer::textout.GetFont().GetWidth(c);
										}
			inline float				_GetTextWidth(const char*str, count_t stringLength)	const {return _GetTextWidth(StringRef(str,stringLength));}
			inline float				_GetTextWidth(const char32_t*str, count_t stringLength)	const {return _GetTextWidth(UTF32StringRef(str,stringLength));}
			inline float				_GetTextWidth(const StringRef&str) const
										{
											if (maskInput)
												return ColorRenderer::textout.GetFont().GetHeight()*0.75*str.GetLength();
											return ColorRenderer::textout.GetUnscaledWidth(str);
										}
			inline float				_GetTextWidth(const UTF32StringRef&str) const
										{
											if (maskInput)
												return ColorRenderer::textout.GetFont().GetHeight()*0.75*str.GetLength();
											return ColorRenderer::textout.GetUnscaledWidth(str);
										}
		protected:
			/**/						Edit(const String&type):Component("Edit/"+type),text(8)	{_Setup();}
		public:
			union
			{
				bool					readonly;		//!< Indicates that this edit can be read and copied but not modified
				bool					readOnly;		//!< Mapping to readonly
			};
			union
			{
				bool					maskInput;		//!< Render dots instead of characters. Prevents copying from this edit (but still allows pasting into it)
				bool					maskedInput;	//!< Mapping to maskInput
				bool					maskedView;	//!< Mapping to maskInput
			};
			FunctionalEvent				onChange,	//!< Event that is fired if the local input is changed
										onEnter;	//!< Event that is fired if the user pressed enter or return on this edit field
			bool						(*acceptChar)(char32_t);	//!< Pointer to a char filter. Only if the function returns true then the character is inserted
				

			static Layout				globalLayout;	//!< Global default edit layout
				
			/**/						Edit():Component("Edit"),readonly(false),maskInput(false),text(8),acceptChar(NULL)	//!< Creates a standard edit
										{
											_Setup();
										}
			void						SetText(const char*new_text);
			void						SetText(const String&new_text);
			void						SetText(const StringRef&new_text);
			TextBuffer&					GetBuffer()			{SignalVisualChange(); return text;}
			const TextBuffer&			GetBuffer() const	{return text;}
			UTF32String					GetTextUTF32()		const {return text.ToStringRef();}
			UTF32StringRef				GetTextRefUTF32()	const {return text.ToStringRef();}
			String						GetTextUTF8()		const {return StringEncoding::UTF32::ToUTF8(text.ToStringRef());}
			virtual	eEventResult		OnFocusGained()	override				{cursorTicks = 0; return RequestingRepaint;}
			virtual	eEventResult		OnFocusLost()	override					{return RequestingRepaint;}
			virtual	float				GetClientMinWidth()	const	override		{return 50;}
			virtual	float				GetClientMinHeight()	const	override		{return ColorRenderer::textout.GetFont().GetHeight();}
			virtual	void				OnColorPaint(ColorRenderer&, bool parentIsEnabled)	override;
			virtual	eEventResult		OnKeyDown(Key::Name key)	override;
			virtual	eEventResult		OnKeyUp(Key::Name key)	override;
			virtual	eEventResult		OnChar(char32_t c)	override;
			virtual	eEventResult		OnMouseDrag(float x, float y)	override;
			virtual	eEventResult		OnMouseDown(float x, float y, TExtEventResult&)	override;
			virtual	eEventResult		OnMouseHover(float x, float y, TExtEventResult&)	override;
			virtual	eEventResult		OnMouseUp(float x, float y)	override;
			virtual	eEventResult		OnTick()	override;
			virtual	bool				IsTabFocusable()	const	override				{return true;}
			virtual	void				UpdateLayout(const M::Rect<float>&parent_region)	override;
		};
		typedef std::shared_ptr<Edit>		PEdit;
		typedef std::shared_ptr<const Edit>	PConstEdit;
		
		typedef Edit	Input;
		
		class Menu;
		typedef std::shared_ptr<Menu>		PMenu;
		typedef std::shared_ptr<const Menu>	PConstMenu;
		
		/**
		@brief Standardized menu entry component
		*/
		class MenuEntry:public Label
		{
			typedef Label				Super;
			bool						menuIsOpen;
			std::weak_ptr<Menu>			parent;		//!< Pointer to the menu that contains this entry. NULL by default, indicating no parent.
			PWindow						menuWindow;	//!< Pointer to the window containing a sub menu (if any). NULL if no sub menu has been created, automatically deleted on destruction
			std::shared_ptr<IToString>	object;	//!< Link to the object used to dynamically retrieve the local entry caption. NULL by default @b Not automatically deleted.
			friend class Menu;
			void						_Setup();
			void						OnMenuHide();
		protected:
			/**/						MenuEntry(const String&sub_type_name):Label("MenuEntry/"+sub_type_name)	{_Setup();}
			void						CorrectMenuWindowSize()	const;
		public:
			FunctionalEvent				onExecute;		//!< Event fired if this menu entry is executed
			bool						openDown;	//!< Indicates that the sub menu of this item (if any) opens down, rather than to the right.
				
			/**/						MenuEntry():Label("MenuEntry")	{_Setup();}
			virtual						~MenuEntry();
			void						SetObject(const std::shared_ptr<IToString>&object);
			const std::shared_ptr<IToString>&GetObject() const	{return object;}
			const PWindow&				RetrieveMenuWindow();	//!< Retrieves (and possibly creates) the window containing the sub menu entries
			const PWindow&				GetMenuWindow() const	{return menuWindow;}
			PMenu						GetMenu();			//!< Retrieves (and possibly creates) the sub menu of this entry
			PConstMenu					GetMenu()	const;	//!< @overload
			void						DiscardMenu();	//!< Discards that local sub menu
			virtual	void				OnColorPaint(ColorRenderer&, bool parentIsEnabled) override;	
			virtual	eEventResult		OnMouseDown(float x, float y, TExtEventResult&) override;
			virtual	eEventResult		OnMouseHover(float x, float y, TExtEventResult&) override;
			virtual	eEventResult		OnFocusGained() override;
			virtual	eEventResult		OnFocusLost() override;
			virtual	eEventResult		OnMouseExit() override;
			virtual	eEventResult		OnKeyDown(Key::Name key) override;

			virtual	void				OnMenuClose(const std::shared_ptr<MenuEntry>&child);
			virtual	void				UpdateLayout(const M::Rect<float>&parent_region) override;
		};
		typedef std::shared_ptr<MenuEntry>		PMenuEntry;
		typedef std::shared_ptr<const MenuEntry>	PConstMenuEntry;
		
		
		/**
			@brief Standard menu component containing menu entries
			
			The Menu container enforces that all of its children are of type MenuEntry
		*/
		class Menu:public ScrollBox
		{
			typedef ScrollBox			Super;

			std::weak_ptr<MenuEntry>	parent;	//!< Parent menu entry. NULL by default
			index_t						selectedEntry;	//!< Currently GetSelected menu entry
			PComponent					selectedComponent;
			bool						horizontal,autoResize;
			M::TVec4<>						leftBackgroundColor,rightBackgroundColor,entryTextColor;
			M::TVec3<>						entryBackgroundColor;
			friend class MenuEntry;
			using ScrollBox::Append;
			void						_Setup();
			void						_ArrangeItems();
			void						_SelectMenuEntry(const PComponent&);
		protected:
			/**/						Menu(const String&sub_type_name):ScrollBox("Menu/"+sub_type_name){_Setup();}
		public:
			static	Layout				globalLayout;	//!< Global standard menu layout
		
			/**/						Menu():ScrollBox("Menu"){_Setup();}
			inline bool					IsHorizontal()	const {return horizontal;}
			void						SetHorizontal(bool b);
			inline bool					DoesAutoResize() const { return autoResize;}
			inline void					SetAutoResize(bool b)	{autoResize = b; _ArrangeItems();}
			inline void					SetBackgroundColor(const M::TVec4<>&color)	{leftBackgroundColor = rightBackgroundColor = color; SignalVisualChange();}
			inline void					SetBackgroundColor(const M::TVec4<>&left, const M::TVec4<>&right)	{leftBackgroundColor = left; rightBackgroundColor = right; SignalVisualChange();}
			inline const M::TVec4<>&		GetLeftBackgroundColor()	const	{return leftBackgroundColor;}
			inline const M::TVec4<>&		GetRightBackgroundColor()	const	{return rightBackgroundColor;}
			void						SetEntryTextColor(const M::TVec4<>&);
			inline const M::TVec4<>&		GetEntryTextColor() const	{return entryTextColor;}
			void						SetEntryBackgroundColor(const M::TVec3<>&color);
			inline const M::TVec3<>&		GetEntryBackgroundColor() const {return entryBackgroundColor;}

			virtual	PMenuEntry			Add(const String&caption);	//!< Adds a new simple menu entry to this menu @param caption Caption of the menu entry @return Pointer to the newly created menu entry
			virtual	bool				Add(const PComponent&component) override;	//!< Adds a component to this menu. Must of a MenuEntry of derivative class. @param component Menu entry to Add. The method fails if the passed pointer is NULL or not of type MenuEntry @return true on success
			virtual	bool				Erase(const PComponent&component) override;
			virtual	bool				Erase(index_t index) override;
			virtual	bool				MoveChildUp(const PComponent&component) override;
			virtual	bool				MoveChildUp(index_t index) override;
			virtual	bool				MoveChildDown(const PComponent&component) override;
			virtual	bool				MoveChildDown(index_t index) override;
			virtual	bool				MoveChildToTop(const PComponent&component) override;
			virtual	bool				MoveChildToTop(index_t index) override;
			virtual	bool				MoveChildToBottom(const PComponent&component) override;
			virtual	bool				MoveChildToBottom(index_t index) override;
			virtual	void				OnColorPaint(ColorRenderer&, bool parentIsEnabled) override;	
		
			virtual	M::TVec2<>				GetIdealSize()	const;		//!< Retrieves the ideal width/height of this menu

			virtual	eEventResult		OnKeyDown(Key::Name key) override;
		};
		
		/**
		@brief Combobox component
			
		The combobox expands a menu if clicked allowing the user to choose a new element
		*/
		class ComboBox:public MenuEntry
		{
		private:
			index_t					selectedEntry;
			void					_Setup();
		protected:
			/**/					ComboBox(const String&sub_type_name):MenuEntry("ComboBox/"+sub_type_name){_Setup();}
		public:
			std::shared_ptr<IToString>	selectedObject;	//!< Currently GetSelected object (if any)
			FunctionalEvent			onChange;		//!< Event that is fired if the GetSelected item changed
			static Layout			globalLayout;
				
			/**/					ComboBox():MenuEntry("ComboBox"){_Setup();}
			virtual	float			GetMinWidth(bool includeOffsets)	const override;
			virtual	void			UpdateLayout(const M::Rect<float>&parent_space) override;
			void					Select(index_t index);
			index_t					GetSelected()	const	{return selectedEntry;}
			virtual	void			OnMenuClose(const PMenuEntry&child) override;
		};
		typedef std::shared_ptr<ComboBox>		PComboBox;
		typedef std::shared_ptr<const ComboBox>	PConstComboBox;
		
		/**
		@brief Checkbox component
		*/
		class CheckBox:public Component
		{
		private:
			bool						down, pressed;
			bool						checked;	//!< True if the checkbox is currently checked
			String						caption;	//!< Checkbox caption
		public:
			struct TStyle
			{
				OpenGL::Texture			boxColor,
										boxNormal,
										checkMark,
										highlightMark;
			};
				
			TStyle						*style;			//!< Inner checkbox style
			FunctionalEvent				onChange;
			static	TStyle				globalStyle;	//!< Global default checkbox style
				
			/**/						CheckBox():Component("Checkbox"),down(false),pressed(false),checked(false),caption("Checkbox"),style(&globalStyle)
										{
											width = GetMinWidth(false);
											height = GetMinHeight(false);
										}
			virtual	void				OnColorPaint(ColorRenderer&, bool parentIsEnabled) override;
			virtual	void				OnNormalPaint(NormalRenderer&, bool parentIsEnabled) override;
			virtual	float				GetMinWidth(bool includeOffsets)	const override;
			virtual	float				GetMinHeight(bool includeOffsets)	const override;
			virtual	eEventResult		OnMouseDrag(float x, float y) override;
			virtual	eEventResult		OnMouseDown(float x, float y, TExtEventResult&) override;
			virtual	eEventResult		OnMouseUp(float x, float y) override;
			inline bool					IsChecked()	const	{return checked;}
			void						SetChecked(bool b)	{if (checked == b) return; checked = b; SignalVisualChange();}
			inline const String&		GetCaption()	const {return caption;}
			CheckBox*					SetCaption(const String&caption_)
										{
											caption = caption_;
											width = GetMinWidth(false);
											SignalLayoutChange();
											return this;
										}
			inline float				GetBoxSize()	const	{return 16.f;}
			M::Rect<>						GetBoxRect() const
										{
											M::Rect<> rect=cellLayout.client;
											float cy = rect.y.center();
											float size = GetBoxSize();
											rect.x.max = rect.x.min + size;
											rect.y.SetCenter(cy,size/2);
											return rect;
										}
		};
		typedef std::shared_ptr<CheckBox>		PCheckBox;
		typedef std::shared_ptr<const CheckBox>	PConstCheckBox;
		
		
		void			LoadTheme(const String&theme_file, float scale=1.0f);
		void			ShowMessage(Operator&op, const String&title, const String&message);
		void			ShowMessage(Operator&op, const String&message);
		inline	void	ShowMessage(const POperator&op, const String&title, const String&message)	{if (!op)return; ShowMessage(*op,title,message);}
		inline	void	ShowMessage(const POperator&op, const String&message)						{if (!op)return; ShowMessage(*op,message);}
		bool			ShowingMessage();
		void			HideMessage();
		void			ShowChoice(Operator&op, const String&title, const String&query, const Array<String>&choices, const std::function<void(index_t)>&onSelect);
		inline void		ShowChoice(Operator&op, const String&query, const Array<String>&choices, const std::function<void(index_t)>&onSelect)	{ShowChoice(op,"Inquiry",query,choices,onSelect);}
		inline void		ShowChoice(const POperator&op, const String&title, const String&query, const Array<String>&choices, const std::function<void(index_t)>&onSelect)	{ShowChoice(*op,title,query,choices,onSelect);}
		inline void		ShowChoice(const POperator&op, const String&query, const Array<String>&choices, const std::function<void(index_t)>&onSelect)	{ShowChoice(*op,"Inquiry",query,choices,onSelect);}
	}
}


#endif
