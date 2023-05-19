// includes, system
#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_NONSTDC_NO_DEPRECATE 1

#include "../../Core/image/converter/png.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <general/parallel_for.h>
//#include <ppl.h>

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  include <windows.h>
#endif

#include "fractal_kernel.h"
#include <math/vector.h>

#include "kernel/config.h"


#define BEGIN	//cout << __LINE__<<"  "<<__func__<<"()"<<endl;
#define	END		//cout << __LINE__<<"  done"<<endl;

ALIGNED16	SSE_VECTOR 		sse_smooth_neighbor_factor,	sse_smooth_sum_factor;

/*
ALIGNED16 VMOD static float c0exported[4];
ALIGNED16 VMOD static float c1exported[4];
ALIGNED16 VMOD static float c2exported[4];
ALIGNED16 VMOD static float c3exported[4];
ALIGNED16 VMOD static float oceanicexported[4];
ALIGNED16 VMOD static float waterexported[4];
*/

namespace DeltaWorks
{

auto intAdd(int x, int y) -> int
{
  return x + y;
}



inline bool isAligned(const void*address)
{
	#ifndef __GNUC__
		return !(((__int64)address)&15);
	#else
		return !(((long)address)&15);
	#endif
}


namespace Fractal
{
	/*static*/	const __m128i		CSSE3Random::random_mask =_mm_set1_epi32(0x7FFF);
	/*static*/	const __m128		CSSE3Random::random_result_factor =_mm_set1_ps(2.0f/32767.0f);
	/*static*/	const __m128i		SupplementalSSE3Random::random_shuffle_mask = _mm_set_epi32(0xB0B00F0E,0xB0B00B0A,0xB0B00706,0xB0B00302),
									SupplementalSSE3Random::random_offset = _mm_set1_epi32(12345);
	/*static*/	const __m128i		CSSE4Random::random_seed_factor = _mm_set1_epi32(1103515245);
	


	INT32	seedDelta(UINT32 vertex_index)
	{
		//return (((vertex_index*15+3)*(vertex_index*11+2)));
		return vertex_index*vertex_index*167 + vertex_index*61 + 5;
	}

	static bool doDegenerate(const TSurfaceSegment&segment, const TContext&context)
	{
		//return true;
		//return (segment.depth >= context.recursive_depth/3);
		return (segment.depth >= 2);

	}


	void		degenerate6position(const TVertex&p0, const TVertex&p1, const TVertex&p2, const TVertex&p3, const TVertex&p4, const TVertex&p5, VMOD TVertex&result, unsigned seed, const TSurfaceSegment&segment, const TContext&context, const TCrater*crater_field, count_t crater_count)
	{
		float weight= SECONDARY_PARENT_WEIGHT;

		int random_seed = seed;

		static const float	w_total = 6.0f*FRACTAL_DEGENERATION+1.0f,
							w_others = FRACTAL_DEGENERATION/w_total,
							w_self = 1.0f/w_total;
				
		result.channel.c0 = (p0.channel.c0+p1.channel.c0+p2.channel.c0+p3.channel.c0+p4.channel.c0+p5.channel.c0)*w_others+result.channel.c0*w_self;
		result.channel.c1 = (p0.channel.c1+p1.channel.c1+p2.channel.c1+p3.channel.c1+p4.channel.c1+p5.channel.c1)*w_others+result.channel.c1*w_self;
		result.channel.c2 = (p0.channel.c2+p1.channel.c2+p2.channel.c2+p3.channel.c2+p4.channel.c2+p5.channel.c2)*w_others + result.channel.c2*w_self;
		result.channel.c3 = (p0.channel.c3+p1.channel.c3+p2.channel.c3+p3.channel.c3+p4.channel.c3+p5.channel.c3)*w_others + result.channel.c3*w_self;
		result.channel.oceanic = (p0.channel.oceanic+p1.channel.oceanic+p2.channel.oceanic+p3.channel.oceanic+p4.channel.oceanic+p5.channel.oceanic)*w_others+result.channel.oceanic*w_self;
		
		float current_height = result.height;
		
		result.height = height<CLAMP_HEIGHT>((const TChannelConfig&)result.channel,context.has_canyons);
		float crater_depth = 0;
		for (unsigned i = 0; i < crater_count; i++)
		{
			float imprint,strength;
			resolveCraterDelta(result.position,crater_field[i],imprint,strength);
			crater_depth *= (1.0f-strength);
			crater_depth += imprint;
		}
		result.height+=crater_depth;
		if (context.has_ocean && fabs(result.height) < 0.01f)	//shore
		{
			result.height*=100.0f;
			result.height = M::Sign(result.height) * shoreFactor(fabs(result.height));
			//result.height = result.height*result.height*result.height;
			result.height/=100.0f;
		}		
		
		Height::applyDifferentially(result.position,current_height,result.height,segment,context);
	}


	void		generate6position(const TVertex&p0, const TVertex&p1, const TVertex&p2, const TVertex&p3, const TVertex&p4, const TVertex&p5, VMOD TVertex&result, unsigned seed, const TSurfaceSegment&segment, const TContext&context, const TCrater*crater_field, count_t crater_count)
	{
		float distance = M::Vec::distance(p0.position,result.position);

		
		float noise = getNoise(distance,segment,context)*FRACTAL_DEGENERATION;

		float weight= SECONDARY_PARENT_WEIGHT;

		int random_seed = seed;

		static const float	w_total = 6.0f*FRACTAL_DEGENERATION+1.0f,
							w_others = FRACTAL_DEGENERATION/w_total,
							w_self = 1.0f/w_total;
				
		result.channel.c0 = M::Clamp((p0.channel.c0 + p1.channel.c0 + p2.channel.c0 + p3.channel.c0 + p4.channel.c0 + p5.channel.c0) * w_others + result.channel.c0 * w_self + noise * getRandom(random_seed)
			#ifdef C0_CUSTOM_NOISE_FACTOR
				*C0_CUSTOM_NOISE_FACTOR
			#endif
			,0,1);
		result.channel.c1 = M::Clamp((p0.channel.c1+p1.channel.c1+p2.channel.c1+p3.channel.c1+p4.channel.c1+p5.channel.c1)*w_others+result.channel.c1*w_self+noise*getRandom(random_seed)
			#ifdef C1_CUSTOM_NOISE_FACTOR
				*C1_CUSTOM_NOISE_FACTOR
			#endif
			,0,1);
		result.channel.c2 = (p0.channel.c2+p1.channel.c2+p2.channel.c2+p3.channel.c2+p4.channel.c2+p5.channel.c2)*w_others + result.channel.c2*w_self;
		result.channel.c3 = M::Clamp((p0.channel.c3+p1.channel.c3+p2.channel.c3+p3.channel.c3+p4.channel.c3+p5.channel.c3)*w_others + result.channel.c3*w_self+noise*getRandom(random_seed)
			#ifdef C3_CUSTOM_NOISE_FACTOR
				*C3_CUSTOM_NOISE_FACTOR
			#endif
			,0,1);
		result.channel.oceanic = M::Clamp((p0.channel.oceanic+p1.channel.oceanic+p2.channel.oceanic+p3.channel.oceanic+p4.channel.oceanic+p5.channel.oceanic)*w_others+result.channel.oceanic*w_self+noise*getRandom(random_seed)*context.oceanic_noise_level,0,1);
		
		float	riff_distance = fabsf(0.5f-result.channel.c2)*5.0f,
				riff_noise = noise*getRandom(random_seed)
				#ifdef C2_CUSTOM_NOISE_FACTOR
					*C2_CUSTOM_NOISE_FACTOR
				#endif
				;

		if (context.has_canyons && riff_distance>=0.02 && riff_distance < 0.13)
			riff_noise/=(1.0f+0.25*result.channel.c3);
		
		
		result.channel.c2 = M::Clamp(result.channel.c2+riff_noise,0,1);


		//result.channel.weight = weightAt(surface.depth);

		
		
		//result.channel.water = 0;//(w0*p0.channel.water+w1*p1.channel.water+w2*p2.channel.water+w3*p3.channel.water);
		
		
		/*{
			// smooth channels: doesn't work
			
			float smooth_strength = getSmoothStrength(distance,p0,p1,surface,context);
			
			float	w2 = 0.25f,
					w3 = 0.25f,
					w0 = 0.25f,
					w1 = 0.25f;
			
			result.channel.c0 = (1.0f-smooth_strength)*result.channel.c0+(smooth_strength*(p0.channel.c0*w0+p1.channel.c0*w1+p2.channel.c0*w2+p3.channel.c0*w3));
			result.channel.c1 = (1.0f-smooth_strength)*result.channel.c1+(smooth_strength*(p0.channel.c1*w0+p1.channel.c1*w1+p2.channel.c1*w2+p3.channel.c1*w3));
			result.channel.c2 = (1.0f-smooth_strength)*result.channel.c2+(smooth_strength*(p0.channel.c2*w0+p1.channel.c2*w1+p2.channel.c2*w2+p3.channel.c2*w3));
			#if FRACTAL_KERNEL!=MINIMAL_KERNEL
				result.channel.c3 = (1.0f-smooth_strength)*result.channel.c3+(smooth_strength*(p0.channel.c3*w0+p1.channel.c3*w1+p2.channel.c3*w2+p3.channel.c3*w3));
			#endif
			#if FRACTAL_KERNEL==FULL_KERNEL
				result.channel.c4 = (1.0f-smooth_strength)*result.channel.c4+(smooth_strength*(p0.channel.c4*w0+p1.channel.c4*w1+p2.channel.c4*w2+p3.channel.c4*w3));
			#endif		
		}*/
		
		#ifdef FRACTAL_RIVERS
			float mid = p0.channel.river_depth*0.5f+p1.channel.river_depth*0.5f;
			result.channel.river_depth = M::Clamp( mid*0.5f+ 0.5f*(p0.channel.river_depth*w0 + p1.channel.river_depth*w1 + p2.channel.river_depth*w2 + p3.channel.river_depth*w3)-fabsf(noise*10.0f*getRandom(random_seed)),0,1);
			//result.channel.river_depth = p0.channel.river_depth*0.5f+p1.channel.river_depth*0.5f;
			//M::Min((p0.channel.river_depth*w0 + p1.channel.river_depth*w1 + p2.channel.river_depth*w2 + p3.channel.river_depth*w3),1.0f);
			
			/*if (surface.depth == context.recursive_depth-4 && getRandom(random_seed) < -0.9)
				result.channel.river_depth = 0.75f;*/
					
		#endif
		
		float current_height = result.height;
		
		result.height = height<CLAMP_HEIGHT>((const TChannelConfig&)result.channel,context.has_canyons);
		float crater_depth = 0;
		for (unsigned i = 0; i < crater_count; i++)
		{
			float imprint,strength;
			resolveCraterDelta(result.position,crater_field[i],imprint,strength);
			crater_depth *= (1.0f-strength);
			crater_depth += imprint;
		}
		result.height+=crater_depth;
		if (context.has_ocean && fabs(result.height) < 0.01f)	//shore
		{
			result.height*=100.0f;
			result.height = M::Sign(result.height) * shoreFactor(fabs(result.height));
			//result.height = result.height*result.height*result.height;
			result.height/=100.0f;
		}		
		
		Height::applyDifferentially(result.position,current_height,result.height,segment,context);
	}



	
	namespace Kernel
	{

		M::SSE::version_t	current_sse_version = M::SSE::V3;
		volatile bool	background=false;	//!< Indicates that the reduced thread loop should be used. This frees one core for other tasks. False by default


		/**
			@brief Determines the number of actual iterations necessary to cover the specified number of raw iterations, given a number of executions per actual iteration
			
			
			@param raw_iterations Number of iterations that must be covered
			@param iterations_per_block Number of raw iterations that can be performed parallely per actual iteration
			
			@return ceil(raw_iterations/iterations_per_block) although performed as an integer operation
		*/
		static inline unsigned numBlockIterations(unsigned raw_iterations, unsigned iterations_per_block=4)
		{
			unsigned result = raw_iterations/iterations_per_block;
			if (raw_iterations%iterations_per_block)
				result++;
			return result;
		}
	
		namespace Operation
		{
	
			const TVertex empty_vertex = { 0,0,0,0,0,0,0,0,0,0,0,0,0 };
			//static System::HyperKernel		kernel(2);//no speed gain
			//static System::Kernel		kernel;

			//static System::ParallelLoop	loop,reduced_loop(NULL,NULL,true);
			//static System::ThreadPool	kernel(1);
			
			
			
			//static THostSurface			*signal_surface = NULL;
			static System::Mutex				mutex;
			
			//static UnifiedOperationParameter		parameter;
			
			//static pOperation						operation;
			//static unsigned							num_iterations;
			//static const unsigned					job_size=128;
			//static const unsigned					job_size=64;
			//static ThreadPoolContext				context;
			
			
			/*
			static	pOperation						subdivideVertexBlock = subdivideVertexBlockSSE3,	//subdivide method used if vertex normals are available
													subdivideParentVertexBlock = subdivideParentVertexBlockSSE3,
													topLevelSubdivideVertexBlock = subdivideVertexBlockSSE3;	//subdivide method used if vertex normals are not available. At this point identical to the standard subdivision method since normals are not used
		
			class Job:public ThreadMain
			{
			public:
					unsigned		first;
		
		
									Job(unsigned index):first(index*job_size)
									{}
								
					void			ThreadMain()
					{
						for (unsigned i = first; i < first+job_size && i < num_iterations; i++)
							operation(i);
					};
			};*/
		

	
	

