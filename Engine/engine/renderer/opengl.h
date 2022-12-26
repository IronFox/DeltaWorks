#ifndef engine_renderer_openglH
#define engine_renderer_openglH



#undef GL_BEGIN
#define GL_BEGIN	//{log_file << __LINE__<<": begin "<<__func__<<nl;}
#undef GL_END
#define GL_END		//{log_file << __LINE__<<": end "<<__func__<<nl;}


#include "renderer.h"
#include "../../gl/gl.h"
#if SYSTEM_VARIANCE==LINUX
	#ifndef Status
	typedef int Status;
	#endif

	#include "../../gl/glx.h"
#endif
#include "../../gl/glu.h"
#include "../../gl/glext.h"
#include "../gl/extensions.h"
//#include "../../container/data_table.h"
#include "../../gl/gl_enhancement.h"
#include <io/log.h>
#include <string/string_buffer.h>
#include <math/resolution.h>

namespace Engine
{
	using namespace DeltaWorks;

	#define ERR_TABLE_ENTRY				10
	#define ERR_CONFIG_UNSUPPORTED		(-2)
	#define ERR_CONFIG_REJECTED			(-3)
	#define ERR_PFD_SET_FAILED			(-4)
	#define ERR_CONTEXT_CREATION_FAILED (-5)
	#define ERR_CONTEXT_BINDING_FAILED	(-6)
	#define ERR_BAD_IMAGE				(-7)
	#define ERR_GL_WINDOW_CREATION_FAILED	(-8)


	#undef T_
	#undef T2_
	#undef M_
	#define T_	template <class C>
	#define T2_	template <class C0, class C1>
	#define M_	template <unsigned Layers>

	
	class OpenGL;


	
	
	struct	TExtShaderConfiguration		//! Additional shader configuration
	{
			bool				requires_tangents;
	
	};
	
	class	ShaderSourceCode:public TExtShaderConfiguration	//! Defined shader source code includig configuration
	{
	public:
			String				code;
			count_t				num_samplers;
			bool				sky_lighting;

								ShaderSourceCode():num_samplers(0),sky_lighting(false)
								{}
	};

	
	namespace GL
	{
		//BYTE formatToChannels(GLenum format);
		GLenum formatToOrder(GLenum format);


		template <typename T>
			class Handle	//! Base OpenGL object providing one handle
			{
			public:
				typedef T		handle_t;
			protected:

				handle_t		handle;				//!< Handle to the referenced data

				/**/			Handle(handle_t h):handle(h) {}
			private:
				/**/			Handle(const Handle<T>&other):handle(0)	{FATAL__("Call to illegal operation");}	//private. never use
				Handle<T>&		operator=(const Handle<T>&)	/** private. derivative classes should never copy on assignment*/ { FATAL__("Call to illegal operation");return *this;};
				friend class	Engine::OpenGL;
			public:
				/**/			Handle():handle(0) {}
				virtual			~Handle()				{}
				virtual	void	clear()=0;// 					{handle=0;}		//!< Erases the local handle and sets it to 0 
				inline void		Clear()	{clear();}
				virtual	void	flush()						{handle=0;}		//!< Flushes (unsets but does not erase) the local handle and sets it to 0
				inline void		Flush()	{flush();}
				void			adoptData(Handle<T>&other)
								{
									clear();
									handle = other.handle;
									other.handle = 0;	//don't call flush() here, inheriting object may call this method before adopting its other content. flush() would drop that content
								}
				void			swap(Handle<T>&other)
								{
									swp(handle,other.handle);
								}
				friend void		swap(Handle<T>&a, Handle<T>&b)	{a.swap(b);}
				inline int		CompareTo(const Handle<T>&other) const
								{
									if (handle > other.handle)
										return 1;
									if (handle < other.handle)
										return -1;
									return 0;
								}
				inline bool		operator==(const Handle<T>&other) const
								{
									return handle == other.handle;
								}
				inline bool		operator!=(const Handle<T>&other) const
								{
									return handle != other.handle;
								}
				inline bool		operator<(const Handle<T>&other) const
								{
									return handle < other.handle;
								}
				inline bool		operator>(const Handle<T>&other) const
								{
									return handle > other.handle;
								}
				inline handle_t	getHandle() const	{return handle;}
				inline handle_t	GetHandle() const	{return handle;}
				/** @brief Determine whether or not the local object is empty. NULL-pointer sensitive @return true if the local object is NULL or the associated handle 0*/
				inline bool		IsEmpty()	const	{return !this || !handle;}
				/** Determine whether or not the local object is empty. NULL-pointer sensitive @return true if the local object is NULL or the associated handle 0*/
				inline bool		IsNotEmpty()	const	{return this!=NULL && handle!=0;}
								
				inline void		overrideSetHandle(handle_t h, bool do_clear=true)	//! Override method (use only to override the native OpenGL texture constructors). Sets the local handle. The method is kept explicit so that users are aware of this being rather unusual and potentially having undesired side-effects
								{
									if (do_clear)
										clear();
									handle = h;
								}
				inline void		OverrideSetHandle(handle_t h, bool doClear=true)	{overrideSetHandle(h,doClear);}
			};

		class	Query:public Handle<GLuint>	//! Query type. The Container<GLuint> handle is used for the query handle
		{
		protected:
			GLuint						geometry_handle;

			friend class Engine::OpenGL;
//			friend void	OpenGL::castQuery(const Query&);
			bool						createQuery();
				
		public:
			/**/						Query():geometry_handle(0)	{}
			virtual						~Query()	{if (!application_shutting_down) clear();}
			virtual	void				clear()	override;
			virtual	void				flush()	override;
			void						adoptData(Query&other);
			void						swap(Query&other);
			friend void					swap(Query&a, Query&b)	{a.swap(b);}
				
			bool						create();
			inline bool					Create()	{return create();}
			bool						createPoint(const float point[3]);
			inline bool					CreatePoint(const float point[3])	{return createPoint(point);}
			bool						createBox(const float lower[3],const float upper[3]);
			inline bool					CreateBox(const float lower[3],const float upper[3])	{return createBox(lower,upper);}
				
			bool						isValid()	const;	//!< Checks if the local query object is valid. A valid query object contains both a valid or 0 handle and a valid or 0 geometry handle
			inline bool					IsValid()	const	{return isValid();}
		};

		class Texture;
		class Shader;



		class Buffer;


		class Texture;

		template <typename Object, typename Inherit>
			class Reference:public Inherit		//! Generic reference type. Can be applied to most GL object types
			{
			public:
				typedef Reference<Object,Inherit>	ThisType;
				typedef typename Object::handle_t	handle_t;
			protected:
				handle_t				handle;				//!< Handle as retrieved from the target object. May be 0
				const Object			*target_object;		//!< Target object pointer. May be NULL

				friend class Engine::OpenGL;
			public:
				/**/					Reference(const Object*object):Inherit(*object),handle(object->getHandle()),target_object(object){}
				/**/					Reference(const Object&object):Inherit(object),handle(object.getHandle()),target_object(&object){}
				/**/					Reference():handle(0),target_object(NULL){}

				ThisType&				operator=(const Object&object)	{target(&object);return *this;}
				void					adoptData(Reference<Object,Inherit>&that)
										{
											this->Inherit::adoptData(that);
											this->handle = that.handle;
											this->target_object = that.target_object;
											that.handle = 0;
											that.target_object = NULL;
										}
				void					swap(Reference<Object,Inherit>&that)
										{
											this->Inherit::swap(that);
											swp(this->handle,that.handle);
											swp(this->target_object,that.target_object);
										}

				void					update()	//! Updates local data from the linked target object
										{
											if (target_object)
											{
												((Inherit&)*this) = *target_object;
												handle = target_object->getHandle();
											}
										}
				inline void				Update()	{update();}
				void					target(const Object*new_target)	//! Changes target to the specified pointer @param new_target New reference target. May be NULL
										{
											target_object = new_target;
											update();
										}
				inline void				SetTarget(const Object*newTarget)	{target(newTarget);}

				inline const Object*	target()	const	{return target_object;}
				inline const Object*	GetTarget()	const	{return target_object;}
				inline handle_t			getHandle() const	{return handle;}
				inline handle_t			GetHandle() const	{return handle;}
				/** Determine whether or not the local object is empty. NULL-pointer sensitive @return true if the local object is NULL or the associated handle 0*/
				inline bool				IsEmpty()	const	{return !this || !handle;}
				/** Determine whether or not the local object is empty. NULL-pointer sensitive @return true if the local object is NULL or the associated handle 0*/
				inline bool				IsNotEmpty()	const	{return this!=NULL && handle!=0;}
				/** reference self rather than target because self may be reference local where target is not. This operator exists for seamless interchangability of a pointer to the object and its reference*/
				inline ThisType*		operator->()	{return this;}
				inline const ThisType*	operator->() const	{return this;}
			};






