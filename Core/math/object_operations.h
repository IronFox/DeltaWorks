#ifndef object_operationsH
#define object_operationsH
/*
This file was generated from template definition 'object.template.php' on 2017 July 3rd 12:23:40
Do not edit
*/


namespace Obj
{
	template <typename T0, typename T1>
		inline	T0	__fastcall	Operator0_sumD(const T0*__p0, const T1*__p1, count_t dimensions)throw()
		{
			T0 result = Math::sqr(__p0[0] - __p1[0]);
			for (register index_t iterator__ = 1; iterator__ < dimensions; iterator__++)
				result = result + Math::sqr(__p0[iterator__] - __p1[iterator__]);
			return result;
		}

	template <typename T0, typename T1>
		inline	T0	__fastcall	Operator1_sumD(const T0*__p0, const T1*__p1, count_t dimensions)throw()
		{
			T0 result = __p0[0]*__p1[0];
			for (register index_t iterator__ = 1; iterator__ < dimensions; iterator__++)
				result = result + __p0[iterator__]*__p1[iterator__];
			return result;
		}

	template <typename T0>
		inline	T0	__fastcall	Operator2_sumD(const T0*__p0, count_t dimensions)throw()
		{
			T0 result = __p0[0]*__p0[0];
			for (register index_t iterator__ = 1; iterator__ < dimensions; iterator__++)
				result = result + __p0[iterator__]*__p0[iterator__];
			return result;
		}


		/* ----- Now dynamic_dimensions instances ----- */
	//now implementing template definition 'bool SphereContainsPoint (2..4) (<const [*] center>, <radius>, <const [*] p>) direct='
	/**
		@brief <br>
		<br>
		SphereContainsPoint() requires array pointers to operate on. Make sure all passed array pointers provide at least as many elements as required (@a dimensions in general for this version)<br>
		Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
	
		@param[in] center 
		@param[in] radius 
		@param[in] p 
		@param dimensions [in] Number of dimensions to process
		@return  
	*/
	template <typename T0, typename T1, typename T2>
		inline	bool	__fastcall	SphereContainsPointD(const T0 *center, T1 radius, const T2 *p, count_t dimensions)throw()
		{
			return (Obj::Operator0_sumD(center, p, dimensions)/*Math::sqr(center:i - p:i)*/) <= M::sqr(radius);
		}

	//now implementing template definition 'bool detectSphereEdgeIntersection|DetectSphereEdgeIntersection (2..4) (<const [*] center>, <radius>, <const [*] e0>, <const [*] e1>) direct='
	/**
		@brief <br>
		<br>
		detectSphereEdgeIntersection() requires array pointers to operate on. Make sure all passed array pointers provide at least as many elements as required (@a dimensions in general for this version)<br>
		Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
	
		@param[in] center 
		@param[in] radius 
		@param[in] e0 
		@param[in] e1 
		@param dimensions [in] Number of dimensions to process
		@return  
	*/
	template <typename T0, typename T1, typename T2, typename T3>
		inline	bool	__fastcall	detectSphereEdgeIntersectionD(const T0 *center, T1 radius, const T2 *e0, const T3 *e1, count_t dimensions)throw()
		{
			if (((Obj::Operator0_sumD(center, e0, dimensions)/*Math::sqr(center:i - e0:i)*/) <= M::sqr(radius)) || ((Obj::Operator0_sumD(center, e1, dimensions)/*Math::sqr(center:i - e1:i)*/) <= M::sqr(radius)))
				return true;
			Array<T0> d(dimensions);
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='DynamicPointers', parameters={e1, e0, d}...
			{
				for (register index_t iterator__=0; iterator__ < dimensions; iterator__++)
				{
					d[iterator__] = e1[iterator__] - e0[iterator__];
				}
			};
			Array<T0> delta(dimensions);
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='DynamicPointers', parameters={e0, center, delta}...
			{
				for (register index_t iterator__=0; iterator__ < dimensions; iterator__++)
				{
					delta[iterator__] = e0[iterator__] - center[iterator__];
				}
			};
			T0	pa = (Obj::Operator1_sumD(d, d, dimensions)/*d:i*d:i*/),
					pb = 2*(Obj::Operator1_sumD(d, delta, dimensions)/*d:i*delta:i*/),
					pc = (Obj::Operator2_sumD(delta, dimensions)/*delta:i*delta:i*/)-M::sqr(radius),
					rs[2];
			BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
			if (!num_rs)
				return false;
			T0	alpha = smallestPositiveResult(rs,num_rs);
			return alpha >= (T0)0 && alpha <= (T0)1;
		}

	/**
		@brief <br>
		<br>
		DetectSphereEdgeIntersection() requires array pointers to operate on. Make sure all passed array pointers provide at least as many elements as required (@a dimensions in general for this version)<br>
		Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
	
		@param[in] center 
		@param[in] radius 
		@param[in] e0 
		@param[in] e1 
		@param dimensions [in] Number of dimensions to process
		@return  
	*/
	template <typename T0, typename T1, typename T2, typename T3>
		inline	bool	__fastcall	DetectSphereEdgeIntersectionD(const T0 *center, T1 radius, const T2 *e0, const T3 *e1, count_t dimensions)throw()
		{
			if (((Obj::Operator0_sumD(center, e0, dimensions)/*Math::sqr(center:i - e0:i)*/) <= M::sqr(radius)) || ((Obj::Operator0_sumD(center, e1, dimensions)/*Math::sqr(center:i - e1:i)*/) <= M::sqr(radius)))
				return true;
			Array<T0> d(dimensions);
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='DynamicPointers', parameters={e1, e0, d}...
			{
				for (register index_t iterator__=0; iterator__ < dimensions; iterator__++)
				{
					d[iterator__] = e1[iterator__] - e0[iterator__];
				}
			};
			Array<T0> delta(dimensions);
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='DynamicPointers', parameters={e0, center, delta}...
			{
				for (register index_t iterator__=0; iterator__ < dimensions; iterator__++)
				{
					delta[iterator__] = e0[iterator__] - center[iterator__];
				}
			};
			T0	pa = (Obj::Operator1_sumD(d, d, dimensions)/*d:i*d:i*/),
					pb = 2*(Obj::Operator1_sumD(d, delta, dimensions)/*d:i*delta:i*/),
					pc = (Obj::Operator2_sumD(delta, dimensions)/*delta:i*delta:i*/)-M::sqr(radius),
					rs[2];
			BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
			if (!num_rs)
				return false;
			T0	alpha = smallestPositiveResult(rs,num_rs);
			return alpha >= (T0)0 && alpha <= (T0)1;
		}

	//now implementing template definition 'bool detectSphereEdgeIntersection|DetectSphereEdgeIntersection (2..4) (<const [*] center>, <radius>, <const [*] e0>, <const [*] e1>, <[2] distances>) direct='
	/**
		@brief Detects whether or not there is an intersection between the given sphere and edge. Returns the intersection distance factors between e0 (=0) and e1 (=1)<br>
		<br>
		detectSphereEdgeIntersection() requires array pointers to operate on. Make sure all passed array pointers provide at least as many elements as required (2 in general for this version)<br>
		Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
	
		@param[in] center 
		@param[in] radius 
		@param[in] e0 
		@param[in] e1 
		@param[out] distances Intersection distances
		@param dimensions [in] Number of dimensions to process
		@return True, if there is an intersection, or all intersection points lie inside the sphere, false otherwise 
	*/
	template <typename T0, typename T1, typename T2, typename T3, typename T4>
		inline	bool	__fastcall	detectSphereEdgeIntersectionD(const T0 *center, T1 radius, const T2 *e0, const T3 *e1, T4 distances[2], count_t dimensions)throw()
		{
			Array<T4> d(dimensions);
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='DynamicPointers', parameters={e1, e0, d}...
			{
				for (register index_t iterator__=0; iterator__ < dimensions; iterator__++)
				{
					d[iterator__] = e1[iterator__] - e0[iterator__];
				}
			};
			Array<T4> delta(dimensions);
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='DynamicPointers', parameters={e0, center, delta}...
			{
				for (register index_t iterator__=0; iterator__ < dimensions; iterator__++)
				{
					delta[iterator__] = e0[iterator__] - center[iterator__];
				}
			};
			T4	pa = (Obj::Operator1_sumD(d, d, dimensions)/*d:i*d:i*/),
					pb = 2*(Obj::Operator1_sumD(d, delta, dimensions)/*d:i*delta:i*/),
					pc = (Obj::Operator2_sumD(delta, dimensions)/*delta:i*delta:i*/)-M::sqr(radius),
					rs[2];
			BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
			if (!num_rs)
				return false;
			if (num_rs == 1)
			{
				distances[0] = distances[1] = rs[0];
				return rs[0] >= (T4)0 && rs[0] <= (T4)1;
			}
			distances[0] = std::min(rs[0],rs[1]);
			distances[1] = std::max(rs[0],rs[1]);
			return (distances[0] <= (T4)1 && distances[1] >= (T4)0);
		}

	/**
		@brief Detects whether or not there is an intersection between the given sphere and edge. Returns the intersection distance factors between e0 (=0) and e1 (=1)<br>
		<br>
		DetectSphereEdgeIntersection() requires array pointers to operate on. Make sure all passed array pointers provide at least as many elements as required (2 in general for this version)<br>
		Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
	
		@param[in] center 
		@param[in] radius 
		@param[in] e0 
		@param[in] e1 
		@param[out] distances Intersection distances
		@param dimensions [in] Number of dimensions to process
		@return True, if there is an intersection, or all intersection points lie inside the sphere, false otherwise 
	*/
	template <typename T0, typename T1, typename T2, typename T3, typename T4>
		inline	bool	__fastcall	DetectSphereEdgeIntersectionD(const T0 *center, T1 radius, const T2 *e0, const T3 *e1, T4 distances[2], count_t dimensions)throw()
		{
			Array<T4> d(dimensions);
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='DynamicPointers', parameters={e1, e0, d}...
			{
				for (register index_t iterator__=0; iterator__ < dimensions; iterator__++)
				{
					d[iterator__] = e1[iterator__] - e0[iterator__];
				}
			};
			Array<T4> delta(dimensions);
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='DynamicPointers', parameters={e0, center, delta}...
			{
				for (register index_t iterator__=0; iterator__ < dimensions; iterator__++)
				{
					delta[iterator__] = e0[iterator__] - center[iterator__];
				}
			};
			T4	pa = (Obj::Operator1_sumD(d, d, dimensions)/*d:i*d:i*/),
					pb = 2*(Obj::Operator1_sumD(d, delta, dimensions)/*d:i*delta:i*/),
					pc = (Obj::Operator2_sumD(delta, dimensions)/*delta:i*delta:i*/)-M::sqr(radius),
					rs[2];
			BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
			if (!num_rs)
				return false;
			if (num_rs == 1)
			{
				distances[0] = distances[1] = rs[0];
				return rs[0] >= (T4)0 && rs[0] <= (T4)1;
			}
			distances[0] = std::min(rs[0],rs[1]);
			distances[1] = std::max(rs[0],rs[1]);
			return (distances[0] <= (T4)1 && distances[1] >= (T4)0);
		}


