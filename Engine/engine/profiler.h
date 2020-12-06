#ifndef engine_profileH
#define engine_profileH

/******************************************************************

Tool helping to analyse certain frame-sections.
Fairly redundant if you can use profiling.

******************************************************************/


#include "timing.h"
#include "../geometry/simple_geometry.h"
#include "textout.h"
#include "aspect.h"
#include "renderer/renderer.h"

namespace Engine
{
	namespace Profiler
	{
		class Channel;
		class Data;
		
		class Group:public Array<Channel,Adopt>
		{
		protected:
				unsigned			res;
				friend class Profiler::Data;
				
				void				setResolution(unsigned res);
		public:
				M::TVec3<float>		color;
				String				name;
		
									Group();
				void				adoptData(Group&other);
				unsigned			resolution()	const
									{
										return res;
									}
				Channel&			channel(index_t index)
									{
										return Array<Channel,Adopt>::operator[](index);
									}
				const Channel&		channel(index_t index) const 
									{
										return Array<Channel,Adopt>::operator[](index);
									}
				count_t				channels()	const
									{
										return Count();
									}
				void				Set(const String name, float red, float green, float blue)
									{
										this->name = name;
										M::Vec::def(color,red,green,blue);
									}
		};
		
		
		class Data:public Array<Group,Adopt>
		{
		protected:
				unsigned			res;
		public:
				Channel&			openChannel(index_t major_channel, index_t minor_channel);
				Channel&			activate(index_t major_channel, index_t minor_channel);
				
									Data();
				//void				update();
				void				update(float weight);
				void				advance();
				unsigned			resolution()	const
									{
										return res;
									}
				void				setResolution(unsigned res);
				Group&				group(index_t index)
									{
										return Array<Group,Adopt>::operator[](index);
									}
				count_t				groups()	const
									{
										return Count();
									}
				count_t				countChannels()	const;
		};
		
		extern Data					time_data,
									fps_data;
	
		namespace Config
		{
			unsigned		channelResolution();
			void			setChannelResolution(unsigned resolution);
			void			setRecordInterval(float seconds);
			float			recordInterval();
		}
	
		Channel*			activeChannel();
		Channel*			openChannel(unsigned major_channel, unsigned minor_channel);
		Channel*			start(unsigned major_channel, unsigned minor_channel);		//!< Starts time measuring on the specified channel, automatically stopping any other active channel
		Channel*			start(Channel*channel);									//!< @overload
		Timer::Time			stop();	//!< Stops the currently active channel and returns the recorded time interval since the last start() invokation (if any)
		
		Channel*			replace(unsigned major_channel, unsigned minor_channel);	//!< Pushes the active channel to the channel stack and starts time measuring on the specified channel instead. Must be terminated using restore()
		Timer::Time			restore();	//!< Stops the currently active channel and returns the recorded time interval since the last replace() invokation. The channel active when replace() was invoked is restarted (if any)
		
		bool				update();				//!< Advances the profiler state by one frame. update() or update(float delta) must be called once each frame, ideally at the end of the frame. Uses internal means to determine the time passed since the last call  @return true if the profiler has advanced all open channels (typically once half a second has passed), false otherwise
		bool				update(float delta);	//!< Advances the profiler state by one frame using a specific frame delta (in seconds). update() or update(float delta) must be called once each frame, ideally at the end of the frame @param delta Time passed during the last frame (in seconds) @return true if the profiler has advanced all open channels (typically once half a second has passed), false otherwise
		
		void				renderOrthographic();
		void				renderPerspective();
		inline	void		renderPerspectiveGraphs()	{renderPerspective();}
		inline	void		render3dGraphs()	{renderPerspective();}
		
		
		const Timer::Time&	lastChange();

		static const float	density_profile_frame_length=0.001;

		class Channel
		{
		protected:
				friend Channel*		activate(Channel*);
				friend void			Config::setChannelResolution(unsigned resolution);
				friend bool			update(float);
				
				friend class Data;
				friend class Group;
				
				Timer::Time		started,
									accumulated,
									minimum,
									maximum,
									//weighted_accumulated,
									frame_accumulated;
				float				faccumulated,
									fminimum,
									fmaximum,
									fframe_accumulated;
				unsigned			counter;
				//float				weighted_counter;
				bool				first_advance;
				
				
				void				setResolution(unsigned resolution);
				void				advance();
				void				nextFrame(float weight);	//!< Advances the frame counter
				void				generateColor(index_t channel_index);
		public:
				Timer::Time			total;		//!< Total time accumulated by this channel
				M::TVec3<float>		color;
				String				name;
				Array<float,Primitive>
									history,	//!< History of average time consumptions
									minimum_history,	//!< History of minimum values of this channel
									maximum_history;	//!< History of maximum values of this channel
									