		/**
			@brief OpenGL texture object
			
			Texture instances can hold textures of 1 to 4 channels, and read their content from either a pixel buffer object or host memory.
		*/
		class Texture:public Handle<GLuint>, public GenericTexture
		{
		protected:
			template <typename GLType>
				static	void						formatAt(BYTE channels, PixelType type, bool compress, GLenum&internal_format, GLenum&import_format);
			static	bool							configureFilter(GLenum target, TextureFilter filter, float anisotropy);
			virtual	void							overrideSetAttributes(UINT32 new_width, UINT32 new_height, BYTE new_channels, TextureDimension)	override {}	//no override allowed

		public:
			class Reference : public GL::Reference<Texture,GenericTexture>
			{
			public:
				typedef GL::Reference<Texture,GenericTexture>	Super;
			private:
				bool					readonly;
				/**/					Reference(const Texture*parent,bool readonly_):Super(parent),readonly(readonly_)	{};
				friend class Texture;
				template <typename data_t>
					void				update(const data_t*data,TextureFilter filter, float anisotropy);
			public:
				/**/					Reference():readonly(true)			{};
				/**/					Reference(Texture*parent):Super(parent),readonly(false)			{};
				/**/					Reference(const Texture*parent):Super(parent),readonly(true)			{};
				/**/					Reference(Texture&parent):Super(&parent),readonly(false)			{};
				/**/					Reference(const Texture&parent):Super(&parent),readonly(true)			{};
				template <typename data_t>
					void				RawUpdate(const data_t*data,TextureFilter filter, float anisotropy)	{update(data,filter,anisotropy);}

				template <typename Nature>
					void				update(const GenericImage<Nature>&image,TextureFilter filter=TextureFilter::Trilinear, float anisotropy=1.f)
										{
											if (image.GetWidth() != texture_width || image.GetHeight() != texture_height || image.contentType() != texture_type || texture_channels != image.channels())
											{
												throw Except::Renderer::TextureTransfer::ParameterFault(Except::globalString("Image properties are incompatible to the local texture properties"));
												return;
											}
											update(image.GetData(),filter,anisotropy);
										}
				template <typename Nature>
					inline void			Update(const GenericImage<Nature>&image,TextureFilter filter=TextureFilter::Trilinear, float anisotropy=1.f)
										{
											update(image,filter,anisotropy);
										}
			};
			


			/**/						Texture()	{};
			virtual						~Texture()	{if (!application_shutting_down) clear();}
			virtual	void				clear()	override;
			virtual	void				flush()	override;
								
				
			Reference					reference()	const		/** Creates a reference object to the local object*/ {return Reference(this,true);}
			Reference					reference()				/** Creates a reference object to the local object*/ {return Reference(this,false);}
			Reference					Refer()	const			/** Creates a reference object to the local object*/ {return Reference(this,true);}
			Reference					Refer()					/** Creates a reference object to the local object*/ {return Reference(this,false);}

			void						adoptData(Texture&other);
			void						swap(Texture&other);
			friend void	swap(Texture&a, Texture&b)	{a.swap(b);}
			/**
			@brief Resizes the local texture to provide the specified 2 dimensiona range of pixels and channels
									
			The method reuses the current texture handle if not 0.
			Resizing is performed only if the texture size has actually changed or the currently loaded texture is a cube map.
			Memory is allocated but not initialized. Automatically called by readFrom() so usually there's no need to call this method manually.
			@param width New image width in pixels
			@param height New image height in pixels
			@param channels New number of channels (1-4).
			*/
			bool						Resize(GLuint width, GLuint height, BYTE channels);
			bool						ExportTo(Image&target)	const;
			/**
			@brief Fills the allocated texture with the pixel content of the specified pixel buffer object
									
			Resize() is automatically invoked
			*/
			bool						load(const Buffer&object, GLuint width, GLuint height, BYTE channels);
			inline bool					Load(const Buffer&object, GLuint width, GLuint height, BYTE channels)	{return load(object,width,height,channels);}
			/**
			@brief Loads texture data from host memory to video memory for rendering

			The texture MIN filter will be set to GL_LINEAR_MIPMAP_LINEAR if @a mipmap is true,
			GL_NEAREST otherwise<br>
			Compilation will fail if the used data type is incompatible with OpenGL. All native types are supported.
			Channel interpretation will vary with the used type. For BYTES 0 is 0.0(black) and 255 is 1.0(white)
			@param data Binary data to load.
			@param width Width of the texture in pixels
			@param height Height of the texture in pixels
			@param channels Number of color channels of the specified texture
			@param type Pixel type to load
			@param compress Set true to compress this texture
			@param clamp_texcoords Initial texture clamp setting. Set true to automatically clamp texcoords to the range [0,1]
			@param filter Specify filtering. Linear only enables mag filter linear, Bilinear enables mag filtering and nearest mipmap layer filtering, Trilinear enables all linear axes. Choosing Bilinear or Trilinear requires the glGenerateMipmapEXT extension to be loaded
			@param anisotropy Max anisotropy value associated with this texture. Applied only if @a filter is either Bilinear or Trilinear
			*/
			template <typename data_t>
				void					loadPixels(const data_t*data, GLuint width, GLuint height, BYTE channels, PixelType type, float anisotropy=1.0f, bool clamp_texcoords=true, TextureFilter filter = TextureFilter::Trilinear, bool compress=false);
			template <typename data_t>
				void					LoadPixels(const data_t*data, GLuint width, GLuint height, BYTE channels, PixelType type, float anisotropy=1.0f, bool clampTexcoords=true, TextureFilter filter = TextureFilter::Trilinear, bool compress=false)
										{loadPixels(data,width,height,channels,type,anisotropy,clampTexcoords,filter,compress);}
				
			template <typename Nature>
				inline void				load(const GenericImage<Nature>&image, float anisotropy=1.0f, bool clamp_texcoords=true, TextureFilter filter=TextureFilter::Trilinear, bool compress=false)
										{
											loadPixels(image.GetData(), image.GetWidth(), image.GetHeight(), image.channels(), image.GetContentType(), anisotropy, clamp_texcoords, filter, compress);
										}
			template <typename Nature>
				inline void				Load(const GenericImage<Nature>&image, float anisotropy=1.0f, bool clamp_texcoords=true, TextureFilter filter=TextureFilter::Trilinear, bool compress=false)
										{
											loadPixels(image.GetData(), image.GetWidth(), image.GetHeight(), image.channels(), image.GetContentType(), anisotropy, clamp_texcoords, filter, compress);
										}
								
			/**
			@brief Loads cube texture content from host memory
			*/
			template <typename data_t>
				void					loadCube(const data_t*data0, const data_t*data1, const data_t*data2, const data_t*data3, const data_t*data4, const data_t*data5, GLuint width, GLuint height, BYTE channels, TextureFilter filter = TextureFilter::Trilinear, bool compress=false);
			template <typename data_t>
				inline void				LoadCube(const data_t*data0, const data_t*data1, const data_t*data2, const data_t*data3, const data_t*data4, const data_t*data5, GLuint width, GLuint height, BYTE channels, TextureFilter filter = TextureFilter::Trilinear, bool compress=false)
										{
											loadCube(data0,data1,data2,data3,data4,data5,width,height,channels,filter,compress);
										}
			template <typename Nature>
				void					loadCube(const GenericImage<Nature> faces[6], TextureFilter filter = TextureFilter::Trilinear, bool compress=false);
			template <typename Nature>
				inline void				LoadCube(const GenericImage<Nature> faces[6], TextureFilter filter = TextureFilter::Trilinear, bool compress=false)
										{
											loadCube(faces,filter,compress);
										}
			template <typename Nature>
				void					loadCube(const GenericImage<Nature>&face0, const GenericImage<Nature>&face1, const GenericImage<Nature>&face2, const GenericImage<Nature>&face3, const GenericImage<Nature>&face4, const GenericImage<Nature>&face5, TextureFilter filter = TextureFilter::Trilinear, bool compress=false);
			template <typename Nature>
				inline void				LoadCube(const GenericImage<Nature>&face0, const GenericImage<Nature>&face1, const GenericImage<Nature>&face2, const GenericImage<Nature>&face3, const GenericImage<Nature>&face4, const GenericImage<Nature>&face5, TextureFilter filter = TextureFilter::Trilinear, bool compress=false)
										{
											loadCube(face0,face1,face2,face3,face4,face5,filter,compress);
										}
				
			inline bool					isTransparent()	const
										{
											return handle && (texture_channels==4 || texture_channels==2);	//RGBA or INTENSITY+ALPHA
										}
			inline bool					IsTransparent()	const	{return isTransparent();}
			bool						isValid()	const;	//!< Checks if the local texture object is valid. A valid texture object contains a valid or 0 texture handle
			inline bool					IsValid()	const		{return isValid();}
			void						overrideSetHandle(GLuint h, UINT32 width, UINT32 height, BYTE num_channels, PixelType type, bool do_clear=true);	//!< Override method (use only to override the native OpenGL texture constructors). Sets the local handle. The method is kept explicit so that users are aware of this being rather unusual and potentially having undesired side-effects
			void						OverrideSetHandle(GLuint h, UINT32 width, UINT32 height, BYTE numChannels, PixelType type, bool doClear=true)
										{
											overrideSetHandle(h,width,height,numChannels,type,doClear);
										}
		};

		
		class	Shader:public Handle<GLShader::Template*>, public TExtShaderConfiguration
		{
		protected:
			friend class Engine::OpenGL;

