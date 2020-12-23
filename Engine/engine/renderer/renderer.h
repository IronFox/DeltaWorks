#ifndef engine_rendererH
#define engine_rendererH


/********************************************************************

Universal visual language-interface and rendering related definitions

********************************************************************/

#include <global_root.h>
#include <math/matrix.h>
#include <image/image.h>
#include "../../geometry/simple_geometry.h"
#include <geometry/cgs.h>
#include "../enumerations.h"
#include <general/system.h>
#include <general/thread_system.h>
#include <container/hashtable.h>
#include "../objects/texture.h"
#include <interface/hashable.h>
#include <general/orthographic_comparator.h>
#include <general/enumeration.h>

#if SYSTEM==UNIX

	#include <X11/Xlib.h>
#endif

#include <general/undef.h>

namespace Engine
{
	using namespace DeltaWorks;

	//errors
	#define		ERR_NO_ERROR		0
	#define		ERR_RETRY			1
	#define		ERR_GENERAL_FAULT	(-1)


	#define DO_VERIFY_VERTEX_BINDING
	//#define DO_VERIFY_BUFFERED_DATA


	/**
	Renderer pixel blend mode
	*/
	CONSTRUCT_ENUMERATION4(BlendMode,None,AlphaBlend,FlareBlend,Add);



	#if SYSTEM==UNIX

		/**
			\brief Window attributes
			
			Characterizes Window attributes
		*/

		struct TWindowAttributes
		{
				int							depth,
											screen_number;
				Visual						*visual;
		};

	#endif


	class VisualInterface;

	/**
	\brief Visual interface attributes
		
	Specifies attributes to be applied during the interface creation process
	*/

	struct TVisualConfig
	{
			BYTE						color_buffer_bits,			//!< Color buffer bits per pixel. Should be 16, 24 or 32
										depth_buffer_bits,			//!< Color buffer bits per pixel. Should be 16 or 24 (32 may cause trouble with FSAA)
										alpha_buffer_bits,			//!< Bits used to store transparency in the buffer. Should be 0, 4 or 8 (0 does not mean no transparency).
										accumulation_buffer_bits,	//!< Accumulation buffer bits per pixel. 0 by default.
										stencil_buffer_bits,		//!< Stencil buffer bits per pixel. Should be 0, 2, 4, or 8
										auxiliary_buffer_bits,		//!< Auxiliary buffer bits per pixel. 0 by default.
										fsaa_samples,				//!< Full screen anti aliasing samples that should be taken per pixel. (0, 2, 4, 6, 8, ...) Depending on local hardware.
										vertical_sync;				//!< 0 or 1. Set 1 to enable vertical synchronization (eats frames).
	};


	
	
	
	
	/**
		\brief Vertex float field section
		
		Struct used to specify the location of a section within the float field
	*/
	struct TVertexSection
	{
		UINT16						offset=0, //!< First float used for the active component with 0 indicating the first float
									length=0;	//!< Number of floats used for the active component. Must be 3 for normals or 0 if deactivated.

		inline						TVertexSection() {}
		/**/						TVertexSection(UINT16 offset, UINT16 length):offset(offset),length(length){}
		void						Set(UINT16 offset,UINT16 length){this->offset = offset;this->length = length;}
		void						Unset(){length = 0;}
		bool						IsSet() const{return length > 0;}
		bool						IsEmpty() const {return !length;}
		bool						IsNotEmpty() const {return IsSet();}

		char						CompareTo(const TVertexSection&)	const;

		bool						operator==(const TVertexSection&)	const;
		bool						operator!=(const TVertexSection&other)	const		{return !operator==(other);}
		bool						operator>(const TVertexSection&other)	const		{return CompareTo(other) > 0;}
		bool						operator<(const TVertexSection&other)	const		{return CompareTo(other) < 0;}

		friend hash_t				Hash(const TVertexSection&s)
									{
										return (hash_t(s.offset)<<16) | hash_t(s.length);
									}
		UINT16						requiredFloats()	const	//! Queries the minimum number of floats per vertex this section requires
									{
										return length>0?offset+length:0;
									}
	};