				//Array<float>		weighted_history;	//!< History of average time consumptions scaled by the length of the respective frame
				
									Channel();
				bool				isActive()	const;
				void				setColor(float r, float g, float b);
				void				adoptData(Channel&other);
				void				start();
				Timer::Time			stop();
				void				accumulatei(const Timer::Time&t);
				void				accumulate(float value);
				void				Set(const String name, float red, float green, float blue)
									{
										this->name = name;
										M::Vec::def(color,red,green,blue);
									}
		};

		
		/**
			Type of data to visualize on a graph
		*/
		enum eView
		{
			Detailed,		//!< Typical detailed view
			Simplified,
			Solid
		};
		


		/*abstract*/ class Graph
		{
		public:
				String			name;					//!< Graph name
				eView			view;					//!< View type
				Data			*data;					//!< Data to display
				SimpleGeometry	frame;
				bool			print_channel_names;	//!< Print channel names
				int				group,					//!< Group to explicitly show. Effective onliy if @a channel is >= 0.
								channel;				//!< Channel to explicitly show. Effective only if @a group is >= 0. If both group and channel are >= 0 then minimum and maximum are supposed to be visualized as well.
								
				
								//show_weighted;			//!< Show weighted time history data rather than uniform one
				
								Graph(Data*data_=&time_data):view(Detailed),data(data_),print_channel_names(true),group(-1),channel(-1)
								{}
		virtual					~Graph();
		
		virtual	void			Draw()=0;
		virtual	void			update()=0;
		virtual	void			install()=0;
		virtual	void			uninstall() {};	//<- this is a microsoft C++ bug. pure virtual should never lack an external
		};
		
		class OrthographicGraph:public Graph
		{
		public:
				M::TFloatRect		position;
				
								OrthographicGraph(Data*data_=&time_data):Graph(data_),position(0,0,1,1)
								{
									frame.select(SimpleGeometry::Lines);
									frame.color(1,1,1);
									frame.vertex(0,0);
									frame.vertex(1,0);
									frame.vertex(1,0);
									frame.vertex(1,1);
									frame.vertex(1,1);
									frame.vertex(0,1);
									frame.vertex(0,1);
									frame.vertex(0,0);
									frame.seal();
								}
		
		virtual	void			install();
		virtual	void			uninstall();
		};
		
		class C3dGraph:public Graph
		{
		public:
				float			width,
								height,
								depth;
				M::TMatrix4<>		system;

				Array<M::TVec3<> >text_location;
		
		static	M::TVec3<>			light,
								background;
								
								C3dGraph(Data*data_=&time_data):Graph(data_),width(1),height(1),depth(1)
								{
									system = M::Matrix<>::eye4;

									frame.select(SimpleGeometry::Lines);
									frame.color(1,1,1,0.5);
									frame.vertex(-1,0,-1);
									frame.vertex(-1,0,-0.8);
									frame.vertex(-1,0,-1);
									frame.vertex(-0.8,0,-1);
									frame.vertex(-1,0,-1);
									frame.vertex(-1,0.1,-1);
									
									frame.vertex(1,0,-1);
									frame.vertex(1,0,-0.8);
									frame.vertex(1,0,-1);
									frame.vertex(0.8,0,-1);
									frame.vertex(1,0,-1);
									frame.vertex(1,0.1,-1);
									
									frame.vertex(1,0,1);
									frame.vertex(1,0,0.8);
									frame.vertex(1,0,1);
									frame.vertex(0.8,0,1);
									frame.vertex(1,0,1);
									frame.vertex(1,0.1,1);
									
									frame.vertex(-1,0,1);
									frame.vertex(-1,0,0.8);
									frame.vertex(-1,0,1);
									frame.vertex(-0.8,0,1);
									frame.vertex(-1,0,1);
									frame.vertex(-1,0.1,1);
									
									
									frame.vertex(-1,1,-1);
									frame.vertex(-1,1,-0.8);
									frame.vertex(-1,1,-1);
									frame.vertex(-0.8,1,-1);
									frame.vertex(-1,1,-1);
									frame.vertex(-1,0.8,-1);
									
									frame.vertex(1,1,-1);
									frame.vertex(1,1,-0.8);
									frame.vertex(1,1,-1);
									frame.vertex(0.8,1,-1);
									frame.vertex(1,1,-1);
									frame.vertex(1,0.8,-1);
									
									frame.vertex(1,1,1);
									frame.vertex(1,1,0.8);
									frame.vertex(1,1,1);
									frame.vertex(0.8,1,1);
									frame.vertex(1,1,1);
									frame.vertex(1,0.8,1);
									
									frame.vertex(-1,1,1);
									frame.vertex(-1,1,0.8);
									frame.vertex(-1,1,1);
									frame.vertex(-0.8,1,1);
									frame.vertex(-1,1,1);
									frame.vertex(-1,0.8,1);

									frame.seal();

								}
		
