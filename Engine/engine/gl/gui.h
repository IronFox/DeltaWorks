#ifndef gl_guiH
#define gl_guiH



#include "../renderer/opengl.h"
#include "../display.h"
#include <math/vector.h>
#include <io/file_system.h>
#include <io/xml.h>
#include <image/converter/magic.h>
#include <container/sorter.h>

#include "texture_font2.h"
#include <structure/event.h>
#include <memory> 

//#include "../../boost_integration/pointer.h"


//#define DEEP_GUI
#undef CreateWindow


namespace Engine
{
	
	/**
		@brief Advanced embedded window system
	*/
	namespace GUI
	{
		class Operator;
		typedef std::shared_ptr<Operator>	POperator;


		struct TCellLayout;

		
		static const float BorderWidth = 7;	//!< Resize border of windows in pixels
		
		static const float font_offset = -2;	//!< Vertical font offset
		
		static const float global_anisotropy = 20;

		enum modal_t
		{
			DefaultWindow,
			ModalWindow
		};

		/**
			@brief Simple hybrid texture containing both color and normal maps
		*/
		struct TTexture
		{
		public:
			float					width,	//!< Width of the texture (in scaled pixels)
									height;	//!< Height of the texture (in scaled pixels)
			OpenGL::Texture			color,	//!< Color map of the texture
									normal;	//!< Normal map of the texture
			static void				load(XML::Node*node, const char*aname, FileSystem::Folder&folder, TTexture&out, float scale);
			void					adoptData(TTexture&other)
									{
										width = other.width;
										height = other.height;
										color.adoptData(other.color);
										normal.adoptData(other.normal);
										other.width = 0;
										other.height = 0;
									}
		};
		
				
		struct TFreeCell	//! Free component layout cell
		{
			M::Rect<float>				region;			//!< Effective cell region
			BYTE					orientation;	//!< Cell orientation. Rotation is 90 degress times orientation
			const OpenGL::Texture	*color,			//!< Cell color texture
									*normal;			//!< Cell normal texture

			/**/					TFreeCell():color(NULL),normal(NULL)	{}
		};
		
		
		/**
			@brief Window layout cell
			
			Each window layout cell contains a relative location and texture used to render the cell
		*/
		struct TCell
		{
		private:
			TCell&					operator=(const TCell&other);
		public:
			float 					width;			//!< Cell width. Constant if @b variableWidth is false. Otherwise updated by WindowLoayout::locateCells()
			bool					variableWidth;	//!< Indicates whether or not this cell has a fixed or variable width
			OpenGL::Texture			colorTexture,	//!< Texture of this cell affecting plain color
									normalTexture;	//!< Texture of this cell affecting normals
				
			void					adoptData(TCell&other)
									{
										width = other.width;
										variableWidth = other.variableWidth;
										colorTexture.adoptData(other.colorTexture);
										normalTexture.adoptData(other.normalTexture);
										other.width = 0;
										other.variableWidth = false;
									}
		};
		
		struct TIOCell	//! Simplified io layout cell used during layout loading
		{
			Image					normal,	//!< Raw normal image
									color;	//!< Raw color image
			bool					variableWidth;	//!< Cell is of variable width

			void					swap(TIOCell&other)
			{
				normal.swap(other.normal);
				color.swap(other.color);
				swp(variableWidth,other.variableWidth);
			}
			friend void				swap(TIOCell&a, TIOCell&b)
			{
				a.swap(b);
			}
		};
		
		struct TIORow	//!< Simplified io layout row used during layout loading
		{
			Ctr::Vector0<TIOCell,Swap>	cells;	//!< Cell container
			bool					variableHeight;	//!< Row is of variable height
			void					swap(TIORow&other)
			{
				cells.swap(other.cells);
				swp(variableHeight,other.variableHeight);
			}
			friend void				swap(TIORow&a, TIORow&b)
			{
				a.swap(b);
			}
		};


		/**
			@brief Final layout cell row.
			
			Each row can hold any number of cells, both of fixed and variable width. The row itself may be of fixed or variable height.
		*/
		struct TRow
		{
		protected:
			TRow&					operator=(const TRow&other) {return *this;}
		public:
			Array<TCell>			cells;	//!< Layout cell container
			float					height;	//!< Row height
			bool					variable_height;	//!< True if this row is of variable height
			count_t					variable_cells;		//!< Number of cells with non-fixed widths. Always at least 1
			float					fixed_width;		//!< Summed width of all cells with fixed widths in this row
		};
		