		/* ----- Now implementing fixed_dimensions instances ----- */
	//now implementing template definition 'bool detectOpticalIntersection (<const [3] p0>, <const [3] p1>, <const [3] p2>, <const [3] b>, <const [3] d>, <&distance>) direct='
	/**
		@brief Checks if there is a valid closer intersection of the specified triangle and ray.<br />
		detectOpticalIntersection() calculates the intersection distance (if any) of the specified triangle and ray.<br />
		If such an intersection occurs and it's closer than the specified<br />
		distance then the distance variable will be updated and the result be true.<br />
		A valid intersection occurs if the ray intersects the triangle plane within the triangle's boundaries and with a positive ray factor.<br />
		An intersection with a negative ray factor - thus behind the ray's base position - will be ignored.<br />
		The function detects front- and backface intersections.<br>
		<br>
		detectOpticalIntersection() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
		Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
	
		@param[in] p0 First triangle vertex
		@param[in] p1 Second triangle vertex
		@param[in] p2 Third triangle vertex
		@param[in] b Ray base position
		@param[in] d Normalized ray direction vector
		@param[out] distance distance In/result distance scalar. This value will be updated if a closer - positive - intersection was detected
		@return true if a positive intersection closer than the specified distance was detected, false otherwise. The specified distance value remains unchanged if the result is false 
	*/
	template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
		inline	bool	__fastcall	detectOpticalIntersection(const TVec3<T0>& p0, const TVec3<T1>& p1, const TVec3<T2>& p2, const TVec3<T3>& b, const TVec3<T4>& d, T5& distance)throw()
		{
			TVec3<T5> d0, d1, n;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={p1, p0, d0}...
			{
				d0.x = p1.x - p0.x;
				d0.y = p1.y - p0.y;
				d0.z = p1.z - p0.z;
			};
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={p2, p0, d1}...
			{
				d1.x = p2.x - p0.x;
				d1.y = p2.y - p0.y;
				d1.z = p2.z - p0.z;
			};
			
			//block inlining void cross (<const [3] v>, <const [3] w>, <[3] result>) direct= for dimensions=3, assembly_mode='Objects', parameters={d0, d1, n}...
			{
				n.x = d0.y*d1.z - d0.z*d1.y;
				n.y = d0.z*d1.x - d0.x*d1.z;
				n.z = d0.x*d1.y - d0.y*d1.x;
			};
			T5	sub_alpha = (n.x*d.x + n.y*d.y + n.z*d.z);
			if (!sub_alpha)
				return false;
			TVec3<T5> dif;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={p0, b, dif}...
			{
				dif.x = p0.x - b.x;
				dif.y = p0.y - b.y;
				dif.z = p0.z - b.z;
			};
			T5	alpha = (n.x*dif.x + n.y*dif.y + n.z*dif.z)/sub_alpha;
			if (alpha < 0 || alpha > distance)
				return false;
			TVec3<T5> n0, n1, cross_point;
			
			//block inlining void cross (<const [3] v>, <const [3] w>, <[3] result>) direct= for dimensions=3, assembly_mode='Objects', parameters={n, d0, n0}...
			{
				n0.x = n.y*d0.z - n.z*d0.y;
				n0.y = n.z*d0.x - n.x*d0.z;
				n0.z = n.x*d0.y - n.y*d0.x;
			};
			
			//block inlining void cross (<const [3] v>, <const [3] w>, <[3] result>) direct= for dimensions=3, assembly_mode='Objects', parameters={n, d1, n1}...
			{
				n1.x = n.y*d1.z - n.z*d1.y;
				n1.y = n.z*d1.x - n.x*d1.z;
				n1.z = n.x*d1.y - n.y*d1.x;
			};
			
			//block inlining void multAdd|mad (2..4) (<const [*] base>, <const [*] vector>, <scalar>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={b, d, alpha, cross_point}...
			{
				cross_point.x = b.x + d.x * alpha;
				cross_point.y = b.y + d.y * alpha;
				cross_point.z = b.z + d.z * alpha;
			};
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={cross_point, p0, dif}...
			{
				dif.x = cross_point.x - p0.x;
				dif.y = cross_point.y - p0.y;
				dif.z = cross_point.z - p0.z;
			};
			T5	beta = (n0.x*dif.x + n0.y*dif.y + n0.z*dif.z)/(n0.x*d1.x + n0.y*d1.y + n0.z*d1.z),
					gamma = (n1.x*dif.x + n1.y*dif.y + n1.z*dif.z)/(n1.x*d0.x + n1.y*d0.y + n1.z*d0.z);
			using Math::GetError;
			if (beta > -GetError<T5>() && gamma > -GetError<T5>() && beta + gamma < (T5)1+ GetError<T5>())
			{
				distance = alpha;
				return true;
			}
			return false;
		}

	//now implementing template definition 'T0 tetrahedronVolume|TetrahedronVolume (<const [3] p0>, <const [3] p1>, <const [3] p2>, <const [3] p3>) direct='
	/**
		@brief Calculates the signed volume of the specified tetrahedron<br>
		<br>
		tetrahedronVolume() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
	
		@param[in] p0 
		@param[in] p1 
		@param[in] p2 
		@param[in] p3 
		@return signed volume 
	*/
	template <typename T0, typename T1, typename T2, typename T3>
		inline	T0	__fastcall	tetrahedronVolume(const TVec3<T0>& p0, const TVec3<T1>& p1, const TVec3<T2>& p2, const TVec3<T3>& p3)throw()
		{
			TVec3<T0> n;
			T0 vn;
			
			//block inlining void triangleNormal (<const [3] p0>, <const [3] p1>, <const [3] p2>, <[3] result>) direct= for dimensions=3, assembly_mode='Objects', parameters={p0, p1, p2, n}...
			{
				TVec3<T0> v, w;
				
				//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] n>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={p1, p0, v}...
				{
					v.x = p1.x - p0.x;
					v.y = p1.y - p0.y;
					v.z = p1.z - p0.z;
				};
				
				//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] n>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={p2, p0, w}...
				{
					w.x = p2.x - p0.x;
					w.y = p2.y - p0.y;
					w.z = p2.z - p0.z;
				};
				
				//block inlining void cross (<const [3] v>, <const [3] w>, <[3] n>) direct= for dimensions=3, assembly_mode='Objects', parameters={v, w, n}...
				{
					n.x = v.y*w.z - v.z*w.y;
					n.y = v.z*w.x - v.x*w.z;
					n.z = v.x*w.y - v.y*w.x;
				};
			};
			vn = (n.x*n.x + n.y*n.y + n.z*n.z);
			using std::fabs;
			using Math::GetError;
			if (fabs(vn) <= GetError<T0>())
				return 0;
			return ((p0.x*n.x + p0.y*n.y + p0.z*n.z)-(p3.x*n.x + p3.y*n.y + p3.z*n.z)) / (vn*6);
		}

	/**
		@brief Calculates the signed volume of the specified tetrahedron<br>
		<br>
		TetrahedronVolume() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
	
		@param[in] p0 
		@param[in] p1 
		@param[in] p2 
		@param[in] p3 
		@return signed volume 
	*/
	template <typename T0, typename T1, typename T2, typename T3>
		inline	T0	__fastcall	TetrahedronVolume(const TVec3<T0>& p0, const TVec3<T1>& p1, const TVec3<T2>& p2, const TVec3<T3>& p3)throw()
		{
			TVec3<T0> n;
			T0 vn;
			
			//block inlining void triangleNormal (<const [3] p0>, <const [3] p1>, <const [3] p2>, <[3] result>) direct= for dimensions=3, assembly_mode='Objects', parameters={p0, p1, p2, n}...
			{
				TVec3<T0> v, w;
				
				//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] n>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={p1, p0, v}...
				{
					v.x = p1.x - p0.x;
					v.y = p1.y - p0.y;
					v.z = p1.z - p0.z;
				};
				
				//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] n>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={p2, p0, w}...
				{
					w.x = p2.x - p0.x;
					w.y = p2.y - p0.y;
					w.z = p2.z - p0.z;
				};
				
				//block inlining void cross (<const [3] v>, <const [3] w>, <[3] n>) direct= for dimensions=3, assembly_mode='Objects', parameters={v, w, n}...
				{
					n.x = v.y*w.z - v.z*w.y;
					n.y = v.z*w.x - v.x*w.z;
					n.z = v.x*w.y - v.y*w.x;
				};
			};
			vn = (n.x*n.x + n.y*n.y + n.z*n.z);
			using std::fabs;
			using Math::GetError;
			if (fabs(vn) <= GetError<T0>())
				return 0;
			return ((p0.x*n.x + p0.y*n.y + p0.z*n.z)-(p3.x*n.x + p3.y*n.y + p3.z*n.z)) / (vn*6);
		}

	//now implementing template definition 'bool detectOpticalQuadIntersection (<const [3] p0>, <const [3] p1>, <const [3] p2>, <const [3] b>, <const [3] d>, <&distance>) direct='
	/**
		@brief Checks if there is a valid closer intersection of the specified quad and ray.<br />
		The unspecified fourth point is implied to be opposite of p0, reflected along the edge between p1 and p2.<br />
		detectOpticalQuadIntersection() calculates the intersection distance (if any) of the specified quad and ray.<br />
		If such an intersection occurs and it's closer than the specified<br />
		distance then the distance variable will be updated and the result be true.<br />
		A valid intersection occurs if the ray intersects the quad plane within the quad's boundaries and with a positive ray factor.<br />
		An intersection with a negative ray factor - thus behind the ray's base position - will be ignored.<br />
		The function detects front- and backface intersections.<br>
		<br>
		detectOpticalQuadIntersection() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
		Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
	
		@param[in] p0 
		@param[in] p1 
		@param[in] p2 
		@param[in] b 
		@param[in] d 
		@param[out] distance 
		@return  
	*/
	template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
		inline	bool	__fastcall	detectOpticalQuadIntersection(const TVec3<T0>& p0, const TVec3<T1>& p1, const TVec3<T2>& p2, const TVec3<T3>& b, const TVec3<T4>& d, T5& distance)throw()
		{
			TVec3<T5> d0, d1, n;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={p1, p0, d0}...
			{
				d0.x = p1.x - p0.x;
				d0.y = p1.y - p0.y;
				d0.z = p1.z - p0.z;
			};
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={p2, p0, d1}...
			{
				d1.x = p2.x - p0.x;
				d1.y = p2.y - p0.y;
				d1.z = p2.z - p0.z;
			};
			
			//block inlining void cross (<const [3] v>, <const [3] w>, <[3] result>) direct= for dimensions=3, assembly_mode='Objects', parameters={d0, d1, n}...
			{
				n.x = d0.y*d1.z - d0.z*d1.y;
				n.y = d0.z*d1.x - d0.x*d1.z;
				n.z = d0.x*d1.y - d0.y*d1.x;
			};
			T5 sub_alpha = (n.x*d.x + n.y*d.y + n.z*d.z);
			if (!sub_alpha)
				return false;
			TVec3<T5> dif;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={p0, b, dif}...
			{
				dif.x = p0.x - b.x;
				dif.y = p0.y - b.y;
				dif.z = p0.z - b.z;
			};
			T5 alpha = (n.x*dif.x + n.y*dif.y + n.z*dif.z) / sub_alpha;
			if (alpha < 0 || alpha > distance)
				return false;
			TVec3<T5> n0, n1, cross_point;
			
			//block inlining void cross (<const [3] v>, <const [3] w>, <[3] result>) direct= for dimensions=3, assembly_mode='Objects', parameters={n, d0, n0}...
			{
				n0.x = n.y*d0.z - n.z*d0.y;
				n0.y = n.z*d0.x - n.x*d0.z;
				n0.z = n.x*d0.y - n.y*d0.x;
			};
			
			//block inlining void cross (<const [3] v>, <const [3] w>, <[3] result>) direct= for dimensions=3, assembly_mode='Objects', parameters={n, d1, n1}...
			{
				n1.x = n.y*d1.z - n.z*d1.y;
				n1.y = n.z*d1.x - n.x*d1.z;
				n1.z = n.x*d1.y - n.y*d1.x;
			};
			
			//block inlining void multAdd|mad (2..4) (<const [*] base>, <const [*] vector>, <scalar>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={b, d, alpha, cross_point}...
			{
				cross_point.x = b.x + d.x * alpha;
				cross_point.y = b.y + d.y * alpha;
				cross_point.z = b.z + d.z * alpha;
			};
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={cross_point, p0, dif}...
			{
				dif.x = cross_point.x - p0.x;
				dif.y = cross_point.y - p0.y;
				dif.z = cross_point.z - p0.z;
			};
			T5	beta = (n0.x*dif.x + n0.y*dif.y + n0.z*dif.z) / (n0.x*d1.x + n0.y*d1.y + n0.z*d1.z),
					gamma = (n1.x*dif.x + n1.y*dif.y + n1.z*dif.z) / (n1.x*d0.x + n1.y*d0.y + n1.z*d0.z);
			if (beta > -TypeInfo<T5>::error && gamma > -TypeInfo<T5>::error && beta < (T5)1+ TypeInfo<T5>::error && gamma < (T5)1+ TypeInfo<T5>::error)
			{
				distance = alpha;
				return true;
			}
			return false;
		}

	//now implementing template definition 'T0 zeroTetrahedronVolume (<const [3] p0>, <const [3] p1>, <const [3] p2>) direct='
	/**
		@brief Calculates the signed volume of the specified tetrahedron with the fourth vertex being the point of origin.<br>_oZeroTetrahedronVolume() calculates the signed volume using the three specified vertices and the point of origin as the fourth vertex.<br>		The sign of the resulting scalar depends on the orientation of the three specified vertices as seen from the point of origin: clockwise=positive, counter clockwise=negative.<br>		This function is quite fast and can be used to quickly determine the volume of a closed hull, simply by calculating the sum of all the triangular tetrahedron volumes.<br>
		<br>
		zeroTetrahedronVolume() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
	
		@param[in] p0 First tetrahedron vertex
		@param[in] p1 Second tetrahedron vertex
		@param[in] p2 Third tetrahedron vertex
		@return signed volume 
	*/
	template <typename T0, typename T1, typename T2>
		inline	T0	__fastcall	zeroTetrahedronVolume(const TVec3<T0>& p0, const TVec3<T1>& p1, const TVec3<T2>& p2)throw()
		{
			TVec3<T0> n;
			T0	vn;
			
			//block inlining void cross (<const [3] v>, <const [3] w>, <[3] result>) direct= for dimensions=3, assembly_mode='Objects', parameters={p0, p1, n}...
			{
				n.x = p0.y*p1.z - p0.z*p1.y;
				n.y = p0.z*p1.x - p0.x*p1.z;
				n.z = p0.x*p1.y - p0.y*p1.x;
			};
			vn = (n.x*n.x + n.y*n.y + n.z*n.z);
			using std::fabs;
			using Math::GetError;
			if (fabs(vn) <= GetError<T0>())
				return 0;
			return (p2.x*n.x + p2.y*n.y + p2.z*n.z) / (vn*6);
		}