			static bool								localShaderIsBound;
			static bool								_Install(const GLShader::Instance*instance);

		public:
			static GLShader::Template::VariableMap	globalMap;
			static GLShader::Template::UserConfig	globalUserConfig;
			static GLShader::Template::Configuration	globalConfig;
			static Texture::Reference				globalSkyTexture;	//!< Sky texture to use during shading. Leave empty to not use sky shading

			static const index_t					globalSkyLayer = 4;	//bit dangerous but let's see

			typedef	ShaderSourceCode				SourceCode;
			typedef GLShader::Instance				Instance;

		
													Shader()
													{
														requires_tangents = false;
													}
			virtual									~Shader()	{if (!application_shutting_down) clear();}
			virtual	void							flush()	override;
			virtual	void							clear()	override;
		
			Instance*								construct()	const
													{
														GLShader::Template::globalRenderConfig.ReDetect();
														return handle->BuildShader();
													}
			inline Instance*						Construct()	const	{return construct();}

			bool									create(const SourceCode&code,bool use_global_status=true);
			inline bool								Create(const SourceCode&code,bool useGlobalStatus=true)	{return create(code,useGlobalStatus);}
			String									Report()	const;
			static bool								compose(const MaterialComposition<Texture::Reference>&config, SourceCode&code_out);
			inline static bool						Compose(const MaterialComposition<Texture::Reference>&config, SourceCode&codeOut)	{return compose(config,codeOut);}
			inline bool								Install()	const;
			static void								Uninstall();
		};

		
		/**
			@brief Unified OpenGL array buffer object
			
			The object automatically discards any loaded OpenGL data.
			Not associated with the OpenGL renderer structure.
		*/
		class Buffer:public Handle<GLuint>
		{
		protected:
				size_t		data_size;		//!< Size of buffered data in bytes

		public:
							Buffer():data_size(0)
							{}
			virtual			~Buffer()	{if (!application_shutting_down) clear();}
			virtual	void	clear() override;
			virtual	void	flush() override;
				
		inline	size_t		size()	const			//!< Retrieves the current object size in bytes.
							{
								return data_size;
							}
				void		adoptData(Buffer&other);
				void		swap(Buffer&other);
				friend void	swap(Buffer&a, Buffer&b)	{a.swap(b);}
							/**
								@brief Resizes the buffer
								
								The method has no effect if the buffer's current size already matches the specified one.
								If the buffer is resized then its content is undefined as no data is actually transfered.
								
								@param size_ Size (in bytes) that the buffer should hold.
								@param type GL_ARRAY_BUFFER_ARB for floats (GLfloat), GL_ELEMENT_ARRAY_BUFFER_ARB for indices (GLuint)
							*/
				void		Resize(size_t size_, GLenum type);
							/**
								@brief Resizes and loads data into the local buffer object
								
								@param data Pointer to the first byte of the data to buffer
								@param type GL_ARRAY_BUFFER_ARB for floats (GLfloat), GL_ELEMENT_ARRAY_BUFFER_ARB for indices (GLuint)
								@param size_ Size (in bytes) of the data to buffer
							*/
				bool		loadData(const void*data, size_t size_, GLenum type);
							
							/**
								@brief Loads data into the local buffer object using the current buffer size
								
								@param data Pointer to the first byte of the data to buffer. Must be at least as long as the current object length
								@param type GL_ARRAY_BUFFER_ARB for floats (GLfloat), GL_ELEMENT_ARRAY_BUFFER_ARB for indices (GLuint)
							*/
				bool		loadData(const void*data, GLenum type);
				bool		isValid()	const;	//!< Checks if the local buffer object is valid. A valid buffer object contains a valid or 0 handle
		};
		
		template <typename T>
			class GeometryBuffer:public Buffer
			{
				typedef		GeometryBuffer<T>	Self;
			protected:
				using Buffer::loadData;
				using Buffer::Resize;

			public:
				inline	bool	load(const T*field, size_t num_units);	//!< Loads the specified data into the local buffer. @a field must be at least @a num_units units long but may be NULL if @a num_units is 0
				inline	bool	load(const Array<T>&field)	{return load(field.pointer(),field.GetLength());}	
				inline	bool	load(const Ctr::PreAllocVector<T>&field)	{return load(field.pointer(),field.GetLength());}

				void	adoptData(GeometryBuffer<T>&other)
				{
					Buffer::adoptData(other);
				}
				void	swap(Self&other)
				{
					Buffer::swap(other);
				}
				friend void		swap(Self&a, Self&b)	{a.swap(b);}

			};

		class SmartBuffer
		{
			typedef SmartBuffer	Self;
		protected:
			size_t				data_size;
			union
			{
				BYTE			*host_data;
				GLuint			device_handle;
			};
			bool				on_device;
		public:
			typedef GLuint		handle_t;

			



		private:
			/**/				SmartBuffer(const SmartBuffer&other):data_size(0),host_data(NULL),on_device(false)
								{
									FATAL__("Call to illegal operation");
								}	//private. never use
			/**/				SmartBuffer&	operator=(const SmartBuffer&)	//!< private. derivative classes should never copy on assignment
								{
									FATAL__("Call to illegal operation");
									return *this;
								};
			friend class		Engine::OpenGL;
		public:
			/**/				SmartBuffer():data_size(0),host_data(NULL),on_device(false)				{}
			virtual				~SmartBuffer()				{if (!application_shutting_down) clear();}
			virtual	void		clear();



			void				adoptData(SmartBuffer&other)
								{
									clear();
									data_size = other.data_size;
									host_data = other.host_data;
									on_device = other.on_device;
									other.data_size = 0;
									other.host_data = NULL;
									other.on_device = false;
								}
			void				swap(SmartBuffer&other)
								{
									swp(data_size,other.data_size);
									swp(host_data,other.host_data);
									swp(on_device,other.on_device);
								}
			friend void			swap(Self&a, Self&b)	{a.swap(b);}

			inline	int			CompareTo(const SmartBuffer&other) const
								{
									if (on_device && !other.on_device)
										return 1;
									if (!on_device && other.on_device)
										return -1;
									if (on_device)
									{
										if (device_handle > other.device_handle)
											return 1;
										if (device_handle < other.device_handle)
											return -1;
									}
									else
									{
										if (host_data > other.host_data)
											return 1;
										if (host_data < other.host_data)
											return -1;
									}
									return 0;
								}
			inline	bool		operator==(const SmartBuffer&other) const
			/**/				{
			/**/					return !operator!=(other);
			/**/				}
			inline	bool		operator!=(const SmartBuffer&other) const
			/**/				{
			/**/					return on_device != other.on_device || (on_device ? device_handle != other.device_handle : host_data != other.host_data);
			/**/				}
			inline	bool		operator<(const SmartBuffer&other) const
			/**/				{
			/**/					return CompareTo(other) < 0;
			/**/				}
			inline	bool		operator>(const SmartBuffer&other) const
			/**/				{
			/**/					return CompareTo(other) > 0;
			/**/				}


			inline	handle_t	getHandle() const {return getDeviceHandle();}

			inline	handle_t	getDeviceHandle() const {return on_device?device_handle:0;}
			inline	handle_t	GetDeviceHandle() const {return on_device?device_handle:0;}
			inline	const void*	getHostData() const		//! Retrieves a pointer to any internally stored host data, or NULL if no such is available. Also returns NULL if the buffered data resides in device space
								{
									return on_device?NULL:host_data;
								}
			inline	const void*	GetHostData() const		/** @copydoc getHostData() */	{return getHostData();}
			inline	bool		IsEmpty()	const	//! Determine whether or not the local object is empty. NULL-pointer sensitive @return true if the local object is NULL or the associated handle 0
								{
									return !this || !data_size;
								}
			inline	bool		IsNotEmpty()	const	//! Determine whether or not the local object is empty. NULL-pointer sensitive @return true if the local object is NULL or the associated handle 0
								{
									return this!=NULL && data_size>0;
								}

			inline	size_t		size()	const			//!< Retrieves the current object size in bytes.
								{
									return data_size;
								}

							
			/**
			@brief Resizes the buffer
								
			The method has no effect if the buffer's current size already matches the specified one.
			If the buffer is resized then its content is undefined as no data is actually transfered.
			The data resizing will affect device data if possible, host data otherwise
			@param size_ Size (in bytes) that the buffer should hold.
			@param type GL_ARRAY_BUFFER_ARB for floats (GLfloat), GL_ELEMENT_ARRAY_BUFFER_ARB for indices (GLuint)
			*/
			void				Resize(size_t size_, GLenum type);
			/**
			@brief Resizes and loads data into the local buffer object
								
			The data will be loaded into device space if possible, host space otherwise
			@param data Pointer to the first byte of the data to buffer
			@param type GL_ARRAY_BUFFER_ARB for floats (GLfloat), GL_ELEMENT_ARRAY_BUFFER_ARB for indices (GLuint)
			@param size_ Size (in bytes) of the data to buffer
			*/
			void				loadData(const void*data, size_t size_, GLenum type);
							