		/**
			@brief General window/component layout
			
			A layout is a collection of cells arranged in rows. Layouts may be applied to both components and windows.
		*/
		class Layout
		{
		protected:
			static void				applyArea(M::Rect<float>&target, const M::Rect<float>&window, const M::Rect<float>&relative);
			Layout&					operator=(const Layout&other) {return *this;}
		public:
			Array<TRow>				rows;				//!< Collection of rows
			M::Rect<float>			titlePosition;		//!< Title position.	Negative values are interpreted relative to the right/top edges, positive ones to the left, bottom edges
			M::Quad<float>			borderEdge,		//!< Distance from the window/component edge to the effective (visual) edge of the layout. All values are >= 0
									clientEdge;		//!< Distance from the window/component edge to the client edge of the layout. All values are >= 0
			float					minWidth,			//!< Minimum width of this layout
									minHeight;			//!< Minimum height of this layout
			count_t					variableRows,		//!< Number of rows with non-fixed heights. Always at least 1
									cellCount;			//!< Total number of cells in this layout
			Layout					*override;			//!< Layout override

			/**/					Layout();

			/**
			@brief Loads a layout from the specified xml file.
					
			The method requires a valid OpenGL context
					
			@param filename Filename  of the xml file to load
			@param scale Relative scale that should be applied to the loaded layout
			*/
			void					LoadFromFile(const PathString&filename, float scale=1.0f);	
			void					UpdateCells(const M::Rect<float>&window_location, TCellLayout&layout)	const;		//!< Updates the final layout of a window or component depending on the window's location
			Layout*					Refer()	{return override!=NULL?override:this;}
		};
		
		

		/**
			@brief Instance of a layout cell applied to a window or component.
			
			Each window/component has its own collection of cell instances even if they use the same layout.
		*/
		struct TCellInstance
		{
			M::Rect<float>				region;			//!< Effective cell region (in pixels)
			const OpenGL::Texture	*colorTexture,	//!< Color texture to Fill the cell with
									*normalTexture;	//!< Normal texture to Fill the cell with
			//float					width;			//!< Effective width of the cell.
				
			/**/					TCellInstance():colorTexture(NULL),normalTexture(NULL){}
		};

		/**
			@brief Effective (applied) cell layout of a window or component
			
			This layout needs to be updated each time the window/component is resized or its layout is changed.
		*/
		struct TCellLayout
		{
			Array<TCellInstance>	cells;				//!< Container for all cell instances (generally unsorted)
			M::Rect<float>				title,				//!< Effective (absolute) title region
									border,				//!< Effective (absolute) region of the visual border. Mouse clicks outside this border are generally applied to the next lower window
									client;				//!< Effective (absolute) region of the client area of the layout.
		
			void					Clear(const M::Rect<>&windowLocation);
		};
		
		

		class Renderer
		{
		private:
			static GLShader::Instance	layerMerger;
			static GLShader::Variable	clearColorVariable;
			Buffer<M::Rect<int>,0>		clipStack;
			M::TVec3<>					clearColor;
			Resolution				subRes;
			TFrameBuffer			stackedTargets[2],
									layerTarget;
			bool					targetingFinal;
			count_t					layerCounter;
			void					_Apply(const M::Rect<int>&port);
			void					_Swap();
			void					_SetView(const M::Rect<int>&port);
		protected:

			bool					layerIsDirty;
			//Display<OpenGL>			&display;
			/**/					Renderer(float clearColorR, float clearColorG, float clearColorB) : layerIsDirty(false) {M::Vec::def(clearColor,clearColorR,clearColorG,clearColorB);}

			const Resolution&		GetTargetResolution()	const	{return subRes;}

			void					FillRect(const M::Rect<>&rect);
			void					FillQuad(const M::TVec2<>&p0, const M::TVec2<>&p1, const M::TVec2<>&p2, const M::TVec2<>&p3);
			void					TextureRect(const M::Rect<>&rect);
			void					TextureRect(const M::Rect<>&rect, const M::Rect<>&texCoordRect);
			void					TextureQuad(const M::TVec2<>&p0, const M::TVec2<>&p1, const M::TVec2<>&p2, const M::TVec2<>&p3);

			void					Configure(const TFrameBuffer&, const Resolution& usage);
			void					Finish();
			void					MarkNewLayer();
		public:
			virtual					~Renderer();

			void					Clip(const M::Rect<float>&region);	//!< Focuses on an area by applying the current viewport and translation to the specified region and further limiting the viewport. The existing translation will be modified by dx and dy
			void					Unclip();	//!< Reverts the focus process by jumping back to the next upper focus
			
			//inline void				ResetFocus()	{focusStack.clear();}
		};

