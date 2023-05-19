#pragma once


#include <math/sse/sse_constants.h>

namespace DeltaWorks
{



namespace Fractal
{
	inline float shoreFactor(float x)
	{
		return -x * x * x + 2 * x * x;
	}

	namespace SSE
	{
		inline __m128	shoreFactor(const __m128 x)
		{
			__m128 sqr = _mm_mul_ps(x, x);
			return _mm_sub_ps(_mm_mul_ps(M::SSE::two, sqr), _mm_mul_ps(sqr, x));
		}

	}



	namespace Height
	{
		using namespace DeltaWorks::Math;	//M::SSE operators


		inline	void	applyDifferentially(const M::TVec3<SSE_VECTOR>&current_position, const SSE_VECTOR&current_height, const SSE_VECTOR&target_height,M::TVec3<>&p0, M::TVec3<>&p1, M::TVec3<>&p2, M::TVec3<>&p3, const TSurfaceSegment&segment, const TContext&context)
		{
			SSE_VECTOR	delta = (target_height - current_height)*context.sse_variance;
			
			M::TVec3<SSE_VECTOR> vd = {
								current_position.x + _mm_set1_ps((float)segment.sector.x*context.sector_size),
								current_position.y + _mm_set1_ps((float)segment.sector.y*context.sector_size),
								current_position.z + _mm_set1_ps((float)segment.sector.z*context.sector_size)
							};
			SSE_VECTOR	factor = _mm_rsqrt_ps( vd.x*vd.x + vd.y*vd.y + vd.z*vd.z)*delta;
			
			M::TVec3<SSE_VECTOR> final;
			M::Vec::mad(current_position,vd,factor,final);

			ALIGNED16 float interchange[4];


			_mm_store_ps((float*)interchange,final.x);
			p0.x = interchange[0];
			p1.x = interchange[1];
			p2.x = interchange[2];
			p3.x = interchange[3];

			_mm_store_ps((float*)interchange,final.y);
			p0.y = interchange[0];
			p1.y = interchange[1];
			p2.y = interchange[2];
			p3.y = interchange[3];

			_mm_store_ps((float*)interchange,final.z);
			p0.z = interchange[0];
			p1.z = interchange[1];
			p2.z = interchange[2];
			p3.z = interchange[3];
		}

		inline	void	applyDifferentially(M::TVec3<>&vector,float current_height, float target_height, const TSurfaceSegment&surface, const TContext&context)
		{
			M::TVec3<>	direction=
			{
				vector.x + (float)surface.sector.x*context.sector_size,
				vector.y + (float)surface.sector.y*context.sector_size,
				vector.z + (float)surface.sector.z*context.sector_size
			};

			M::Vec::normalize(direction);
			//dvNormalize(direction);

			M::Vec::mad(vector,direction,(target_height-current_height)*context.variance);
		}

		inline	void	apply(M::TVec3<>&vector, float scaled_relative_height, const TSurfaceSegment&surface, const TContext&context)
		{
			M::TVec3<> direction=
			{
				vector.x + (float)surface.sector.x*context.sector_size,
				vector.y + (float)surface.sector.y*context.sector_size,
				vector.z + (float)surface.sector.z*context.sector_size
			};

			M::Vec::normalize(direction);



			M::TVec3<> p;
			M::Vec::mult(direction,context.base_heightf,p);
			p.x -= context.sector_size*surface.sector.x;
			p.y -= context.sector_size*surface.sector.y;
			p.z -= context.sector_size*surface.sector.z;
			
			M::Vec::mad(p,direction,scaled_relative_height,vector);
		}


	}

	inline	void	resolveDirection(const M::TVec3<float>&vector, const TSurfaceSegment&surface, const TContext&context, M::TVec3<float>&out)
	{
		out.x = vector.x + (float)surface.sector.x*context.sector_size;
		out.y = vector.y + (float)surface.sector.y*context.sector_size;
		out.z = vector.z + (float)surface.sector.z*context.sector_size;
		M::Vec::normalize(out);
	}	

	inline	void	resolveDirection(const float vector[3], const TSurfaceSegment&surface, const TContext&context, M::TVec3<float>&out)
	{
		out.x = vector[0] + (float)surface.sector.x*context.sector_size;
		out.y = vector[1] + (float)surface.sector.y*context.sector_size;
		out.z = vector[2] + (float)surface.sector.z*context.sector_size;
		M::Vec::normalize(out);
	}	
	
	inline	void	resolveDirection(const float vector[3], const TSurfaceSegment&surface, const TContext&context, float out[3])
	{
		out[0] = vector[0] + (float)surface.sector.x*context.sector_size;
		out[1] = vector[1] + (float)surface.sector.y*context.sector_size;
		out[2] = vector[2] + (float)surface.sector.z*context.sector_size;
		M::Vec::normalize(M::Vec::ref3(out));
	}	
	
	
	inline void		resolveDirectionBlock(const M::TVec3<>&p0, const M::TVec3<>&p1, const M::TVec3<>&p2, const M::TVec3<>&p3, 
											const TSurfaceSegment&segment, const TContext&context, 
											M::TVec3<>&out0, M::TVec3<>&out1, M::TVec3<>&out2, M::TVec3<>&out3)
	{
		SSE_VECTOR	px = SSE_DEFINE4(p0.x,p1.x,p2.x,p3.x)+SSE_DEFINE1((float)segment.sector.x*context.sector_size),
					py = SSE_DEFINE4(p0.y,p1.y,p2.y,p3.y)+SSE_DEFINE1((float)segment.sector.y*context.sector_size),
					pz = SSE_DEFINE4(p0.z,p1.z,p2.z,p3.z)+SSE_DEFINE1((float)segment.sector.z*context.sector_size),
					factor = _mm_rsqrt_ps( px*px + py*py + pz*pz);
		
		ALIGNED16	float	exported[4];
		_mm_store_ps(exported,px*factor);
		out0.x = exported[0];
		out1.x = exported[1];
		out2.x = exported[2];
		out3.x = exported[3];
				
		_mm_store_ps(exported,py*factor);
		out0.y = exported[0];
		out1.y = exported[1];
		out2.y = exported[2];
		out3.y = exported[3];		
		
		_mm_store_ps(exported,pz*factor);
		out0.z = exported[0];
		out1.z = exported[1];
		out2.z = exported[2];
		out3.z = exported[3];
	}