	struct TTexVertexSection : public TVertexSection
	{
		bool						generate_reflection_coords;			//!< Reflect texture coordinates. Most commonly only applied when mapping cubes. Effective only if @a TVertexSection::length is 0
			
									TTexVertexSection():generate_reflection_coords(false)
									{}

		friend hash_t				Hash(const TTexVertexSection&s)
									{
										return HashValue(Hash((const TVertexSection&)s)).AddGeneric(s.generate_reflection_coords);
									}
	};

	/**
		\brief Material layer configuration
		
		Structure holding basic material layer information
	*/
	class MaterialLayer:public IHashable
	{
	public:
		typedef PixelType				content_t;


		SystemType						system_type;			//!< Texcoord system type. Shaders may ignore this setting. Defaults to Identity
		content_t						content_type;			//!< Type of contained texel. Shaders may ignore this setting. Defaults to Color
			

		bool						clamp_x,			//!< Clamp texture coordinates along x axis. Disabled by default
									clamp_y,			//!< Clamp texture coordinates along y axis. Disabled by default
									clamp_z,			//!< Clamp texture coordinates along z axis. Disabled by default
									enabled;			//!< Layer is enabled. Other values are not reliable and may be filled with garbage if this value is set false. Defaults to true
		UINT32						combiner;			//!< GL combiner (usually GL_MODULATE or GL_DECAL). Somewhat deprecated.
		const M::TMatrix4<>			*custom_system;		//!< Remote texcoord 4x4 system matrix. Must be set non-NULL if system_type is SystemType::Custom. This pointer may be invalid if system_type is NOT SystemType::Custom.
			
									MaterialLayer()
									{
										resetToDefault();
									}

		void						resetToDefault();

		void						setClamp(bool do_clamp)
									{
										clamp_x = clamp_y = clamp_z = do_clamp;
									}
		bool						IsNormalMap() const
									{
										return content_type != PixelType::Color;
									}
		bool						isValid() const
									{
										return enabled
												&& system_type != SystemType::None
												&& content_type != PixelType::None
												&& (system_type != SystemType::Custom || custom_system != NULL);
									}
										
		char						CompareTo(const MaterialLayer&)		const;

		bool						operator==(const MaterialLayer&)		const;
		bool						operator!=(const MaterialLayer&other)	const		{return !operator==(other);}
		bool						operator>(const MaterialLayer&other)	const		{return CompareTo(other) > 0;}
		bool						operator<(const MaterialLayer&other)	const		{return CompareTo(other) < 0;}

		virtual	hash_t				ToHash()	const	override
									{
										return enabled? 
												(int)(HashValue(system_type)
													.AddGeneric(content_type)
													.AddGeneric( (UINT32(clamp_x)<<24) | (UINT32(clamp_y)<<16) | (UINT32(clamp_z)<<8)  /*| UINT32(reflect)*/)
													.AddGeneric(combiner)
													.Add(system_type == SystemType::Custom && custom_system != NULL ? StdMemHash(custom_system,16*sizeof(float)) : 0)
												)
												:
												0;
									}
	};


	/**
		@brief Color definition
	*/
	struct TColor:public M::TVec4<>
	{
		typedef M::TVec4<>	Super;

		void						Set(float intensity)	//! Sets the intensity of the local color. red, green, and blue are set to the specified value, alpha is set to 1.0
									{
										red = green = blue = intensity;
										alpha = 1.0f;
									}
		void						Set(float new_red, float new_green, float new_blue, float new_alpha = 1.0f)	//!< Updates the local color vector to the specified color values
									{
										red = new_red;
										green = new_green;
										blue = new_blue;
										alpha = new_alpha;
									}
		void						Set(const M::TVec3<float>&channel_values)	//!< Updates the local color vector. The alpha channel is set to 1.0
									{
										rgb = channel_values;
										alpha = 1.0f;
									}