	//now implementing template definition 'T0 zeroTriangleSize (<const [2] p0>, <const [2] p1>) direct='
	/**
		@brief <br>
		<br>
		zeroTriangleSize() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
	
		@param[in] p0 
		@param[in] p1 
		@return  
	*/
	template <typename T0, typename T1>
		inline	T0	__fastcall	zeroTriangleSize(const TVec2<T0>& p0, const TVec2<T1>& p1)throw()
		{
			return (p0.x*p1.y - p0.y*p1.x)/2;
		}

	//now implementing template definition 'T0 signedTriangleSize (<const [2] p0>, <const [2] p1>, <const [2] p2>) direct='
	/**
		@brief <br>
		<br>
		signedTriangleSize() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
	
		@param[in] p0 
		@param[in] p1 
		@param[in] p2 
		@return  
	*/
	template <typename T0, typename T1, typename T2>
		inline	T0	__fastcall	signedTriangleSize(const TVec2<T0>& p0, const TVec2<T1>& p1, const TVec2<T2>& p2)throw()
		{
			return	(p0.x*p1.y+p1.x*p2.y+p2.x*p0.y
					-p1.x*p0.y-p2.x*p1.y-p0.x*p2.y)/2;
		}

	//now implementing template definition 'bool detEdgeIntersection (<const [2] a0>, <const [2] a1>, <const [2] b0>, <const [2] b1>, <&alpha>, <&beta>) direct='
	/**
		@brief <br>
		<br>
		detEdgeIntersection() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
		Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
	
		@param[in] a0 
		@param[in] a1 
		@param[in] b0 
		@param[in] b1 
		@param[out] alpha 
		@param[out] beta 
		@return  
	*/
	template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
		inline	bool	__fastcall	detEdgeIntersection(const TVec2<T0>& a0, const TVec2<T1>& a1, const TVec2<T2>& b0, const TVec2<T3>& b1, T4& alpha, T5& beta)throw()
		{
			TVec2<T4> d0, d1, d2;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=2, assembly_mode='Objects', parameters={b0, a0, d0}...
			{
				d0.x = b0.x - a0.x;
				d0.y = b0.y - a0.y;
			};
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=2, assembly_mode='Objects', parameters={a1, a0, d1}...
			{
				d1.x = a1.x - a0.x;
				d1.y = a1.y - a0.y;
			};
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=2, assembly_mode='Objects', parameters={b1, b0, d2}...
			{
				d2.x = b1.x - b0.x;
				d2.y = b1.y - b0.y;
			};
			using Math::GetError;
			T4	sub = d2.x * d1.y - d2.y * d1.x,
					error = GetError<T4>();
			using std::fabs;
			if (fabs(sub)<error)
				return false;
			T4 	a = (d0.y*d2.x-d0.x*d2.y)/sub,
					b = (d0.y*d1.x-d0.x*d1.y)/sub;
			alpha = a;
			beta = b;
			return a >= -error && a <= (T4)1+error && b >= -error && b <= (T4)1+error;
		}

	//now implementing template definition 'bool detectOpticalSphereIntersection (<const [3] center>, <radius>, <const [3] b>, <const [3] d>) direct='
	/**
		@brief <br>
		<br>
		detectOpticalSphereIntersection() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
		Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
	
		@param[in] center 
		@param[in] radius 
		@param[in] b 
		@param[in] d 
		@return  
	*/
	template <typename T0, typename T1, typename T2, typename T3>
		inline	bool	__fastcall	detectOpticalSphereIntersection(const TVec3<T0>& center, T1 radius, const TVec3<T2>& b, const TVec3<T3>& d)throw()
		{
			TVec3<T0> delta;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={b, center, delta}...
			{
				delta.x = b.x - center.x;
				delta.y = b.y - center.y;
				delta.z = b.z - center.z;
			};
			T0	pa = 1,
					pb = 2*(d.x*delta.x + d.y*delta.y + d.z*delta.z),
					pc = (delta.x*delta.x + delta.y*delta.y + delta.z*delta.z)-M::sqr(radius),
					rs[2];
			BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
			return num_rs > 0;
		}

	//now implementing template definition 'bool detectOpticalSphereIntersection (<const [3] center>, <radius>, <const [3] b>, <const [3] d>, <&distance>) direct='
	/**
		@brief <br>
		<br>
		detectOpticalSphereIntersection() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
		Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
	
		@param[in] center 
		@param[in] radius 
		@param[in] b 
		@param[in] d 
		@param[out] distance 
		@return  
	*/
	template <typename T0, typename T1, typename T2, typename T3, typename T4>
		inline	bool	__fastcall	detectOpticalSphereIntersection(const TVec3<T0>& center, T1 radius, const TVec3<T2>& b, const TVec3<T3>& d, T4& distance)throw()
		{
			TVec3<T4> delta;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={b, center, delta}...
			{
				delta.x = b.x - center.x;
				delta.y = b.y - center.y;
				delta.z = b.z - center.z;
			};
			T4	pa = 1,
					pb = 2*(d.x*delta.x + d.y*delta.y + d.z*delta.z),
					pc = (delta.x*delta.x + delta.y*delta.y + delta.z*delta.z)-M::sqr(radius),
					rs[2];
			BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
			if (!num_rs)
				return false;
			T4	alpha = smallestPositiveResult(rs,num_rs);
			if (alpha >= distance)
				return false;
			distance = alpha;
			return true;
		}

	//now implementing template definition 'bool SphereContainsPoint (2..4) (<const [*] center>, <radius>, <const [*] p>) direct='
	/**
		@brief <br>
		<br>
		2 dimensional specialized version of SphereContainsPoint()<br>
		SphereContainsPoint() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
		Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
	
		@param[in] center 
		@param[in] radius 
		@param[in] p 
		@return  
	*/
	template <typename T0, typename T1, typename T2>
		inline	bool	__fastcall	SphereContainsPoint(const TVec2<T0>& center, T1 radius, const TVec2<T2>& p)throw()
		{
			return (Math::sqr(center.x - p.x) + Math::sqr(center.y - p.y)) <= M::sqr(radius);
		}

	/**
		@brief <br>
		<br>
		3 dimensional specialized version of SphereContainsPoint()<br>
		SphereContainsPoint() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
		Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
	
		@param[in] center 
		@param[in] radius 
		@param[in] p 
		@return  
	*/
	template <typename T0, typename T1, typename T2>
		inline	bool	__fastcall	SphereContainsPoint(const TVec3<T0>& center, T1 radius, const TVec3<T2>& p)throw()
		{
			return (Math::sqr(center.x - p.x) + Math::sqr(center.y - p.y) + Math::sqr(center.z - p.z)) <= M::sqr(radius);
		}

	/**
		@brief <br>
		<br>
		4 dimensional specialized version of SphereContainsPoint()<br>
		SphereContainsPoint() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
		Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
	
		@param[in] center 
		@param[in] radius 
		@param[in] p 
		@return  
	*/
	template <typename T0, typename T1, typename T2>
		inline	bool	__fastcall	SphereContainsPoint(const TVec4<T0>& center, T1 radius, const TVec4<T2>& p)throw()
		{
			return (Math::sqr(center.x - p.x) + Math::sqr(center.y - p.y) + Math::sqr(center.z - p.z) + Math::sqr(center.w - p.w)) <= M::sqr(radius);
		}

	//now implementing template definition 'bool detectSphereEdgeIntersection|DetectSphereEdgeIntersection (2..4) (<const [*] center>, <radius>, <const [*] e0>, <const [*] e1>) direct='
	/**
		@brief <br>
		<br>
		2 dimensional specialized version of detectSphereEdgeIntersection/DetectSphereEdgeIntersection()<br>
		detectSphereEdgeIntersection() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
		Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
	
		@param[in] center 
		@param[in] radius 
		@param[in] e0 
		@param[in] e1 
		@return  
	*/
	template <typename T0, typename T1, typename T2, typename T3>
		inline	bool	__fastcall	detectSphereEdgeIntersection(const TVec2<T0>& center, T1 radius, const TVec2<T2>& e0, const TVec2<T3>& e1)throw()
		{
			if (((Math::sqr(center.x - e0.x) + Math::sqr(center.y - e0.y)) <= M::sqr(radius)) || ((Math::sqr(center.x - e1.x) + Math::sqr(center.y - e1.y)) <= M::sqr(radius)))
				return true;
			TVec2<T0> d;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=2, assembly_mode='Objects', parameters={e1, e0, d}...
			{
				d.x = e1.x - e0.x;
				d.y = e1.y - e0.y;
			};
			TVec2<T0> delta;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=2, assembly_mode='Objects', parameters={e0, center, delta}...
			{
				delta.x = e0.x - center.x;
				delta.y = e0.y - center.y;
			};
			T0	pa = (d.x*d.x + d.y*d.y),
					pb = 2*(d.x*delta.x + d.y*delta.y),
					pc = (delta.x*delta.x + delta.y*delta.y)-M::sqr(radius),
					rs[2];
			BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
			if (!num_rs)
				return false;
			T0	alpha = smallestPositiveResult(rs,num_rs);
			return alpha >= (T0)0 && alpha <= (T0)1;
		}

	/**
		@brief <br>
		<br>
		2 dimensional specialized version of detectSphereEdgeIntersection/DetectSphereEdgeIntersection()<br>
		DetectSphereEdgeIntersection() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
		Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
	
		@param[in] center 
		@param[in] radius 
		@param[in] e0 
		@param[in] e1 
		@return  
	*/
	template <typename T0, typename T1, typename T2, typename T3>
		inline	bool	__fastcall	DetectSphereEdgeIntersection(const TVec2<T0>& center, T1 radius, const TVec2<T2>& e0, const TVec2<T3>& e1)throw()
		{
			if (((Math::sqr(center.x - e0.x) + Math::sqr(center.y - e0.y)) <= M::sqr(radius)) || ((Math::sqr(center.x - e1.x) + Math::sqr(center.y - e1.y)) <= M::sqr(radius)))
				return true;
			TVec2<T0> d;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=2, assembly_mode='Objects', parameters={e1, e0, d}...
			{
				d.x = e1.x - e0.x;
				d.y = e1.y - e0.y;
			};
			TVec2<T0> delta;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=2, assembly_mode='Objects', parameters={e0, center, delta}...
			{
				delta.x = e0.x - center.x;
				delta.y = e0.y - center.y;
			};
			T0	pa = (d.x*d.x + d.y*d.y),
					pb = 2*(d.x*delta.x + d.y*delta.y),
					pc = (delta.x*delta.x + delta.y*delta.y)-M::sqr(radius),
					rs[2];
			BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
			if (!num_rs)
				return false;
			T0	alpha = smallestPositiveResult(rs,num_rs);
			return alpha >= (T0)0 && alpha <= (T0)1;
		}

	/**
		@brief <br>
		<br>
		3 dimensional specialized version of detectSphereEdgeIntersection/DetectSphereEdgeIntersection()<br>
		detectSphereEdgeIntersection() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
		Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
	
		@param[in] center 
		@param[in] radius 
		@param[in] e0 
		@param[in] e1 
		@return  
	*/
	template <typename T0, typename T1, typename T2, typename T3>
		inline	bool	__fastcall	detectSphereEdgeIntersection(const TVec3<T0>& center, T1 radius, const TVec3<T2>& e0, const TVec3<T3>& e1)throw()
		{
			if (((Math::sqr(center.x - e0.x) + Math::sqr(center.y - e0.y) + Math::sqr(center.z - e0.z)) <= M::sqr(radius)) || ((Math::sqr(center.x - e1.x) + Math::sqr(center.y - e1.y) + Math::sqr(center.z - e1.z)) <= M::sqr(radius)))
				return true;
			TVec3<T0> d;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={e1, e0, d}...
			{
				d.x = e1.x - e0.x;
				d.y = e1.y - e0.y;
				d.z = e1.z - e0.z;
			};
			TVec3<T0> delta;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={e0, center, delta}...
			{
				delta.x = e0.x - center.x;
				delta.y = e0.y - center.y;
				delta.z = e0.z - center.z;
			};
			T0	pa = (d.x*d.x + d.y*d.y + d.z*d.z),
					pb = 2*(d.x*delta.x + d.y*delta.y + d.z*delta.z),
					pc = (delta.x*delta.x + delta.y*delta.y + delta.z*delta.z)-M::sqr(radius),
					rs[2];
			BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
			if (!num_rs)
				return false;
			T0	alpha = smallestPositiveResult(rs,num_rs);
			return alpha >= (T0)0 && alpha <= (T0)1;
		}