	inline void		resolveDirectionBlock(const float p0[3], const float p1[3], const float p2[3], const float p3[3], 
											const TSurfaceSegment&segment, const TContext&context, 
											M::TVec3<>&out0, M::TVec3<>&out1, M::TVec3<>&out2, M::TVec3<>&out3)
	{
		SSE_VECTOR	px = SSE_DEFINE4(p0[0],p1[0],p2[0],p3[0])+SSE_DEFINE1((float)segment.sector.x*context.sector_size),
					py = SSE_DEFINE4(p0[1],p1[1],p2[1],p3[1])+SSE_DEFINE1((float)segment.sector.y*context.sector_size),
					pz = SSE_DEFINE4(p0[2],p1[2],p2[2],p3[2])+SSE_DEFINE1((float)segment.sector.z*context.sector_size),
					factor = _mm_rsqrt_ps( px*px + py*py + pz*pz);
		
		ALIGNED16	float	exported[4];
		_mm_store_ps(exported,px*factor);
		out0.x = exported[0];
		out1.x = exported[1];
		out2.x = exported[2];
		out3.x = exported[3];
				
		_mm_store_ps(exported,py*factor);
		out0.y = exported[0];
		out1.y = exported[1];
		out2.y = exported[2];
		out3.y = exported[3];		
		
		_mm_store_ps(exported,pz*factor);
		out0.z = exported[0];
		out1.z = exported[1];
		out2.z = exported[2];
		out3.z = exported[3];
	}


	inline	unsigned char*	getTexel3(unsigned char*texel_data, unsigned x, unsigned y, unsigned dimension)
	{
		return texel_data+(y*dimension+x)*3;
	}

	inline	unsigned char*	getTexel4(unsigned char*texel_data, unsigned x, unsigned y, unsigned dimension)
	{
		return texel_data+(y*dimension+x)*4;
	}

	inline	unsigned char*	getTexel1(unsigned char*texel_data, unsigned x, unsigned y, unsigned dimension)
	{
		return texel_data+(y*dimension+x);
	}


	inline	const unsigned char*	getTexel3(const unsigned char*texel_data, unsigned x, unsigned y, unsigned dimension)
	{
		return texel_data+(y*dimension+x)*3;
	}

	inline	const unsigned char*	getTexel4(const unsigned char*texel_data, unsigned x, unsigned y, unsigned dimension)
	{
		return texel_data+(y*dimension+x)*4;
	}

	inline	const unsigned char*	getTexel1(const unsigned char*texel_data, unsigned x, unsigned y, unsigned dimension)
	{
		return texel_data+(y*dimension+x);
	}

	inline	const unsigned char*	getTexel1Clamp(const unsigned char*texel_data, unsigned x, unsigned y, unsigned dimension)
	{
		return getTexel1(texel_data,M::Min(x,dimension-1),M::Min(y,dimension-1),dimension);
	}

	inline	const unsigned char*	getTexel3Clamp(const unsigned char*texel_data, unsigned x, unsigned y, unsigned dimension)
	{
		return getTexel3(texel_data,M::Min(x,dimension-1),M::Min(y,dimension-1),dimension);
	}

	inline	const unsigned char*	getTexel4Clamp(const unsigned char*texel_data, unsigned x, unsigned y, unsigned dimension)
	{
		return getTexel4(texel_data,M::Min(x,dimension-1),M::Min(y,dimension-1),dimension);
	}


	


	#define VERTEX_INDEX(_X_,_Y_)	((((_Y_)*((_Y_)+1))>>1)+(_X_))

	inline	unsigned	vertexIndex(unsigned x, unsigned y)
	{
		//return y*(y+1)/2+x;
		return VERTEX_INDEX(x,y);
	}
	
	
	inline float	getWeight(float h, float average, float variance, float depth)
	{
		return  1.0f+(h<average?0.9f*(average-h)/variance: (0.9f-h*0.7f)*(h-average)/variance)*(1.0f-depth);
	}


	inline float	getRandom(int&seed)
	{
		seed = seed * 1103515245 + 12345;
		//return (float)((unsigned)(seed/65536) % 32768)/(float)32767*2.0f-1.0f;
		return (float)((seed>>16) & 0x7FFF)/(float)32767*2.0f-1.0f;
		
	}


	inline float Clamp(float rs)
	{
	
		#define BOUNDARY 1.0f
		#define TOP_ROUND 0.005f
		
		if (rs > BOUNDARY-TOP_ROUND)
		{
			rs -= (BOUNDARY+TOP_ROUND);
			if (rs > TOP_ROUND*2)
				rs = BOUNDARY-rs+TOP_ROUND;//-0.25f*TOP_ROUND;
			else
				rs = BOUNDARY-TOP_ROUND*M::Sqr((rs)/(2*TOP_ROUND));
		}	
		
		return rs;
	}



	template <bool Clamp>
		inline	float	height(const TChannelConfig&channel, bool canyons)
		{
			FATAL__("Invalid invokation");
			return 0;
		}

	template <>
		inline	float	height<false>(const TChannelConfig&channel, bool canyons)
		{
			float	rs;
		
		
			// base height:
			rs = M::Min(M::Min(channel.c0,channel.c1),M::Min(channel.c2,channel.c3));
		
		
			rs += channel.c0*channel.c1*channel.c2*channel.c3*0.5;
		
			//rs = M::Sign(rs)*powf(fabs(rs),1.25f)*0.6f; //1.5
			rs = M::Sign(rs)*powf(fabs(rs),1.5f)*0.6f;
		
		
			//rs += dbCubicStep(channel.c0,0.4,0.6)*0.1f*channel.c1;	//mountains
			rs -= M::CubicStep(channel.c2*channel.c3,0.6,0.8)*0.15f*channel.c3;	//valleys
		
			rs -= M::CubicStep(channel.oceanic,0.4,0.5)*0.5f;	//oceanic basin
		
		
			//rs -= dbCubicStep(channel.oceanic,0.2,0.4)*0.045f;	//inner elevation / outer shore suppression
		
			#if CANYON_LOOPS
				//canyons:
				if (canyons)
				{
					float	riff_distance = fabs(0.5-channel.c2)*5.0f;
					rs -= 0.02*channel.c3*(1.0- M::CubicStep(riff_distance,CANYON_SLOPE_BEGIN,CANYON_SLOPE_END));
				}
			#endif
			rs += (1.0f- M::CubicStep(channel.oceanic,0,0.4))*0.15;	//continental upheav
		
			rs += 0.01;
		
		
			#ifdef FRACTAL_RIVERS
				//rs -= 0.1*dbCubicStep(channel.river_depth,0.25f,0.75f);
				rs -= 0.1*vmax(channel.river_depth*1.5f-0.25f,0.0f);
			#endif

			return rs;
		}
	template <>
		inline	float	height<true>(const TChannelConfig&channel, bool canyons)
		{
			return Clamp(height<false>(channel,canyons));
		}
	
