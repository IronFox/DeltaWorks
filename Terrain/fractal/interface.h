#ifndef fractal_interfaceH
#define fractal_interfaceH

/*
	This module implements the general fractal interface: bodies, segments, index patterns, aspects, seeds, geometries, and hashing functions
*/



#include <math/vclasses.h>
#include <math/object.h>
#include <math/object_def.h>
#include <general/timer.h>
#include <general/system.h>
#include <general/thread_system.h>
#include <general/random.h>
#include <container/production_pipe.h>
#include <container/sorter.h>
#include <container/vectorList.h>
#include <io/mutex_log.h>
#include <io/delta_crc32.h>
#include <general/auto.h>
#include <engine/renderer/opengl.h>
#include <geometry/coordinates.h>
#include <io/log_stream.h>
//#include <io/mutex_log.h>

#include <engine/aspect.h>

#include "fractal_kernel.h"

#define FRACTAL_NURBS_CONST		0.35

//#define HYBRID_KERNEL

//MAKE_SORT_CLASS(PrioritySort,priority)
//MAKE_SORT_CLASS(DepthSort,depth)
//MAKE_SORT_CLASS(RadiusSort,radius)


#define DEBUG_POINT(_DOMAIN_)	//CONSOLE_DEBUG_POINT

class PrioritySort
{
public:
    template <class Type>
		static inline bool greaterObject(const Type*obj0, const Type*obj1)
		{
			return (((*obj0)->priority) > ((*obj1)->priority));
		}
    template <class Type, class Ident>
		static  inline bool greater(const Type*obj, const Ident&id)
		{
			return ((*obj)->priority) > (id);
		}
    template <class Type>
		static inline bool lesserObject(const Type*obj0, const Type*obj1)
		{
			return (((*obj0)->priority) < ((*obj1)->priority));
		}
    template <class Type, class Ident>
		static  inline bool less(const Type*obj, const Ident&id)\
		{
			return ((*obj)->priority) < (id);
		}
};

class DistanceSquareSort
{
public:
    template <class Type>
		static inline bool greaterObject(const Type*obj0, const Type*obj1)
		{
			return (((*obj0)->distance_square) > ((*obj1)->distance_square));
		}
    template <class Type, class Ident>
		static  inline bool greater(const Type*obj, const Ident&id)
		{
			return ((*obj)->distance_square) > (id);
		}
    template <class Type>
		static inline bool lesserObject(const Type*obj0, const Type*obj1)
		{
			return (((*obj0)->distance_square) < ((*obj1)->distance_square));
		}
    template <class Type, class Ident>
		static  inline bool less(const Type*obj, const Ident&id)\
		{
			return ((*obj)->distance_square) < (id);
		}
};

//#define VERTEX_DEFINITION






/* warn-options and settings for fractal modules and derivatives */


//#define WARN_BUFFER_LIMIT_EXCEEDED	{}//{ErrMessage("limit reached!");}
//#define FRAC_NO_PIPES					//erases and inserts sectors immediately bypassing pipes
//#define FRAC_DISABLE_PATTERNS			//indicate that derivative modules should not use dynamic patterns
//#define FRAC_DISABLE_TTL				//do not check for timeouts
//#define FRAC_RANDOM_STATISTICS		//show trace random-balance (use resetStatistics() and getStatistics())
//#define FRAC_USE_THREADS				//enable thread-usage
//#define FRAC_MIN_FRAME_AGE		2	//2


/*#define USE_KERNEL			1
#define TRACE_LINEAR_PAST3	1		//not use kernel from trace3...trace5 (only effective if USE_KERNEL is not 0)
#define PROJECT_ON_KERNEL	0
#define PRESERVE_PARENT_NORMALS	0	//set 1 to not regenerate parent normals on child layers*/



/*#define BEGIN
#define END
#define LBEGIN*/

#ifndef BEGIN
	#define BEGIN	//logfile<<"entering: "+String(__func__)+"\n";
#endif
#ifndef END
	#define END		//logfile<<"exiting: "+String(__func__)+"\n";
#endif

#ifndef LBEGIN
	#define LBEGIN	//logfile<<"begin: "+String(__func__)+"\n";
#endif

#define FRACTAL_BUILD_STATISTIC

/**
	@brief Fractal landscape generator
*/
namespace DeltaWorks::Fractal
{
	extern SynchronizedLogFile		logfile;
	
	extern unsigned			num_sectors;		//!< Total number of sectors created at this time
	extern unsigned			max_sectors,		//!< Absolute number of faces allowed at any given time.
							build_operations;	//!< Number of build or update operations registered. Available only if FRACTAL_BUILD_STATISTIC is defined
	extern Time::Delta		build_time;			//!< Time spent performing build or update operations. Available only if FRACTAL_BUILD_STATISTIC is defined
	extern TContext			default_context;	//!< Empty default context passed for certain kernel operations

	extern bool				mipmapping,				//!< Set true to enable automatic mipmap level generation of segment textures. True by default
							texture_compression,	//!< Set true to enable texture compression on segment textures. False by default. Leave this off. Slow, faulty.
							background_subdiv;		//!< Set true to process additional subdivision in the background. Background operations will be started after calling the subdivide() method and stopped before the first process...() call

	
	/**
		@brief Published background subdivision methods
	*/
	namespace BackgroundSubDiv
	{
		void										signalEnd();	//!< Signals that background subdivision should come to an end at the earliest possibility
	}



	//static const float		default_sector_size=50000.0f;
	
	
	typedef M::Composite::sector_t	sector_t;		//!< Sector coordinate type. Typically int32

	struct TMapVertex;
	
	class SurfaceSegment;
	class DataSurface;
	
	template <unsigned>
		class TemplateSurface;
	
	class Body;
	template <unsigned, unsigned>
		class TemplateBody;

	class VertexMap;
	template <unsigned,unsigned>
		class TemplateMap;
	
	class Geometry;
	
	
	String				randomWord();	//!< Creates a random word usable as seed for new fractal bodies
	void				terminate();	//!< Implodes the used kernel. This should be called before the application quits
	void				makeSphereGeometry(Geometry&target);	//!< Adjusts the given geometry to approximate a sphere
	Geometry			makeSphereGeometry();					//!< Creates a new sphere approximating geometry
	void				makeHaloGeometry(Geometry&target);		//!< Adjusts the given geometry to approximate a halo shape
	Geometry			makeHaloGeometry();						//!< Creates a new halo approximating geometry
	String				getConfiguration();
	

	
	void				reportFatalException(const TCodeLocation&location, const String&message);
	
	#define FRAC_FATAL(message)	Fractal::reportFatalException(CLOCATION,message);
	
	

	

	

	typedef	void		(*pInit)		(TChannelConfig&vertex_channel, Random&random, const TContext*context);	//!< Initializes channel data of a new top level vertex \param vertex_channel Target channel data \param random Initialized random source
	
	
	/**
		\brief Fractal geometry
	
		The fractal geometry is used to describe the rough geometry of a fractal shape. It may consist of only a singular triangle, or a sphere, or any shape as required.
	*/
	class Geometry 
	{
	public:
			typedef M::TVec3<>	Vertex;