	/**
		@brief <br>
		<br>
		3 dimensional specialized version of detectSphereEdgeIntersection/DetectSphereEdgeIntersection()<br>
		DetectSphereEdgeIntersection() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
		Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
	
		@param[in] center 
		@param[in] radius 
		@param[in] e0 
		@param[in] e1 
		@return  
	*/
	template <typename T0, typename T1, typename T2, typename T3>
		inline	bool	__fastcall	DetectSphereEdgeIntersection(const TVec3<T0>& center, T1 radius, const TVec3<T2>& e0, const TVec3<T3>& e1)throw()
		{
			if (((Math::sqr(center.x - e0.x) + Math::sqr(center.y - e0.y) + Math::sqr(center.z - e0.z)) <= M::sqr(radius)) || ((Math::sqr(center.x - e1.x) + Math::sqr(center.y - e1.y) + Math::sqr(center.z - e1.z)) <= M::sqr(radius)))
				return true;
			TVec3<T0> d;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={e1, e0, d}...
			{
				d.x = e1.x - e0.x;
				d.y = e1.y - e0.y;
				d.z = e1.z - e0.z;
			};
			TVec3<T0> delta;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={e0, center, delta}...
			{
				delta.x = e0.x - center.x;
				delta.y = e0.y - center.y;
				delta.z = e0.z - center.z;
			};
			T0	pa = (d.x*d.x + d.y*d.y + d.z*d.z),
					pb = 2*(d.x*delta.x + d.y*delta.y + d.z*delta.z),
					pc = (delta.x*delta.x + delta.y*delta.y + delta.z*delta.z)-M::sqr(radius),
					rs[2];
			BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
			if (!num_rs)
				return false;
			T0	alpha = smallestPositiveResult(rs,num_rs);
			return alpha >= (T0)0 && alpha <= (T0)1;
		}

	/**
		@brief <br>
		<br>
		4 dimensional specialized version of detectSphereEdgeIntersection/DetectSphereEdgeIntersection()<br>
		detectSphereEdgeIntersection() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
		Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
	
		@param[in] center 
		@param[in] radius 
		@param[in] e0 
		@param[in] e1 
		@return  
	*/
	template <typename T0, typename T1, typename T2, typename T3>
		inline	bool	__fastcall	detectSphereEdgeIntersection(const TVec4<T0>& center, T1 radius, const TVec4<T2>& e0, const TVec4<T3>& e1)throw()
		{
			if (((Math::sqr(center.x - e0.x) + Math::sqr(center.y - e0.y) + Math::sqr(center.z - e0.z) + Math::sqr(center.w - e0.w)) <= M::sqr(radius)) || ((Math::sqr(center.x - e1.x) + Math::sqr(center.y - e1.y) + Math::sqr(center.z - e1.z) + Math::sqr(center.w - e1.w)) <= M::sqr(radius)))
				return true;
			TVec4<T0> d;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=4, assembly_mode='Objects', parameters={e1, e0, d}...
			{
				d.x = e1.x - e0.x;
				d.y = e1.y - e0.y;
				d.z = e1.z - e0.z;
				d.w = e1.w - e0.w;
			};
			TVec4<T0> delta;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=4, assembly_mode='Objects', parameters={e0, center, delta}...
			{
				delta.x = e0.x - center.x;
				delta.y = e0.y - center.y;
				delta.z = e0.z - center.z;
				delta.w = e0.w - center.w;
			};
			T0	pa = (d.x*d.x + d.y*d.y + d.z*d.z + d.w*d.w),
					pb = 2*(d.x*delta.x + d.y*delta.y + d.z*delta.z + d.w*delta.w),
					pc = (delta.x*delta.x + delta.y*delta.y + delta.z*delta.z + delta.w*delta.w)-M::sqr(radius),
					rs[2];
			BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
			if (!num_rs)
				return false;
			T0	alpha = smallestPositiveResult(rs,num_rs);
			return alpha >= (T0)0 && alpha <= (T0)1;
		}

	/**
		@brief <br>
		<br>
		4 dimensional specialized version of detectSphereEdgeIntersection/DetectSphereEdgeIntersection()<br>
		DetectSphereEdgeIntersection() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
		Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
	
		@param[in] center 
		@param[in] radius 
		@param[in] e0 
		@param[in] e1 
		@return  
	*/
	template <typename T0, typename T1, typename T2, typename T3>
		inline	bool	__fastcall	DetectSphereEdgeIntersection(const TVec4<T0>& center, T1 radius, const TVec4<T2>& e0, const TVec4<T3>& e1)throw()
		{
			if (((Math::sqr(center.x - e0.x) + Math::sqr(center.y - e0.y) + Math::sqr(center.z - e0.z) + Math::sqr(center.w - e0.w)) <= M::sqr(radius)) || ((Math::sqr(center.x - e1.x) + Math::sqr(center.y - e1.y) + Math::sqr(center.z - e1.z) + Math::sqr(center.w - e1.w)) <= M::sqr(radius)))
				return true;
			TVec4<T0> d;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=4, assembly_mode='Objects', parameters={e1, e0, d}...
			{
				d.x = e1.x - e0.x;
				d.y = e1.y - e0.y;
				d.z = e1.z - e0.z;
				d.w = e1.w - e0.w;
			};
			TVec4<T0> delta;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=4, assembly_mode='Objects', parameters={e0, center, delta}...
			{
				delta.x = e0.x - center.x;
				delta.y = e0.y - center.y;
				delta.z = e0.z - center.z;
				delta.w = e0.w - center.w;
			};
			T0	pa = (d.x*d.x + d.y*d.y + d.z*d.z + d.w*d.w),
					pb = 2*(d.x*delta.x + d.y*delta.y + d.z*delta.z + d.w*delta.w),
					pc = (delta.x*delta.x + delta.y*delta.y + delta.z*delta.z + delta.w*delta.w)-M::sqr(radius),
					rs[2];
			BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
			if (!num_rs)
				return false;
			T0	alpha = smallestPositiveResult(rs,num_rs);
			return alpha >= (T0)0 && alpha <= (T0)1;
		}

	//now implementing template definition 'bool detectSphereEdgeIntersection|DetectSphereEdgeIntersection (2..4) (<const [*] center>, <radius>, <const [*] e0>, <const [*] e1>, <[2] distances>) direct='
	/**
		@brief Detects whether or not there is an intersection between the given sphere and edge. Returns the intersection distance factors between e0 (=0) and e1 (=1)<br>
		<br>
		2 dimensional specialized version of detectSphereEdgeIntersection/DetectSphereEdgeIntersection()<br>
		detectSphereEdgeIntersection() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
		Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
	
		@param[in] center 
		@param[in] radius 
		@param[in] e0 
		@param[in] e1 
		@param[out] distances Intersection distances
		@return True, if there is an intersection, or all intersection points lie inside the sphere, false otherwise 
	*/
	template <typename T0, typename T1, typename T2, typename T3, typename T4>
		inline	bool	__fastcall	detectSphereEdgeIntersection(const TVec2<T0>& center, T1 radius, const TVec2<T2>& e0, const TVec2<T3>& e1, TVec2<T4>& distances)throw()
		{
			TVec2<T4> d;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=2, assembly_mode='Objects', parameters={e1, e0, d}...
			{
				d.x = e1.x - e0.x;
				d.y = e1.y - e0.y;
			};
			TVec2<T4> delta;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=2, assembly_mode='Objects', parameters={e0, center, delta}...
			{
				delta.x = e0.x - center.x;
				delta.y = e0.y - center.y;
			};
			T4	pa = (d.x*d.x + d.y*d.y),
					pb = 2*(d.x*delta.x + d.y*delta.y),
					pc = (delta.x*delta.x + delta.y*delta.y)-M::sqr(radius),
					rs[2];
			BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
			if (!num_rs)
				return false;
			if (num_rs == 1)
			{
				distances.x = distances.y = rs[0];
				return rs[0] >= (T4)0 && rs[0] <= (T4)1;
			}
			distances.x = std::min(rs[0],rs[1]);
			distances.y = std::max(rs[0],rs[1]);
			return (distances.x <= (T4)1 && distances.y >= (T4)0);
		}

	/**
		@brief Detects whether or not there is an intersection between the given sphere and edge. Returns the intersection distance factors between e0 (=0) and e1 (=1)<br>
		<br>
		2 dimensional specialized version of detectSphereEdgeIntersection/DetectSphereEdgeIntersection()<br>
		DetectSphereEdgeIntersection() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
		Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
	
		@param[in] center 
		@param[in] radius 
		@param[in] e0 
		@param[in] e1 
		@param[out] distances Intersection distances
		@return True, if there is an intersection, or all intersection points lie inside the sphere, false otherwise 
	*/
	template <typename T0, typename T1, typename T2, typename T3, typename T4>
		inline	bool	__fastcall	DetectSphereEdgeIntersection(const TVec2<T0>& center, T1 radius, const TVec2<T2>& e0, const TVec2<T3>& e1, TVec2<T4>& distances)throw()
		{
			TVec2<T4> d;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=2, assembly_mode='Objects', parameters={e1, e0, d}...
			{
				d.x = e1.x - e0.x;
				d.y = e1.y - e0.y;
			};
			TVec2<T4> delta;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=2, assembly_mode='Objects', parameters={e0, center, delta}...
			{
				delta.x = e0.x - center.x;
				delta.y = e0.y - center.y;
			};
			T4	pa = (d.x*d.x + d.y*d.y),
					pb = 2*(d.x*delta.x + d.y*delta.y),
					pc = (delta.x*delta.x + delta.y*delta.y)-M::sqr(radius),
					rs[2];
			BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
			if (!num_rs)
				return false;
			if (num_rs == 1)
			{
				distances.x = distances.y = rs[0];
				return rs[0] >= (T4)0 && rs[0] <= (T4)1;
			}
			distances.x = std::min(rs[0],rs[1]);
			distances.y = std::max(rs[0],rs[1]);
			return (distances.x <= (T4)1 && distances.y >= (T4)0);
		}

	/**
		@brief Detects whether or not there is an intersection between the given sphere and edge. Returns the intersection distance factors between e0 (=0) and e1 (=1)<br>
		<br>
		3 dimensional specialized version of detectSphereEdgeIntersection/DetectSphereEdgeIntersection()<br>
		detectSphereEdgeIntersection() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
		Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
	
		@param[in] center 
		@param[in] radius 
		@param[in] e0 
		@param[in] e1 
		@param[out] distances Intersection distances
		@return True, if there is an intersection, or all intersection points lie inside the sphere, false otherwise 
	*/
	template <typename T0, typename T1, typename T2, typename T3, typename T4>
		inline	bool	__fastcall	detectSphereEdgeIntersection(const TVec3<T0>& center, T1 radius, const TVec3<T2>& e0, const TVec3<T3>& e1, TVec2<T4>& distances)throw()
		{
			TVec3<T4> d;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={e1, e0, d}...
			{
				d.x = e1.x - e0.x;
				d.y = e1.y - e0.y;
				d.z = e1.z - e0.z;
			};
			TVec3<T4> delta;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={e0, center, delta}...
			{
				delta.x = e0.x - center.x;
				delta.y = e0.y - center.y;
				delta.z = e0.z - center.z;
			};
			T4	pa = (d.x*d.x + d.y*d.y + d.z*d.z),
					pb = 2*(d.x*delta.x + d.y*delta.y + d.z*delta.z),
					pc = (delta.x*delta.x + delta.y*delta.y + delta.z*delta.z)-M::sqr(radius),
					rs[2];
			BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
			if (!num_rs)
				return false;
			if (num_rs == 1)
			{
				distances.x = distances.y = rs[0];
				return rs[0] >= (T4)0 && rs[0] <= (T4)1;
			}
			distances.x = std::min(rs[0],rs[1]);
			distances.y = std::max(rs[0],rs[1]);
			return (distances.x <= (T4)1 && distances.y >= (T4)0);
		}