	template <bool Clamp>
		inline	void	polyHeight(const TChannelConfig&ch0,const TChannelConfig&ch1,const TChannelConfig&ch2,const TChannelConfig&ch3, bool canyons, VMOD float result[4])
		{
			FATAL__("Invalid invokation");
		}

	template <>
		inline	void	polyHeight<false>(const TChannelConfig&ch0,const TChannelConfig&ch1,const TChannelConfig&ch2,const TChannelConfig&ch3, bool canyons, VMOD float result[4])
		{
			ALIGNED16 const __m128	half = _mm_set1_ps(0.5f),
									one = _mm_set1_ps(1.0f),
									c015 = _mm_set1_ps(0.15f),
									c5 = _mm_set1_ps(5.0f),
									c001 = _mm_set1_ps(0.01f),
									c06 = _mm_set1_ps(0.6f);
	
	
			ALIGNED16 VMOD float	out[4];
		
			ALIGNED16 __m128	c0 = _mm_set_ps(ch0.c0,ch1.c0,ch2.c0,ch3.c0),
								c1 = _mm_set_ps(ch0.c1,ch1.c1,ch2.c1,ch3.c1),
								c2 = _mm_set_ps(ch0.c2,ch1.c2,ch2.c2,ch3.c2),
								c3 = _mm_set_ps(ch0.c3,ch1.c3,ch2.c3,ch3.c3),
								oceanic = _mm_set_ps(ch0.oceanic,ch1.oceanic,ch2.oceanic,ch3.oceanic);
				
		
			ALIGNED16 __m128 rs = _mm_min_ps( _mm_min_ps(c0,c1), _mm_min_ps(c2,c3));
			rs = _mm_add_ps(rs, _mm_mul_ps(_mm_mul_ps( _mm_mul_ps(c0,c1), _mm_mul_ps(c2,c3)),half));
		
			//rs = _mm_mul_ps(_mm_mul_ps(rs,_mm_sqrt_ps(_mm_sqrt_ps(M::SSE::abs(rs)))),c06);//1.25
			rs = _mm_mul_ps(_mm_mul_ps(rs,_mm_sqrt_ps(M::SSE::Abs(rs))),c06);//1.5
			//rs = _mm_mul_ps(_mm_mul_ps(rs,M::SSE::abs(rs)),c06);	//2.0
		
		
			/*
			_mm_store_ps(out,rs);
			out[0] = M::Sign(out[0])*pow(fabs(out[0]),1.3f)*0.6f;
			out[1] = M::Sign(out[1])*pow(fabs(out[1]),1.3f)*0.6f;
			out[2] = M::Sign(out[2])*pow(fabs(out[2]),1.3f)*0.6f;
			out[3] = M::Sign(out[3])*pow(fabs(out[3]),1.3f)*0.6f;
			rs = _mm_load_ps(out);
			*/

			rs = _mm_sub_ps(rs,_mm_mul_ps(M::SSE::CubicStep(_mm_mul_ps(c2,c3),0.6,0.8),_mm_mul_ps(c015,c3)));
		
			rs = _mm_sub_ps(rs,_mm_mul_ps(M::SSE::CubicStep(oceanic,0.4,0.5),half));
		
		
			//rs -= dbCubicStep(channel.oceanic,0.2,0.4)*0.045f;	//inner elevation / outer shore suppression
		
			#if CANYON_LOOPS
				//canyons:
				if (canyons)
				{
					static const __m128	slope_begin = _mm_set1_ps(CANYON_SLOPE_BEGIN),
										slope_end =  _mm_set1_ps(CANYON_SLOPE_END);
					__m128	riff_distance = _mm_mul_ps(M::SSE::Abs(_mm_sub_ps(half,c2)),c5);
					rs = _mm_sub_ps(rs, _mm_mul_ps(_mm_mul_ps(_mm_set1_ps(0.02f),c3),_mm_sub_ps(one, M::SSE::CubicStep(riff_distance,slope_begin,slope_end))));
				}
			#endif
			rs = _mm_add_ps(rs, _mm_mul_ps(_mm_set1_ps(0.15f),_mm_sub_ps(one,M::SSE::CubicStep(oceanic,0,0.4))));
			rs = _mm_add_ps(rs, c001);
		
		
			//clamp
		

			#ifdef FRACTAL_RIVERS
				__m128 river = _mm_setr_ps(ch0.river_depth,ch1.river_depth,ch2.river_depth,ch3.river_depth);
				//rs -= 0.1*dbCubicStep(channel.river_depth,0.25f,0.75f);
				rs = _mm_sub_ps(rs, _mm_max_ps(_mm_sub_ps(_mm_mul_ps(river,_mm_set1_ps(1.5f)),_mm_set1_ps(0.25f)),_mm_set1_ps(0.0f)));
			#endif
		
		

			_mm_store_ps((float*)result,rs);
			swp(result[0],result[3]);
			swp(result[1],result[2]);
		}

	template <>
		inline	void	polyHeight<true>(const TChannelConfig&ch0,const TChannelConfig&ch1,const TChannelConfig&ch2,const TChannelConfig&ch3, bool canyons, VMOD float result[4])
		{
			polyHeight<false>(ch0,ch1,ch2,ch3,canyons,result);
			result[0] = Fractal::Clamp(result[0]);
			result[1] = Fractal::Clamp(result[1]);
			result[2] = Fractal::Clamp(result[2]);
			result[3] = Fractal::Clamp(result[3]);
		}