		class ColorRenderer : public Renderer
		{
		private:
			M::TVec4<>					color;
			Buffer<M::TVec4<>,0>		colorStack;
			void					_UpdateState();
			void					_UpdateState(const GL::Texture::Reference&);
			void					_UpdateState(const GL::Texture::Reference&,const GL::Texture::Reference&);
		public:
			static Textout<GLTextureFont2>		textout;		//!< Global textout used to render text
			/**/					ColorRenderer() : Renderer(0,0,0)	{}
			void					ModulateColor(const M::TVec4<>&);
			void					ModulateColor(const M::TVec3<>&, float alpha=1.f);
			void					ModulateColor(float greyTone, float alpha=1.f);
			void					ModulateColor(float r, float g, float b, float a = 1.f);

			void					SetTextPosition(float x, float y);
			void					WriteText(const char*string, count_t length)	{WriteText(StringRef(string,length));}
			void					WriteText(const String&str)	{WriteText(str.ref());}
			void					WriteText(const StringRef&text);
			float					GetUnscaledWidth(const StringRef&text) const;
			float					GetUnscaledWidth(const char*text, count_t textLength) const {return GetUnscaledWidth(StringRef(text,textLength));}

			void					Paint(const TFreeCell&);
			void					SetPointSize(float size);
			void					PaintPoint(float x, float y);
			void					RenderLine(float x0, float y0, float x1, float y1);

			void					FillRect(const M::Rect<>&rect);
			void					FillQuad(const M::TVec2<>&p0, const M::TVec4<>&color0, const M::TVec2<>&p1, const M::TVec4<>&color1, const M::TVec2<>&p2, const M::TVec4<>&color2, const M::TVec2<>&p3,const M::TVec4<>&color3);
			void					TextureRect(const M::Rect<>&rect,const GL::Texture::Reference&);
			void					TextureRect(const M::Rect<>&rect, const M::Rect<>&texCoordRect,const GL::Texture::Reference&);
			void					TextureRect(const M::Rect<>&rect,const GL::Texture::Reference&,const GL::Texture::Reference&);
			void					TextureRect(const M::Rect<>&rect, const M::Rect<>&texCoordRect,const GL::Texture::Reference&,const GL::Texture::Reference&);
			void					TextureQuad(const M::TVec2<>&p0, const M::TVec2<>&p1, const M::TVec2<>&p2, const M::TVec2<>&p3, const GL::Texture::Reference&);
			void					TextureQuad(const M::TVec2<>&p0, const M::TVec2<>&p1, const M::TVec2<>&p2, const M::TVec2<>&p3, const GL::Texture::Reference&, const GL::Texture::Reference&);

			void					PushColor();
			void					PopColor();
			void					PeekColor();

			void					MarkNewLayer();

			GLTextureFont2&			GetFont()	const	{return textout.GetFont();}


			void					Configure(const TFrameBuffer&, const Resolution& usage);
			void					Finish();


			//void					ResetColor();
		};

		class NormalRenderer : public Renderer
		{
		private:
			static GLShader::Instance	normalRenderer;
			static GLShader::Variable	normalSystemVariable;
			M::TMatrix3<>				normalSystem;
			Buffer<M::TMatrix3<>,0>	normalSystemStack;

			void					_UpdateState(const GL::Texture::Reference&);
		public:
			/**/					NormalRenderer() : Renderer(0.5f,0.5f,1.f)	{}

			void					ScaleNormals(float x, float y);
			void					ScaleNormals(const M::TVec2<>&);
			void					ScaleNormals(float x, float y, float z);
			void					ScaleNormals(const M::TVec3<>&);
			void					TransformNormals(const M::TMatrix3<>&);

			void					PushNormalMatrix();
			void					PopNormalMatrix();
			void					PeekNormalMatrix();

			void					Paint(const TFreeCell&, bool invertNormals);

			void					TextureRect(const M::Rect<>&rect,const GL::Texture::Reference&);
			void					TextureRect(const M::Rect<>&rect, const M::Rect<>&texCoordRect,const GL::Texture::Reference&);
			void					TextureQuad(const M::TVec2<>&p0, const M::TVec2<>&p1, const M::TVec2<>&p2, const M::TVec2<>&p3, const GL::Texture::Reference&);

			void					MarkNewLayer();
	
			void					Configure(const TFrameBuffer&, const Resolution& usage);
			void					Finish();

		};

	
		class Window;
		typedef std::shared_ptr<Window>	PWindow;


		class Component;
		typedef std::shared_ptr<Component>	PComponent;
		typedef std::shared_ptr<const Component>	PConstComponent;
		
		/**
			@brief Root component
			
			Visual components are the abstract foundation of the GUI system.
		*/
		class Component: public std::enable_shared_from_this<Component>
		{
		private:
			static	void 						ReadChar(char c);
		protected:
			std::weak_ptr<Window>				windowLink;	//!< Link to the owning window. Not NULL if this component is part of any window.
			friend class Window;
			bool								enabled;		//!< Indicates that this item may receive events
			bool								visible;		//!< Indicates that this item is visible. Invisible items are automatically treated as disabled.
											
