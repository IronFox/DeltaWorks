#pragma once

/*
	This module implements all parallel functions that the fractal system operates on.
	You might compare this with an 'engine room', or worker base, that performs less abstract and more mundane operations in the most efficient way possible.
*/


#include <general/exception.h>
#include <global_string.h>

//#include "../math/composite.h"
#include <math/sse.h>

#include <engine/gl/texture.h>
#include <engine/gl/buffer_object.h>
#include "fractal_types.h"
#include "segment_map.h"
#include <math/vector.h>
#include <math/sse/sse_operators.h>
#include <math/sse/sse_constants.h>
//#include <general/thread_system.h>





namespace DeltaWorks::Fractal
{

	class CSSE3Random
	{
	protected:
			//__m128				current;
			M::TVec4<INT32>		seeds;
	public:
	
	static	const __m128i		random_mask /*=_mm_set1_epi32(0x7FFF)*/;
	static	const __m128		random_result_factor /*=_mm_set1_ps(2.0f/32767.0f)*/;


								CSSE3Random(const M::TVec4<INT32>&seeds_):seeds(seeds_)
								{}
	inline	__m128				advance()	throw()
								{
									M::Vec::mult(seeds,1103515245);
									M::Vec::addVal(seeds,12345);
									int rnd0 = (seeds.x>>16);
									int rnd1 = (seeds.y>>16);
									int rnd2 = (seeds.z>>16);
									int rnd3 = (seeds.w>>16);
									return _mm_sub_ps(
												_mm_mul_ps(
													_mm_cvtepi32_ps(
														_mm_and_si128(
															_mm_setr_epi32(rnd0,rnd1,rnd2,rnd3),
															random_mask
														)
													),
													random_result_factor
												),
												M::SSE::one
											);
								}
	};

	#ifndef __GNUC__

		class SupplementalSSE3Random:public CSSE3Random
		{
		public:
		static	const __m128i		random_shuffle_mask /*= _mm_set_epi32(0xB0B00F0E,0xB0B00B0A,0xB0B00706,0xB0B00302)*/,
									random_offset /*= _mm_set1_epi32(12345)*/;

									SupplementalSSE3Random(const M::TVec4<INT32>&seeds_):CSSE3Random(seeds_)
									{}

		inline	__m128				advance()	throw()
									{
										M::Vec::mult(seeds,1103515245);
										__m128i seed = _mm_add_epi32(_mm_load_si128((const __m128i*)seeds.v),random_offset);
										_mm_store_si128((__m128i*)seeds.v,seed);
										return _mm_sub_ps(
													_mm_mul_ps(
														_mm_cvtepi32_ps(
															_mm_and_si128(
																_mm_shuffle_epi8(
																	seed,
																	random_shuffle_mask
																),
																random_mask
															)
														),
														random_result_factor
													),
													M::SSE::one
												);
									}
		};

		class CSSE4Random : public SupplementalSSE3Random
		{
		public:
		static	const __m128i		random_seed_factor;/* = _mm_set1_epi32(1103515245) */
				__m128i				seed;

									CSSE4Random(const M::TVec4<INT32>&seeds_):SupplementalSSE3Random(seeds_),seed(_mm_load_si128((const __m128i*)seeds_.v))
									{}
		inline	__m128				advance()	throw()
									{
										seed = _mm_add_epi32(_mm_mul_epi32(seed,random_seed_factor),random_offset);
										return _mm_sub_ps(
													_mm_mul_ps(
														_mm_cvtepi32_ps(
															_mm_and_si128(
																_mm_shuffle_epi8(
																	seed,
																	random_shuffle_mask
																),
																random_mask
															)
														),
														random_result_factor
													),
													M::SSE::one
												);
									}
		};
	#else /*#ifndef __GNUC__*/

		typedef CSSERandom	CSSSE3Random,CSSE4Random;

	#endif /*#ifndef __GNUC__*/