	/**
		@brief Detects whether or not there is an intersection between the given sphere and edge. Returns the intersection distance factors between e0 (=0) and e1 (=1)<br>
		<br>
		3 dimensional specialized version of detectSphereEdgeIntersection/DetectSphereEdgeIntersection()<br>
		DetectSphereEdgeIntersection() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
		Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
	
		@param[in] center 
		@param[in] radius 
		@param[in] e0 
		@param[in] e1 
		@param[out] distances Intersection distances
		@return True, if there is an intersection, or all intersection points lie inside the sphere, false otherwise 
	*/
	template <typename T0, typename T1, typename T2, typename T3, typename T4>
		inline	bool	__fastcall	DetectSphereEdgeIntersection(const TVec3<T0>& center, T1 radius, const TVec3<T2>& e0, const TVec3<T3>& e1, TVec2<T4>& distances)throw()
		{
			TVec3<T4> d;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={e1, e0, d}...
			{
				d.x = e1.x - e0.x;
				d.y = e1.y - e0.y;
				d.z = e1.z - e0.z;
			};
			TVec3<T4> delta;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={e0, center, delta}...
			{
				delta.x = e0.x - center.x;
				delta.y = e0.y - center.y;
				delta.z = e0.z - center.z;
			};
			T4	pa = (d.x*d.x + d.y*d.y + d.z*d.z),
					pb = 2*(d.x*delta.x + d.y*delta.y + d.z*delta.z),
					pc = (delta.x*delta.x + delta.y*delta.y + delta.z*delta.z)-M::sqr(radius),
					rs[2];
			BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
			if (!num_rs)
				return false;
			if (num_rs == 1)
			{
				distances.x = distances.y = rs[0];
				return rs[0] >= (T4)0 && rs[0] <= (T4)1;
			}
			distances.x = std::min(rs[0],rs[1]);
			distances.y = std::max(rs[0],rs[1]);
			return (distances.x <= (T4)1 && distances.y >= (T4)0);
		}

	/**
		@brief Detects whether or not there is an intersection between the given sphere and edge. Returns the intersection distance factors between e0 (=0) and e1 (=1)<br>
		<br>
		4 dimensional specialized version of detectSphereEdgeIntersection/DetectSphereEdgeIntersection()<br>
		detectSphereEdgeIntersection() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
		Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
	
		@param[in] center 
		@param[in] radius 
		@param[in] e0 
		@param[in] e1 
		@param[out] distances Intersection distances
		@return True, if there is an intersection, or all intersection points lie inside the sphere, false otherwise 
	*/
	template <typename T0, typename T1, typename T2, typename T3, typename T4>
		inline	bool	__fastcall	detectSphereEdgeIntersection(const TVec4<T0>& center, T1 radius, const TVec4<T2>& e0, const TVec4<T3>& e1, TVec2<T4>& distances)throw()
		{
			TVec4<T4> d;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=4, assembly_mode='Objects', parameters={e1, e0, d}...
			{
				d.x = e1.x - e0.x;
				d.y = e1.y - e0.y;
				d.z = e1.z - e0.z;
				d.w = e1.w - e0.w;
			};
			TVec4<T4> delta;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=4, assembly_mode='Objects', parameters={e0, center, delta}...
			{
				delta.x = e0.x - center.x;
				delta.y = e0.y - center.y;
				delta.z = e0.z - center.z;
				delta.w = e0.w - center.w;
			};
			T4	pa = (d.x*d.x + d.y*d.y + d.z*d.z + d.w*d.w),
					pb = 2*(d.x*delta.x + d.y*delta.y + d.z*delta.z + d.w*delta.w),
					pc = (delta.x*delta.x + delta.y*delta.y + delta.z*delta.z + delta.w*delta.w)-M::sqr(radius),
					rs[2];
			BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
			if (!num_rs)
				return false;
			if (num_rs == 1)
			{
				distances.x = distances.y = rs[0];
				return rs[0] >= (T4)0 && rs[0] <= (T4)1;
			}
			distances.x = std::min(rs[0],rs[1]);
			distances.y = std::max(rs[0],rs[1]);
			return (distances.x <= (T4)1 && distances.y >= (T4)0);
		}

	/**
		@brief Detects whether or not there is an intersection between the given sphere and edge. Returns the intersection distance factors between e0 (=0) and e1 (=1)<br>
		<br>
		4 dimensional specialized version of detectSphereEdgeIntersection/DetectSphereEdgeIntersection()<br>
		DetectSphereEdgeIntersection() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
		Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
	
		@param[in] center 
		@param[in] radius 
		@param[in] e0 
		@param[in] e1 
		@param[out] distances Intersection distances
		@return True, if there is an intersection, or all intersection points lie inside the sphere, false otherwise 
	*/
	template <typename T0, typename T1, typename T2, typename T3, typename T4>
		inline	bool	__fastcall	DetectSphereEdgeIntersection(const TVec4<T0>& center, T1 radius, const TVec4<T2>& e0, const TVec4<T3>& e1, TVec2<T4>& distances)throw()
		{
			TVec4<T4> d;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=4, assembly_mode='Objects', parameters={e1, e0, d}...
			{
				d.x = e1.x - e0.x;
				d.y = e1.y - e0.y;
				d.z = e1.z - e0.z;
				d.w = e1.w - e0.w;
			};
			TVec4<T4> delta;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=4, assembly_mode='Objects', parameters={e0, center, delta}...
			{
				delta.x = e0.x - center.x;
				delta.y = e0.y - center.y;
				delta.z = e0.z - center.z;
				delta.w = e0.w - center.w;
			};
			T4	pa = (d.x*d.x + d.y*d.y + d.z*d.z + d.w*d.w),
					pb = 2*(d.x*delta.x + d.y*delta.y + d.z*delta.z + d.w*delta.w),
					pc = (delta.x*delta.x + delta.y*delta.y + delta.z*delta.z + delta.w*delta.w)-M::sqr(radius),
					rs[2];
			BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
			if (!num_rs)
				return false;
			if (num_rs == 1)
			{
				distances.x = distances.y = rs[0];
				return rs[0] >= (T4)0 && rs[0] <= (T4)1;
			}
			distances.x = std::min(rs[0],rs[1]);
			distances.y = std::max(rs[0],rs[1]);
			return (distances.x <= (T4)1 && distances.y >= (T4)0);
		}

	//now implementing template definition 'bool detTriangleRayIntersection (<const [3] t0>, <const [3] t1>, <const [3] t2>, <const [3] b>, <const [3] f>, <[3] result>) direct='
	/**
		@brief Similar to _oDetTriangleEdgeIntersection(), _oDetTriangleRayIntersection() determines the factors (a, b, c) of the intersection of the specified triangle and ray (if any).<br>Unless the ray and triangle are parallel the resulting factors are written to the specified out array: out[0] = a, out[1] = b, out[2] = c.<br>The solved intersection equation is: x (point of intersection) = t0 + (t1-t0)*a + (t2-t0)*b = b + f*c<br>
		<br>
		detTriangleRayIntersection() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
	
		@param[in] t0 
		@param[in] t1 
		@param[in] t2 
		@param[in] b 
		@param[in] f Ray direction vector: any length greater 0
		@param[out] result 
		@return  true if a point of intersection could be determined, false otherwise. The out array will remain unchanged if the function returns false. 
	*/
	template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
		inline	bool	__fastcall	detTriangleRayIntersection(const TVec3<T0>& t0, const TVec3<T1>& t1, const TVec3<T2>& t2, const TVec3<T3>& b, const TVec3<T4>& f, TVec3<T5>& result)throw()
		{
			TVec3<T5> d0, d1, n, dif, n0, n1, cross_point;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={t1, t0, d0}...
			{
				d0.x = t1.x - t0.x;
				d0.y = t1.y - t0.y;
				d0.z = t1.z - t0.z;
			};
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={t2, t0, d1}...
			{
				d1.x = t2.x - t0.x;
				d1.y = t2.y - t0.y;
				d1.z = t2.z - t0.z;
			};
			
			//block inlining void cross (<const [3] v>, <const [3] w>, <[3] result>) direct= for dimensions=3, assembly_mode='Objects', parameters={d0, d1, n}...
			{
				n.x = d0.y*d1.z - d0.z*d1.y;
				n.y = d0.z*d1.x - d0.x*d1.z;
				n.z = d0.x*d1.y - d0.y*d1.x;
			};
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={t0, b, dif}...
			{
				dif.x = t0.x - b.x;
				dif.y = t0.y - b.y;
				dif.z = t0.z - b.z;
			};
			T5	sub_alpha = (n.x*f.x + n.y*f.y + n.z*f.z);
			if (!sub_alpha)
				return false;
			T5	alpha = static_cast<T5>((n.x*dif.x + n.y*dif.y + n.z*dif.z))/static_cast<T5>(sub_alpha);
			
			//block inlining void cross (<const [3] v>, <const [3] w>, <[3] result>) direct= for dimensions=3, assembly_mode='Objects', parameters={n, d0, n0}...
			{
				n0.x = n.y*d0.z - n.z*d0.y;
				n0.y = n.z*d0.x - n.x*d0.z;
				n0.z = n.x*d0.y - n.y*d0.x;
			};
			
			//block inlining void cross (<const [3] v>, <const [3] w>, <[3] result>) direct= for dimensions=3, assembly_mode='Objects', parameters={n, d1, n1}...
			{
				n1.x = n.y*d1.z - n.z*d1.y;
				n1.y = n.z*d1.x - n.x*d1.z;
				n1.z = n.x*d1.y - n.y*d1.x;
			};
			
			//block inlining void multAdd|mad (2..4) (<const [*] base>, <const [*] vector>, <scalar>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={b, f, alpha, cross_point}...
			{
				cross_point.x = b.x + f.x * alpha;
				cross_point.y = b.y + f.y * alpha;
				cross_point.z = b.z + f.z * alpha;
			};
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={cross_point, t0, dif}...
			{
				dif.x = cross_point.x - t0.x;
				dif.y = cross_point.y - t0.y;
				dif.z = cross_point.z - t0.z;
			};
			T5	beta = static_cast<T5>((n0.x*dif.x + n0.y*dif.y + n0.z*dif.z))/static_cast<T5>((n0.x*d1.x + n0.y*d1.y + n0.z*d1.z)),
					gamma = static_cast<T5>((n1.x*dif.x + n1.y*dif.y + n1.z*dif.z))/static_cast<T5>((n1.x*d0.x + n1.y*d0.y + n1.z*d0.z));
			result.x = gamma;
			result.y = beta;
			result.z = alpha;
			return true;
		}

	//now implementing template definition 'bool detDeltaTriangleRayIntersection (<const [3] t_base>, <const [3] edge_x>, <const [3] edge_y>, <const [3] b>, <const [3] f>, <[3] result>) direct='
	/**
		@brief @copydoc detTriangleRayIntersectionE<br />
		Variant of detTriangleRayIntersection, using edges delta, rather than absolute positions<br>
		<br>
		detDeltaTriangleRayIntersection() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
	
		@param[in] t_base 
		@param[in] edge_x 
		@param[in] edge_y 
		@param[in] b 
		@param[in] f Ray direction vector: any length greater 0
		@param[out] result 
		@return  true if a point of intersection could be determined, false otherwise. The out array will remain unchanged if the function returns false. 
	*/
	template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
		inline	bool	__fastcall	detDeltaTriangleRayIntersection(const TVec3<T0>& t_base, const TVec3<T1>& edge_x, const TVec3<T2>& edge_y, const TVec3<T3>& b, const TVec3<T4>& f, TVec3<T5>& result)throw()
		{
			TVec3<T5> n, dif, n0, n1, cross_point;
			
			//block inlining void cross (<const [3] v>, <const [3] w>, <[3] result>) direct= for dimensions=3, assembly_mode='Objects', parameters={edge_x, edge_y, n}...
			{
				n.x = edge_x.y*edge_y.z - edge_x.z*edge_y.y;
				n.y = edge_x.z*edge_y.x - edge_x.x*edge_y.z;
				n.z = edge_x.x*edge_y.y - edge_x.y*edge_y.x;
			};
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={t_base, b, dif}...
			{
				dif.x = t_base.x - b.x;
				dif.y = t_base.y - b.y;
				dif.z = t_base.z - b.z;
			};
			T5	sub_alpha = (n.x*f.x + n.y*f.y + n.z*f.z);
			if (!sub_alpha)
				return false;
			T5	alpha = static_cast<T5>((n.x*dif.x + n.y*dif.y + n.z*dif.z))/static_cast<T5>(sub_alpha);
			
			//block inlining void cross (<const [3] v>, <const [3] w>, <[3] result>) direct= for dimensions=3, assembly_mode='Objects', parameters={n, edge_x, n0}...
			{
				n0.x = n.y*edge_x.z - n.z*edge_x.y;
				n0.y = n.z*edge_x.x - n.x*edge_x.z;
				n0.z = n.x*edge_x.y - n.y*edge_x.x;
			};
			
			//block inlining void cross (<const [3] v>, <const [3] w>, <[3] result>) direct= for dimensions=3, assembly_mode='Objects', parameters={n, edge_y, n1}...
			{
				n1.x = n.y*edge_y.z - n.z*edge_y.y;
				n1.y = n.z*edge_y.x - n.x*edge_y.z;
				n1.z = n.x*edge_y.y - n.y*edge_y.x;
			};
			
			//block inlining void multAdd|mad (2..4) (<const [*] base>, <const [*] vector>, <scalar>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={b, f, alpha, cross_point}...
			{
				cross_point.x = b.x + f.x * alpha;
				cross_point.y = b.y + f.y * alpha;
				cross_point.z = b.z + f.z * alpha;
			};
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={cross_point, t_base, dif}...
			{
				dif.x = cross_point.x - t_base.x;
				dif.y = cross_point.y - t_base.y;
				dif.z = cross_point.z - t_base.z;
			};
			T5	beta = static_cast<T5>((n0.x*dif.x + n0.y*dif.y + n0.z*dif.z))/static_cast<T5>((n0.x*edge_y.x + n0.y*edge_y.y + n0.z*edge_y.z)),
					gamma = static_cast<T5>((n1.x*dif.x + n1.y*dif.y + n1.z*dif.z))/static_cast<T5>((n1.x*edge_x.x + n1.y*edge_x.y + n1.z*edge_x.z));
			result.x = gamma;
			result.y = beta;
			result.z = alpha;
			return true;
		}