		public:
			static	void						SignalKeyDown(Key::Name key);	//!< Key down event linked to the keyboard interface
			static	void						SignalKeyUp(Key::Name key);		//!< Key up event linked to the keyboard interface
		
		
			enum eEventResult		//! Generic event result
			{
				Unsupported,		//!< The invoked event type is not supported by the component in its current state or at all
				Handled,			//!< The invoked event has been handled but no visual changes occured
				RequestingRepaint,	//!< The invoked event has been handled and the component requests a window repaint
				RequestingReshape	//!< The invoked event has been handled and the component requests a complete window layout update and repaint
			};

			enum ePurpose
			{
				GenericRequest,
				HoverRequest,
				ClickRequest,
				MouseWheelRequest
			};


			struct TExtEventResult	//! Struct to hold additional event results of certain event methods
			{
				Mouse::eCursor					customCursor;	//!< Cursor to replace the current one with
				//shared_ptr<Component>			caught_by;		//!< Component that actually caught the event
			};

			M::Rect<float>						currentRegion;	//!< Current component region. This rectangle completely surrounds the component including all cells of its layout (if any)
			M::Quad<float>						offset;			//!< Signed offset from the parent region. Effective only if @b anchored.coord[x] is true. should be negative for right/top offset
			M::Quad<bool>						anchored;		//!< Indicates that the respective coordinates of the final component region is calculated relative to the respective parent edge.
			float								width,			//!< Fixed component width if either anchored.left or anchored.right is false. Has no effect if both anchored.left and anchored.right are true
												height;			//!< Fixed component height if either anchored.bottom or anchored.top is false. Has no effect if both anchored.bottom and anchored.top are true
			const String						typeName;		//!< Constant type name of this component. Assigned during construction (usually the class name without the leading 'C')
			TCellLayout							cellLayout;		//!< Effective applied cell layout. This variable is updated even if this component has no layout
			Layout*								layout;			//!< Layout attached to this component or NULL if this component has no layout
			float								tickInterval;	//!< Interval (in seconds) between executions of the onTick() method


		
				
				
												Component(const String&typeName);	//!< Constructs a new component. Requires a typeName
			virtual								~Component();
			virtual	void						UpdateLayout(const M::Rect<float>&parentRegion);	//!< Updates the applied layout and possibly existing other components to the changed parent region rectangle. Also updates @b current_region @param parent_region Absolute location of the respective parent component or window
			virtual	float						GetClientMinWidth()	const	{return 0;}			//!< Queries the minimum width of the inner content of this component (excluding the minimum size of the layout)
			virtual	float						GetClientMinHeight()	const	{return 0;}			//!< Queries the minimum height of the inner content of this component (excluding the minimum size of the layout)
			virtual	float						GetMinWidth(bool includeOffsets)	const;			//!< Queries the effective minimum width of this component	@param includeOffsets Set true to also include anchor offsets @return Minimum width of this component
			virtual	float						GetMinHeight(bool includeOffsets)	const;			//!< Queries the effective minimum height of this component 	@param includeOffsets Set true to also include anchor offsets @return Minimum height of this component
			virtual	void						OnColorPaint(ColorRenderer&, bool parentIsEnabled);			//!< Causes this component to repaint its color components
			virtual	void						OnNormalPaint(NormalRenderer&, bool parentIsEnabled);		//!< Causes this component to repaint its normal components
			//virtual	shared_ptr<Component>		getFocused()					{return shared_from_this();};	//!< Retrieves the actually focused element from the component returned by onMouseDown(). Returns this by default
			virtual PComponent					GetComponent(float x, float y, ePurpose purpose, bool&outIsEnabled)	{outIsEnabled &= IsEnabled(); return shared_from_this();};
			virtual bool						IsEventTranslucent() const		{return false;}
			virtual	eEventResult				OnMouseDown(float x, float y, TExtEventResult&)		{return Unsupported;};	//!< Triggered if the primary mouse button was pressed over this component @param x Window space x coordinate of the mouse cursor @param y Window space y coordinate of the mouse cursor @return Event result
			virtual	eEventResult				OnMouseHover(float x, float y, TExtEventResult&)	{return Unsupported;};	//!< Triggered if the mouse was moved while above this component @param x Window space x coordinate of the mouse cursor @param y Window space y coordinate of the mouse cursor @return Event result
			virtual	eEventResult				OnMouseExit()					{return Unsupported;};						//!< Triggered if the mouse has left the area of this component @return Event result
			virtual	eEventResult				OnMouseDrag(float x, float y)	{return Unsupported;};						//!< Triggered if the mouse was moved while the primary mouse button is down and this component is the currently clicked component @param x Window space x coordinate of the mouse cursor @param y Window space y coordinate of the mouse cursor @return Event result
			virtual	eEventResult				OnMouseUp(float x, float y)		{return Unsupported;};						//!< Triggered if the primary mouse button was released and this component was the clicked component
			virtual	eEventResult				OnMouseWheel(float x, float y, short delta)		{return Unsupported;};		//!< Triggered if the mouse wheel was used while the mouse cursor is above this component
			virtual	eEventResult				OnFocusGained()					{return Unsupported;}						//!< Triggered if this component has gained the input focus
			virtual	eEventResult				OnFocusLost()					{return Unsupported;}						//!< Triggered if this component has lost the input focus
			virtual	eEventResult				OnKeyDown(Key::Name key)		{return Unsupported;}						//!< Triggered if the user has pressed a supported key while this component had the focus. This event may fire multiple times before the user releases the key and the onKeyUp() event method is triggered.
			virtual	eEventResult				OnKeyUp(Key::Name key)			{return Unsupported;}						//!< Triggered if the user has released a supported key while this component had the focus.
			virtual	eEventResult				OnChar(char c)					{return Unsupported;}						//!< Triggered if the user has pressed a character key while this component had the focus.
			virtual	eEventResult				OnTick()						{return Unsupported;}						//!< Triggered each time the counter hit @b tick_interval seconds while this component has the focus
			virtual	bool						IsTabFocusable() const			{return false;}								//!< Queries whether or not this entry is focusable via the tab key
			virtual bool						CanHandleMouseWheel() const		{return false;}
			void								SetWindow(const std::weak_ptr<Window>&wnd);						//!< Updates the local window link variable as well as that of all children (recursively)
			bool								IsFocused()	const;															//!< Queries whether or not this component currently has the input focus
			PWindow								GetWindow()	const				{return windowLink.lock();}						//!< Retrieves the super window of this component (if any)
			virtual	PConstComponent				GetChild(index_t) const			{return PConstComponent();}							//!< Queries the nth child of this component (if any)
			virtual	PComponent					GetChild(index_t)	{return PComponent();}									//!< @overload
			virtual	count_t						CountChildren()	const {return 0;}											//!< Queries the number of children of this component (if any) @return Number of children
			virtual	index_t						GetIndexOfChild(const PComponent&child) const;							//!< Determines the index of the specified child or 0xFFFFFFFF if the specified component is no child of this component.
			PComponent							GetSuccessorOfChild(const PComponent&child);									//!< Queries the successor element of the specified one @return successor or NULL if no successor could be found
			void								Locate(const M::Rect<float>&parentRegion,M::Rect<float>&region)	const;	//!< Resolves the absolute location of the local item based on the specified parent region.
			virtual void						SetEnabled(bool enabled);				//!< Enables/disables the ability of this component to receive events. Disabled components may have a different style. A redraw is automatically issued
			bool								IsEnabled()	const	{return enabled;}
			virtual void						SetVisible(bool visible);				//!< Changes the visibility of this component. A redraw is automatically issued
			bool								IsVisible()	const	{return visible;}
			void								SignalLayoutChange() const;					//!< Signals that the layout of the local component has changed in such a way that all components must be re-arranged
			void								SignalVisualChange() const;					//!< Signals that the local component must be redrawn
			