	struct TVBOCompileParameter
	{
		const TSurfaceSegment	*segment;
		const TContext			*context;
		const VertexMap				*map;
		float					coord_base,
								coord_stretch;

	};



	/**
		@brief Determines the normalized direction vector (as seen from planet center) of a vertex
		
		@param position Vertex position
		@param segment Vertex parent segment
		@param context Vertex parent context
	*/
	inline	void		resolveDirection(const float position[3], const TSurfaceSegment&segment, const TContext&context, float out[3]);	//!< @overload
	inline	void		resolveDirection(const float position[3], const TSurfaceSegment&segment, const TContext&context, M::TVec3<>&out);	//!< @overload
	inline	void		resolveDirectionBlock(	const float p0[3], const float p1[3], const float p2[3], const float p3[3], 
											const TSurfaceSegment&segment, const TContext&context, 
											float out0[3], float out1[3], float out2[3], float out3[3]);		//!< Block version of resolveDirection(), processing four elements at once using SSE
	inline	unsigned char*	getTexel1(unsigned char*texel_data, unsigned x, unsigned y, unsigned dimension);	//!< Retrieves a texel of a luminance texture
	inline	unsigned char*	getTexel3(unsigned char*texel_data, unsigned x, unsigned y, unsigned dimension);	//!< Retrieves a texel of an RGB texture
	inline	unsigned char*	getTexel4(unsigned char*texel_data, unsigned x, unsigned y, unsigned dimension);	//!< Retrieves a texel of an RGBA texture
	
	inline	const unsigned char*	getTexel1(const unsigned char*texel_data, unsigned x, unsigned y, unsigned dimension);	//!< Retrieves a texel of a luminance texture
	inline	const unsigned char*	getTexel3(const unsigned char*texel_data, unsigned x, unsigned y, unsigned dimension);	//!< Retrieves a texel of an RGB texture
	inline	const unsigned char*	getTexel4(const unsigned char*texel_data, unsigned x, unsigned y, unsigned dimension);	//!< Retrieves a texel of an RGBA texture

	inline	const unsigned char*	getTexel1Clamp(const unsigned char*texel_data, unsigned x, unsigned y, unsigned dimension);	//!< Retrieves a texel of a luminance texture. Texture coordinates are M::Clamp to the valid range
	inline	const unsigned char*	getTexel3Clamp(const unsigned char*texel_data, unsigned x, unsigned y, unsigned dimension);	//!< Retrieves a texel of an RGB texture. Texture coordinates are M::Clamp to the valid range
	inline	const unsigned char*	getTexel4Clamp(const unsigned char*texel_data, unsigned x, unsigned y, unsigned dimension);	//!< Retrieves a texel of an RGBA texture. Texture coordinates are M::Clamp to the valid range

	inline float	getNoiseFactor(const TSurfaceSegment& surface, const TContext& context);
	inline float shoreFactor(float x);
	INT32	seedDelta(UINT32 vertex_index);


	namespace SSE
	{
		inline SSE_VECTOR shoreFactor(const SSE_VECTOR x);
	}

	namespace Height
	{
		inline	void	applyDifferentially(const M::TVec3<SSE_VECTOR>&current_position, const SSE_VECTOR&current_height, const SSE_VECTOR&target_height,M::TVec3<>&p0, M::TVec3<>&p1, M::TVec3<>&p2, M::TVec3<>&p3, const TSurfaceSegment&segment, const TContext&context);
		inline	void	applyDifferentially(float vector[3],float current_height, float target_height, const TSurfaceSegment&segment, const TContext&context);	//!< Differential version of the above. The vertex will be positioned relative to its current position depending on the difference between @a current_height and @b target_height

		inline	void	apply(M::TVec3<>&vector, float scaled_relative_height, const TSurfaceSegment&surface, const TContext&context);	//!< Sets the absolute height of an individual position @param vector Position to set the height of @param scaled_relative_height New height relative to body base height @param segment Parent surface segment @param context Parent context
	}