	inline float	getNoiseFactor(const TSurfaceSegment&surface, const TContext&context)
	{
		float rs = 37.5f/context.base_heightf;
		float depth = (float)surface.depth/context.recursive_depth;
		//float rs = (float)(1.0f/(context.base_heightf)/6*250)*0.9f;//*M::Clamp(M::Min(p0[0],p0[1]),0.1,0.2))
		//rs *= 0.2f+0.8f*depth;
		//rs *= 0.1f+0.9f*depth;
		#if 1
			rs *= (1.0f-M::Sqr(1.0f-depth));
		#else
			rs *= depth*1.7f;
		#endif
		rs *= context.noise_level;




		/*if (depth < 0.5)
			rs *= (1.5f-depth);*/
			
		return rs;


	}

	inline float	getNoise(float distance, const TSurfaceSegment&surface, const TContext&context)
	{
		return distance*getNoiseFactor(surface,context);
	}



	inline void		generate3inner(const TVertex&p0, const TVertex&p1, const TVertex&p2, VMOD TVertex&result, unsigned seed, const TSurfaceSegment&surface, const TContext&context, const TCrater*crater_field, count_t crater_count)
	{
		#if SSE_COORDINATES
			float distance = M::SSE::floatDistance(p0.position,p1.position);	
		#else
			float distance = M::Vec::distance(p0.position,p1.position);
		#endif
		
		
		float noise = getNoise(distance,surface,context);
		

		float weight= SECONDARY_PARENT_WEIGHT;//*2.0f;

		int random_seed = seed;


		float	total_weight = 2.0f+SECONDARY_PARENT_WEIGHT,
				w2 = weight/total_weight,
				w0 = 1.0f/total_weight,
				w1 = w0;
		
		
		
		result.channel.c0 = M::Clamp((w0*p0.channel.c0+w1*p1.channel.c0+w2*p2.channel.c0)+noise*getRandom(random_seed)
			#ifdef C0_CUSTOM_NOISE_FACTOR
				*C0_CUSTOM_NOISE_FACTOR
			#endif
			,0,1);
		result.channel.c1 = M::Clamp((w0*p0.channel.c1+w1*p1.channel.c1+w2*p2.channel.c1)+noise*getRandom(random_seed)
			#ifdef C1_CUSTOM_NOISE_FACTOR
				*C1_CUSTOM_NOISE_FACTOR
			#endif
			,0,1);
		result.channel.c2 = w0*p0.channel.c2+w1*p1.channel.c2+w2*p2.channel.c2;
		result.channel.c3 = M::Clamp((w0*p0.channel.c3+w1*p1.channel.c3+w2*p2.channel.c3)+noise*getRandom(random_seed)
			#ifdef C3_CUSTOM_NOISE_FACTOR
				*C3_CUSTOM_NOISE_FACTOR
			#endif
			,0,1);
		result.channel.oceanic = M::Clamp((w0*p0.channel.oceanic+w1*p1.channel.oceanic+w2*p2.channel.oceanic)+noise*getRandom(random_seed)*context.oceanic_noise_level,0,1);
		
		
		float	riff_distance = fabsf(0.5f-result.channel.c2)*5.0f,
				riff_noise = noise*getRandom(random_seed) 
			#ifdef C2_CUSTOM_NOISE_FACTOR
				*C2_CUSTOM_NOISE_FACTOR
			#endif
			;
		
		#ifdef C2_NOISE_LEVEL
			riff_noise *= C2_NOISE_LEVEL;
		#endif

		#if CANYON_LOOPS && REDUCE_CANYON_SLOPE_NOISE
			if (context.has_canyons && riff_distance>CANYON_SLOPE_BEGIN && riff_distance < CANYON_SLOPE_END)
				riff_noise/=(1.0f+0.25*result.channel.c3);
		#endif
		
		result.channel.c2 = M::Clamp(result.channel.c2+riff_noise,0,1);


		//result.channel.age = ageAt(surface.depth);
		

		

		
		
		result.channel.water = 0;//(w0*p0.channel.water+w1*p1.channel.water+w2*p2.channel.water);
		
		/*{
			// smooth channels: doesn't work
			
			float smooth_strength = getSmoothStrength(distance,p0,p1,surface,context);
			
			float	w2 = 0.333f,
					w0 = 0.333f,
					w1 = 0.333f;
			
			result.channel.c0 = (1.0f-smooth_strength)*result.channel.c0+(smooth_strength*(p0.channel.c0*w0+p1.channel.c0*w1+p2.channel.c0*w2));
			result.channel.c1 = (1.0f-smooth_strength)*result.channel.c1+(smooth_strength*(p0.channel.c1*w0+p1.channel.c1*w1+p2.channel.c1*w2));
			result.channel.c2 = (1.0f-smooth_strength)*result.channel.c2+(smooth_strength*(p0.channel.c2*w0+p1.channel.c2*w1+p2.channel.c2*w2));
			
			#if FRACTAL_KERNEL!=MINIMAL_KERNEL
				result.channel.c3 = (1.0f-smooth_strength)*result.channel.c3+(smooth_strength*(p0.channel.c3*w0+p1.channel.c3*w1+p2.channel.c3*w2));
			#endif
			#if FRACTAL_KERNEL==FULL_KERNEL
				result.channel.c4 = (1.0f-smooth_strength)*result.channel.c4+(smooth_strength*(p0.channel.c4*w0+p1.channel.c4*w1+p2.channel.c4*w2));
			#endif		
			
		}*/
		
		
		#ifdef FRACTAL_RIVERS
			result.channel.river_depth = M::Clamp(p0.channel.river_depth*0.25f+p1.channel.river_depth*0.25f + 0.5f*(p0.channel.river_depth*w0 + p1.channel.river_depth*w1 + p2.channel.river_depth*w2)+noise*getRandom(random_seed),0,1);
			
			/*if (surface.depth == context.recursive_depth-4 && getRandom(random_seed) < -0.9)
				result.channel.river_depth = 1.0f;*/
			
		#endif
		
		
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
				//result.height*result.height*result.height;
			result.height/=100.0f;
		}		

