/*
	This headers assembles a range of possible vertex processing functions.
	It may be included any number of times but must be done so using a uniquely defined OPERATION_NAME preprocessor variable each time.

	The file requires and preserves the following preprocessor variables:

	PARALLEL_OPERATION		modifiers and return type of the function to construct

	The file requires and automatically unsets the following preprocessor variables:

	LINEAR_INTERPOLATION	0 or 1, indicating the method to use for interpolating new vertices' base locations. Non-linear interpolation is deprecated. Leave at 1.
*/



template <typename Random>
PARALLEL_OPERATION subdivideVertexBlock(unsigned iteration,const UnifiedOperationParameter&parameter)
{
	ALIGNED16 float interchange[4];
	
	
	
	
	VMOD TVertex*field = parameter.generateVertex.vertex_field;
	const VertexMap&map = *parameter.map;
	unsigned range = 
			map.child_vertex_count;
	const TContext&context = *parameter.context;
	const TSurfaceSegment&segment = *parameter.segment;
	
	unsigned	index[4] = 
				{
					map.child_vertex_index[iteration*4],
					map.child_vertex_index[(iteration*4+1)%range],
					map.child_vertex_index[(iteration*4+2)%range],
					map.child_vertex_index[(iteration*4+3)%range]
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



	const TVertex	*p0[4],*p1[4],*p2[4],*p3[4];
	ALIGNED16 float	p3_exists[4];
	#undef FILL_PARENTS
	#define FILL_PARENTS(_INDEX_)	\
		p0[_INDEX_] = (const TVertex*)(field+info[_INDEX_]->parent0);\
		p1[_INDEX_] = (const TVertex*)(field+info[_INDEX_]->parent1);\
		p2[_INDEX_] = (const TVertex*)(field+info[_INDEX_]->parent2);\
		p3[_INDEX_] = info[_INDEX_]->borderDirection != -1?&empty_vertex:(const TVertex*)(field+info[_INDEX_]->parent3);\
		p3_exists[_INDEX_] = info[_INDEX_]->borderDirection == -1;

	FILL_PARENTS(0)
	FILL_PARENTS(1)
	FILL_PARENTS(2)
	FILL_PARENTS(3)


	ALIGNED16	M::TVec4<INT32>		seeds;
	
	
	#define FILL_SEED(_INDEX_)\
		{\
			if (info[_INDEX_]->borderDirection != -1)\
			{\
				unsigned elinear;\
				switch (info[_INDEX_]->borderDirection)\
				{\
					case 0:\
						elinear = info[_INDEX_]->y;\
					break;\
					case 1:\
						elinear = info[_INDEX_]->x;\
					break;\
					case 2:\
						elinear = context.edge_length-info[_INDEX_]->x-1;\
					break;\
				}\
				if (xCoord(p0[_INDEX_]->position) > xCoord(p1[_INDEX_]->position))\
					elinear = context.edge_length-elinear-1;\
				seeds.v[_INDEX_] = segment.outer_int_seed[info[_INDEX_]->borderDirection]+seedDelta(elinear);\
			}\
			else\
				seeds.v[_INDEX_] = segment.inner_int_seed+seedDelta(index[_INDEX_]);\
		}
			
	
	FILL_SEED(0)
	FILL_SEED(1)
	FILL_SEED(2)
	FILL_SEED(3)
	
	
	M::TVec3<SSE_VECTOR>	p0_position =
					{
						_mm_setr_ps(p0[0]->position.x,p0[1]->position.x,p0[2]->position.x,p0[3]->position.x),
						_mm_setr_ps(p0[0]->position.y,p0[1]->position.y,p0[2]->position.y,p0[3]->position.y),
						_mm_setr_ps(p0[0]->position.z,p0[1]->position.z,p0[2]->position.z,p0[3]->position.z)
					},
				p1_position =
					{
						_mm_setr_ps(p1[0]->position.x,p1[1]->position.x,p1[2]->position.x,p1[3]->position.x),
						_mm_setr_ps(p1[0]->position.y,p1[1]->position.y,p1[2]->position.y,p1[3]->position.y),
						_mm_setr_ps(p1[0]->position.z,p1[1]->position.z,p1[2]->position.z,p1[3]->position.z)
					},
				d =
					{
						p0_position.x-p1_position.x,
						p0_position.y-p1_position.y,
						p0_position.z-p1_position.z
					};
	SSE_VECTOR	distance = _mm_sqrt_ps( M::Vec::dot(d));
	

	
	
	float	
			distance_factor = 	getNoiseFactor(segment,context);

	/*if (depth < 0.5)
		distance_factor *= (1.5f-depth);*/

	/*if (depth < 0.75)
		distance_factor = 0;*/

	__m128 noise =	_mm_mul_ps(distance,_mm_set1_ps(distance_factor));
	

	
		//__m128 parent##_INDEX_##_##_CHANNEL_ = _mm_setr_ps(p##_INDEX_##0.channel._CHANNEL_, p##_INDEX_##1.channel._CHANNEL_,p##_INDEX_##2.channel._CHANNEL_,p##_INDEX_##3.channel._CHANNEL_);
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

	
	Random	random(seeds);
	
	
	const __m128	gain_up = _mm_set1_ps(WATER_GAIN_UP),
					gain_down = _mm_set1_ps(WATER_GAIN_DOWN);

	
	#undef SET_CHANNEL

	__m128	secondary_weight = _mm_set1_ps(SECONDARY_PARENT_WEIGHT),
			//weight_sum = _mm_add_ps(_mm_add_ps(p0_weight,p1_weight),_mm_mul_ps(secondary_weight,_mm_add_ps(p2_weight,p3_weight))),
			//w2 = _mm_mul_ps(secondary_weight,_mm_div_ps(p2_weight,weight_sum)),
			//w3 = _mm_mul_ps(secondary_weight,_mm_div_ps(p3_weight,weight_sum)),
			//w0 = _mm_div_ps(p0_weight,weight_sum),
			//w1 = _mm_div_ps(p1_weight,weight_sum),
			p3_weight = _mm_mul_ps(secondary_weight,_mm_load_ps(p3_exists)),
			weight_sum = _mm_add_ps(_mm_set1_ps(2.0f+SECONDARY_PARENT_WEIGHT),p3_weight),
			//w0 = _mm_rcp_ps(weight_sum),	//oookay. Obviously <-This line does not do the same as...
			w0 = _mm_div_ps(M::SSE::one,weight_sum), //...<-This line. It should, shouldn't it? The difference is subtle, but there
			w1 = w0,
			w2 = _mm_div_ps(secondary_weight,weight_sum),
			w3 = _mm_div_ps(p3_weight,weight_sum);
	#define SET_CHANNEL(_CHANNEL_)\
		__m128 interpolated_##_CHANNEL_ = _mm_add_ps(\
								_mm_add_ps( _mm_mul_ps(w0,p0_##_CHANNEL_), _mm_mul_ps(w1,p1_##_CHANNEL_)),\
								_mm_add_ps( _mm_mul_ps(w2,p2_##_CHANNEL_), _mm_mul_ps(w3,p3_##_CHANNEL_))\
							);
/*
	ASSERT__(isAligned(&secondary_weight));
	ASSERT__(isAligned(&weight_sum));
	ASSERT__(isAligned(&w0));
	ASSERT__(isAligned(&w1));
	ASSERT__(isAligned(&w2));
	ASSERT__(isAligned(&w3));
*/

	
	#undef SET_NOISE_CHANNEL0
	#define SET_NOISE_CHANNEL0(_CHANNEL_)\
		SET_CHANNEL(_CHANNEL_)\
		__m128 _CHANNEL_ = _mm_add_ps(\
						interpolated_##_CHANNEL_\
						,\
						_mm_mul_ps(\
							noise,\
							random.advance()\
						)\
					);\
		_CHANNEL_ = M::SSE::Clamp(\
						_CHANNEL_,\
						M::SSE::zero,M::SSE::one);	

	#undef SET_NOISE_CHANNEL
	#define SET_NOISE_CHANNEL(_CHANNEL_,_CUSTOM_FACTOR_)\
		SET_CHANNEL(_CHANNEL_)\
		__m128 _CHANNEL_ = _mm_add_ps(\
						interpolated_##_CHANNEL_\
						,\
						_mm_mul_ps(\
							_mm_mul_ps(\
								noise,\
								random.advance()\
							)\
							,\
							_mm_set1_ps(_CUSTOM_FACTOR_)\
						)\
					);\
		_CHANNEL_ = M::SSE::Clamp(\
						_CHANNEL_,\
						M::SSE::zero,M::SSE::one);
		//CHECK_SSE_IS_CONSTRAINED(_CHANNEL_,0,1);

	#undef SET_NOISE_CHANNEL2
	#define SET_NOISE_CHANNEL2(_CHANNEL_,_CUSTOM_FACTOR_)\
		SET_CHANNEL(_CHANNEL_)\
		__m128 _CHANNEL_ = _mm_add_ps(\
						interpolated_##_CHANNEL_\
						,\
						_mm_mul_ps(\
							_mm_mul_ps(\
								noise,\
								random.advance()\
							)\
							,\
							_CUSTOM_FACTOR_\
						)\
					);\
		_CHANNEL_ = M::SSE::Clamp(\
						_CHANNEL_,\
						M::SSE::zero,M::SSE::one);
		
						

	SET_NOISE_CHANNEL(c0,C0_CUSTOM_NOISE_FACTOR)
	#ifdef C1_CUSTOM_NOISE_FACTOR
		SET_NOISE_CHANNEL(c1,C1_CUSTOM_NOISE_FACTOR)
	#else
		SET_NOISE_CHANNEL0(c1);
	#endif
	SET_CHANNEL(c2)
	SET_NOISE_CHANNEL(c3,C3_CUSTOM_NOISE_FACTOR)
	
	ALIGNED16 float er[4];
	
	//ASSERT__(isAligned(er));
	/*_c4(seeds,quad_seeds);
	_mm_store_ps(er,random);*/
	
	
	SET_NOISE_CHANNEL2(oceanic,context.sse_oceanic_noise_level)

	
	
	__m128	
			riff_noise = _mm_mul_ps(noise,random.advance());
	#ifdef C2_CUSTOM_NOISE_FACTOR
		riff_noise = _mm_mul_ps(riff_noise,_mm_set1_ps(C2_CUSTOM_NOISE_FACTOR));
	#endif
	
	#if CANYON_LOOPS && REDUCE_CANYON_SLOPE_NOISE
		if (context.has_canyons)
		{
			static const __m128	canyon_slope_end = _mm_set1_ps(CANYON_SLOPE_END),
								canyon_slope_begin = _mm_set1_ps(CANYON_SLOPE_BEGIN);
			__m128	riff_distance = _mm_mul_ps(M::SSE::Abs(_mm_sub_ps(M::SSE::half,interpolated_c2)),M::SSE::five),
					reduce_noise = _mm_mul_ps(M::SSE::factorCompareGreater(riff_distance,canyon_slope_begin),M::SSE::factorCompareGreater(canyon_slope_end,riff_distance));
			riff_noise = _mm_add_ps(
							_mm_mul_ps(_mm_sub_ps(M::SSE::one,reduce_noise),riff_noise),
							_mm_mul_ps(
								reduce_noise,
								_mm_div_ps(
									riff_noise,
									_mm_add_ps(
										M::SSE::one,
										_mm_mul_ps(
											M::SSE::point25,
											c3
										)
									)
								)
							)
						);
		}
	#endif

	__m128 c2 = M::SSE::Clamp(
			_mm_add_ps(interpolated_c2,riff_noise),
			M::SSE::zero,M::SSE::one);
		
	/*
	result[0]->channel.water = 0;
	result[1]->channel.water = 0;
	result[2]->channel.water = 0;
	result[3]->channel.water = 0;*/
	
	//__m128 backup = c3;
	
	#define EXPORT_CHANNEL(_CHANNEL_)\
		_mm_store_ps((float*)interchange,_CHANNEL_);\
		result[0]->channel._CHANNEL_ = interchange[0];\
		result[1]->channel._CHANNEL_ = interchange[1];\
		result[2]->channel._CHANNEL_ = interchange[2];\
		result[3]->channel._CHANNEL_ = interchange[3];

	#define EXPORT_INTERPOLATED_CHANNEL(_CHANNEL_)\
		_mm_store_ps((float*)interchange,interpolated_##_CHANNEL_);\
		result[0]->channel._CHANNEL_ = interchange[0];\
		result[1]->channel._CHANNEL_ = interchange[1];\
		result[2]->channel._CHANNEL_ = interchange[2];\
		result[3]->channel._CHANNEL_ = interchange[3];
		
	
	EXPORT_CHANNEL(c0)
	EXPORT_CHANNEL(c1)
	EXPORT_CHANNEL(c2)
	EXPORT_CHANNEL(c3)
	EXPORT_CHANNEL(oceanic)


	#undef SET_PARENT_HEIGHTS
	#define SET_PARENT_HEIGHTS(_INDEX_)\
	{\
		h##_INDEX_ = _mm_setr_ps(p##_INDEX_[0]->FRACTAL_HEIGHT,p##_INDEX_[1]->FRACTAL_HEIGHT,p##_INDEX_[2]->FRACTAL_HEIGHT,p##_INDEX_[3]->FRACTAL_HEIGHT);\
	}
	
	SSE_VECTOR	h0,h1,h2,h3;

	SET_PARENT_HEIGHTS(0)
	SET_PARENT_HEIGHTS(1)
	SET_PARENT_HEIGHTS(2)
	
	((SSE_VECTOR&)h3) = _mm_setr_ps(
			info[0]->borderDirection == -1?p3[0]->FRACTAL_HEIGHT:(p0[0]->FRACTAL_HEIGHT+p1[0]->FRACTAL_HEIGHT+p2[0]->FRACTAL_HEIGHT)/3.0f,
			info[1]->borderDirection == -1?p3[1]->FRACTAL_HEIGHT:(p0[1]->FRACTAL_HEIGHT+p1[1]->FRACTAL_HEIGHT+p2[1]->FRACTAL_HEIGHT)/3.0f,
			info[2]->borderDirection == -1?p3[2]->FRACTAL_HEIGHT:(p0[2]->FRACTAL_HEIGHT+p1[2]->FRACTAL_HEIGHT+p2[2]->FRACTAL_HEIGHT)/3.0f,
			info[3]->borderDirection == -1?p3[3]->FRACTAL_HEIGHT:(p0[3]->FRACTAL_HEIGHT+p1[3]->FRACTAL_HEIGHT+p2[3]->FRACTAL_HEIGHT)/3.0f
		);

	


	
	
	#if LINEAR_INTERPOLATION == 1
		
		M::TVec3<SSE_VECTOR>	v = 
					{
						(p0_position.x+p1_position.x)* M::SSE::half,
						(p0_position.y+p1_position.y)* M::SSE::half,
						(p0_position.z+p1_position.z)* M::SSE::half
					};
		SSE_VECTOR current_height = (h0+h1)*M::SSE::half;
	#else
	

	//nurbs interpolation:. this method cannot be used on the top level because normals are not available there ... bummer



		#if 1

		/*
			This method now actually does work to some degree. Copy h3 from h2 if unavailable (see above)
			Segment edges do show but the result is sufficient to conclude it does not generate better results than linear interpolation. Pitty.
		*/
		SSE_VECTOR	v[3];
		{
			SplineQuad<SSE_VECTOR>	quad;
			
			float	pos3[4][3],
					norm3[4][3];

			#define LOCATE_PARENT3(_INDEX_)\
				if (info[_INDEX_]->borderDirection != -1)\
				{\
					float base[3],d[3],n0[3],n1[3];\
					_center(p0[_INDEX_]->position,p1[_INDEX_]->position,base);\
					_sub(p1[_INDEX_]->position,p0[_INDEX_]->position,d);\
					_center(p0[_INDEX_]->normal,p1[_INDEX_]->normal,n0);\
					_cross(n0,d,n1);\
					_reflect(base, n1, p2[_INDEX_]->position, pos3[_INDEX_]);\
					_reflectVector(n1, p2[_INDEX_]->normal, norm3[_INDEX_]);\
				}\
				else\
				{\
					_c3(p3[_INDEX_]->position,pos3[_INDEX_]);\
					_c3(p3[_INDEX_]->normal,norm3[_INDEX_]);\
				}
			LOCATE_PARENT3(0)
			LOCATE_PARENT3(1)
			LOCATE_PARENT3(2)
			LOCATE_PARENT3(3)
			SSE_VECTOR	
						
						n0[3] = 
							{
								_mm_setr_ps(p0[0]->normal[0],p0[1]->normal[0],p0[2]->normal[0],p0[3]->normal[0]),
								_mm_setr_ps(p0[0]->normal[1],p0[1]->normal[1],p0[2]->normal[1],p0[3]->normal[1]),
								_mm_setr_ps(p0[0]->normal[2],p0[1]->normal[2],p0[2]->normal[2],p0[3]->normal[2])
							},
						n1[3] = 
							{
								_mm_setr_ps(p1[0]->normal[0],p1[1]->normal[0],p1[2]->normal[0],p1[3]->normal[0]),
								_mm_setr_ps(p1[0]->normal[1],p1[1]->normal[1],p1[2]->normal[1],p1[3]->normal[1]),
								_mm_setr_ps(p1[0]->normal[2],p1[1]->normal[2],p1[2]->normal[2],p1[3]->normal[2])
							},
						p2_position[3] =
							{
								_mm_setr_ps(p2[0]->position[0],p2[1]->position[0],p2[2]->position[0],p2[3]->position[0]),
								_mm_setr_ps(p2[0]->position[1],p2[1]->position[1],p2[2]->position[1],p2[3]->position[1]),
								_mm_setr_ps(p2[0]->position[2],p2[1]->position[2],p2[2]->position[2],p2[3]->position[2])
							},
						n2[3] = 
							{
								_mm_setr_ps(p2[0]->normal[0],p2[1]->normal[0],p2[2]->normal[0],p2[3]->normal[0]),
								_mm_setr_ps(p2[0]->normal[1],p2[1]->normal[1],p2[2]->normal[1],p2[3]->normal[1]),
								_mm_setr_ps(p2[0]->normal[2],p2[1]->normal[2],p2[2]->normal[2],p2[3]->normal[2])
							},
						p3_position[3] =
							{
								_mm_setr_ps(pos3[0][0],pos3[1][0],pos3[2][0],pos3[3][0]),
								_mm_setr_ps(pos3[0][1],pos3[1][1],pos3[2][1],pos3[3][1]),
								_mm_setr_ps(pos3[0][2],pos3[1][2],pos3[2][2],pos3[3][2])
							},
						n3[3] =
							{
								_mm_setr_ps(norm3[0][0],norm3[1][0],norm3[2][0],norm3[3][0]),
								_mm_setr_ps(norm3[0][1],norm3[1][1],norm3[2][1],norm3[3][1]),
								_mm_setr_ps(norm3[0][2],norm3[1][2],norm3[2][2],norm3[3][2])
							};
			quad.build(p0_position,n0,p3_position,n3,p1_position,n1,p2_position,n2, M::SSE::point4);
			M::SSE::resolveSplineQuadCenter(quad, v);
		}
		SSE_VECTOR current_height = (h0+h1+h2+h3)* M::SSE::point25;
		#endif

		#if 0
			//this method does work but generates rather profoundly edged stripes in high noise areas. not recommended
		SSE_VECTOR	v[3];
		{
			SSE_VECTOR	delta[3];
			_sub(p1_position,p0_position,delta);
			
			SSE_VECTOR	n0[3] = {
									_mm_setr_ps(p0[0]->normal[0],p0[1]->normal[0],p0[2]->normal[0],p0[3]->normal[0]),
									_mm_setr_ps(p0[0]->normal[1],p0[1]->normal[1],p0[2]->normal[1],p0[3]->normal[1]),
									_mm_setr_ps(p0[0]->normal[2],p0[1]->normal[2],p0[2]->normal[2],p0[3]->normal[2])
								},
						n1[3] = {
									_mm_setr_ps(p1[0]->normal[0],p1[1]->normal[0],p1[2]->normal[0],p1[3]->normal[0]),
									_mm_setr_ps(p1[0]->normal[1],p1[1]->normal[1],p1[2]->normal[1],p1[3]->normal[1]),
									_mm_setr_ps(p1[0]->normal[2],p1[1]->normal[2],p1[2]->normal[2],p1[3]->normal[2])
								},
						d0[3],d1[3],control0[3],control1[3],t[3];
			//_sub(p1.final_position,p0.final_position,delta);
			//_div(delta,distance);
			_cross(delta,n0,t);
			_cross(n0,t,d0);
			
			_cross(delta,n1,t);
			_cross(n1,t,d1);
			
			M::SSE::normalize(d0);
			M::SSE::normalize(d1);

			/*_normalize(d0);
			_normalize(d1);
			if (_dot(d0,delta)<=0)
				FATAL__("d0 inverted");
			if (_dot(d1,delta)<=0)
				FATAL__("d1 inverted");*/
			/*control0[0] = p0x+d0[0]*distance*M::SSE::point3;
			control0[1] = p0y+d0[1]*distance*M::SSE::point3;
			control0[2] = p0z+d0[2]*distance*M::SSE::point3;
			
			control1[0] = p1x-d1[0]*distance*M::SSE::point3;
			control1[1] = p1y-d1[1]*distance*M::SSE::point3;
			control1[2] = p1z-d1[2]*distance*M::SSE::point3;*/
			
			ByReference::_mad(p0_position,d0,distance*M::SSE::point3,control0);
			ByReference::_mad(p1_position,d1,-distance*M::SSE::point3,control1);
			
			//_resolveNURBS(p0.final_position, control0, control1, p1.final_position, 0.5, v.uniform_position);			//!< Resolves a NURB-Spline point. \param p0 Curve start point \param p1 First curve control point \param p2 Second curve control point \param p3 Curve end point \param t Position scalar along the curve (0-1) \param out Out vector for the resolved NURBS position
			
			M::SSE::nurbsCenter(p0_position,control0,control1,p1_position,v);
		}
		#endif
	#endif

		
	polyHeight<CLAMP_HEIGHT>((const TChannelConfig&)result[0]->channel,(const TChannelConfig&)result[1]->channel,(const TChannelConfig&)result[2]->channel,(const TChannelConfig&)result[3]->channel,context.has_canyons,interchange);
		
	SSE_VECTOR h = _mm_load_ps((float*)interchange);
		
	//CHECK_SSE_IS_CONSTRAINED(h,-1,1);
		
	//ASSERT__(isAligned(&h));
		
		
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
				shore_relative = _mm_mul_ps(h,M::SSE::hundred);
					
		__m128 sign = _mm_sub_ps(_mm_mul_ps(M::SSE::factorCompareGreater(shore_relative,M::SSE::zero),M::SSE::two),M::SSE::one);
		shore_relative = _mm_mul_ps(Fractal::SSE::shoreFactor(M::SSE::Abs(shore_relative)), sign);
		//shore_relative = _mm_mul_ps(_mm_mul_ps(shore_relative,shore_relative),shore_relative);

		shore_relative = _mm_div_ps(shore_relative,M::SSE::hundred);
		h = _mm_add_ps(
				_mm_mul_ps(h,_mm_sub_ps(M::SSE::one,shore)),
				_mm_mul_ps(shore_relative,shore)
				);	
	}
	_mm_store_ps((float*)interchange,h);
		

	//if DEGENERATE is 0 then these variables are not initializes. if DEGENERATE is 1 then they contain the previous vertex height. They are however copied to 'current_height' further up.
	result[0]->FRACTAL_HEIGHT = interchange[0];
	result[1]->FRACTAL_HEIGHT = interchange[1];
	result[2]->FRACTAL_HEIGHT = interchange[2];
	result[3]->FRACTAL_HEIGHT = interchange[3];
		
	__m128	beneath_parent,
			water = _mm_setzero_ps();
			
	#undef ADD_PARENT_WATER
	#define ADD_PARENT_WATER(_PINDEX_)\
		beneath_parent = M::SSE::factorCompareGreater((const SSE_VECTOR&)h##_PINDEX_,h);\
		water = _mm_add_ps(\
					water,\
					_mm_mul_ps(\
						w##_PINDEX_,\
						_mm_mul_ps(\
							p##_PINDEX_##_water,\
							_mm_add_ps(\
								_mm_mul_ps(\
									beneath_parent,\
									gain_down\
								),\
								_mm_mul_ps(\
									_mm_sub_ps(M::SSE::one,beneath_parent),\
									gain_up\
								)\
							)\
						)\
					)\
				);



	ADD_PARENT_WATER(0)
	ADD_PARENT_WATER(1)
	ADD_PARENT_WATER(2)
	ADD_PARENT_WATER(3)
	#if DEGENERATE==1
		ADD_PARENT_WATER(4)
		ADD_PARENT_WATER(5)
	#endif
			

	if (context.has_ocean)
	{
		__m128	add_oceanic_water = M::SSE::factorCompareGreater(_mm_set1_ps(-0.005f),h);
		water = _mm_add_ps(water,_mm_mul_ps(add_oceanic_water,M::SSE::point1));
	}
			
	water = M::SSE::Clamp(
				_mm_add_ps(
					water,
					_mm_mul_ps(
						_mm_mul_ps(
							noise,
							random.advance()
						),
						_mm_set1_ps(WATER_NOISE)
					)
				)
				,0,context.fertility);
			
	_mm_store_ps((float*)interchange,water);
	result[0]->channel.water = interchange[0];
	result[1]->channel.water = interchange[1];
	result[2]->channel.water = interchange[2];
	result[3]->channel.water = interchange[3];


	Height::applyDifferentially(v, current_height, h,result[0]->position,result[1]->position,result[2]->position, result[3]->position, segment,context);


	
	
	
}

#undef LINEAR_INTERPOLATION