	#define VERTEX_INDEX(_X_,_Y_)	((((_Y_)*((_Y_)+1))>>1)+(_X_))

	inline	unsigned	vertexIndex(unsigned x, unsigned y);	//!< Retrieves the linear index of a vertex from its grid coordinates
	inline	float		getWeight(float h, float average, float variance, float depth);	//!< Determines the weight of a vertex. @param h Current scalar vertex height in the range [-1,+1] @param average Average height of surrounding vertices in the range [-1,+1] @param variance Max difference between surrounding vertices' height and the specified average height @param depth Parent segment depth (0=most detailed, 1=least detailed segment)
	/**
		@brief Queries a random float value in the range [-1,+1] and advances the seed
		@param [in/out] seed value
		@return random value
	*/
	inline	float		getRandom(int&seed);
	inline	float		Clamp(float height);		//!< Clamps a height scalar to the range [-1,+1]. Uses rounded clamping to prevent sharp edges. @param height Current vertex height @return Vertex height in the range [-1,+1]
	template <bool Clamp>
		inline	float	height(const TChannelConfig&channel, bool canyons);	//!< Determines the (M::Clamp) scalar height of a vertex based on its channel configuration @param channel Vertex channel configuration @return Scalar vertex height in the range [-1,+1]
	template <bool Clamp>
		inline	void	polyHeight(const VMOD TChannelConfig&ch0,const VMOD TChannelConfig&ch1,const VMOD TChannelConfig&ch2,const VMOD TChannelConfig&ch3, bool canyons, VMOD float result[4]);	//!< Determines the M::Clamp scalar heights of four vertices at once using SSE
	inline	float		getNoise(float distance, const TSurfaceSegment&segment, const TContext&context);	//!< Determines the noise level of a vertex, based on the distance between its primary parents, as well as segment and context configurations. @param distance Absolute distance between the two primary vertex parents @param segment Parent segment of the new child vertex @param context Parent context configuration
	inline	void		generate3inner(const TVertex&p0, const TVertex&p1, const TVertex&p2, VMOD TVertex&result, unsigned seed, const TSurfaceSegment&segment, const TContext&context, const TCrater*crater_field, count_t crater_count);				//!< Generates an edge vertex based on three parents @param p0 First primary parent @param p1 Second primary parent @param p2 First (and only) secondary parent @param result [out] Resulting vertex. The vertex will be configured but not positioned @param seed Integer seed to use for the generation process @param segment Parent segment of the new vertex @param context Parent context
	inline	void		generate3center(const TVertex&p0, const TVertex&p1, const TVertex&p2, VMOD TVertex&result, unsigned seed, const TSurfaceSegment&segment, const TContext&context, const TCrater*crater_field, count_t crater_count);		//!< Identical to the above but positions the new vertex between its primary parents first. The resulting height is @b not applied
	inline	void		generate4inner(const TVertex&p0, const TVertex&p1, const TVertex&p2, const TVertex&p3, VMOD TVertex&result, int seed, const TSurfaceSegment&segment, const TContext&context, const TCrater*crater_field, count_t crater_count);	//!< Generates a vertex based on four parents @param p0 First primary parent @param p1 Second primary parent @param p2 First secondary parent @param p3 Second secondary parent @param result [out] Resulting vertex. The vertex will be configured but not positioned @param seed Integer seed to use for the generation process @param segment Parent segment of the new vertex @param context Parent context
	inline	void		generate4center(const TVertex&p0, const TVertex&p1, const TVertex&p2, const TVertex&p3, VMOD TVertex&result, unsigned seed, const TSurfaceSegment&segment, const TContext&context, const TCrater*crater_field, count_t crater_count);		//!< Identical to the above but positions the new vertex between its primary parents first. The resulting height is @b not applied
			void		generate6position(const TVertex&p0, const TVertex&p1, const TVertex&p2, const TVertex&p3, const TVertex&p4, const TVertex&p5, VMOD TVertex&result, unsigned seed, const TSurfaceSegment&segment, const TContext&context, const TCrater*crater_field, count_t crater_count);		//!< Parent subdivision operation. The target vertex is required to already be in a valid location. The vertex will be automatically position when done
	inline	void		compileAppearance(const TVertex&vertex, const TSurfaceSegment&segment, const TContext&context, TVertexAppearance&vout);	//!< Compiles a fractal vertex to an appearance configuration @param vertex Vertex to compile, @param segment Parent segment @param context Parent context, @param vout [out] Resulting vertex appearance