		#if 0
		{	//remove spikes
			
			float	min = M::Min(M::Min(p0.FRACTAL_HEIGHT,p1.FRACTAL_HEIGHT),p2.FRACTAL_HEIGHT),
					max = vmax(vmax(p0.FRACTAL_HEIGHT,p1.FRACTAL_HEIGHT),p2.FRACTAL_HEIGHT),
					avg = (min+max)/2.0f,
					rng = vmax((max-min)/2.0f,0.001f)/1.5f;
			
			float depth = (float)surface.depth/context.recursive_depth;
			result.channel.weight = getWeight(result.FRACTAL_HEIGHT,avg,rng,depth);
			
			
			
		
		
		
			/* constrain channels+height: doesn't work
			const TVertex*max=&p0,*min=&p0;
			if (max->height < p1.height)
				max = &p1;
			if (max->height < p2.height)
				max = &p2;
				
			if (min->height > p1.height)
				min = &p1;
			if (min->height > p2.height)
				min = &p2;
			
			if (result.height < min->height)
			{
				result.channel = min->channel;
				result.height = min->height;
			}
			elif (result.height > max->height)
			{
				result.channel = max->channel;
				result.height = max->height;
			}
			*/
		
		
			/* constrain height: doesn't work
			float	min = M::Min(M::Min(p0.height,p1.height),p2.height),
					max = vmax(vmax(p0.height,p1.height),p2.height),
					avg = (min+max)/2.0f,
					rng = (max-min)/2.0f;
			
			result.height = M::Clamp(result.height,avg-rng,avg+rng);
			*/
		}
		#endif
			//result.channel.water = 0;
			if (p0.FRACTAL_HEIGHT > result.FRACTAL_HEIGHT)
				result.channel.water += p0.channel.water*WATER_GAIN_DOWN*w0;
			else
				result.channel.water += p0.channel.water*WATER_GAIN_UP*w0;
			if (p1.FRACTAL_HEIGHT > result.FRACTAL_HEIGHT)
				result.channel.water += p1.channel.water*WATER_GAIN_DOWN*w1;
			else
				result.channel.water += p1.channel.water*WATER_GAIN_UP*w1;
			if (p2.FRACTAL_HEIGHT > result.FRACTAL_HEIGHT)
				result.channel.water += p2.channel.water*WATER_GAIN_DOWN*w2;
			else
				result.channel.water += p2.channel.water*WATER_GAIN_UP*w2;
			
			if (context.has_ocean && result.FRACTAL_HEIGHT < -0.005f)
				result.channel.water += 0.1f;
			result.channel.water = M::Clamp(result.channel.water+noise*getRandom(random_seed)*WATER_NOISE,0.0f,context.fertility);
			
	}
	
	inline void		generate3center(const TVertex&p0, const TVertex&p1, const TVertex&p2, VMOD TVertex&result, unsigned seed, const TSurfaceSegment&surface, const TContext&context, const TCrater*crater_field, count_t crater_count)
	{
		M::Vec::center(p0.position, p1.position, result.position);

		generate3inner(p0,p1,p2,result,seed,surface,context,crater_field,crater_count);
	}
	

	inline void		generate4inner(const TVertex&p0, const TVertex&p1, const TVertex&p2, const TVertex&p3, VMOD TVertex&result, int seed, const TSurfaceSegment&surface, const TContext&context, const TCrater*crater_field, count_t crater_count)
	{
		float distance = M::Vec::distance(p0.position,p1.position);

		
		float noise = getNoise(distance,surface,context);

		float weight= SECONDARY_PARENT_WEIGHT;

		int random_seed = seed;

		float	total_weight = 2.0f+2.0f*SECONDARY_PARENT_WEIGHT,
				w2 = weight/total_weight,
				w3 = weight/total_weight,
				w0 = 1.0f/total_weight,
				w1 = 1.0f/total_weight;
				
		result.channel.c0 = M::Clamp((w0*p0.channel.c0+w1*p1.channel.c0+w2*p2.channel.c0+w3*p3.channel.c0)+noise*getRandom(random_seed) 
			#ifdef C0_CUSTOM_NOISE_FACTOR
				*C0_CUSTOM_NOISE_FACTOR
			#endif
			,0,1);
		result.channel.c1 = M::Clamp((w0*p0.channel.c1+w1*p1.channel.c1+w2*p2.channel.c1+w3*p3.channel.c1)+noise*getRandom(random_seed)
			#ifdef C1_CUSTOM_NOISE_FACTOR
				*C1_CUSTOM_NOISE_FACTOR
			#endif
			,0,1);
		result.channel.c2 = w0*p0.channel.c2+w1*p1.channel.c2+w2*p2.channel.c2+w3*p3.channel.c2;
		result.channel.c3 = M::Clamp((w0*p0.channel.c3+w1*p1.channel.c3+w2*p2.channel.c3+w3*p3.channel.c3)+noise*getRandom(random_seed)
			#ifdef C3_CUSTOM_NOISE_FACTOR
				*C3_CUSTOM_NOISE_FACTOR
			#endif
			,0,1);
		result.channel.oceanic = M::Clamp((w0*p0.channel.oceanic+w1*p1.channel.oceanic+w2*p2.channel.oceanic+w3*p3.channel.oceanic)+noise*getRandom(random_seed)*context.oceanic_noise_level,0,1);
		
		float	riff_distance = fabsf(0.5f-result.channel.c2)*5.0f,
				riff_noise = noise*getRandom(random_seed)
				#ifdef C2_CUSTOM_NOISE_FACTOR
					*C2_CUSTOM_NOISE_FACTOR
				#endif
				;
		#if CANYON_LOOPS && REDUCE_CANYON_SLOPE_NOISE
			if (context.has_canyons && riff_distance>CANYON_SLOPE_BEGIN && riff_distance < CANYON_SLOPE_END)
				riff_noise/=(1.0f+0.25*result.channel.c3);
		#endif
		
		
		result.channel.c2 = M::Clamp(result.channel.c2+riff_noise,0,1);


		//result.channel.weight = weightAt(surface.depth);

		
		
		result.channel.water = 0;//(w0*p0.channel.water+w1*p1.channel.water+w2*p2.channel.water+w3*p3.channel.water);
		
		
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
		
		
		result.height = height<CLAMP_HEIGHT>((const TChannelConfig&)result.channel,context.has_canyons);
		float crater_depth = 0;
		for (index_t i = 0; i < crater_count; i++)
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
		

		
		//result.channel.water = 0;
		if (p0.FRACTAL_HEIGHT > result.FRACTAL_HEIGHT)
			result.channel.water += p0.channel.water*WATER_GAIN_DOWN*w0;
		else
			result.channel.water += p0.channel.water*WATER_GAIN_UP*w0;
		if (p1.FRACTAL_HEIGHT > result.FRACTAL_HEIGHT)
			result.channel.water += p1.channel.water*WATER_GAIN_DOWN*w1;
		else
			result.channel.water += p1.channel.water*WATER_GAIN_UP*w1;
		if (p2.FRACTAL_HEIGHT > result.FRACTAL_HEIGHT)
			result.channel.water += p2.channel.water*WATER_GAIN_DOWN*w2;
		else
			result.channel.water += p2.channel.water*WATER_GAIN_UP*w2;
		if (p3.FRACTAL_HEIGHT > result.FRACTAL_HEIGHT)
			result.channel.water += p3.channel.water*WATER_GAIN_DOWN*w3;
		else
			result.channel.water += p3.channel.water*WATER_GAIN_UP*w3;
		
		if (context.has_ocean && result.FRACTAL_HEIGHT < -0.005)
			result.channel.water += 0.1;
		result.channel.water = M::Clamp(result.channel.water+noise*getRandom(random_seed)*WATER_NOISE,0.0f,context.fertility);

	}
	