			/**
			@brief Loads data into the local buffer object using the current buffer size
								
			@param data Pointer to the first byte of the data to buffer. Must be at least as long as the current object length
			@param type GL_ARRAY_BUFFER_ARB for floats (GLfloat), GL_ELEMENT_ARRAY_BUFFER_ARB for indices (GLuint)
			*/
			void				streamData(const void*data, size_t size_, GLenum type);
			bool				isValid()		const;	//!< Checks if the local buffer object is valid. A valid buffer object contains a valid or 0 handle. If the local object links to host space, then the result is always true
			bool				isOnDevice()	const	{return on_device;}
		};
		
		template <typename T>
			class SmartGeometryBuffer:public SmartBuffer
			{
			protected:
				using SmartBuffer::loadData;
				using SmartBuffer::Resize;
				using SmartBuffer::streamData;

			public:



				struct Data
				{
					/**/			Data()	{}
					/**/			Data(const SmartGeometryBuffer<T>&)	{}
				
				};

				typedef GL::Reference<SmartGeometryBuffer,Data> Reference;

				Reference			Refer()	const			/** Creates a reference object to the local object*/ {return Reference(this);}



				inline	void		load(const T*field, count_t num_units);	//!< Loads the specified data into the local buffer. @a field must be at least @a num_units units long but may be NULL if @a num_units is 0
				inline	void		load(const ArrayData<T>&field)		{load(field.pointer(),field.GetLength());}	
				inline	void		load(const Ctr::BasicVector<T>&field)	{load(field.pointer(),field.GetLength());}
				template <typename StructType>
					inline	void	loadStructs(const StructType*field, count_t num_structs)	{load((const T*)field,num_structs * sizeof(StructType)/sizeof(T));}
				template <typename StructType>
					inline	void	loadStructs(const ArrayData<StructType>&field)	{loadStructs(field.pointer(),field.GetLength());}
				template <typename StructType>
					inline	void	loadStructs(const Ctr::BasicVector<StructType>&field)	{loadStructs(field.pointer(),field.GetLength());}
				inline	const T*	getHostData() const		{return static_cast<const T*>(SmartBuffer::getHostData());}//!< Retrieves a pointer to any internally stored host data, or NULL if no such is available. Also returns NULL if the buffered data resides in device space
				inline	count_t		countPrimitives()	const			//! Retrieves the number of primitives currently loaded into the buffer.
									{
										return data_size/sizeof(T);
									}
			};



		typedef SmartGeometryBuffer<GLfloat>	VBO;
		typedef SmartGeometryBuffer<GLuint>		IBO;

		class FBO:public Handle<GLuint>
		{
			typedef FBO				Self;
		protected:
			TFBOConfig	config;
			friend class Engine::OpenGL;
		public:
			struct BaseConfiguration
			{
				DepthStorage		depthStorage;
				BYTE				numColorTargets;
				const GLenum		*format;
				bool				filtered;

				/**/				BaseConfiguration(DepthStorage depthStorage, BYTE numColorTargets, const GLenum*format, bool filtered=true) 
									: depthStorage(depthStorage), numColorTargets(numColorTargets),
									format(format),filtered(filtered){}

			};
			struct Configuration : public BaseConfiguration
			{
				Resolution			resolution;

				/**/				Configuration(Resolution resolution, DepthStorage depthStorage, BYTE numColorTargets, const GLenum*format, bool filtered=true) 
									: resolution(resolution), BaseConfiguration(depthStorage,numColorTargets,format,filtered){}
				/**/				Configuration(Resolution resolution, BaseConfiguration config) 
									: resolution(resolution), BaseConfiguration(config){}
			};


			virtual					~FBO()	{if (!application_shutting_down) clear();}
			virtual	void			flush()	override;
			virtual	void			clear()	override;
			void					adoptData(FBO&other);
			void					swap(FBO&other);
			friend void				swap(Self&a, Self&b)	{a.swap(b);}
		
			void					Resize(const Resolution&res);
			inline bool				create(const Resolution&res, DepthStorage depthStorage, BYTE numColorTargets, const GLenum*format, bool filtered=true)	{return create(Configuration(res,depthStorage,numColorTargets,format,filtered));}
			inline bool				Create(const Resolution&res, DepthStorage depthStorage, BYTE numColorTargets, const GLenum*format, bool filtered=true)	{return create(Configuration(res,depthStorage,numColorTargets,format,filtered));}
			bool					create(const Configuration&config);
			inline bool				Create(const Configuration&config)	{return create(config);}
			inline bool				create(const Resolution&res, const BaseConfiguration&config)	{return create(Configuration(res,config));}
			inline bool				Create(const Resolution&res, const BaseConfiguration&config)	{return create(Configuration(res,config));}
			inline bool				primaryHasAlpha()	const	{return config.numColorTargets > 0 && Extension::FormatHasAlpha(config.colorTarget[0].textureFormat);}
			inline bool				PrimaryHasAlpha()	const	{return primaryHasAlpha();}
			//unsigned				getChannelsOfTarget(BYTE target) const;
			//inline unsigned			GetChannelsOfTarget(BYTE target) const {return getChannelsOfTarget(target);}
			inline const Resolution&size() const
									{
										return config.resolution;
									}
			inline UINT				GetWidth()	const	{return config.resolution.width;}
			inline UINT				Width()	const	{return config.resolution.width;}
			inline UINT				GetHeight()const	{return config.resolution.height;}
			inline UINT				Height()const	{return config.resolution.height;}
			inline const Resolution&getResolution()	const	{return config.resolution;}
			inline const Resolution&resolution()	const	{return config.resolution;}
			inline const Resolution&GetResolution()	const	{return config.resolution;}
			inline GLuint			getTextureHandle(BYTE target)	const
									{
										ASSERT_LESS__(target,config.numColorTargets);
										return config.colorTarget[target].textureHandle;
									}
			inline	GLuint			GetTextureHandle(BYTE target)	const	{return getTextureHandle(target);}
			inline	GLuint			getDepthTextureHandle()	const
									{
										return config.depthTarget.storageType == DepthStorage::Texture ? config.depthTarget.handle : 0;
									}
			inline	GLuint			GetDepthTextureHandle()	const	{return getDepthTextureHandle();}
			bool					exportColorTo(Image&out_image,BYTE target=0)	const;
			inline bool				ExportColorTo(Image&outImage,BYTE target=0)		const	{return exportColorTo(outImage,target);}
			bool					exportColorTo(FloatImage&out_image,BYTE target=0)	const;
			inline bool				ExportColorTo(FloatImage&outImage,BYTE target=0)	const	{return exportColorTo(outImage,target);}
			bool					exportColorTo(UnclampedFloatImage&out_image,BYTE target=0)	const;
			inline bool				ExportColorTo(UnclampedFloatImage&outImage,BYTE target=0)	const	{return exportColorTo(outImage,target);}
			Texture::Reference		reference(UINT target=0)	const;
			Texture::Reference		Refer(UINT target=0)		const	{return reference(target);}
			Texture::Reference		ReferDepth()	const;
			void					generateMIPLayers(UINT target=0);
			inline void				GenerateMIPLayers(UINT target=0)	{generateMIPLayers(target);}
			bool					isValid()	const;	//!< Checks if the local frame buffer object is valid. A valid frame buffer object contains a valid or 0 handle
			inline bool				IsValid()	const	/**@copydoc isValid()*/	{return isValid();}
			const TFBOConfig&		GetConfig() const {return config;}
		};





		struct RenderSetup
		{
				unsigned					normal_map_layer;			//!<effective normal map layer (UNSIGNED_UNDEF for disabled)
				count_t						bound_texcoord_layers,				//!<total number of bound texture-layers
											bound_texture_layers;
				bool						expect_tangent_normals,		//!< true if incoming normals are tangent space normals
											morphing,					//!< true if the bound vertex data is morphing
											multi_texture_blend,
											lighting_enabled,			//!< Lighting was enabled during last call
											changed;					//!< Render configuration changed since the last render call
				const MaterialConfiguration*material;					//!< Pointer to the last installed material. May be NULL or invalid out of rendering sequence
				Array<PLight>				enabled_light_field;		//!<link from the assigned light to its origin
				count_t						num_enabled_lights;			//!<number of linked (enabled) lights
				SystemType					matrix_type[32];			//!<type of the bound matrix (MT_IDENTITY by default)
				TextureDimension			texture_type[32];			//!<type of bound texture
				TextureMapping				texcoord_type[32];			//!<type of used texcoord-generator

									//dot3_bound;

											RenderSetup();
				void						updateSpecs(StringBuffer&target)	const;
		};	