			PARALLEL_OPERATION	copyParentVertex(unsigned iteration,const UnifiedOperationParameter&parameter)
			{
				const VertexMap&map = *parameter.map;
				const VertexMap::TParentInfo&info = map.parent_vertex_info[iteration];
				VMOD TVertex&result = parameter.generateVertex.vertex_field[info.index];
				unsigned parent_index = info.origin[parameter.segment->child_index];
				VMOD const TVertex*parent_vertex_field = (VMOD const TVertex*)parameter.generateVertex.parent_vertex_field;
				result = parent_vertex_field[parent_index];
/*				result.channel = parent_vertex_field[parent_index].channel;
				result.height = parent_vertex_field[parent_index].height;
				_c3(parent_vertex_field[parent_index].position,result.position);
				_c3(parent_vertex_field[parent_index].normal,result.normal);*/
				
				M::Vec::add(result.position,parameter.generateVertex.transition);
				
				
				#if 0
				float h,min=1,max=-1,rng=0,avg=0;
				const unsigned int	x = map.vertex_descriptor[parent_index].x,
									y = map.vertex_descriptor[parent_index].y;
				
				unsigned y_max = map.vertex_maximum;
				{
					if (x)
					{
						h = parent_vertex_field[vertexIndex(x-1,y)].FRACTAL_HEIGHT;
						
						if (h < min)
							min = h;
						if (h > max)
							max = h;
						
						if (y)
						{
							h = parent_vertex_field[vertexIndex(x-1,y-1)].FRACTAL_HEIGHT;
							if (h < min)
								min = h;
							if (h > max)
								max = h;
						}
					}
					if (x < y)
					{
						h = parent_vertex_field[vertexIndex(x+1,y)].FRACTAL_HEIGHT;
						if (h < min)
							min = h;
						if (h > max)
							max = h;
						
						if (y)
						{
							h = parent_vertex_field[vertexIndex(x,y-1)].FRACTAL_HEIGHT;
							if (h < min)
								min = h;
							if (h > max)
								max = h;
						}
					}
					if (y < y_max)
					{
						h = parent_vertex_field[vertexIndex(x,y+1)].FRACTAL_HEIGHT;
						if (h < min)
							min = h;
						if (h > max)
							max = h;
						
						h = parent_vertex_field[vertexIndex(x+1,y+1)].FRACTAL_HEIGHT;
						if (h < min)
							min = h;
						if (h > max)
							max = h;
					}
					avg = (min+max)/2.0f;
					rng = M::vmax((max-min)/2.0f,0.001f);//0.75f;
				}
				
				
				float depth = (float)Operation::parameter.segment->depth/Operation::parameter.context->recursive_depth;
				result.channel.weight = getWeight(result.FRACTAL_HEIGHT,avg,rng,depth);
				#endif
				
			}
			
			PARALLEL_OPERATION	subdivideVertex(unsigned iteration,const UnifiedOperationParameter&parameter)
			{
				VMOD TVertex*field = parameter.generateVertex.vertex_field;
				const VertexMap&map = *parameter.map;
				unsigned range = map.child_vertex_count;
				const TContext&context = *parameter.context;
				const TSurfaceSegment&segment = *parameter.segment;
			

				unsigned	linear = map.child_vertex_index[iteration];
							
				const TVertexDescriptor&info = map.vertex_descriptor[linear];
				
				
				VMOD TVertex&result = field[linear];
				
				
				const TVertex	&p0 = (const TVertex&)field[info.parent0],
								&p1 = (const TVertex&)field[info.parent1],
								&p2 = (const TVertex&)field[info.parent2];

				
				/*for (unsigned i = 0; i < linear; i++)
					getRandom(seed);*/
				
				if (info.borderDirection != -1)
				{
					
					unsigned elinear;
					switch (info.borderDirection)
					{
						case 0:
							elinear = info.y;
						break;
						case 1:
							elinear = info.x;
						break;
						case 2:
							elinear = context.edge_length-info.x-1;
						break;
					}
					if (xCoord(p0.position) > xCoord(p1.position))
						elinear = context.edge_length-elinear-1;
							
					int seed = segment.outer_int_seed[info.borderDirection]+seedDelta(elinear);
					generate3center(p0,p1,p2,result,seed,segment,context,parameter.generateVertex.crater_field,parameter.generateVertex.crater_count );
				}
				else
				{
					int seed = segment.inner_int_seed+seedDelta(linear);
					generate4center(p0,p1,p2,(const TVertex&)field[info.parent3],result,seed,segment,context,parameter.generateVertex.crater_field,parameter.generateVertex.crater_count);
				}
					//generate4(p0,p1,p2,parent.vertex[info.parent_space[segment.child_index].parent3],result,segment.inner_int_seed+(((linear*15+3)*(linear*11+2))&0xFFFF),segment,context);
					//generate4(p0,p1,p2,parent.vertex[info.parent_space[segment.child_index].parent3],result,segment.inner_int_seed+((int*)p0.position)[0]+((int*)p0.position)[1]*3+((int*)p0.position)[2]*7,segment,context);

				//result.height = height(result.channel);
				//_center(p0.position,p1.position,result.position);
				
				
				
				Height::applyDifferentially(result.position,(p0.height+p1.height)/2,result.height,segment,context);
				
			}