	inline void		generate4center(const TVertex&p0, const TVertex&p1, const TVertex&p2, const TVertex&p3, VMOD TVertex&result, unsigned seed, const TSurfaceSegment&surface, const TContext&context, const TCrater*crater_field, count_t crater_count)
	{
		M::Vec::center(p0.position, p1.position, result.position);
		generate4inner(p0,p1,p2,p3, result,seed,surface,context,crater_field,crater_count);
	}
	
	inline	void		compileVBOVertex(const TVertex&vertex, const TVertexDescriptor&info, const TVBOCompileParameter&parameter, float out[TSurfaceSegment::floats_per_vbo_vertex])
	{
		M::Vec::ref3(out) = vertex.position;
		out[3] = vertex.height;//*context.variance;
				
		resolveDirection(vertex.position,*parameter.segment,*parameter.context,M::Vec::ref3(out+4));
		out[7] = (float)info.x*parameter.coord_stretch;
		out[8] = parameter.coord_base+(float)info.y*parameter.coord_stretch;
	}




	inline void	compileAppearance(const TVertex&vertex, const TSurfaceSegment&surface, const TContext&context, TVertexAppearance&vout)
	{
		resolveDirection(vertex.position,surface,context,vout.up);
		
		vout.normal = vertex.normal;
		M::Vec::normalize(vout.normal);
		
		


		/*
			Notes:
			* sand and natural areas should be mutualy exclusive. quickly changing areas between sand and plant are not nice
			* both sand and natural areas feed on temperature. near the poles you'd find neither sand nor natural areas
			* make sure snow is not too close to intensive green areas. it looks odd from further away.
			* snow is water too. if you increase snow due to lack of temperature, increase water too
		*/
		
		vout.planarity = M::Vec::dot(vout.normal,vout.up);
		vout.temperature = (1.0f- M::Sqr(vout.up.y))*context.temperature;
		float	//surficial = dbLinearStep(vertex.height,-0.01f,0.0f),
				snow_fade,
				general_fertility,
				general_sand,
				abs_height = fabs(vertex.height);
		
		if (vertex.height > 0 || !context.has_ocean)
		{

			vout.snow_line = 1.5*vout.temperature*(1.0f-0.5f*vertex.channel.water);
			snow_fade = 0.05+ M::Sqr((float)(surface.depth+1)/context.recursive_depth)*0.1;
			vout.snow = (M::linearStep(vertex.height,vout.snow_line-0.5f,vout.snow_line)*vout.planarity+ M::linearStep(vertex.height,0.9f,1.0f))*context.fertility;
			vout.water = vertex.channel.water+vout.snow*0.5;
			general_fertility = M::linearStep(vout.water+0.3*(vout.temperature),0.3f,0.8f);
			if (context.has_ocean)
				general_fertility *= M::Min(abs_height*100.0f,1.0f);

			general_sand = M::Clamp(M::Sqr(1.0f-vertex.height),0,1)* M::Sqr(vout.temperature)*(1.0f-general_fertility);
			
			vout.sand = M::CubicStep(vout.planarity,0.25f,0.75f)*general_sand;
				//fertility = dbClamped(/*planarity*/dbSqr(dbClamped(snow_line-vertex.height,0.0f,1.0f)+0.1f)*general_fertility*3*dbCubicStep(planarity,0.15f,0.75f),0.0f,1.0f-sand-fabsf(vertex.height));
			vout.fertility = M::Clamp(M::Sqr(M::Clamp(vout.snow_line-vertex.height,0.0f,1.0f)+0.1f)* M::CubicStep(vout.planarity,0.4,0.8)*3*general_fertility,0.0f,M::Clamp(1.0f-(vout.sand+abs_height),0,1));
			vout.snow = M::linearStep(vout.snow,0.5-snow_fade,0.5+snow_fade);
			
			if (context.has_ocean && vertex.height < 0.05f)
				vout.snow *= M::Max(1.0f+(-0.05f+vertex.height)*80.0f*(0.25f+0.75f*(1.0f-M::Sqr(vout.planarity))),0.0f);
				
		}
		else
		{
			vout.snow = 0;
			vout.water = 0;
			vout.snow_line = 0;
			general_fertility = 0.0f;
			general_sand = M::Clamp(M::Sqr(1.0f-(vertex.height)),0,1)*M::Sqr(vout.temperature);
			vout.fertility = 0;
			vout.sand = M::CubicStep(vout.planarity,0.25f,0.75f)*general_sand;
		}
			
		if (context.has_ocean)
		{
			const float	upper_shore_boundary = vout.planarity > 0.5f?(1.0f-M::Sqr((vout.planarity-0.55f)/0.5f))*0.005f:0.0025f,
						lower_shore_boundary = -0.01f,
						sabulosity = vout.planarity > 0.7f?vout.temperature*M::Sqr((vout.planarity-0.7f)/0.3f):0.0f;
						
			
			if (vertex.height < upper_shore_boundary && vertex.height > lower_shore_boundary)
			{
				float shore_strength = vertex.height >= 0.0f?
											M::Min((1.0f-vertex.height/upper_shore_boundary)*1.1f,1.0f):
											M::Min((1.0f-vertex.height/lower_shore_boundary)*1.1f,1.0f);
				vout.sand = M::Clamp(vout.sand+shore_strength*(2.0f*sabulosity-1.0f),0.0f,1.0f);
				vout.fertility = M::Max(vout.fertility-shore_strength*(0.25f+0.75f*(1.0f-vout.planarity))*4.0f,0.0f);
			}
		}

		
	}