	inline	void		compileAppearanceBlock(const TVertex&v0, const TVertex&v1, const TVertex&v2, const TVertex&v3,
											const TSurfaceSegment&segment, const TContext&context,
											TVertexAppearance&a0, TVertexAppearance&a1, TVertexAppearance&a2, TVertexAppearance&a3);	//!< Identical to the above but compiles four vertices to four vertex appearances at once using SSE


	inline	void		resolveCraterDelta(const M::TVec3<>&p, const TCrater&crater, float&imprint, float&strength);
	inline	void		resolveCraterDeltaBlock(const SSE_VECTOR&x, const SSE_VECTOR&y, const SSE_VECTOR&z, const TSSECrater&crater, SSE_VECTOR&imprint, SSE_VECTOR&strength);


	inline	void		compileVBOVertex(const TVertex&vertex, const TVertexDescriptor&info, const TVBOCompileParameter&parameter, float out[TSurfaceSegment::floats_per_vbo_vertex]);

			
	/**
		@brief Fractal processing kernel

		This namespace contains all fractal low level operations where speed is the first and foremost goal.
	*/
	namespace Kernel
	{
		/**
			@brief Unified parameter type that is passed to low level fractal functions

			The required configuration members are ordered in structs that match the names of the invoked low level function.
		*/
		class UnifiedOperationParameter
		{
		public:
				const VertexMap			*map;			//!< Pointer to the matching mapping structure
				const TContext			*context;		//!< Pointer to the calling context (typically owned by a fractal body/planet)
				const TSurfaceSegment	*segment;		//!< Segment that is immediately responsible for the invocation
				System::Mutex			*output_mutex;	//!< Output serialization. Required by certain operations (e.g. groundQuery). Undefined otherwise
				bool					*touched;		//!< Debug array. Allows to mark processed elements


			union
			{
				struct
				{
					VMOD TVertex		*vertex_field;			//!< Vertex field to operate on
					const TVertex		*parent_vertex_field;	//!< Reference vertex field to retrieve parent vertices from
					M::TVec3<float>		transition;				//!< Transition between parent and child vertex space
					TSSECrater			*sse_crater_field;		//!< Relevant crater field, pre-converted to SSE
					TCrater				*crater_field;			//!< Relevant crater field, non-SSE
					count_t				crater_count;			//!< Number of relevant craters provided by both sse_crater_field and crater_field
				}						generateVertex;			//!< Variables required by generateVertex operations (initial child generation process)
				
				struct
				{
					TVertex				*vertex_field;			//!< Vertex field o operate on
				}						generateNormal;			//!< Variables required by generateNormal operations (final child generation process)

				struct
				{
					TVertex				*vertex_field,				//!< Vertex field to operate on
										*neighbor_vertex_field;		//!< Vertex field of the respective neighbor
					const unsigned		*edge,						//!< Edge indices in this segment
										*neighbor_edge;				//!< Edge indices in the neighboring segment
					M::TVec3<float>		edge_transition;			//!< Edge transition (translation) between this segment and the neighbor
					TCrater				*crater_field;				//!< Relevant crater field, non-SSE
					count_t				crater_count;				//!< Number of relevant craters
				}						mergeEdge;				//!< Variables required by mergeEdge operations (edge synchronization)
				