			PARALLEL_OPERATION degenerateVertexBlock(unsigned iteration,const UnifiedOperationParameter&parameter)
			{
				using namespace M::SSE;

				ALIGNED16 float interchange[4];
					
					
				VMOD TVertex*field = parameter.generateVertex.vertex_field;
				const VertexMap&map = *parameter.map;
				unsigned range = map.inner_parent_vertex_count;
				const TContext&context = *parameter.context;
				const TSurfaceSegment&segment = *parameter.segment;
				
				unsigned	index[4] = 
							{
								map.inner_parent_vertex_info[iteration*4].index,
								map.inner_parent_vertex_info[(iteration*4+1)%range].index,
								map.inner_parent_vertex_info[(iteration*4+2)%range].index,
								map.inner_parent_vertex_info[(iteration*4+3)%range].index
							};
					
				const TMapVertex	*info[4] = 
									{
										map.vertex_descriptor + index[0],
										map.vertex_descriptor + index[1],
										map.vertex_descriptor + index[2],
										map.vertex_descriptor + index[3]
									};
				
				VMOD TVertex	*result[4] =
								{
									field+index[0],
									field+index[1],
									field+index[2],
									field+index[3]
								};


				const TVertex	*p0[4],*p1[4],*p2[4],*p3[4],*p4[4],*p5[4],*p6[4];	//p6 is the result itself
				#undef FILL_PARENTS
				#define FILL_PARENTS(_INDEX_)	\
					p0[_INDEX_] = (const TVertex*)(field+info[_INDEX_]->child[0]);\
					p1[_INDEX_] = (const TVertex*)(field+info[_INDEX_]->child[1]);\
					p2[_INDEX_] = (const TVertex*)(field+info[_INDEX_]->child[2]);\
					p3[_INDEX_] = (const TVertex*)(field+info[_INDEX_]->child[3]);\
					p4[_INDEX_] = (const TVertex*)(field+info[_INDEX_]->child[4]);\
					p5[_INDEX_] = (const TVertex*)(field+info[_INDEX_]->child[5]);\
					p6[_INDEX_] = result[_INDEX_];

				FILL_PARENTS(0)
				FILL_PARENTS(1)
				FILL_PARENTS(2)
				FILL_PARENTS(3)


					
				M::TVec3<SSE_VECTOR> v = {
									_mm_setr_ps(result[0]->position.x,result[1]->position.x,result[2]->position.x,result[3]->position.x),
									_mm_setr_ps(result[0]->position.y,result[1]->position.y,result[2]->position.y,result[3]->position.y),
									_mm_setr_ps(result[0]->position.z,result[1]->position.z,result[2]->position.z,result[3]->position.z)
								},
							p0_position = {
									_mm_setr_ps(p0[0]->position.x,p0[1]->position.x,p0[2]->position.x,p0[3]->position.x),
									_mm_setr_ps(p0[0]->position.y,p0[1]->position.y,p0[2]->position.y,p0[3]->position.y),
									_mm_setr_ps(p0[0]->position.z,p0[1]->position.z,p0[2]->position.z,p0[3]->position.z)
								},
							d =
								{
									v.x-p0_position.x,
									v.y-p0_position.y,
									v.z-p0_position.z
								};
				SSE_VECTOR	distance = _mm_sqrt_ps( M::Vec::dot(d)),
							current_height = _mm_setr_ps(result[0]->FRACTAL_HEIGHT,result[1]->FRACTAL_HEIGHT,result[2]->FRACTAL_HEIGHT,result[3]->FRACTAL_HEIGHT);

					
					

					
				#undef SET_PARENT_CHANNEL
				#define SET_PARENT_CHANNEL(_INDEX_,_CHANNEL_)\
					__m128 p##_INDEX_##_##_CHANNEL_ = _mm_setr_ps(p##_INDEX_[0]->channel._CHANNEL_, p##_INDEX_[1]->channel._CHANNEL_,p##_INDEX_[2]->channel._CHANNEL_,p##_INDEX_[3]->channel._CHANNEL_);

					
				#undef SET_PARENT
				#define SET_PARENT(_INDEX_)\
					SET_PARENT_CHANNEL(_INDEX_,c0)\
					SET_PARENT_CHANNEL(_INDEX_,c1)\
					SET_PARENT_CHANNEL(_INDEX_,c2)\
					SET_PARENT_CHANNEL(_INDEX_,c3)\
					SET_PARENT_CHANNEL(_INDEX_,oceanic)\
					SET_PARENT_CHANNEL(_INDEX_,water)
						//SET_PARENT_CHANNEL(_INDEX_,weight)

				SET_PARENT(0)
				SET_PARENT(1)
				SET_PARENT(2)
				SET_PARENT(3)
				SET_PARENT(4)
				SET_PARENT(5)
				SET_PARENT(6)

					
					
					
				#undef SET_CHANNEL

				float weight_sum = FRACTAL_DEGENERATION*6.0f+1.0f;	//six children to randomize from, each at FRACTAL_DEGENERATION*100%, self at 100%
				__m128	
						general_weight = _mm_set1_ps(FRACTAL_DEGENERATION/weight_sum),
					/*	w0 = general_weight,
						w1 = general_weight,
						w2 = general_weight,
						w3 = general_weight,
						w4 = general_weight,
						w5 = general_weight,*/
						w6 = _mm_set1_ps(1.0f/weight_sum);
				#define SET_CHANNEL(_CHANNEL_)\
					__m128 interpolated_##_CHANNEL_ = \
										_mm_add_ps(\
											_mm_mul_ps(\
												_mm_add_ps(\
													_mm_add_ps(\
														_mm_add_ps(p0_##_CHANNEL_, p1_##_CHANNEL_),\
														_mm_add_ps(p2_##_CHANNEL_, p3_##_CHANNEL_)\
													),\
													_mm_add_ps( p4_##_CHANNEL_, p5_##_CHANNEL_)\
												),\
												general_weight\
											),\
											_mm_mul_ps(w6,p6_##_CHANNEL_)\
										);
											

				SET_CHANNEL(c0)
				SET_CHANNEL(c1)
				SET_CHANNEL(c2)
				SET_CHANNEL(c3)
				SET_CHANNEL(oceanic)

				
				

				#define EXPORT_INTERPOLATED_CHANNEL(_CHANNEL_)\
					_mm_store_ps((float*)interchange,interpolated_##_CHANNEL_);\
					result[0]->channel._CHANNEL_ = interchange[0];\
					result[1]->channel._CHANNEL_ = interchange[1];\
					result[2]->channel._CHANNEL_ = interchange[2];\
					result[3]->channel._CHANNEL_ = interchange[3];
					
				
				EXPORT_INTERPOLATED_CHANNEL(c0)
				EXPORT_INTERPOLATED_CHANNEL(c1)
				EXPORT_INTERPOLATED_CHANNEL(c2)
				EXPORT_INTERPOLATED_CHANNEL(c3)
				EXPORT_INTERPOLATED_CHANNEL(oceanic)

					
				polyHeight<CLAMP_HEIGHT>((const TChannelConfig&)result[0]->channel,(const TChannelConfig&)result[1]->channel,(const TChannelConfig&)result[2]->channel,(const TChannelConfig&)result[3]->channel,context.has_canyons,interchange);
					
				SSE_VECTOR h = _mm_load_ps((float*)interchange);
					
				SSE_VECTOR crater_depth = _mm_setzero_ps();
				for (unsigned i = 0; i < parameter.generateVertex.crater_count; i++)
				{
					SSE_VECTOR imprint, strength;
					resolveCraterDeltaBlock(v.x,v.y,v.z,parameter.generateVertex.sse_crater_field[i],imprint,strength);
					crater_depth *= (M::SSE::one-strength);
					crater_depth += imprint;
				}
				h+=crater_depth;
					
				if (context.has_ocean)
				{
					__m128 shore = M::SSE::factorCompareGreater(_mm_set1_ps(0.01f),M::SSE::Abs(h)),
							shore_relative = _mm_mul_ps(h, M::SSE::hundred);
							
					__m128 sign = _mm_sub_ps(_mm_mul_ps(M::SSE::factorCompareGreater(shore_relative,M::SSE::zero),M::SSE::two),M::SSE::one);
					shore_relative = _mm_mul_ps(SSE::shoreFactor(M::SSE::Abs(shore_relative)),sign);
					//shore_relative = _mm_mul_ps(_mm_mul_ps(shore_relative,shore_relative),shore_relative);


					shore_relative = _mm_div_ps(shore_relative,M::SSE::hundred);
					h = _mm_add_ps(
							_mm_mul_ps(h,_mm_sub_ps(M::SSE::one,shore)),
							_mm_mul_ps(shore_relative,shore)
							);	
				}
				_mm_store_ps((float*)interchange,h);
					

				result[0]->FRACTAL_HEIGHT = interchange[0];
				result[1]->FRACTAL_HEIGHT = interchange[1];
				result[2]->FRACTAL_HEIGHT = interchange[2];
				result[3]->FRACTAL_HEIGHT = interchange[3];
					

				SSE_VECTOR	delta = (h - current_height)*context.sse_variance;
				M::TVec3<SSE_VECTOR>	vd = {
										v.x + _mm_set1_ps((float)segment.sector.x*context.sector_size),
										v.y + _mm_set1_ps((float)segment.sector.y*context.sector_size),
										v.z + _mm_set1_ps((float)segment.sector.z*context.sector_size)
									};
				SSE_VECTOR	factor = _mm_rsqrt_ps( vd.x*vd.x + vd.y*vd.y + vd.z*vd.z)*delta;
						
				M::TVec3<SSE_VECTOR> final;
				M::Vec::mad(v,vd,factor,final);
					
				_mm_store_ps((float*)interchange,final.x);
				result[0]->position.x = interchange[0];
				result[1]->position.x = interchange[1];
				result[2]->position.x = interchange[2];
				result[3]->position.x = interchange[3];

				_mm_store_ps((float*)interchange,final.y);
				result[0]->position.y = interchange[0];
				result[1]->position.y = interchange[1];
				result[2]->position.y = interchange[2];
				result[3]->position.y = interchange[3];

				_mm_store_ps((float*)interchange,final.z);
				result[0]->position.z = interchange[0];
				result[1]->position.z = interchange[1];
				result[2]->position.z = interchange[2];
				result[3]->position.z = interchange[3];

			}





			

			


			PARALLEL_OPERATION	generateNormal(unsigned iteration,const UnifiedOperationParameter&parameter)
			{
				TVertex*vertices = parameter.generateNormal.vertex_field;
				const VertexMap&map = *parameter.map;

				unsigned max_row = map.vertex_maximum;
				const TVertexDescriptor	&info = map.vertex_descriptor[iteration];
				
				TVertex	&v = vertices[iteration];
				M::TVec3<>&normal = v.normal;
				const M::TVec3<>&vp = v.position;
				M::Vec::clear(normal);
			
				
				if (info.y > 0)
				{
					if (info.x > 0)
					{
						_oAddTriangleNormal(vertices[vertexIndex(info.x-1,info.y-1)].position,vertices[vertexIndex(info.x-1,info.y)].position,vp,normal);
					}

					if (info.x < info.y)
					{
						if (info.x > 0)
						{
							_oAddTriangleNormal((vertices[vertexIndex(info.x,info.y-1)].position),(vertices[vertexIndex(info.x-1,info.y-1)].position),vp,normal);
						}
						_oAddTriangleNormal((vertices[vertexIndex(info.x+1,info.y)].position),(vertices[vertexIndex(info.x,info.y-1)].position),vp,normal);
					}
				}
				
				if (info.y < max_row)
				{
					if (info.x > 0)
					{
						_oAddTriangleNormal((vertices[vertexIndex(info.x-1,info.y)].position),(vertices[vertexIndex(info.x,info.y+1)].position),vp,normal);
					}
					_oAddTriangleNormal((vertices[vertexIndex(info.x,info.y+1)].position),(vertices[vertexIndex(info.x+1,info.y+1)].position),vp,normal);
					if (info.x < info.y)
						_oAddTriangleNormal((vertices[vertexIndex(info.x+1,info.y+1)].position),(vertices[vertexIndex(info.x+1,info.y)].position),vp,normal);
				}
				
			}

			PARALLEL_OPERATION	generateNormalBlock(unsigned iteration,const UnifiedOperationParameter&parameter)
			{
				TVertex*vertices = parameter.generateNormal.vertex_field;
				const VertexMap&map = *parameter.map;

				unsigned max_row = map.vertex_maximum;
				const TVertexDescriptor	&info0 = map.vertex_descriptor[(iteration<<2)],
									&info1 = map.vertex_descriptor[((iteration<<2)+1)%map.vertex_count],
									&info2 = map.vertex_descriptor[((iteration<<2)+2)%map.vertex_count],
									&info3 = map.vertex_descriptor[((iteration<<2)+3)%map.vertex_count];
				
				TVertex	&v0 = vertices[(iteration<<2)],
						&v1 = vertices[((iteration<<2)+1)%map.vertex_count],
						&v2 = vertices[((iteration<<2)+2)%map.vertex_count],
						&v3 = vertices[((iteration<<2)+3)%map.vertex_count];
				
				M::TVec3<SSE_VECTOR>	normal = {
								_mm_setzero_ps(),
								_mm_setzero_ps(),
								_mm_setzero_ps()
								},
						m = {
								_mm_setr_ps(v0.position.x,v1.position.x,v2.position.x,v3.position.x),
								_mm_setr_ps(v0.position.y,v1.position.y,v2.position.y,v3.position.y),
								_mm_setr_ps(v0.position.z,v1.position.z,v2.position.z,v3.position.z)
								};
				#undef SET_OUTER0
				#undef SET_OUTER1
				#undef SET_OUTER2
				#undef SET_OUTER3
				#undef SET_OUTER4
				#undef SET_OUTER5
				
				#define SET_OUTER0(_INDEX_)\
					const M::TVec3<>&outer##_INDEX_##_0 = info##_INDEX_.x > 0 && info##_INDEX_.y > 0?vertices[vertexIndex(info##_INDEX_.x-1,info##_INDEX_.y-1)].position:v##_INDEX_.position;
					
				#define SET_OUTER1(_INDEX_)\
					const M::TVec3<>&outer##_INDEX_##_1 = info##_INDEX_.x > 0?vertices[vertexIndex(info##_INDEX_.x-1,info##_INDEX_.y)].position:v##_INDEX_.position;
				
				#define SET_OUTER2(_INDEX_)\
					const M::TVec3<>&outer##_INDEX_##_2 = info##_INDEX_.y < max_row?vertices[vertexIndex(info##_INDEX_.x,info##_INDEX_.y+1)].position:v##_INDEX_.position;
				
				#define SET_OUTER3(_INDEX_)\
					const M::TVec3<>&outer##_INDEX_##_3 = info##_INDEX_.y < max_row?vertices[vertexIndex(info##_INDEX_.x+1,info##_INDEX_.y+1)].position:v##_INDEX_.position;
					
				#define SET_OUTER4(_INDEX_)\
					const M::TVec3<>&outer##_INDEX_##_4 = info##_INDEX_.x < info##_INDEX_.y?vertices[vertexIndex(info##_INDEX_.x+1,info##_INDEX_.y)].position:v##_INDEX_.position;
				
				#define SET_OUTER5(_INDEX_)\
					const M::TVec3<>&outer##_INDEX_##_5 = info##_INDEX_.x < info##_INDEX_.y?vertices[vertexIndex(info##_INDEX_.x,info##_INDEX_.y-1)].position:v##_INDEX_.position;
					

				SET_OUTER0(0)
				SET_OUTER0(1)
				SET_OUTER0(2)
				SET_OUTER0(3)
				
				SET_OUTER1(0)
				SET_OUTER1(1)
				SET_OUTER1(2)
				SET_OUTER1(3)
				
				SET_OUTER2(0)
				SET_OUTER2(1)
				SET_OUTER2(2)
				SET_OUTER2(3)
				
				SET_OUTER3(0)
				SET_OUTER3(1)
				SET_OUTER3(2)
				SET_OUTER3(3)
				
				SET_OUTER4(0)
				SET_OUTER4(1)
				SET_OUTER4(2)
				SET_OUTER4(3)
				
				SET_OUTER5(0)
				SET_OUTER5(1)
				SET_OUTER5(2)
				SET_OUTER5(3)
						
				M::TVec3<SSE_VECTOR>
						o0 = {
									_mm_setr_ps(outer0_0.x,outer1_0.x,outer2_0.x,outer3_0.x),
									_mm_setr_ps(outer0_0.y,outer1_0.y,outer2_0.y,outer3_0.y),
									_mm_setr_ps(outer0_0.z,outer1_0.z,outer2_0.z,outer3_0.z)
								},
						o1 = {
									_mm_setr_ps(outer0_1.x,outer1_1.x,outer2_1.x,outer3_1.x),
									_mm_setr_ps(outer0_1.y,outer1_1.y,outer2_1.y,outer3_1.y),
									_mm_setr_ps(outer0_1.z,outer1_1.z,outer2_1.z,outer3_1.z)
								},
						o2 = {
									_mm_setr_ps(outer0_2.x,outer1_2.x,outer2_2.x,outer3_2.x),
									_mm_setr_ps(outer0_2.y,outer1_2.y,outer2_2.y,outer3_2.y),
									_mm_setr_ps(outer0_2.z,outer1_2.z,outer2_2.z,outer3_2.z)
								},
						o3 = {
									_mm_setr_ps(outer0_3.x,outer1_3.x,outer2_3.x,outer3_3.x),
									_mm_setr_ps(outer0_3.y,outer1_3.y,outer2_3.y,outer3_3.y),
									_mm_setr_ps(outer0_3.z,outer1_3.z,outer2_3.z,outer3_3.z)
								},
						o4 = {
									_mm_setr_ps(outer0_4.x,outer1_4.x,outer2_4.x,outer3_4.x),
									_mm_setr_ps(outer0_4.y,outer1_4.y,outer2_4.y,outer3_4.y),
									_mm_setr_ps(outer0_4.z,outer1_4.z,outer2_4.z,outer3_4.z)
								},
						o5 = {
									_mm_setr_ps(outer0_5.x,outer1_5.x,outer2_5.x,outer3_5.x),
									_mm_setr_ps(outer0_5.y,outer1_5.y,outer2_5.y,outer3_5.y),
									_mm_setr_ps(outer0_5.z,outer1_5.z,outer2_5.z,outer3_5.z)
								};
				
				_oAddTriangleNormal(o0, o1, m, normal);
				_oAddTriangleNormal(o1, o2, m, normal);
				_oAddTriangleNormal(o2, o3, m, normal);
				_oAddTriangleNormal(o3, o4, m, normal);
				_oAddTriangleNormal(o4, o5, m, normal);
				_oAddTriangleNormal(o5, o0, m, normal);
				
				ALIGNED16	float	exported[16];
				
				_mm_store_ps(exported,normal.x);
				v0.normal.x = exported[0];
				v1.normal.x = exported[1];
				v2.normal.x = exported[2];
				v3.normal.x = exported[3];

				_mm_store_ps(exported,normal.y);
				v0.normal.y = exported[0];
				v1.normal.y = exported[1];
				v2.normal.y = exported[2];
				v3.normal.y = exported[3];

				_mm_store_ps(exported,normal.z);
				v0.normal.z = exported[0];
				v1.normal.z = exported[1];
				v2.normal.z = exported[2];
				v3.normal.z = exported[3];
				
				/*if (_dot(v0.normal) <= GetError<float>())
					v0.normal[0] = 1;
				if (_dot(v1.normal) <= GetError<float>())
					v1.normal[0] = 1;
				if (_dot(v2.normal) <= GetError<float>())
					v2.normal[0] = 1;
				if (_dot(v3.normal) <= GetError<float>())
					v3.normal[0] = 1;*/
					

			}

			PARALLEL_OPERATION	mergeEdge(unsigned linear, const UnifiedOperationParameter&parameter)
			{
				TVertex	*these_vertices = parameter.mergeEdge.vertex_field,
						*those_vertices = parameter.mergeEdge.neighbor_vertex_field;
				const TSurfaceSegment&this_surface = *parameter.segment;
				const VertexMap&map = *parameter.map;
				const unsigned	*edge = parameter.mergeEdge.edge,
								*neighbor_edge = parameter.mergeEdge.neighbor_edge,
								edge_length = map.vertex_range;
				const TMapVertex*info_field = map.vertex_descriptor;
				
				
			
				const TVertexDescriptor&this_info = info_field[edge[linear]];

				TVertex	&this_v = these_vertices[edge[linear]],
						&that_v = those_vertices[neighbor_edge[edge_length-linear-1]];
				

				if (this_info.grid_vertex)
				{
					//can't process this here due to order of execution
				}
				else
				{
					const TVertexDescriptor&that_info = info_field[neighbor_edge[edge_length-linear-1]];
					
					//bool forward = xCoord(these_vertices[this_info.parent0].position) < xCoord(these_vertices[this_info.parent1].position);
					unsigned edge_linear = linear;
					/*if (!forward)
						edge_linear = edge_length-1-linear;*/
					
					int seed = this_surface.outer_int_seed[this_info.borderDirection]+seedDelta(edge_linear);
					{
						TVertex	&p0 = these_vertices[this_info.parent0],
								&p0b = those_vertices[that_info.parent1],
								&p1 = these_vertices[this_info.parent1],
								&p1b = those_vertices[that_info.parent0];
						
						ASSERT3__(M::Similar(p0.height,p0b.height),linear,p0.height,p0b.height);
						ASSERT3__(M::Similar(p1.height,p1b.height),linear,p1.height,p1b.height);
						/*p0b.channel.weight = p0.channel.weight = (p0.channel.weight+p0b.channel.weight)/2.0f;
						p1b.channel.weight = p1.channel.weight = (p1.channel.weight+p1b.channel.weight)/2.0f;*/
						
						const TVertex	&p2 = these_vertices[this_info.parent2],
										&p3 = those_vertices[that_info.parent2];

						generate4center(	p0,p1,
											p2,p3,
											this_v,
											seed,this_surface,*parameter.context,
											parameter.mergeEdge.crater_field,parameter.mergeEdge.crater_count
											);
						//this_v.height = height(this_v.channel);
						//_center(p0.position,p1.position,this_v.position);
						Height::applyDifferentially(this_v.position,(p0.height+p1.height)/2,this_v.height,this_surface,*parameter.context);
					}
					that_v.channel = this_v.channel;
					that_v.height = this_v.height;
					M::Vec::sub((this_v.position),parameter.mergeEdge.edge_transition,(that_v.position));
				}
				M::Vec::add((this_v.normal),(that_v.normal),(this_v.normal));
				M::Vec::copy(this_v.normal,that_v.normal);
			}

			/**
				@brief Second phase of edge merging. Re-subdivides inner parent vertices
			*/
			PARALLEL_OPERATION	mergeEdge2(unsigned linear, const UnifiedOperationParameter&parameter)
			{
				if (!linear || linear == parameter.map->vertex_range-1)	//skip first and last vertex here. Not possible to gather children around corner at this point
					return;

				TVertex	*these_vertices = parameter.mergeEdge.vertex_field,
						*those_vertices = parameter.mergeEdge.neighbor_vertex_field;
				const TSurfaceSegment&this_surface = *parameter.segment;
				const VertexMap&map = *parameter.map;
				const unsigned	*edge = parameter.mergeEdge.edge,
								*neighbor_edge = parameter.mergeEdge.neighbor_edge,
								edge_length = map.vertex_range;
				const TMapVertex*info_field = map.vertex_descriptor;
				
				
			
				const TMapVertex&this_info = info_field[edge[linear]];

				TVertex	&this_v = these_vertices[edge[linear]],
						&that_v = those_vertices[neighbor_edge[edge_length-linear-1]];
				

				if (this_info.grid_vertex)
				{
					const TMapVertex&that_info = info_field[neighbor_edge[edge_length-linear-1]];
					
					//bool forward = xCoord(these_vertices[edge[linear]].position) < xCoord(these_vertices[edge[linear-1]].position);
					unsigned edge_linear = linear;
					/*if (!forward)
						edge_linear = edge_length-1-linear;*/
					
					int seed = this_surface.outer_int_seed[this_info.borderDirection]+seedDelta(edge_linear);
					{
						TVertex	&n0 = these_vertices[this_info.child[0]],
								&n1 = these_vertices[this_info.child[1]],
								&n2 = these_vertices[this_info.child[2]],
								&n3 = these_vertices[this_info.child[3]],
								&n3b = those_vertices[that_info.child[0]],
								&n4b = those_vertices[that_info.child[1]],
								&n5b = those_vertices[that_info.child[2]],
								&n0b = those_vertices[that_info.child[3]];
						/*ASSERT3__(similar(n0.height,n0b.height),linear,n0.height,n0b.height);	never fired
						ASSERT3__(similar(n3.height,n3b.height),linear,n3.height,n3b.height);*/

						/*p0b.channel.weight = p0.channel.weight = (p0.channel.weight+p0b.channel.weight)/2.0f;
						p1b.channel.weight = p1.channel.weight = (p1.channel.weight+p1b.channel.weight)/2.0f;*/
						
//						generate6position(	n0,n1,n2,n3,n4b,n5b,
						degenerate6position(n0,n1,n2,n3,n4b,n5b,
											this_v,
											seed,this_surface,*parameter.context,
											parameter.mergeEdge.crater_field,parameter.mergeEdge.crater_count
											);
						//this_v.height = height(this_v.channel);
						//_center(p0.position,p1.position,this_v.position);
					}
					that_v.channel = this_v.channel;
					that_v.height = this_v.height;
					M::Vec::sub((this_v.position),parameter.mergeEdge.edge_transition,(that_v.position));
				}
				/*_add(this_v.normal,that_v.normal,this_v.normal);
				_c3(this_v.normal,that_v.normal);*/
			}



			PARALLEL_OPERATION	copyEdge(unsigned iteration,const UnifiedOperationParameter&parameter)
			{
				unsigned index = parameter.copyEdge.destination_edge[iteration];
				const TVertexDescriptor&this_info = parameter.map->vertex_descriptor[index];
				TVertex			&destination_v = parameter.copyEdge.destination_vertex_field[parameter.copyEdge.destination_edge[iteration]];
				const TVertex	&source_v = parameter.copyEdge.source_vertex_field[parameter.copyEdge.source_edge[parameter.map->vertex_range-iteration-1]];
				
				//if (!this_info.grid_vertex)
				{
					//ASSERT3__(M::SSE::floatDistance(source_v.position-parameter.copyEdge.edge_transition,destination_v.position)<parameter.context->variance*2,M::SSE::ToString(source_v.position),M::SSE::ToString(parameter.copyEdge.edge_transition),M::SSE::ToString(destination_v.position));
					destination_v.channel = source_v.channel;
					destination_v.height = source_v.height;
					
					M::Vec::sub((source_v.position),parameter.copyEdge.edge_transition,(destination_v.position));
				}
				M::Vec::copy(source_v.normal,destination_v.normal);
			}
			

			PARALLEL_OPERATION	generateCover(unsigned iteration,const UnifiedOperationParameter&parameter)
			{
				TCoverVertex&vout = parameter.generateCover.out_vertices[iteration];
				const TVertex&vertex = parameter.generateCover.in_vertices[parameter.generateCover.index_field[iteration]];

				vout.channel = vertex.channel;
				
				M::Vec::copy(vertex.position,vout.position);
				
				vout.height = vertex.height;
				
				compileAppearance(vertex,*parameter.segment, *parameter.context, vout);
				
			}
			
			PARALLEL_OPERATION	generateFullCoverBlock(unsigned iteration,const UnifiedOperationParameter&parameter)
			{
				unsigned max = parameter.map->vertex_count;
				unsigned	i0 = (iteration<<2),
							i1 = ((iteration<<2)+1)%max,
							i2 = ((iteration<<2)+2)%max,
							i3 = ((iteration<<2)+3)%max;

				/*
				parameter.touched[i0] = true;
				parameter.touched[i1] = true;
				parameter.touched[i2] = true;
				parameter.touched[i3] = true;*/

				TCoverVertex	&vout0 = parameter.generateCover.out_vertices[i0],
								&vout1 = parameter.generateCover.out_vertices[i1],
								&vout2 = parameter.generateCover.out_vertices[i2],
								&vout3 = parameter.generateCover.out_vertices[i3];
				const TVertex	&v0 = parameter.generateCover.in_vertices[i0],
								&v1 = parameter.generateCover.in_vertices[i1],
								&v2 = parameter.generateCover.in_vertices[i2],
								&v3 = parameter.generateCover.in_vertices[i3];
				vout0.channel = v0.channel;
				vout1.channel = v1.channel;
				vout2.channel = v2.channel;
				vout3.channel = v3.channel;
				
				M::Vec::copy(v0.position,vout0.position);
				M::Vec::copy(v1.position,vout1.position);
				M::Vec::copy(v2.position,vout2.position);
				M::Vec::copy(v3.position,vout3.position);
				
				vout0.height = v0.height;
				vout1.height = v1.height;
				vout2.height = v2.height;
				vout3.height = v3.height;


				
				compileAppearanceBlock(v0, v1, v2, v3, *parameter.segment, *parameter.context, vout0, vout1, vout2, vout3);
			}
			
			PARALLEL_OPERATION	generateInnerCoverBlock(unsigned iteration,const UnifiedOperationParameter&parameter)
			{
				
				unsigned	max = parameter.map->inner_vertex_count;
				unsigned	o0 = (iteration<<2),
							o1 = ((iteration<<2)+1)%max,
							o2 = ((iteration<<2)+2)%max,
							o3 = ((iteration<<2)+3)%max;				
				unsigned	i0 = parameter.map->inner_vertex_index[o0],
							i1 = parameter.map->inner_vertex_index[o1],
							i2 = parameter.map->inner_vertex_index[o2],
							i3 = parameter.map->inner_vertex_index[o3];
				TCoverVertex	&vout0 = parameter.generateCover.out_vertices[o0],
								&vout1 = parameter.generateCover.out_vertices[o1],
								&vout2 = parameter.generateCover.out_vertices[o2],
								&vout3 = parameter.generateCover.out_vertices[o3];
				const TVertex	&v0 = parameter.generateCover.in_vertices[i0],
								&v1 = parameter.generateCover.in_vertices[i1],
								&v2 = parameter.generateCover.in_vertices[i2],
								&v3 = parameter.generateCover.in_vertices[i3];
				vout0.channel = v0.channel;
				vout1.channel = v1.channel;
				vout2.channel = v2.channel;
				vout3.channel = v3.channel;
				
				M::Vec::copy(v0.position,vout0.position);
				M::Vec::copy(v1.position,vout1.position);
				M::Vec::copy(v2.position,vout2.position);
				M::Vec::copy(v3.position,vout3.position);
				
				vout0.height = v0.height;
				vout1.height = v1.height;
				vout2.height = v2.height;
				vout3.height = v3.height;
				
				compileAppearanceBlock(v0, v1, v2, v3, *parameter.segment, *parameter.context, vout0, vout1, vout2, vout3);
			}
			
			PARALLEL_OPERATION	generateEdgeCoverBlock(unsigned iteration,const UnifiedOperationParameter&parameter)
			{
				unsigned	max = parameter.map->vertex_maximum;
				unsigned	o0 = (iteration<<2),
							o1 = ((iteration<<2)+1)%max,
							o2 = ((iteration<<2)+2)%max,
							o3 = ((iteration<<2)+3)%max;				
				unsigned	i0 = parameter.generateCover.index_field[o0],
							i1 = parameter.generateCover.index_field[o1],
							i2 = parameter.generateCover.index_field[o2],
							i3 = parameter.generateCover.index_field[o3];
				TCoverVertex	&vout0 = parameter.generateCover.out_vertices[o0],
								&vout1 = parameter.generateCover.out_vertices[o1],
								&vout2 = parameter.generateCover.out_vertices[o2],
								&vout3 = parameter.generateCover.out_vertices[o3];
				const TVertex	&v0 = parameter.generateCover.in_vertices[i0],
								&v1 = parameter.generateCover.in_vertices[i1],
								&v2 = parameter.generateCover.in_vertices[i2],
								&v3 = parameter.generateCover.in_vertices[i3];
				vout0.channel = v0.channel;
				vout1.channel = v1.channel;
				vout2.channel = v2.channel;
				vout3.channel = v3.channel;
				
				M::Vec::copy(v0.position,vout0.position);
				M::Vec::copy(v1.position,vout1.position);
				M::Vec::copy(v2.position,vout2.position);
				M::Vec::copy(v3.position,vout3.position);
				
				vout0.height = v0.height;
				vout1.height = v1.height;
				vout2.height = v2.height;
				vout3.height = v3.height;
				
				compileAppearanceBlock(v0, v1, v2, v3, *parameter.segment, *parameter.context, vout0, vout1, vout2, vout3);
			}
			

			PARALLEL_OPERATION	generateVBO(unsigned iteration,const UnifiedOperationParameter&parameter)
			{
				float*v = parameter.generateVBO.out_vertices+iteration*TSurfaceSegment::floats_per_vbo_vertex;
				unsigned index = parameter.generateVBO.index_field[iteration];
				const TVertex&vertex = parameter.generateVBO.in_vertices[index];
				const VertexMap&map = *parameter.map;
				const TVertexDescriptor&info = map.vertex_descriptor[index];
				M::Vec::copy(vertex.position,M::Vec::ref3(v));
				v[3] = vertex.height;//*context.variance;
				
				resolveDirection(vertex.position,*parameter.segment,*parameter.context,M::Vec::ref3(v+4));
				v[7] = (float)info.x*parameter.generateVBO.stretch;
				v[8] = parameter.generateVBO.base+(float)info.y*parameter.generateVBO.stretch;
			}



			PARALLEL_OPERATION	generateTexel(unsigned iteration,const UnifiedOperationParameter&parameter)
			{
				
				const TMapVertex*	info_field = parameter.generateTexel.info_field;

				const TVertexDescriptor	&info = info_field[iteration];
				BYTE*texel_data = parameter.generateTexel.texel_data;
				/*
					x and y coordinates in the final texture
				*/
				unsigned dimension = parameter.generateTexel.dimension;
				unsigned px = info.x;
				unsigned py = info.y+TEXTURE_SEAM_EXTENSION+1;
				
				bool extent = px+TEXTURE_SEAM_EXTENSION+1 >= py;
				
				if (px >= dimension || py >= dimension)
					FATAL__("Pixel out of bounds: "+String(px)+", "+String(py)+" / "+String(dimension));
				
				unsigned char	*normal_texel = getTexel3(texel_data,px,py,dimension);
							

				const TVertex&vertex = parameter.generateTexel.vertex_field[iteration];
				float nlen = M::Vec::length(vertex.normal);
				
				normal_texel[0] = (unsigned char)(255.0f*(0.5f+vertex.normal.x/nlen*0.5f));
				normal_texel[1] = (unsigned char)(255.0f*(0.5f+vertex.normal.y/nlen*0.5f));
				normal_texel[2] = (unsigned char)(255.0f*(0.5f+vertex.normal.z/nlen*0.5f));
				
				if (extent)
				{
					#if TEXTURE_SEAM_EXTENSION >= 1
						_c3(normal_texel,getTexel3(texel_data,px+1,py,dimension));
						if (py==TEXTURE_SEAM_EXTENSION+1)
							_c3(normal_texel,getTexel3(texel_data,px,py-1,dimension));
					#endif
					#if TEXTURE_SEAM_EXTENSION >= 2
						_c3(normal_texel,getTexel3(texel_data,px+1,py-1,dimension));
					#endif
				}

				px = dimension-px-1;
				py = dimension-py-1;
				/*if (x == y || x == y+1)
					return;*/
				if (px >= dimension || py >= dimension)
					FATAL__("o_O");
				
				
				unsigned char* style_texel = getTexel3(texel_data,px,py,dimension);
				
				#if !defined(COLOR_CHANNELS) || COLOR_CHANNELS>=3
					TVertexAppearance	appearance;
					compileAppearance(vertex, *parameter.segment, *parameter.context, appearance);


					/*
					style_texel[0] = (unsigned char)(255.0f*sand);
					style_texel[1] = (unsigned char)(255.0f*fertility);
					style_texel[2] = (unsigned char)(255.0f*snow);*/
					#if COLOR_CHANNELS == 3
						style_texel[0] = (BYTE)(255*appearance.planarity);
						style_texel[1] = (BYTE)(255*appearance.general_fertility);
						style_texel[2] = (BYTE)(255*appearance.water*0.66);
					#else
						style_texel[0] = (BYTE)(255*appearance.sand);
						style_texel[1] = (BYTE)(255*appearance.fertility);
						style_texel[2] = (BYTE)(255*appearance.snow);
					#endif
				#elif COLOR_CHANNELS == 0
					style_texel[0] = (BYTE)(255*vertex.channel.c0);
					style_texel[1] = (BYTE)(255*vertex.channel.c1);
					style_texel[2] = (BYTE)(255*vertex.channel.c2);
				#elif COLOR_CHANNELS == 1
					style_texel[0] = (BYTE)(255*vertex.channel.c3);
					style_texel[1] = 0;
					style_texel[2] = (BYTE)(255*(vertex.height*0.5+0.5));
				#elif COLOR_CHANNELS == 2
					style_texel[0] = (BYTE)(255*vertex.channel.oceanic);
					style_texel[1] = (BYTE)(255*(vertex.height*0.5+0.5));
					style_texel[2] = (BYTE)(255*vertex.channel.water);

				#endif
				
				if (extent)
				{
					#if TEXTURE_SEAM_EXTENSION >= 1
						_c3(style_texel,getTexel3(texel_data,px-1,py,dimension));
						if (py+TEXTURE_SEAM_EXTENSION+1==dimension-1)
							_c3(style_texel,getTexel3(texel_data,px,py+1,dimension));
					#endif
					#if TEXTURE_SEAM_EXTENSION >= 2
						_c3(style_texel,getTexel3(texel_data,px-1,py+1,dimension));
					#endif
				
					/*style_texel[0] = 255;
					style_texel[1] = 0;
					style_texel[2] = 0;*/
				}	
			}

			static inline float heightError(const TVertex&vertex, const TSurfaceSegment&segment, const TContext&context)
			{
				M::TVec3<double> p;
				M::Vec::mult(segment.sector,context.sector_size,p);
				M::Vec::add(p,vertex.position);
				double h = (M::Vec::length(p)-(double)context.base_heightf)/(double)context.variance;
				return fabs(h-vertex.height)*100;
			}
			
			PARALLEL_OPERATION	generateTexelBlock(unsigned iteration,const UnifiedOperationParameter&parameter)
			{
				
				const TMapVertex*	info_field = parameter.generateTexel.info_field;

				unsigned	dimension = parameter.generateTexel.dimension,
							space = parameter.map->vertex_count;
				
				const TVertexDescriptor	&info0 = info_field[iteration*4],
									&info1 = info_field[(iteration*4+1)%space],
									&info2 = info_field[(iteration*4+2)%space],
									&info3 = info_field[(iteration*4+3)%space];
				BYTE*texel_data = parameter.generateTexel.texel_data;


				/*
					x and y coordinates in the final texture
				*/
			
				#undef SET_TEXEL_COORDS
				#define SET_TEXEL_COORDS(_INDEX_)	unsigned px##_INDEX_ = info##_INDEX_.x, py##_INDEX_ = info##_INDEX_.y+TEXTURE_SEAM_EXTENSION+1;
				
				SET_TEXEL_COORDS(0)
				SET_TEXEL_COORDS(1)
				SET_TEXEL_COORDS(2)
				SET_TEXEL_COORDS(3)
				
				#define SET_TEXEL_EXTEND(_INDEX_)	bool extent##_INDEX_ = px##_INDEX_+TEXTURE_SEAM_EXTENSION+1 >= py##_INDEX_;\
					if (px##_INDEX_ >= dimension || py##_INDEX_ >= dimension)\
						FATAL__("Texel "#_INDEX_" out of bounds: "+String(px##_INDEX_)+", "+String(py##_INDEX_)+" / "+String(dimension));

				SET_TEXEL_EXTEND(0)
				SET_TEXEL_EXTEND(1)
				SET_TEXEL_EXTEND(2)
				SET_TEXEL_EXTEND(3)

				#define SET_TEXEL_VERTEX(_INDEX_)	const TVertex&vertex##_INDEX_ = parameter.generateTexel.vertex_field[(iteration*4+_INDEX_)%space];\
													unsigned char	*normal_texel##_INDEX_ = getTexel3(texel_data,px##_INDEX_,py##_INDEX_,dimension);
				
				SET_TEXEL_VERTEX(0)
				SET_TEXEL_VERTEX(1)
				SET_TEXEL_VERTEX(2)
				SET_TEXEL_VERTEX(3)
				
				
				SSE_VECTOR	nx = SSE_DEFINE4(vertex0.normal.x,vertex1.normal.x,vertex2.normal.x,vertex3.normal.x),
							ny = SSE_DEFINE4(vertex0.normal.y,vertex1.normal.y,vertex2.normal.y,vertex3.normal.y),
							nz = SSE_DEFINE4(vertex0.normal.z,vertex1.normal.z,vertex2.normal.z,vertex3.normal.z),
							nfc = _mm_rsqrt_ps(nx*nx + ny*ny + nz*nz),
							normal_color0 = M::SSE::c128+nx*nfc*M::SSE::c127,
							normal_color1 = M::SSE::c128+ny*nfc*M::SSE::c127,
							normal_color2 = M::SSE::c128+nz*nfc*M::SSE::c127;
							/*normal_color0 = M::SSE::c255 * (M::SSE::half+M::SSE::half*nx*nfc),
							normal_color1 = M::SSE::c255 * (M::SSE::half+M::SSE::half*ny*nfc),
							normal_color2 = M::SSE::c255 * (M::SSE::half+M::SSE::half*nz*nfc);*/
				ALIGNED16	float exported[4];
				
				#define EXPORT_NORMAL_COMPONENT(_COORD_)\
					_mm_store_ps(exported,normal_color##_COORD_);\
					normal_texel0[_COORD_] = (BYTE)exported[0];\
					normal_texel1[_COORD_] = (BYTE)exported[1];\
					normal_texel2[_COORD_] = (BYTE)exported[2];\
					normal_texel3[_COORD_] = (BYTE)exported[3];
				
				EXPORT_NORMAL_COMPONENT(0)
				EXPORT_NORMAL_COMPONENT(1)
				EXPORT_NORMAL_COMPONENT(2)
				
				
				/*
				#define FILL_NORMAL(_INDEX_)\
						{\
							unsigned char	*normal_texel = getTexel3(texel_data,px##_INDEX_,py##_INDEX_,dimension);\
							float nlen = _length(vertex##_INDEX_.normal);\
							normal_texel[0] = (unsigned char)(255.0f*(0.5f+vertex##_INDEX_.normal[0]/nlen*0.5f));\
							normal_texel[1] = (unsigned char)(255.0f*(0.5f+vertex##_INDEX_.normal[1]/nlen*0.5f));\
							normal_texel[2] = (unsigned char)(255.0f*(0.5f+vertex##_INDEX_.normal[2]/nlen*0.5f));\
							EXTEND_NORMAL(_INDEX_)\
						}
				*/		
				
				#if TEXTURE_SEAM_EXTENSION >= 2
					#define EXTEND_NORMAL(_INDEX_)\
						if (extent##_INDEX_)\
						{\
							_c3(normal_texel##_INDEX_,getTexel3(texel_data,px##_INDEX_+1,py##_INDEX_,dimension));\
							if (py##_INDEX_==TEXTURE_SEAM_EXTENSION+1)\
								_c3(normal_texel##_INDEX_,getTexel3(texel_data,px##_INDEX_,py##_INDEX_-1,dimension));\
							_c3(normal_texel##_INDEX_,getTexel3(texel_data,px##_INDEX_+1,py##_INDEX_-1,dimension));\
						}
					#define EXTEND_STYLE_TEXEL(_INDEX_)\
						if (extent##_INDEX_)\
						{\
							_c3(style_texel##_INDEX_,getTexel3(texel_data,px##_INDEX_-1,py##_INDEX_,dimension));\
							if (py##_INDEX_+TEXTURE_SEAM_EXTENSION+1==dimension-1)\
								_c3(style_texel##_INDEX_,getTexel3(texel_data,px##_INDEX_,py##_INDEX_+1,dimension));\
							_c3(style_texel##_INDEX_,getTexel3(texel_data,px##_INDEX_-1,py##_INDEX_+1,dimension));\
						}
						
				#elif TEXTURE_SEAM_EXTENSION >= 1
					#define EXTEND_NORMAL(_INDEX_)\
						if (extent##_INDEX_)\
						{\
							_c3(normal_texel##_INDEX_,getTexel3(texel_data,px##_INDEX_+1,py##_INDEX_,dimension));\
							if (py##_INDEX_==TEXTURE_SEAM_EXTENSION+1)\
								_c3(normal_texel##_INDEX_,getTexel3(texel_data,px##_INDEX_,py##_INDEX_-1,dimension));\
						}
					#define EXTEND_STYLE_TEXEL(_INDEX_)\
						if (extent##_INDEX_)\
						{\
							_c3(style_texel##_INDEX_,getTexel3(texel_data,px##_INDEX_-1,py##_INDEX_,dimension));\
							if (py##_INDEX_+TEXTURE_SEAM_EXTENSION+1==dimension-1)\
								_c3(style_texel##_INDEX_,getTexel3(texel_data,px##_INDEX_,py##_INDEX_+1,dimension));\
						}
				#else
					#define EXTEND_NORMAL(_INDEX_)
					#define EXTEND_STYLE_TEXEL(_INDEX_)
				#endif
				
				EXTEND_NORMAL(0)
				EXTEND_NORMAL(1)
				EXTEND_NORMAL(2)
				EXTEND_NORMAL(3)
				
				
				#define FLIP_TEXEL_COORDS(_INDEX_)\
						px##_INDEX_ = dimension-px##_INDEX_-1;\
						py##_INDEX_ = dimension-py##_INDEX_-1;
				
				FLIP_TEXEL_COORDS(0)
				FLIP_TEXEL_COORDS(1)
				FLIP_TEXEL_COORDS(2)
				FLIP_TEXEL_COORDS(3)
				/*if (x == y || x == y+1)
					return;*/
				/*if (px >= dimension || py >= dimension)
					FATAL__("o_O");*/
				
				#define SET_STYLE_TEXEL(_INDEX_)	unsigned char* style_texel##_INDEX_ = getTexel3(texel_data,px##_INDEX_,py##_INDEX_,dimension);
				
				SET_STYLE_TEXEL(0)
				SET_STYLE_TEXEL(1)
				SET_STYLE_TEXEL(2)
				SET_STYLE_TEXEL(3)
				
				#if !defined(COLOR_CHANNELS) || COLOR_CHANNELS==3
					TVertexAppearance	a0,a1,a2,a3;
					compileAppearanceBlock(vertex0, vertex1, vertex2, vertex3, *parameter.segment, *parameter.context, a0, a1, a2, a3);

					
					
					/*
					style_texel[0] = (unsigned char)(255.0f*sand);
					style_texel[1] = (unsigned char)(255.0f*fertility);
					style_texel[2] = (unsigned char)(255.0f*snow);*/
					#if COLOR_CHANNELS == 3
						#define FILL_STYLE_TEXEL(_INDEX_)\
							style_texel##_INDEX_[0] = (BYTE)(255*a##_INDEX_.planarity);\
							style_texel##_INDEX_[1] = (BYTE)(255*a##_INDEX_.general_fertility);\
							style_texel##_INDEX_[2] = (BYTE)(255*a##_INDEX_.water*0.66);
					#else
						#define FILL_STYLE_TEXEL(_INDEX_)\
							style_texel##_INDEX_[0] = (BYTE)(255*a##_INDEX_.sand);\
							style_texel##_INDEX_[1] = (BYTE)(255*a##_INDEX_.fertility);\
							style_texel##_INDEX_[2] = (BYTE)(255*a##_INDEX_.snow);
					#endif
				#elif COLOR_CHANNELS == 0
					#define FILL_STYLE_TEXEL(_INDEX_)\
						style_texel##_INDEX_[0] = (BYTE)(255*vertex##_INDEX_.channel.c0);\
						style_texel##_INDEX_[1] = (BYTE)(255*vertex##_INDEX_.channel.c1);\
						style_texel##_INDEX_[2] = (BYTE)(255*vertex##_INDEX_.channel.c2);
				#elif COLOR_CHANNELS == 1
					#define FILL_STYLE_TEXEL(_INDEX_)\
						style_texel##_INDEX_[0] = (BYTE)(255*vertex##_INDEX_.channel.c3);\
						style_texel##_INDEX_[1] = 0;\
						style_texel##_INDEX_[2] = (BYTE)(255*vertex##_INDEX_.height);
				#elif COLOR_CHANNELS == 2
					#define FILL_STYLE_TEXEL(_INDEX_)\
						style_texel##_INDEX_[0] = (BYTE)(255*vertex##_INDEX_.channel.oceanic);\
						style_texel##_INDEX_[1] = (BYTE)(255*(vertex##_INDEX_.height*0.5+0.5));\
						style_texel##_INDEX_[2] = (BYTE)(255*vertex##_INDEX_.channel.water);
				#elif COLOR_CHANNELS == 4
					#define FILL_STYLE_TEXEL(_INDEX_)\
						style_texel##_INDEX_[0] = (BYTE)(255*heightError(vertex##_INDEX_,*parameter.segment, *parameter.context));\
						style_texel##_INDEX_[1] = 0;\
						style_texel##_INDEX_[2] = 0;
				#endif
				FILL_STYLE_TEXEL(0)
				FILL_STYLE_TEXEL(1)
				FILL_STYLE_TEXEL(2)
				FILL_STYLE_TEXEL(3)
				
				EXTEND_STYLE_TEXEL(0)
				EXTEND_STYLE_TEXEL(1)
				EXTEND_STYLE_TEXEL(2)
				EXTEND_STYLE_TEXEL(3)
			}

			


			PARALLEL_OPERATION cast(unsigned iteration,const UnifiedOperationParameter&parameter)
			{
				const unsigned*triangle_indices = parameter.cast.triangle_indices;
				const TVertex*vertices = parameter.cast.vertex_field;
				const TVertex	&v0 = vertices[triangle_indices[iteration*3]],
								&v1 = vertices[triangle_indices[iteration*3+1]],
								&v2 = vertices[triangle_indices[iteration*3+2]];
				M::TVec3<>	d0,d1,n,dif,n0,n1,cross_point;

				M::Vec::sub((v1.position),(v0.position),d0);
				M::Vec::sub((v2.position),(v0.position),d1);
				M::Vec::cross(d0,d1,n);
				M::Vec::sub((v0.position),parameter.cast.b,dif);
				float	sub_alpha = M::Vec::dot(n,parameter.cast.d);
				if (sub_alpha==0.0f)
					return;
				float	alpha = M::Vec::dot(n,dif)/sub_alpha;
				M::Vec::cross(n,d0,n0);
				M::Vec::cross(n,d1,n1);
				M::Vec::mad(parameter.cast.b,parameter.cast.d,alpha,cross_point);
				M::Vec::sub(cross_point,(v0.position),dif);
				float	beta = M::Vec::dot(n0,dif)/M::Vec::dot(n0,d1),
						gamma = M::Vec::dot(n1,dif)/M::Vec::dot(n1,d0);
				if (beta >= 0 && gamma >= 0 && beta+gamma <= 1 && alpha >= 0)
				{
					TBaseRayIntersection&intersection = *parameter.cast.intersection;
					if (!intersection.isSet)
					{
						intersection.isSet = true;
						intersection.vertex[0] = &v0;
						intersection.vertex[1] = &v1;
						intersection.vertex[2] = &v2;
						intersection.fc[0] = gamma;
						intersection.fc[1] = beta;
						intersection.fc[2] = alpha;
					}
				}
			}


			PARALLEL_OPERATION groundCast(index_t iteration,const UnifiedOperationParameter&parameter)
			{
				const unsigned*triangle_indices = parameter.groundCast.triangle_indices;
				const TVertex*vertices = parameter.groundCast.vertex_field;
				const TVertex	&v0 = vertices[triangle_indices[iteration*3]],
								&v1 = vertices[triangle_indices[iteration*3+1]],
								&v2 = vertices[triangle_indices[iteration*3+2]];
				
				
				
				M::TVec3<>	d0,d1,n,dif,n0,n1,cross_point;
				if (M::Vec::dot((v0.normal),parameter.groundCast.d)>0.0f)
					return;

				M::Vec::sub((v1.position),(v0.position),d0);
				M::Vec::sub((v2.position),(v0.position),d1);
				M::Vec::cross(d0,d1,n);
				M::Vec::sub((v0.position),parameter.groundCast.b,dif);
				float	sub_alpha = M::Vec::dot(n,parameter.groundCast.d);
				if (sub_alpha==0.0f)
					return;
				float	alpha = M::Vec::dot(n,dif)/sub_alpha;
				M::Vec::cross(n,d0,n0);
				M::Vec::cross(n,d1,n1);
				M::Vec::mad(parameter.groundCast.b,parameter.groundCast.d,alpha,cross_point);
				M::Vec::sub(cross_point,(v0.position),dif);
				float	beta = M::Vec::dot(n0,dif)/M::Vec::dot(n0,d1),
						gamma = M::Vec::dot(n1,dif)/M::Vec::dot(n1,d0);
				
				if (beta >= 0 && gamma >= 0 && beta+gamma <= 1 /*&& alpha >= -0.1*/)
				{
					TBaseGroundInfo&ground = *parameter.groundCast.ground;
					parameter.output_mutex->lock();
					if (!ground.isSet)
					{
						ground.isSet = true;
						ground.height_over_ground = alpha;
						ground.ground_height = v0.height + (v1.height-v0.height)*gamma + (v2.height-v0.height)*beta;
						M::Vec::mad((v0.position),d0,gamma,ground.position);
						M::Vec::mad(ground.position,d1,beta);
						
						M::TVec3<>	nd0,
								nd1;
						M::Vec::sub((v1.normal),(v0.normal),nd0);
						M::Vec::sub((v2.normal),(v0.normal),nd1);
						M::Vec::mad((v0.normal),nd0,gamma,ground.normal);
						M::Vec::mad(ground.normal,nd1,beta);
					}
					parameter.output_mutex->release();
				}
			}
		
		}
		
		
		
		
		
		
		bool			setSSE(M::SSE::version_t version)
		{
			#ifndef __GNUC__
				if (version >= M::SSE::V4)
				{
					/*Operation::subdivideVertexBlock = Operation::subdivideVertexBlockSSE4;
					Operation::subdivideParentVertexBlock = Operation::subdivideParentVertexBlockSSE4;
					Operation::topLevelSubdivideVertexBlock = Operation::subdivideVertexBlock;*/
				}
				elif (version >= M::SSE::V3Supplemental)
				{
					/*Operation::subdivideVertexBlock = Operation::subdivideVertexBlockSSSE3;
					Operation::subdivideParentVertexBlock = Operation::subdivideParentVertexBlockSSSE3;
					Operation::topLevelSubdivideVertexBlock = Operation::subdivideVertexBlock;*/
				}
				else
			#endif
			if (version >= M::SSE::V3)
			{
				/*Operation::subdivideVertexBlock = Operation::subdivideVertexBlockSSE3;
				Operation::subdivideParentVertexBlock = Operation::subdivideParentVertexBlockSSE3;
				Operation::topLevelSubdivideVertexBlock = Operation::subdivideVertexBlock;*/
			}
			else
				return false;
			current_sse_version = version;
			return true;
		}
	
	static	AlignedArray<TSSECrater,16>	static_crater_field;
		
		void generateFinalLayer(TSurfaceSegment&segment, const TContext&context,const VertexMap&map)
		{
			BEGIN
			UnifiedOperationParameter parameter;
			parameter.segment = &segment;
			parameter.context = &context;
			parameter.map = &map;
			
			parameter.generateVertex.vertex_field = segment.vertex_field.pointer();
			parameter.generateVertex.parent_vertex_field = NULL;
			
			
			if (segment.crater_field.GetLength() > static_crater_field.GetLength())
			{
				static_crater_field.SetSize(segment.crater_field.GetLength());

			}
			for (unsigned i = 0; i < segment.crater_field.GetLength(); i++)
			{
				const VMOD TCrater&from = segment.crater_field[i];
				TSSECrater&to = static_crater_field[i];
				to.orientation.x = _mm_set1_ps(from.orientation.x);
				to.orientation.y = _mm_set1_ps(from.orientation.y);
				to.orientation.z = _mm_set1_ps(from.orientation.z);
				to.base.x = _mm_set1_ps(from.base.x);
				to.base.y = _mm_set1_ps(from.base.y);
				to.base.z = _mm_set1_ps(from.base.z);
				to.radius = _mm_set1_ps(from.radius);
				to.depth = _mm_set1_ps(from.depth);
			}
			
			parameter.generateVertex.sse_crater_field = static_crater_field.pointer();
			parameter.generateVertex.crater_field = segment.crater_field.pointer();
			parameter.generateVertex.crater_count = unsigned(segment.crater_field.GetLength());
			
			/*Kernel::parameter.generateVertex.sse_crater_field = static_crater_pointer;
			Kernel::parameter.generateVertex.crater_field = segment.crater_field.pointer();
			Kernel::parameter.generateVertex.crater_count = segment.crater_field.GetLength();*/
			
			
			
			M::Vec::clear(parameter.generateVertex.transition);

			unsigned num_iterations = numBlockIterations(map.child_vertex_count);

			
			//Kernel::Operation::execute(Kernel::Operation::topLevelSubdivideVertexBlock,num_iterations,128);	//128 seems fine here
			switch (current_sse_version)
			{
				case M::SSE::V3:
					Concurrency::parallel_for(unsigned(0),num_iterations,[&](unsigned i)
					{
						Kernel::Operation::subdivideVertexBlock<CSSE3Random>(i,parameter);
					});
				break;
				case M::SSE::V3Supplemental:
					Concurrency::parallel_for(unsigned(0),num_iterations,[&](unsigned i)
					{
						Kernel::Operation::subdivideVertexBlock<SupplementalSSE3Random>(i,parameter);
					});
				break;
				case M::SSE::V4:
					Concurrency::parallel_for(unsigned(0),num_iterations,[&](unsigned i)
					{
						Kernel::Operation::subdivideVertexBlock<CSSE4Random>(i,parameter);
					});
				break;
			}
			
			END
		}	
		
		
		void generateVertices(TSurfaceSegment&segment, const TContext&context,const VertexMap&map)
		{
			BEGIN
			segment.vertex_field.SetSize(map.vertex_count);

			UnifiedOperationParameter parameter;
			
			parameter.segment = &segment;
			parameter.context = &context;
			parameter.map = &map;
			
			parameter.generateVertex.vertex_field = segment.vertex_field.pointer();
			parameter.generateVertex.parent_vertex_field = segment.parent->vertex_field.pointer();
			
			
			

			if (segment.crater_field.GetLength() > static_crater_field.GetLength())
			{
				static_crater_field.SetSize(segment.crater_field.GetLength());

			}
			for (unsigned i = 0; i < segment.crater_field.GetLength(); i++)
			{
				const VMOD TCrater&from = segment.crater_field[i];
				TSSECrater&to = static_crater_field[i];
				to.orientation.x = _mm_set1_ps(from.orientation.x);
				to.orientation.y = _mm_set1_ps(from.orientation.y);
				to.orientation.z = _mm_set1_ps(from.orientation.z);
				to.base.x = _mm_set1_ps(from.base.x);
				to.base.y = _mm_set1_ps(from.base.y);
				to.base.z = _mm_set1_ps(from.base.z);
				to.radius = _mm_set1_ps(from.radius);
				to.depth = _mm_set1_ps(from.depth);
			}
			
			parameter.generateVertex.sse_crater_field = static_crater_field.pointer();
			parameter.generateVertex.crater_field = segment.crater_field.pointer();
			parameter.generateVertex.crater_count = unsigned(segment.crater_field.GetLength());
			
			
			M::TVec3<> delta;
			M::Vec::sub(segment.parent->sector,segment.sector,delta);
			
			M::Vec::mult(delta,context.sector_size,parameter.generateVertex.transition);

			//Kernel::Operation::execute(Kernel::Operation::copyParentVertex,map.parent_vertex_count,256);
			/*if (Kernel::background)
			{
				ParallelLoop<UnifiedOperationParameter,Operation::copyParentVertex,256,true>::execute(map.parent_vertex_count,parameter);
			}
			else*/
				Concurrency::parallel_for(unsigned(0),map.parent_vertex_count,[&](unsigned i)
				{
					Kernel::Operation::copyParentVertex(i,parameter);
				});

			
			unsigned num_iterations = numBlockIterations(map.child_vertex_count);
			//Kernel::Operation::execute(Kernel::Operation::subdivideVertexBlock,num_iterations,128);	//128 seems fine here
			switch (current_sse_version)
			{
				case M::SSE::V3:
					Concurrency::parallel_for(unsigned(0),num_iterations,[&](unsigned i)
					{
						Kernel::Operation::subdivideVertexBlock<CSSE3Random>(i,parameter);
					});
				break;
				case M::SSE::V3Supplemental:
					Concurrency::parallel_for(unsigned(0),num_iterations,[&](unsigned i)
					{
						Kernel::Operation::subdivideVertexBlock<SupplementalSSE3Random>(i,parameter);
					});
				break;
				case M::SSE::V4:
					Concurrency::parallel_for(unsigned(0),num_iterations,[&](unsigned i)
					{
						Kernel::Operation::subdivideVertexBlock<CSSE4Random>(i,parameter);
					});
				break;
			}

			END
		}	

		bool degenerate(TSurfaceSegment&segment, const TContext&context, const VertexMap&map)
		{
			if (!doDegenerate(segment,context))
				return false;
			if (segment.flags&TSurfaceSegment::ParentsProcessed)
				return false;
			segment.flags |= TSurfaceSegment::ParentsProcessed;
			segment.flags |= TSurfaceSegment::RequiresUpdate;


			UnifiedOperationParameter parameter;

			parameter.segment = &segment;
			parameter.context = &context;
			parameter.map = &map;
			
			parameter.generateVertex.vertex_field = segment.vertex_field.pointer();
			parameter.generateVertex.parent_vertex_field = NULL;
			
			
			

			if (segment.crater_field.GetLength() > static_crater_field.GetLength())
			{
				static_crater_field.SetSize(segment.crater_field.GetLength());

			}
			for (unsigned i = 0; i < segment.crater_field.GetLength(); i++)
			{
				const VMOD TCrater&from = segment.crater_field[i];
				TSSECrater&to = static_crater_field[i];
				to.orientation.x = _mm_set1_ps(from.orientation.x);
				to.orientation.y = _mm_set1_ps(from.orientation.y);
				to.orientation.z = _mm_set1_ps(from.orientation.z);
				to.base.x = _mm_set1_ps(from.base.x);
				to.base.y = _mm_set1_ps(from.base.y);
				to.base.z = _mm_set1_ps(from.base.z);
				to.radius = _mm_set1_ps(from.radius);
				to.depth = _mm_set1_ps(from.depth);
			}
			
			parameter.generateVertex.sse_crater_field = static_crater_field.pointer();
			parameter.generateVertex.crater_field = segment.crater_field.pointer();
			parameter.generateVertex.crater_count = unsigned(segment.crater_field.GetLength());
			
			

			//Kernel::Operation::execute(Kernel::Operation::degenerateVertexBlock,numBlockIterations(map.inner_parent_vertex_count),128);
			unsigned num_iterations = numBlockIterations(map.inner_parent_vertex_count);
			/*if (Kernel::background)
			{
				ParallelLoop<UnifiedOperationParameter,Operation::degenerateVertexBlock,128,true>::execute(num_iterations,parameter);
			}
			else*/
				Concurrency::parallel_for(unsigned(0),num_iterations,[&](unsigned i)
				{
					Kernel::Operation::degenerateVertexBlock(i,parameter);
				});


			for (BYTE k = 0; k < 3; k++)
				if (segment.flags&(TBaseSurfaceSegment::Edge0Open<<k))
				{
					//let's get back to this

					FATAL__("Feature not implemented");
				}

			//generateNormals(segment,context,map);
			return true;
		}
		
		

		void generateNormals(TSurfaceSegment&segment, const TContext&context,const VertexMap&map)
		{
			BEGIN
			UnifiedOperationParameter parameter;
			parameter.segment = &segment;
			parameter.context = &context;
			parameter.map = &map;
			
			parameter.generateNormal.vertex_field = segment.vertex_field.pointer();
				
			
			unsigned iterations = numBlockIterations(map.vertex_count);
			//Kernel::Operation::execute(Kernel::Operation::generateNormalBlock,iterations,128);				
			/*if (Kernel::background)
			{
				ParallelLoop<UnifiedOperationParameter,Operation::generateNormalBlock,128,true>::execute(iterations,parameter);
			}
			else*/
				Concurrency::parallel_for(unsigned(0),iterations,[&](unsigned i)
				{
					Kernel::Operation::generateNormalBlock(i,parameter);
				});

			
			END
		}	

		static bool fullyMerged(const TSurfaceSegment&segment)
		{
			return (   ((segment.flags & TSurfaceSegment::Edge0Merged) || (segment.flags & TSurfaceSegment::Edge0Open))
					&& ((segment.flags & TSurfaceSegment::Edge1Merged) || (segment.flags & TSurfaceSegment::Edge1Open))
					&& ((segment.flags & TSurfaceSegment::Edge2Merged) || (segment.flags & TSurfaceSegment::Edge2Open)));
		}


		unsigned	mergeEdges(TSurfaceSegment&segment, const TContext&context,const VertexMap&map)
		{
			ASSERT__((segment.flags&(TBaseSurfaceSegment::HasData)) != 0);
			
			UnifiedOperationParameter parameter;
			parameter.context = &context;
			parameter.map = &map;
		
			unsigned result = 0;
			
			for (BYTE k = 0; k < 3; k++)
			{
			
				//ASSERT1__(segment.neighbor_link[k].orientation<3,segment.neighbor_link[k].orientation);
				TSurfaceSegment*neighbor = segment.neighbor_link[k].segment;
				BYTE norientation = segment.neighbor_link[k].orientation;
				
				if (!neighbor || !(neighbor->flags&(TBaseSurfaceSegment::HasData)))
				{
					continue;
				}
				
				//ASSERT_EQUAL__(&segment, segment.neighbor_link[k].segment->neighbor_link[segment.neighbor_link[k].orientation].segment);
				//ASSERT_EQUAL__(k, segment.neighbor_link[k].segment->neighbor_link[segment.neighbor_link[k].orientation].orientation);
				
				if ((segment.flags&(TBaseSurfaceSegment::EdgeMergedOffset<<k)) && (neighbor->flags&(TBaseSurfaceSegment::EdgeMergedOffset<<norientation)))
					continue;	//both adjacent edges have been merged. no need to merge this edge
					
				
				if (!(segment.flags&(TBaseSurfaceSegment::EdgeMergedOffset<<k)) && !(neighbor->flags&(TBaseSurfaceSegment::EdgeMergedOffset<<norientation)))
				{
					bool merge_forward = segment.neighbor_link[k].primary;
						//segment.inner_int_seed > neighbor->inner_int_seed;	//crude... but effective. well with a 1 in 4bil fail probability. let's try the primary attribute instead
					M::TVec3<> delta;
					if (merge_forward)
					{
						parameter.mergeEdge.crater_field = segment.crater_field.pointer();
						parameter.mergeEdge.crater_count = unsigned(segment.crater_field.GetLength());

						parameter.mergeEdge.vertex_field = segment.vertex_field.pointer();
						parameter.mergeEdge.neighbor_vertex_field = neighbor->vertex_field.pointer();
						
						parameter.mergeEdge.edge = map.border_index[k];
						parameter.mergeEdge.neighbor_edge = map.border_index[norientation];
						parameter.segment = &segment;

						M::Vec::sub(neighbor->sector,segment.sector,delta);
						
					}
					else
					{
						parameter.mergeEdge.crater_field = neighbor->crater_field.pointer();
						parameter.mergeEdge.crater_count = unsigned(neighbor->crater_field.GetLength());

						parameter.mergeEdge.neighbor_vertex_field = segment.vertex_field.pointer();
						parameter.mergeEdge.vertex_field = neighbor->vertex_field.pointer();
						
						parameter.mergeEdge.neighbor_edge = map.border_index[k];
						parameter.mergeEdge.edge = map.border_index[norientation];

						parameter.segment = neighbor;

						M::Vec::sub(segment.sector,neighbor->sector,delta);
						
					}
					M::Vec::mult(delta,context.sector_size,parameter.mergeEdge.edge_transition);
					
			
					//Kernel::Operation::execute(Kernel::Operation::mergeEdge,map.vertex_range);
					/*#if FRACTAL_SMOOTH == 1
						Kernel::Operation::loop.execute(Kernel::Operation::mergeParentEdge,map.vertex_range,32);
					#endif*/

					//Kernel::Operation::execute(Kernel::Operation::mergeEdge,map.vertex_range,32);
					/*if (Kernel::background)
					{
						ParallelLoop<UnifiedOperationParameter,Operation::mergeEdge,32,true>::execute(map.vertex_range,parameter);
					}
					else*/
						Concurrency::parallel_for(unsigned(0),map.vertex_range,[&](unsigned i)
						{
							Kernel::Operation::mergeEdge(i,parameter);
						});

					if (doDegenerate(segment,context))
					{
						//Kernel::Operation::execute(Kernel::Operation::mergeEdge2,map.vertex_range,32);
						/*if (Kernel::background)
						{
							ParallelLoop<UnifiedOperationParameter,Operation::mergeEdge2,32,true>::execute(map.vertex_range,parameter);
						}
						else*/
							Concurrency::parallel_for(unsigned(0),map.vertex_range,[&](unsigned i)
							{
								Kernel::Operation::mergeEdge2(i,parameter);
							});
						segment.flags |= TBaseSurfaceSegment::RequiresUpdate;
						neighbor->flags |= TBaseSurfaceSegment::RequiresUpdate;
					}

					segment.flags |= TBaseSurfaceSegment::RequiresTextureUpdate;
					neighbor->flags |= TBaseSurfaceSegment::RequiresTextureUpdate;

				}
				elif (!(segment.flags&(TBaseSurfaceSegment::EdgeMergedOffset<<k)))	//this edge has not been merged (and is new compared to the neighboring edge)
				{
					parameter.copyEdge.destination_edge = map.border_index[k];
					parameter.copyEdge.source_edge = map.border_index[norientation];
					parameter.copyEdge.destination_vertex_field = segment.vertex_field.pointer();
					parameter.copyEdge.source_vertex_field = neighbor->vertex_field.pointer();
					parameter.segment = &segment;
					
					M::TVec3<> delta;
					M::Vec::sub(segment.sector,neighbor->sector,delta);
					
					M::Vec::mult(delta,context.sector_size,parameter.copyEdge.edge_transition);
					
					
					//DEBUG_POINT(k)
					//Kernel::Operation::execute(Kernel::Operation::copyEdge,map.vertex_range,32);
					/*if (Kernel::background)
					{
						ParallelLoop<UnifiedOperationParameter,Operation::copyEdge,32,true>::execute(map.vertex_range,parameter);
					}
					else*/
						Concurrency::parallel_for(unsigned(0),map.vertex_range,[&](unsigned i)
						{
							Kernel::Operation::copyEdge(i,parameter);
						});
					
					segment.flags |= TBaseSurfaceSegment::RequiresTextureUpdate;
				}
				else	// this edge has been merged but not the opposing one
				{
					//DEBUG_POINT(k)
					parameter.copyEdge.source_edge = map.border_index[k];
					parameter.copyEdge.destination_edge = map.border_index[norientation];
					parameter.copyEdge.source_vertex_field = segment.vertex_field.pointer();
					parameter.copyEdge.destination_vertex_field = neighbor->vertex_field.pointer();
					parameter.segment = &segment;
					
					M::TVec3<> delta;
					M::Vec::sub(neighbor->sector,segment.sector,delta);
					
					M::Vec::mult(delta,context.sector_size,parameter.copyEdge.edge_transition);
					
					
					
					//DEBUG_POINT(k)
					//Kernel::Operation::execute(Kernel::Operation::copyEdge,map.vertex_range,32);
					/*if (Kernel::background)
					{
						ParallelLoop<UnifiedOperationParameter,Operation::copyEdge,32,true>::execute(map.vertex_range,parameter);
					}
					else*/
						Concurrency::parallel_for(unsigned(0),map.vertex_range,[&](unsigned i)
						{
							Kernel::Operation::copyEdge(i,parameter);
						});					
					neighbor->flags |= TBaseSurfaceSegment::RequiresTextureUpdate;
				}
				

				//DEBUG_POINT(k)
				segment.flags |= (TBaseSurfaceSegment::EdgeMergedOffset<<k);
				neighbor->flags |= (TBaseSurfaceSegment::EdgeMergedOffset<<norientation);
				
				if (fullyMerged(*neighbor))
					result += degenerate(*neighbor,context,map);
				//DEBUG_POINT(k)
			}
			if (fullyMerged(segment))
				result += degenerate(segment,context,map);
			//DEBUG_POINT(k)
			
			END
			return result;
		}

		void	updateTexture(TSurfaceSegment&segment, const TContext&context, const VertexMap&map, bool generate_mipmap, bool compress_textures)
		{
			BEGIN
			const BYTE	num_channels = 3;
			unsigned	range = context.edge_length,
						image_width = range+1+TEXTURE_SEAM_EXTENSION,	//full range plus 1 for diagonal +1 separation row. should suffice but may have to increase
						num_texels = (image_width) * (image_width),
						num_values = num_texels*num_channels;
			//ASSERT1__(!(image_width%2),image_width);	//at the very least it should divisible by 2 //non-power-of-2 from now on
			size_t		data_size = sizeof(GLubyte)*num_values;
		
			static Array<BYTE,Primitive>	write_buffer;
			write_buffer.SetSize(num_values);
			
			UnifiedOperationParameter parameter;
			
			parameter.segment = &segment;
			parameter.context = &context;
			parameter.map = &map;
			
			parameter.generateTexel.info_field = map.vertex_descriptor;
			parameter.generateTexel.vertex_field = segment.vertex_field.pointer();
			parameter.generateTexel.dimension = image_width;
			parameter.generateTexel.texel_data = write_buffer.pointer();
			
			//Kernel::Operation::execute(Kernel::Operation::generateTexel,map.vertex_count);
			unsigned num_iterations = numBlockIterations(map.vertex_count);
			//Kernel::Operation::execute(Kernel::Operation::generateTexelBlock,num_iterations);
			/*cout << "Creating texture iterating "<< num_iterations<<" / "<<map.vertex_count<<endl;
			cout << "Num Values is "<<num_values<<endl;
			cout << "Data size is "<<data_size<<endl;*/
			//Kernel::Operation::execute(Kernel::Operation::generateTexelBlock,num_iterations,128);
			Concurrency::parallel_for(unsigned(0),num_iterations,[&](unsigned i)
			{
				Kernel::Operation::generateTexelBlock(i,parameter);
			});


				//Kernel::Operation::loop.execute(Kernel::Operation::generateTexel,map.vertex_count,128);
			
			/*static ThreadGranularityOptimizer	optimizer(128);
			Kernel::Operation::loop.execute(Kernel::Operation::generateTexelBlock,num_iterations,optimizer.begin());
			optimizer.end(num_iterations,CLOCATION);*/
			
			
			segment.texture.loadPixels(write_buffer.pointer(),image_width,image_width,num_channels,PixelType::Color,Engine::glExtensions.maxTextureMaxAnisotropy, true, generate_mipmap?Engine::TextureFilter::Trilinear:Engine::TextureFilter::Linear,compress_textures);

			#if 0
			{
				
				unsigned	mipmap_image_width = image_width>>1,
							mipmap_num_texels = (mipmap_image_width) * (mipmap_image_width),
							mipmap_num_values = mipmap_num_texels*num_channels;
				//ASSERT1__(!(image_width%2),image_width);	//at the very least it should divisible by 2 //non-power-of-2 from now on
				//size_t		data_size = sizeof(GLubyte)*num_values;
			
				static Array<BYTE>	mipmap_buffer;
				mipmap_buffer.resize(mipmap_num_values);

				Kernel::Operation::parameter.generateMIPMAP.in_texel_data = write_buffer;
				Kernel::Operation::parameter.generateMIPMAP.in_texture_size = image_width;
				Kernel::Operation::parameter.generateMIPMAP.out_texel_data = mipmap_buffer;
				Kernel::Operation::parameter.generateMIPMAP.out_texture_size = mipmap_image_width;
				Kernel::Operation::parameter.generateMIPMAP.out_num_texels = mipmap_num_texels;
				
				if (Kernel::current_sse_version >= M::SSE::V3Supplemental)
					Kernel::Operation::loop.execute(Kernel::Operation::generateMIPMAPBlock16,numBlockIterations(mipmap_num_texels,16),128);
				else
					FATAL__("Not implemented");
					//Kernel::Operation::loop.execute(Kernel::Operation::generateMIPMAP,mipmap_num_texels,128);

				ASSERT__(segment.texture.loadMIPMAPLevel(mipmap_buffer.pointer(),mipmap_image_width,mipmap_image_width,num_channels,1));
			}
			#endif

			END
		}
		
		
		void	updateVBO(TSurfaceSegment&segment, const TContext&context, const VertexMap&full_map, const VertexMap&vbo_map, const ArrayData<unsigned>&vbo_indices)
		{
			BEGIN
			static Array<float,Primitive>	write_buffer;

			Arrays::count_t	new_size = vbo_indices.Count();

			Arrays::count_t edge_depth = M::vmax(segment.vbo_edge[0].Count(),M::vmax(segment.vbo_edge[1].Count(),segment.vbo_edge[2].Count())),
							edge_vertices = ((1<<vbo_map.exponent)-(1<<(vbo_map.exponent-edge_depth)))*3;

			ASSERT2__(edge_depth <= vbo_map.exponent,edge_depth,vbo_map.exponent);
			new_size += edge_vertices;

			new_size *= TSurfaceSegment::floats_per_vbo_vertex;



			if (write_buffer.size() < new_size)
				write_buffer.SetSize(new_size);


			UnifiedOperationParameter parameter;

				
			parameter.segment = &segment;
			parameter.context = &context;
			parameter.map = &full_map;
			
			unsigned	seam_texels = 1+TEXTURE_SEAM_EXTENSION,
						texture_dimension = context.edge_length+1+TEXTURE_SEAM_EXTENSION;
			parameter.generateVBO.out_vertices = write_buffer.pointer();
			parameter.generateVBO.index_field = vbo_indices.pointer();
			parameter.generateVBO.in_vertices = segment.vertex_field.pointer();
			parameter.generateVBO.base = ((float)seam_texels)/(float)(texture_dimension);	//seam
			parameter.generateVBO.stretch = 1.0f-parameter.generateVBO.base;	//stretch minus seam
			parameter.generateVBO.stretch /= (float)full_map.vertex_maximum;	//normalize grid coordinates to the range [0=left edge, 1=right edge]
			
			//Kernel::Operation::execute(Kernel::Operation::generateVBO,vbo_indices.Count());
			//Kernel::Operation::execute(Kernel::Operation::generateVBO,unsigned(vbo_indices.Count()),192);
			Concurrency::parallel_for(unsigned(0),unsigned(vbo_indices.Count()),[&](unsigned i)
			{
				Kernel::Operation::generateVBO(i,parameter);
			});
			
				

			float*offset = write_buffer + vbo_indices.Count()*TSurfaceSegment::floats_per_vbo_vertex;

			for (Arrays::count_t i = 0; i < edge_depth; i++)
			{
				Arrays::count_t copy_length = (1<<(vbo_map.exponent-i-1))*TSurfaceSegment::floats_per_vbo_vertex,
								edge_length = copy_length+TSurfaceSegment::floats_per_vbo_vertex;
				for (BYTE k = 0; k < 3; k++)
				{
					if (segment.vbo_edge[k].Count() > i)
					{
						ASSERT_EQUAL__(edge_length,segment.vbo_edge[k][i].GetLength());
						memcpy(offset,segment.vbo_edge[k][i].pointer(),copy_length*sizeof(float));
						offset += copy_length;


						if (segment.vbo_edge[(k+2)%3].Count() > i)
						{
							const float	*predecessor_corner = segment.vbo_edge[(k+2)%3][i].pointer()+copy_length,
										*successor_corner = segment.vbo_edge[k][i].pointer();

							ASSERT3__(M::Vec::similar(M::Vec::ref3(predecessor_corner),M::Vec::ref3(successor_corner)),M::Vec::toString(M::Vec::ref3(predecessor_corner)),M::Vec::toString(M::Vec::ref3(successor_corner)),i);
						}
					}
					else
					{
						if (segment.vbo_edge[(k+2)%3].Count() > i)	//copy first vertex from last neighbor edge vertex
						{
							memcpy(offset,segment.vbo_edge[(k+2)%3][i].pointer()+copy_length,TSurfaceSegment::floats_per_vbo_vertex*sizeof(float));
						}
						offset += copy_length;
					}
				}
			}

			ASSERT__(offset <= write_buffer.pointer()+write_buffer.GetLength());
			ASSERT_EQUAL__(offset-write_buffer.pointer(),new_size);


			segment.vbo.load(write_buffer.pointer(),(offset - write_buffer.pointer()));	//4vtx + 3norm + 2tcoord

			END
		}
		
		/*
		void	updateCloudVBO(TSurfaceSegment&segment, const TContext&context, DeviceArray<unsigned>&device_cloud_triangles, unsigned range)
		{
			ASSERT1__(device_cloud_triangles.GetLength()>0 && !(device_cloud_triangles.GetLength()%3),device_cloud_triangles.GetLength());
			TDeviceSurface	this_surface;
			((TBaseSurface&)this_surface) = segment;
			this_surface.vertex = segment.vertex_field.pointer();
			
			segment.cloud_vbo.resize(device_cloud_triangles.GetLength()/3*9*sizeof(float));	//3 base + 3 ceiling + 3 norm
		
			unsigned blocks = ceilDiv(device_cloud_triangles.GetLength()/3,block_size);
			dim3 block(block_dimension, block_dimension, 1);
			dim3 grid(blocks, 1, 1);

			float*vertex_data;
			
			segment.cloud_vbo.registerAndMap(vertex_data);
			
			generateCloudVBO<<< grid, block >>>(vertex_data,this_surface, context, device_cloud_triangles.pointer(),device_cloud_triangles.GetLength(), range-1);
		
			segment.cloud_vbo.release();
		}
		*/
		
		void	retrieveCover(const TSurfaceSegment&segment, const TContext&context, const ArrayData<unsigned>&cover_indices, ArrayData<TCoverVertex>&out_cover)
		{
			BEGIN
			out_cover.SetSize(cover_indices.Count());
			
			UnifiedOperationParameter parameter;
			parameter.segment = &segment;
			parameter.context = &context;
			parameter.map = NULL;
			
			parameter.generateCover.out_vertices = out_cover.pointer();
			parameter.generateCover.in_vertices = segment.vertex_field.pointer();
			parameter.generateCover.index_field = cover_indices.pointer();
			
			//Kernel::Operation::execute(Kernel::Operation::generateCover,cover_indices.Count());
			//Kernel::Operation::execute(Kernel::Operation::generateCover,unsigned(cover_indices.Count()),192);
			Concurrency::parallel_for(unsigned(0),unsigned(cover_indices.Count()),[&](unsigned i)
			{
				Kernel::Operation::generateCover(i,parameter);
			});
			END
		}
		
		void	retrieveFullPrimaryCover(const TSurfaceSegment&segment, const TContext&context, const VertexMap&map, ArrayData<TCoverVertex>&out_cover)
		{
			BEGIN
			count_t num_vertices = map.inner_vertex_count;
			for (BYTE k = 0; k < 3; k++)
				if (segment.neighbor_link[k].primary)
					num_vertices += map.vertex_maximum;	//do not include end vertex, which belongs to the next edge
			
			out_cover.SetSize(num_vertices);
			
			UnifiedOperationParameter parameter;
			parameter.segment = &segment;
			parameter.context = &context;
			parameter.map = &map;
			
			parameter.generateCover.out_vertices = out_cover.pointer();
			parameter.generateCover.in_vertices = segment.vertex_field.pointer();
			parameter.generateCover.index_field = NULL;
			
			if (segment.neighbor_link[0].primary && segment.neighbor_link[1].primary && segment.neighbor_link[2].primary)	//all edges belong to this segment
			{
				//out_cover.SetSize(map.vertex_count);
				ASSERT_EQUAL__(out_cover.Count(),map.vertex_count);
				
				unsigned num_iterations = numBlockIterations(map.vertex_count);
				
				Concurrency::parallel_for(unsigned(0),num_iterations,[&](unsigned i)
				{
					Kernel::Operation::generateFullCoverBlock(i,parameter);
				});

			}
			else
			{
				unsigned num_iterations = numBlockIterations(map.inner_vertex_count);
					
				//Kernel::Operation::execute(Kernel::Operation::generateInnerCoverBlock,num_iterations,192);
				Concurrency::parallel_for(unsigned(0),num_iterations,[&](unsigned i)
				{
					Kernel::Operation::generateInnerCoverBlock(i,parameter);
				});


				parameter.generateCover.out_vertices += map.inner_vertex_count;
				
				num_iterations = numBlockIterations(map.vertex_maximum);
				
				for (BYTE k = 0; k < 3; k++)
					if (segment.neighbor_link[k].primary)
					{
						parameter.generateCover.index_field = map.border_index[k];
						
						//Kernel::Operation::execute(Kernel::Operation::generateEdgeCoverBlock,num_iterations,128);
						Concurrency::parallel_for(unsigned(0),num_iterations,[&](unsigned i)
						{
							Kernel::Operation::generateEdgeCoverBlock(i,parameter);
						});

						parameter.generateCover.out_vertices += map.vertex_maximum;
					}
				ASSERT_CONCLUSION(out_cover,parameter.generateCover.out_vertices);
			}
			//no chance to do it fully parallel. have to generate core, then outer edges in each a separate operation. may however distinguish the special case of all three edges being primary
			//Kernel::Operation::execute(Kernel::Operation::generateCover,cover_indices.Count());
			END
		}
		
		
		bool	rayCast(const TSurfaceSegment&segment,const M::TVec3<>&b, const M::TVec3<>&d, const ArrayData<unsigned>&triangles, TBaseRayIntersection&intersection)
		{
			BEGIN
			UnifiedOperationParameter parameter;
			parameter.segment = &segment;
			parameter.context = NULL;
			parameter.map = NULL;

			static System::Mutex mutex;
			parameter.output_mutex = &mutex;
			
			parameter.cast.triangle_indices = triangles.pointer();
			parameter.cast.vertex_field = segment.vertex_field.pointer();
			parameter.cast.intersection = &intersection;
			M::Vec::copy(b,parameter.cast.b);
			M::Vec::copy(d,parameter.cast.d);
			
			//Kernel::Operation::execute(Kernel::Operation::cast,unsigned(triangles.Count()/3));
			Concurrency::parallel_for(unsigned(0),unsigned(triangles.Count()/3),[&](unsigned i)
			{
				Kernel::Operation::cast(i,parameter);
			});


			/*if (intersection.isset)
			{
				lout << "intersection detected:"<<nl;
				lout << "  fc="<<_toString(intersection.fc)<<nl;
				lout << "  v[0]="<<_toString(intersection.vertex[0].position)<<nl;
				lout << "  v[1]="<<_toString(intersection.vertex[1].position)<<nl;
				lout << "  v[2]="<<_toString(intersection.vertex[2].position)<<nl;
			}*/
			END
			return intersection.isSet;
		}
		
		bool	groundQuery(const TSurfaceSegment&segment, const M::TVec3<>&b, const M::TVec3<>&down, const ArrayData<unsigned>&triangles, TBaseGroundInfo&ground)
		{
			BEGIN
			UnifiedOperationParameter parameter;
			static System::Mutex	mutex;

			parameter.output_mutex = &mutex;
			parameter.segment = &segment;
			parameter.context = NULL;
			parameter.map = NULL;
			
			parameter.groundCast.triangle_indices = triangles.pointer();
			parameter.groundCast.vertex_field = segment.vertex_field.pointer();
			parameter.groundCast.ground = &ground;
			M::Vec::copy(b,parameter.groundCast.b);
			M::Vec::copy(down,parameter.groundCast.d);
			
			
		
			//Kernel::Operation::execute(Kernel::Operation::groundCast,unsigned(triangles.Count()/3));
			Concurrency::parallel_for(index_t(0),(triangles.Count()/3),[&](index_t i)
			{
				Kernel::Operation::groundCast(i,parameter);
			});

			END
			return ground.isSet;
		}
		
	}
	
}
}