		/**
			\brief OpenGL render state

			Structure holding the runtime render state
		*/
		struct RenderState
		{
				//count_t				texcoord_layers;				//!<number of bound texture-layers that have been affected by the active vertex binding
									//floats_per_vertex;				//!<expected number of floats per vertex.
				/*TVertexSection		vertex,					//!<bound vertex section
									normal,					//!<bound normal section
									color,						//!<bound color section
									tangent,					//!< bound tangent section
									uvw_section[32];			//!<map from the uvw-layer [0, uvw_layers) the section-offset and count is messured in float-units per vertex [0,...)*/
				//unsigned			uvw_target[32];			//!<index of the actual texture-layer that the uvw-coords are bound to

				bool				//index_object,				//!<true if an index-object has been bound
									matrix_changed,			//!<true if the matrix has been reloaded at least once while lighting was disabled
									in_query,					//!<currently querying object-visibility;
									single_texture_bound,		//!< true if useTexture was recently evoked with a non NULL parameter
									reconstruct_shader,		//!< true if active constellation changed
									upload_geometry,			//!< true if geometry should be uploaded the the gpu if possible
									geometry_lock;				//!< true if \a upload_geometry is currently readonly

				bool				indices_bound;				//!< True if indices have been bound
				const GLuint		*index_reference;			//!<pointer to bound index-data (required if index_object is set false)
				//GLenum				index_type;				//!<type of bound index-data (")
				//unsigned			index_size;				//!<size of the bound index-type in bytes
				RenderSetup			render_setup;
				Texture				normal_cube_texture;

									RenderState();
		};
		
				/**
			@brief Scoped object that assures any opengl context is available during the lifetime of this object
		*/
		class ContextLock
		{
		private:
				bool				is_bound;
		public:
									ContextLock();
									~ContextLock();
		};




		namespace V2
		{
			namespace Detail
			{
				typedef std::pair<GLuint, TextureDimension>	TTextureInfo;

				inline GLenum Translate(TextureDimension t)
				{
					switch (t)
					{
						case TextureDimension::Linear:
							return GL_TEXTURE_1D;
						case TextureDimension::Planar:
							return GL_TEXTURE_2D;
						case TextureDimension::Volume:
							return GL_TEXTURE_3D;
						case TextureDimension::Cube:
							return GL_TEXTURE_CUBE_MAP;
						default:
							return 0;
					}
				}
				TTextureInfo		Describe(const Texture&);
				TTextureInfo		Describe(const Texture*);
				TTextureInfo		Describe(const Texture::Reference&);
				TTextureInfo		Describe(const FBO&object);
				void				BindTexture(index_t layer, const TTextureInfo& handle);
				void				Configure(const TTextureInfo&, bool clamp);
			}
			template <typename T>
				inline void			BindTexture(index_t layer, const T&texture)	{Detail::BindTexture(layer,Detail::Describe(texture));}
				void				UnbindTexture(index_t layer);
			template <typename T>
				inline void			ConfigureTexture(T&texture, bool clamp)	{Detail::Configure(Detail::Describe(texture),clamp);}


			class TextureState
			{
			private:
				count_t		_texturesBound,
							_temporaryFallBackTo;

				inline void		_Done()	{glActiveTexture(GL_TEXTURE0);}
				void			_Reset();
				static inline void		_StreamBind(index_t layer, const Detail::TTextureInfo&info)
				{
					ASSERT__(info.first != 0);
					glActiveTexture((GLuint)(GL_TEXTURE0 + layer));
					glBindTexture(Detail::Translate(info.second),info.first);
				}
				template <typename T>
					void		_Bind(const T&texture)	{_StreamBind(_texturesBound++,Detail::Describe(texture));}
			public:
				/**/			TextureState():_texturesBound(0),_temporaryFallBackTo(0)	{}
				void			LockCurrentBinding();
				void			UnlockBinding();
				void			UnbindTextures()	{_Reset();}
				template <typename T>
					void		BindTextures(const T&texture)	{_Reset(); _Bind(texture); _Done();}
				template <typename T0, typename T1>
					void		BindTextures(const T0&t0, const T1&t1)	{_Reset(); _Bind(t0); _Bind(t1); _Done();}
				template <typename T0, typename T1, typename T2>
					void		BindTextures(const T0&t0, const T1&t1, const T2&t2)	{_Reset(); _Bind(t0); _Bind(t1); _Bind(t2); _Done();}
				template <typename T0, typename T1, typename T2, typename T3>
					void		BindTextures(const T0&t0, const T1&t1, const T2&t2, const T3&t3)	{_Reset(); _Bind(t0); _Bind(t1); _Bind(t2); _Bind(t3); _Done();}
				template <typename T0, typename T1, typename T2, typename T3, typename T4>
					void		BindTextures(const T0&t0, const T1&t1, const T2&t2, const T3&t3, const T4&t4)	{_Reset(); _Bind(t0); _Bind(t1); _Bind(t2); _Bind(t3); _Bind(t4); _Done();}
				template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
					void		BindTextures(const T0&t0, const T1&t1, const T2&t2, const T3&t3, const T4&t4, const T5&t5)	{_Reset(); _Bind(t0); _Bind(t1); _Bind(t2); _Bind(t3); _Bind(t4); _Bind(t5); _Done();}
				template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
					void		BindTextures(const T0&t0, const T1&t1, const T2&t2, const T3&t3, const T4&t4, const T5&t5, const T6&t6)	{_Reset(); _Bind(t0); _Bind(t1); _Bind(t2); _Bind(t3); _Bind(t4); _Bind(t5); _Bind(t6); _Done();}
				template <typename T>
					void		BindTextureArray(const T*field, count_t length)	{_Reset(); for (index_t i = 0; i < length; i++) _Bind(field[i]); _Done();}
			};
		}


	}

	namespace V2 = GL::V2;






	/**
		\brief Fog configuration
		
		Configuration structure for the fog environment.
	*/
	class Fog
	{
	public:
			float						nearRange, //!< Fog start (should be small compared to the scene)
										farRange,	//!< Fog end (should be large compared to the scene)
										density;	//!< Fog density
			M::TVec4<float>				color;	//!< Fog color - the 4th component should be 1.0f
			enum
			{
				Linear,					//!< Linear fog (commonly used)
				Exp,					//!< Exponential fog
				Exp2					//!< Exp2 fog
			}							type;		//!< Fog type
			
										Fog():nearRange(0.1),farRange(100),density(1.0),type(Linear)
										{
											M::Vec::set(color,1);
										}
										Fog(float range, float red=1, float green=1, float blue=1):nearRange(0.1),farRange(range),density(1.0),type(Linear)
										{
											M::Vec::def(color,red,green,blue,1);
										}
										Fog(float near_range_,float range, float red, float green, float blue):nearRange(near_range_),farRange(range),density(1.0),type(Linear)
										{
											M::Vec::def(color,red,green,blue,1);
										}
										Fog(float range, const M::TVec3<>&fog_color):nearRange(0.1),farRange(range),density(1.0),type(Linear)
										{
											color.rgb = fog_color;
											color.alpha = 1;
										}
										Fog(float near_range_, float range, const M::TVec3<>&fog_color):nearRange(near_range_),farRange(range),density(1.0),type(Linear)
										{
											color.rgb = fog_color;
											color.alpha = 1;
										}
	};





	/**
		\brief OpenGL Visual Interface

		OpenGL is responsible for binding OpenGL to a Window as well as providing a functional interface for the more common tasks.
	*/
	class OpenGL: public VisualInterface
	{
	public:
		#if SYSTEM==WINDOWS
			typedef		HGLRC			context_t;
		#elif SYSTEM==UNIX
			typedef		GLXContext		context_t;
		#endif
	
	private:
		friend class GL::ContextLock;

		//plattform specific gl-data:
		#if SYSTEM==WINDOWS
						typedef	HGLRC		GLContext;
						struct				GLBinding
						{
							GLContext		gl_context;
							HDC				device_context;
						};
						HWND				window;		//!< Handle of the active window
						HDC					device_context;		//!< Handle of the device context
		#elif SYSTEM==UNIX
						typedef	GLXContext	GLContext;
						struct				GLBinding
						{
							GLContext		gl_context;
							::Display		*display;			//!< Handle of the chosen display
							Window			wnd;				//!< Handle of the active window
						};
						XVisualInfo			*visual;			//!< Handle of the chosen visual
						::Display			*display;			//!< Handle of the chosen display
						Window				wnd;				//!< Handle of the active window
		#endif
						GLContext			gl_context,clone;			//!< Handle of the OpenGL context
		static			Ctr::PreAllocVector<GLBinding,4>	created_contexts;		//!< All created contexts

						bool				last_full_screen,			//!< specifies whether or not fullscreen was applied earlier
											verbose;
		static			int					error_code;				//!< error-code from the last operation
		static			LogFile				log_file;
		static			GL::RenderState		state;					//!< Active OpenGL render state

		SimpleGeometry					pivot,omni,spot,direct;
		bool							lights_defined = false;
		index_t							active_scene_index,		//!< Current light scenario index
										scene_index_counter;
		Ctr::IndexContainer<LightScene>	scenes;				//!< Light scenario container (index mapped)
		LightScene						*active_scene;		//!< Currently active light scenario
		bool							lighting_enabled;	//!< Lighting is currently enabled (some rendering processes behave differently when lighting is enabled)