			Array<Vertex>			vertexField;	//!< Geometrical vertices
			Array<UINT32>			triangleField,	//!< Triangular index path. Each three indices form one triangle using the vertices stored in \b vertex_field
									edgeField;		//!< Edge index path. Each two indices form one edge, using the vertices stored in @b vertexField

	};

	
	
	
	/**
		@brief 64 bit helper hash function
		
		Not my concept, only my implementation. I know, it looks very very odd.<br>
		Found here:<br>
		http://www.java-forums.org/new-java/17026-i-need-32-64-bit-hash-function.html

	*/
	class Hash64
	{
	public:
			UINT64			byte_table[0x100];
	static const UINT64		start_value = 0xBB40E64DA205B064ULL,
							factor = 7664345821815920749ULL;
	
	
							Hash64()
							{
								UINT64 h = 0x544B2FBACAAF1684ULL;
								for (unsigned i = 0; i < 256; i++)
								{
									for (unsigned j = 0; j < 31; j++)
									{
										h = (h >> 7) ^ h;
										h = (h << 11) ^ h;
										h = (h >> 10) ^ h;
									}
									byte_table[i] = h;
								}
							}
			
			UINT64			operator()(const void*data, size_t len)	const
							{
								UINT64 hashed = start_value;
								const BYTE*field = (const BYTE*)data;
								for (size_t i = 0; i < len; i++)
								{
									hashed = (hashed * factor) ^ byte_table[field[i]];
								}
								return hashed;
							}
	};
  

	
	
	/**
		\brief Dynamic length fractal seed
		
		Rotatable, expandable<br>
		This seed type expands as recursion progresses. Due to the basically infinite nature
		of this seed, it may or may not cause significant additional memory consumption.
	*/
	class DynamicSeed
	{
	public:
			typedef Array<BYTE,Primitive>	ValueType;	//!< Seed data type
			
			ValueType				value;	//!< Binary data content of this seed
			UINT32					as_int;	//!< Auto-updated int32 representation of the seed
			
									DynamicSeed();
									DynamicSeed(const String&string);
			String					toHex()	const;	//!< Converts the seed to a printable string
			DynamicSeed&			hash(const String&string);	//!< Builds the local seed from a string
			DynamicSeed&			operator=(const ValueType&array);	//!< Builds the local seed from an existing binary seed
			void					implant(const DynamicSeed&seed, BYTE child_id);
			void					rotate(unsigned depth, BYTE orientation);
			void					randomize();	//!< Randomizes the local seed content
			UINT32					toInt()	const;	//!< Explicitly queries the int representation of the local seed. Under normal circumstances the result is identical to the local @a as_int variable
	};
	
	
	/**
		@brief Fixed length 64 bit fractal seed
		
		Rotatable, shiftable<br>
		This seed type stays 64 bit at all times. Increases processing and storage efficiency
		while preserving sufficient diversity for any feasible application scenario.
	*/
	class Seed64
	{
	public:
	static	Hash64					hashFunction;	//!< Hashing functor
	
			typedef UINT64			ValueType;	//!< Seed data type
			
			UINT64					value;	//!< Raw 64bit seed
			UINT32					as_int;	//!< Auto-updated int32 representation of the seed
			
									Seed64();
									Seed64(const String&string);
			String					toHex()	const;	//!< Converts the seed to a printable string
			Seed64&				hash(const String&string);	//!< Builds the local seed from a string
			Seed64&				operator=(const UINT64&seed);	//!< Builds the local seed from an existing binary seed
			void					implant(const Seed64&seed, BYTE child_id);
			void					rotate(unsigned depth, BYTE orientation);
			void					randomize();	//!< Randomizes the local seed content
			/**
				@brief Explicitly queries the int representation of the local seed. Under normal circumstances the result is identical to the local @a as_int variable
				
				Algorithm designed by Thomas Wang, Jan 1997<br />
				http://www.concentric.net/~Ttwang/tech/inthash.htm
			*/
			UINT32					toInt()	const;
	};
	
	

	typedef Seed64				Seed;	//!< Could use dynamic seeds here too
	
	
	/**
		@brief Fractal aspect description, relative to the coordinate space of the fractal body it is applied to

		An aspect provides all information required to determine the visibility and proximity of a surface segment.
	*/
	class Aspect
	{
	public:
			
			//Engine::Aspect<>	aspect;				//!< View aspect (projection, modelview matrix, etc)
			Engine::Frustum<>	frustum;				//!< Frustrum for visibility checks. Extracted from the currently applied camera object
			M::Composite::Coordinates	relative_coordinates;	//!< Central view position. Relative to the system of the applied body
			M::TVec3<float>		retraction_delta;		//!< Retraction offset from the aspect coordinates to the actual point of view. Unless the applied camera is a focused 3rd-person camera this would be a zero-vector. Note that the calculated frustum volume must already include this vector
			bool				require_reflection;		//!< True if patch reflection should be checked for visibility
			unsigned			min_layer;				//!< Minimum visible layer. Layers whoes depth value is less or equal to  this value are not subdivided
			float				lod;					//!< Level of detail. Typically in the range [1,5] but the ideal value largely depends on the used detail exponent and noise level.
			
								Aspect():require_reflection(true),min_layer(0),lod(1)
								{
									M::Vec::clear(retraction_delta);
								}
								/**
									@brief Calculates the translation vector from aspect to segment space
								
								*/
		template<typename T>
			void				getTranslation(const M::TVec3<sector_t>&segment_sector, M::TVec3<T>&out, float sector_size)	const
								{
									out.x = (T)(segment_sector.x-relative_coordinates.sector.x)*(T)sector_size - (T)relative_coordinates.remainder.x;
									out.y = (T)(segment_sector.y-relative_coordinates.sector.y)*(T)sector_size - (T)relative_coordinates.remainder.y;
									out.z = (T)(segment_sector.z-relative_coordinates.sector.z)*(T)sector_size - (T)relative_coordinates.remainder.z;
								}
	};
	
	/**
		@brief Helper structure to hold all information of an intersection between a ray and a fractal shape
	*/
	struct TRayIntersection:public TBaseRayIntersection
	{
			SurfaceSegment*			segment;	//!< Affected lowest detail surface
	};
	
	/**
		@brief Extended helper structure to hold all information of a ground detection as well as the detected segment
	*/
	struct TGroundInfo: public TBaseGroundInfo
	{
			SurfaceSegment*			segment;	//!< Parent segment of the detected ground point
	};
	
	/**
		\brief Triangular fractal root construction
		
		A shape represents a collection of fractal faces as constructed from a fractal geometry (Geometry).
		To ensure floating point precission even with larger compositions, the shape provides a fixed sector size (stored in Body::context).
		The sector is a integer vector that each surface segment centers on. All of its vertices are relative to this vector.
	*/
	class Body
	{
	protected:
			//Random						random;			//!< Global random variable to retrieve random values from

			/*ThreadPool::Family			thread_family;	//!< Family of all threads belonging to this shape
			ProductionPipe<SurfaceSegment>	waste_pipe,		//!< Pipe used to temporarily store unlinked faces
										focus_pipe;		//!< Pipe used to temporarily store faces that require children
			ProductionPipe<SurfaceSegment> 	project_pipe;		//!< Pipe used to temporarily store faces that have been considered visible
			Array<ThreadObject*>		operation_field;		//!< Thread reference field used to quickly start parallel processes (passed to the kernel)*/

	static	SurfaceSegment				*production;	//!< Currently produced surface patch (if any)
	static	Time::Stamp				trace_time;		//!< Global time when tracing began
	static	Vector<SurfaceSegment*>	need_children;	//!< Buffer containing all surface segments that have requested children during the last frame. Usually in ascending order of priority
			

									
			friend class SurfaceSegment;
			