			static void							ResetFocused();																	//!< Unsets the currently focused component. Identical to passing an empty (null) pointer to SetFocused()
			static void 						SetFocused(const PComponent&component);								//!< Changes the currently focused component

			POperator							GetOperator()	const;
			POperator							RequireOperator()	const;
		};

		
		/**
			@brief Icon container
			
			The icon is considered invisible if the contained texture is empty.
			The values of the other variables have no effect in that case.
		*/
		struct TIcon
		{
			float								aspect;	//!< Pixel aspect (width/height) of the icon
			M::Rect<float>							texcoords;	//!< Texture coordinates of the icon in the specified texture
			OpenGL::Texture						texture;	//!< Effective (color) texture
		};

		
		
		
		/**
			@brief Window coordinates
			
			The specified coordinates describe the center of the window on the screen
		*/
		struct TScreenPosition
		{
			union
			{
				struct
				{
					float						x,		//!< X coordinates of the window in pixels. This value is multiplied by @b shellRadius to retrieve the absolute coordinates
												y,		//!< Y coordinates of the window in pixels. This value is multiplied by @b shellRadius to retrieve the absolute coordinates
												shellRadius;	//!< Effective shell radius (distance of the window from the screen). A higher shell radius will result in a smaller window
				};
				float							coord[3];	//!< Vector mapping of the above attributes
			};
		};


		CONSTRUCT_ENUMERATION4(SizeChange,Free,FixedWidth,FixedHeight,Fixed);

