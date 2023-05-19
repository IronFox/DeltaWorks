#ifndef cgsH
#define cgsH

#include <set>
#include "../io/riff_handler.h"
#include "../io/byte_stream.h"
#include "../image/image.h"
#include "../io/texture_compression.h"
#include "../io/log.h"
#include "../math/object_def.h"
#include "../math/object.h"
#include "../container/hashtable.h"

#include "../string/int_name.h"
#include "../general/ref.h"

namespace DeltaWorks
{

	#define RIFF_ID		0x20204449
	#define RIFF_FACE	0x45434146

	#define CGS_FILE_VERSION	0x0102

	#define STRUCTURE_TEX_LAYERS	4

	#define EMPTY_TEXTURES_ARE_PLANAR	true	//set to false if empty textures should be ignored. otherwise they will be treated as planar


	const char* shortenFile(const char*full_file);


	//#define FUZZY_COMPARE		//may cause problems - assuming equal crc means equal data

	namespace CGS	//! Compiled Geometrical Structure
	{
		using namespace Container;
		using namespace Math;

		#define CGS_DECLARE_ADOPTING_DEF(_class_)\
			}}\
			DECLARE_T__(CGS::_class_,Adopting);\
			namespace DeltaWorks{namespace CGS\
			{

		#define CGS_DECLARE_ADOPTING(_class_)\
			}}\
			DECLARE__(CGS::_class_,Adopting);\
			namespace DeltaWorks{namespace CGS\
			{

		#define CGS_DECLARE_ADOPTING_T(_class_)\
			}}\
			DECLARE_T__(CGS::_class_,Adopting);\
			namespace DeltaWorks{namespace CGS\
			{
		#define CGS_DECLARE_ADOPTING_2(_class_,_type0_,_type1_)\
			}}\
			DECLARE_T2__(CGS::_class_,_type0_,_type1_,Adopting);\
			namespace DeltaWorks{namespace CGS\
			{


		enum eTargetTableBits
		{
			RelinkBit		= 0x1,

			ObjectBit		= 0x2	| RelinkBit,
			AnimatorBit		= 0x4,
			WheelBit		= 0x8	| RelinkBit,
			AcceleratorBit	= 0x10	| RelinkBit,

			Everything = 0xFFFFFFFF

		};

		enum eMaterialFlags	//! Attribute flags of materials
		{
			AlphaTestFlag	= 0x1,	//!< Material is masked (transparency tested). Masked material fragments are either fully opaque or invisible but never blended.
			MirrorFlag		= 0x2, 	//!< Used for cube textures. Implies that the generated texture coordinates immitate reflective appearance. ( MASK_FLAG and MIRROR_FLAG/CLAMP_FLAGs are of different domains but kept seperated for redundance purposes)
			ClampXFlag		= 0x4,	//!< The x texture coordinate of vertices on the respective texture layer is clamped to [0,1]
			ClampYFlag		= 0x8,	//!< The y texture coordinate of vertices on the respective texture layer is clamped to [0,1]
			ClampZFlag		= 0x10	//!< The z texture coordinate of vertices on the respective texture layer is clamped to [0,1]
		};
	
		enum eParticleFlags	//! Particle configuration flags
		{
			IgnoreParticleRotation	= 0x1,	//!< Ignore particle rotation (saved particles will not feature rotation parameters)
			IgnoreParticleTexcoords	= 0x2	//!< Ignore individual texture coordinates (saved particles will not feature individual texture coordinates)
		};
	
		enum eGeneralFlags	//! Generic non-persistent object/wheel/accelerator/construct flags
		{
			AnimationTargetFlag = 0x1		//!< Object is targeted by at least one animator
		};
	
	
		enum vertex_flags_t	//!< Flags applied to vertex collections
		{
			HasNormalFlag = 0x1,
			HasTangentFlag = 0x2,
			HasColorFlag = 0x4
		};


		template <typename T>
			inline void copy3(const T from[3], T to [3])
			{
				to[0] = from[0];
				to[1] = from[1];
				to[2] = from[2];
			}

		template <typename T>
			inline void copy4(const T from[4], T to [4])
			{
				to[0] = from[0];
				to[1] = from[1];
				to[2] = from[2];
				to[3] = from[3];
			}



		static const Riff::RIFF_INDEX_ID		RIFF_DATA	= 0x41544144;

		static const long long 			DATA_OBJECTS	= 0x20535443454A424FLL,
										DATA_TEXTURES	= 0x5345525554584554LL,
										DATA_MATERIAL	= 0x4C4149524554414DLL,
										DATA_ANIMATOR	= 0x524F54414D494E41LL,
										DATA_CONNECTR	= 0x525443454E4E4F43LL;

		#define VSIZE(vlyr, flags) (3+2*vlyr+(flags&CGS::HasTangentFlag?3:0)+(flags&CGS::HasNormalFlag?3:0)+(flags&CGS::HasColorFlag?4:0))



		extern M::TVec4<>			ambient_default,	//!< Global default ambient material color
								diffuse_default,	//!< Global default diffuse material color
								specular_default,	//!< Global default specular material color
								emission_default;	//!< Global default emission material color
		extern float			shininess_default;	//!< Global default material shininess
					
		extern LogFile			*log;	//!< Global CGS log file (NULL/disabled by default)



		class	TextureObject;
		class	TextureA;
		struct	TLayer;
		template <class> class	MaterialA;
		template <class> class	RenderObjectA;
		template <unsigned> struct TStep;
		template <class, unsigned> class TraceA;
		template <class> class	AnimatorA;
		template <class> struct TWheel;
		template <class> struct TAccelerator;
		template <class> class	Tracks;
		template <class> struct TConnector;
		class TextureResource;
			class LocalTexResource;
		template <class> class VertexChunk;
		template <class> class IndexContainerA;
		template <class> class SubGeometryA;
		template <class> class Geometry;

		template <class> class StaticSubInstanceA;
		template <class> class StaticInstance;
		template <class> class AnimatableInstance;
		template <class> class AnimatableSubInstanceA;

		class StdDef;

		template <class Def>
			class Constructor;


		typedef Ctr::Array<index_t>					Path;
		typedef Container::BasicVector<index_t>	BuildPath;

		TextureA* cloneTexture(TextureA*source);
		Riff::Chunk*openBlock(Riff::Chunk&riff,tName name);



		class StdDef	//! Global default definition structure. A definition structure defines what kind of types should be used for what data. It is adviced to use this Def-structure if geometries are written to and/or read from files.
		{
		public:
		typedef float	FloatType;			//!< Type used for default floating point storage and operations
		typedef float	PhHullFloatType;	//!< Type used to describe the physical hull of a sub geometry	
		typedef double	PhFloatType;		//!< Type used for physical floating point storage and operations
		typedef double	SystemType;			//!< Type used for system storage and operations
		typedef UINT32	IndexType;			//!< Type used to represent geometry vertex indices
		};




		class WheelCompatible			//! Wheel attachment base class
		{};
		class AcceleratorCompatible		//! Accelerator attachment base class
		{};
		class SubGeometryCompatible			//! Geometry child attachment base class
		{};


		struct TChunkInfoIO	//! Vertex container information file io structure
		{
				UINT32									vertex_layers;	//!< Number of 2 component texture coordinates per vertex
		};


		struct TRenderObjectInfoIO	//! Render object information file io structure
		{
				String									tname;		//!< Object target name
				UINT32									vcnt,		//!< Total number of vertices
														vflags,		//!< Vertex flags
														detail;		//!< Effective detail layer
		};


		struct TAnimationStatus
		{
				M::TVec3<double>						translation,	//!< Translation vector
														rotation;		//!< Rotation momentum vector
		};
	


		template <class Def=StdDef>
		struct TMaterialInfoIO		//! Material info file io structure.
		{
				typedef typename Def::FloatType	FLOAT;
			
				UINT32									flags;		//!< Material flags
				FLOAT									ambient[4],	//!< Material ambient color
														diffuse[4],	//!< Material diffuse color
														emission[4],	//!< Material emission
														specular[4],	//!< Material specular color
														shininess,			//!< Material shininess level (0-1 mapped to exponent 0-128)
														alpha_threshold;	//!< Transparency border used to determine visibility of a fragment if flags contains MASK_FLAG
		};

		struct TLayerIO	//! Layer info file io structure
		{
				UINT32									combiner,	//!< Used combiner (OpenGL env combiner at this point)
														flags;		//!< Layer flags
				INT64									sname;		//!< Texture target name
		};


		template <class Def=StdDef>
			struct TConnector	//! Static connector for NURBS tracks. Connectors are global objects.
			{
				M::TVec3<typename Def::PhFloatType>		p0,	//!< Connector beginning
														p1,	//!< Connector end
														direction;	//!< Connector direction
			};
	
		template <class Def=StdDef>
			struct TMounting	//!< V1.0 generic mounting for external objects. Mountings are sub geometry objects.
			{
				enum eFlags	//! Applicable mounting flags
				{
					Dropable	=0x1,	//!< Mounted object can be dropped vertically (additional fuel tank, bomb, parachute load)
					Missile		=0x2,	//!< Mounted object can be activated like a missile (dropped first before it activates)
					Gun			=0x4	//!< Mounted object can be fired like a gun
				};
			
				UINT32									flags;			//!< Usage flags
				M::TVec3<typename Def::FloatType>			position,	//!< Relative mounting position (relative to the respective parent sub geometry)
														direction,	//!< Relative mounting (negative Z) direction
														vertical,	//!< Relative vertical (positive Y) direction
														dimension;	//!< Maximum attached width, height, and depth. The bounding box is described by -dimension/2 -> dimension/2
				typename Def::FloatType					max_payload;	//!< Maximum attachable weight or 0 for any.
			};

		struct TInstanceObject	//! Geometry instance attachment base class
		{};

		class TextureObject	//! Texture attachment base class
		{};

		class TextureA:/*public Reference<TextureObject>,*/ public TextureObject	//! Texture data object
		{
		public:
			name64_t								name;		//!< 64 bit integer name of this texture
			Ctr::Array<Ctr::Array<BYTE>,Adopt>				face_field;	//!< (compressed) face data
			hash_t									data_hash;	//!< Hash value of the provided data

													TextureA();
			void									adoptData(TextureA&other);
			void									updateHash();
			void									downSample(BYTE exponent_modifier, String*error_out=NULL);	//!< Downsamples the local texture by the specified exponent. \param exponent_modifier Value to decrement the size exponents by. 1 would halve both width and height of all textures, 2 scale down to 1/4 and so on. \param error_out Optional pointer to a string to contain error descriptions (if any)
			void									limitSizeExponent(BYTE max_exponent, String*error_out=NULL);	//!< Limites the size (width/height) exponent to the specified exponent. \param max_exponent Maximum size exponent allowed once this method finished  \param error_out Optional pointer to a string to contain error descriptions (if any)
			bool									IsEmpty()	const;	//!< Queries whether or not the local texture holds data
			bool									isSimilar(const TextureA&other)	const;

			friend hash_t							ToHash(const TextureA&t)
			{
				return t.data_hash;
			}
		};
		CGS_DECLARE_ADOPTING(TextureA);

		struct TLayer	//! Material layer configuration
		{
				UINT32									combiner;	//!< Layer combiner. At this point this value is derived from OpenGL combiners. Default combiner is GL_MODULATE (0x2100)
				bool									cube_map,	//!< Set true if this layer is a cube layer. This information is usually derived from or matched to the linked texture type.
														mirror_map,	//!< Only effective in combination with \b cube_map. Set true if the cube texcoords should immitate reflective appearance. Otherwise the cube will be mapped via the vertex normals.
														clamp_x,	//!< Only effective if \b cube_map is set false. Set true to clamp x/u-texcoords outside the range [0,1]. If \b clamp_x is set false then the texture will be repeated in x/u direction
														clamp_y;	//!< Only effective if \b cube_map is set false. Set true to clamp y/v-texcoords outside the range [0,1]. If \b clamp_y is set false then the texture will be repeated in y/v direction
				name64_t								source_name;	//!< Source texture name (int64)
				TextureA								*source;		//!< Source texture link. May be NULL if the texture has not yet been loaded or is unavailable.
		};


		bool		layerRequiresTexCoords(const TLayer&layer);	//!< Checks whether or not the specified layer requires texcoords @param layer Layer to check @return true if the specified layer requires texcoords, false otherwise


		template <class C>
		class DimensionParser //! Structure used to determine the dimensions of a structure.
		{
		public:
			M::Box<C>										dimension;	//!< Effective dimensions
			bool										set;			//!< True if dimensions are defined

														DimensionParser();
			template <class C0>
				void									parse(const C0 point[3]);	//!< Includes a point into the given dimension space
			template <class C0>
				void									parse(const M::TVec3<C0>&point);	//!< Includes a point into the given dimension space
			template <class C0>
				void									parse(const M::Box<C0>&box);	//!< Includes a box into the given dimension space
		};

		template <class Def>
			struct TParticle	//! Particle definition. A particle is a rectangular textured face that faces the camera at all times.
			{
				M::TVec3<typename Def::FloatType>			center;		//!< Particle center (relative to the respective parent system)
				typename Def::FloatType					width,		//!< Particle width
														height,		//!< Particle height
														z_angle;	//!< Rotation angle around the relative particle z-axis (-360..360)
				M::TVec2<typename Def::FloatType>			texcoord[4];	//!< Particle texture coordinates in the order: lower left, lower right, upper right, upper left. Usually (0,0), (1,0), (1,1), (0,1)
			};



		template <class Def=StdDef>
			class VertexContainerA	//! Vertex container of a render object
			{
			private:
			static	bool									equal;

															VertexContainerA(const VertexContainerA<Def>&)	{}
			public:
					Ctr::Array<typename Def::FloatType>			vdata;	//!< Actual vertex data. Each vertex is stored in the form [position[3],normal[3],{tangent[3]},{color[4]},texcoord[vlyr][2]].
					UINT16									vlyr;	//!< Number of texture coordinates (2 components each) provided by each vertex
					UINT32									vcnt,	//!< Total number of vertices in this vertex container
															vflags,
															vertex_crc;		//!< crc32 of the local vertex data
					M::TVec3<typename Def::FloatType>			cage[8];		//!< Reserved for client usage (not filled or referenced by the internal CGS system)
			
															VertexContainerA();
					void									updateCRC();	//!< Updates crc32 checksum on vertex change
					void									adoptData(VertexContainerA<Def>&other);
					unsigned								vsize() const;		//!< Queries number of floats per vertex
					count_t									vlen()	const;		//!< Queries total number of floats in the float-field.
					void									SetSize(count_t vertex_count, UINT16 layers, UINT32 flags=HasNormalFlag);	//!< Depreciated SetSize function. Resizes the local vertex field to match the given parameters. Any existing vertex data is lost if the size of the underlying vertex array changes.
					void									stretch(UINT16 layers, UINT32 flags);		//!< Stretches the vertex-field and copies the old data. Newly added fields are not cleared.
					bool									operator>(const VertexContainerA<Def>&other) const;							//!< Greater operator to create a linear order among vertex containers. The greater operator must be called before the respective less-operator is invoked,
					bool									operator<(const VertexContainerA<Def>&other) const;							//!< Less operator to create a linear order among vertex containers.  The greater operator must be called before the respective less-operator is invoked,
				template <class Def1>
					VertexContainerA<Def>&					operator=(const VertexContainerA<Def1>&other);			//!< Copy operator
				template <class C>
					void									scale(const C&factor);								//!< Scales all local vertices by the specified factor. Normals and texture coordinates are left unchanged.
			};


			CGS_DECLARE_ADOPTING_DEF(VertexContainerA);



		template <class Def=StdDef>
			class IndexContainerA	//! Index container of a render object
			{
			private:
			static	bool									equal;

															IndexContainerA(const IndexContainerA<Def>&)	{}

			public:
					Ctr::Array<typename Def::IndexType>			idata;	//!< Raw index data
					UINT32									index_crc;		//!< crc32 of the local index data (for fast distinguishing)
				union
				{
					struct
					{
						UINT32								triangles,		//!< Number of triangles in the idata field. Each triangle occupies 3 indices. Triangles are put before quads
															quads;			//!< Number of quads in the idata field. Each quad occupies 4 indices. Quads are put after triangles.
					};
					UINT32									composition[2];	//!< Ctr::Array access for faster IO operations
				};
				

															IndexContainerA():triangles(0),quads(0)	{}
					void									SetSize(UINT32 triangles, UINT32 quads);
					bool									operator>(const IndexContainerA<Def>&other) const;	//!< Greater operator to create a linear order among index containers. The greater operator must be called before the respective less-operator is invoked,
					bool									operator<(const IndexContainerA<Def>&other) const;	//!< Less operator to create a linear order among index containers. The greater operator must be called before the respective less-operator is invoked,
					void									adoptData(IndexContainerA<Def>&other);
				template <class Def1>
					IndexContainerA<Def>&					operator=(const IndexContainerA<Def1>&other);		//!< Copy operator
				
					count_t									countFaces()					const;	//!< Counts all faces (triangles and quads) of the local index container. Quads and triangles are treated equaly
					count_t									countTriangles()				const;	//!< Counts all triangles of the local index container. Quads are counted twice (for each two triangles per quad)
					count_t									countQuads()					const;	//!< Counts all quads of the local index container. Triangles are ignored.
			};
			CGS_DECLARE_ADOPTING_DEF(IndexContainerA);



		class MaterialObject	//! Material info attachment base class
		{
		public:
			virtual ~MaterialObject()	{};
		};

		class MaterialHook		//!< Material data / render object attachment base class
		{};

		/*!
			\brief Object reference as seen from material perspective
		
			A RenderObjectA<Def> instance effectively describes one detail level of one SubGeometry<Def> instance as seen by the respective parent material.
			For each one SubGeometry<Def> instance only one RenderObjectA<Def> instance may exist per detail level and material.
		*/
		template <class Def=StdDef>
			class RenderObjectA /*: public Reference<MaterialHook>*/
			{
			private:
			static	bool									equal;

															RenderObjectA(const RenderObjectA<Def>&)	{}
					void									operator=(const RenderObjectA<Def>&)	const	{}

			public:
					String									tname;		//!< Name of the target SubGeometry<Def> instance (prelink)
					SubGeometryA<Def>						*target;	//!< Pointer to the respective SubGeometry<Def> instance
					unsigned								detail;		//!< Effective detail level with 0 being the highest detail and each subsequent layer approximately halving the displayed detail
			
					IndexContainerA<Def> 					ipool;			//!< Index data of the local render object
					VertexContainerA<Def>					vpool;			//!< Vertex data of the local render object
					Ctr::Array<TParticle<Def> >				particle_field;	//!< Particles of the local render object. See TParticle for details.
					unsigned								particle_config;//!< Particle configuration flags

															RenderObjectA();
					void									adoptData(RenderObjectA<Def>&other);
					count_t									countIndices()							const;	//!< Counts all indices of the local render object
					count_t									countFaces()							const;	//!< Counts all faces (triangles and quads) of the local render object
					count_t									countTriangles()						const;	//!< Counts all triangles of the local render object
					count_t									countQuads()							const;	//!< Counts all quads of the local render object
					count_t									countFaces(unsigned detail_layer)		const;	//!< Returns the total number of faces of the local render object if the specified detail layer matches the local detail layer, 0 otherwise
					count_t									countTriangles(unsigned detail_layer)	const;	//!< Returns the total number of triangles of the local render object if the specified detail layer matches the local detail layer, 0 otherwise
					count_t									countQuads(unsigned detail_layer)		const;	//!< Returns the total number of quads of the local render object if the specified detail layer matches the local detail layer, 0 otherwise
			
					String									ToString(const String&intend="")		const;	//!< Generates a string representation of the local data (not containing any actual vertex or index data).
				template <class Def1>
					RenderObjectA<Def>&						operator=(const RenderObjectA<Def1>&other);		//!< Copy operator
					bool									operator>(const RenderObjectA<Def>&other) const;	//!< Greater operator to create a linear order among render objects. The greater operator must be called before the respective less-operator is invoked,
					bool									operator<(const RenderObjectA<Def>&other) const;	//!< Less operator to create a linear order among render objects. The greater operator must be called before the respective less-operator is invoked,
			
					void									invert();							//!< Inverts the effective vertex order of each face (quad or triangle) of the local render object, thus inverting the visibility of each face if backface culling is enabled.

					bool									validIndices();						//!< Determines whether or not the indices of the local render object are valid. Valid indices are less than the respective total number of available vertices in the vertex container.
				template <class C>
					void									scale(const C&factor);								//!< Scales the local render object (vertices) by the specified factor. Normals are left unchanged.
			
			
			//		String								difference(RenderObject&other, const String&intend);
			};

		CGS_DECLARE_ADOPTING_DEF(RenderObjectA);



		class MaterialColors
		{
		public:
			bool										alpha_test,			//!< Set true if the local material is masked (transparency tested). Masked material fragments are either fully opaque or invisible but never blended.
														fully_reflective;	//!< Set true to apply any environment map everywhere on the material. All other material properties are ignored
			M::TVec4<float>								ambient,			//!< Material ambient color. The last component is supposed to be 1.
														diffuse,			//!< Material diffuse color. The last component is supposed to be 1.
														specular,			//!< Material specular color. The last component is supposed to be 1.
														emission;			//!< Material emission. The last component is supposed to be 1.
			float										shininess,			//!< Material shininess. The stored scalar [0,1] is mapped to the effective shininess exponent [0,128]
														alpha_threshold;	//!< Alpha border used to determine visibility. Effective only if \b masked is set true.

														MaterialColors();
			bool										Similar(const MaterialColors&other)	const;	//!< Determines whether or not the local material info is similar to the specified other material info. This method may return false where the finally loaded appearance is in fact similar, however never the other way around. \param other Material info to compare to \return true if the local material info is similar to the specified material info, false otherwise.
			bool										operator==(const MaterialColors&other) const;
			bool										operator!=(const MaterialColors&other) const	{return !operator==(other);}

		};

		/*!
			\brief CGS material info structure
		
			The material info structure holds all material parameters (i.e. colors) as opposed to the actual geometrical rendering data.
		*/
		class MaterialInfo : public MaterialColors
		{
		public:
			std::weak_ptr<MaterialObject>				attachment;

			Ctr::Array<TLayer>								layer_field;		//!< Ctr::Array of texture layers. The number of texture layers must correspond to the number of texture coordinates provided by the vertex container of each rendering object (of this material)

														MaterialInfo();
			bool										Similar(const MaterialInfo&other)	const;	//!< Determines whether or not the local material info is similar to the specified other material info. This method may return false where the finally loaded appearance is in fact similar, however never the other way around. \param other Material info to compare to \return true if the local material info is similar to the specified material info, false otherwise.
			bool										operator==(const MaterialInfo&other) const;
			bool										operator!=(const MaterialInfo&other) const	{return !operator==(other);}
			MaterialInfo&								operator=(const MaterialColors&other);
			count_t										countCoordLayers()	const;					//!< Retrieves the number of 2 component texture coordinates, this material requires (not all layers do require texture coordinates).
			void										postCopyLink(TextureResource*);				//!< Relinks texture resources if a CGS geometry has been copied (automatically evoked)
			void										adoptData(MaterialInfo&other);
		};
		CGS_DECLARE_ADOPTING(MaterialInfo);



		/*!
			\brief CGS material data structure
		
			The material data structure holds the actual geometrical rendering data.
		*/
		template <class Def=StdDef>
		class MaterialData /*:public Reference<MaterialHook>*/
		{
		private:
		static	bool									equal;

														MaterialData(const MaterialData<Def>&)	{}
				void									operator=(const MaterialData<Def>&)	const	{}

		public:
													
				Ctr::Array<RenderObjectA<Def>,Adopt>	object_field;				//!< Rendering objects provided by the local material
				UINT16									coord_layers;				//!< Global number of 2 component texture coordinates provided by all vertices of this material

														MaterialData()	{}
				bool									operator>(const MaterialData<Def>&other) const;	//!< Greater operator to create a linear order among material data sections. The greater operator must be called before the respective less-operator is invoked,
				bool									operator<(const MaterialData<Def>&other) const;	//!< Less operator to create a linear order among material data sections. The greater operator must be called before the respective less-operator is invoked,
			template <class Def1>
				MaterialData<Def>&						operator=(const MaterialData<Def1>&other);			//!< Copy operator
				void									postCopyLink(Geometry<Def>*);						//!< Relinks render object targets if a CGS geometry has been copied (automatically evoked)
				void									adoptData(MaterialData<Def>&other);
			
		};
		CGS_DECLARE_ADOPTING_DEF(MaterialData);



		/*!
			\brief Central CGS material structure
		
			A CGS material structure consists of an info section and a data section. The info section can be used to bind a material to some sort of rendering context while
			the data section consists of actual geometrical data.
		*/
		template <class Def=StdDef>
		class MaterialA
		{
		private:
														MaterialA(const MaterialA<Def>&)	{}
		public:
				MaterialInfo							info;	//!< Info section of the local material
				MaterialData<Def>						data;	//!< Data section of the local material
				String									name;	//!< Name of the local material

														MaterialA()	{}
				void									SetSize(count_t layers, count_t objects);	//!< Mostly depreciated. Used to SetSize the respective info.layer_field and data.object_field arrays.
				void									adoptData(MaterialA<Def>&other);
				void									linkTextures(TextureResource*res);				//!< Links referenced textures via their name to loaded texture resources provided by the specified texture resource. \param res Texture resource to retrieve textures from.
				String									ToString(const String&intend="")		const;	//!< Creates a string representation of the local material \param intend Intendation string put before each new line of the returned string.
				count_t									countFaces()							const;	//!< Counts all faces (quads and triangles)  of all detail levels of this material
				count_t									countTriangles()						const;	//!< Counts all triangles of all detail levels of this material
				count_t									countQuads()							const;	//!< Counts all quads of all detail levels of this material
				count_t									countFaces(unsigned detail_layer)		const;	//!< Counts all faces (quads and triangles) of the specified detail level of this material
				count_t									countTriangles(unsigned detail_layer)	const;	//!< Counts all triangles of the specified detail level of this material
				count_t									countQuads(unsigned detail_layer)		const;	//!< Counts all quads of the specified detail level of this material
				count_t									countVertices()							const;	//!< Counts all vertices of all detail levels of this material
			template <class Def1>
				MaterialA<Def>&							operator=(const MaterialA<Def1>&other);
				void									postCopyLink(Geometry<Def>*,TextureResource*);					//!< Relinks the info and data sections if a geometry instance has been copied (automatically invoked)
		//		String								difference(Material&other, const String&intend);

				void									storeTargetNames();								//!< Pre-copy method. Stores the respective target names of textures and sub geometry instances.
			template <class C>
				void									scale(const C&factor);							//!< Scales the local material (vertices) by the specified factor. Normals are left unchanged.
		};
		CGS_DECLARE_ADOPTING_DEF(MaterialA);

		/*!
			\brief Abstract texture resource
		*/
		class TextureResource
		{
		public:
		virtual	void									setLocalResource(const LocalTexResource&)	{}
		virtual count_t									countEntries();					//!< Counts all textures provided by the texture resource
		virtual TextureA*								retrieve(const tName&name);		//!< Retrieves a texture via its int64 name
		virtual TextureA*								entry(index_t index);			//!< Retrieves a texture via its index (ranging [0,countEntries()-1])
				void									downSampleAll(BYTE exponent_modifier);	//!< Downsamples all local textures by the specified exponent. \param exponent_modifier Value to decrement the size exponents by. 1 would halve both width and height of all textures, 2 scale down to 1/4 and so on.
		};


		template <unsigned B>
		struct TStep		//! Animation step
		{
				double									start,			//!< Start time of this step
														end,			//!< End time of this step
														slope0[B],		//!< Beginning slope (per second)
														slope1[B],		//!< Ending slope (per second)
														range[B];		//!< Change vector during this step. The effective animation change is the scaled local vector plus the sum of all preceeding steps.
		};

		template <class T, unsigned B>
			class TraceA		//! Collection of steps for one specific target
			{
			public:
					String									tname;			//!< Target name
					T										*target;		//!< Target pointer (object, wheel, or accelerator depending on type)
					Ctr::Array<TStep<B> >						step_field;		//!< Steps of this trace

															TraceA();
														
					void									current(const double&time, double out[B])	const;	//!< Calculates the animation status at the specified time
					void									calculateSlopes();	//!< Fills slope0 and slope1 of all steps
					void									adoptData(TraceA<T,B>&other);
					void									revert();
			};

		CGS_DECLARE_ADOPTING_2(TraceA,class,unsigned);


		template <class Def=StdDef>
		class AnimatorA		//! General animator class 
		{
		private:
		/*												Animator(const Animator<Def>&other) {}
				void									operator=(const Animator<Def>&other) const {}*/
			
		template <class T, unsigned B>
			static void 								loadTraces(Ctr::ArrayData<TraceA<T,B> >&traces,Riff::File&riff,const char*riff_name0, const char*riff_name1);
		template <class T, unsigned B>
			static void									saveTraces(const Ctr::ArrayData<TraceA<T,B> >&traces,Riff::Chunk*riff,const char*riff_name);
		template <class T, unsigned B>
			static void 								storeTargetNames(Ctr::ArrayData<TraceA<T,B> >&field);
		
		public:

				String									name;				//!< Animator name
				Ctr::Array<TraceA<SubGeometryA<Def>,6>,Adopt>	obj_trace_field;//!< Object animation traces. Each trace animates one SubGeometry<Def> instance. The first three components of the change vectors are linear translation, the next three components represent the rotation momentum
				Ctr::Array<TraceA<TAccelerator<Def>,1>,Adopt>	acc_trace_field;//!< Accelerator animation traces. Each trace animates one TAccelerator<Def> instance. Each step vector consists of just one value, the effective acceleration strength [0,1].
				Ctr::Array<TraceA<TWheel<Def>,2>,Adopt>		whl_trace_field;	//!< Wheel animation traces. Each trace animates one TWheel<Def> instance. Each step vector consists of two components, the first being the acceleration strength [0,1], the second the brake strength [0,1].

														AnimatorA();
			template <class Def1>
				AnimatorA<Def>&							operator=(const AnimatorA<Def1>&other);	//!< Copy operator
				void									adoptData(AnimatorA<Def>&other);
				void									SetSize(count_t obj_count, count_t acc_count, count_t whl_traces);	//!< Mostly depreciated SetSize method. Resizes the local trace arrays. The content of an array is lost if it is resized.
				String									ToString(const String&intend="")	const;	//!< Generates a string representation of the local animation \param intend Intendation string put before each new line of the resulting string.
				bool									valid(Geometry<Def>*structure);			//!< Determines whether or not all trace links are valid. This procedure is very slow. Use only when debugging.
				double									executionTime()						const;	//!< Determines the number of seconds this animation takes
			
				void									loadFromRiff(Riff::File&riff, index_t index_in_context);	//!< Loads the local animator from the current context of a RIFF resource
				void									link(Geometry<Def>*domain);					//!< Links the local traces via their respective target names.
				void									saveToRiff(Riff::Chunk*riff)	const;					//!< Saves the local animator data to the specified riff chunk context
			
				void									storeTargetNames();								//!< Retrieves and stores the respective trace target names
				double									GetLength()	const;								//!< Retrieves the length (in seconds) of this animation
			
				void									revert();										//!< Reverts all animator traces. No data is actually allocated or freed.
			template <class C>
				void									scale(const C&factor);								//!< Scales the animator (steps) by the specified factor.
	//			void									calculateSlopes();								//!< Fills slope0 and slope1 of all steps of all traces
			
		};

		CGS_DECLARE_ADOPTING_DEF(AnimatorA);


		/*!
			\brief Wheel collection
		
			Tracks describes a collection of wheels that are supposed to form a larger wheel construct (i.e. tank tracks).
		*/
		template <class Def=StdDef>
		class Tracks
		{
		public:
				struct Member
				{
					String								name;
					TWheel<Def>*						wheel;
				};

				SubGeometryA<Def>						*domain;		//!< Link to the sub geometry that currently contains this construct
				Ctr::Array<Member>							member_field;	//!< All wheels that are part of the local tracks compound
				//AnimatorA<Def>							rotation;		//!< Rotation animation
				UINT32									flags;			//!< Non-persistent general purpose flags
			
				void									adoptData(Tracks<Def>&other)
				{
					if (this == &other)
						return;
					domain = other.domain;
					member_field.adoptData(other.member_field);
					flags = other.flags;
				}
		};
			CGS_DECLARE_ADOPTING_DEF(Tracks);



		/*!
			\brief Wheel descriptor
		*/
		template <class Def=StdDef>
		struct TWheel /*:public Reference<WheelCompatible>*/
		{
				SubGeometryA<Def>						*domain;		//!< Link to the sub geometry that currently contains this wheel
				String									name;			//!< Name of this wheel
				M::TVec3<typename Def::PhFloatType>		position,		//!< 3d coordinates of the wheel center relative to the respective parent SubGeometry<Def> instance system.
														contraction,	//!< Vector describing to what direction and extent the wheel moves during a suspension animation (relative to its base position and the respective parent system orientation)
														axis;			//!< Wheel rotation axis (relative to the respective parent system orientation)
													
				typename Def::PhFloatType				radius,			//!< Wheel radius
														width;			//!< Wheel width
				UINT32									flags;			//!< Non-persistent general purpose flags
				union
				{
					typename Def::PhFloatType			parameter[7];	//!< Physical wheel attributes
					struct
					{
						typename Def::PhFloatType		power,				//!< General acceleration power
														zero_efficiency,	//!< Efficiency in atmosphere-less environments (in the range [0,1])
														grip,				//!< Grip scalar (multiplied by the respective ground grip) (in the range [0,1])
														suspension_strength,//!< Physical strength of the wheel suspension
														suspension_friction,//!< Friction of the wheel suspension to prevent oscillation(in the range [0,1])
														rotation_friction,	//!< Wheel rotation friction
														density;			//!< Wheel material density (to calculate the wheel weight)
					};
				};
													
				AnimatorA<Def>							suspension,		//!< Suspension animation
														rotation;		//!< Rotation animation
		};

		/*!
			\brief Accelerator descriptor
		
			An accelerator describes a singular linear force.
		*/
		template <class Def=StdDef>
			struct TAccelerator /*:public Reference<AcceleratorCompatible>*/
			{
				SubGeometryA<Def>						*domain;		//!< Link to the sub geometry that currently contains this accelerator
				//name64_t								name;			//!< Accelerator name
				String									name;
				M::TVec3<typename Def::PhFloatType>		position,		//!< 3d coordinates of the accelerator base relative to the respective parent SubGeometry<Def> instance system.
														direction;		//!< Accelerator force direction (relative to the respective parent system orientation)
				typename Def::PhFloatType				power,			//!< Maximum acceleration power
														zero_efficiency;	//!< Accelerator efficiency in atmosphere-less environments [0,1]
				typename Def::FloatType					flame_length,	//!< Optical flame length
														flame_width;	//!< Optical flame diameter
				M::TVec3<typename Def::FloatType>			flame_color;	//!< Flame base color
				UINT32									flags;			//!< Non-persistent general purpose flags
			};

		/*!
			\brief SubGeometry<Def> meta information
		*/
		template <class Def=StdDef>
			struct TMetaInfo
			{
				M::TMatrix4<typename Def::SystemType>		system;		//!< System relative to the respective parent system
				M::TVec3<typename Def::SystemType>			center;		//!< Mass center relative to the local system matrix
				typename Def::SystemType				radius,			//!< Average mass distribution from center relative to the local system matrix
														volume,			//!< Volume of the local SubGeometry<Def>
														density;		//!< Material density to calculate the local weight
				typename Def::FloatType					shortest_edge_length;	//!< Minimum length among all edges of the local geometry. Accounts for the most detailed layer only
				UINT32									flags;			//!< Non-persistent general purpose flags (of sub geometries
			};


		/*!
			\brief Simple linear texture resource
		*/
		class LocalTexResource:public TextureResource
		{
		public:
				Ctr::Array<TextureA,Adopt>					entry_field;		//!< Linear texture field

		virtual	count_t									countEntries();				//!< Returns the number of entries in the local texture resource
		virtual	TextureA*								retrieve(const tName&name);	//!< Retrieves a texture via its name
		virtual	TextureA*								entry(index_t index);		//!< Retrieves a texture via its index
		};

	
		template <class Def=StdDef>
			class Iterator		//! SubGeometry iterator class. Can be used to iterate through all sub geometries of a Geometry instance
			{
			private:
					typedef Ctr::ArrayData<SubGeometryA<Def> > 	Context;

					Container::Vector<Context*>			context_stack;
					Container::Vector<SubGeometryA<Def>*>	entry_stack;
					Geometry<Def>							*super;
					Context									*context;
					SubGeometryA<Def>						*current;
			public:
															Iterator(Geometry<Def>*target);
															Iterator(Geometry<Def>&target);
					SubGeometryA<Def>&						operator*()		const;
					SubGeometryA<Def>*						operator->()	const;
				
					bool									operator++();
					bool									operator++(int);
				
					operator SubGeometryA<Def>*	() const {return current;};				
					operator bool()	const;
			};
	
		/*!
			\brief Abstract callback object for the Geometry<Def>::walkChildren() method.
		*/
		template <class Def=StdDef>
		class SubGeometryCallback
		{
		public:
		virtual	void									geometryCallback(SubGeometryA<Def>*)=0;	//!< Abstract callback method, which will be invoked exactly once for each SubGeometry<Def> instance
		};

		/*!
			\brief Geometry sub geometry instance
		
			A SubGeometry<Def> instance represents a node in the geometry tree. Each sub geometry may have any number of children, each positioned relative to the local system matrix.
			Each sub geometry provides relative geometrical information as well as physical data, wheels, accelerators and constructs.
		*/
		template <class Def=StdDef>
		class SubGeometryA /*:public Reference<SubGeometryCompatible>*/
		{
		private:
				static String							error;

				void									saveToRiff(Riff::Chunk*chunk)	const;
				void									loadFromRiff(Riff::File&source);
				void									linkAnimators(Geometry<Def>*domain);
				void									postCopyLink(Geometry<Def>*domain);

				template <class C>
				bool									extractDimensions(const M::TMatrix4<typename Def::SystemType>&system, DimensionParser<C>&parser) const;
				void									checkLinkage();
				template <class C>
				void									extractAbsoluteRadius(C&radius) const;
				template <class C>
				void									extractAbsoluteRadius(const M::TVec3<C>&center, C&radius) const;
				template <class C>
				void									extractRadiusRec(C&radius) const;
				template <class C>
				void									extractRadiusRec(const M::TVec3<C>&center, C&radius) const;
				void									setSystemRec(const M::TMatrix4<typename Def::SystemType>&system);
				void									resetLinkageRec();
				void									toInstance(StaticSubInstanceA<Def>&tree);
				void									toInstance(AnimatableSubInstanceA<Def>&tree);
				void									defLinkageStr(String&rs, const String&intend);
				count_t									countVertices(unsigned detail=0);
				String									ToString(const String&intend="")	const;
				void									verifyIntegrity(Geometry<Def>*super, const String&path ="");
				bool									checkIntegrity(Geometry<Def>*super, const String&path="");


				void									remap(BuildPath&path, Geometry<Def>&geometry, UINT32 flags);
			

				friend	class		Geometry<Def>;

		public:
				typedef TVsDef<typename Def::FloatType>			VsDef;		//!< Visual definition for the vs_hull_field object array
				typedef TVsDef<typename Def::PhHullFloatType>	PhDef;		//!< Physical definition for the phHull object. VsDef suffices in what is needed these days
				//typedef TPhFileDef<typename Def::PhFloatType>	PhFileDef;

				Ctr::Array<SubGeometryA<Def>,Adopt>					child_field;		//!< Children
				Mesh<PhDef>										phHull;			//!< Physical hull
				Ctr::Array<Mesh<VsDef>,Adopt>						vs_hull_field;		//!< Visual hulls (one for each detail level with 0 being the highest detail)
				Ctr::Array<Tracks<Def>,Adopt>						tracks_field;		//!< Chained wheels
				Ctr::Array<TAccelerator<Def> >						accelerator_field;	//!< Accelerators
				Ctr::Array<TMounting<Def> >							mounting_field;		//!< Mountings
				Ctr::Array<TWheel<Def> >								wheel_field;		//!< Wheels
				String											name;				//!< Sub geometry name
				TMetaInfo<Def>									meta;				//!< Sub geometry meta information
				M::TMatrix4<typename Def::SystemType>			path,				//!< Local path (A path is the product of the local system and all parent systems and must be updated whenever one of the parent systems changed)
																*system_link;		//!< Linked path. This pointer initially points to the local path variable but may be relinked to identify a different sub geometry instance of the local geometry.



														SubGeometryA();
														SubGeometryA(const SubGeometryA<Def>&other);
				template <class T>						SubGeometryA(const SubGeometryA<T>&other);
								
				void									clear();									//!< Clears all arrays and resets all variables of the local sub geometry instance

				bool									isMember(SubGeometryA<Def>*obj)		const;	//!< Recursivly determines whether or not the specified sub geometry instance is a direct or indirect descendent of the local sub geometry instance \param obj sub geometry instance to look for \return true if the specified sub geometry instance is a descendant of the local sub geometry instance, false otherwise
				bool									isMember(TWheel<Def>*whl)			const;	//!< Recursivly determines whether ot not the specified wheel belongs to the local sub geometry instance or any of its decendents \param whl Wheel to look for \return true if the specified wheel is a member of the local sub geometry instance or any of its decendents, false otherwise.
				bool									isMember(TAccelerator<Def>*acc)		const;	//!< Recursivly determines whether ot not the specified accelerator belongs to the local sub geometry instance or any of its decendents \param acc Accelerator to look for \return true if the specified accelerator is a member of the local sub geometry instance or any of its decendents, false otherwise.

				void									storeTargetNames();							//!< Stores pointer target names (animator trace targets, wheel pointers, etc) in the respective structures.

			
			template <class C>
				bool									extractDimensions(M::Box<C>&dim) const;			//!< Determines the relative dimensions of the first visual detail layer of the local sub geometry instance (non recursive) \param dim Dimension field to store the visual dimensions in \return true if the local visual hull contained at least one vertex, false otherwise
			template <class C>
				void									extractRadius(C&radius) const;				//!< Determines the local visual radius (distance between the point of origin an the furthest vertex) and stores it in out(non recursive) \param radius Out radius
			template <class C>
				void									addWeight(C&weight)		const;				//!< Recursivly adds the local weight (volume*density) to the passed weight reference.
			template <class C>
				void									scale(const C&factor);								//!< Scales the entire sub geometry (sub children, objects, systems) by the specified factor. Normals are left unchanged.		
				void									createVisualMapsRec(unsigned detail, unsigned max_depth);				//!< Creates map objects for all visual objects of the specified detail (if not already created). Also generates visual normals		 \param detail Target detail layer (0=most detail) \param max_depth Maximum map recursion depth as passed on to Object<>::buildMap()
				void									walkChildrenRec(void (*childCallback)(SubGeometryA<Def>*));					//!< Sequentially passes all objects (on any sub layer) to the specified callback function
				void									walkChildrenRec(SubGeometryCallback<Def>*callback);							//!< Sequentially passes all objects (on any sub layer) to the specified callback handler
			
			template <typename T>
				void									extractAverageVisualEdgeLength(index_t layer, T&length)	const;		//!< Determines the local average visual edge length (non recursive). \param layer Detail level to determine the average visual edge length of \param length Out scalar to store the average edge length in
			template <typename T>
				void									extractShortestVisualEdgeLength(index_t layer, T&length)	const;		//!< Determines the local average visual edge length (non recursive). \param layer Detail level to determine the average visual edge length of \param length Out scalar to store the average edge length in
			template <class Def0>
				SubGeometryA<Def>&						operator=(const SubGeometryA<Def0>&origin);
				SubGeometryA<Def>&						operator=(const SubGeometryA<Def>&origin);
				void									adoptData(SubGeometryA<Def>&other);
			
				void									clearFlags(UINT32 clear_mask=0xFFFFFFFF, bool geometries=true, bool wheels=true, bool accelerators=true, bool constructs=true);	//!< Recursivly clears the specified flags. @param clear_mask Mask to limit clearing of flags. Only bits in this value are cleared @param geometries Apply to sub geometries @param wheels Apply to wheels @param accelerators Apply to accelerators @param constructs Apply to constructs
				void									flagAnimationTargets(UINT32 flag=AnimationTargetFlag);	//!< Recursivly adds the specified flag to targeted sub geometries, wheels, and accelerators
			

		private:
			template<typename T0, typename T1>
				static inline void						_IncludeEdgeLength(T0&edgeLen, const T1&newEdgeLen)
														{
															if (newEdgeLen > TypeInfo<T1>::error)
																edgeLen = M::Min(edgeLen,newEdgeLen);
														}

														//		String								difference(SubGeometry&other, const String&intend);
		};
			CGS_DECLARE_ADOPTING_DEF(SubGeometryA);


		template <typename T>
			class System	//! Basic system type
			{
			public:
					class Callback
					{
					public:
					virtual		void					onSystemChange()=0;
					};
		
					M::TMatrix4<T>						matrix;
					Callback							*const callback;
			
														System(Callback*callback);
														System(const System<T>&other);
				template <typename T2>
														System(const System<T2>&other);
					System<T>&							operator=(const System<T>&other);
				template <typename T2>
					System<T>&							operator=(const System<T2>&other);
				
					void								loadIdentity(bool do_callback=true);					//!< Loads the identity matrix into the local system matrix \param do_callback If set System will execute the assigned callback link (if any)
				
					inline void							setPosition(const T&x, const T&y, const T&z);			//!< Sets the local system base position \param x X-component of the new system position \param y Y-component of the new system position \param z Z-component of the new system position
					inline void							setPositionNoCallback(const T&x, const T&y, const T&z);	//!< Identical to the above except that it does not execute the callback link
					M::TMatrix4<T>*						getMatrix()	{return &matrix;}
				template <typename T2>
					inline void							setPosition(const M::TVec3<T2>&point);							//!< Sets the local system base position \param point New system base position
				template <typename T2>
					inline void							setPositionNoCallback(const M::TVec3<T2>&point);				//!< Identical to the above except that it does not execute the callback link
					inline void							moveTo(const T&x, const T&y, const T&z);				//!< Identical to setPosition()
					inline void							moveToNoCallback(const T&x, const T&y, const T&z);		//!< Identical to the above except that it does not execute the callback link
				template <typename T2>
					inline void							moveTo(const M::TVec3<T2>&point);								//!< Identical to setPosition()
				template <typename T2>
					inline void							moveToNoCallback(const M::TVec3<T2>&point);					//!< Identical to the above except that it does not execute the callback link
				
					inline void							translate(const T&x, const T&y, const T&z);				//!< Translates the local system matrix and executes the assigned callback link (if any) \param x Delta vector x component \param y Delta vector y component \param z Delta vector z component \param do_update Set true to automatically update path matrices of all local instance children when done
					inline void							translateBy(const T&x, const T&y, const T&z);			//!< Identical to translate()
					inline void							translateNoCallback(const T&x, const T&y, const T&z);	//!< Identical to translate() except that it does not execute the callback link
				template <typename T2>
					inline void							translate(const M::TVec3<T2>&vector);							//!< Translates the local system matrix and executes the assigned callback link (if any) \param vector Delta vector
				template <typename T2>
					inline void							translateNoCallback(const M::TVec3<T2>&vector);				//!< Identical to the above except that it does not execute the callback link
				template <typename T2>
					inline void							translate(const M::TVec3<T2>&vector, const T&factor);			//!< Translates the local system matrix and executes the assigned callback link (if any) \param vector Base delta vector \param factor Factor to scale \b vector by
				template <typename T2>
					inline void							translateNoCallback(const M::TVec3<T2>&vector, const T&factor);//!< Identical to the above except that it does not execute the callback link
					void								rotate(const T&angle, const T&axis_x,const T&axis_y, const T&axis_z);	//!< Rotates the axis vectors of the local system about an arbitrary axis and executes the assigned callback link (if any). The system base position is left unchanged. The given axis may be of any length greater 0. \param angle (360 degrees angle) \param axis_x X component of the rotation axis \param y Y component of the rotation axis \param z Z component of the rotation axis \param do_update Set true to automatically update path matrices of all local instance children when done
					void								rotateNoCallback(const T&angle, const T&axis_x,const T&axis_y, const T&axis_z);	//!< Identical to the above except that it does not execute the callback link
				template <typename T2>
					void								rotate(const T&angle, const M::TVec3<T2>&axis);	//!< Rotates the axis vectors of the the local system about an arbitrary axis and executes the assigned callback link (if any). The system base position is left unchanged. The given axis is required to be normalized. \param angle (360 degrees angle) \param axis Normalized axis to rotate about \param do_update Set true to automatically update path matrices of all local instance children when done
				template <typename T2>
					void								rotateNoCallback(const T&angle, const M::TVec3<T2>&axis);	//!< Identical to the above except that it does not execute the callback link
				
					inline void							update()	//! Executes the assigned callback link (if any)
														{
															if (callback)
																callback->onSystemChange();
														}
			
			};

		/*!
			\brief Central geometry structure
		
			The CGS::Geometry class is the central data structure used to hold, load, save and manage in general visual and physical data.
			Geometry data may be instanced using CGS::StaticInstance or directly used for rendering/simulation.
		*/
		template <class Def=StdDef>
		class Geometry:public MaterialHook, public System<typename Def::SystemType>::Callback
		{
		private:
				static String							error;
			
				void									loadMaterials(Riff::File&riff);//, bool post_strip, unsigned min_strip);
				void									loadObjects(Riff::File&riff);
				void									loadAnimators(Riff::File&riff);
				void									loadConnectors(Riff::File&riff);
				void									loadTextures(Riff::File&riff);

				const SubGeometryA<Def>*				lookupChild(const index_t*path_begin, index_t path_length)	const;
				SubGeometryA<Def>*						lookupChild(const index_t*path_begin, index_t path_length);

		
		protected:
				void									onSystemChange();

		public:
				typedef typename Def::FloatType			Float;
				typedef typename Def::IndexType			Index;
			
			
				Float									radius;				//!< Visual radius (updated when loaded)
				Ctr::Array<SubGeometryA<Def>,Adopt>			object_field;		//!< Primary sub geometry field
				LocalTexResource						local_textures;			//!< Local texture resource
				TextureResource							*texture_resource;		//!< Actually used texture resource
				Ctr::Array<AnimatorA<Def>,Adopt>		animator_field;		//!< Global animator field
				Ctr::Array<TConnector<Def> >			connector_field;	//!< Track connectors
				Ctr::Array<MaterialA<Def>,Adopt>		material_field;		//!< Material field
				String									info,				//!< Abstract info
														xml;				//!< XML formated information
				M::TMatrix4<typename Def::SystemType>	*system_link;		//!< Current system link
				System<typename Def::SystemType>		root_system;		//!< Local system
			

				Container::StringTable<Path>			object_table,
														wheel_table,
														accelerator_table,
														animator_table;
			
			
														Geometry();
														Geometry(Riff::File&source);
														Geometry(const Geometry<Def>&other);
				template <class Def0>					Geometry(const Geometry<Def0>&other);
		inline	bool									usesLocalTextureResource()	const		{return texture_resource == &local_textures;}
				void									clear();								//!< Clears the local structure, erasing all arrays and reseting all variables.
				void									SetSize(count_t objects, count_t materials, count_t animators, count_t connectors);
				std::shared_ptr<StaticInstance<Def> >	createInstance(bool copy_root_system=false);				//!< Creates a new instance (a spawn) of the local geometry. The new instance refers to the local geometry but may be animated separatly.
				std::shared_ptr<AnimatableInstance<Def> >createAnimatableInstance(bool copy_root_system=false);		//!< Creates a new animatable instance (a spawn) of the local geometry. The new instance refers to the local geometry but may be animated separatly.
				void									adoptInstance(StaticInstance<Def>&instance,bool copy_root_system=false);		//!< Adopts an existing instance to the local geometry. The instance will refer to the local geometry but may be animated separatly.
				void									adoptInstance(AnimatableInstance<Def>&instance,bool copy_root_system=false);		//!< Adopts an existing instance to the local geometry. The instance will refer to the local geometry but may be animated separatly.
				void									resetLinkage();			//!< Resets the local system links to their defaults (pointing to local system variables rather than remote ones).
				void									resetSystems();			//!< Resets system matrices to their original content
			
				void									storeTargetNames();		//!< Globally stores target names in the respective structures (textures, render objects, wheels, animators, etc). Recursive.
				void									relinkEverything();		//!< Relinks everything (textures, render objects, wheels, animators, etc) based on the stored target names.

				bool									SaveToFile(const PathString::char_t*filename)	const;	//!< Saves the local geometry to the specified file. \return true on success
				bool									SaveToFile(const PathString&filename)	const;	//!< @copydoc saveToFile()
				bool									LoadFromFile(const PathString::char_t*filename,TextureResource*resource=NULL);	//!< Loads the local geometry from the specified file overwriting any previously loaded local data. \return true on success
				bool									LoadFromFile(const PathString&filename,TextureResource*resource=NULL);	//!< @copydoc loadFromFile()
				void									saveEmbedded(Riff::Chunk&target, bool embed_textures)	const;	//!< Saves the local geometry to an existing RIFF chunk. \param target RIFF chunk to save to \param embed_textures Set true to also save local textures to the specified RIFF chunk
				void									loadFromRiff(Riff::File&source,TextureResource*resource=NULL);		//!< Loads the local geometry from the current context of the specified RIFF file.
				void									loadEmbedded(Riff::File&riff, TextureResource*resource); 	//!< Similar to loadFromRiff() but using a remote texture resource instead.

				SubGeometryA<Def>*						lookupObject(const String&name);								//!< Attempts to recursivly find a sub geometry instance via its name \param name int64 name of the sub geometry instance to look for \return Pointer to the respective sub geometry instance or NULL if it could not be found.
				const SubGeometryA<Def>*				lookupObject(const String&name)		const;					//!< Attempts to recursivly find a sub geometry instance via its name \param name int64 name of the sub geometry instance to look for \return Pointer to the respective sub geometry instance or NULL if it could not be found.
				TWheel<Def>*							lookupWheel(const String&name);								//!< Attempts to recursivly find a wheel via its name \param name int64 name of the wheel to look for \return Pointer to the respective wheel or NULL if it could not be found.
				const TWheel<Def>*						lookupWheel(const String&name)			const;				//!< Attempts to recursivly find a wheel via its name \param name int64 name of the wheel to look for \return Pointer to the respective wheel or NULL if it could not be found.
				TAccelerator<Def>*						lookupAccelerator(const String&name);						//!< Attempts to recursivly find an accelerator via its name \param name int64 name of the accelerator to look for \return Pointer to the respective accelerator or NULL if it could not be found.
				const TAccelerator<Def>*				lookupAccelerator(const String&name)	const;					//!< Attempts to recursivly find an accelerator via its name \param name int64 name of the accelerator to look for \return Pointer to the respective accelerator or NULL if it could not be found.
				AnimatorA<Def>*							lookupAnimator(const String&name);						//!< Attempts to recursivly find an accelerator via its name \param name int64 name of the accelerator to look for \return Pointer to the respective accelerator or NULL if it could not be found.
				const AnimatorA<Def>*					lookupAnimator(const String&name)	const;					//!< Attempts to recursivly find an accelerator via its name \param name int64 name of the accelerator to look for \return Pointer to the respective accelerator or NULL if it could not be found.
			
				/**
					@brief Rebuilds one or more of the local tables, and/or relinks all target links

					The specified target table(s) will be cleared and completely rebuilt from the local geometry state.
					Animation target links (and other pointers) will be re-generated if @a table_targets contains the RelinkBit.
					With the Exception of AnimatorBit, all other default bits contain this flag, thus enforcing relinkage.
					(Animators cannot be targeted, thus AnimatorBit does not include the RelinkBit)

					@param table_targets One or more of the flags in CGS::eTargetTableBits
				*/
				void									remap(UINT32 table_targets);


				bool									isMember(SubGeometryA<Def>*obj)				const;				//!< Recursivly determines whether or not the specified sub geometry instance is a direct or indirect descendent of the local geometry \param obj sub geometry instance to look for \return true if the specified sub geometry instance is a descendant of the local geometry, false otherwise
				bool									isMember(TWheel<Def>*whl)				const;				//!< Recursivly determines whether ot not the specified wheel belongs to any of the primary children of the local geometry or any of their decendents \param whl Wheel to look for \return true if the specified wheel is a member of any of the children of the local geometry or their decendents, false otherwise.
				bool									isMember(TAccelerator<Def>*acc)		const;					//!< Recursivly determines whether ot not the specified accelerator belongs to any of the primary children of the local geometry or any of their decendents \param acc Accelerator to look for \return true if the specified accelerator is a member of any of the children of the local geometry or their decendents, false otherwise.


			template <class C>
				bool									extractDimensions(M::Box<C>&dim)				const;				//!< Determines the visual dimensions of the local geometry. \param dim Dimension field to write the dimensions to \return true if the geometry is not empty and the dimensions could be extracted, false otherwise.
			template <class C>
				void									extractRadius(C&radius)					const;				//!< Extracts the visual radius (the distance between the geometry point of origin and the furthest sub geometry vertex) of the local geometry
			template <class C>
				void									extractAverageVisualEdgeLength(unsigned layer, C&length)	const;		//!< Determines the average visual edge length. \param layer Detail level to determine the average visual edge length of (0=highest detail) \param length Out scalar to store the average edge length in
			template <typename T>
				void									extractShortestVisualEdgeLength(index_t layer, T&length)	const;
			
			template <class C>
				C										extractRadiusAround(const M::TVec3<C>&center)	const;				//!< Determines the visual radius around a specific center
			template <class C>
				void									resolveWeight(C&weight)					const;				//!< Calculates the weight of the local geometry based on the volumes of the children and their respective material densities. The result of this method is undefined if the volumes were not properly calculated
				void									updateRadius();												//!< Updates the local visual radius variable
			template <class C>
				void									scale(const C&factor);								//!< Scales the entire geometry (materials, objects, systems) by the specified factor. Normals are left unchanged.
				void									swapComponents(BYTE c0, BYTE c1);					//!< Swaps the two specified coordinate components of the entire geometry (materials, objects, systems)
			template <class C>
				void									translate(const M::TVec3<C>&delta);						//!< Translates all local objects by the specified delta vector \param delta Vector to translate by
				void									checkLinkage();										//!< Checks linkage pointers and throws an exception in case of an error
				template <class Def0>
				Geometry<Def>&							operator=(const Geometry<Def0>&other);
				Geometry<Def>&							operator=(const Geometry<Def>&other);
			
			template <class Def0>
				void									copy(const Geometry<Def0>&other);

				void									adoptData(Geometry<Def>&other);
			
				//void									appendDetailLayer(Geometry<Def>&other);			//!< Appends the specified geometry as the next lower detail layer to the end of the detail layer list.
			
			
			
				void									verifyIntegrity();									//!< More thorough version of checkLinkage(). Verifies everything and throws an exception in case of an error
				bool									checkIntegrity();									//!< Identical to to verifyIntegrity() but does not throw an error but rather return the validity. Use errorStr() to retrieve an error representation. \return true if the local geometry is valid, false otherwise.
		static	const String&							errorStr();											//!< Retrieves an error representation of the last occured error
			
				count_t									countVertices(unsigned detail=0);					//!< Counts all vertices on the specified detail levle
				String									getLinkageString();
				String									ToString(bool details=false)			const;
				count_t									countVertices()							const;		//!< Counts all vertices on all detail levels
				count_t									countFaces()							const;		//!< Counts all faces (quads and triangles) on all detail levels
				count_t									countTriangles()						const;		//!< Counts all triangles on all detail levels
				count_t									countQuads()							const;		//!< Counts all quads on all detail levels
				count_t									countFaces(unsigned detail_layer)		const;		//!< Counts all faces on the specified detail layer
				count_t									countTriangles(unsigned detail_layer)	const;		//!< Counts all traingles on the specified detail layer
				count_t									countQuads(unsigned detail_layer)		const;		//!< Counts all quads on the specified detail layer
				unsigned								maxDetail()								const;	//!< Queries the maximum render object detail layer of this structure with the detail of all other render objects being equal or less than the returned value
			
				void									createVisualMaps(unsigned detail, unsigned max_depth=4);				//!< Updates visual normals and creates map objects for all visual objects of the specified detail (if not already created). Also generates visual normals \param detail Target detail layer (0=most detail) \param max_depth Maximum map recursion depth as passed on to Object<>::buildMap()
				void									walkChildren(void (*geometryCallback)(SubGeometryA<Def>*));					//!< Sequentially passes all objects (on any sub layer) to the specified callback function
				void									walkChildren(SubGeometryCallback<Def>*callback);							//!< Sequentially passes all objects (on any sub layer) to the specified callback handler
			
				void									clearFlags(UINT32 clear_mask=0xFFFFFFFF, bool geometries=true, bool wheels=true, bool accelerators=true, bool constructs=true);	//!< Recursivly clears the specified flags. @param clear_mask Mask to limit clearing of flags. Only bits in this value are cleared @param geometries Apply to sub geometries @param wheels Apply to wheels @param accelerators Apply to accelerators @param constructs Apply to constructs
				void									flagAnimationTargets(UINT32 flag=AnimationTargetFlag);	//!< Recursivly adds the specified flag to targeted sub geometries, wheels, and accelerators
		
				void									makeFromConstructor(const Constructor<Def>&);
				void									makeSimpleObject(const Float*vertex,count_t vertices,const Index*index,count_t indices);	//!< Clears any existing local data and generates a simple triangular visual geometry with one SubGeometry<Def> instance from the provided data \param vertex Float field. Each vertex is required to provide 6 floats, 3 for the position and 3 for the normal. \param vertices Total number of vertices \param index Index field. Each triangle requires three indices. \param indices Total number of indices (should be 3*number of triangles)
				void									makeSimpleObject(const Float*vertex,count_t vertices,count_t layers, const Index*index,count_t indices);	//!< Clears any existing local data and generates a simple triangular visual geometry with a number of pre-generated empty texture layers and one SubGeometry<Def> instance from the provided data \param vertex Float field. Each vertex is required to provide 3 elements for the position, 3 for the normal plus 2 for each layer. \param vertices Total number of vertices \param layers Number of (empty) texture layers that should be generated. \param index Index field. Each triangle requires three indices. \param indices Total number of indices (should be 3*number of triangles)
				void									makeSphere(Float radius, count_t iterations);		//!< Clears any existing data and generates a simple textureless sphere \param radius Sphere radius \param iterations Sphere resolution. The more iterations, the higher the detail. Usually 5 means rather low, 50 means very high and everything above overkill.
			template <typename T0>
				void									makeBox(const M::Box<T0>&);	//!< Clears any existing data and generates a simple textureless box \param lower_corner Lower corner of the box \param upper_corner Upper corner of the box
				void									makeBox(Float lower_x, Float lower_y, Float lower_z, Float upper_x, Float upper_y, Float upper_z);	//!< \overload
				void									makePlane(Float width, Float depth, count_t x_segments, count_t z_segments);
		//		String								difference(Geometry*other);
		};

			CGS_DECLARE_ADOPTING_DEF(Geometry);


	
		/*!
			\brief Instance (spawn) of a Geometry<Def> instance
		*/
		template <class Def=StdDef>
			class GeometryInstance:/*public Reference<TInstanceObject>,*/ public System<typename Def::SystemType>, public System<typename Def::SystemType>::Callback
			{
			public:
					void									onSystemChange()	{};
			public:
			typedef System<typename Def::SystemType>		Super;
					Geometry<Def>							*target;		//!< Target (origin) geometry
					using Super::matrix;
				
															GeometryInstance();
			virtual											~GeometryInstance()	{};


					void									adoptData(GeometryInstance<Def>&other)
															{
																target = other.target;
																matrix = other.matrix;
															}
			};
	
		/*!
			\brief Static (non-animated but movable) instance (spawn) of a Geometry<Def> instance
		
			A geometry instance defines an implemented instance of a geometry, referring to the same geometrical data but in different locations.
			A StaticInstance<Def> object technically copies the entire system tree of the original geometry and needs to update its path variables whenever the instance is moved.
		*/
		template <class Def=StdDef>
			class StaticInstance:public GeometryInstance<Def>
			{
			public:
					void									onSystemChange();
			public:
			typedef GeometryInstance<Def>					Super;
				
					Ctr::Array<StaticSubInstanceA<Def>,Adopt>	child_field;	//!< Primary children

															StaticInstance();
					void									update();			//!< Recursivly updates all path variables. Call this method whenever the system matrix has been changed
					void									link();					//!< Recursivly links the local system matrices to the respective system_link variables of the origin geometry, thus localizing it to the current status of this instance. Only one instance can be linked to the same geometry at any given time.
					void									restoreFromGeometry();	//!< Recursivly restores all system matrices from the respective origin geometry.

					void									adoptData(StaticInstance<Def>&other)
															{
																child_field.adoptData(other.child_field);
																Super::adoptData(other);
															}
				
				template <typename T0, typename T1, typename T2>
					bool									detectOpticalIntersection(const M::TVec3<T0>&b, const M::TVec3<T1>&d, T2&distance)	const;	//!< Detects optical intersections of the local instance and the specified ray. \param b Base vector of the casting ray \param d Normalized direction vector of the casting ray \param distance In/out distance to the nearest intersection \return true if the ray intersects the local geometry at the location of the instance closer than the specified distance, false otherwise.
			};
			CGS_DECLARE_ADOPTING_DEF(StaticInstance);


		template <class Def=StdDef>
			class SubGeometryInstance
			{
			public:
					SubGeometryA<Def>						*target;		//!< Target (origin) sub geometry instance
					M::TMatrix4<typename Def::SystemType>	system,		//!< Local system
															path;		//!< Local path (product of parent path and local system)
		
															SubGeometryInstance();
			virtual											~SubGeometryInstance()	{}

					M::TMatrix4<typename Def::SystemType>*	GetPath()	{return&path;}
			};
		
		template <class Def=StdDef>
			class StaticSubInstanceA:public SubGeometryInstance<Def>	//! StaticInstance<Def> child class
			{
				typedef SubGeometryInstance<Def>	Super;
			public:
					Ctr::Array<StaticSubInstanceA<Def>,Adopt>	child_field;	//!< Sub children
														
															StaticSubInstanceA();
					void									updatePath(const M::TMatrix4<typename Def::SystemType>&parent);	//!< Recursivly updates the local path variable using the specified parent system
					void									linkObjects();									//!< Recursivly links to the respective target sub geometry instance
					void									restoreFromGeometry();							//!< Recursivly restores all system matrices from the respective origin sub geometry instance.

					void									adoptData(StaticInstance<Def>&other)
															{
																child_field.adoptData(other.child_field);
															}


				template <typename T0, typename T1, typename T2>
					bool									detectOpticalIntersection(const M::TVec3<T0>&b, const M::TVec3<T1>&d, T2&distance)	const;	//!< Detects optical intersections of the local instance and the specified ray. \param b Base vector of the casting ray \param d Normalized direction vector of the casting ray \param distance In/out distance to the nearest intersection \return true if the ray intersects the local geometry at the location of the instance closer than the specified distance, false otherwise.
			};
			CGS_DECLARE_ADOPTING_DEF(StaticSubInstanceA);


	
	

	
		template <class Def=StdDef>
			struct TSubGeometryAnimator
			{
				SubGeometryA<Def>					*target;
				TraceA<SubGeometryA<Def>,6>			*target_trace;
				TAnimationStatus					current,
													*out_target;	//!< Target out vector (6 elements). @b current will be added to this vector on update
												
			};
	
		template <class Def=StdDef>
			struct TAcceleratorAnimator
			{
				TAccelerator<Def>					*target;
				TraceA<TAccelerator<Def>,1>			*target_trace;
				double								current_acceleration;
				double								*out_target;	//!< Target out acceleration float. @b current_acceleration will be added to this float on update
			};

		template <class Def=StdDef>
			struct TWheelAnimator
			{
				TWheel<Def>							*target;
				TraceA<TWheel<Def>,2>				*target_trace;
				union
				{
					double							current[2];
					struct
					{
						double						current_acceleration,
													current_brake_status;
					};
				};
				double								*out_target;	//!< Target out wheel vector (2 elements). @b current_acceleration and @b current_brake_status will be added to this vector on update
			};
	
	
		template <class Def=StdDef>
			class AnimatorInstanceA
			{
			public:
					AnimatorA<Def>						*target;
					double								progress;	//!< Current progress of this animation in seconds
					bool								active;
				
					Ctr::Array<TSubGeometryAnimator<Def> >	obj_animator_field;
					Ctr::Array<TAcceleratorAnimator<Def> >	acc_animator_field;
					Ctr::Array<TWheelAnimator<Def> >			whl_animator_field;
				
														AnimatorInstanceA();
					void								build(AnimatorA<Def>*target);
					void								link(const Container::PointerTable<double*>&map,const Container::PointerTable<TAnimationStatus*>&map2);
					void								update();
					void								adoptData(AnimatorInstanceA<Def>&other);
			};
			CGS_DECLARE_ADOPTING_DEF(AnimatorInstanceA);

	
	
		/*!
			\brief Animatable instance (spawn) of a Geometry<Def> instance
		
			A animatable geometry instance defines an implemented instance of a geometry, refering to the same geometrical data but in different locations.
			A AnimatableInstance<Def> object technically copies the entire system tree of the original geometry and needs to update its path variables whenever the instance is moved.
		*/
		template <class Def=StdDef>
			class AnimatableInstance:public GeometryInstance<Def>
			{
			public:
					void									onSystemChange();
			public:
		
			typedef GeometryInstance<Def>					Super;
					using Super::target;
				
					Ctr::Array<AnimatorInstanceA<Def>,Adopt>		animator_field;
				
					struct	TAnimationMap	//! Performance optimization map
					{
						Container::PreAllocVector<AnimatorInstanceA<Def>*>		sub_animator_field;				//!< Animators (wheel/construct) of sub geometries ordered limearily for faster access
						Container::PreAllocVector<AnimatableSubInstanceA<Def>*>	animated_sub_field;				//!< Linear list of all sub geometries that are affected by some animator for faster state reseting
						Container::PreAllocVector<double*>						animated_wheel_status_field,	//!< Linear list of all animated wheels' status vectors (two floats per entry) for faster reseting
															animated_accel_status_field;	//!< Linear list of all animated accelerators' status vectors (one float per entry) for faster reseting
					};
					TAnimationMap							animation_map;
				
					Ctr::Array<AnimatableSubInstanceA<Def>,Adopt>child_field;	//!< Primary children

															AnimatableInstance();
					void									update();				//!< Recursivly updates all path variables and animations. Call this method whenever the system matrix has been changed
					void									link();					//!< Recursivly links the local system matrices to the respective system_link variables of the origin geometry, thus localizing it to the current status of this instance. Only one instance can be linked to the same geometry at any given time.
					void									restoreFromGeometry(bool reset_animations);	//!< Recursivly restores all system matrices from the respective origin geometry. @param reset_animations Set true to also set all animations to progress 0
					AnimatorInstanceA<Def>*					findInstanceOf(AnimatorA<Def>*animator);		//!< Attempts to locate the animator instance of the specified animator @param animator Pointer to the animator to look for @return Pointer to the matching animator instance or NULL if no such could be found
					AnimatorInstanceA<Def>*					findAnimator(const String&name);					//!< Locates the first global animation to match the specified name or NULL if no such exists
				
					void									updateAnimationMap();	//!< Recursivly maps elements of animation_map based on the flags of their respective targets
					double*									findStatusOf(TWheel<Def>*wheel);	//!< Recursivly searches for the status field of the specified wheel @param wheel Wheel to look for @return Pointer to the 2 element status vector of the specified wheel or NULL if the wheel could not be found
					double*									findStatusOfWheel(const Ctr::ArrayData<index_t>&path);	//!< Locates the status of a wheel via its index path @param path Index path to the requested wheel. Each entry except for the last specifies the index of a sub geometry instance, the last specifies the index of the wheel in its parent sub geometry. @return status array or NULL if the path is invalid
					double*									findStatusOfAccelerator(const Ctr::ArrayData<index_t>&path);	//!< Locates the status of an accelerator via its index path @param path Index path to the requested wheel. Each entry except for the last specifies the index of a sub geometry instance, the last specifies the index of the accelerator in its parent sub geometry. @return status variable pointer or NULL if the path is invalid
					AnimatorInstanceA<Def>*					findRotationAnimatorOfWheel(const Ctr::ArrayData<index_t>&path);	//!< Locates the rotation animator of a wheel via its index path @param path Index path to the requested wheel. Each entry except for the last specifies the index of a sub geometry instance, the last specifies the index of the wheel in its parent sub geometry. @return Animator instance or NULL if the path is invalid
					AnimatorInstanceA<Def>*					findSuspensionAnimatorOfWheel(const Ctr::ArrayData<index_t>&path);	//!< Locates the suspension animator of a wheel via its index path @param path Index path to the requested wheel. Each entry except for the last specifies the index of a sub geometry instance, the last specifies the index of the wheel in its parent sub geometry. @return Animator instance or NULL if the path is invalid

					void									adoptData(AnimatableInstance<Def>&other)
															{
																Super::adoptData(other);
																animator_field.adoptData(other.animator_field);
																animation_map.sub_animator_field.adoptData(other.animation_map.sub_animator_field);
																animation_map.animated_sub_field.adoptData(other.animation_map.animated_sub_field);
																animation_map.animated_wheel_status_field.adoptData(other.animation_map.animated_wheel_status_field);
																child_field.adoptData(other.child_field);
															}

				
				
				template <typename T0, typename T1, typename T2>
					bool									detectOpticalIntersection(const M::TVec3<T0>&b, const M::TVec3<T1>&d, T2&distance)	const;	//!< Detects optical intersections of the local instance and the specified ray. \param b Base vector of the casting ray \param d Normalized direction vector of the casting ray \param distance In/out distance to the nearest intersection \return true if the ray intersects the local geometry at the location of the instance closer than the specified distance, false otherwise.
			};

		template <class Def=StdDef>
			class AnimatableSubInstanceA:public SubGeometryInstance<Def>	//! AnimatableInstance<Def> child class
			{
			public:
					typedef	SubGeometryInstance<Def>		Super;

					using Super::target;
					using Super::path;
					using Super::system;

					Ctr::Array<AnimatorInstanceA<Def>,Adopt>		wheel_rotation_animators,
															wheel_suspension_animators;
															//construct_rotation_animators;
					#ifdef __GNUC__
						using SubGeometryInstance<Def>::system;
						using SubGeometryInstance<Def>::path;
						using SubGeometryInstance<Def>::target;
					#endif
				
					Ctr::Array<AnimatableSubInstanceA<Def>,Adopt>	child_field;	//!< Sub children
					TAnimationStatus						animation_status;	//!< Animation status vector
					M::TMatrix4<typename Def::SystemType>	animated_system;	//!< Mutated local system
					Ctr::Array<double>						wheel_status,		//!< Contains two floats - one for acceleration, one for braking - per wheel
															accelerator_status;	//!< Contains one float per accelerator
				
															AnimatableSubInstanceA();

					void									adoptData(AnimatableSubInstanceA<Def>&other)
															{
																wheel_rotation_animators.adoptData(other.wheel_rotation_animators);
																wheel_suspension_animators.adoptData(other.wheel_suspension_animators);
																//construct_rotation_animators.adoptData(other.construct_rotation_animators);
																child_field.adoptData(other.child_field);
																animation_status = other.animation_status;
																animated_system = other.animated_system;
																wheel_status.adoptData(other.wheel_status);
																accelerator_status.adoptData(other.accelerator_status);

																other.animation_status.translation = M::Vector3<double>::zero;
																other.animation_status.rotation = M::Vector3<double>::zero;
																other.animated_system = M::Matrix<typename Def::SystemType>::eye4;
															}
					void									updatePath(const M::TMatrix4<typename Def::SystemType>&parent);	//!< Recursivly updates the local path variable using the specified parent system
					void									linkObjects();									//!< Recursivly links to the respective target sub geometry instance
					void									restoreFromGeometry(bool reset_animations);		//!< Recursivly restores all system matrices from the respective origin sub geometry instance.
					void									mapEntities(PointerTable<double*>&, PointerTable<TAnimationStatus*>&);			//!< Recursivly maps instances (sub geometries, wheels, accelerators) to their respecitive local status variables
					void									linkEntities(const PointerTable<double*>&, const PointerTable<TAnimationStatus*>&);	//!< Recursivly links animator traces (sub geometry, wheel, accelerator) to their respecitive status variables
					void									resetAnimationStatus();							//!< Recursivly resets all animation status variables to 0
					void									updateAnimations();								//!< Recursivly updates all animations
					AnimatorInstanceA<Def>*					findInstanceOf(AnimatorA<Def>*animator);		//!< Attempts to recursivly locate the animator instance of the specified animator @param animator Pointer to the animator to look for @return Pointer to the matching animator instance or NULL if no such could be found
					void									fillAnimationMap(typename AnimatableInstance<Def>::TAnimationMap&map);	//!< Recursivly maps elements of animation_map based on the flags of their respective targets
				template <typename T0, typename T1, typename T2>
					bool									detectOpticalIntersection(const M::TVec3<T0>&b, const M::TVec3<T1>&d, T2&distance)	const;	//!< Detects optical intersections of the local instance and the specified ray. \param b Base vector of the casting ray \param d Normalized direction vector of the casting ray \param distance In/out distance to the nearest intersection \return true if the ray intersects the local geometry at the location of the instance closer than the specified distance, false otherwise.
					double*									findStatusOf(TWheel<Def>*wheel);	//!< Recursivly searches for the status field of the specified wheel @param wheel Wheel to look for @return Pointer to the 2 element status vector of the specified wheel or NULL if the wheel could not be found				
			};
			CGS_DECLARE_ADOPTING_DEF(AnimatableSubInstanceA);


		typedef StaticInstance<StdDef>	StdInstance;


		/**
		@brief Geometry constructor helper class

		Simplifies dynamic geometry construction
		*/
		template <class Def=StdDef>
			class Constructor
			{
			public:
				typedef typename Def::FloatType	Float;
				typedef typename Def::IndexType	Index;
				typedef typename Def::SystemType SysFloat;

				struct VConfig
				{
					count_t				vsize;
					count_t				numTextureLayers;
					UINT				vertexFlags;
				};

				class Object
				{
				public:
					struct LOD
					{
						PreAllocVector<Float,0>	vertexData;
						PreAllocVector<Index,0>	triangleIndices,
										quadIndices;
						void			swap(LOD&other)
										{
											vertexData.swap(other.vertexData);
											triangleIndices.swap(other.triangleIndices);
											quadIndices.swap(other.quadIndices);
										}
						void			Clear()	{vertexData.clear();triangleIndices.clear();quadIndices.clear();}
					};
				private:
					friend class Constructor<Def>;

					PreAllocVector<LOD,2,Swap>	lods;
					index_t				normalsFromVertex,
										normalsFromTriangle,
										normalsFromQuad;
					Index				voffset;
					VConfig				config;
					M::TMatrix4<SysFloat>system;
					LOD					*currentLOD;

					void				_OnActiveLODChange()
										{
											voffset = 0;
											normalsFromTriangle = 0;
											normalsFromQuad = 0;
											normalsFromVertex = 0;
										}
				public:
					/**/				Object():voffset(0),normalsFromVertex(0),normalsFromTriangle(0),normalsFromQuad(0), system(M::Matrix<typename Def::SystemType>::eye4)	{currentLOD = &lods.Append();}
					void				Clear()
										{
											lods.Truncate(1);
											lods.Last().Clear();
											currentLOD = lods.pointer();
											_OnActiveLODChange();
										}
					void				ClearCompletely()
										{
											lods.Clear();
											currentLOD = NULL;
											_OnActiveLODChange();
										}
					void				OverrideSetConfig(const VConfig&cfg)
										{
											config = cfg;
										}
					void				swap(Object&other)
										{
											lods.swap(other.lods);
											swp(currentLOD,other.currentLOD);
											swp(config,other.config);
											swp(system,other.system);
											swp(voffset,other.voffset);
											swp(normalsFromVertex,other.normalsFromVertex);
											swp(normalsFromTriangle,other.normalsFromTriangle);
											swp(normalsFromQuad,other.normalsFromQuad);
										}
					void				adoptData(Object&other)
										{
											lods.adoptData(other.lods);
											currentLOD = other.currentLOD;
											other.currentLOD = NULL;
											config = other.config;
											system = other.system;
											voffset = other.voffset;
											normalsFromVertex = other.normalsFromVertex;
											normalsFromTriangle = other.normalsFromTriangle;
											normalsFromQuad = other.normalsFromQuad;
										}
					void				AppendAndActivateNewLOD()
										{
											index_t result = lods.Count();
											currentLOD = &lods.Append();
											_OnActiveLODChange();
										}
					/**
					Retrieves the address of the first float of the specified vertex in the current LOD
					@param at Vertex index, relative to the current vertex offset
					@return Pointer to the first float of the specified vertex in the current LOD
					*/
					const float*		GetVertex(index_t at) const
					{
						at += voffset;
						return currentLOD->vertexData + at * config.vsize;
					}
					count_t				CountLODs()	const	{return lods.Count();}
					void				SetActiveLOD(index_t lodIndex)	{ASSERT_LESS__(lodIndex,lods.Count()); currentLOD = lods + lodIndex;_OnActiveLODChange();}
					index_t				GetActiveLOD() const	{return currentLOD - lods.pointer();}
					const LOD&			GetLOD(index_t lodIndex) const { return lods[lodIndex]; }
					template <typename T>
						void			SetSystem(const M::TMatrix4<T>&m){M::Mat::copy(m,system);}
					const M::TMatrix4<SysFloat>& GetSystem()	const {return system;}
					count_t				CountTextureLayers()	const	{return config.numTextureLayers;}
					UINT				GetVertexFlags()		const	{return config.vertexFlags;}
					count_t				GetVertexSize()			const	{return config.vsize;}
					void				SetVertexOffset(Index offset)	{voffset = offset;}
					void				SetVertexOffsetToCurrent()		{voffset = (Index)(currentLOD->vertexData.GetLength() / config.vsize);}
					void				SetComputeNormalsBegin();
					void				SetGenerateNormalsBegin()		{SetComputeNormalsBegin();}
					void				ComputeNormals();
					void				GenerateNormals()				{ComputeNormals();}
					template <typename T>
						Object&			MakeTriangle(T v0, T v1, T v2)
										{
											Index*t = currentLOD->triangleIndices.appendRow(3);
											t[0] = (Index)(v0 + voffset);
											t[1] = (Index)(v1 + voffset);
											t[2] = (Index)(v2 + voffset);
											return *this;
										}
					template <typename T>
						Object&			MakeQuad(T v0, T v1, T v2, T v3)
										{
											Index*t = currentLOD->quadIndices.appendRow(4);
											t[0] = (Index)(v0 + voffset);
											t[1] = (Index)(v1 + voffset);
											t[2] = (Index)(v2 + voffset);
											t[3] = (Index)(v3 + voffset);
											return *this;
										}
					template <typename T>
						Object&			MakeTriangleInv(T v2, T v1, T v0)
										{
											Index*t = currentLOD->triangleIndices.appendRow(3);
											t[0] = (Index)(v0 + voffset);
											t[1] = (Index)(v1 + voffset);
											t[2] = (Index)(v2 + voffset);
											return *this;
										}
					template <typename T>
						Object&			MakeQuadInv(T v1, T v0, T v3, T v2)
										{
											Index*t = currentLOD->quadIndices.appendRow(4);
											t[0] = (Index)(v0 + voffset);
											t[1] = (Index)(v1 + voffset);
											t[2] = (Index)(v2 + voffset);
											t[3] = (Index)(v3 + voffset);
											return *this;
										}
					const Index*		GetTriangleIndices(index_t lod)	const {return lods[lod].triangleIndices.pointer();}
					const Index*		GetQuadIndices(index_t lod)		const {return lods[lod].quadIndices.pointer();}
					const Float*		GetVertices(index_t lod)			const {return lods[lod].vertexData.pointer();}
					count_t				CountTriangleIndices(index_t lod)	const {return lods[lod].triangleIndices.GetLength();}
					count_t				CountQuadIndices(index_t lod)		const {return lods[lod].quadIndices.GetLength();}
					count_t				CountTriangles(index_t lod)	const {return CountTriangleIndices(lod)/3;}
					count_t				CountQuads(index_t lod)		const {return CountQuadIndices(lod)/4;}
					count_t				CountFloats(index_t lod)	const {return lods[lod].vertexData.GetLength();}
					count_t				CountVertices(index_t lod)	const {return CountFloats(lod) / config.vsize;}
					M::Box<Float>		GetBoundingBox(index_t lod=0)const;

					void				VerifyIntegrity(bool verifyAllVerticesAreUsed)	const;

					template <typename T>
						Index			MakeVertex(const M::TVec3<T>&location)
						{
							Index result = (Index)(currentLOD->vertexData.GetLength() / config.vsize);
							Float*out = currentLOD->vertexData.appendRow(config.vsize);
							Float*end = out + config.vsize;
							M::Vec::copy(location,M::Vec::ref3(out)); out+=3;
							for (;out != end; ++out)
								(*out) = Float(0);
							return result;
						}
					template <typename T>
						Index			MakeVertex(const M::TVec3<T>&location, const M::TVec2<T>&texcoords0)
						{
							Index result = (Index)(currentLOD->vertexData.GetLength() / config.vsize);
							Float*out = currentLOD->vertexData.appendRow(config.vsize);
							Float*end = out + config.vsize;
							M::Vec::copy(location,M::Vec::ref3(out)); out+=3;
							if (config.vertexFlags&HasNormalFlag)
							{
								M::Vec::clear(M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasTangentFlag)
							{
								M::Vec::clear(M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasColorFlag)
							{
								M::Vec::clear(M::Vec::ref4(out)); out+=4;
							}
							if (config.numTextureLayers > 0)
							{
								M::Vec::copy(texcoords0,M::Vec::ref2(out)); out+=2;
							}
							for (;out != end; ++out)
								(*out) = Float(0);
							return result;
						}
					template <typename T>
						Index			MakeVertex(const M::TVec3<T>&location, const M::TVec2<T>&texcoords0, const M::TVec2<T>&texcoords1)
						{
							Index result = (Index)(currentLOD->vertexData.GetLength() / config.vsize);
							Float*out = currentLOD->vertexData.appendRow(config.vsize);
							Float*end = out + config.vsize;
							M::Vec::copy(location,M::Vec::ref3(out)); out+=3;
							if (config.vertexFlags&HasNormalFlag)
							{
								M::Vec::clear(M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasTangentFlag)
							{
								M::Vec::clear(M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasColorFlag)
							{
								M::Vec::clear(M::Vec::ref4(out)); out+=4;
							}
							if (config.numTextureLayers > 0)
							{
								M::Vec::copy(texcoords0,M::Vec::ref2(out)); out+=2;
							}
							if (config.numTextureLayers > 1)
							{
								M::Vec::copy(texcoords1,M::Vec::ref2(out)); out+=2;
							}
							for (;out != end; ++out)
								(*out) = Float(0);
							return result;
						}
					template <typename T>
						Index			MakeVertex(const M::TVec3<T>&location, const M::TVec2<T>&texcoords0, const M::TVec2<T>&texcoords1, const M::TVec2<T>&texcoords2)
						{
							Index result = (Index)(currentLOD->vertexData.GetLength() / config.vsize);
							Float*out = currentLOD->vertexData.appendRow(config.vsize);
							Float*end = out + config.vsize;
							M::Vec::copy(location,M::Vec::ref3(out)); out+=3;
							if (config.vertexFlags&HasNormalFlag)
							{
								M::Vec::clear(M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasTangentFlag)
							{
								M::Vec::clear(M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasColorFlag)
							{
								M::Vec::clear(M::Vec::ref4(out)); out+=4;
							}
							if (config.numTextureLayers > 0)
							{
								M::Vec::copy(texcoords0,M::Vec::ref2(out)); out+=2;
							}
							if (config.numTextureLayers > 1)
							{
								M::Vec::copy(texcoords1,M::Vec::ref2(out)); out+=2;
							}
							if (config.numTextureLayers > 2)
							{
								M::Vec::copy(texcoords2,M::Vec::ref2(out)); out+=2;
							}
							for (;out != end; ++out)
								(*out) = Float(0);
							return result;
						}


					template <typename T>
						Index			MakeVertex(const M::TVec3<T>&location, const M::TVec3<T>&normal)
						{
							Index result = (Index)(currentLOD->vertexData.GetLength() / config.vsize);
							Float*out = currentLOD->vertexData.appendRow(config.vsize);
							Float*end = out + config.vsize;
							M::Vec::copy(location,M::Vec::ref3(out)); out+=3;
							if (config.vertexFlags&HasNormalFlag)
							{
								M::Vec::copy(normal,M::Vec::ref3(out)); out+=3;
							}
							for (;out != end; ++out)
								(*out) = Float(0);
							return result;
						}
					template <typename T>
						Index		MakeVertex(const M::TVec3<T>&location, const M::TVec3<T>&normal, const M::TVec2<T>&texcoords0)
						{
							Index result = (Index)(currentLOD->vertexData.GetLength() / config.vsize);
							Float*out = currentLOD->vertexData.appendRow(config.vsize);
							Float*end = out + config.vsize;
							M::Vec::copy(location,M::Vec::ref3(out)); out+=3;
							if (config.vertexFlags&HasNormalFlag)
							{
								M::Vec::copy(normal,M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasTangentFlag)
							{
								M::Vec::clear(M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasColorFlag)
							{
								M::Vec::clear(M::Vec::ref4(out)); out+=4;
							}
							if (config.numTextureLayers > 0)
							{
								M::Vec::copy(texcoords0,M::Vec::ref2(out)); out+=2;
							}
							for (;out != end; ++out)
								(*out) = Float(0);
							return result;
						}
					template <typename T>
						Index			MakeVertex(const M::TVec3<T>&location, const M::TVec3<T>&normal, const M::TVec2<T>&texcoords0, const M::TVec2<T>&texcoords1)
						{
							Index result = (Index)(currentLOD->vertexData.GetLength() / config.vsize);
							Float*out = currentLOD->vertexData.appendRow(config.vsize);
							Float*end = out + config.vsize;
							M::Vec::copy(location,M::Vec::ref3(out)); out+=3;
							if (config.vertexFlags&HasNormalFlag)
							{
								M::Vec::copy(normal,M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasTangentFlag)
							{
								M::Vec::clear(M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasColorFlag)
							{
								M::Vec::clear(M::Vec::ref4(out)); out+=4;
							}
							if (config.numTextureLayers > 0)
							{
								M::Vec::copy(texcoords0,M::Vec::ref2(out)); out+=2;
							}
							if (config.numTextureLayers > 1)
							{
								M::Vec::copy(texcoords1,M::Vec::ref2(out)); out+=2;
							}
							for (;out != end; ++out)
								(*out) = Float(0);
							return result;
						}
					template <typename T>
						Index			MakeVertex(const M::TVec3<T>&location, const M::TVec3<T>&normal, const M::TVec2<T>&texcoords0, const M::TVec2<T>&texcoords1, const M::TVec2<T>&texcoords2)
						{
							Index result = (Index)(currentLOD->vertexData.GetLength() / config.vsize);
							Float*out = currentLOD->vertexData.appendRow(config.vsize);
							Float*end = out + config.vsize;
							M::Vec::copy(location,M::Vec::ref3(out)); out+=3;
							if (config.vertexFlags&HasNormalFlag)
							{
								M::Vec::copy(normal,M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasTangentFlag)
							{
								M::Vec::clear(M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasColorFlag)
							{
								M::Vec::clear(M::Vec::ref4(out)); out+=4;
							}
							if (config.numTextureLayers > 0)
							{
								M::Vec::copy(texcoords0,M::Vec::ref2(out)); out+=2;
							}
							if (config.numTextureLayers > 1)
							{
								M::Vec::copy(texcoords1,M::Vec::ref2(out)); out+=2;
							}
							if (config.numTextureLayers > 2)
							{
								M::Vec::copy(texcoords2,M::Vec::ref2(out)); out+=2;
							}
							for (;out != end; ++out)
								(*out) = Float(0);
							return result;
						}








					template <typename T>
						Index		MakeVertex(const M::TVec3<T>&location, const M::TVec4<T>&color)
						{
							Index result = (Index)(currentLOD->vertexData.GetLength() / config.vsize);
							Float*out = currentLOD->vertexData.appendRow(config.vsize);
							Float*end = out + config.vsize;
							M::Vec::copy(location,M::Vec::ref3(out)); out+=3;
							if (config.vertexFlags&HasNormalFlag)
							{
								M::Vec::clear(M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasTangentFlag)
							{
								M::Vec::clear(M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasColorFlag)
							{
								M::Vec::copy(color,M::Vec::ref4(out)); out+=4;
							}
							for (;out != end; ++out)
								(*out) = Float(0);
							return result;
						}
					template <typename T>
						Index			MakeVertex(const M::TVec3<T>&location, const M::TVec4<T>&color, const M::TVec2<T>&texcoords0)
						{
							Index result = (Index)(currentLOD->vertexData.GetLength() / config.vsize);
							Float*out = currentLOD->vertexData.appendRow(config.vsize);
							Float*end = out + config.vsize;
							M::Vec::copy(location,M::Vec::ref3(out)); out+=3;
							if (config.vertexFlags&HasNormalFlag)
							{
								M::Vec::clear(M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasTangentFlag)
							{
								M::Vec::clear(M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasColorFlag)
							{
								M::Vec::copy(color,M::Vec::ref4(out)); out+=4;
							}
							if (config.numTextureLayers > 0)
							{
								M::Vec::copy(texcoords0,M::Vec::ref2(out)); out+=2;
							}
							for (;out != end; ++out)
								(*out) = Float(0);
							return result;
						}
					template <typename T>
						Index			MakeVertex(const M::TVec3<T>&location, const M::TVec4<T>&color, const M::TVec2<T>&texcoords0, const M::TVec2<T>&texcoords1)
						{
							Index result = (Index)(currentLOD->vertexData.GetLength() / config.vsize);
							Float*out = currentLOD->vertexData.appendRow(config.vsize);
							Float*end = out + config.vsize;
							M::Vec::copy(location,M::Vec::ref3(out)); out+=3;
							if (config.vertexFlags&HasNormalFlag)
							{
								M::Vec::clear(M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasTangentFlag)
							{
								M::Vec::clear(M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasColorFlag)
							{
								M::Vec::copy(color,M::Vec::ref4(out)); out+=4;
							}
							if (config.numTextureLayers > 0)
							{
								M::Vec::copy(texcoords0,M::Vec::ref2(out)); out+=2;
							}
							if (config.numTextureLayers > 1)
							{
								M::Vec::copy(texcoords1,M::Vec::ref2(out)); out+=2;
							}
							for (;out != end; ++out)
								(*out) = Float(0);
							return result;
						}
					template <typename T>
						Index			MakeVertex(const M::TVec3<T>&location, const M::TVec4<T>&color, const M::TVec2<T>&texcoords0, const M::TVec2<T>&texcoords1, const M::TVec2<T>&texcoords2)
						{
							Index result = (Index)(currentLOD->vertexData.GetLength() / config.vsize);
							Float*out = currentLOD->vertexData.appendRow(config.vsize);
							Float*end = out + config.vsize;
							M::Vec::copy(location,M::Vec::ref3(out)); out+=3;
							if (config.vertexFlags&HasNormalFlag)
							{
								M::Vec::clear(M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasTangentFlag)
							{
								M::Vec::clear(M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasColorFlag)
							{
								M::Vec::copy(color,M::Vec::ref4(out)); out+=4;
							}
							if (config.numTextureLayers > 0)
							{
								M::Vec::copy(texcoords0,M::Vec::ref2(out)); out+=2;
							}
							if (config.numTextureLayers > 1)
							{
								M::Vec::copy(texcoords1,M::Vec::ref2(out)); out+=2;
							}
							if (config.numTextureLayers > 2)
							{
								M::Vec::copy(texcoords2,M::Vec::ref2(out)); out+=2;
							}
							for (;out != end; ++out)
								(*out) = Float(0);
							return result;
						}


					template <typename T>
						Index			MakeVertex(const M::TVec3<T>&location, const M::TVec3<T>&normal, const M::TVec4<T>&color)
						{
							Index result = (Index)(currentLOD->vertexData.GetLength() / config.vsize);
							Float*out = currentLOD->vertexData.appendRow(config.vsize);
							Float*end = out + config.vsize;
							M::Vec::copy(location,M::Vec::ref3(out)); out+=3;
							if (config.vertexFlags&HasNormalFlag)
							{
								M::Vec::copy(normal,M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasTangentFlag)
							{
								M::Vec::clear(M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasColorFlag)
							{
								M::Vec::copy(color,M::Vec::ref4(out)); out+=4;
							}
							for (;out != end; ++out)
								(*out) = Float(0);
							return result;
						}
					template <typename T>
						Index			MakeVertex(const M::TVec3<T>&location, const M::TVec3<T>&normal, const M::TVec4<T>&color, const M::TVec2<T>&texcoords0)
						{
							Index result = (Index)(currentLOD->vertexData.GetLength() / config.vsize);
							Float*out = currentLOD->vertexData.appendRow(config.vsize);
							Float*end = out + config.vsize;
							M::Vec::copy(location,M::Vec::ref3(out)); out+=3;
							if (config.vertexFlags&HasNormalFlag)
							{
								M::Vec::copy(normal,M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasTangentFlag)
							{
								M::Vec::clear(M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasColorFlag)
							{
								M::Vec::copy(color,M::Vec::ref4(out)); out+=4;
							}
							if (config.numTextureLayers > 0)
							{
								M::Vec::copy(texcoords0,M::Vec::ref2(out)); out+=2;
							}
							for (;out != end; ++out)
								(*out) = Float(0);
							return result;
						}
					template <typename T>
						Index			MakeVertex(const M::TVec3<T>&location, const M::TVec3<T>&normal, const M::TVec4<T>&color, const M::TVec2<T>&texcoords0, const M::TVec2<T>&texcoords1)
						{
							Index result = (Index)(currentLOD->vertexData.GetLength() / config.vsize);
							Float*out = currentLOD->vertexData.appendRow(config.vsize);
							Float*end = out + config.vsize;
							M::Vec::copy(location,M::Vec::ref3(out)); out+=3;
							if (config.vertexFlags&HasNormalFlag)
							{
								M::Vec::copy(normal,M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasTangentFlag)
							{
								M::Vec::clear(M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasColorFlag)
							{
								M::Vec::copy(color,M::Vec::ref4(out)); out+=4;
							}
							if (config.numTextureLayers > 0)
							{
								M::Vec::copy(texcoords0,M::Vec::ref2(out)); out+=2;
							}
							if (config.numTextureLayers > 1)
							{
								M::Vec::copy(texcoords1,M::Vec::ref2(out)); out+=2;
							}
							for (;out != end; ++out)
								(*out) = Float(0);
							return result;
						}
					template <typename T>
						Index			MakeVertex(const M::TVec3<T>&location, const M::TVec3<T>&normal, const M::TVec4<T>&color, const M::TVec2<T>&texcoords0, const M::TVec2<T>&texcoords1, const M::TVec2<T>&texcoords2)
						{
							Index result = (Index)(currentLOD->vertex_data.GetLength() / config.vsize);
							Float*out = currentLOD->vertex_data.appendRow(config.vsize);
							Float*end = out + config.vsize;
							M::Vec::copy(location,M::Vec::ref3(out)); out+=3;
							if (config.vertexFlags&HasNormalFlag)
							{
								M::Vec::copy(normal,M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasTangentFlag)
							{
								M::Vec::clear(M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasColorFlag)
							{
								M::Vec::copy(color,M::Vec::ref4(out)); out+=4;
							}
							if (config.numTextureLayers > 0)
							{
								M::Vec::copy(texcoords0,M::Vec::ref2(out)); out+=2;
							}
							if (config.numTextureLayers > 1)
							{
								M::Vec::copy(texcoords1,M::Vec::ref2(out)); out+=2;
							}
							if (config.numTextureLayers > 2)
							{
								M::Vec::copy(texcoords2,M::Vec::ref2(out)); out+=2;
							}
							for (;out != end; ++out)
								(*out) = Float(0);
							return result;
						}



					template <typename T>
						Index			MakeVertex(const M::TVec3<T>&location, const M::TVec3<T>&normal, const M::TVec3<T>&tangent)
						{
							Index result = (Index)(currentLOD->vertexData.GetLength() / config.vsize);
							Float*out = currentLOD->vertexData.appendRow(config.vsize);
							Float*end = out + config.vsize;
							M::Vec::copy(location,M::Vec::ref3(out)); out+=3;
							if (config.vertexFlags&HasNormalFlag)
							{
								M::Vec::copy(normal,M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasTangentFlag)
							{
								M::Vec::copy(tangent,M::Vec::ref3(out)); out+=3;
							}
							for (;out != end; ++out)
								(*out) = Float(0);
							return result;
						}
					template <typename T>
						Index			MakeVertex(const M::TVec3<T>&location, const M::TVec3<T>&normal, const M::TVec3<T>&tangent, const M::TVec2<T>&texcoords0)
						{
							Index result = (Index)(currentLOD->vertexData.GetLength() / config.vsize);
							Float*out = currentLOD->vertexData.appendRow(config.vsize);
							Float*end = out + config.vsize;
							M::Vec::copy(location,M::Vec::ref3(out)); out+=3;
							if (config.vertexFlags&HasNormalFlag)
							{
								M::Vec::copy(normal,M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasTangentFlag)
							{
								M::Vec::copy(tangent,M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasColorFlag)
							{
								M::Vec::clear(M::Vec::ref4(out)); out+=4;
							}
							if (config.numTextureLayers > 0)
							{
								M::Vec::copy(texcoords0,M::Vec::ref2(out)); out+=2;
							}
							for (;out != end; ++out)
								(*out) = Float(0);
							return result;
						}
					template <typename T>
						Index			MakeVertex(const M::TVec3<T>&location, const M::TVec3<T>&normal, const M::TVec3<T>&tangent, const M::TVec2<T>&texcoords0, const M::TVec2<T>&texcoords1)
						{
							Index result = (Index)(currentLOD->vertexData.GetLength() / config.vsize);
							Float*out = currentLOD->vertexData.appendRow(config.vsize);
							Float*end = out + config.vsize;
							M::Vec::copy(location,M::Vec::ref3(out)); out+=3;
							if (config.vertexFlags&HasNormalFlag)
							{
								M::Vec::copy(normal,M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasTangentFlag)
							{
								M::Vec::copy(tangent,M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasColorFlag)
							{
								M::Vec::clear(M::Vec::ref4(out)); out+=4;
							}
							if (config.numTextureLayers > 0)
							{
								M::Vec::copy(texcoords0,M::Vec::ref2(out)); out+=2;
							}
							if (config.numTextureLayers > 1)
							{
								M::Vec::copy(texcoords1,M::Vec::ref2(out)); out+=2;
							}
							for (;out != end; ++out)
								(*out) = Float(0);
							return result;
						}
					template <typename T>
						Index			MakeVertex(const M::TVec3<T>&location, const M::TVec3<T>&normal, const M::TVec3<T>&tangent, const M::TVec2<T>&texcoords0, const M::TVec2<T>&texcoords1, const M::TVec2<T>&texcoords2)
						{
							Index result = (Index)(currentLOD->vertexData.GetLength() / config.vsize);
							Float*out = currentLOD->vertexData.appendRow(config.vsize);
							Float*end = out + config.vsize;
							M::Vec::copy(location,M::Vec::ref3(out)); out+=3;
							if (config.vertexFlags&HasNormalFlag)
							{
								M::Vec::copy(normal,M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasTangentFlag)
							{
								M::Vec::copy(tangent,M::Vec::ref3(out)); out+=3;
							}
							if (config.vertexFlags&HasColorFlag)
							{
								M::Vec::clear(M::Vec::ref4(out)); out+=4;
							}
							if (config.numTextureLayers > 0)
							{
								M::Vec::copy(texcoords0,M::Vec::ref2(out)); out+=2;
							}
							if (config.numTextureLayers > 1)
							{
								M::Vec::copy(texcoords1,M::Vec::ref2(out)); out+=2;
							}
							if (config.numTextureLayers > 2)
							{
								M::Vec::copy(texcoords2,M::Vec::ref2(out)); out+=2;
							}
							for (;out != end; ++out)
								(*out) = Float(0);
							return result;
						}
				};

			private:
				VConfig			config;
				PreAllocVector<Object,1,Adopt>	objects;
			public:
				MaterialColors	colors;


				/**/			Constructor(count_t num_layers=0, UINT vertexFlags=0)
								{
									config.numTextureLayers = num_layers;
									config.vertexFlags = vertexFlags;
									config.vsize = VSIZE(num_layers,vertexFlags);
								}
				void			swap(Constructor<Def>&other)
								{
									objects.swap(other.objects);
									swp(config,other.config);
								}
				void			adoptData(Constructor<Def>&other)
								{
									objects.adoptData(other.objects);
									config = other.config;
								}
				count_t			CountTextureLayers()	const	{return config.numTextureLayers;}
				UINT			GetVertexFlags()		const	{return config.vertexFlags;}
				count_t			GetVertexSize()			const	{return config.vsize;}
				count_t			CountObjects()			const	{return objects.Count();}
				count_t			CountLODs()				const;
				#undef GetObject
				Object&			GetObject(index_t index)		{return objects[index];}
				const Object&	GetObject(index_t index)const	{return objects[index];}
				Object&			AppendObject()	{Object&result = objects.Append(); result.config = config; return result;}
				void			Clear()			{objects.reset();}
				void			VerifyIntegrity(bool verifyAllVerticesAreUsed)	const;
			};
		CGS_DECLARE_ADOPTING_T(Constructor)


		#include "cgs.tpl.h"

	}
}

#endif