		bool						operator==(const TColor&other) const
									{
										return M::Vec::similar(*this,other);
									}

		char						CompareTo(const TColor&other) const
									{
										return M::Vec::compare(*this,other);
									}

		bool						operator!=(const TColor&other)	const		{return !operator==(other);}
		bool						operator>(const TColor&other)	const		{return CompareTo(other) > 0;}
		bool						operator<(const TColor&other)	const		{return CompareTo(other) < 0;}


		TColor&						operator=(const M::TVec4<>&other)	{Super::operator=(other); return*this;}
	};

	/**
		@brief Material color configuration
	*/
	class MaterialColors : public IHashable
	{
	public:
		TColor						ambient,		//!< Ambient light intensity vector
									diffuse,		//!< Diffuse light intensity vector
									specular,		//!< Specular light intensity vector
									emission;		//!< Emission light intensity vector
		float						alpha_threshold;	//!< @deprecated{should implement this feature using shaders} Minimum alpha value for a fragment to be visible. Effective only if @a alpha_test is set true
		bool						alpha_test,			//!< @deprecated{should implement this feature using shaders}
									blend,				//!< Indicates that alpha blending should be used for this material (true by default)
									fully_reflective;	//!< Configures the shader to apply any reflection map everywhere, thus overriding any other material parameters (safe for normal mapping)
		BYTE						shininess_exponent;	//!< Shininess exponent in the range 1-128. The higher, the sharper the reflected cone will be. Defaults to 16

									MaterialColors();
		virtual						~MaterialColors()	{};
		virtual	hash_t				ToHash() const	override;

		void 						read(const CGS::MaterialInfo&info);	//!< Fills all local variables in accordance with the specified CGS material

		char						CompareTo(const MaterialColors&)	const;

		bool						operator==(const MaterialColors&)	const;
		bool						operator!=(const MaterialColors&other)	const		{return !operator==(other);}
		bool						operator>(const MaterialColors&other)	const		{return CompareTo(other) > 0;}
		bool						operator<(const MaterialColors&other)	const		{return CompareTo(other) < 0;}

		void						resetToDefault();
	};

	/**
		@brief Basic vertex binding

		Contains sections for vertex coordinates, normal, tangent, and color
	*/
	class VertexBinding	: public IHashable
	{
		typedef VertexBinding			Self;
	public:
		TVertexSection					vertex,				//!< Section of the float field used to specify the position of a vertex
										tangent,			//!< Section of the float field used to specify the tangent of a vertex. This field is required if the material features tangen space normal maps. OpenGL maps this section one past the last used texcoord field
										normal,				//!< Section of the float field used to specify the normal of a vertex
										color;				//!< Section of the float field used to specify the color of a vertex (if lighting is disabled)
		Array<TTexVertexSection>		texcoords;			//!< Texcoord binding
		UINT16							floats_per_vertex;	//!< Vertex size in elements. This value is used to calculate the vertex stride value and must include all active vertex sections

		/**/							VertexBinding(count_t layers=0);
		virtual							~VertexBinding()	{};
		virtual hash_t					ToHash() const override;

		void							swap(VertexBinding&other)
										{
											swp(vertex,other.vertex);
											swp(tangent,other.tangent);
											swp(normal,other.normal);
											swp(color,other.color);
											texcoords.swap(other.texcoords);
											swp(floats_per_vertex,other.floats_per_vertex);
										}
		friend void						swap(Self&a, Self&b)	{a.swap(b);}

		char							CompareTo(const VertexBinding&)	const;

		bool							operator==(const VertexBinding&)		const;
		bool							operator!=(const VertexBinding&other)	const	{return !operator==(other);}
		bool							operator>(const VertexBinding&other)	const	{return CompareTo(other) > 0;}
		bool							operator<(const VertexBinding&other)	const	{return CompareTo(other) < 0;}

		UINT16							minFloatsPerVertex()	const;	//!< Calculates the minimal number of floats per vertex that this binding requires. A valid vertex binding complies with minFloatsPerVertex() <= floats_per_vertex