						void				initGL();					//!< Applies the initial OpenGL configuration
		static			void 				generateNormalCubeSide(Image&target,  float ox, float oy, float oz,  float xx, float xy, float xz, float yx, float yy, float yz);

						String				sampleTexture(unsigned layer);	//!< Returns a string describing the sampling of the specified texture layer depending on the type of texture currently bound

		static			void				updatePosition(const Light*light);

		static			GLBinding			getCurrentContext();
		static			bool				hasCurrentContext();
		static			void				setCurrentContext(const GLBinding&);
						void				Bind(const VertexBinding&binding, const float*field);

		static	index_t					getIndexOf(const PLight&);
		static	void					setIndexOf(const PLight&,index_t);
		static	LightScene*				getLightSceneOf(const PLight&);
		static	bool					hasMoved(const PLight&);
		static	bool					wasModified(const PLight&);
		static	void					setHasMoved(const PLight&,bool);
		static	void					setWasModified(const PLight&,bool);


	protected:
		friend class Light;

		static			bool				TargetFBO(const GL::FBO&);
						void				TargetBackbuffer();
		inline			void				onModelviewChange();

		void				enableLight(const PLight&);
		void				disableLight(const PLight&);
		void				updateLight(const PLight&);
		void				updateLightPosition(const PLight&);




		/**
			@brief Installs a material for rendering
			
			@param config					Material color configuration and vertex binding to load
			@param list						Pointer to a sequence of textures. May be of type Texture::Reference or const pointers to Texture. May be NULL to indicate that no textures are available for this material
			@param shader					Shader instance to install. May be NULL, causing the deinstallation of any currently installed shader
			@param preserve_active_shader	Set true to leave the active shader installed
		 */
		template <typename TextureType>
			void					genericBindMaterial(const MaterialConfiguration&config, const TextureType*list, bool has_shader);


	public:
		static void					BuildMipMaps(const GL::FBO&, unsigned target, const GLShader::Instance&mipMapShader);
		static void					BuildMipMaps(const GL::Texture::Reference&, const GLShader::Instance&mipMapShader, GLenum format=GL_RGBA8);

		typedef GLfloat				FloatType;					//!< Float type best used for float fields to achive top render speed
		typedef GLuint				IndexType;					//!< Index type best used for index fields to achive top render speed

		typedef GL::VBO				VBO;
		typedef GL::IBO				IBO;
		typedef GL::FBO				FBO;
		typedef GL::Texture			Texture;
		typedef GL::Query			Query;			//!< Handle for pixel visibility queries
		typedef GL::Shader			Shader;			//!< Handle to a shader instance
		typedef MaterialComposition<Texture::Reference>
									Material;		//!< Material-Container
		typedef GL::ContextLock		ContextLock;

		static OpenGL				*globalInstance;		//!< Global default instance (set by Eve<OpenGL>)

		static bool					enableGeometryUpload();				//!< Globally enables geometry gmem upload (enabled by default) \return true if geometry upload is possible
		static void					disableGeometryUpload();			//!< Globally disables geometry gmem upload (enabled by default)
		static bool					queryGeometryUpload();				//!< Queries geometry upload state \return true if geometry is enabled and possible
		static void					lockGeometryUpload();				//!< Locks current geometry upload state. Any succeeding enable or disableGeometryUpload() calls will fail until unlockGeometryUpload is called
		static void					unlockGeometryUpload();				//!< Unlocks current geometry upload state.
		static bool					queryGeometryUploadLock();			//!< Queries whether or not changes to the geometry upload state are currently locked \return true if upload is currently locked
		//lighting: now OpenGL specific. Deprecated, but must be maintained for backwards compatibility (for now)
		void						enableLighting();					//!< Globally enables lighting.
		void						disableLighting();					//!< Globally disables lighting.
		bool						lightingEnabled()	const;			//!< Queries whether or not lighting is currently enabled
		PLight						createLight(bool enable=true);
		void						discardLight(const PLight&light);
		void						clearLights();
		void						resetLighting();
		bool						pickLightScene(index_t  scenario);		//!< Picks a light scenario as active scenario (this may be slow at times). \param scenario Index of an existing scenario that should be used from now on (0 = default scenario). \return true on success
		index_t						getLightScene()	const;
		index_t						createLightScene();						//!< Creates a new light scenario (collection of lights). \return Index of a new light scenario.
		void						discardLightScene(index_t scenario);	//!< Erases a given light scenario. \param scenario Index of an existing scenario that should be erased.
		bool						isLightScene(index_t scenario)	const;	//!< Queries if the specified light scenario exists. \param scenario Index of the light scenario that should be queried.
		void						getSceneLights(ArrayData<PLight>&array, bool enabled_only);	//!< Retrieves pointers to all lights in the active light scene
		void						RenderLights(const M::TVec3<>&cameraLocation);
						
		void						updateLighting(bool force=false);	//!< Completely refreshs the current lighting scenery
						

		static bool					queryBegin(bool depthTest=true);											//!< Set up query environment (disable rendering etc.) \param depthTest if set true then rendered pixels may be occluded by objects \return true if the query-environment could be set up
		static void					queryEnd();																//!< Ends querying and returns the original state.
		static void					castQuery(const Query&query);										//!< Renders the linked object. To extract the number of pixels on the screen call resolveObjectSize(const OpenGL::QueryObject&) \param query Handle of the respective query object
		static void					castPointQuery(const Query&query, const M::TVec3<>&point);				//!< Dynamically renders one point. To extract the number of pixels on the screen call resolveObjectSize(const OpenGL::QueryObject&) \param query Handle of the respective query object \param point Pointer to a location array in R
		static GLuint 				resolveQuery(const Query&query);										//!< Resolves the number of pixels drawn during the last queryBegin()/queryEnd() sequence at the time @a query was cast

		static inline void			cullNormal();		//!< Sets face cull mode to normal (back face culling)
		static inline void			cullInverse();		//!< Sets face cull mode to inverse (front face culling)
		static inline void			cullAll();			//!< Sets face cull mode to all (no faces will be rendered)
		static inline void			cullNone();		//!< Sets face cull mode to none (all faces will be rendered)
		static inline void			depthMask(bool mask);	//!< Sets depth mask
		static inline void			setDepthTest(eDepthTest depthTest);	//!< Sets depth test to use

		static inline void			setDefaultBlendFunc();	//!< Sets default alpha blend function
		static inline void			setDefaultBlendMode();	//!< Sets default alpha blend function
		static inline void			defaultBlendFunc();	//!< Sets default alpha blend function
		static inline void			defaultBlendMode();	//!< Sets default alpha blend function
		static inline void			setFlareBlendFunc();	//!< Sets additive alpha weighted blend function
		static inline void			setFlareBlendMode();	//!< @overload
		static inline void			flareBlendFunc();	//!< Sets additive alpha weighted blend function
		static inline void			flareBlendMode();	//!< @overload

		static inline void			setFog(const Fog&fog,bool enabled=true);							//!< Applies given fog configuration \param fog Fog configuration \param enabled Set true to enable fog
		static inline void			setFog(bool enabled);													//!< Dynamically enables or disables fog \param enabled Set true to enable fog
		static inline void			SetBackbufferClearColor(float red, float green, float blue, float alpha);	//!< Changes the active clear color \param red Red color component (0.0f - 1.0f) \param green Green color component (0.0f - 1.0f) \param blue Blue color component (0.0f - 1.0f) \param alpha Transparency color component (0.0f - 1.0f)
		static inline void			SetBackbufferClearColor(const M::TVec3<>&color, float alpha);						//!< Changes the active clear color \param color Pointer to a color vector in R \param alpha Transparency color component (0.0f - 1.0f)
		static inline void			SetBackbufferClearColor(const M::TVec4<>&color);									//!< Changes the active clear color \param color Pointer to a 4d color vector.

		bool						useTexture(const Texture&, bool clamp = false, bool override_safety = false);
		bool						useTexture(const Texture*, bool clamp = false, bool override_safety = false);
		bool						useTexture(const Texture::Reference&, bool clamp = false, bool override_safety = false);
		bool						useTexture(const FBO&object, bool clamp = false, bool override_safety = false);


		void						bindMaterial(const MaterialConfiguration&config, const Texture *const * list, const Shader&shader);
		void						bindMaterial(const MaterialConfiguration&config, const Texture::Reference*list, const Shader&shader);
		void						bindMaterial(const MaterialConfiguration&config, const Shader&shader)	{bindMaterial(config,(const Texture::Reference*)NULL,shader);}
		void						bindMaterial(const Material&material, const Shader&shader)				{bindMaterial(material,material.textures.pointer(),shader);}

		void						bindMaterial(const MaterialConfiguration&config, const Texture *const * list, const Shader::Instance*shader);
		void						bindMaterial(const MaterialConfiguration&config, const Texture::Reference*list, const Shader::Instance*shader);
		void						bindMaterial(const MaterialConfiguration&config, const Shader::Instance*shader)	{bindMaterial(config,(const Texture::Reference*)NULL,shader);}
		void						bindMaterial(const Material&material, const Shader::Instance*shader)				{bindMaterial(material,material.textures.pointer(),shader);}