				struct
				{
					const unsigned		*destination_edge,			//!< Indices of the vertices to copy to
										*source_edge;				//!< Indices of the vertices to copy from
					TVertex				*destination_vertex_field;	//!< Vertex field to copy to
					const TVertex		*source_vertex_field;		//!< Vertex field to copy from
					M::TVec3<float>		edge_transition;			//!< Edge transition (translation) between this segment and the neighbor
				}						copyEdge;					//!< Variables required by copyEdge operations (edge synchronization)
				
				struct
				{
					TCoverVertex		*out_vertices;				//!< Cover vertices to write to
					const TVertex		*in_vertices;				//!< Source fractal vertices to read from
					const unsigned		*index_field;				//!< Indices of the vertices to process
				}						generateCover;				//!< Variables required by generateCover operations (cover retrieval process)
				
				
				
				
				struct
				{
					float				*out_vertices;				//!< Out render vertices
					const unsigned		*index_field;				//!< Indices of the vertices to process
					const TVertex		*in_vertices;				//!< Source fractal vertices
					float				base,						//!< Texture coordinate offset
										stretch;					//!< Texture coordinate scale
				}						generateVBO;				//!< Variables required to generate the vertex buffer object 
				
				struct
				{
					const TMapVertex	*info_field;				//!< Info field of all vertices that should be processed
					const TVertex		*vertex_field;				//!< Source fractal vertices
					unsigned			dimension;					//!< Texture size in texels (width=height)
					BYTE				*texel_data;				//!< Out texel data (3 components per texel)
				}						generateTexel;				//!< Variables required to generate the texel cover
				
				struct
				{
					const unsigned		*triangle_indices;			//!< Triangle vertex indices (three per triangle)
					const TVertex		*vertex_field;				//!< Vertex field to retrieve source vertices from
					TBaseRayIntersection*intersection;				//!< Out intersection structure (one, not NULL)
					M::TVec3<float>		b,							//!< Sector relative ray base position
										d;							//!< Sector relative normalized ray direction vector
				}						cast;						//!< Variables required for ray-landscape-intersection
				
				struct
				{
					const unsigned		*triangle_indices;			//!< Triangle vertex indices (three per triangle)
					const TVertex		*vertex_field;				//!< Vertex field to retrieve source vertices from
					TBaseGroundInfo		*ground;					//!< Out intersection structure (one, not NULL)
					M::TVec3<float>		b,							//!< Sector relative ray base position
										d;							//!< Sector relative normalized ground-ray direction vector
				}						groundCast;					//!< Variables required for ground detection (underneath the observer)


				
			};
				
		
		
		
		};
	
		
		typedef void (*pOperation)(unsigned iteration);	//!< Operation function pointer. @deprecated
	
		/**
			@brief Internal parallel fractal operations. Do not directly call
		*/
		namespace Operation
		{
			extern const TVertex empty_vertex;



			#define PARALLEL_OPERATION	void

			#define LINEAR_INTERPOLATION	1
			#include "kernel/subdivideVertexBlock.h"


			PARALLEL_OPERATION	copyParentVertex(unsigned iteration,const UnifiedOperationParameter&parameter);
			PARALLEL_OPERATION	subdivideVertex(unsigned iteration,const UnifiedOperationParameter&parameter);
			PARALLEL_OPERATION	generateNormal(unsigned iteration,const UnifiedOperationParameter&parameter);
			PARALLEL_OPERATION	mergeEdge(unsigned iteration,const UnifiedOperationParameter&parameter);
			PARALLEL_OPERATION	copyEdge(unsigned iteration,const UnifiedOperationParameter&parameter);
			PARALLEL_OPERATION	generateCover(unsigned iteration,const UnifiedOperationParameter&parameter);	//!< Generates the segments coverage based on a partial index field
			