		virtual	void 					read(const CGS::MaterialInfo&info, UINT32 flags);	//!< Fills all local variables in accordance with the specified CGS material, floats per coordinate and flags

		void							resetToDefault(bool erase_layers=true);		//!< Restores default layer binding @param erase_layers Set true to resize the number of texcoord layers to 0. If set false then the existing layers will be set to default values
	};

	typedef VertexBinding	VertexDefinition,VertexDeclaration;


	/**
		\brief Material configuration
		
		Structure holding material finding information
	*/


	class MaterialConfiguration:public MaterialColors
	{
		typedef MaterialConfiguration	Self;
	public:
		Array<MaterialLayer>		layers;
		String						name;
			
									MaterialConfiguration(count_t layers=0); //!< Constructs a new material \param layers Number of texture layers to use
									MaterialConfiguration(const CGS::MaterialInfo&, UINT32 flags);
									MaterialConfiguration(const CGS::MaterialInfo*, UINT32 flags);

		void						resetToDefault(bool colors=true, bool erase_layers=false); //!< Loads default values into the structure. \param colors If set true then default colors will be loaded. @param erase_layers If set true then the number of layers will be reduced to zero

		virtual	void				setLayers(count_t num_layers)	{layers.ResizePreserveContent(num_layers);/*texcoords.ResizePreserveContent(num_layers);*/};	//!< Changes the number of layers. Copies the content of the existing layer array. \param layers New number of layers.

		virtual	void 				read(const CGS::MaterialInfo&info, UINT32 flags);	//!< Fills all local variables in accordance with the specified CGS material, floats per coordinate and flags

		char						CompareTo(const MaterialConfiguration&)		const;

		bool						operator==(const MaterialConfiguration&)		const;
		bool						operator!=(const MaterialConfiguration&other)	const	{return !operator==(other);}
		bool						operator>(const MaterialConfiguration&other)	const	{return CompareTo(other) > 0;}
		bool						operator<(const MaterialConfiguration&other)	const	{return CompareTo(other) < 0;}

		void						adoptData(MaterialConfiguration&other)
									{
										MaterialColors::operator=(other);
										layers.adoptData(other.layers);
										name.adoptData(other.name);
									}
		void						swap(MaterialConfiguration&other)
									{
										swp(((MaterialColors&)*this),((MaterialColors&)other));
										layers.swap(other.layers);
										name.swap(other.name);
									}
		friend void					swap(Self&a, Self&b)	{a.swap(b);}
	

		virtual	hash_t				ToHash() const	override;

	//virtual	bool						isConsistent() const	{return VertexBinding::texcoords.Count() == layers.Count();}
	};



	typedef MaterialConfiguration	MaterialConfig;

	extern TVisualConfig				default_buffer_config;
	extern M::TMatrix4<float>				environment_matrix;
	extern bool							erase_unused_textures;


	namespace VisualEnum
	{
		
		enum eLayerFlags //! Material layer configuration flags
		{
			MirrorFlag			= CGS::MirrorFlag,	//!< Derived from CGS::MirrorFlag. Causes generated 3d texture coordinates to immitate the behavior of reflected light-rays. Effective only in combination with CubeMapFlag
			ClampXFlag			= CGS::ClampXFlag,	//!< Derived from CGS::ClampXFlag. Causes non-generated texture coordinates to be clamped to the range [0,1] along the X/U-axis
			ClampYFlag			= CGS::ClampYFlag,	//!< Derived from CGS::ClampYFlag. Causes non-generated texture coordinates to be clamped to the range [0,1] along the Y/V-axis
			ClampZFlag			= CGS::ClampZFlag,	//!< Derived from CGS::ClampZFlag. Causes non-generated texture coordinates to be clamped to the range [0,1] along the Z/W-axis
			CustomSystemFlag	= 0x0200,			//!< Specifies that a custom or the environment matrix should be loaded as texcoord transformation matrix of this layer
			EnvSystemFlag		= 0x0400,			//!< Specifies that the environment matrix should be loaded as texcoord transformation matrix of this layer. Effective only in combination with CustomSystemFlag
			CubeMapFlag			= 0x0800,			//!< Specifies that the texture bound to this layer should be a cube map. Required for MirrorFlag
			LayerDisabledFlag	= 0x1000,			//!< Specifies that this layer should not be rendered
			BlendLayerFlag		= 0x2000,			//!< Non implemented stub.
			NormalMapFlag		= 0x4000,			//!< Specifies that this layer's texture should be interpreted as a normal map
			TangentNormalFlag	= 0x8000			//!< Specifies that the normals provided by this layer should be interpreted as tangent space, rather than object space. Effective only in combination with @a NormalMapFlag
		};
		
	}