									Body(const String&name, float sector_size_, unsigned exponent_, unsigned recursion_depth,const ArrayData<unsigned>*vbo_vertex_indices_, const ArrayData<unsigned>*triangle_field_,const VertexMap*full_map_,const VertexMap*reduced_map_);
									
	public:
			Array<DataSurface*>		faces;			//!< Field of face pointers (one for each face). The pointers are managed and automatically erased if the root shape is erased. Top level faces are always data faces
			Vector<SurfaceSegment*>	face_buffer,		//!< Buffer containing all surfaces that have been traced during the last process() operation
									reflection_buffer,	//!< Buffer containing all surface reflection that have been traced during the last processReflection() operation
									water_buffer,		//!< Buffer containing all water segments that have been traced during the last processWater() operation
									attachment_buffer;	//!< Buffer containing all surfaces with attachments that have been traced during the last process() operation. Unlike face_buffer, surfaces with attachments are not ignored when focusing
			
			TContext				*context;							//!< Pointer to the first 16byte aligned address in @a context_field
			BYTE					context_field[sizeof(TContext)+15];	//!< Alignment field to locate @a context in
			
			const unsigned			exponent;			//!< Central map exponent
			unsigned				min_river_layer,	//!< Minimum recursive depth for new rivers to be spawned. Currently not used
									max_river_layer;	//!< Maximum recursive depth for new rivers to be spawned. Currently not used
			
			String					name;				//!< Fractal body name
			
			
			Vector<TCrater>			craters;			//!< All craters on this body


			//M::Composite::Coordinates		center;	
			
			const VertexMap			*full_map,				//!< Pointer to a full map instance, describing the nature of the full fractal detail of each surface segment
									*reduced_map;			//!< Pointer to a reduced map instance, describing the nature of the reduced (geometrical) detail of each surface segment
			const ArrayData<unsigned>
									*triangle_field,		//!< Field of triangles for height lookup. These indices describe one triangle per three elements, covering each surface segment for ground lookups or ray intersections
									*vbo_vertex_indices;	//!< Index field mapping the vertices of a reduced VBO segment geometry to their respective fractal origin vertices
			
			bool					fractal_clouds;		//!< Not used

			unsigned				least_traced_level;	//!< Most detailed layer that has been chosen for rendering
			uint64_t				range;				//!< Number of vertices in one direction as derived from the detail exponent.
			Seed					root_seed;			//!< Seed used to generate this shape. Using the same seed and root geometry will result in the same structure.

	virtual							~Body()
									{
										clearFaces();
									};
	
			void					clearFaces();	//!< Deletes all faces and resizes the faces array to zero.
			void					purgeSegments();	//!< Deletes all sub faces
	
	protected:
			void					reset();						//!< Resets \b face_buffer and \b least_traced_level . Automatically called by trace0()
	public:
	
			//void					clearChildren(unsigned depth);	

	static	bool					can_update;		//!< Mostly for internal use. Indicates that the processStep() method may invoke the update() method of surface segments. This variable is set to false during background subdivision since the processing thread does not own a valid opengl context binding

	static	void					flushQueue();	//!< Flushes the list of requested fractal children. Usually this method is not required to be called explicitly
	static	void					exportQueue(ArrayData<SurfaceSegment*>&);	//!< Exportes the list of requested fractal children in order.
			
			/**
				@brief Central processing entry point
				
				process() updates the local planet's provided level of detail to the specified view. Faces that exceeded their time to live are erased.
				Invoke the static subdivide() function once done
				
				
				@param time Current time
				@param top_level_ttl Time (in seconds) that top level surfaces remain in memory even if not referenced. Each sub level receives 90% of the next upper level's ttl time interval
				@param view View to adopt to
			*/
			void					process(const Time::Stamp&time, float top_level_ttl, const Aspect&view);
			
			/**
				@brief Alternate version of process
				
				The invoking context assumes that the entire body is invisible and thus avoids opacity checks. Only timeout operations are performed as well as the local face buffer reset.
				It is crucial to invoke either process() or processInvisible() once ber fractal body each frame or unreferenced data may never be freed
				
				
				@param time Current time
				@param top_level_ttl Time (in seconds) that top level surfaces remain in memory even if not referenced. Each sub level receives 90% of the next upper level's ttl time interval.
					Note that the execution of this method assumes that none of the segments of this body are currently referenced
			*/
			void					processInvisible(const Time::Stamp&time, float top_level_ttl);
			
			
			/**
				@brief Subdivides pending surface segments
				
				@param time_frame Time (in seconds) that the finalizer may spend building new surface patches. Should not exceed 1/20th of a second
			*/
	static	void					subdivide(float time_frame);
	
			/**
				@brief Processes a single finalization step and advances the step counter.

				Automatically retrieves new faces from the production pipe if necessary.
				@return true if more steps can be performed, false otherwise
			*/
	static	bool					processStep();
			
			/**
				@brief Determines the current level of overloading on the fractal processor of this surface
				
				The overload level is determined from the summed priorities of all remaining child requesting vertex patches (SurfaceSegment's).
				The returned value is 0 if all faces have been processed and the processor is idle
				
				 @return Current overload level of this shape
			*/
	static	float					currentOverload();
			
			/**
				@brief Performs a recursive ray casting test on the currently available data
				
				M::Composite are required to be in planet space.
				
				@param b Ray base point. Intersections before this point are ignored
				@param d Normalized ray direction
				@param intersection [out] Reference to an intersection object that should hold all available information of the detected intersection
				@return true if an intersection was detected, false otherwise
			*/
			bool					rayCast(const M::Composite::Coordinates&b, const M::TVec3<>& d, TRayIntersection&intersection);

			/**
				@brief Performs a recursive ground query on the currently available data

				M::Composite are required to be in planet space

				@param b Point to find the nearest ground under
				@param min_layer Depth of the most detailed surface segments that may be queried. Recursion will not continue past this layer. 0 indicates that all segments should be processed
				@param ground [out] Reference to a ground object that should any located ground coordinates
				@return true if a ground intersection could be located, false otherwise
			*/
			bool					groundQuery(const M::Composite::Coordinates&b, unsigned min_layer, TGroundInfo&ground);

			
			void					checkLinkage();	//!< Verifies the integrity of the interface linkage. An exception of type Exception is thrown if this test fails. This test is slow and should not be called unless bugs are expected.
/*
			SurfaceSegment*				detectVisualPathCollision(const double p0[3],const double p1[3],const int sector[3], float&scalar_out, float point_out[3],float normal_out[3], int sector_out[3]);	//!< Determines an intersection of the specified egde and the most detailed faces with a priority >= 0. Faces that have been rendered invisible are not checked. The method returns face, point and normal. Both point and normal remain undefined if the method returns NULL.
			SurfaceSegment*				detectVisualCollision(const double p0[3],const double p1[3],const int sector[3], float&scalar_out, float point_out[3],float normal_out[3], int sector_out[3]);	//!< Determines an intersection of the specified egde and the most detailed visible faces. The method returns face, point and normal. Both point and normal remain undefined if the method returns NULL.
			SurfaceSegment*				detectCollision(const double p0[3],const double p1[3],const int sector[3], float&scalar_out, float point_out[3],float normal_out[3], int sector_out[3]);	//!< Determines an intersection of the specified egde and the most detailed faces. The method returns face, point and normal. Both point and normal remain undefined if the method returns NULL.
*/
	