		class WindowPosition
		{
		public:
			M::TVec2<>			center;	//!< Window center position in pixels relative to the center of the screen. Actual size depending on to the current display resolution. (0,0) always points to the center of the screen. Defaults to (0,0)
			M::TVec2<>			size;	//!< Window size (x = width, y = height) in pixels. Actual size depending on to the current display resolution. Must be > 0 at all times. Defaults to 100x100
			SizeChange		sizeChange;
			bool			fixedPosition;	//!< True if the position of the finished window should be unchangeable, false otherwise. Defaults to false.

			/**/			WindowPosition(SizeChange sizeChange=SizeChange::Free, bool fixedPosition=false):sizeChange(sizeChange),fixedPosition(fixedPosition)	{M::Vec::clear(center);M::Vec::set(size,100);}
			/**/			WindowPosition(const M::TVec2<>&center, const M::TVec2<>&size,SizeChange sizeChange=SizeChange::Free, bool fixedPosition=false):center(center),size(size),sizeChange(sizeChange),fixedPosition(fixedPosition)	{}
			/**/			WindowPosition(float x, float y, float width, float height,SizeChange sizeChange=SizeChange::Free, bool fixedPosition=false):sizeChange(sizeChange),fixedPosition(fixedPosition)	{M::Vec::def(center,x,y); M::Vec::def(size,width,height);}
		};

		class NewWindowConfig
		{
		public:
			WindowPosition	initialPosition;
			String			windowName;
			bool			isModal;

			/**/			NewWindowConfig(bool isModal=false):windowName("Unnamed window"),isModal(isModal)	{}
			/**/			NewWindowConfig(const String&name, bool isModal=false):windowName(name),isModal(isModal)	{}
			/**/			NewWindowConfig(const String&name, const WindowPosition&position, bool isModal=false):initialPosition(position), windowName(name),isModal(isModal)	{}
		};


		/**
			@brief GUI Window
		*/
		class Window: public std::enable_shared_from_this<Window>
		{
		protected:
			friend class Component;
			friend void	 render();
				//friend bool	 mouseWheel(short delta);
		private:
			Layout					*layout;		//!< Used layout (if any) or NULL. Unmanaged at this point.
			TCellLayout				cellLayout;	//!< Actual layout as applied by the active layout.
		public:
			std::shared_ptr<Component>		rootComponent;
			std::weak_ptr<Operator>			operatorLink;
				
			struct ClickResult
			{
				enum value_t	//! Click result returned by the Resolve() method
				{
					Missed,			//!< The mouse cursor is not above the window
					Ignored,		//!< The mouse cursor is above the window but a click would have no effect
					Component,		//!< The mouse cursor is above the component
					DragWindow,			//!< The mouse cursor is above the window but not above the edges or the component causing the window to be dragged if the primary mouse button is pressed.
					ResizeTopRight,		//!< The mouse cursor is above the top right resize corner of the window
					ResizeRight,		//!< The mouse cursor is above the right window edge
					ResizeBottomRight,	//!< The mouse cursor is above the bottom right resize corner of the window
					ResizeBottom,		//!< The mouse cursor is above the bottom window edge
					ResizeBottomLeft,	//!< The mouse cursor is above the bottom left resize corner of the window
					ResizeLeft,			//!< The mouse cursor is above the left window edge
					ResizeTopLeft,		//!< The mouse cursor is above the top left resize corner of the window
					ResizeTop			//!< The mouse cursor is above the top window edge
				};
			};
				
		
			#ifdef DEEP_GUI
				TScreenPosition		current_center,	//!< Current shell location of this window (x,y,radius)
									origin,			//!< Origin shell location
									destination;	//!< Target shell location
			#else
				float				x,				//!< Central window X position (center point) in the range [-display.GetClientWidth()/2, display.GetClientWidth()/2]
									y;				//!< Central window Y position (center point) in the range [-display.GetClientHeight()/2, display.GetClientHeight()/2]
			#endif
			M::TVec2<UINT>				exp;			//!< Current texture exponent along the respective axis
			Resolution				size;			//!< Window size
			M::TVec2<>					fsize,			//!< Window size as float
									usage;			//!< Usage of the texture buffers in the respective direction (0-1)

			float					progress;		//!< Animation progress from @b origin to @b destination (0-1)
				
			static	Layout			commonStyle,	//!< Common window style
									menuStyle,		//!< Menu window
									hintStyle;		//!< Hint display window
			String					title;			//!< Window title. Empty by default
			/*EClickResult			click_result,	//!< Result of the last click event that was caught (or not caught) by this window
									hover_result;	//!< Result of the last mouse hover event that was processed by this window*/
			TFrameBuffer			colorBuffer,	//!< Texture buffer to store the color components of the window in
									normalBuffer;	//!< Texture buffer to store the normal components of the window in
			bool					sizeChanged,	//!< Indicates that the window size has changed and onResize needs to be triggered after the next layout update
									layoutChanged,	//!< Indicates that the general window content has changed and requires a layout and render update into the respective color and normal buffers
									visualChanged,	//!< Indicates that the window should be repainted
									fixedPosition;	//!< Window is fixed to its current location and may neither be resized nor moved
			SizeChange				sizeChange;
			bool					toneBackground,	//!< Indicates that overly bright background colors behind this window should be toned down. True by default
									appearsFocused;	//!< Indicates that this window should always look focused, even when it is not. False by default