			PARALLEL_OPERATION	generateFullCoverBlock(unsigned iteration,const UnifiedOperationParameter&parameter);	//!< Generates complete segment coverage in full resolution
			PARALLEL_OPERATION	generateInnerCoverBlock(unsigned iteration,const UnifiedOperationParameter&parameter);	//!< Generates the inner segment coverage in full resolution (no edges)
			PARALLEL_OPERATION	generateEdgeCoverBlock(unsigned iteration,const UnifiedOperationParameter&parameter);	//!< Generates the edge segment coverage in full resolution including the first but excluding the last edge vertex
			
			PARALLEL_OPERATION	generateVBO(unsigned iteration,const UnifiedOperationParameter&parameter);
			PARALLEL_OPERATION	generateTexel(unsigned iteration,const UnifiedOperationParameter&parameter);
			PARALLEL_OPERATION	cast(unsigned iteration,const UnifiedOperationParameter&parameter);
			PARALLEL_OPERATION	groundCast(index_t iteration,const UnifiedOperationParameter&parameter);
			
			
			
		}

	
		
		

		/**
			@brief Defines what SSE version to use for certain operations.
			
			Currently effectively distinguished are SSE::V3, SSE::V3Supplemental, and SSE::V4
			Warning: choosing an SSE architecture that is not supported by the processor will result in a crash.
			
			@param version	SSE version to apply.
			@return true if the provided SSE version is accepted by the fractal system, false otherwise. The current implementation rejects SSE versions below 3
		*/
		bool			setSSE(M::SSE::version_t version);






		/**
			@brief Generates the data vertices  of a surface. The surface's device space vertex array is resized to match if needed

			Affected by Kernel::background
		*/
		void 			generateVertices(TSurfaceSegment&segment, const TContext&context,const VertexMap&map);
		
		/**
			@brief Inner production function to be called to generate the final most subdivided layer of a segment
			
			The function assumes all but the most detailed vertices have been generated and can be read from. Only subdivision is performed - no vertex copying.
			Unaffected by Kernel::reduced
			
			@param segment Surface segment to generate vertices of
			@param context Execution context
			@param map Applicable vertex map
		*/
		void			generateFinalLayer(TSurfaceSegment&segment, const TContext&context,const VertexMap&map);
		
		

		/**
			@brief (Re)Generates the normals of a surface segment

			The functions updates the normals of each of the vertices of the specified segment.
			Edge and corner normals are calculated from reduced neighbor data. Should be invoked before merging edges.
			Affected by Kernel::reduced
		*/
		void 			generateNormals(TSurfaceSegment&segment, const TContext&context,const VertexMap&map);

		/**
			@brief Performs edge merging on the specified segment

			Edges that are already merged are not reprocessed. Parent vertex degeneration is performed if all edges of this or a neighboring segment have been merged
			and degeneration is deemed appropriate.

			@param segment Segment to merge the edges of
			@param context Fractal content that the specified segment is part of
			@param map VertexMap to use for the specified segment
			@return Number of segments (local and neighboring segments) that have been degenerated
		*/
		unsigned		mergeEdges(TSurfaceSegment&segment, const TContext&context, const VertexMap&map);

		/**
			@brief Updates the texture associated with the specified segment based on its current data

			@param segment Segment to update the texture of
			@param context Segment context
			@param map Full segment map
			@param generate_mipmap Set true to generate MIPMAP levels on the fly. Passing false disables mipmap generation and referencing. Using MIPMAPing implies using anisotropy to hide borders
			@param compress_texture Set true to automatically compress the generated texture. Passing false disables texture compression for this texture
		*/
		void			updateTexture(TSurfaceSegment&segment, const TContext&context, const VertexMap&map, bool generate_mipmap, bool compress_texture);
		void			updateVBO(TSurfaceSegment&segment, const TContext&context, const VertexMap&full_map, const VertexMap&vbo_map, const ArrayData<unsigned>&vbo_indices);
		//void			updateCloudVBO(TSurfaceSegment&segment, const TContext&context, DeviceArray<unsigned>&device_cloud_triangles, unsigned range);