	//now implementing template definition 'void triangleNormal (<const [3] p0>, <const [3] p1>, <const [3] p2>, <[3] result>) direct='
	/**
		@brief <br>
		<br>
		triangleNormal() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
	
		@param[in] p0 
		@param[in] p1 
		@param[in] p2 
		@param[out] result 
	*/
	template <typename T0, typename T1, typename T2, typename T3>
		inline	void	__fastcall	triangleNormal(const TVec3<T0>& p0, const TVec3<T1>& p1, const TVec3<T2>& p2, TVec3<T3>& result)throw()
		{
			TVec3<T3> v, w;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={p1, p0, v}...
			{
				v.x = p1.x - p0.x;
				v.y = p1.y - p0.y;
				v.z = p1.z - p0.z;
			};
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={p2, p0, w}...
			{
				w.x = p2.x - p0.x;
				w.y = p2.y - p0.y;
				w.z = p2.z - p0.z;
			};
			
			//block inlining void cross (<const [3] v>, <const [3] w>, <[3] result>) direct= for dimensions=3, assembly_mode='Objects', parameters={v, w, result}...
			{
				result.x = v.y*w.z - v.z*w.y;
				result.y = v.z*w.x - v.x*w.z;
				result.z = v.x*w.y - v.y*w.x;
			};
		}

	//now implementing template definition 'T4 quadraticTriangleDistance (<const [3] t0>, <const [3] t1>, <const [3] t2>, <const [3] p>, <[3] normal>) direct='
	/**
		@brief <br>
		<br>
		quadraticTriangleDistance() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
	
		@param[in] t0 
		@param[in] t1 
		@param[in] t2 
		@param[in] p 
		@param[out] normal 
		@return  
	*/
	template <typename T0, typename T1, typename T2, typename T3, typename T4>
		inline	T4	__fastcall	quadraticTriangleDistance(const TVec3<T0>& t0, const TVec3<T1>& t1, const TVec3<T2>& t2, const TVec3<T3>& p, TVec3<T4>& normal)throw()
		{
			
			//block inlining void triangleNormal (<const [3] p0>, <const [3] p1>, <const [3] p2>, <[3] result>) direct= for dimensions=3, assembly_mode='Objects', parameters={t0, t1, t2, normal}...
			{
				TVec3<T4> v, w;
				
				//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] normal>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={t1, t0, v}...
				{
					v.x = t1.x - t0.x;
					v.y = t1.y - t0.y;
					v.z = t1.z - t0.z;
				};
				
				//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] normal>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={t2, t0, w}...
				{
					w.x = t2.x - t0.x;
					w.y = t2.y - t0.y;
					w.z = t2.z - t0.z;
				};
				
				//block inlining void cross (<const [3] v>, <const [3] w>, <[3] normal>) direct= for dimensions=3, assembly_mode='Objects', parameters={v, w, normal}...
				{
					normal.x = v.y*w.z - v.z*w.y;
					normal.y = v.z*w.x - v.x*w.z;
					normal.z = v.x*w.y - v.y*w.x;
				};
			};
			TVec3<T4> d0, d1, dif, n0, n1, cross_point;
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={t1, t0, d0}...
			{
				d0.x = t1.x - t0.x;
				d0.y = t1.y - t0.y;
				d0.z = t1.z - t0.z;
			};
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={t2, t0, d1}...
			{
				d1.x = t2.x - t0.x;
				d1.y = t2.y - t0.y;
				d1.z = t2.z - t0.z;
			};
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={p, t0, dif}...
			{
				dif.x = p.x - t0.x;
				dif.y = p.y - t0.y;
				dif.z = p.z - t0.z;
			};
			T4	sub_alpha = (normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);
			using std::fabs;
			using Math::GetError;
			if (fabs(sub_alpha) <= GetError<T4>())
				return (Math::sqr(t0.x - p.x) + Math::sqr(t0.y - p.y) + Math::sqr(t0.z - p.z));
			T4	alpha = (normal.x*dif.x + normal.y*dif.y + normal.z*dif.z)/sub_alpha;
			
			//block inlining void cross (<const [3] v>, <const [3] w>, <[3] result>) direct= for dimensions=3, assembly_mode='Objects', parameters={normal, d0, n0}...
			{
				n0.x = normal.y*d0.z - normal.z*d0.y;
				n0.y = normal.z*d0.x - normal.x*d0.z;
				n0.z = normal.x*d0.y - normal.y*d0.x;
			};
			
			//block inlining void cross (<const [3] v>, <const [3] w>, <[3] result>) direct= for dimensions=3, assembly_mode='Objects', parameters={normal, d1, n1}...
			{
				n1.x = normal.y*d1.z - normal.z*d1.y;
				n1.y = normal.z*d1.x - normal.x*d1.z;
				n1.z = normal.x*d1.y - normal.y*d1.x;
			};
			
			//block inlining void multAdd|mad (2..4) (<const [*] base>, <const [*] vector>, <scalar>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={p, normal, alpha, cross_point}...
			{
				cross_point.x = p.x + normal.x * alpha;
				cross_point.y = p.y + normal.y * alpha;
				cross_point.z = p.z + normal.z * alpha;
			};
			
			//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={cross_point, t0, dif}...
			{
				dif.x = cross_point.x - t0.x;
				dif.y = cross_point.y - t0.y;
				dif.z = cross_point.z - t0.z;
			};
			T4	beta = (n0.x*dif.x + n0.y*dif.y + n0.z*dif.z)/(n0.x*d1.x + n0.y*d1.y + n0.z*d1.z),
					gamma = (n1.x*dif.x + n1.y*dif.y + n1.z*dif.z)/(n1.x*d0.x + n1.y*d0.y + n1.z*d0.z);
			if (beta >= 0)
			{
				if (gamma >= 0)
				{
					if (beta+gamma <= 1)
						return (Math::sqr(cross_point.x - p.x) + Math::sqr(cross_point.y - p.y) + Math::sqr(cross_point.z - p.z));
					TVec3<T4> d2, n2;
					
					//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={t2, t1, d2}...
					{
						d2.x = t2.x - t1.x;
						d2.y = t2.y - t1.y;
						d2.z = t2.z - t1.z;
					};
					T4	fc = ((cross_point.x*d2.x + cross_point.y*d2.y + cross_point.z*d2.z)-(t1.x*d2.x + t1.y*d2.y + t1.z*d2.z))/(d2.x*d2.x + d2.y*d2.y + d2.z*d2.z);
					if (fc < 0)
						return (Math::sqr(t1.x - p.x) + Math::sqr(t1.y - p.y) + Math::sqr(t1.z - p.z));
					if (fc > 1)
						return (Math::sqr(t2.x - p.x) + Math::sqr(t2.y - p.y) + Math::sqr(t2.z - p.z));
					{
						TVec3<T4> d0, d1;
						
						//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={t1, p, d1}...
						{
							d1.x = t1.x - p.x;
							d1.y = t1.y - p.y;
							d1.z = t1.z - p.z;
						};
						
						//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={t2, t1, d0}...
						{
							d0.x = t2.x - t1.x;
							d0.y = t2.y - t1.y;
							d0.z = t2.z - t1.z;
						};
						TVec3<T4> r;
						
						//block inlining void cross (<const [3] v>, <const [3] w>, <[3] result>) direct= for dimensions=3, assembly_mode='Objects', parameters={d0, d1, r}...
						{
							r.x = d0.y*d1.z - d0.z*d1.y;
							r.y = d0.z*d1.x - d0.x*d1.z;
							r.z = d0.x*d1.y - d0.y*d1.x;
						};
						return (r.x*r.x + r.y*r.y + r.z*r.z)/(d0.x*d0.x + d0.y*d0.y + d0.z*d0.z);
					}
				}
				T4	fc = ((cross_point.x*d1.x + cross_point.y*d1.y + cross_point.z*d1.z)-(t0.x*d1.x + t0.y*d1.y + t0.z*d1.z))/(d1.x*d1.x + d1.y*d1.y + d1.z*d1.z);
				if (fc < 0)
					return (Math::sqr(t0.x - p.x) + Math::sqr(t0.y - p.y) + Math::sqr(t0.z - p.z));
				if (fc > 1)
					return (Math::sqr(t2.x - p.x) + Math::sqr(t2.y - p.y) + Math::sqr(t2.z - p.z));
				{
					TVec3<T4> dx;
					
					//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={t0, p, dx}...
					{
						dx.x = t0.x - p.x;
						dx.y = t0.y - p.y;
						dx.z = t0.z - p.z;
					};
					TVec3<T4> r;
					
					//block inlining void cross (<const [3] v>, <const [3] w>, <[3] result>) direct= for dimensions=3, assembly_mode='Objects', parameters={d1, dx, r}...
					{
						r.x = d1.y*dx.z - d1.z*dx.y;
						r.y = d1.z*dx.x - d1.x*dx.z;
						r.z = d1.x*dx.y - d1.y*dx.x;
					};
					return (r.x*r.x + r.y*r.y + r.z*r.z)/(d1.x*d1.x + d1.y*d1.y + d1.z*d1.z);
				}
			}
			T4	fc = ((cross_point.x*d0.x + cross_point.y*d0.y + cross_point.z*d0.z)-(t0.x*d0.x + t0.y*d0.y + t0.z*d0.z))/(d0.x*d0.x + d0.y*d0.y + d0.z*d0.z);
			if (fc < 0)
				return (Math::sqr(t0.x - p.x) + Math::sqr(t0.y - p.y) + Math::sqr(t0.z - p.z));
			if (fc > 1)
				return (Math::sqr(t1.x - p.x) + Math::sqr(t1.y - p.y) + Math::sqr(t1.z - p.z));
			{
				TVec3<T4> dx;
				
				//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={t0, p, dx}...
				{
					dx.x = t0.x - p.x;
					dx.y = t0.y - p.y;
					dx.z = t0.z - p.z;
				};
				TVec3<T4> r;
				
				//block inlining void cross (<const [3] v>, <const [3] w>, <[3] result>) direct= for dimensions=3, assembly_mode='Objects', parameters={dx, d0, r}...
				{
					r.x = dx.y*d0.z - dx.z*d0.y;
					r.y = dx.z*d0.x - dx.x*d0.z;
					r.z = dx.x*d0.y - dx.y*d0.x;
				};
				return (r.x*r.x + r.y*r.y + r.z*r.z)/(d0.x*d0.x + d0.y*d0.y + d0.z*d0.z);
			}
		}

	//now implementing template definition 'T4 triangleDistance (<const [3] t0>, <const [3] t1>, <const [3] t2>, <const [3] p>, <[3] normal>) direct='
	/**
		@brief <br>
		<br>
		triangleDistance() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
	
		@param[in] t0 
		@param[in] t1 
		@param[in] t2 
		@param[in] p 
		@param[out] normal 
		@return  
	*/
	template <typename T0, typename T1, typename T2, typename T3, typename T4>
		inline	T4	__fastcall	triangleDistance(const TVec3<T0>& t0, const TVec3<T1>& t1, const TVec3<T2>& t2, const TVec3<T3>& p, TVec3<T4>& normal)throw()
		{
			return vsqrt(quadraticTriangleDistance(t0,t1,t2,p,normal));
		}

};

template <count_t Current, count_t Dimensions, bool Terminal>
	class ObjV_Include__
	{};