	template <typename T>
		inline	void				getTranslation(const M::TVec3<sector_t>& from_sector, const M::TVec3<sector_t>&to_sector,M::TVec3<T> &out_vector)	//! Retrieves the translation vector between two sectors
									{
										out_vector.x = (T)context->sector_size * (T)(to_sector.x-from_sector.x);
										out_vector.y = (T)context->sector_size * (T)(to_sector.y-from_sector.y);
										out_vector.z = (T)context->sector_size * (T)(to_sector.z-from_sector.z);
									}
	template <typename T0, typename T1>
		inline void					convertToAbsolute(const M::TVec3<sector_t>& sector, const M::TVec3<T0>& vertex, M::TVec3<T1>& absolute_vertex)	const	//! Converts the specified local vertex to its counterpart in absolute space
									{
										absolute_vertex.x = (T1)context->sector_size * (T1)sector.x + (T1)vertex.x;
										absolute_vertex.y = (T1)context->sector_size * (T1)sector.y + (T1)vertex.y;
										absolute_vertex.z = (T1)context->sector_size * (T1)sector.z + (T1)vertex.z;
									}									

	static	void					defaultInit(TChannelConfig&vertex, Random&random, const TContext*context);	//!< Default vertex initialization function
			/**
				\brief Completely rebuilds this shape from scratch. The specified geometry is used as base shape
				\param geometry Fractal geometry to use as shape
			*/
			void					rebuildFromGeometry(const Geometry&geometry, pInit init_function=defaultInit);
	virtual	DataSurface*			makeFace()=0;	//!< Abstract top level face constructor

			SurfaceSegment*		anySegment(unsigned depth);	//!< Retrieves the first surface of the specified depth. For analysis purposes @return First matching segment or NULL if no such exists @param depth Depth of the requested segment with 0 being the most detailed segments

			void					subdivideAllSegments(bool generate_render_data);	//!< Forces the subdivision of all leaf segments of this body. Does not check for priority or memory limitations
	};
	
	
	/**
		@brief  Fractal surface data patch

		SurfaceSegment is the general container for surface data patches.
		Provides additional link and rendering information as well as members that control the process flow.
	*/
	class SurfaceSegment:public TSurfaceSegment
	{
	protected:
									SurfaceSegment(unsigned exponent, unsigned vertex_count);
									SurfaceSegment(unsigned exponent_, unsigned vertex_count_, Body*super_, SurfaceSegment*parent_,BYTE child_index_,const Time::Stamp now);
			void					modifySphere(const M::AbstractSphere<float>&child_sphere);	//!< Alter the local face sphere to include the given child sphere and pass the local sphere on to the respective parent recursivly.
			TSurfaceSegmentLink		resolveChild(const TSurfaceSegmentLink&link, BYTE index);					//!< Returns a link to the child of the given face in the given direction \param link Link to the respective parent neighbor \param index Orientation index of the requested child (mostly opposing the index of the respective neighbor)
			TSurfaceSegmentLink		resolveChildRec(const TSurfaceSegmentLink&link, BYTE index);
			void					include(SurfaceSegment*child);	//!< Modifies the local SurfaceSegment::global_sphere variable so that it includes the specified child's SurfaceSegment::global_sphere
			int						setPriority(const Aspect&view, bool override_only,const Time::Stamp&);	//!< Invokes the project() callback function to determine the local priority @return local priority @param view View to apply @param override_only Indicates that this segment may have priority only if it features an override flag
			void					setReflectionPriority(const Aspect&view);
			void					setWaterPriority(const Aspect&view);

			void					copyVBOedge(BYTE edge, bool first_half);
	public:
			class Attachment	//! Custom surface attachment
			{
			public:
			virtual					~Attachment()	{};
			};
			
			/**
				@brief Neighbor detail degeneration vector

				This structure contains one detail difference value per neighbor. Only detail degeneration is mapped.
				If a neighbor is of higher detail then the respective @a delta value is 0.
				This vector must be updated once per frame for every visible segment and is used to determine the applicable pattern for surface rendering.
			*/
			union TNeighborDepthConfig
			{
				unsigned			delta[3];			//!< Detail difference (degrade only) to the currently rendered neighboring face.
				struct
				{
					unsigned		delta0,				//!< Detail difference to neighbor #0
									delta1,				//!< Detail difference to neighbor #1
									delta2;				//!< Detail difference to neighbor #2
				};
			};
	
	
	

			Seed					inner_seed,			//!< Seed used for inner vertex content and passed to child #3
									outer_seed[3];		//!< Seed used for vertices bordering neighbor i and passed to child #i
			const unsigned			exponent,			//!< Size exponent
									vertex_range,		//!< Number of vertices in one direction
									vertex_count;		//!< Number of vertices (derivable from exponent)

			M::AbstractSphere<float>local_sphere,		//!< Sphere including all local vertices.
									global_sphere;		//!< Sphere including all local and child vertices.
			
			M::Vec3<>				corner[3],			//!< Sector relative surface corner vector
									water_center;		//!< Water sphere center (sector relative)
	
			Body*					super;				//!< Governing Body
			M::TVec3<>					translation;		//!< Temporal translation vector of this surface relative to the view coordinates used during the last project() operation. Valid only if this surface was inserted into the project buffer
			float					distance_square,	//!< Squared distance between view coordinates and the global sphere of this surface
									noisiness;			//!< Maximum occuring height variance relative to the local segement radius. Resampled during finishBuild(). Approximated in the range [0,1]

			TNeighborDepthConfig	surface,			//!< Neighbor detail degeneration vector, determined for landscape surface rendering
									reflection,			//!< Neighbor detail degeneration vector, determined for landscape reflection rendering. Not used if reflection rendering is disabled
									water;				//!< Neighbor detail degeneration vector, determined for segment water rendering. Not used if water rendering is disabled
			
			
			unsigned				priority;			//!< Process priority. Updated during setPriority() if this surface segment is considered visible or required for subdivision of neighboring segments
	static	const int				frame_delay = 50;					//!< Number of frames that is assigned to @a delay_counter whenever the local texture has been updated. The segment must request a minor texture update at least this often to be updated
	static	bool					allow_non_crucial_texture_update;	//!< Allows the project() method to update segment textures if minor vertex data changes have occured. Automatically set false with the first such update
			int						delay_counter;		//!< Texture update counter. Unless geometrical updates become necessary the update() method of a requesting segment must execute this often until the texture is actually updated (allow_non_crucial_texture_update must also be true)
			//Random					random;				//!< Random generator structure.
			
			union
			{
				SurfaceSegment			*child[4];			//!< Children. Either all are set or none.
				struct
				{
					SurfaceSegment		*c0,*c1,*c2,*c3;
				};
			};
			
			Time::Stamp			referenced;			//!< Time of last project() call.

			struct Visibility	//! Segment visibility flags
			{
				enum visibility_t	//! Segment visibility flags
				{
					Invisible	=	0x00,		//!< Segment is entirely invisible (Full compare only).
					Range		=	0x01,		//!< @deprecated used to signal that this segment is in range but considered visible by the frustum
					Primary		=	0x02,		//!< This segment is considered visible for landscape rendering. Set by setPriority()
					Reflection	=	0x04,		//!< This segment is considered visible for landscape reflection rendering. Set by setReflectionPriority()
					Water		=	0x08,		//!< This segment is considered visible for water surface rendering. Set by setWaterPriority()
					Override	=	0x10		//!< Visibility override. Local subdivision is required for neighboring segments to subdivide.
				};
			};
			