		void						bindMaterialIgnoreShader(const MaterialConfiguration&config, const Texture *const * list);			//!< Similar to bindMaterial() . The method assumes that no shader will be installed, which is mostly fine as long as texcoods reflection is not enabled in the texture and performed by the shader (which may lead to undefined behavior)
		void						bindMaterialIgnoreShader(const MaterialConfiguration&config, const Texture::Reference*list);
		void						bindMaterialIgnoreShader(const MaterialConfiguration&config)	{bindMaterialIgnoreShader(config,(const Texture::Reference*)NULL);}
		void						bindMaterialIgnoreShader(const Material&material)				{bindMaterialIgnoreShader(material,material.textures.pointer());}

		static inline void			overrideEmission(const M::TVec4<>&emission_color);	//!< Overrides the emission color specified by the last bindMaterial() or bindMaterilIgnoreShader() call

		void						bindVertices(const VBO&vobj, const VertexBinding&binding);
		void						bindVertices(const VBO::Reference&vobj, const VertexBinding&binding);
		void						bindIndices(const IBO&iobj);
		void						unbindIndices();
		void						renderExplicit(GLuint type, index_t vertex_offset, GLsizei vertex_count);	//!< Render with explicit given configuration \param type OpenGL primitive type (GL_TRIANGLES, GL_QUADS, etc.) \param vertex_offset First vertex to render \param vertex_count Number of vertices to render
	T_	inline void						renderSub(const C system[4*4], unsigned vertex_offset, unsigned vertex_count);
	T_	inline void						renderQuadsSub(const C system[4*4], unsigned vertex_offset, unsigned vertex_count);
		inline void						render(unsigned vertex_offset, unsigned vertex_count);
		inline void						renderQuads(unsigned vertex_offset, unsigned vertex_count);
	T_	inline void						renderStrip(const C system[4*4], unsigned vertex_offset, unsigned vertex_count);
	T_	inline void						renderQuadStrip(const C system[4*4], unsigned vertex_offset, unsigned vertex_count);
		inline void						renderStrip(unsigned vertex_offset, unsigned vertex_count);
		inline void						renderQuadStrip(unsigned vertex_offset, unsigned vertex_count);
		inline void						renderVertexStrip(unsigned vertex_offset, unsigned vertex_count);
		void						render(const SimpleGeometry&structure);
		static void						face(const tTVertex&v0, const tTVertex&v1, const tTVertex&v2, const tTVertex&v3);		//!< Renders a quad using the four specified texture-vertices
		static void						face(const tTVertex&v0, const tTVertex&v1, const tTVertex&v2);							//!< Renders a triangle using the three specified texture-vertices
		static void						face(const tCTVertex&v0, const tCTVertex&v1, const tCTVertex&v2, const tCTVertex&v3);	//!< Renders a quad using the four specified color-texture-vertices
		static void						face(const tCTVertex&v0, const tCTVertex&v1, const tCTVertex&v2);						//!< Renders a triangle using the three specified color-texture-vertices
		static void						face(const tNTVertex&v0, const tNTVertex&v1, const tNTVertex&v2, const tNTVertex&v3);	//!< Renders a quad using the four specified normal-texture-vertices
		static void						face(const tNTVertex&v0, const tNTVertex&v1, const tNTVertex&v2);						//!< Renders a triangle using the three specified normal-texture-vertices
		static void						face(const tNVertex&v0, const tNVertex&v1, const tNVertex&v2, const tNVertex&v3);		//!< Renders a quad using the four specified normal-vertices
		static void						face(const tNVertex&v0, const tNVertex&v1, const tNVertex&v2);							//!< Renders a triangle using the three specified normal-vertices
		static void						face(const tCVertex&v0, const tCVertex&v1, const tCVertex&v2, const tCVertex&v3);		//!< Renders a quad using the four specified color-vertices
		static void					face(const tCVertex&v0, const tCVertex&v1, const tCVertex&v2);							//!< Renders a triangle using the three specified color-vertices
		static void					face(const TVertex&v0, const TVertex&v1, const TVertex&v2, const TVertex&v3);			//!< Renders a quad using the four specified vertices
		static void					face(const TVertex&v0, const TVertex&v1, const TVertex&v2);								//!< Renders a triangle using the three specified vertices
		static void					segment(const tCVertex&v0, const tCVertex&v1);												//!< Renders a line segment using the two specified color-vertices
		static void					segment(const TVertex&v0, const TVertex&v1);												//!< Renders a line segment using the two specified vertices
	T_	static inline void			enterSubSystem(const M::TMatrix4<C>&system);																	//!< Enters a sub-system affecting all following render-calls \param system Pointer to a 4x4 system matrix
		static inline void			exitSubSystem();																				//!< Returns from a sub-system to the next higher system
		void						unbindAll();																					//!< Unbinds the currently bound material, all textures and all vertex/index objects
	
		inline void					storeCamera();																					//!< Pushes the current modelview and projection matrices to the stack
		inline void					restoreCamera();																				//!< Restores the modelview and projection matrices from the stack, overwriting the currently loaded modelview and projection matrices
		void						SetCameraMatrices(const M::TMatrix4<>&view, const M::TMatrix4<>&projection, const M::TMatrix4<>&viewInvert);
		/**
		@brief Pops and restores the last pushed render state from the stack.
		*/
		void						RestoreRendererState();
		//void						StoreRendererState();	//implemented by VisualInterface
		void						SetDepthTest(bool depthTest, bool depthWrite);
		void						SetRasterizer(bool fill, bool cull);
		void						SetBlendMode(BlendMode);
		
		void						NextFrameNoClr();																				//!< Swaps buffers
		void						NextFrame();																					//!< Swaps buffers and clears the back buffer

		static String				renderState();																					//!< Queries the current render state \return String containing the current OpenGL render state


		/**/						OpenGL();

	#if SYSTEM==WINDOWS
		HWND						createWindow(const String&class_name, const String&window_name,int x, int y, unsigned width, unsigned height, bool hide_border, TVisualConfig&config);
		bool						CreateContext(HWND hWnd, TVisualConfig&config, const Resolution&res);
	#elif SYSTEM==UNIX
		virtual						~OpenGL();
		bool						CreateContext(::Display*connection, TVisualConfig&config, TWindowAttributes&out_attributes);
		bool						BindContext(Window window);
	#endif
		void						setVerbose(bool);
		void						initDefaultExtensions();
		bool						linkCallingThread();																			//!< Links OpenGL to the calling thread - necessary to perform multi threaded rendering @return true on success
		context_t					linkContextClone();				//!< Attempts to create a clone of the local context for multi-threaded processing and automatically binds it to the local thread
		void						unlinkAndDestroyContextClone(context_t);
		void						adoptCurrentContext();																			//!< Retrieves device and gl context from the current rendering environment overwriting and currently bound contexts
		
		static void					Capture(Image&target);																			//!< Copies the current visible image to the specified image
		static void					Capture(FloatImage&target);																			//!< Copies the current visible image to the specified image
		static void					Capture(Texture&target, unsigned width, unsigned height);										//!< Captures the current back buffer content to the specified texture
		static void					CaptureDepth(Texture&target, unsigned width, unsigned height);									//!< Captures the current back buffer depth content to the specified texture
		void						LinkContextClone (context_t);
		context_t					CreateContextClone();
		void						DestroyContext();																				//!< Destroys the active rendering context
		void						SetViewport(const RECT&region, const Resolution&windowRes);																//!< Sets a new rendering region	\param region Rectangle specifying the new rendering region in the active window
		static const char*			GetErrorStr();																					//!< Returns a string-representation of the last occured error \return String-representation of the last occured error
		static int					GetErrorCode();																				//!< Returns an index representation of the last occured error \return Error code of the last occured error
		const char*					GetName();																						//!< Returns identity \return "OpenGL"
		inline void					SignalWindowResize(const Resolution&)	{}
	#if SYSTEM==WINDOWS
						HDC							getDC();
						HGLRC						getGLContext();
	#elif SYSTEM==UNIX
						XVisualInfo*				getVisual();
						HDC							getDC()	{return 0;}
	#endif
	};

	#undef T_
	#undef T2_
	#undef M_