			const bool				isModal;		//!< True if this window does not allow events to pass further down.
			Time::Stamp				hidden;			//!< Time stamp when this window was hidden
				

			FunctionalEvent			onFocusGained,	//!< Triggered whenever this window gains the focus (also, if it has just become visible)
									onFocusLost,	//!< Triggered whenever this window loses the focus (also, if it has just been removed)
									onHide,			//!< Triggered shortly before a window (or menu) is removed from the window/menu stack
									onResize;		//!< Triggered whenever this window is resized (new size has been adapted when this event is fired, but more updates may follow)

			/**/					Window(bool modal, Layout*style=&commonStyle);	//!< Creates a new window using the specified style
			virtual					~Window()
									{}
				
			void					SetSize(float width, float height);				//!< Updates window dimensions
			void					SetWidth(float width);							//!< Updates window width
			void					SetHeight(float height);						//!< Updates window height
			void					Drag(const M::TVec2<float>&d);								//!< Causes the window to move by the specified delta vector. The specified vector is modifable to allow the method to reduce it if dragging is not possible
			void					DragResize(M::TVec2<float>&d,ClickResult::value_t resolved);	//!< Causes the window to be resized by the specified delta vector.  The specified vector is modifable to allow the method to reduce it if resizing is not possible
			void					SignalMouseUp(float x, float y);						//!< Signals that the mouse has been released
			ClickResult::value_t	Resolve(float x_, float y_, float&inner_x, float&inner_y);	//!< Resolves how a click would be handled given the current mouse position @param x_ Mouse x coordinate @param y_ Mouse y coordinate @param inner_x X coordinate of the resulting window relative mouse position @param inner_y Y coordinate of the resulting window relative mouse position @return Resolve result
			void					RenderBuffers(Display<OpenGL>&display);							//!< Rerenders the window content into the buffers
			#ifdef DEEP_GUI
				void					setShellDestination(float new_shell_radius);				//!< Changes the window animation target
			#endif
		
			float					GetMinHeight()	const;								//!< Resolves the minimum height of this window in pixels
			float					GetMinWidth()	const;								//!< Resolves the minimum width of this window in pixels
			void					UpdateLayout();										//!< Updates the layout and component layout if existing
			void					SetComponent(const PComponent&component);					//!< Changes the primary component of this window @param discardable Set true if the assigned component may be erased when the window is deleted or another component assigned
			void					Apply(Component::eEventResult rs);	//!< Applies the result of a component event to the local state variables
			
			bool					Hide();	//!< Attempts to remove the local window from its operator

			void					SetLayout(Layout*layout);	//!< Changes the active window layout @param layout New layout. May be NULL.
			Layout*					GetLayout() const {return layout;}

			const TCellLayout&		GetCellLayout()	const {return cellLayout;}
			/**
			@brief Creates a new window without inserting it
			@param config Initial window configuration
			@param component Root component of the new window
			*/
			static PWindow			CreateNew(const NewWindowConfig&config, const PComponent&component=PComponent());
			/**
			@brief Creates a new window with a custom layout without inserting it
			@param config Initial window configuration
			@param layout Custom layout to use for this window. May be NULL
			@param component Root component of the new window
			*/
			static PWindow			CreateNew(const NewWindowConfig&config, Layout*layout,const PComponent&component);
		};
		
		
		enum mode_t
		{
			Planar,
			Cylindrical
		};



		class Operator: public std::enable_shared_from_this<Operator>	//! GUI Instance
		{
		public:
			ColorRenderer				colorRenderer;
			NormalRenderer				normalRenderer;
		protected:
			bool						owns_mouse_down;		//!< True if the currently pressed mouse button is handled by the window system
			Display<OpenGL>				*display;
			const Mouse					*mouse;
			InputMap					*input;
			mode_t						mode;
			bool						created,
										stack_changed;
			Buffer<PWindow>				windowStack;
			Buffer<std::weak_ptr<Window> >	menu_stack;
			OrthographicAspect<>		window_space;
			Camera<>					projected_space;
			GLuint						layer_texture;


			void						cylindricVertex(float px, float py, float tx, float ty, float r=1)	const;
			void						cylindricVertex(Window*window, float x, float y)	const;
			void						planarVertex(float px, float py, float tx, float ty) const;
			void						planarVertex(Window*window, float x, float y)	const;