			UINT32					visibility;	//!< Flag composition of Visibility flags. Reinitialized during setPriority() , enriched during setReflectionPriority() and setWaterPriority()
									
			Attachment				*attachment;	//!< Custom attachment pointer. Initially set to NULL. Automatically deleted on object destruction.

	static	void	(*onUpdate)(SurfaceSegment*surface);			//!< Callback function invoked each time a surface segment is updated
	static	void	(*onSegmentAvailable)(SurfaceSegment*surface);	//!< Callback function invoked each time a new surface segment first becomes visible. Only invoked once per surface segment
	
	static	unsigned				max_load;	//!< Maximum number of faces that may be loaded at the same time. 0 (disabled) by default.
	static	bool					may_subdivide;	//!< Global subdivision setting. Set to false to prevent subdivison

	virtual							~SurfaceSegment();
		template <typename T0, typename T1>
			inline void	convertToAbsolute(const M::TVec3<T0>&vertex, M::TVec3<T1>&absolute_vertex)	const	//!< Converts the specified local vertex to its counterpart in absolute space
			{
				absolute_vertex.x = (T1)super->context->sector_size * (T1)sector.x + (T1)vertex.x;
				absolute_vertex.y = (T1)super->context->sector_size * (T1)sector.y + (T1)vertex.y;
				absolute_vertex.z = (T1)super->context->sector_size * (T1)sector.z + (T1)vertex.z;
			}
		
		template <typename T0, typename T1>
			inline void	convertToRelative(const M::TVec3<T0>&absolute_vertex, M::TVec3<T1>&relative_vertex)	const	//!< Converts the specified global vertex to its counterpart in local space
			{
				relative_vertex.x = (T1)(absolute_vertex.x-(T0)super->context->sector_size * (T0)sector.x);
				relative_vertex.y = (T1)(absolute_vertex.y-(T0)super->context->sector_size * (T0)sector.y);
				relative_vertex.z = (T1)(absolute_vertex.z-(T0)super->context->sector_size * (T0)sector.z);
			}
			void					setNeighbors(const TSurfaceSegmentLink&link0, const TSurfaceSegmentLink&link1, const TSurfaceSegmentLink&link2);
			void					resetLinkage();		//!< Clears both remote and local neighbor links
			void					unhook();			//!< Clears all remote neighbor links to the local surface. Does not change the local neighbor links.
			/**
				@brief Reinitializes certain local variables

				reinit() updates the local sector (based on parent data, if existing),
				creates localized copies of applicable parent craters
				as well as parent geometrical edges (with the exception of the most detailed edge),
				and determines all four required seeds.
				This method also makes sure that neighbors are properly informed of the existence of this segment.

				reinit() is usually called shortly after creation of the local surface segment, once all existing neighbors are assigned.
			*/
			void					reinit();

	protected:
			void					resolve(unsigned d0, unsigned d1, unsigned d2);	//!< Internal method used
			void					resolveReflection(unsigned d0, unsigned d1, unsigned d2);
			void					resolveWater(unsigned d0, unsigned d1, unsigned d2);
	public:
			void					resolve();				//!< Recursively updates the detail difference vector @a surface
			void					resolveReflection();	//!< Recursively updates the detail difference vector @a reflection
			void					resolveWater();			//!< Recursively updates the detail difference vector @a water
			/**
				@brief Builds the local vertex data set from its parent's vertex data

				This includes generation of vertices and local normals, not textures.
				build() also sets the HasData flag.
			*/
			void					build();
			void					finishBuild();	//!< Retrieves the three corner vertices, updates the surrounding sphere, and links neighbors. Automatically called by build()

			/**
				@brief Recursively determines the closest intersection of this segment (and all its children) and the specified ray

				@param b Base coordinates of the ray (in planet space)
				@param d Normalized direction vector of the ray
				@param intersection [out] Resulting closest intersection (if any)
				@return true if an intersection has been detected, false otherwise
			*/
			bool					rayCast(const M::Composite::Coordinates&b, const M::TVec3<>& d, TRayIntersection&intersection);
			/**
				@brief Recursively determines the nearest ground coordinate of this segment (and all its children)

				@param b Base coordinates of the point beneath which the ground should be found (in planet space)
				@param d Normalized direction towards the planet core
				@param min_layer Minimum layer that the recursive process may reach. Further subdivided surface segments are ignored
				@return true if a ground point was found among this segment and all its children, false otherwise
			*/
			bool					groundQuery(const M::Composite::Coordinates&b, const M::TVec3<>&d, unsigned min_layer, TGroundInfo&ground);
			
//			void					synchronizeBorders();
			
			/**
				@brief Recursively erases all child segments of this segment

				The HasChildren flag is cleared, all child pointers set to NULL, the production pointer
				set to NULL (if currently set to this), and the surrounding sphere updated.
			*/
			void					dropChildren();
			/**
				@brief Creates all children (if non existing)
				
				Data is not generated. However neighbor, child, and parent links are properly assigned. Seeds are prepared and edge open flags are assigned if appropriate.
				
				@param time Current time
				@param set_flag Set true to automatically set the HasChildren flag. Set false to leave flags as they are
			*/
			void					createChildren(const Time::Stamp&time, bool set_flag=true);

			/**
				@brief Recursively determines the visibility, priority, and necessity for subdivison of this surface segment and all its children

				The project() method invokes setPriority() on all children of the local segment to determine their primary visibility, necessity for subdivison,
				and priority. The local @a referenced variable is updated if this segment is considered visible in any way (also by override).
				It also checks timeouts of all invisible surface segments via checkTimeouts()
				This method must be called once per frame. This is typically done by Body::process()

				@param view Body-relative aspect that should be applied
				@param ttl Time (in seconds) that this surface segment can survive without being visible. This value degenerates as recursion progresses
				@param time Current time
				@param override_only Signals that this surface segment and all its children are no longer considered visible and should only be processed
										based on their Visibility::Override visibility flag.
			*/
			void					project(const Aspect&view, float ttl, const Time::Stamp&time, bool override_only);
			/**
				@brief Recursively determines the visibility and necessity for reflection subdivison of this surface segment and all its children

				The projectReflection() method invokes setReflectionPriority() on all children of the local segment to determine their reflection visibility
				and necessity for subdivision.
				This method must be invoked after calling project() each frame. This is typically done by Body::process()

				@param view Body-relative aspect that should be applied
			*/
			void					projectReflection(const Aspect&view);
			/**
				@brief Recursively determines the visibility and necessity for water subdivison of this surface segment and all its children

				The projectWater() method invokes setWaterPriority() on all children of the local segment to determine their water surface visibility
				and necessity for subdivision.
				This method must be invoked after calling project() each frame. This is typically done by Body::process()

				@param view Body-relative aspect that should be applied
			*/
			void					projectWater(const Aspect&view);
			//void					projectClouds(bool has_translation);

			/**
				@brief Recursively checks if this surface segment or any of its children have exceeded their maximum time to live

				checkTimeouts() compares the @a referenced variable with the current time.
				Should this segment have exceeded its maximum invisible life-span then it (and all of its children)
				is automatically discarded.

				@param time Current time
				@param ttl Time (in seconds) that this surface segment can survive without being visible. This value degenerates as recursion progresses
			*/
			void					checkTimeouts(const Time::Stamp&time, float ttl);
			/**
				@brief Synchronizes edge data with all neighbors

				Only edges that have not been merged in any prior invocation are affected.
				The merging process copies or generates edge child vertices depending on the flag constellation.
				It also invokes segment degeneration if applicable and updates edge normals.
				This method is automatically invoked by Body::processStep()
			*/
			void					mergeEdges();