template <count_t Current, count_t Dimensions>
	class ObjV_Include__<Current,Dimensions,true>
	{
	public:

		/* ----- Now implementing pointer based recursion terminators ----- */

		/* ----- Now implementing object based recursion terminators ----- */

		/* ----- Now implementing helper class terminators ----- */
		template <typename T0, typename T1>
			static inline	T0	__fastcall	Operator3_sum(const T0 __p0[Dimensions], const T1 __p1[Dimensions])throw()
			{
				return Math::sqr(__p0[Current] - __p1[Current]);
			}

		template <typename T0, typename T1>
			static inline	T0	__fastcall	Operator4_sum(const T0 __p0[Dimensions], const T1 __p1[Dimensions])throw()
			{
				return __p0[Current]*__p1[Current];
			}

		template <typename T0>
			static inline	T0	__fastcall	Operator5_sum(const T0 __p0[Dimensions])throw()
			{
				return __p0[Current]*__p0[Current];
			}

		template <typename T0, typename T1>
			static inline	T0	__fastcall	Operator6_sum(const T0 __p0[Dimensions], const T1 __p1[Dimensions])throw()
			{
				return Math::sqr(__p0[Current] - __p1[Current]);
			}

		template <typename T0, typename T1>
			static inline	T0	__fastcall	Operator7_sum(const T0 __p0[Dimensions], const T1 __p1[Dimensions])throw()
			{
				return __p0[Current]*__p1[Current];
			}

		template <typename T0>
			static inline	T0	__fastcall	Operator8_sum(const T0 __p0[Dimensions])throw()
			{
				return __p0[Current]*__p0[Current];
			}

	};