		virtual	void			install();
		virtual	void			uninstall();
			template <class Font>
				void			printCaptions();
		};
		
		
		template <class Font>
			/*static*/ class TextoutManager
			{
			protected:
			static	Engine::Textout<Font>		local,*active;
			public:
			static	Engine::Textout<Font>*		get();
			static	void						Set(Engine::Textout<Font>*);
			};
		
		template <class GL, class Base>
			class RenderGraph:public Base
			{
			private:
					GL									*renderer;
			protected:
					typename GL::VBO					vbo;
					typename GL::IBO					ibo;
					Ctr::Buffer<typename GL::FloatType>	vertices;
					Ctr::Buffer<typename GL::IndexType>	indices;
					Array<float,Primitive>				h_field,
														h_block;
					MaterialConfig						material;
					VertexBinding						binding;
			static	SimpleGeometry						background;
			static	bool								geometry_defined;
					SimpleGeometry						outlines,frame;
			
			public:
					const unsigned						dimensions;
					bool								opaque;
					float								fade_intensity;
				
														RenderGraph(Data*data_,unsigned dimensions_):Base(data_),material(0U),renderer(NULL),dimensions(dimensions_),opaque(dimensions_==3),fade_intensity(1)
														{
															binding.vertex.Set(0,dimensions);
															binding.color.Set(dimensions,4);
															binding.normal.Unset();
															binding.floats_per_vertex = 4+dimensions;
														}
					
					void								setRenderer(GL*renderer_)
					{
						renderer = renderer_;
					}
					
					GL*									getRenderer();
			protected:
					void								init();
					void								renderGeometry();
					void								beginUpdate();
					void								shade();
					void								finishUpdate();
				template <class Font>
					void								printCaptions();

			};
		
		template <class GL, class Font>
			class ColumnGraph:public RenderGraph<GL,OrthographicGraph>
			{
			public:
					typedef RenderGraph<GL,OrthographicGraph>	Base;
					using Base::h_field;
					using Base::h_block;
					using Base::view;
					using Base::name;
					using Base::data;
					using Base::channel;
					using Base::group;
					using Base::print_channel_names;
					using Base::position;
					using Base::frame;
					using Base::show_weighted;
			
			protected:
					float								highest;
					String								str_current;

					
					
					void								addRow(const M::TVec3<>&color, unsigned res);
			public:
					bool								caption_left;
					
														ColumnGraph(Data*data_=&time_data);
					void								Draw();
					void								update();
			};
		
		template <class GL, class Font>
			class StackedGraph:public RenderGraph<GL,OrthographicGraph>
			{
			public:
					typedef RenderGraph<GL,OrthographicGraph>	Base;

					using Base::h_field;
					using Base::h_block;
					using Base::view;
					using Base::name;
					using Base::data;
					using Base::print_channel_names;
					using Base::position;
					using Base::frame;
					//using Base::show_weighted;
					using Base::channel;
					using Base::group;
			
			protected:
					float								scale,
														highest;
					String								str_current;

					
					
					void								addLineRow(const M::TVec3<>&color);
					void								addMaxRow(const M::TVec3<>&color);
					void								addRow(const M::TVec3<>&color);
			public:
					bool								caption_left;
					
														StackedGraph(Data*data_=&time_data);
					void								Draw();
					void								update();
			};
			
		template <class GL, class Font>
			class StairGraph:public RenderGraph<GL,C3dGraph>
			{
			public:
					typedef RenderGraph<GL,C3dGraph>	Base;

					using Base::h_field;
					using Base::h_block;
					using Base::view;
					using Base::name;
					using Base::data;
					using Base::width;
					using Base::height;
					using Base::depth;
					using Base::frame;
					using Base::print_channel_names;
					//using Base::show_weighted;
					using Base::channel;
					using Base::group;
			protected:
					
					float								highest;
					String								str_current;
					
					
					void								addRow(const M::TVec3<>&color, float depth);
			public:
					
														StairGraph(Data*data_=&time_data);
					void								Draw();
					void								update();
			};
			
		template <class GL, class Font>
			class HeightGraph:public RenderGraph<GL,C3dGraph>
			{
			public:
					typedef RenderGraph<GL,C3dGraph>	Base;

					using Base::view;
					using Base::name;
					using Base::data;
					using Base::width;
					using Base::height;
					using Base::depth;
					using Base::frame;
					using Base::print_channel_names;
					//using Base::show_weighted;
					using Base::channel;
					using Base::group;
			protected:
					float								highest;
					String								str_current;
					
					
			public:
														HeightGraph(Data*data_=&time_data);
					void								Draw();
					void								update();
			};
			
			
			extern Ctr::Vector0<Graph*>					orthographic_graphs, perspective_graphs;
	}


}

#include "profiler.tpl.h"

#endif