	/*! \fn	bool OpenGL::queryObject(QueryObject&target, const float lower_corner[3], const float upper_corner[3])
	 *	\brief Creates a new query object
	 *	\param target Target query object (will be reused if not empty)
	 *	\param lower_corner Pointer to an R vector defining the lower corner of the bounding box.
	 *	\param upper_corner Pointer to an R vector defining the upper corner of the bounding box.
	 *	\return true if the query object could be created, false otherwise.

	 Creates a query object from a bounding box. When testing the box all faces will be rendered without affecting any of the buffers.
	 If the resulting pixel count is above 10 then the specified object is considered visible.


	 *	\fn	QueryObject OpenGL::queryObject(const float lower_corner[3], const float upper_corner[3])
	 *	\brief Creates a new query object
	 *	\param lower_corner Pointer to an R vector defining the lower corner of the bounding box.
	 *	\param upper_corner Pointer to an R vector defining the upper corner of the bounding box.
	 *	\return Handle of a new query object (which will be empty if the query object could not be created)

	 Creates a query object from a bounding box. When testing the box all faces will be rendered without affecting any of the buffers.
	 If the resulting pixel count is above 10 then the specified object is considered visible.


	 *	\fn	bool	OpenGL::queryObject(QueryObject&target, const float point[3])
	 *	\brief Creates a new query object
	 *	\param target Target query object (will be reused if not empty)
	 *	\param point Pointer to an R vector defining the 3d position of the point to test.
	 *	\return true if the query object could be created, false otherwise.

	 Creates a query object from a point. When testing the point one single pixel will be rendered without affecting any of the buffers.
	 If the resulting pixel count is 1 then the specified pixel is visible.


	 *	\fn	QueryObject	OpenGL::queryObject(const float point[3])
	 *	\brief Creates a new query object
	 *	\param point Pointer to an R vector defining the 3d position of the point to test.
	 *	\return Handle of a new query object (which will be empty if the query object could not be created)

	 Creates a query object from a point. When testing the point one single pixel will be rendered without affecting any of the buffers.
	 If the resulting pixel count is 1 then the specified pixel is visible.


	 *	\fn	QueryObject	OpenGL::queryObject()
	 *	\brief Creates a new query object
	 *	\return Handle of a new query object (which will be empty if the query object could not be created)

	 Creates a query object from an empty object. This is for manually testing pixels for visibility.


	 *	\fn	PBufferObject	OpenGL::pbufferObject(unsigned width, unsigned height, ePBufferType type, BYTE fsaa)
	 *	\brief Creates a new pixel buffer object
	 *	\param width Width of the new pixel buffer object in pixels
	 *	\param height Height of the new pixel buffer object in pixels
	 *	\param type Pixel buffer type
	 *	\param fsaa Full scene anti-aliasing samples per pixel. Note that usually pixel buffers do not support fsaa.
	 *	\return Handle of a new pixel buffer object (which will be empty if the pbuffer object could not be created)

	 Creates a new pixel buffer object. Pixel buffer objects are still somewhat experimental and have not been tested under linux.
	 Binding is currently disabled.


	 *	\fn	bool	OpenGL::pbufferObject(PBufferObject&target,unsigned width, unsigned height, ePBufferType type, BYTE fsaa)
	 *	\brief Creates a new pixel buffer object
	 *	\param target Target pbuffer object (will be reused if not empty)
	 *	\param width Width of the new pixel buffer object in pixels
	 *	\param height Height of the new pixel buffer object in pixels
	 *	\param type Pixel buffer type
	 *	\param fsaa Full scene anti-aliasing samples per pixel. Note that usually pixel buffers do not support fsaa.
	 *	\return true if the pixel buffer object could be created, false otherwise

	 Creates a new pixel buffer object. Pixel buffer objects are still somewhat experimental and have not been tested under linux.
	 Binding is currently disabled.

	 *	\fn	bool	OpenGL::useTexture(const TextureObject&object, bool clamp = false)
	 *	\brief Binds the specifed texture object to OpenGL
	 *	\param object Handle of the respective texture object
	 *	\param clamp Set true to clamp texture-coordinates to [0,1]
	 *	\return true if the specified texture could be bound, false otherwise

	 This function simply binds the specified texture to the GL-context and will affect all following render commands.
	 Be sure not to use this function if a material is currently bound or the application will terminate.
	 Calling \a useTexture(TextureObject()) will unbind any bound texture.


	 *	\fn	void OpenGL::bindMaterial(Structure::MaterialInfo<C>*info,TextureObject*list,bool treat_empty_as_planar=false)
	 *	\brief Binds the specifed material object to OpenGL for subsequent render calls
	 *	\param info Pointer to a CVStructure material info structure
	 *	\param list Pointer to an array of texture objects (one for each layer specified in the info struct)
	 *	\param treat_empty_as_planar If set true then OpenGL will assign 2d-texture coordinates to empty texture layers. Not recommended.

	 This function binds a Structure::MaterialInfo<C> structure using a list of textures to OpenGL for subsequent rendering calls.
	 Note that the bound material specifies the number of floats that must be specified per vertex during later render calls.


	 *	\fn	void OpenGL::bindMaterial(const MaterialConfiguration&config, TextureObject*list)
	 *	\brief Binds the specifed material object to OpenGL for subsequent render calls
	 *	\param config Reference to an eve material configuration structure
	 *	\param list Pointer to an array of texture objects (one for each layer specified in the config struct)

	 This function binds an MaterialConfiguration structure using a list of textures to OpenGL for subsequent rendering calls.
	 Note that the bound material specifies the number of floats that must be specified per vertex during later render calls.


	 *	\fn	void OpenGL::bindMaterial(const MaterialObject&material)
	 *	\brief Binds the specifed material object to OpenGL for subsequent render calls
	 *	\param material Reference to an eve material object

	 This function binds a material object structure to OpenGL for subsequent rendering calls.
	 Note that the bound material specifies the number of floats that must be specified per vertex during later render calls.


	 *	\fn	void OpenGL::bindVertices(C*vdata, const VertexBufferObject&vobj, unsigned band, const TCodeLocation&location)
	 *	\brief Binds vertex data for rendering
	 *	\param vdata Pointer to an array containing the vertex data in local memory
	 *	\param vobj Handle of a vertex buffer object containing the data in vram
	 *	\param band Number of float values per vertex for verification
	 *	\param location Provide CLOCATION as location parameter

	 A material must be bound before invoking this method.
	 Both local and remote vertex data must be specified to insure the binding.
	 If available the remote vertex field will be used. Otherwise the local field is used. Due to this double binding the client application
	 does not need to know whether or not the vertex buffer extension is available or the data has been uploaded at all.
	 The method throws an error if the specified \a band parameter is less than the bound material's constrains.


	 *	\fn	void OpenGL::bindIndices(C*idata, const IndexBufferObject&iobj)
	 *	\brief Binds index data for rendering
	 *	\param vdata Pointer to an array containing the index data in local memory
	 *	\param vobj Handle of an index buffer object containing the data in vram

	 A material must be bound before invoking this method.
	 Both local and remote index data must be specified to insure the binding.
	 If available the remote index field will be used. Otherwise the local field is used. Due to this double binding the client application
	 does not need to know whether or not the vertex buffer extension is available or the data has been uploaded at all.


	 *	\fn	void OpenGL::renderSub(C*system, unsigned vertex_offset, unsigned vertex_count)
	 *	\brief Renders a triangle portion of the bound vertex/index data in the specified sub-system
	 *	\param system Pointer to a 4x4 system matrix
	 *	\param vertex_offset First vertex to render with 0 indicating the first vertex in the field
	 *	\param vertex_count Number of vertices to render (3 needed per triangle)

	 The method enters the specified sub-system, renders the specified number of triangles and exits the system again.
	 Both vertex and index fields must be bound in order to use this method.


	 *	\fn	void OpenGL::render(unsigned vertex_offset, unsigned vertex_count)
	 *	\brief Renders a triangle portion of the bound vertex/index data
	 *	\param vertex_offset First vertex to render with 0 indicating the first vertex in the field
	 *	\param vertex_count Number of vertices to render (3 needed per triangle)

	 The method renders the specified number of triangles.
	 Both vertex and index fields must be bound in order to use this method.


	 *	\fn	void OpenGL::renderStrip(C*system, unsigned vertex_offset, unsigned vertex_count)
	 *	\brief Renders a triangle-strip in the specified sub-system
	 *	\param system Pointer to a 4x4 system matrix
	 *	\param vertex_offset First vertex to render with 0 indicating the first vertex in the field
	 *	\param vertex_count Number of vertices to render (3 needed for the first triangle, 1 for each following)

	 The method enters the specified sub-system, renders the specified triangle strip and exits the system again.
	 Both vertex and index fields must be bound in order to use this method.


	 *	\fn	void OpenGL::renderStrip(unsigned vertex_offset, unsigned vertex_count)
	 *	\brief Renders a triangle-strip
	 *	\param vertex_offset First vertex to render with 0 indicating the first vertex in the field
	 *	\param vertex_count Number of vertices to render (3 needed for the first triangle, 1 for each following)

	 The method renders the specified triangle strip.
	 Both vertex and index fields must be bound in order to use this method.


	 *	\fn	void OpenGL::renderVertexStrip(unsigned vertex_offset, unsigned vertex_count)
	 *	\brief Renders a triangles without use of the index field
	 *	\param vertex_offset First vertex to render with 0 indicating the first vertex in the field
	 *	\param vertex_count Number of vertices to render (3 needed per triangle)

	 The method renders the specified number of triangles.
	 A vertex field must be bound in order to use this method.


	 *	\fn	void OpenGL::render(const SimpleGeometry&structure)
	 *	\brief Renders the specified structure
	 *	\param structure Reference to the structure to render


	*/

}

#include "opengl.tpl.h"
#endif