			/**
				@brief Updates the local texture and VBO geometry if necessary

				This method is invoked once per frame by project() whenever this surface segment is prepared for rendering.
				If the local RequiresUpdate flag is set then both texture and VBO geometry are updated immediately.
				If only the RequiresTextureUpdate flag is set then an update is considered non-crucial and potentially postponed.
			*/
			void					update();
			void					updateClouds();	//!< Currently not in use

			/**
				@brief Determines the order of the specified egde

				orderOfEdge() uses the current corner vectors to determine the absolute order of the specified edge.
				The result can directly be fed into the @a primary variable of a neighbor link.
				The method's result is asymmetrical. If a neighboring segment invokes the method on its copy of the same edge
				then the result is negated.

				@param edge Index of the edge to query in the range [0,2]
				@return Edge order (the result is always the same but can only be savely used to determine ownership of an edge)
			*/
			bool 					orderOfEdge(BYTE edge)	const;
			
			/**
				@brief Recursively assigns the Visibility::Override flag to the next surface segment responsible for generating a needed surface segment
				
				This method is invoked if a visible or otherwise required surface segment wants to subdivide but has one or more non-synchronized edges.
				The nearest parent of the non existing required surface segment is Visibility::Override flagged. Also all of its parents are flagged that way.
				The flags are valid at most one frame, then reset.
				
				@param child_index Index of the calling child (0-3)
				@param neighbor_index Index of the needed neighbor (0-2)
				@param t Time to assign to the referenced member variable of triggered segments
				@param priority_to_set Priority value to assign to triggered segments
			*/
			void					triggerOverrideFlag(BYTE child_index, BYTE neighbor_index,const Time::Stamp&t, unsigned priority_to_set);
			/**
				@brief Inner method of triggerOverrideFlag(BYTE child_index, BYTE neighbor_index)
				
				Flags the local segment and all parents. NULL-pointer sensitive
				
				@param t Time to assign to the referenced member variable of triggered segments
				@param priority_to_set Priority value to assign to triggered segments
				@return true on success, false if the local segment is not existing or not ready
			*/
			bool					triggerOverrideFlag(const Time::Stamp&t, unsigned priority_to_set);
	
			
			/**
				@brief Default visual sphere definition method
				
				defaultDefineSphere() updates @b sphere so that it roughly includes all vertices of the local fractal face.
				The method is programmed for performance and thus does not check each vertex separatly.
				Under normal circumstances the resulting sphere will be 10% larger than required to minimize non-included vertices.
				
				@param sphere Sphere to modify
			*/
			void					defaultDefineSphere(M::AbstractSphere<float>&sphere);

			/**
				@brief Updates @b local_sphere and @b global_sphere of this segment and all parents

				The method first determines @a local_sphere by use of the three corner vertices.
				@a global_sphere is then set to contain @a local_sphere as well as @a global_sphere of all
				immediate children.
				All parents are recursively notified of the change so that their surround global spheres may be updated as well.
			*/
			void					updateSphere();
			/**
				@deprecated crude debug method
			*/
			void					checkLinkage();	
			/**
				@brief Retrieves one cover vertex per rendered vertex of this surface
				
				The provided array will be resized to contain the coverage of the local segment at the same resolution as it is geometrically rendered at.
				
				@param cover_vertices [out] Cover vertex array to be filled (will be resized if necessary)
			*/
			void					getCoverVertices(ArrayData<TCoverVertex>&cover_vertices)	const;
			/**
				@brief Similar to getCoverVertices() except that it retrieves the full resolution of vertices
				
				Edge vertices that are not primary to the local surface segment are not included 
			*/
			void					getFullPrimaryCoverage(ArrayData<TCoverVertex>&cover_vertices)	const;
			/**
				@brief Determines an identity string of the local surface segment.

				The resulting string has the form<br>
				[body name]/[top most segment index]/[index path]<br>
				e.g.<br>
				"planet/17/032310"<br>
				The concluding index path contains each one number in the range [0,3] to specify the index of the child to follow.

				@return Segment identity string
			*/
			String					identify()	const;
			SurfaceSegment*		anyChild(unsigned depth);	//!< Retrieves the first surface of the specified depth. For analysis purposes @return First matching segment or NULL if no such exists @param depth Depth of the requested segment with 0 being the most detailed segments
			
			void					registerLeafSegments();	//!< Recursively adds all leaf segments (segments that have no children) to Body::need_children. Also checks @b this.
	};
	
	/**
		\brief Fractal root face
		
		This segment type is used as the top most segment layer. None of its children are of this type.
		Originally it used to hold locally generated data in contrast to the CUDA-generated data of SurfaceSegment<br>
		Now it merely serves as a wrapper with additional methods

	*/
	class DataSurface:public SurfaceSegment//, public ThreadObject
	{
	protected:
			//Construct::TProjectThreadParameter	split_param[4];
			
									
			
									DataSurface(unsigned exponent, unsigned vertex_count);

			static	bool			faceCollision(const TVertex&v0, const TVertex&v1, const TVertex&v2, const M::TVec3<>&coord, M::TVec3<>&point_out, M::TVec3<>&normal_out);
			
			friend class Body;


	public:
			//TVertex				*vertex_pointer;			//!< Non proprietary pointer to the first element of the appointed vertex array
			

			
	//virtual							~DataSurface();
			
			/**
				@brief Retrieves a specific vertex via its grid coordinates

				The vertex is retrieved via its column and row. Invalid indices are not checked

				@param col Vertex column in the range [0,row]
				@param row Vertex row in the range [0,1<<exponent]
				@return Reference to the requested vertex
			*/
			inline TVertex&		getVertex(unsigned col, unsigned row)
			{
				return vertex_field[super->full_map->getIndex(col,row)];
			}			
			inline const TVertex&	getVertex(unsigned col, unsigned row)	const	//! @overload
			{
				return vertex_field[super->full_map->getIndex(col,row)];
			}
			
			/**
				@brief Retrieves one of the three corner vertices from fractal data

				Invalid indices are not checked.

				@param index Index of the requested corner vertex in the range [0,2]

				@return Reference to the requested vertex
			*/
			inline TVertex&		getCorner(BYTE index)
			{
				return vertex_field[super->full_map->border_index[index][0]];
			}			
			inline const TVertex&	getCorner(BYTE index)	const	//! @overload
			{
				return vertex_field[super->full_map->border_index[index][0]];
			}
			

			//void					copyEdge(BYTE k);
			
			void					finishConstruction();	//!< Generates normals and sets the HasData flag
	
			void					recSyncBorders();	//debugging only

			
			/**
				@brief Generates a subselection of vertices based on already generated vertices

				This method allows to construct the content of the local surface segment in a fractal manner without any existing parent surface segment.
				Parent vertices, required for the construction process, are taken from already defined vertices. The distance to these is defined
				by the @a level parameter which specifies the recursive operation level.
				The first generation of vertices is subdivided using merely the three corner vertices, subsequent vertex subdivision is processed using
				the already generated vertices, and so on.<br>
				The process should go high to low starting at @a exponent-1 and ending at 0 with all surface segments of one body being processed before
				moving on to the next level. Segments should also be synchronized by invoking syncGeneratedBorders() using the same level as parameter.<br>
				This method is invoked and required only by Body::rebuildFromGeometry()

				@param level Recursive level in the range [0,exponent-1]. Should be invoked going high to low.

			*/
			void					generateLayer(unsigned level);
			void					generateLayer(unsigned level, void(*updateHeight)(TVertex&vertex));	//!< Version of generateLayer() using a custom height update function
			void					syncGeneratedBorders(unsigned level);	//!< Synchronizes border vertices as generated by generateLayer(level).
	};