		/**
			@brief Retrieves the (geometrical) appearance cover of a surface segment

			@param segment Segment to retrieve the cover of
			@param context Segment parent context
			@param cover_indicies Indices of the vertices to retrieve the cover of
			@param out_cover 
		*/
		void			retrieveCover(const TSurfaceSegment&segment, const TContext&context, const ArrayData<unsigned>&cover_indices, ArrayData<TCoverVertex>&out_cover);
		void			retrieveFullPrimaryCover(const TSurfaceSegment&segment, const TContext&context, const VertexMap&map, ArrayData<TCoverVertex>&out_cover);
		


		/**
			@brief Performs a ray intersection check of the specified surface and ray
			@param surface Surface to check against
			@param b Ray base point relative to the specified surface's sector
			@param d Normalized ray direction vector
			@param triangles Device space index array containing each three indices per triangle, which address the vertices of the specified surface
			@param intersection [out] Intersection data to hold detailed information on the detected intersection. Except for the @a isset member variable, the referenced structure remains unchanged if no intersection was detected
			@return true if an intersection was detected, false otherwise
		*/
		bool			rayCast(const TSurfaceSegment&segment,const M::TVec3<float>&b, const M::TVec3<float>&d, const ArrayData<unsigned>&triangles, TBaseRayIntersection&intersection);
		/**
			@brief Performs a vertical ray intersection check to find a ground data beneath a specific location
			@param surface Surface to check against
			@param b Base point relative to the specified surface's sector
			@param down Normalized down vector to indicate the lookup direction. The factor of this vector and the surface (if any) is directly used to determine the height above ground
			@param triangles Device space index array containing each three indices per triangle, which address the vertices of the specified surface
			@param ground [out] Reference to a ground info struct to hold more detailed information.
			@return true if the specified point is above (or beneath) the specified surface, false if no intersection was detected
		*/
		bool			groundQuery(const TSurfaceSegment&segment, const M::TVec3<>&b, const M::TVec3<>&down, const ArrayData<unsigned>&triangles, TBaseGroundInfo&ground);



		extern volatile bool	background;		//!< Indicates that the reduced thread loop should be used. This frees one core for other tasks. False by default. Only certain operations are affected by this switch. Other operations will be processed on the primary kernel as usual.
	}

	/*
		The following are more or less deprecated copy functions. Remnants of darker days
	*/

	inline void  _c4(const float v[4], float w[4])
	{
		w[0] = v[0];
		w[1] = v[1];
		w[2] = v[2];
		w[3] = v[3];
	}

	inline void  _c4(const float v[4], M::TVec4<>&w)
	{
		w.x = v[0];
		w.y = v[1];
		w.z = v[2];
		w.w = v[3];
	}


	inline void  _c4(const BYTE v[4], BYTE w[4])
	{
		w[0] = v[0];
		w[1] = v[1];
		w[2] = v[2];
		w[3] = v[3];
	}
	
	
	inline void  _c3(const float v[3], float w[3])
	{
		w[0] = v[0];
		w[1] = v[1];
		w[2] = v[2];
	}

	inline void  _c3(const float v[3], M::TVec3<>&w)
	{
		w.x = v[0];
		w.y = v[1];
		w.z = v[2];
	}


	inline void  _c3(const BYTE v[3], BYTE w[3])
	{
		w[0] = v[0];
		w[1] = v[1];
		w[2] = v[2];
	}


	inline void  _c2(const float v[2], float w[2])
	{
		w[0] = v[0];
		w[1] = v[1];
	}

	inline void  _c2(const float v[2], M::TVec2<>&w)
	{
		w.x = v[0];
		w.y = v[1];
	}


	inline void  _c2(const BYTE v[2], BYTE w[2])
	{
		w[0] = v[0];
		w[1] = v[1];
	}
}


#include "fractal_kernel.tpl.h"