	inline void	compileAppearanceBlock(const TVertex&v0, const TVertex&v1, const TVertex&v2, const TVertex&v3,
									const TSurfaceSegment&surface, const TContext&context,
									TVertexAppearance&a0, TVertexAppearance&a1, TVertexAppearance&a2, TVertexAppearance&a3)
	{


			/*resolveDirection(v0.position,surface,context,a0.up);
			resolveDirection(v1.position,surface,context,a1.up);
			resolveDirection(v2.position,surface,context,a2.up);
			resolveDirection(v3.position,surface,context,a3.up);*/
			resolveDirectionBlock(v0.position, v1.position, v2.position, v3.position, surface, context, a0.up, a1.up, a2.up, a3.up);
		
		M::SSE::normalizeBlock(v0.normal,v1.normal,v2.normal,v3.normal,
							a0.normal,a1.normal,a2.normal,a3.normal);
		/*_c3(v0.normal,a0.normal);
		_normalize(a0.normal);
		_c3(v1.normal,a1.normal);
		_normalize(a1.normal);
		_c3(v2.normal,a2.normal);
		_normalize(a2.normal);
		_c3(v3.normal,a3.normal);
		_normalize(a3.normal);*/
		
		const float snow_fade_float = 0.05+M::Sqr((float)(surface.depth+1)/context.recursive_depth)*0.1;
		const SSE_VECTOR	height = SSE_DEFINE4(v0.height,v1.height,v2.height,v3.height),
							abs_height = M::SSE::Abs(height),
							inv_height = _mm_sub_ps(M::SSE::one,abs_height),
							sand_height = _mm_sub_ps(M::SSE::one,height),//ranging [0,2]. should be M::Clamp after usage
							pre_water = SSE_DEFINE4(v0.channel.water,v1.channel.water,v2.channel.water,v3.channel.water),
							snow_fade =	_mm_set1_ps(snow_fade_float);

						
						

		
		SSE_VECTOR	normal_x = SSE_DEFINE4(a0.normal.x,a1.normal.x,a2.normal.x,a3.normal.x),
					normal_y = SSE_DEFINE4(a0.normal.y,a1.normal.y,a2.normal.y,a3.normal.y),
					normal_z = SSE_DEFINE4(a0.normal.z,a1.normal.z,a2.normal.z,a3.normal.z),
					up_x = SSE_DEFINE4(a0.up.x,a1.up.x,a2.up.x,a3.up.x),
					up_y = SSE_DEFINE4(a0.up.y,a1.up.y,a2.up.y,a3.up.y),
					up_z = SSE_DEFINE4(a0.up.z,a1.up.z,a2.up.z,a3.up.z),
				
				planarity =			normal_x*up_x + normal_y*up_y + normal_z*up_z,
				temperature =		(M::SSE::one-up_y*up_y)*context.sse_temperature,
				surfacial =			context.has_ocean?SSE_VECTOR(M::SSE::factorCompareGreater(height,M::SSE::zero)):M::SSE::one,
				snow_line =			surfacial* M::SSE::onepoint5*temperature*(M::SSE::one-M::SSE::half*pre_water),
				snow =				(M::SSE::linearStep(height,snow_line-M::SSE::half,snow_line)*planarity+M::SSE::linearStep(height,M::SSE::point9,M::SSE::one))
									*surfacial*context.sse_fertility,
				water = 			surfacial*(pre_water+snow*M::SSE::half),
				general_fertility =	(SSE_VECTOR)M::SSE::linearStep(water+temperature*M::SSE::point3,M::SSE::point3,M::SSE::point8);
		if (context.has_ocean)
			general_fertility *= _mm_min_ps(abs_height*M::SSE::hundred,M::SSE::one)*surfacial;

		SSE_VECTOR	general_sand =		M::SSE::Clamp(M::Sqr(sand_height),M::SSE::zero,M::SSE::one)*M::Sqr(temperature)*(M::SSE::one-general_fertility),
					sand =				M::SSE::CubicStep(planarity,M::SSE::point25,M::SSE::point75)*general_sand,
					inner_fertility =	M::SSE::Clamp(
										snow_line-height,
										M::SSE::zero,M::SSE::one
										)
										+M::SSE::point1,
					fertility =			M::SSE::Clamp(
										M::Sqr(inner_fertility)*M::SSE::CubicStep(planarity,M::SSE::point4,M::SSE::point8)*general_fertility*M::SSE::three,
										M::SSE::zero,
										M::SSE::Clamp(M::SSE::one-(sand+abs_height),M::SSE::zero,M::SSE::one)
										);
							
		snow =	M::SSE::linearStep(snow,0.5f-snow_fade_float, 0.5+snow_fade_float);
		
		__m128	snow_keep =	M::SSE::one-SSE_DEFINE1(context.has_ocean)*M::SSE::factorCompareGreater(SSE_DEFINE1(0.05f),height);
		
		snow =	snow*_mm_max_ps(
						snow_keep,
						M::SSE::Clamp(M::SSE::one+(SSE_DEFINE1(-0.05f)+height)*SSE_DEFINE1(80.0f)*(M::SSE::point25+M::SSE::point75*(M::SSE::one-M::Sqr(planarity))),M::SSE::zero,M::SSE::one)
					);
					
			
		if (context.has_ocean)
		{
			const SSE_VECTOR	mostly_planar =					M::SSE::factorCompareGreater(planarity,M::SSE::half),
								planar = 						M::SSE::factorCompareGreater(planarity,M::SSE::point7),
								planar_upper_shore_boundary =	(M::SSE::one-M::Sqr((planarity-SSE_DEFINE1(0.55f))/M::SSE::half))*SSE_DEFINE1(0.005f),
								upper_shore_boundary =			mostly_planar*planar_upper_shore_boundary
																+
																(M::SSE::one-mostly_planar)*SSE_DEFINE1(0.0025f),
								lower_shore_boundary =			SSE_DEFINE1(-0.01f),
								sabulosity =					planar*temperature*M::Sqr((planarity-M::SSE::point7)/M::SSE::point3),
								is_shore =						(SSE_VECTOR)M::SSE::factorCompareGreater(upper_shore_boundary,height)*M::SSE::factorCompareGreater(height,lower_shore_boundary),
								not_shore =						M::SSE::one-is_shore,
								boundary =						surfacial*upper_shore_boundary + (M::SSE::one-surfacial)*lower_shore_boundary,
								shore_strength =				_mm_min_ps(
																(M::SSE::one-height/boundary)*M::SSE::onepoint1,
																M::SSE::one
															);
			sand =		is_shore*M::SSE::Clamp(sand+shore_strength*(M::SSE::two*sabulosity-M::SSE::one),M::SSE::zero,M::SSE::one)
						+
						not_shore*sand;
			fertility = is_shore*_mm_max_ps(fertility-shore_strength*(M::SSE::point25+M::SSE::point75*(M::SSE::one-planarity))*M::SSE::four,M::SSE::zero)
						+
						not_shore*fertility;
		}
		
		ALIGNED16	float exported[4];
		#undef EXPORT_APPEARANCE
		#define EXPORT_APPEARANCE(_MEMBER_)\
			_mm_store_ps(exported,_MEMBER_);\
			a0._MEMBER_ = exported[0];\
			a1._MEMBER_ = exported[1];\
			a2._MEMBER_ = exported[2];\
			a3._MEMBER_ = exported[3];
		
		EXPORT_APPEARANCE(snow_line)
		EXPORT_APPEARANCE(snow)
		EXPORT_APPEARANCE(water)
		EXPORT_APPEARANCE(sand)
		EXPORT_APPEARANCE(fertility)
		EXPORT_APPEARANCE(temperature)
		EXPORT_APPEARANCE(planarity)
	}

	
	/**
		f(x) = (x^5- (max(x*2-1,0))^5 +(max(x*2.8-2.026944824306054581095813539164,0))^5)/0.27609113417640907818006659315573
	
	*/
	