	/**
		@brief Mostly outdated template wrapper to the DataSurface class.

	*/
	template <unsigned Exponent>
		class TemplateSurface:public DataSurface
		{
		public:
									TemplateSurface();
									TemplateSurface(Body*super_);
									TemplateSurface(Body*super_,TemplateSurface<Exponent>*parent, BYTE child_index, const Time::Stamp&time);
											
									TemplateSurface(Body*super_,TemplateSurface<Exponent>*parent, BYTE child_index, const TSurfaceSegmentLink&n0, const TSurfaceSegmentLink&n1, const TSurfaceSegmentLink&n2, const Time::Stamp&time);
		};
	
	/**
		@brief Template wrapper for the Body class

		Requires exponent and geometrical exponent reduction as parameters.

		@tparam Exponent Geometrical complexity exponent to use for all surface segments of this body. This value describes the full internal complexity, which is only applied to textures in its full resolution.
		@tparam ExponentReduction Level by which the full internal complexity should be reduced before VBO geometries are constructed. Savely tested has been a value of up to 3, thus reducing the linear complexity
									by a factor of 8 and the triangle complexity by a factor of 64.
	*/
	template <unsigned Exponent, unsigned ExponentReduction>
		class TemplateBody:public Body
		{
		public:
				typedef TemplateSurface<Exponent>				Face;		//!< Contained top level surface segment type
				typedef TemplateMap<Exponent>					FullMap;	//!< Map construct defining the nature of full triangle surface segments
				typedef TemplateMap<Exponent-ExponentReduction>ReducedMap;	//!< Map construct defining the nature of the reduced (geometrically rendered) segment layout

		static	Array<unsigned,Primitive>						vbo_vertex_index_field,	//!< Array of indices that defines for each vertex of reduced complexity where to find its origin in full complexity
																triangles;				//!< Array of triangle indices that defines a simple (reduced by parameter) covering of the entire segment area. Each three indices define one triangle. The contained vertex indices point to vertices of full complexity.
				
										TemplateBody(float sector_size=default_sector_size, unsigned max_recursion_level=32);
										TemplateBody(const String&name, float sector_size=default_sector_size, unsigned max_recursion_level=32);
		virtual	DataSurface*			makeFace();
		/**
			@brief Fills @b vbo_vertex_index_field and @b triangles

			This method must be invoked once before using either @a vbo_vertex_index_field or @a triangles

			@param lookup_reduction Exponent reduction by which the full fractal complexity should be reduced for @a triangles . Does not affect @a vbo_vertex_index_field.
		*/
		static	void					assembleMaps(unsigned lookup_reduction);
		};
	
	
	/**
		@brief Triangle index pattern storage
		
		AbstractIndexPatterns stores a number of triangular patterns for different surface-to-neighbor-constellations.
	*/
	class AbstractIndexPatterns
	{
	public:
			UINT32					*const pattern,			//!< Pointer to index patterns
									*const pattern_offset,	//!< Pattern offsets. A pattern @a i covers all entries of @a pattern from @a pattern_offset[i] to @a pattern_offset[i+1]
									*const core_pattern;	//!< Pointer to a pattern covern only the inner non-border triangles
			const UINT32 			num_patterns,			//!< Number of patterns stored in this structure. Up to @a num_patterns+1 entries of @a pattern_offset are valid
									exponent,				//!< Size exponent used to generate the local index pattern set
									step0,					//!< Factor applied to neighbor 0 detail difference
									step1;					//!< Factor applied to neighbor 1 detail difference 
									//step2 equals 1

									AbstractIndexPatterns(UINT32*const pattern, UINT32*const pattern_offset, UINT32*const core_pattern, UINT32 num_patterns, UINT32 exponent);
			unsigned				getPatternIndexFor(const SurfaceSegment*seg, const SurfaceSegment::TNeighborDepthConfig&config)	const; 	//!< Retrieves the index of the pattern most suited for the current status of the specified surface. The resulting index can be used as array index of @a pattern_offset @param seg Parent surface segment @param config Depth configuration to retrieve the pattern for @return applicable pattern
			unsigned				getPatternIndexFor(const SurfaceSegment::TNeighborDepthConfig&config,const SurfaceSegment*seg)	const; 	//!< @overload
			unsigned				getReflectionPatternIndexFor(const SurfaceSegment*face);	//!< Retrieves the index of the pattern most suited for the current status of the specified surface. The resulting index can be used as array index of @a pattern_offset
	};
	
	template <unsigned Exponent>
		class CorePattern
		{
		public:
				typedef TemplateMap<Exponent>	MapType;
		public:
				static const unsigned	range = (1<<Exponent)+1;
		protected:
				static UINT32 			core_pattern[(range-4)*(range-4)*3];

				static void			 	createCorePattern();


		};

	/**
		@brief Template derivative of AbstractIndexPatterns
		
		IndexPatterns must be constructed via create() before it can be used.
	*/
	template <unsigned Exponent>
		class IndexPatterns:public AbstractIndexPatterns, protected CorePattern<Exponent>
		{
			typedef CorePattern<Exponent> Super;
		protected:
				
				static UINT32			*current,
										*ocurrent;
				static bool				created;
																					
				static void				makePattern(unsigned r0, unsigned r1, unsigned r2);
		public:
				using Super::range;
				using Super::createCorePattern;


				static UINT32			pattern[(Exponent+1)*(Exponent+1)*(Exponent+1)*range*range*3],	//!< Maps to AbstractIndexPatterns::pattern
										pattern_offset[(Exponent+1)*(Exponent+1)*(Exponent+1)+1];		//!< Maps to AbstractIndexPatterns::pattern_offset
		 
										IndexPatterns();
				void					create();			//!< Creates the local pattern. Must be called at least once before accessing the contents of @a pattern and @a pattern_offset . Subsequent calls to this method have no effect
		};

	template <unsigned Exponent>
		class PrimaryIndexPattern : public CorePattern<Exponent>
		{
		public:
				
				static const unsigned	range = (1<<Exponent)+1;
		private:
				static bool				created;
																					
		public:
				static UINT32			pattern[range*range*3],	//!< Maps to AbstractIndexPatterns::pattern
										pattern_length;
		 
				void					create();			//!< Creates the local pattern. Must be called at least once before accessing the contents of @a pattern and @a pattern_offset . Subsequent calls to this method have no effect
		};

	/**
		@brief IndexPatterns with extended border information

		Due to the degeneration process parent vertices do not remain unchanged during subdivision. As such the
		original parent vertices along each edge must be carried along during each subdivision.<br>
		Original parent vertices are arranged in order of the distance to the respective parent segment. During
		each degeneration step the number of vertices is halved.

	*/
	template <unsigned Exponent>
		class ExtIndexPatterns:public IndexPatterns<Exponent>
		{
		public:
		protected:
				using IndexPatterns<Exponent>::map;
				using IndexPatterns<Exponent>::current;
				using IndexPatterns<Exponent>::ocurrent;
				using IndexPatterns<Exponent>::created;
				using IndexPatterns<Exponent>::range;
				using IndexPatterns<Exponent>::core_pattern;
				using IndexPatterns<Exponent>::pattern;
				using IndexPatterns<Exponent>::pattern_offset;
				using IndexPatterns<Exponent>::createCorePattern;
				typedef TemplateMap<Exponent>	MapType;