	/* ** objects **

	objects must:

		- auto-initialize to empty
		- be copyable via operator= and constructor
		- be comparable via operator== and operator!=
		- be comparable via operator< and operator>
		- NOT auto-delete on destruction (except local data)
		- be distinguishable so ambiguities cannot occur on any method other than discardObject(...) and IsEmpty(...)
		- NOT keep references to client-data
		- provide a const IsEmpty()-method to determine whether not not content is available.
		
	*/


	template <typename Texture>
		class MaterialComposition:public MaterialConfiguration //! Eve material container (including textures)
		{
			typedef MaterialComposition	Self;
		public:
			Array<Texture>		textures;


			inline	bool		operator==(const MaterialComposition<Texture>&other) const
								{
									return MaterialConfiguration::operator==(other) && textures == other.textures;
								}

			inline	char		CompareTo(const MaterialComposition<Texture>&other)	const
			{
				return OrthographicComparison(MaterialConfiguration::CompareTo(other)).AddComparison(textures,other.textures);
			}

			inline	bool		operator!=(const MaterialComposition<Texture>&other)	const	{return !operator==(other);}
			inline	bool		operator>(const MaterialComposition<Texture>&other)	const	{return CompareTo(other) > 0;}
			inline	bool		operator<(const MaterialComposition<Texture>&other)	const	{return CompareTo(other) < 0;}

						
			virtual	void		setLayers(count_t num_layers)
								{
									MaterialConfiguration::setLayers(num_layers);
									textures.ResizePreserveContent(num_layers);
								}

			virtual	void 		read(const CGS::MaterialInfo&info, UINT32 flags)
								{
									MaterialConfiguration::read(info,flags);
									textures.SetSize(MaterialConfiguration::layers.Count());
								}

			void				adoptData(MaterialComposition&other)
								{
									MaterialConfiguration::adoptData(other);
									textures.adoptData(other.textures);
								}
			void				swap(MaterialComposition&other)
								{
									MaterialConfiguration::swap(other);
									textures.swap(other.textures);
								}
			friend void			swap(Self&a, Self&b)	{a.swap(b);}

			virtual	hash_t		ToHash() const override
								{
									HashValue hash(MaterialConfiguration::ToHash());
									hash.Add(StdMemHash(textures.pointer(),textures.GetContentSize()));

									return hash;
								}

			virtual	bool		isConsistent() const	{return MaterialConfiguration::layers.Count() == textures.Count();}

		};
	


	

/**
	\brief Visual interface super class

	All interfaces are supposed to comply with the specifications in this file
	*/
	class VisualInterface
	{
	protected:
		struct TRendererState
		{
			BlendMode		blendMode;
			bool			fill,cull,depthTest,depthWrite;
		};

		TRendererState				current;
			
			
		void						PopStateStackToCurrent()
		{
			ASSERT__(stack.IsNotEmpty());
			current = stack.Pop();
		}
		
	public:
		/**
		@brief Pushes the current render state (depth-test,rasterizer,blend-mode) to the stack.
		*/
		void						StoreRendererState()	{ASSERT_LESS__(stack.Count(), 16); stack << current;}
		//void						RestoreRendererState();	//must be implemented by 

	private:
		Buffer0<TRendererState>	stack;

	};
}

#include "renderer.tpl.h"

#endif