	inline void	resolveCraterDelta(const M::TVec3<>&p, const TCrater&crater, float&imprint, float&strength)
	{
		//return 0;
		M::TVec3<>	d1,radial;
		M::Vec::subtract(crater.base,p,d1);
		M::Vec::cross(crater.orientation,d1,radial);
		float x0 = M::Vec::length(radial)/crater.radius*2.0f-1.0f;
		if (x0 > 1)
		{
			imprint = 0;
			strength = 0;
			return;
		}
		if (x0 < 0)
		{
			imprint = -crater.depth;
			strength = 0.9f;
			return;
		}
		//return (x0-1)*crater.depth;
		
		float	x1 = M::Max(x0*2-1,0),
				x2 = M::Max(x0*2.8f-2.026944824306054581095813539164f,0.0f);
		imprint = ((x0*x0*x0*x0*x0 - x1*x1*x1*x1*x1 + x2*x2*x2*x2*x2)/0.27609113417640907818006659315573f-1.0f)*crater.depth;
		strength = (1.0f- M::CubicFactor(M::linearStep(x0,0.7f,1.0f)))*0.9f;
	}
	
	inline void	resolveCraterDeltaBlock(const SSE_VECTOR&x, const SSE_VECTOR&y, const SSE_VECTOR&z, const TSSECrater&crater, SSE_VECTOR&imprint, SSE_VECTOR&strength)
	{
		//return _mm_setzero_ps();
		ALIGNED16 static const SSE_VECTOR c0 = _mm_set1_ps(2.8f),
									c1 = _mm_set1_ps(2.026944824306054581095813539164f),
									c2 = _mm_set1_ps(0.27609113417640907818006659315573f);
		M::TVec3<SSE_VECTOR>	d1={crater.base.x-x, crater.base.y-y, crater.base.z-z},
							radial;
		//_subtract(crater.base,p,d1);
		M::Vec::cross(crater.orientation,d1,radial);
		SSE_VECTOR	x0 = _mm_max_ps(_mm_min_ps(_mm_sqrt_ps(radial.x*radial.x+radial.y*radial.y+radial.z*radial.z)/crater.radius*M::SSE::two-M::SSE::one,M::SSE::one),M::SSE::zero);
		
		//return (x0-M::SSE::one)*crater.depth;
		
		SSE_VECTOR	x1 = _mm_max_ps(x0*M::SSE::two - M::SSE::one , M::SSE::zero),
					x2 = _mm_max_ps(x0*c0-c1,M::SSE::zero);
		imprint = ((x0*x0*x0*x0*x0 - x1*x1*x1*x1*x1 + x2*x2*x2*x2*x2)/c2-M::SSE::one)*crater.depth;
		strength = (M::SSE::one-M::SSE::CubicFactor(M::SSE::linearStep(x0,M::SSE::point7,M::SSE::one)))*M::SSE::point9;

		
	}
	
	/*
		float	r2 = (M::Min(r*8.0f,1.0f));
	return (M::Min(r*r*r*r*r*r*2,(r-1.0f)*(r-1.0f)*4.0f+1.0f)+(1.0f+2.0f*r2*r2*r2-3.0f*r2*r2)*0.5f)-1;
*/
	
}


}