				static void				makePattern(unsigned g0, unsigned g1, unsigned g2);
		public:
				static const unsigned	ext_vertices = 3*((1<<Exponent)-1),					//!< Number of additional border vertices used to smooth out gaps to neighboring segments
										all_vertices = MapType::vertices + ext_vertices;	//!< Total number of vertices including border vertices.

				//static unsigned			extVertexCount();
				/**
					@brief Determines the index of the first vertex of the specified degeneration index

					@param degeneration Degeneration index in the range [1,exponent]
					@return Absolute vertex index (counting includes non-border vertices)
				*/
				static unsigned 		getBaseVertexIndex(unsigned degeneration);
				/**
					@brief Determines the degeneration, edge index and vertex index along that edge from a given absolute vertex index

					This operation may be slow if invoked very frequently

					@param index Absolute index of the vertex to query in the range [0,all_vertices-1]
					@param degeneration [out] Degeneration level in the range [0,exponent] where 0 means the vertex in question is no degenerated edge vertex
					@param edge [out] Index of the edge that contains the specified vertex in the range [0,2]
					@param edge_local_index [out] Index of the vertex along the determined edge. &gt= 0. Largest possible index along one edge depends on the degeneration level
					@param true on success, false otherwise. Out variables may not be written to if the method returns false
				*/
				static bool				getVertexLocation(unsigned index, unsigned&degeneration, BYTE&edge, unsigned&edge_local_index);

				void					create();			//!< Creates the local pattern. Must be called at least once before accessing the contents of @a pattern and @a pattern_offset . Subsequent calls to this method have no effect
		};
	
	/**
		\brief Texture space class to roll a planar texture around a sphere
		
		The RollSpace class can be used to project a seamless planar texture onto the surface of a sphere in such a way that the observer believes
		thet the texture is actually fixed on the geometry. The object will produce a system that can be loaded into a graphics context to use the vertex
		space coordinates as texture coordinates.
		This illusion requires the following conditions to be met:<br />
		<ul>
		<li>The texture must be seamless. Large texture coordinates are used</li>
		<li>The texture should be roughly equal colored to not create ugly patterns on larger views</li>
		<li>The used texture range (the visual size of one repeated pattern) should be below 1% of the sphere's size or the illusion will become obvious</li>
		<li>The used clip-size must be a multiple of any used texture range or odd texture jumps will occur</li>
		</ul>
		
	*/
	class RollSpace
	{
	private:
			void	mod(double&val)	const;
	public:
			M::TMatrix4<double>	invert;		//!< Final (unscaled) system to load to a graphics context. Use exportTo() to create a scaled version
			M::TVec3<double>		texture_root,	//!< Current texture zero space. This is the absolute location of the texture point of origin
								texture_up,		//!< Vertical texture space axis pointing along the (invisible) up axis of an applied texture
								texture_x,		//!< Horizontal texture space axis pointing along the x axis of an applied texture
								texture_y;		//!< Depth texture space axis pointing along the y axis of an applied texture
			double				modal_range;		//!< Coordinate range to capture \b texture_root in. Texture coordinates generated by RollSpace can become very large and unclipped texture offsets would soon create undesired effects.
			
					RollSpace(double modal_range=M::Composite::Coordinates::default_sector_size);							//!< Creates a new roll space with texture_up pointing along the y-axis in space. \param modal_range Coordinate range to capture \b texture_root in. Texture coordinates generated by RollSpace can become very large and unclipped texture offsets would soon create undesired effects.
			void	roll(const M::TVec3<double>&up, const M::TVec3<double>&translation);		//!< Rolls the local system so that the new texture up coordinate matches \b up. \param up Vertical direction \param translation Observer movement
			void	roll(const M::TVec3<float>&up, const M::TVec3<float>& translation);		//!< Rolls the local system so that the new texture up coordinate matches \b up. \param up Vertical direction \param translation Observer movement
			void	roll(const M::TVec3<float>&up, const M::TVec3<double>&translation);		//!< Rolls the local system so that the new texture up coordinate matches \b up. \param up Vertical direction \param translation Observer movement
			void	exportTo(M::TMatrix4<float>&system, double range, bool include_translation=true) const;			//!< Creates a scaled system \param system Out pointer to a system matrix to write to \param range Size of one texture pattern @param include_translation Set true to include system translation. False will set the translation column of the target matrix to 0
	};
	
	
	float			relativeSeamWidth(unsigned exponent);	//!< Calculates the relative safety seam width (in texels) between style and normal regions of a segment texture. The resulting value is auto-transformed to texture coordinate space (range [0,1])
	float			relativeTexelSize(unsigned exponent);	//!< Calculates the width/height of a single texture pixel at the specified exponent in texture coordinate space (range [0,1])








#if 0
 triangle order-specification:

vertices:

	col->
row 1
 |	23
 V	456
	7890
	12345
	678901
	...

children:

	0
  . 3 .
1	.	2

.
|\
| \
|0 \
|---\
|\ 3|\
|1\ |2\
|__\|__\

(the third child`s orientation is inverted)

neighbors:

	.
 0 / \ 2
  .___.
	1

orientation (note arrows):

0:
	  .
-> 0 / \ 2
	.___.
	  1

1:
	  .
-> 1 / \ 0
	.___.
	  2

2:
	  .
-> 2 / \ 1
	.___.
	  0

corner-vertices:

0	(0,1)



1		2	(1,0)
(0,0)



vertex hertiage

p=parent
s=GetSibling

		  . p0 .
	. s0 _	|  _  s3 .
p3 .    _	v	_	  . p2
	 s1 .	|	. s2
			p1


Extended border system:
Children contain part of their parents vertex data to adapt to level of detail changes (necessary due to parent vertex degeneration).
These vertices will be placed after the standard inner vertices in the following pattern:

[core vertices][edge0,parent0, length=(2<<(exponent-1))-1][edge0,parent1,length=(2<<exponent-2))-1]...[edge1,parent0][edge1,parent1]...[edge2,parent0]...
Total vertex volume along one edge: (exponent-1)^2




Override system:
The system may look odd but it works (though perhaps not perfectly)
Override priority is added to normal priority by a setPriority() invocation that determines that not all its edges have been synchronized.
As such it assigns additional priority (its own priority in fact) to the next available parents (and their parents, recursively) of the missing
neighbor(s). Since the project() method is inherently invoked in a sub-optimal order in regards to override priorities, two scenarios commonly
occur:

a) (prior case) the segment receives additional priority ahead of its respective project() (and thus setPriority()) invocation.
The Visibility::Override flag is set, thus preventing it from resetting its current priority value. It will, however, clear the flag.
Should the respective segment be considered visible by itself, then the override priority will be overwritten with its local priority.

b) (successive case) the segment receives additional priority after its respective project() invocation. Subdivision can no longer take place
during this frame, however the increased priority will take effect during the next subdivision.
Other than that, this case behaves just like case a), though delayed until the next frame.

Case b) may put considerably more priority on a surface segment than case a) if the respective segment is both visible and in demand.
Both effects do not last longer than two frames. The imbalance effect is thus considered minor.

#endif



	
	


}

#include "interface.tpl.h"


#endif