template <count_t Current, count_t Dimensions>
	class ObjV_Include__<Current,Dimensions,false>
	{
	public:

		/* ----- Now implementing helper class iterators ----- */

		/* --- Now processing 'Pointers/Operator:+||const [*]|const [*]|<>|Math::sqr(v:i - w:i)' --- */
		template <typename T0, typename T1>
			static inline	T0	__fastcall	Operator3_sum(const T0 __p0[Dimensions], const T1 __p1[Dimensions])throw()
			{
				return (
						Math::sqr(__p0[Current] - __p1[Current]) +
						ObjV_Include__<Current+1,Dimensions, GreaterOrEqual<Current+2,Dimensions>::eval>::Operator3_sum(__p0,__p1)
					);
			}


		/* --- Now processing 'Pointers/Operator:+||const [*]|const [*]|<>|v:i*w:i' --- */
		template <typename T0, typename T1>
			static inline	T0	__fastcall	Operator4_sum(const T0 __p0[Dimensions], const T1 __p1[Dimensions])throw()
			{
				return (
						__p0[Current]*__p1[Current] +
						ObjV_Include__<Current+1,Dimensions, GreaterOrEqual<Current+2,Dimensions>::eval>::Operator4_sum(__p0,__p1)
					);
			}


		/* --- Now processing 'Pointers/Operator:+||const [*]|<>|v:i*v:i' --- */
		template <typename T0>
			static inline	T0	__fastcall	Operator5_sum(const T0 __p0[Dimensions])throw()
			{
				return (
						__p0[Current]*__p0[Current] +
						ObjV_Include__<Current+1,Dimensions, GreaterOrEqual<Current+2,Dimensions>::eval>::Operator5_sum(__p0)
					);
			}


		/* --- Now processing 'Objects/Operator:+||const [*]|const [*]|<>|Math::sqr(v:i - w:i)' --- */
		template <typename T0, typename T1>
			static inline	T0	__fastcall	Operator6_sum(const T0 __p0[Dimensions], const T1 __p1[Dimensions])throw()
			{
				return (
						Math::sqr(__p0[Current] - __p1[Current]) +
						ObjV_Include__<Current+1,Dimensions, GreaterOrEqual<Current+2,Dimensions>::eval>::Operator6_sum(__p0,__p1)
					);
			}


		/* --- Now processing 'Objects/Operator:+||const [*]|const [*]|<>|v:i*w:i' --- */
		template <typename T0, typename T1>
			static inline	T0	__fastcall	Operator7_sum(const T0 __p0[Dimensions], const T1 __p1[Dimensions])throw()
			{
				return (
						__p0[Current]*__p1[Current] +
						ObjV_Include__<Current+1,Dimensions, GreaterOrEqual<Current+2,Dimensions>::eval>::Operator7_sum(__p0,__p1)
					);
			}


		/* --- Now processing 'Objects/Operator:+||const [*]|<>|v:i*v:i' --- */
		template <typename T0>
			static inline	T0	__fastcall	Operator8_sum(const T0 __p0[Dimensions])throw()
			{
				return (
						__p0[Current]*__p0[Current] +
						ObjV_Include__<Current+1,Dimensions, GreaterOrEqual<Current+2,Dimensions>::eval>::Operator8_sum(__p0)
					);
			}


		/* ----- Now implementing pointer based recursion terminators ----- */

		/* ----- Now implementing object based recursion terminators ----- */

		/* ----- Now implementing pointer based recursion-indirect functions ----- */
		//now implementing template definition 'bool SphereContainsPoint (2..4) (<const [*] center>, <radius>, <const [*] p>) direct='
		/**
			@brief <br>
			<br>
			SphereContainsPoint() requires array pointers to operate on. Make sure all passed array pointers provide at least as many elements as required (<Dimensions> in general for this version)<br>
			Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
		
			@param[in] center 
			@param[in] radius 
			@param[in] p 
			@return  
		*/
		template <typename T0, typename T1, typename T2>
			static	inline	bool	__fastcall	SphereContainsPoint(const T0 center[Dimensions], T1 radius, const T2 p[Dimensions])throw()
			{
				return (ObjV_Include__<0,Dimensions,false>::Operator3_sum(center, p)/*Math::sqr(center:i - p:i)*/) <= M::sqr(radius);
			}

		//now implementing template definition 'bool detectSphereEdgeIntersection|DetectSphereEdgeIntersection (2..4) (<const [*] center>, <radius>, <const [*] e0>, <const [*] e1>) direct='
		/**
			@brief <br>
			<br>
			detectSphereEdgeIntersection() requires array pointers to operate on. Make sure all passed array pointers provide at least as many elements as required (<Dimensions> in general for this version)<br>
			Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
		
			@param[in] center 
			@param[in] radius 
			@param[in] e0 
			@param[in] e1 
			@return  
		*/
		template <typename T0, typename T1, typename T2, typename T3>
			static	inline	bool	__fastcall	detectSphereEdgeIntersection(const T0 center[Dimensions], T1 radius, const T2 e0[Dimensions], const T3 e1[Dimensions])throw()
			{
				if (((ObjV_Include__<0,Dimensions,false>::Operator3_sum(center, e0)/*Math::sqr(center:i - e0:i)*/) <= M::sqr(radius)) || ((ObjV_Include__<0,Dimensions,false>::Operator3_sum(center, e1)/*Math::sqr(center:i - e1:i)*/) <= M::sqr(radius)))
					return true;
				T0 d[Dimensions];
				
				//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Pointers', parameters={e1, e0, d}...
				{
					d[Current] = e1[Current] - e0[Current];
					VecV_Include__<Current+1,Dimensions,GreaterOrEqual<Current+2,Dimensions>::eval>::subtract(e1, e0, d);
				};
				T0 delta[Dimensions];
				
				//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Pointers', parameters={e0, center, delta}...
				{
					delta[Current] = e0[Current] - center[Current];
					VecV_Include__<Current+1,Dimensions,GreaterOrEqual<Current+2,Dimensions>::eval>::subtract(e0, center, delta);
				};
				T0	pa = (ObjV_Include__<0,Dimensions,false>::Operator4_sum(d, d)/*d:i*d:i*/),
						pb = 2*(ObjV_Include__<0,Dimensions,false>::Operator4_sum(d, delta)/*d:i*delta:i*/),
						pc = (ObjV_Include__<0,Dimensions,false>::Operator5_sum(delta)/*delta:i*delta:i*/)-M::sqr(radius),
						rs[2];
				BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
				if (!num_rs)
					return false;
				T0	alpha = smallestPositiveResult(rs,num_rs);
				return alpha >= (T0)0 && alpha <= (T0)1;
			}

		/**
			@brief <br>
			<br>
			DetectSphereEdgeIntersection() requires array pointers to operate on. Make sure all passed array pointers provide at least as many elements as required (<Dimensions> in general for this version)<br>
			Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
		
			@param[in] center 
			@param[in] radius 
			@param[in] e0 
			@param[in] e1 
			@return  
		*/
		template <typename T0, typename T1, typename T2, typename T3>
			static	inline	bool	__fastcall	DetectSphereEdgeIntersection(const T0 center[Dimensions], T1 radius, const T2 e0[Dimensions], const T3 e1[Dimensions])throw()
			{
				if (((ObjV_Include__<0,Dimensions,false>::Operator3_sum(center, e0)/*Math::sqr(center:i - e0:i)*/) <= M::sqr(radius)) || ((ObjV_Include__<0,Dimensions,false>::Operator3_sum(center, e1)/*Math::sqr(center:i - e1:i)*/) <= M::sqr(radius)))
					return true;
				T0 d[Dimensions];
				
				//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Pointers', parameters={e1, e0, d}...
				{
					d[Current] = e1[Current] - e0[Current];
					VecV_Include__<Current+1,Dimensions,GreaterOrEqual<Current+2,Dimensions>::eval>::subtract(e1, e0, d);
				};
				T0 delta[Dimensions];
				
				//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Pointers', parameters={e0, center, delta}...
				{
					delta[Current] = e0[Current] - center[Current];
					VecV_Include__<Current+1,Dimensions,GreaterOrEqual<Current+2,Dimensions>::eval>::subtract(e0, center, delta);
				};
				T0	pa = (ObjV_Include__<0,Dimensions,false>::Operator4_sum(d, d)/*d:i*d:i*/),
						pb = 2*(ObjV_Include__<0,Dimensions,false>::Operator4_sum(d, delta)/*d:i*delta:i*/),
						pc = (ObjV_Include__<0,Dimensions,false>::Operator5_sum(delta)/*delta:i*delta:i*/)-M::sqr(radius),
						rs[2];
				BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
				if (!num_rs)
					return false;
				T0	alpha = smallestPositiveResult(rs,num_rs);
				return alpha >= (T0)0 && alpha <= (T0)1;
			}

		//now implementing template definition 'bool detectSphereEdgeIntersection|DetectSphereEdgeIntersection (2..4) (<const [*] center>, <radius>, <const [*] e0>, <const [*] e1>, <[2] distances>) direct='
		/**
			@brief Detects whether or not there is an intersection between the given sphere and edge. Returns the intersection distance factors between e0 (=0) and e1 (=1)<br>
			<br>
			detectSphereEdgeIntersection() requires array pointers to operate on. Make sure all passed array pointers provide at least as many elements as required (2 in general for this version)<br>
			Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
		
			@param[in] center 
			@param[in] radius 
			@param[in] e0 
			@param[in] e1 
			@param[out] distances Intersection distances
			@return True, if there is an intersection, or all intersection points lie inside the sphere, false otherwise 
		*/
		template <typename T0, typename T1, typename T2, typename T3, typename T4>
			static	inline	bool	__fastcall	detectSphereEdgeIntersection(const T0 center[Dimensions], T1 radius, const T2 e0[Dimensions], const T3 e1[Dimensions], T4 distances[2])throw()
			{
				T4 d[Dimensions];
				
				//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Pointers', parameters={e1, e0, d}...
				{
					d[Current] = e1[Current] - e0[Current];
					VecV_Include__<Current+1,Dimensions,GreaterOrEqual<Current+2,Dimensions>::eval>::subtract(e1, e0, d);
				};
				T4 delta[Dimensions];
				
				//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Pointers', parameters={e0, center, delta}...
				{
					delta[Current] = e0[Current] - center[Current];
					VecV_Include__<Current+1,Dimensions,GreaterOrEqual<Current+2,Dimensions>::eval>::subtract(e0, center, delta);
				};
				T4	pa = (ObjV_Include__<0,Dimensions,false>::Operator4_sum(d, d)/*d:i*d:i*/),
						pb = 2*(ObjV_Include__<0,Dimensions,false>::Operator4_sum(d, delta)/*d:i*delta:i*/),
						pc = (ObjV_Include__<0,Dimensions,false>::Operator5_sum(delta)/*delta:i*delta:i*/)-M::sqr(radius),
						rs[2];
				BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
				if (!num_rs)
					return false;
				if (num_rs == 1)
				{
					distances[0] = distances[1] = rs[0];
					return rs[0] >= (T4)0 && rs[0] <= (T4)1;
				}
				distances[0] = std::min(rs[0],rs[1]);
				distances[1] = std::max(rs[0],rs[1]);
				return (distances[0] <= (T4)1 && distances[1] >= (T4)0);
			}

		/**
			@brief Detects whether or not there is an intersection between the given sphere and edge. Returns the intersection distance factors between e0 (=0) and e1 (=1)<br>
			<br>
			DetectSphereEdgeIntersection() requires array pointers to operate on. Make sure all passed array pointers provide at least as many elements as required (2 in general for this version)<br>
			Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
		
			@param[in] center 
			@param[in] radius 
			@param[in] e0 
			@param[in] e1 
			@param[out] distances Intersection distances
			@return True, if there is an intersection, or all intersection points lie inside the sphere, false otherwise 
		*/
		template <typename T0, typename T1, typename T2, typename T3, typename T4>
			static	inline	bool	__fastcall	DetectSphereEdgeIntersection(const T0 center[Dimensions], T1 radius, const T2 e0[Dimensions], const T3 e1[Dimensions], T4 distances[2])throw()
			{
				T4 d[Dimensions];
				
				//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Pointers', parameters={e1, e0, d}...
				{
					d[Current] = e1[Current] - e0[Current];
					VecV_Include__<Current+1,Dimensions,GreaterOrEqual<Current+2,Dimensions>::eval>::subtract(e1, e0, d);
				};
				T4 delta[Dimensions];
				
				//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Pointers', parameters={e0, center, delta}...
				{
					delta[Current] = e0[Current] - center[Current];
					VecV_Include__<Current+1,Dimensions,GreaterOrEqual<Current+2,Dimensions>::eval>::subtract(e0, center, delta);
				};
				T4	pa = (ObjV_Include__<0,Dimensions,false>::Operator4_sum(d, d)/*d:i*d:i*/),
						pb = 2*(ObjV_Include__<0,Dimensions,false>::Operator4_sum(d, delta)/*d:i*delta:i*/),
						pc = (ObjV_Include__<0,Dimensions,false>::Operator5_sum(delta)/*delta:i*delta:i*/)-M::sqr(radius),
						rs[2];
				BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
				if (!num_rs)
					return false;
				if (num_rs == 1)
				{
					distances[0] = distances[1] = rs[0];
					return rs[0] >= (T4)0 && rs[0] <= (T4)1;
				}
				distances[0] = std::min(rs[0],rs[1]);
				distances[1] = std::max(rs[0],rs[1]);
				return (distances[0] <= (T4)1 && distances[1] >= (T4)0);
			}


		/* ----- Now implementing object based recursion-indirect functions ----- */
		//now implementing template definition 'bool SphereContainsPoint (2..4) (<const [*] center>, <radius>, <const [*] p>) direct='
		/**
			@brief <br>
			<br>
			SphereContainsPoint() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
			Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
		
			@param[in] center 
			@param[in] radius 
			@param[in] p 
			@return  
		*/
		template <typename T0, typename T1, typename T2>
			static	inline	bool	__fastcall	SphereContainsPoint(const TVec<T0,Dimensions>& center, T1 radius, const TVec<T2,Dimensions>& p)throw()
			{
				return (ObjV_Include__<0,Dimensions,false>::Operator6_sum(center.v, p.v)/*Math::sqr(center:i - p:i)*/) <= M::sqr(radius);
			}

		//now implementing template definition 'bool detectSphereEdgeIntersection|DetectSphereEdgeIntersection (2..4) (<const [*] center>, <radius>, <const [*] e0>, <const [*] e1>) direct='
		/**
			@brief <br>
			<br>
			detectSphereEdgeIntersection() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
			Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
		
			@param[in] center 
			@param[in] radius 
			@param[in] e0 
			@param[in] e1 
			@return  
		*/
		template <typename T0, typename T1, typename T2, typename T3>
			static	inline	bool	__fastcall	detectSphereEdgeIntersection(const TVec<T0,Dimensions>& center, T1 radius, const TVec<T2,Dimensions>& e0, const TVec<T3,Dimensions>& e1)throw()
			{
				if (((ObjV_Include__<0,Dimensions,false>::Operator6_sum(center.v, e0.v)/*Math::sqr(center:i - e0:i)*/) <= M::sqr(radius)) || ((ObjV_Include__<0,Dimensions,false>::Operator6_sum(center.v, e1.v)/*Math::sqr(center:i - e1:i)*/) <= M::sqr(radius)))
					return true;
				TVec<T0,Dimensions> d;
				
				//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={e1, e0, d}...
				{
					d.v[Current] = e1.v[Current] - e0.v[Current];
					VecV_Include__<Current+1,Dimensions,GreaterOrEqual<Current+2,Dimensions>::eval>::subtract(e1, e0, d);
				};
				TVec<T0,Dimensions> delta;
				
				//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={e0, center, delta}...
				{
					delta.v[Current] = e0.v[Current] - center.v[Current];
					VecV_Include__<Current+1,Dimensions,GreaterOrEqual<Current+2,Dimensions>::eval>::subtract(e0, center, delta);
				};
				T0	pa = (ObjV_Include__<0,Dimensions,false>::Operator7_sum(d.v, d.v)/*d:i*d:i*/),
						pb = 2*(ObjV_Include__<0,Dimensions,false>::Operator7_sum(d.v, delta.v)/*d:i*delta:i*/),
						pc = (ObjV_Include__<0,Dimensions,false>::Operator8_sum(delta.v)/*delta:i*delta:i*/)-M::sqr(radius),
						rs[2];
				BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
				if (!num_rs)
					return false;
				T0	alpha = smallestPositiveResult(rs,num_rs);
				return alpha >= (T0)0 && alpha <= (T0)1;
			}

		/**
			@brief <br>
			<br>
			DetectSphereEdgeIntersection() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
			Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
		
			@param[in] center 
			@param[in] radius 
			@param[in] e0 
			@param[in] e1 
			@return  
		*/
		template <typename T0, typename T1, typename T2, typename T3>
			static	inline	bool	__fastcall	DetectSphereEdgeIntersection(const TVec<T0,Dimensions>& center, T1 radius, const TVec<T2,Dimensions>& e0, const TVec<T3,Dimensions>& e1)throw()
			{
				if (((ObjV_Include__<0,Dimensions,false>::Operator6_sum(center.v, e0.v)/*Math::sqr(center:i - e0:i)*/) <= M::sqr(radius)) || ((ObjV_Include__<0,Dimensions,false>::Operator6_sum(center.v, e1.v)/*Math::sqr(center:i - e1:i)*/) <= M::sqr(radius)))
					return true;
				TVec<T0,Dimensions> d;
				
				//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={e1, e0, d}...
				{
					d.v[Current] = e1.v[Current] - e0.v[Current];
					VecV_Include__<Current+1,Dimensions,GreaterOrEqual<Current+2,Dimensions>::eval>::subtract(e1, e0, d);
				};
				TVec<T0,Dimensions> delta;
				
				//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={e0, center, delta}...
				{
					delta.v[Current] = e0.v[Current] - center.v[Current];
					VecV_Include__<Current+1,Dimensions,GreaterOrEqual<Current+2,Dimensions>::eval>::subtract(e0, center, delta);
				};
				T0	pa = (ObjV_Include__<0,Dimensions,false>::Operator7_sum(d.v, d.v)/*d:i*d:i*/),
						pb = 2*(ObjV_Include__<0,Dimensions,false>::Operator7_sum(d.v, delta.v)/*d:i*delta:i*/),
						pc = (ObjV_Include__<0,Dimensions,false>::Operator8_sum(delta.v)/*delta:i*delta:i*/)-M::sqr(radius),
						rs[2];
				BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
				if (!num_rs)
					return false;
				T0	alpha = smallestPositiveResult(rs,num_rs);
				return alpha >= (T0)0 && alpha <= (T0)1;
			}

		//now implementing template definition 'bool detectSphereEdgeIntersection|DetectSphereEdgeIntersection (2..4) (<const [*] center>, <radius>, <const [*] e0>, <const [*] e1>, <[2] distances>) direct='
		/**
			@brief Detects whether or not there is an intersection between the given sphere and edge. Returns the intersection distance factors between e0 (=0) and e1 (=1)<br>
			<br>
			detectSphereEdgeIntersection() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
			Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
		
			@param[in] center 
			@param[in] radius 
			@param[in] e0 
			@param[in] e1 
			@param[out] distances Intersection distances
			@return True, if there is an intersection, or all intersection points lie inside the sphere, false otherwise 
		*/
		template <typename T0, typename T1, typename T2, typename T3, typename T4>
			static	inline	bool	__fastcall	detectSphereEdgeIntersection(const TVec<T0,Dimensions>& center, T1 radius, const TVec<T2,Dimensions>& e0, const TVec<T3,Dimensions>& e1, TVec2<T4>& distances)throw()
			{
				TVec<T4,Dimensions> d;
				
				//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={e1, e0, d}...
				{
					d.v[Current] = e1.v[Current] - e0.v[Current];
					VecV_Include__<Current+1,Dimensions,GreaterOrEqual<Current+2,Dimensions>::eval>::subtract(e1, e0, d);
				};
				TVec<T4,Dimensions> delta;
				
				//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={e0, center, delta}...
				{
					delta.v[Current] = e0.v[Current] - center.v[Current];
					VecV_Include__<Current+1,Dimensions,GreaterOrEqual<Current+2,Dimensions>::eval>::subtract(e0, center, delta);
				};
				T4	pa = (ObjV_Include__<0,Dimensions,false>::Operator7_sum(d.v, d.v)/*d:i*d:i*/),
						pb = 2*(ObjV_Include__<0,Dimensions,false>::Operator7_sum(d.v, delta.v)/*d:i*delta:i*/),
						pc = (ObjV_Include__<0,Dimensions,false>::Operator8_sum(delta.v)/*delta:i*delta:i*/)-M::sqr(radius),
						rs[2];
				BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
				if (!num_rs)
					return false;
				if (num_rs == 1)
				{
					distances.x = distances.y = rs[0];
					return rs[0] >= (T4)0 && rs[0] <= (T4)1;
				}
				distances.x = std::min(rs[0],rs[1]);
				distances.y = std::max(rs[0],rs[1]);
				return (distances.x <= (T4)1 && distances.y >= (T4)0);
			}

		/**
			@brief Detects whether or not there is an intersection between the given sphere and edge. Returns the intersection distance factors between e0 (=0) and e1 (=1)<br>
			<br>
			DetectSphereEdgeIntersection() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>
			Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>
		
			@param[in] center 
			@param[in] radius 
			@param[in] e0 
			@param[in] e1 
			@param[out] distances Intersection distances
			@return True, if there is an intersection, or all intersection points lie inside the sphere, false otherwise 
		*/
		template <typename T0, typename T1, typename T2, typename T3, typename T4>
			static	inline	bool	__fastcall	DetectSphereEdgeIntersection(const TVec<T0,Dimensions>& center, T1 radius, const TVec<T2,Dimensions>& e0, const TVec<T3,Dimensions>& e1, TVec2<T4>& distances)throw()
			{
				TVec<T4,Dimensions> d;
				
				//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={e1, e0, d}...
				{
					d.v[Current] = e1.v[Current] - e0.v[Current];
					VecV_Include__<Current+1,Dimensions,GreaterOrEqual<Current+2,Dimensions>::eval>::subtract(e1, e0, d);
				};
				TVec<T4,Dimensions> delta;
				
				//block inlining void subtract|sub (2..4) (<const [*] v>, <const [*] w>, <[*] result>) direct=1 for dimensions=3, assembly_mode='Objects', parameters={e0, center, delta}...
				{
					delta.v[Current] = e0.v[Current] - center.v[Current];
					VecV_Include__<Current+1,Dimensions,GreaterOrEqual<Current+2,Dimensions>::eval>::subtract(e0, center, delta);
				};
				T4	pa = (ObjV_Include__<0,Dimensions,false>::Operator7_sum(d.v, d.v)/*d:i*d:i*/),
						pb = 2*(ObjV_Include__<0,Dimensions,false>::Operator7_sum(d.v, delta.v)/*d:i*delta:i*/),
						pc = (ObjV_Include__<0,Dimensions,false>::Operator8_sum(delta.v)/*delta:i*delta:i*/)-M::sqr(radius),
						rs[2];
				BYTE num_rs = Math::solveSqrEquation(pa,pb,pc,rs);
				if (!num_rs)
					return false;
				if (num_rs == 1)
				{
					distances.x = distances.y = rs[0];
					return rs[0] >= (T4)0 && rs[0] <= (T4)1;
				}
				distances.x = std::min(rs[0],rs[1]);
				distances.y = std::max(rs[0],rs[1]);
				return (distances.x <= (T4)1 && distances.y >= (T4)0);
			}

	};

template <count_t Dimensions, count_t First=0>
	class ObjUnroll:public ObjV_Include__<First,Dimensions,GreaterOrEqual<First+1,Dimensions>::eval> 
	{};

#endif