			/**
			@brief Projects a point of a window to the screen
			@param window Window to use as reference
			@param x Window relative x coordinate (-1 to 1)
			@param y Window relative y coordinate (-1 to 1)
			*/
			void						project(Window*window, float x, float y, M::TVec2<float>&p)	const;
			void						unproject(const M::TVec3<float>&f, M::TVec2<float>&p) const;	//f required to be normalized

			void						unprojectMouse(M::TVec2<float>&p)	const;
			void						render(const PWindow&window, float w, float h, bool is_menu);//!< Renders this window given the current display dimensions @param w Display width @param h Display Height @param is_menu True if this window is rendered as a menu, false otherwise
			void						renderBox(const PWindow&window, float w, float h, bool is_menu);
			float						radiusOf(index_t stack_layer)	const;

			void						bind(Key::Name key);

			/**/						Operator(Display<OpenGL>&display, const Mouse&mouse, InputMap&input, mode_t mode);

		public:
			virtual						~Operator()	{ }
		
			static POperator			create(Display<OpenGL>&display, const Mouse&mouse, InputMap&input, mode_t mode=Cylindrical);
			inline static POperator		Create(Display<OpenGL>&display, const Mouse&mouse, InputMap&input, mode_t mode=Cylindrical)	{return create(display,mouse,input,mode);}
			Display<OpenGL>&			getDisplay()	{return *display;}
			inline Display<OpenGL>&		GetDisplay()	{return *display;}
			void						render();					//!< Animates and renders all windows. Also triggers certain mouse movement related component events
			inline void					Render()	{render();}
			bool						ownsMouseDown()	const	{return owns_mouse_down;}
			inline bool					OwnsMouseDown()	const	{return owns_mouse_down;}
			void						updateDisplaySize();	//!< Updates local texture sizes based on display size
			inline void					UpdateDisplaySize()	{updateDisplaySize();}
			bool						showingModalWindows()	const;
			inline bool					ShowingModalWindows()	const	{return showingModalWindows();}
			PWindow						getTopWindow() const;	//!< Retrieves the top-most window
			inline PWindow				GetTopWindow() const	{return getTopWindow();}
			void						ShowWindow(const PWindow&window);	//!< Appends a window to the local window stack as new top level window. If the window is already inserted then it will simply be moved to the top position @param window Window to append @param managed Set true to also Add the window to the local container, automatically deleting it if no longer necessary
			bool						HideWindow(const PWindow&window);				//!< Removes a window from the window stack (does not delete the window)
			bool						windowIsVisible(const PWindow&window)	const;
			inline bool					WindowIsVisible(const PWindow&window)	const	{return windowIsVisible(window);}
			/**
			@brief Creates a new window, and immediately shows it
			@param config Initial window configuration
			@param component Root component of the new window
			*/
			PWindow						ShowNewWindow(const NewWindowConfig&config, const PComponent&component=PComponent());
			/**
			@brief Creates a new window with a custom layout, and immediately shows it
			@param config Initial window configuration
			@param component Root component of the new window
			*/
			PWindow						ShowNewWindow(const NewWindowConfig&config, Layout*layout,const PComponent&component);



			bool						mouseDown();			//!< Signals that the main mouse button has been pressed.
			inline bool					SignalMouseDown()	{return mouseDown();}
			void						mouseUp();				//!< Signals that the main mouse button has been released
			inline void					SignalMouseUp()	{mouseUp();}
			bool						mouseWheel(short delta);	//!< Signals that the mouse wheel has been used
			inline bool					SignalMouseWheel(short delta)	{return mouseWheel(delta);}

			void						showMenu(const PWindow&menuWindow);
			inline void					ShowMenu(const PWindow&menuWindow)	{showMenu(menuWindow);}
			void						hideMenus();
			inline void					HideMenus()	{hideMenus();}

			PComponent					HoveringOver()	const;
			PComponent					GetComponentUnderMouse(bool*enabledOut=NULL)	const;
			PWindow						GetWindowUnderMouse()	const;
		};

		void							LoadBump(const PathString&filename, OpenGL::Texture&target);	//!< Loads a bump texture
		void							LoadBump(const PathString&filename, Image&target);			//!< Loads a bump texture as normals into an image
		void							LoadColor(const PathString&filename, OpenGL::Texture&target);	//!< Loads a color texture
		
		void							SetBlurEffect(bool setting);	//!< Updates the blur effect setting. If both blur and refract are disabled then texture copying is disabled altogether
		void							SetRefractEffect(bool setting);	//!< Updates the refraction effect setting. If both blur and refract are disabled then texture copying is disabled altogether
		void							SetBumpEffect(bool setting);	//!< Disables bump maps. This also disables refraction
		
	}



}



#endif

