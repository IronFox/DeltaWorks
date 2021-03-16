#ifndef object_mathH
#define object_mathH

/******************************************************************

Collection of template object-functions.
Also provides template-structures for abstract objects.

******************************************************************/



#include "matrix.h"
#include "../container/queue.h"
#include "object_def.h"
#include "../container/buffer.h"
#include "../general/parallel_for.h"


//tags for ObjMap::map
#define O_VERTICES	1
#define O_EDGES		2
#define O_TRIANGLES	4
#define O_QUADS		8
#define O_FACES		(O_TRIANGLES | O_QUADS)
#define O_ALL		(O_VERTICES | O_EDGES | O_FACES)

#define COMPLEXITY_CONST	(M_E*M_E)
#define GLOBAL_SECTOR_MAP_SIZE	0x100000

#define MAX_RECURSION	400

//#define THROW_BUFFER_ERROR

//efficiency in 1000 lookups per second

//const eff(med)			eff(low)
//1.0	< 130				<150
//1.6	~130				~153
//2.0	145 - 150			~169		<<
//3.0	135 - 138			140 - 145
//3.8	145 - 147			~112
//4.0	~150				~112
//4.2	< 90
//4.5	< 90
//5.0	< 90

namespace DeltaWorks
{

	namespace Math	//! Collection of geometry related mathematical functions and classes
	{

		#include "object_operations.h"


		template <class> class TriangleMesh;
		template <class> class ObjRefList;
		template <class> class ObjMap;
		template <class> class ObjBuffer;
		template <class> class Mesh;
		template <class> class DynamicMesh;
	
		template <class> class AbstractSphere;
		template <class> class AbstractCylinder;
		template <class> class AbstractHull;
	
	
		template <class> struct TObjRef;
		template <class> class MeshVertex;
		template <class> class MeshEdge;
		template <class> class MeshQuad;
		template <class> class MeshTriangle;

		template <class> struct TGraphDef;
		template <class> struct TFaceGraphDef;
		template <class> struct TVertex2;
		template <class> struct TVertex3;
		template <class> struct TVertex3N;

		template <class> class SplineQuad;



		MFUNC4	(void)			_oTriangleNormal(const M::TVec3<C0>&p0, const M::TVec3<C1>&p1, const M::TVec3<C2>&p2, M::TVec3<C3>&result);		//!< Calculates the normal of the specified triangle and stores it in \b out. The result is not normalized \param p0 First triangle vertex, \param p1 Second triangle vertex \param p2 Third triangle vertex \param result [out] Vector for the resulting normal
		MFUNC4	(void)			_oAddTriangleNormal(const M::TVec3<C0>&p0, const M::TVec3<C1>&p1, const M::TVec3<C2>&p2, M::TVec3<C3>&inout);	//!< Calculates the normal of the specified triangle and adds it to the provided \b inout normal vector. The resulting local normal is not normalized before adding. \param p0 First triangle vertex, \param p1 Second triangle vertex \param p2 Third triangle vertex \param inout In/out vector to add the local triangle normal to
		MFUNC	(C)				_oTriangleSize(const C*o, unsigned band);								//!< Calculates the size of the specified 3 dimensional triangle \param o Field of floats defining the triangle \param band Number of floats per vertex \return Size of the specified triangle
		MFUNC3	(C0)			_oTriangleSize(const C0*v1, const C1*v2,const C2*v3);					//!< Calculates the size of the specified 3 dimensional triangle \param p0 First triangle vertex, \param p1 Second triangle vertex \param p2 Third triangle vertex \return Size of the specified triangle
		MFUNC3	(char)			_oTriangleOrientation2(const C0 p0[2], const C1 p1[2], const C2 p2[2]);	//!< Calculates the orientation of the specified 2 dimensional triangle. The respective sign results from the triangle orientation (clockwise = negative, counter clockwise = positive) \param p0 First triangle vertex, \param p1 Second triangle vertex \param p2 Third triangle vertex \return Triangle orientation
		MFUNC3	(C0)			_oSignedTriangleSize2(const C0 p0[2], const C1 p1[2], const C2 p2[2]);	//!< Calculates the signed size of the specified 2 dimensional triangle. The respective sign results from the triangle orientation (clockwise = positive, counter clockwise = negative) \param p0 First triangle vertex, \param p1 Second triangle vertex \param p2 Third triangle vertex \return signed triangle size
		MFUNC2	(C0)			_oZeroTriangleSize2(const C0 p0[2], const C1 p1[2]);					//!< Calculates the signed size of the specified 2 dimensional triangle with the third vertex being the point of origin. The respective sign results from the triangle orientation (clockwise = positive, counter clockwise = negative) \param p0 First triangle vertex, \param p1 Second triangle vertex \return signed triangle size

	#if 0
		MFUNC5	(C0)			_oQuadraticTriangleDistance(const C0 t0[3],const C1 t1[3],const C2 t2[3], const C3 p[3], C4 normal_out[3]);	//!< Calculates the quadratic minimum distance between any point of the described triangle and the specified remote point @a p @param t0 
		MFUNC5	(C0)			_oTriangleDistance(const C0 t0[3],const C1 t1[3],const C2 t2[3], const C3 p[3], C4 normal_out[3]);	//!< Calculates the quadratic minimum distance between any point of the described triangle and the specified remote point @a p @param t0 
		MFUNC4	(C0)			_oQuadraticTriangleDistance(const C0 t0[3],const C1 t1[3],const C2 t2[3], const C3 p[3]);	//!< Calculates the quadratic minimum distance between any point of the described triangle and the specified remote point @a p @param t0 
		MFUNC4	(C0)			_oTriangleDistance(const C0 t0[3],const C1 t1[3],const C2 t2[3], const C3 p[3]);	//!< Calculates the quadratic minimum distance between any point of the described triangle and the specified remote point @a p @param t0 
	#endif

		MFUNC6	(void)			_oTriangleCoord(const C0 v0[3], const C1 v1[3], const C2 v2[3], const C3&x, const C4&y, C5 out[3]);
	
	

		/*!
			\brief Checks if there is a valid intersection of the specified triangle and ray
		
			_oDetectIntersection() calculates the intersection distance (if any) of the specified triangle and ray. If such an intersection occurs then the distance variable will be updated and the result be true.
			A valid intersection occurs if the ray intersects the triangle plane within the triangle's boundaries. The resulting distance value can be positive or negative.
			The function detects front- and backfacing intersections.
		
			\param p0 First triangle vertex
			\param p1 Second triangle vertex
			\param p2 Third triangle vertex
			\param b Ray base position
			\param d Normalized ray direction vector
			\param distance Out distance scalar. This value will be updated if an intersection was detected. @b distance remains unchanged if no intersection was detected.
			\return true if an intersection was detected, false otherwise. The specified distance value remains unchanged if the result is false.
		*/
		MFUNC6	(bool)		_oDetectIntersection(const C0 p0[3], const C1 p1[3], const C2 p2[3], const C3 b[3], const C4 d[3], C5&distance);
	

		/*!
			\brief Checks if there is a valid closer intersection of the specified quad and ray. @b bugged! don't use
		
			_oDetectOpticalQuadIntersection() calculates the intersection distance (if any) of the specified quad (a rhomb) and ray. If such an intersection occurs and it's closer than the specified
			distance then the distance variable will be updated and the result be true.
			The quad is formed by the three specified vertices plus a fourth rhomb coordinate beyond the edge between p1 and p2 - a reflection of p0.
			A valid intersection occurs if the ray intersects the rhomb plane(s) within the rhomb's boundaries and with a positive ray factor.
			An intersection with a negative ray factor - thus behind the ray's base position - will be ignored.
			The function detects front- and backface intersections.
			@b bugged! don't use
		
			\param p0 First triangle vertex
			\param p1 Second triangle vertex
			\param p2 Third triangle vertex
			\param b Ray base position
			\param d Normalized ray direction vector
			\param distance In/out distance scalar. This value will be updated if a closer (positive) intersection was detected.
			\return true if a positive intersection closer than the specified distance was detected, false otherwise. The specified distance value remains unchanged if the result is false.
		*/
		MFUNC6	(bool)		_oDetectOpticalQuadIntersection(const C0 p0[3], const C1 p1[3], const C2 p2[3], const C3 b[3], const C4 d[3], C5&distance);	//the fourth rhomb coordinate lies beyond the edge between vertex 1 and 2, a reflection of vertex 0
	
		/*!
			\brief Checks for a valid closer intersection of the specified sphere and ray
		
			_oDetectOpticalSphereIntersection() calculates the intersection distance (if any) of the specified sphere and ray. If such an intersection occurs and it's closer than the specified
			distance then the distance variable will be updated and the result be true.
			A valid intersection occurs if the ray intersects the sphere and with a positive ray factor. An intersection with a negative ray factor - thus behind the ray's base position - will be ignored.
			Since there are usually two intersection points the smallest positive intersection will be used for the result.
			Thus, if the ray base point is within the sphere, there will always be a valid intersection, although maybe not a closer one.
	
			\param center Sphere center
			\param radius Sphere radius
			\param b Ray base position
			\param d Normalized ray direction vector
			\param distance In/out distance scalar. This value will be updated if a closer (positive) intersection was detected.
			\return true if a positive intersection closer than the specified distance was detected, false otherwise. The specified distance value remains unchanged if the result is false.
		*/
		MFUNC5	(bool)		_oDetectOpticalSphereIntersection(const C0 center[3], const C1&radius, const C2 b[3], const C3 d[3], C4&distance);	// d is expected to be normalized
	
		/*!
			\brief Checks for a valid intersection of the specified sphere and ray
		
			This function behaves similar to the above version except that it doesn't check for any upper distance constraint. It does however still exclude negative intersections
		
			\param center Sphere center
			\param radius Sphere radius
			\param b Ray base position
			\param d Normalized ray direction vector
			\return true if a positive intersection closer than the specified distance was detected, false otherwise. The specified distance value remains unchanged if the result is false.
		*/
		MFUNC4	(bool)		_oDetectOpticalSphereIntersection(const C0 center[3], const C1&radius, const C2 b[3], const C3 d[3]);	// d is expected to be normalized
	
		/*!
			\brief Determines the factors of intersection of a triangle and an edge
		
			_oDetTriangleEdgeIntersection() determines the factors (a, b, c) of the intersection of the specified triangle and edge (if any).
			Unless the edge and triangle are parallel the resulting factors are written to the specified out array: out[0] = a, out[1] = b, out[2] = c.<br />
			The solved intersection equation is: x (point of intersection) = t0 + (t1-t0)*a + (t2-t0)*b = e0 + (e1-e0)*c
		
			\param t0 First triangle vertex
			\param t1 Second triangle vertex
			\param t2 Third triangle vertex
			\param e0 First edge vertex
			\param e1 Second edge vertex
			\param out Out factor field. 
			\return true if a point of intersection could be determined, false otherwise. The out array will remain unchanged if the function returns false.
		*/
		MFUNC6	(bool)		_oDetTriangleEdgeIntersection(const C0 t0[3], const C1 t1[3], const C2 t2[3], const C3 e0[3], const C4 e1[3], C5 out[3]);
	
		/*!
			\brief Determines the factors of intersection of a triangle and a ray
		
			Similar to _oDetTriangleEdgeIntersection(), _oDetTriangleRayIntersection() determines the factors (a, b, c) of the intersection of the specified triangle and ray (if any).
			Unless the ray and triangle are parallel the resulting factors are written to the specified out array: out[0] = a, out[1] = b, out[2] = c.<br />
			The solved intersection equation is: x (point of intersection) = t0 + (t1-t0)*a + (t2-t0)*b = b + f*c
		
			\param t0 First triangle vertex
			\param t1 Second triangle vertex
			\param t2 Third triangle vertex
			\param b Ray base vector
			\param d Ray direction vector (not null vector)
			\param out Out factor field. 
			\return true if a point of intersection could be determined, false otherwise. The out array will remain unchanged if the function returns false.
		*/
		MFUNC6	(bool)		_oDetTriangleRayIntersection(const C0 t0[3], const C1 t1[3], const C2 t2[3], const C3 b[3], const C4 d[3], C5 out[3]);
	
		/*!
			\brief Checks if a determined triangle/edge intersection lies within the boundaries of the respective edge and triangle
		
			_oValidTriangleEdgeIntersection() should be used on the resulting factors of a _oDetTriangleEdgeIntersection() call.
		
			\param factors Resulting factors as returned by _oDetTriangleEdgeIntersection()
			\return true if the specified factors indicate that the intersection was within the edge and triangle range, false otherwise.
		*/
		MFUNC1	(bool)		_oValidTriangleEdgeIntersection(const C0 factors[3]);
	
		/*!
			\brief Expands the specified dimension field using the specified vertex where needed
		
			If any component of the specified vertex is less than the respective lower corner component or greater than the respective upper corner component then it will
			be copied to the respective corner value, effectivly expanding the dimension field.
			The first \a band components of the \a dimensions field are supposed to hold the lower corner of a volume,
			the last \a band components the upper corner of a volume. These corners need to be set up before invoking this function for the desired result.
		
			\param vertex Pointer to a float array providing at least \a band components
			\param dimensions Out dimension field providing at least 2*\a band components
			\param band Number of dimensions
		*/
		MFUNC2	(void)		_oDetDimension(const C0*vertex, C1*dimensions, unsigned band);
	
		/*!
			\brief Expands the specified dimension field using the specified vertex where needed
		
			If any component of the specified vertex is less than the respective lower corner component or greater than the respective upper corner component then it will
			be copied to the respective corner value, effectivly expanding the dimension field.
			The first 3 components of the \a dimensions field are supposed to hold the lower corner of a volume,
			the last 3 components the upper corner of a volume. These corners need to be set up before invoking this function for the desired result.
		
			\param vertex Vertex coordinates
			\param dimensions Out dimension field
			\param band Number of dimensions
		*/
		MFUNC2	(void)		_oDetDimension(const M::TVec3<C0>&vertex, M::Box<C1>&dimensions);
		MFUNC3	(void)		_oDetDimension(const M::TVec3<C0>&vertex, M::TVec3<C1>&lower, M::TVec3<C2>&upper);	//!< Identical to the above for separate lower/upper boundaries
		MFUNC2V (void)		_oDetDimensionV(const C0*vertex, C1*dimensions);			//!< Compile time defined variable version of _oDetDimension()
	
		/*!
			\brief Detects the time of collision (if any) of two moving circles in R2
		
			_oDetCircleCollision() assumes that the two circles are not already intersecting at their respective initial positions. If a collision occurs within
			the described frame then the time of first touch will be written to \a t.
		
			\param center0 Center of the first circle at the beginning of the frame
			\param delta0 Movement vector of the first circle
			\param r0 Radius of the first circle
			\param center1 Center of the second circle at the beginning of the frame
			\param delta1 Movement vector of the second circle
			\param r1 Radius of the second circle
			\param t Out scalar containing the time of collision (if any) ranging from 0(frame begin) to 1(frame end)
			\return true if a collision was detected, false otherwise.
		*/
		MFUNC7	(bool)		_oDetCircleCollision(const C0 center0[2], const C1 delta0[2], const C2&r0, const C3 center1[2], const C4 delta1[2], const C5&r1, C6&t);

		/*!
			\brief Detects the time of collision (if any) of two moving spheres in R3
		
			_oDetSphereCollision() assumes that the two spheres are not already intersecting at their respective initial positions. If a collision occurs within
			the described frame then the time of first touch will be written to \a t.
		
			\param center0 Center of the first sphere at the beginning of the frame
			\param delta0 Movement vector of the first sphere
			\param r0 Radius of the first sphere
			\param center1 Center of the second sphere at the beginning of the frame
			\param delta1 Movement vector of the second sphere
			\param r1 Radius of the second sphere
			\param t Out scalar containing the time of collision (if any) ranging from 0(frame begin) to 1(frame end)
			\return true if a collision was detected, false otherwise.
		*/
		MFUNC7	(bool)		_oDetSphereCollision(const C0 center0[3], const C1 delta0[3], const C2&r0, const C3 center1[3], const C4 delta1[3], const C5&r1, C6&t);
	
		/*!
			\brief Detects the time of collision (if any) of a stationary and a moving edge in R3
	
			_oDetEdgeIntersection() assumes that one edge is stationary while the other moves. This scenario occurs when detecting collisions between two moving objects
			with the geometrical data of one object being imported into the space of the other.
		
			\param local0 First vertex of the stationary edge
			\param local1 Second vertex of the stationary edge
			\param prev0 First vertex of the moving edge at the beginning of the frame
			\param prev1 Second vertex of the moving edge at the beginning of the frame
			\param next0 First vertex of the moving edge at the end of the frame
			\param next1 Second vertex of the moving edge at the end of the frame
			\param out_time Out time of collision (if any) ranging from 0(frame begin) to 1(frame end)
			\param out_local_intersection Out scalar describing the point of intersection along the stationary edge.
			\param out_other_intersection Out scalar describing the point of intersection along the moving edge.
			\return true if a collision was detected, false otherwise
		*/
		MFUNC9	(bool)		_oDetEdgeIntersection(const C0*local0, const C1*local1, const C2*prev0, const C3*prev1, const C4*next0, const C5*next1, C6&out_time, C7&out_local_intersection, C8&out_other_intersection);
	
		/*!
			\brief Detects the time of collision (if any) of a stationary triangle and a moving vertex (forming an edge) in R3
		
			_oDetFaceEdgeIntersection() assumes that the triangle is stationary while the vertex moves along an edge describing the movement frame. This scenario occurs when detecting collisions between two moving objects
			with the geometrical data of one object being imported into the space of the other.
	
			\param local0 First vertex of the stationary triangle
			\param local1 Second vertex of the stationary triangle
			\param local2 Third vertex of the stationary triangle
			\param prev Position of the vertex at the beginning of the frame
			\param next Position of the vertex at the end of the frame
			\param out_time Time of collision (if any) ranging from 0(frame begin) to 1(frame end)
			\param out_position Collision point (if any) along the edge on the surface of the triangle
			\return true if a collision was detected, false otherwise.
		*/
		MFUNC7	(bool)		_oDetFaceEdgeIntersection(const C0*local0, const C1*local1, const C2*local2, const C3*prev, const C4*next, C5&out_time, C6*out_position);
	
		/*!
			\brief Detects the time of collision (if any) of a moving triangle (forming a volume) and a stationary vertex in R3
		
			_oDetVolumeVertexIntersection() assumes that the vertex is stationary while the triangle moves along three edges decribing the movement frame.
			This scenario occurs when detecting collisions between two moving objects with the geometrical data of one object being imported into the space of the other.<br />
			The collision point can be calculated by first interpolating the triangle vertex positions to v0t, v1t, v2t and then interpolating these vertices:<br />
			point of collision = v0t + (v1t-v0t)*x + (v2t-v0t)*y
		
			\param p0 Position of the stationary vertex
			\param v0t0 Position of the first triangule vertex at the beginning of the frame
			\param v1t0 Position of the second triangule vertex at the beginning of the frame
			\param v2t0 Position of the third triangule vertex at the beginning of the frame
			\param v0t1 Position of the first triangule vertex at the end of the frame
			\param v1t1 Position of the second triangule vertex at the end of the frame
			\param v2t1 Position of the third triangule vertex at the end of the frame
			\param time Out time of collision (if any) ranging from 0(frame begin) to 1(frame end)
			\param x X factor along the v0/v1 axis of the point of collision (if any)
			\param y Y factor along the v0/v2 axis of the point of collision (if any)
			\return true if a collision was detected, false otherwise.
		*/
		MFUNC10	(bool)		_oDetVolumeVertexIntersection(const C0 p[3], const C1 v0t0[3], const C2 v1t0[3], const C3 v2t0[3], const C4 v0t1[3], const C5 v1t1[3], const C6 v2t1[3], C7&time, C8&x, C9&y);
	
		/*!
			\brief Determines the point of intersection of a plane and a ray
		
			\param base Plane base point
			\param normal Normalized plane normal
			\param vbase Ray base point
			\param vdir Normalized ray direction vector
			\param out Point of intersection (if any)
			\return Ray factor. If the specified ray and plane are parallel then the resulting factor will be very large but not NAN.
		*/
		MFUNC5	(C4)			_oIntersectPoint(const C0 base[3], const C1 normal[3], const C2 vbase[3], const C3 vdir[3], C4 out[3]);
	
		namespace Obj
		{
			template <typename F, typename I>
				bool	Triangulate(const ArrayRef<TVec2<F> >&vertices, BasicBuffer<I>&outIndices);
			template <typename F, typename I>
				bool	TriangulateF(const count_t numVertices, const std::function<TVec2<F>(index_t)>&getVertex, BasicBuffer<I>&outIndices);


			/*!
			@brief Checks for a valid closer intersection of the specified box and ray
		
			Checks for intersections with each box side.
			The box is considered solid. If the specified base point is found to be located within the box, the result is true and distance set to 0 if the given distance is not already 0.
			
		
			@param box Box declaration. Must be valid
			@param b Ray base position
			@param d Normalized ray direction vector
			@param[inout] distance In/out distance scalar. This value will be updated if a closer (positive) intersection was detected.
			@return true if a positive intersection closer than the specified distance was detected, false otherwise. The specified distance value remains unchanged if the result is false.
			*/
			MFUNC4	(bool)		DetectOpticalBoxIntersection(const M::Box<C0>&box, const M::TVec3<C1>&b, const M::TVec3<C2>&d, C3&distance);


			/*!
			@brief Checks for a valid closer intersection of the specified sphere and ray
		
			The sphere is considered solid. If the specified base point is found to be located within the sphere, the result is true and distance set to 0 if the given distance is not already 0.
		
		
			@param sphere Sphere declaration. Must be valid
			@param b Ray base position
			@param d Normalized ray direction vector
			@param[inout] distance In/out distance scalar. This value will be updated if a closer (positive) intersection was detected.
			@return true if a positive intersection closer than the specified distance was detected, false otherwise. The specified distance value remains unchanged if the result is false.
			*/
			MFUNC4	(bool)		DetectOpticalSphereIntersection(const M::Sphere<C0>&box, const M::TVec3<C1>&b, const M::TVec3<C2>&d, C3&distance);

			MFUNC3	(bool)		DetectSphereEdgeIntersection(const M::Sphere<C0>&sphere, const M::TVec3<C1>&e0, const M::TVec3<C2>&e1);
		}



		/*!
			\brief Checks if the specified edge and box intersect
	
			\param p0 First edge vertex
			\param p1 Second edge vertex
			\param box Volume field with the first three components describing the lower, the last three components the upper boundaries of the box.
			\return true if an intersection was detected, false otherwise.
		*/
		MFUNC3	(bool)		_oIntersectsBox(const M::TVec3<C0>&p0, const M::TVec3<C1>&p1, const M::Box<C2>&box);
	
		/*!
			\brief Projects a point onto a plane
		
			\param base Base vector of the plane
			\param normal Normal of the plane (may be of any length greater 0)
			\param p Point to project
			\param out Out projected point
		*/
		MFUNC4	(void)		_oProject(const C0 base[3], const C1 normal[3], const C2 p[3], C3 out[3]);
	
		/*!
			\brief Calculates the projection factor
		
			Using the resulting factor, the following would project the point: (projected) = \b p + \b normal * [result]
		
			\param base Base vector of the plane
			\param normal Normal of the plane (may be of any length greater 0)
			\param p Point to project
			\return Projection factor
		*/
		MFUNC3	(C0)			_oProjectionFactor(const C0 base[3], const C1 normal[3], const C2 p[3]);
	
		/*!
			\brief Determines a triangulation of a 2 dimensional polygon
		
			The specified polygon may have any orientation but no two polygon sides may intersect or the function fails.
			The given target object will contain one vertex for each specified vertex with the respective x and y coordinates. z will be set to 0.
			All \b index member variables will be set accordingly, \b marked will be set false.
		
			\param vertex Vertex float field. Each vertex is required to provide two components (x and y) making the entire field at least 2*vertices floats long
			\param vertices Number of vertices of the polygon
			\param target Out object that should contain the finished triangulation
			\return true if the specified polygon could be completely triangulated, false otherwise.
		*/
		MFUNC2	(bool)		_oTriangulate(const M::TVec2<C0>*vertex, count_t vertices, Mesh<C1>&target);//two entries per vertex
	
		template <typename ContainerT, typename Index>
			MF_DECLARE	(bool)	_oTriangulate3(const ContainerT&vertex_field, Container::BasicBuffer<Index>&target);

		/*!
			\overload
			\param vertex_field Vertex float field. Each vertex is required to provide two components (x and y). Thus the number of polygon vertices is vertex_field.GetLength()/2
		*/
		MFUNC2	(bool)		_oTriangulate(const Ctr::ArrayData<M::TVec2<C0> >&vertex_field, Mesh<C1>&target);//two entries per vertex

		/*!
			\brief Checks if the two specified edges intersect in R3 (given a certain tollerance)
		
			_oDetEdgeIntersection() checks if the two given edges intersect in one point and returns the edge factor of both edges if so.
			To determine the final point of intersection calculate:<br />
			p := other0 + (other1-other0)*out_other_intersection<br />
			or<br />
			p := local0 + (local1-local0)*out_local_intersection
		
			\param local0 First vertex of the first edge
			\param local1 Second vertex of the first edge
			\param other0 First vertex of the second edge
			\param other1 Second vertex of the second edge
			\param out_local_intersection Out edge factor of the intersection point along the first edge.
			\param out_other_intersection Out edge factor of the intersection point along the second edge
			\return true if the two edges intersect, false otherwise
		*/
		MFUNC6	(bool)		_oDetEdgeIntersection(const C0 local0[3], const C1 local1[3], const C2 other0[3], const C3 other1[3], C4&out_local_intersection, C5&out_other_intersection);
	
		/*!
			\brief Checks if the two specified edges intersect in R2

			To determine the final point of intersection calculate:<br />
			p := other0 + (other1-other0)*out_other_intersection<br />
			or<br />
			p := local0 + (local1-local0)*out_local_intersection
		
			\param local0 First vertex of the first edge
			\param local1 Second vertex of the first edge
			\param other0 First vertex of the second edge
			\param other1 Second vertex of the second edge
			\param out_local_intersection Out edge factor of the intersection point along the first edge.
			\param out_other_intersection Out edge factor of the intersection point along the second edge
			\return true if the two edges intersect, false otherwise
		*/
		MFUNC6	(bool)		_oDetEdgeIntersection2(const C0 local0[2], const C1 local1[2], const C2 other0[2], const C3 other1[2], C4&out_local_intersection, C5&out_other_intersection);

		/*!
			\brief Creates a TGraphDef object from a triangle index path
		
			The function creates a graph object from the specified triangle collection. The resulting graph object provides all required elements (vertices, triangles, quads, edges) and the respective links between these.
			However since no vertex information is specified vertex positions are undefined.
		
			\param mesh Target mesh
			\param triangle_index_field Ctr::Array containing the triangle vertex indices. Each three indices form one triangle.
		*/
		MFUNC2	(void)		_oMakeTriangleGraph(Mesh<TGraphDef<C0> >&mesh, const Ctr::ArrayData<C1>&triangle_index_field);
	
		/*!
			\brief Enhanced version of _oMakeTriangleGraph(). Creates a TFaceGraphDef object from a triangle and a quad index path
		
			The function creates a graph object from the specified index sections.  The resulting graph object provides all required elements (vertices, triangles, quads, edges) and the respective links between these.
			However since no vertex information is specified vertex positions are undefined.
		
			\param mesh Target mesh
			\param triangle_index_field Ctr::Array containing the triangle vertex indices. Each three indices form one triangle.
			\param quad_index_field Ctr::Array containing the quad vertex indices. Each four indices form one quad.
		*/
		MFUNC2	(void)		_oMakeGraph(Mesh<TFaceGraphDef<C0> >&mesh, const Ctr::ArrayData<C1>&triangle_index_field, const Ctr::ArrayData<C1>&quad_index_field);
	
		/*!
			\brief Dynamic enhanced version of _oMakeTriangleGraph(). Creates a dynamic TFaceGraphDef mesh from a triangle and a quad index path
		
			The function creates a dynamic graph object from the specified index sections.  The resulting graph object provides all required elements (vertices, triangles, quads, edges) and the respective links between these.
			However since no vertex information is specified vertex positions are undefined.
		
			\param mesh Target dynamic mesh
			\param triangle_index_field Ctr::Array containing the triangle vertex indices. Each three indices form one triangle.
			\param quad_index_field Ctr::Array containing the quad vertex indices. Each four indices form one quad.
		*/
		MFUNC2	(void)		_oMakeGraph(DynamicMesh<TFaceGraphDef<C0> >&mesh, const Ctr::ArrayData<C1>&triangle_index_field, const Ctr::ArrayData<C1>&quad_index_field);
	
		/*!
			\brief Creates a TFaceGraphDef mesh from a general index path containing triangles, triangle strips, quads, and quad strips
		
			The function creates a graph mesh from the specified index sections.  The resulting graph mesh provides all required elements (vertices, triangles, quads, edges) and the respective links between these.
			However since no vertex information is specified vertex positions are undefined.
		
			\param mesh Target mesh
			\param index_field General index array. The type of primitive formed by what indices is defined by the respective tsegment_field and qsegment_field arrays
			\param tsegment_field Triangle index array. The first element of the array specifies the number of indices in raw triangles with each three indices forming one triangle. Subsequent entries describe each the number of indices of one triangle strip (following any preceeding triangles/triangle strips)
			\param qsegment_field Quad index array. The first element of the array specifies the number of indices in raw quads (following any preceding triangles/triangle strips). with each four indices forming one quad. Subsequent entries describe each the number of indices of one quad strip (following any preceeding triangles/triangle strips/quads/quad strips)
		*/
		MFUNC2	(bool)		_oMakeGraph(Mesh<TFaceGraphDef<C0> >&mesh, const Ctr::ArrayData<C1>&index_field, const Ctr::ArrayData<C1>&tsegment_field, const Ctr::ArrayData<C1>&qsegment_field);

		/*!
			\brief Creates a dynamic TFaceGraphDef mesh from a general index path containing triangles, triangle strips, quads, and quad strips
		
			The function creates a dynamic graph mesh from the specified index sections.  The resulting graph mesh provides all required elements (vertices, triangles, quads, edges) and the respective links between these.
			However since no vertex information is specified vertex positions are undefined.
		
			\param mesh Target dynamic mesh
			\param index_field General index array. The type of primitive formed by what indices is defined by the respective tsegment_field and qsegment_field arrays
			\param tsegment_field Triangle index array. The first element of the array specifies the number of indices in raw triangles with each three indices forming one triangle. Subsequent entries describe each the number of indices of one triangle strip (following any preceeding triangles/triangle strips)
			\param qsegment_field Quad index array. The first element of the array specifies the number of indices in raw quads (following any preceeding triangles/triangle strips). with each four indices forming one quad. Subsequent entries describe each the number of indices of one quad strip (following any preceeding triangles/triangle strips/quads/quad strips)
		*/
		MFUNC2	(bool)		_oMakeGraph(DynamicMesh<TFaceGraphDef<C0> >&mesh, const Ctr::ArrayData<C1>&index_field, const Ctr::ArrayData<C1>&tsegment_field, const Ctr::ArrayData<C1>&qsegment_field);
	
	
		MFUNC	(void)		_oUnlink(MeshEdge<TGraphDef<C> >*edge);				//!< Unlinks the specified edge from its vertices (if any)
		MFUNC	(void)		_oLinkToVertices(MeshEdge<TGraphDef<C> >*edge);		//!< Links the specified edge to its vertices (if any)
		MFUNC	(void)		_oUnlinkFromVertices(MeshTriangle<TFaceGraphDef<C> >*triangle, MeshVertex<TFaceGraphDef<C> >*except);	//!< Unlinks the specified triangle from all its vertices except the specified \b except vertex
		MFUNC	(void)		_oUnlinkFromVertices(MeshQuad<TFaceGraphDef<C> >*quad, MeshVertex<TFaceGraphDef<C> >*except);			//!< Unlinks the specified quad from all its vertices except the specified \b except vertex
	
 	
		MFUNC	(count_t)		_oCountUnmarked(const Ctr::BasicBuffer<C>&list);		//!< Counts the number of elements whoes 'marked' member is false


		MFUNC	(TriangleMesh<C>*)	_oSphere(const C&radius, count_t iterations);	//!< Generates a spherical triangle mesh \param radius Sphere radius \param iterations Surface resolution (5 = very low, 50 = very high)
		MFUNC2	(void)				_oSphere(TriangleMesh<C0>&obj,const C1&radius, count_t iterations);		//!< Generates a spherical triangle mesh \param obj Target triangle mesh \param radius Sphere radius \param iterations Surface resolution (5 = very low, 50 = very high)





		template <class Float> struct TVertex2	//! 2 dimensional vertex
		{
				M::TVec2<Float>					p;
		};
	
		template <class Float> struct TVertex3	//! 3 dimensional vertex
		{
				M::TVec3<Float>					p;
		};
	
		template <class Float> struct TVertex3N:public TVertex3<Float>	//! 3 dimensional vertex providing a normal
		{
				M::TVec3<Float>					n;
				bool							added;
		};

		template <class Def>
			class TMeshFaceLink	//! General link to a face (quad or triangle)
			{
			public:
					union
					{
						MeshQuad<Def>					*quad;		//!< Pointer to the respective quad target. Use if \b is_quad is true
						MeshTriangle<Def>				*triangle;	//!< Pointer to the respective triangle target. Use if \b is_quad is false
					};
					bool								is_quad;	//!< Specifies whether or not the target face is a quad. If so use the respective \b quad pointer, else the \b triangle pointer
			
			
					MF_DECLARE(char)					GetIndexOf(const MeshVertex<Def>*)	const;		//!< Determines the index of a vertex as seen by the target face \return vertex index or -1 if the vertex is not part of the target face or no target face is linked
					MF_DECLARE(char)					GetIndexOf(const MeshEdge<Def>*)	const;		//!< Compiles only if Def is some variation of TGraphDef. Determines the index of an edge as seen by the target face \return edge index or -1 if the edge is not part of the target face or no target face is linked
					MF_DECLARE(BYTE)					requireIndexOf(const MeshVertex<Def>*)	const;	//!< Determines the index of a vertex as seen by the target face. Throws an exception if the specified vertex is not part of the target face or the target face is not linked
					MF_DECLARE(BYTE)					requireIndexOf(const MeshEdge<Def>*)	const;	//!< Compiles only if Def is some variation of TGraphDef. Determines the index of an edge as seen by the target face. Throws an exception if the specified vertex is not part of the target face or the target face is not linked
		
					MFUNC2 (void)						replaceNeighbor(C0*face,C1*with);				//!< Compiles only if Def is some variation of either TGraphDef or TFaceGraphDef. Replaces a neighbor face link of the target face (if any)
					MFUNC1 (void)						replaceNeighbor(C0*face,const TMeshFaceLink<Def>&with);	//!< Compiles only if Def is some variation of either TGraphDef or TFaceGraphDef. Replaces a neighbor face link of the target face (if any)
					MFUNC1 (void)						unsetNeighbor(C0*face);									//!< Compiles only if Def is some variation of either TGraphDef or TFaceGraphDef. Unsets the neighbor link to the specified face as seen from the target face (if any)
					MF_DECLARE(const TMeshFaceLink<Def>&)neighbor(BYTE index)	const;							//!< Compiles only if Def is some variation of either TGraphDef or TFaceGraphDef. Retrieves the specified neighboring face of the target face. \param index Index of the neighbor to retrieve with 0 being the neighbor between the first and second face vertex.
		
					MF_DECLARE(const TMeshFaceLink<Def>&)getNextAround(const MeshVertex<Def>*vertex)	const;		//TFaceGraphDef
					MF_DECLARE(const TMeshFaceLink<Def>&)getNextAround(BYTE index)					const;		//TFaceGraphDef
			
					MF_DECLARE(bool)					marked()	const;										//!< Queries whether ot not the target face is marked
					MF_DECLARE(void)					mark();													//!< Marks the target face
			
					MF_DECLARE(String)					ToString(bool full=true)	const;						//!< Generates a string representation of the target face (if any)
		
					MFUNC1 (void)						setNeighbor(MeshVertex<Def>*v,C0*face);					//TGraphDef and TFaceGraphDef
					MF_DECLARE(void)					Set(MeshQuad<Def>*q);									//!< Sets the local face target to the specified quad
					MF_DECLARE(void)					Set(MeshTriangle<Def>*t);								//!< Sets the local face target to the specified triangle
					MF_DECLARE(void)					Unset();												//!< Unsets the local face target (quad/triangle are set to NULL)
					MF_DECLARE(MF_NO_TYPE)				operator bool() const {return triangle != NULL;}		//!< Implicit bool cast \return true if the local face target is set (not NULL), false otherwise
			};



		template <class Def>
			struct TGraphDefVertex:public Def::Vertex	//! Additional vertex information (msvc++ apparently crashes with internal errors when processing methods of classes nested in template classes)
			{
			private:
				MF_DECLARE (bool)							walkLeft(TMeshFaceLink<TGraphDef<Def> > face, Ctr::ArrayData<TMeshFaceLink<TGraphDef<Def> > >&fbuffer, count_t&fcnt);
			public:
				MeshEdge<TGraphDef<Def> >			*first,		//!< Pointer to the first connected edge or NULL if no edge is connected
													*last;		//!< Pointer to the last connected edge or NULL if no edge is connected
				count_t								degree;		//!< Number of edges connected to this vertex
			
				MF_CONSTRUCTOR						TGraphDefVertex();
				MF_DECLARE (void)						Insert(MeshEdge<TGraphDef<Def> >*edge);	//!< Inserts an edge into the local edge list
				MF_DECLARE (bool)						unlink(MeshEdge<TGraphDef<Def> >*edge);	//!< Removes an edge from the local edge list
				MF_DECLARE (MeshEdge<TGraphDef<Def> >*)	findEdgeTo(MeshVertex<TGraphDef<Def> >*vertex)	const;	//!< Attempts to find an edge leading from the local vertex to the specified target vertex \param vertex Target vertex \return Pointer to an edge connecting the local vertex and the target vertex or NULL if no such could be found.
			};

		template <class Def>
			struct TGraphDefTriangle:public Def::Triangle	//! Additional triangle information
			{
				union
				{
					struct
					{
						TMeshFaceLink<TGraphDef<Def> >	n[3];			//!< Ctr::Array access link to all neighbors
						MeshEdge<TGraphDef<Def> >		*edge[3];		//!< Ctr::Array access link to all edges
					};
					struct
					{
						TMeshFaceLink<TGraphDef<Def> >	n0,		//!< Link to the first neighboring face (across the edge between the first and second triangle vertex)
														n1,		//!< Link to the second neighboring face (across the edge between the second and third triangle vertex)
														n2;		//!< Link to the third neighboring face (across the edge between the third and first vertex)
						MeshEdge<TGraphDef<Def> >		*e0,	//!< Link to the first bordering edge (between the first and second triangle vertex)
														*e1,	//!< Link to the second bordering edge (between the second and third triangle vertex)
														*e2;	//!< Link to the third bordering edge (between the third and first triangle vertex)
					};
				};
				unsigned						attrib;			//!< Custom triangle attribute
			
				MF_DECLARE	(char)					GetIndexOf(const MeshQuad<TGraphDef<Def> >*q)		const;	//!< Determines the index of the specified neighboring quad \return requested index or -1 if the specified quad is no neighbor of the local triangle
				MF_DECLARE	(char)					GetIndexOf(const MeshTriangle<TGraphDef<Def> >*t)	const;	//!< Determines the index of the specified neighboring triangle \return requested index or -1 if the specified triangle is no neighbor of the local triangle
				MF_DECLARE	(char)					GetIndexOf(const MeshEdge<TGraphDef<Def> >*e)		const;	//!< Determines the index of the specified edge \return requested index or -1 if the specified edge does not border the local triangle
				MF_DECLARE	(BYTE)					requireIndexOf(const MeshQuad<TGraphDef<Def> >*q)		const;	//!< Identical to GetIndexOf() except that requireIndexOf() throws an exception if -1 would have been returned
				MF_DECLARE	(BYTE)					requireIndexOf(const MeshTriangle<TGraphDef<Def> >*t)	const;	//!< Identical to GetIndexOf() except that requireIndexOf() throws an exception if -1 would have been returned
				MF_DECLARE	(BYTE)					requireIndexOf(const MeshEdge<TGraphDef<Def> >*e)		const;	//!< Identical to GetIndexOf() except that requireIndexOf() throws an exception if -1 would have been returned
			
				MF_DECLARE	(char)					FindAndReplace(MeshQuad<TGraphDef<Def> >*neighbor, MeshQuad<TGraphDef<Def> >*with);
				MF_DECLARE	(char)					FindAndReplace(MeshQuad<TGraphDef<Def> >*neighbor, MeshTriangle<TGraphDef<Def> >*with);
				MF_DECLARE	(char)					FindAndReplace(MeshTriangle<TGraphDef<Def> >*neighbor, MeshTriangle<TGraphDef<Def> >*with);
				MF_DECLARE	(char)					FindAndReplace(MeshTriangle<TGraphDef<Def> >*neighbor, MeshQuad<TGraphDef<Def> >*with);
				MF_DECLARE	(char)					FindAndReplace(MeshEdge<TGraphDef<Def> >*e, MeshEdge<TGraphDef<Def> >*with);			
			
				MF_DECLARE	(String)				ToString(bool full=true)	const;		//!< Generates a string representation of the local triangle
			};

		template <class Def>
			struct TGraphDefQuad:public Def::Quad	//!< Additional quad information
			{
				union
				{
					struct
					{
						TMeshFaceLink<TGraphDef<Def> >	n[4];			//!< Ctr::Array access link to all neighbors
						MeshEdge<TGraphDef<Def> >		*edge[4];		//!< Ctr::Array access link to all edges
					};
					struct
					{
						TMeshFaceLink<TGraphDef<Def> >	n0,		//!< Link to the first neighboring face (across the edge between the first and second quad vertex)
														n1,		//!< Link to the second neighboring face (across the edge between the second and third quad vertex)
														n2,		//!< Link to the third neighboring face (across the edge between the third and fourth quad vertex)
														n3;		//!< Link to the fourth neighboring face (across the edge between the fourth and first quad vertex)
						MeshEdge<TGraphDef<Def> >		*e0,		//!< Link to the first bordering edge (between the first and second quad vertex)
														*e1,		//!< Link to the second bordering edge (between the second and third quad vertex)
														*e2,		//!< Link to the third bordering edge (between the third and fourth quad vertex)
														*e3;		//!< Link to the fourth bordering edge (between the fourth and first quad vertex)
					};
				};

				unsigned						attrib;
			
				MF_DECLARE	(char)					GetIndexOf(const MeshQuad<TGraphDef<Def> >*q)				const;	//!< Determines the index of the specified neighboring quad \return requested index or -1 if the specified quad is no neighbor of the local quad
				MF_DECLARE	(char)					GetIndexOf(const MeshTriangle<TGraphDef<Def> >*t)			const;	//!< Determines the index of the specified neighboring triangle \return requested index or -1 if the specified triangle is no neighbor of the local quad
				MF_DECLARE	(char)					GetIndexOf(const MeshEdge<TGraphDef<Def> >*e)				const;	//!< Determines the index of the specified edge \return requested index or -1 if the specified edge does not border the local quad
				MF_DECLARE	(BYTE)					requireIndexOf(const MeshQuad<TGraphDef<Def> >*q)		const;	//!< Identical to GetIndexOf() except that requireIndexOf() throws an exception if -1 would have been returned
				MF_DECLARE	(BYTE)					requireIndexOf(const MeshTriangle<TGraphDef<Def> >*t)	const;	//!< Identical to GetIndexOf() except that requireIndexOf() throws an exception if -1 would have been returned
				MF_DECLARE	(BYTE)					requireIndexOf(const MeshEdge<TGraphDef<Def> >*e)		const;	//!< Identical to GetIndexOf() except that requireIndexOf() throws an exception if -1 would have been returned
			
				MF_DECLARE	(char)					FindAndReplace(MeshQuad<TGraphDef<Def> >*v, MeshQuad<TGraphDef<Def> >*with);
				MF_DECLARE	(char)					FindAndReplace(MeshQuad<TGraphDef<Def> >*v, MeshTriangle<TGraphDef<Def> >*with);
				MF_DECLARE	(char)					FindAndReplace(MeshTriangle<TGraphDef<Def> >*v, MeshTriangle<TGraphDef<Def> >*with);
				MF_DECLARE	(char)					FindAndReplace(MeshTriangle<TGraphDef<Def> >*v, MeshQuad<TGraphDef<Def> >*with);
				MF_DECLARE	(char)					FindAndReplace(MeshEdge<TGraphDef<Def> >*e, MeshEdge<TGraphDef<Def> >*with);
			
				MF_DECLARE	(String)				ToString(bool full=true)	const;		//!< Generates a string representation of the local quad
			};

		/*!
			\brief Graph definition structure for Mesh instances
		
			A TGraphDef provides the following additional links: Vertex to all connected edges, Triangle/Quad to all its edges/neighboring faces.
			Additionally TGraphDef instances require a sub definition structure.
		*/
		template <class Def> struct TGraphDef
		{
			typedef typename Def::Type				Type;	//!< Base float type as derived from the respective sub definition structure
		
			typedef TGraphDefVertex<Def>	Vertex;
		
			struct Edge:public Def::Edge	//! Additional edge information
			{
				MeshEdge<TGraphDef<Def> >			*next[2];	//!< Pointers to the respective next edges in the list. The first entry points to the next edge of the first vertex, the second to the next edge of the second edge vertex.
			};
		
			typedef TGraphDefTriangle<Def>	Triangle;
			typedef TGraphDefQuad<Def>		Quad;

		
			struct Mesh:public Def::Mesh	//! Additonal mesh information
			{
				struct
				{
					index_t						first,	//!< First used vertex index
												last;	//!< Last used vertex index
				}								vregion;
			};
		};

		template <class Def>
			struct TFaceGraphDefVertex:public Def::Vertex		//! Additional vertex information
			{
			private:
				MF_DECLARE (bool)							walkLeft(TMeshFaceLink<TFaceGraphDef<Def> > face, Ctr::ArrayData<TMeshFaceLink<TFaceGraphDef<Def> > >&fbuffer, count_t&fcnt);
			public:
				typedef TMeshFaceLink<TFaceGraphDef<Def> >FaceLink;		//!< Shortcut

				FaceLink							first,	//!< Link to the first connected face
													last;	//!< Link to the last connected face
				count_t								degree;	//!< Number of connected faces
				typename Def::Type					weight;	//!< Vertex weight (for detail reduction)
			
				MF_CONSTRUCTOR						TFaceGraphDefVertex();
				MF_DECLARE (void)						Insert(MeshQuad<TFaceGraphDef<Def> >*quad);			//!< Inserts the specified quad into the local list of connected faces
				MF_DECLARE (void)						Insert(MeshTriangle<TFaceGraphDef<Def> >*triangle);	//!< Inserts the specified triangle into the local list of connected faces
				MF_DECLARE (bool)						unlink(MeshQuad<TFaceGraphDef<Def> >*quad);			//!< Removes the specified quad from the local list of connected faces
				MF_DECLARE (bool)						unlink(MeshTriangle<TFaceGraphDef<Def> >*triangle);	//!< Removes the specified triangle from the local list of connected faces
			
				/*!
					\brief Closes the mesh over the local vertex
				
					bypass() unlinks the local vertex, closing the mesh over the local vertex. The effective triangle count around this vertex is reduced by one for each non-closed environment segment
					and two if the environment is closed.
					Existing faces are reused if possible. Faces that cannot be reused are marked. All marked faces are unlinked and can be discarded after this operation has finished.
					Occassionally new triangles need to be allocated from the passed dynamic mesh.
					Once finished the local link map will be empty, leaving the vertex entirely free.
				
					\param mesh Dynamic mesh to allocate new triangles from
					\return Number of triangles that were dropped during this operation. Both marked triangles as well as converted quads count each as 1.
				*/
				MF_DECLARE (count_t)					bypass(DynamicMesh<TFaceGraphDef<Def> >&mesh);
				MF_DECLARE (FaceLink)					findFace(const MeshVertex<TFaceGraphDef<Def> >*v)		const;		//!< Returns the first face in the local list of connected faces that also links to the specified vertex or an unset face link if no such exists.
				MF_DECLARE (FaceLink)					findFace(const MeshVertex<TFaceGraphDef<Def> >*v, BYTE index)		const;	//!< Returns the first face in the local list of connected faces that also links to the specified vertex from the specified index or an unset face link if no such exists.
			};

		template <class Def>
			struct TFaceGraphDefTriangle:public Def::Triangle	//!< Additional triangle information
			{
				union
				{
					struct
					{
						TMeshFaceLink<TFaceGraphDef<Def> >	n[3],				//!< Ctr::Array access link to all neighbors
															next[3];			//!< Ctr::Array access link to the respective next faces in the vertex face lists
					};
					struct
					{
						TMeshFaceLink<TFaceGraphDef<Def> >	n0,					//!< Link to the first neighboring face (across the edge between the first and second triangle vertex)
															n1,					//!< Link to the second neighboring face (across the edge between the second and third triangle vertex)
															n2,					//!< Link to the third neighboring face (across the edge between the third and first triangle vertex)
															next0,				//!< Next face in the list of connected faces of the first vertex
															next1,				//!< Next face in the list of connected faces of the second vertex
															next2;				//!< Next face in the list of connected faces of the third vertex
					};
				};
				unsigned						attrib;							//!< Custom triangle attribute value
			
				MF_DECLARE	(char)					GetIndexOf(const MeshQuad<TFaceGraphDef<Def> >*q)		const;	//!< Determines the index of the specified neighboring quad \return requested index or -1 if the specified quad is no neighbor of the local triangle
				MF_DECLARE	(char)					GetIndexOf(const MeshTriangle<TFaceGraphDef<Def> >*t)	const;	//!< Determines the index of the specified neighboring triangle \return requested index or -1 if the specified triangle is no neighbor of the local triangle
				MF_DECLARE	(char)					indexOfTriangle(const MeshTriangle<TFaceGraphDef<Def> >*t)	const;	//!< Determines the index of the specified neighboring triangle \return requested index or -1 if the specified triangle is no neighbor of the local triangle
				MF_DECLARE	(BYTE)					requireIndexOf(const MeshQuad<TFaceGraphDef<Def> >*q)		const;	//!< Identical to GetIndexOf() except that requireIndexOf() throws an exception if -1 would have been returned
				MF_DECLARE	(BYTE)					requireIndexOf(const MeshTriangle<TFaceGraphDef<Def> >*t)	const;	//!< Identical to GetIndexOf() except that requireIndexOf() throws an exception if -1 would have been returned
				MF_DECLARE	(BYTE)					requireIndexOfTriangle(const MeshTriangle<TFaceGraphDef<Def> >*t)	const;	//!< Explicit version of the above
			
				MF_DECLARE	(char)					FindAndReplace(MeshQuad<TFaceGraphDef<Def> >*neighbor, MeshQuad<TFaceGraphDef<Def> >*with);
				MF_DECLARE	(char)					FindAndReplace(MeshQuad<TFaceGraphDef<Def> >*neighbor, MeshTriangle<TFaceGraphDef<Def> >*with);
				MF_DECLARE	(char)					FindAndReplace(MeshTriangle<TFaceGraphDef<Def> >*neighbor, MeshTriangle<TFaceGraphDef<Def> >*with);
				MF_DECLARE	(char)					FindAndReplace(MeshTriangle<TFaceGraphDef<Def> >*neighbor, MeshQuad<TFaceGraphDef<Def> >*with);
			
				MF_DECLARE	(String)					ToString(bool full=true)	const;		//!< Generates a string representation of the local triangle
			};

		template <class Def>
			struct TFaceGraphDefQuad:public Def::Quad	//! Additional quad information
			{
				union
				{
					struct
					{
						TMeshFaceLink<TFaceGraphDef<Def> >	n[4],				//!< Ctr::Array access link to all neighbors
															next[4];			//!< Ctr::Array access link to the respective next faces in the vertex face lists
					};
					struct
					{
						TMeshFaceLink<TFaceGraphDef<Def> >	n0,					//!< Link to the first neighboring face (across the edge between the first and second quad vertex)
															n1,					//!< Link to the second neighboring face (across the edge between the second and third quad vertex)
															n2,					//!< Link to the third neighboring face (across the edge between the third and fourth quad vertex)
															n3,					//!< Link to the fourth neighboring face (across the edge between the fourth and first quad vertex)
															next0,				//!< Next face in the list of connected faces of the first vertex
															next1,				//!< Next face in the list of connected faces of the second vertex
															next2,				//!< Next face in the list of connected faces of the third vertex
															next3;				//!< Next face in the list of connected faces of the fourth vertex
					};
				};

				unsigned							attrib;			//!< Custom quad attribute
			
				MF_DECLARE	(char)					GetIndexOf(const MeshQuad<TFaceGraphDef<Def> >*q)				const;	//!< Determines the index of the specified neighboring quad \return requested index or -1 if the specified quad is no neighbor of the local quad
				MF_DECLARE	(char)					GetIndexOf(const MeshTriangle<TFaceGraphDef<Def> >*t)			const;	//!< Determines the index of the specified neighboring triangle \return requested index or -1 if the specified triangle is no neighbor of the local quad
				MF_DECLARE	(BYTE)					requireIndexOf(const MeshQuad<TFaceGraphDef<Def> >*q)		const;	//!< Identical to GetIndexOf() except that requireIndexOf() throws an exception if -1 would have been returned
				MF_DECLARE	(BYTE)					requireIndexOf(const MeshTriangle<TFaceGraphDef<Def> >*t)	const;	//!< Identical to GetIndexOf() except that requireIndexOf() throws an exception if -1 would have been returned
			
				MF_DECLARE	(char)					FindAndReplace(MeshQuad<TFaceGraphDef<Def> >*v, MeshQuad<TFaceGraphDef<Def> >*with);
				MF_DECLARE	(char)					FindAndReplace(MeshQuad<TFaceGraphDef<Def> >*v, MeshTriangle<TFaceGraphDef<Def> >*with);
				MF_DECLARE	(char)					FindAndReplace(MeshTriangle<TFaceGraphDef<Def> >*v, MeshTriangle<TFaceGraphDef<Def> >*with);
				MF_DECLARE	(char)					FindAndReplace(MeshTriangle<TFaceGraphDef<Def> >*v, MeshQuad<TFaceGraphDef<Def> >*with);
		
				MF_DECLARE	(String)					ToString(bool full=true)	const;		//!< Generates a string representation of the local quad
			};

		/*!
			\brief Face oriented graph definition structure for Mesh instances
		
			A TFaceGraphDef provides the following additional links: Vertex to all connected faces, Triangle/Quad to all its neighboring faces.
			The main difference to TGraphDef is that face do not keep track of their neighboring edges and vertices map faces instead of edges.
			Additionally TGraphDef instances require a sub definition structure.
		*/
		template <class Def> struct TFaceGraphDef
		{
			typedef typename Def::Type				Type;			//!< Base float type as derived from the respective sub definition structure
			typedef TMeshFaceLink<TFaceGraphDef<Def> >FaceLink;		//!< Shortcut
		
			typedef TFaceGraphDefVertex<Def>		Vertex;

		
		
			struct Edge:public Def::Edge		//! Additional edge information (none)
			{};

			typedef TFaceGraphDefTriangle<Def>		Triangle;
			typedef TFaceGraphDefQuad<Def>			Quad;
		

		
			struct Mesh:public Def::Mesh	//! Additional mesh information
			{
				struct
				{
					index_t						first,	//!< Index of the first used triangle index
												last;	//!< Index of the last used triangle index
				}								vregion;
			};
		};
	/*

		template <class Def> struct TObjDataVertex:public Def::Vtx
		{
				typedef typename Def::Type	FTYPE	ALIGN1;
		
				FTYPE					position[4];
		}	ALIGN1;

		template <class Def> struct TObjDataEdge:public Def::Edg
		{
				FUINT32					vertex_index[2],face_index[2];
		}	ALIGN1;

		template <class Def> struct TObjDataQuad:public Def::Quad
		{
				FUINT32					vertex_index[4];
		}	ALIGN1;
	*/


		struct TMeshPrimitive
		{
			index_t						index;			//!< Index of this primitive in the respective field
			bool						marked;			//!< Reserved for custom usage
		};

		template <class Def>
			class MeshVertex:public TMeshPrimitive, public Def::Vertex	//! Base vertex of a Mesh<> instance
			{
			public:
					M::TVec3<typename Def::Type>	position;		//!< Position of this vertex in R3
			};
	

		template <class Def>
			class MeshEdge:public TMeshPrimitive, public Def::Edge	//! Base edge of a Mesh<> instance
			{
			public:
				union
				{
					MeshVertex<Def>			*vertex[2];		//!< Vertices this edge is connected to 
					struct
					{
						MeshVertex<Def>		*v0,			//!< First connected vertex (not NULL)
											*v1;			//!< Second connect vertex (not NULL)
					};
				};
				union
				{
					struct
					{
						TMeshFaceLink<Def>			n[2];			//!< Faces this edge is bordering
					};
					struct
					{
						TMeshFaceLink<Def>		neighbor0,				//!< First bordering face (required to be set)
												neighbor1;				//!< Second bordering face (may be unset)
					};
				};
			
			MF_DECLARE	(void)					linkUnary(MeshTriangle<Def>*t);	//!< Sets the first neighbor link to the specified triangle and unsets the second link
			MF_DECLARE	(void)					linkUnary(MeshQuad<Def>*q);		//!< Sets the first neighbor link to the specified quad and unsets the second link
			MF_DECLARE	(char)					GetIndexOf(const MeshVertex<Def>*v)			const;	//!< Determines the index of the specified vertex. \return requested index or -1 if the specified vertex is not linked by this edge
			MF_DECLARE	(char)					GetIndexOf(const MeshTriangle<Def>*t)			const;	//!< Determines the index of the specified triangle. \return requested index or -1 if the specified triangle is not linked by this edge
			MF_DECLARE	(char)					GetIndexOf(const MeshQuad<Def>*t)				const;	//!< Determines the index of the specified quad. \return requested index or -1 if the specified quad is not linked by this edge
			MF_DECLARE	(BYTE)					requireIndexOf(const MeshVertex<Def>*v)		const;	//!< Identical to GetIndexOf() except that requireIndexOf() throws an exception if -1 would have been returned
			MF_DECLARE	(BYTE)					requireIndexOf(const MeshTriangle<Def>*t)	const;	//!< Identical to GetIndexOf() except that requireIndexOf() throws an exception if -1 would have been returned
			MF_DECLARE	(BYTE)					requireIndexOf(const MeshQuad<Def>*t)		const;	//!< Identical to GetIndexOf() except that requireIndexOf() throws an exception if -1 would have been returned
	
			MF_DECLARE	(char)					FindAndReplace(MeshVertex<Def>*v, MeshVertex<Def>*with);
			MF_DECLARE	(char)					FindAndReplace(MeshQuad<Def>*v, MeshQuad<Def>*with);
			MF_DECLARE	(char)					FindAndReplace(MeshQuad<Def>*v, MeshTriangle<Def>*with);
			MF_DECLARE	(char)					FindAndReplace(MeshTriangle<Def>*v, MeshTriangle<Def>*with);
			MF_DECLARE	(char)					FindAndReplace(MeshTriangle<Def>*v, MeshQuad<Def>*with);
			
			};

		template <class Def>
			class MeshTriangle:public TMeshPrimitive, public Def::Triangle	//! Base triangle of a Mesh<> instance
			{
			public:
				union
				{
					MeshVertex<Def>			*vertex[3];		//!< Vertices this triangle is connected to
					struct
					{
						MeshVertex<Def>			*v0,			//!< Pointer to the first connected vertex (not NULL)
												*v1,			//!< Pointer to the second connected vertex (not NULL)
												*v2;			//!< Pointer to the third connected vertex (not NULL)
					};
				};

			MF_DECLARE	(char)					GetIndexOf(const MeshVertex<Def>*v)	const;		//!< Determines the index of the specified vertex. \return requested index or -1 if the specified vertex is not linked by this triangle
			MF_DECLARE	(BYTE)					requireIndexOf(const MeshVertex<Def>*v)	const;	//!< Identical to GetIndexOf() except that requireIndexOf() throws an exception if -1 would have been returned
			MF_DECLARE	(char)					FindAndReplace(MeshVertex<Def>*v, MeshVertex<Def>*with);
			};
	
		template <class Def>
			class MeshQuad:public TMeshPrimitive, public Def::Quad	//! Base quad of a Mesh<> instance
			{
			public:
				union
				{
					MeshVertex<Def>				*vertex[4];		//!< Vertices this quad is connected to
					struct
					{
						MeshVertex<Def>			*v0,			//!< Pointer to the first connected vertex (not NULL)
												*v1,			//!< Pointer to the second connected vertex (not NULL)
												*v2,			//!< Pointer to the third connected vertex (not NULL)
												*v3;			//!< Pointer to the fourth connected vertex (not NULL)
					};
				};

			MF_DECLARE	(char)					GetIndexOf(const MeshVertex<Def>*v)	const;		//!< Determines the index of the specified vertex. \return requested index or -1 if the specified vertex is not linked by this quad
			MF_DECLARE	(BYTE)					requireIndexOf(const MeshVertex<Def>*v)	const;	//!< Identical to GetIndexOf() except that requireIndexOf() throws an exception if -1 would have been returned
			MF_DECLARE	(char)					FindAndReplace(MeshVertex<Def>*v, MeshVertex<Def>*with);
			};
	

		/*!
			\brief Simple triangle mesh
		
			A triangle mesh consists of a field of vertices with each three vertices defining one triangle.
		*/
		template <class C>
			class TriangleMesh
			{
			public:
					count_t					vertices;	//!< Total number of vertices
					unsigned				band;		//!< Number of components per vertex
					Ctr::Array<C>				data;		//!< Actual vertex field

											TriangleMesh():vertices(0),band(0)	{}
											TriangleMesh(count_t vertices, unsigned band);
			virtual							~TriangleMesh();
			};
	
		/*!
			\brief Dynamic version of the Mesh class
		
			A dynamic mesh stores vertices, egdes, triangles, and quads in dynamic lists rather than fixed arrays, allowing the geometry to change without any inner links
			to become invalid.
		*/
		template <class Def>
			class DynamicMesh:public Def::Mesh
			{
			public:
					M::Vector3<MeshVertex<Def> >		vertices;		//!< List of vertex objects
					M::Vector3<MeshEdge<Def> >		edges;			//!< List of edge objects
					M::Vector3<MeshQuad<Def> >		quads;			//!< List of quad objects
					M::Vector3<MeshTriangle<Def> >	triangles;		//!< List of triangle objects

			static	String							error;
			
			MF_DECLARE	(void)						clear();		//!< Erases all contained elements
			MF_DECLARE	(void)						determineVertexWeights();	//!< Calculates the weight of each vertex depending on the overall geometry. Vertices with a greater weight value should be considered less redundant
			MF_DECLARE	(bool)						valid()	const;				//!< Checks validity of the local mesh
			};

		/*!
			\brief Abstract geometry

			An instance of Mesh requires a definition structure providing types for general floats and additional vertex, edge, quad, and triangle data.
			See object_def.h for examples. Note that these definition structures are \b not identical to CGS definition structures.
			Definition structures may be recursive as seen with the TGraphDef and TFaceGraphDef definition structures.
			Mesh instances may be copied and provide an adoptData() method.
	
		*/
		template <class Def>
			class Mesh:public Def::Mesh
			{
				typedef Mesh<Def>					Self;
			private:
			static	String							_error;
			static	MF_DECLARE (void)				_mergeIn(MeshVertex<Def>*&voffset,MeshEdge<Def>*&eoffset,MeshTriangle<Def>*&toffset,MeshQuad<Def>*&qoffset,const Mesh<Def>&obj);
					ObjMap<Def>						*_map;				//!< Pointer to an octree containing the local data. NULL by default. Automatically erased by the local object on deletion if not NULL.
			public:
					typedef MeshVertex<Def>			Vertex;
					typedef MeshEdge<Def>			Edge;
					typedef MeshTriangle<Def>		Triangle;
					typedef MeshQuad<Def>			Quad;

					Ctr::Array<Vertex>				vertex_field;		//!< Vertex array
					Ctr::Array<Edge>				edge_field;			//!< Edge array
					Ctr::Array<Triangle>			triangle_field;		//!< Triangle array
					Ctr::Array<Quad>				quad_field;			//!< Quad array
				
			MF_CONSTRUCTOR							Mesh();
			MF_CONSTRUCTOR							Mesh(const Mesh<Def>&other);
			MF_CONSTRUCTOR							Mesh(Mesh<Def>&&other):_map(other._map),vertex_field(other.vertex_field),edge_field(other.edge_field),triangle_field(other.triangle_field),quad_field(other.quad_field)
													{
														other._map = NULL;
														ASSERT_IS_NULL__(other.vertex_field.pointer());	//must be moved, otherwise consistency is violated
													}
			template <class T>MF_CONSTRUCTOR		Mesh(const Mesh<T>&other);
			virtual									~Mesh();
			MF_DECLARE	(void)						clear();			//!< Clears all local data
			MF_DECLARE	(void)						Resize(count_t vcnt, count_t ecnt, count_t tcnt, count_t qcnt);	//!< @deprecated Resizes all arrays to the respective sizes.
			MF_DECLARE	(void)						unmark(unsigned selection=O_ALL);	//!< Sets the 'marked' member of the specified member components to false \param selection May be any combination of O_VERTICES, O_EDGES, O_TRIANGLES, and O_QUADS. Additionally O_FACES combine O_TRIANGLES and O_QUADS and O_ALL means all components. Non specified components are left unchanged.

			MF_DECLARE	(void)						correct(unsigned selection=O_ALL);	//!< Attempts to correct any existing link errors among the specified components. Incorrect components that cannot be repaired are dropped. \param selection May be any combination of O_VERTICES, O_EDGES, O_TRIANGLES, and O_QUADS. Additionally O_FACES combine O_TRIANGLES and O_QUADS and O_ALL means all components. Non specified components are left unchanged.
			MF_DECLARE	(void)						invert(unsigned selection=O_FACES);	//!< Inverts the orientation of the specified face components.  \param selection May be any combination of O_VERTICES, O_EDGES, O_TRIANGLES, and O_QUADS. Additionally O_FACES combine O_TRIANGLES and O_QUADS and O_ALL means all components. Non specified components are left unchanged.
				
			MF_DECLARE	(void)						generateEdges();					//!< Generates edges based on the local face data. Additional linkage such as that of TGraphDef is not generated.
			MF_DECLARE	(bool)						isConvex(bool update_edges=false);	//!< Uses existing edge data to determine whether or not the local mesh is convex. Edge data is generated via generateEdges() if it doesn't exist @return true if the local mesh is convex
				
			MF_DECLARE	(void)						generateVertexNormals();			//!< Generates vertex normals based on the current quad/triangle configuration. The used definition structure must provide normals per vertex for this method to work
			MF_DECLARE	(void)						generateFaceNormals();				//!< Generates triangle/quad normals. The used definition structure must provide normals per triangle/quad for this method to work
			MF_DECLARE	(void)						generateNormals();					//!< Generates both vertex and triangle/quad normals. The used definition structure must provide normals per vertex and triangle/quad for this method to work
				
			MF_DECLARE	(void)						toGraph(Mesh<TGraphDef<Def> >&target)			const;				//!< Generates a graph mesh of the local geometrical data
			MF_DECLARE	(void)						toGraph(DynamicMesh<TGraphDef<Def> >&target)		const;				//!< Generates a dynamic graph mesh of the local geometrical data
			MF_DECLARE	(void)						toGraph(Mesh<TFaceGraphDef<Def> >&target, bool include_edges=true)		const;	//!< Generates a face graph mesh of the local geometrical data \param target Target graph mesh \param include_edges Set true to also generate edges for the resulting face graph.
			MF_DECLARE	(void)						toGraph(DynamicMesh<TFaceGraphDef<Def> >&target, bool include_edges=true)const;	//!< Generates a dynamic face graph mesh of the local geometrical data \param target Target graph mesh \param include_edges Set true to also generate edges for the resulting face graph.	                                                                                                                                                                                     
			
					/**
						@brief Replaces the local geometry content with a 3d function plotting of the specified function
					
						The resulting function will be build along X and Z with the function result pointing along the Y axis.
						The method does not generate edges. Also additional information such as normals optionally defined in the definition structure is not generated
					
						@param min_x Bottom edge of the plotted section along the x axis
						@param max_x Top edge of the plotted section along the x axis
						@param xres Number of quads used along the x axis
						@param min_z Bottom edge of the plotted section along the z axis
						@param max_z Top edge of the plotted section along the z axis
						@param zres Number of quads used along the z axis
						@param function Pointer to a function to resolve the height values of each vertex
					*/
			template <typename T>
			MF_DECLARE	(void)						buildFromFunction(const T&min_x, const T&max_x, count_t xres, const T&min_z, const T&max_z, count_t zres, T (*function)(const T&x, const T&z));

			MF_DECLARE	(void)						buildCube();	//!< Rebuilds the local geometry as a cube in the range [-1,+1]³
			MF_DECLARE	(void)						buildSphere(count_t resolution=50);	//!< Rebuilds the local geometry as a sphere of radius 1. @param resolution Number of vertices along the equador, twice the number of vertices from north to south pole. The total number of vertices is thus resolution*resolution/2
			MF_DECLARE	(void)						buildCylinder(count_t resolution=50);


				/*!
					\brief Checks if there is a valid closer intersection of the local mesh and a ray
				
					The method calculates the intersection distance (if any) of the specified ray and the local geometry. If such an intersection occurs and it's closer than the specified
					distance then the distance variable will be updated and the result be true. detectOpticalIntersection() uses _oDetectOpticalIntersection().
				
					\param b Ray base position
					\param d Normalized ray direction vector
					\param distance In/out distance scalar. This value will be updated if a closer (positive) intersection was detected.
					\return true if a positive intersection closer than the specified distance was detected, false otherwise. The specified distance value remains unchanged if the result is false.
				*/
				template <typename T0, typename T1>
			MF_DECLARE	(bool)						detectOpticalIntersection(const T0 b[3], const T1 d[3], float&distance)	const;
				
				
			MF_DECLARE	(bool)						IsEmpty()																				const;	//!< Returns true if the local mesh contains no vertices, egdes, or faces
			MF_DECLARE	(bool)						IsValid(const TMeshFaceLink<Def>&link, bool may_be_null=true, bool may_be_marked=true)	const;	//!< Checks if the specified face link is valid \param may_be_null Set true to allow an unset (NULL) target \param may_be_marked Set true to allow a marked target
			MF_DECLARE	(sindex_t)					linkToIndex(const TMeshFaceLink<Def>&link)												const;	//!< Converts a link to an integer index. The resulting int is negative for a quad target and positive for a triangle target. Quad indices start with -1 and decrease, triangle	indices start with 0 and increase. If the specified link is not set then the result will be the largest positive integer.
			MF_DECLARE	(bool)						indexToLink(sindex_t index, TMeshFaceLink<Def>&link);													//!< Converts a link integer to an actual face link. The method effectivly reverts the result of linkToIndex(). \param index Face target index as retrieved from linkToIndex() \param link Target face link. The target face link will be unset if \b index is invalid. \return true if the specified index was valid and link could be set, false otherwise
				
			MF_DECLARE	(typename Def::Type)		getVolume(bool alternate = true)					const;		//!< Determines the volume of the local geometry. If the local geometry uses m as unit then the result will be m³. The resulting value may be inaccurate if the local hull is not closed.
		
				
			/** 
				@brief (Re)generates the map of the local geometry, and replaces the map in the local mesh.
		
				@param tag Components that should be mapped. May be any combination of O_VERTICES, O_EDGES, O_TRIANGLES, and O_QUADS.
							Additionally O_FACES combine O_TRIANGLES and O_QUADS and O_ALL means all components. Non specified components are not mapped.

				@param max_depth Maximum recursion depth.
			*/
			MF_DECLARE	(void)						buildMap(BYTE tag, unsigned max_depth);
			MF_DECLARE	(ObjMap<Def>*)				getMap()											const	{return _map;};
			MF_DECLARE	(void)						flushMap();													//!< Discards the local map, if existent
			MF_DECLARE	(void)						verifyIntegrity()									const;	//!< Checks the validity of the local geometry and triggers a fatal exception if it isn't.
			MF_DECLARE	(bool)						valid(bool check_range=true)						const;	//!< Checks the validity of the local geometry. \param check_range Set true to also check link pointers for broken links, otherwise just for broken neighbor links and collapsed vertex pointers. \return true if the local geometry is found valid, false otherwise. Use errorStr() to retrieve an error description if the method returns false.
			MF_DECLARE	(String)					errorStr()											const;	//!< Generates a string representation of the last occured error
			MF_DECLARE	(void)						vertexDimensions(M::Box<typename Def::Type>&field)	const;	//!< Determines a box enclosing all local vertices \param field Out bounding box with the first three components defining the lower corner and the last three components defining the upper corner
			MF_DECLARE	(void)						getBoundingBox(M::Box<typename Def::Type>&field)		const	{vertexDimensions(field);}	//!< Determines a box enclosing all local vertices \param field Out bounding box with the first three components defining the lower corner and the last three components defining the upper corner

			template <typename T>
			MF_DECLARE	(void)						scale(const T&factor);									//!< Scales the local geometry by the specified factor.

			MF_DECLARE	(String)					difference(Mesh<Def>&other, const String&intend);	//!< Depreciated. Attempts to form a string presentation of the difference between the local and the remote mesh.
			MF_DECLARE	(String)					ToString()	const;										//!< Retrieves a string representation of the local mesh. Does not contain any actual components.
				
			MF_DECLARE	(void)						join(const Mesh<Def>*others, count_t count);		//!< Adds a number of mesh objects to the local mesh
			MF_DECLARE	(void)						join(const Ctr::Array<Mesh<Def> >&others);				//!< Adds a number of mesh objects to the local mesh
				
			MF_DECLARE	(void)						adoptData(Mesh<Def>&other);
			MF_DECLARE	(void)						swap(Mesh<Def>&other);
			friend void								swap(Self&a, Self&b)	{a.swap(b);}

			MF_DECLARE	(Mesh<Def>&)				operator=(const Mesh<Def>&other);
			template <class T>
			MF_DECLARE	(Mesh<Def>&)				operator=(const Mesh<T>&other);
			};




		/*!
			\brief Balanced octree for Mesh instances
	  
			A ObjMap may be root, inner node or leaf of an octree generated from a Mesh instance.
			The octree adapts to the actual geometry dimension and evently splits sectors into 8 sub sectors where needed.
		*/
		template <class Def>
			class ObjMap
			{
			public:
				typedef typename Def::Type	C;

				MF_DECLARE	(void)			recursiveMap(ObjMap<Def>*source,BYTE tag); //upwards (slow)
				MF_DECLARE	(void)			recursiveMap(Ctr::ArrayData<MeshVertex<Def>*> vbuffer[8], Ctr::ArrayData<MeshEdge<Def>*> ebuffer[8], Ctr::ArrayData<MeshTriangle<Def>*> tbuffer[8],Ctr::ArrayData<MeshQuad<Def>*> qbuffer[8],BYTE tag); //downwards (fast)
				MF_DECLARE	(bool)			recursiveLookup(const C*lower_corner, const C*upper_corner);
				MF_DECLARE	(count_t)		recursiveLookupEdge(const C*p0, const C*p1);
				MF_DECLARE	(count_t)		recursiveLookupSphere(const C*p, const C&r);

				Ctr::Array<MeshVertex<Def>*>		vertex_field;
				Ctr::Array<MeshTriangle<Def>*>	triangle_field;
				Ctr::Array<MeshQuad<Def>*>		quad_field;
				Ctr::Array<MeshEdge<Def>*>		edge_field;
			public:
				static Container::Buffer<ObjMap<Def>*>sector_map;					//!< Global sector lookup buffer
	
				M::Box<C>						dim;								//!< Bounding box
				ObjMap<Def>*				child[8];							//!< Pointer to node children or NULL if the respective child does not exist
				unsigned					level;								//!< Recursive level of the local map (0=lowest level/leaf)
				count_t						bad_hits;							//!< Number of double mappings encountered during the last lookup
				bool						keep_dimensions;					//!< Set true to not redetermine the geometry dimensions during map() calls
			#ifndef THROW_BUFFER_ERROR
				bool						overflow;							//!< Automatically set true if an overflow occured during the last lookup call
			#endif

											ObjMap(unsigned depth, bool keep = false);		//!< Default map constructor \param depth Maximum recursion depth \param keep Forwarded to the local \b keep_dimensions variable
											ObjMap(const ObjMap<Def>&other);				//!< Copy constructor
											ObjMap(const M::Box<C>&dimension, unsigned level);	//!< Default child constructor \param dimension Bounding box of this node \param level Recursive level of this node
				virtual						~ObjMap();
				MF_DECLARE	(void)			map(Mesh<Def>&mesh, BYTE tag);					//!< Recursivly maps the specified mesh into the local tree \param mesh Mesh to map \param tag Components that should be mapped. May be any combination of O_VERTICES, O_EDGES, O_TRIANGLES, and O_QUADS. Additionally O_FACES combine O_TRIANGLES and O_QUADS and O_ALL means all components. Non specified components are not mapped. 
				MF_DECLARE	(void)			drop(index_t index, BYTE tag = O_ALL);				//!< Drops all elements of the specified type and index
				MF_DECLARE	(void)			reset(unsigned depth, bool keep=false);				//!< Clears all local data and updates the initial configuration
				MF_DECLARE	(void)			clear();											//!< Clears the local node and erases all children
				MF_DECLARE	(bool)			empty()													const;	//!< Queries whether or not the local tree is empty
				MF_DECLARE	(count_t)		Count()													const;
				MF_DECLARE	(count_t)		countSubMaps()											const;
				MF_DECLARE	(count_t)		lookup(const C lower_corner[3], const C upper_corner[3]);		//!< Recursivly preforms a lookup for sectors intersecting (or residing inside) the specified volume. Found sectors are written to \b sector_map. Required for succeeding resolve calls \param lower_corner Lower volume corner \param upper_corner Upper volume corner \return Number of sectors found inside or on the border of the specified volume 
				MF_DECLARE	(count_t)		lookupEdge(const C p0[3], const C p1[3]);						//!< Recursivly performs a lookup for sectors intersecting the specified edge. Found sectors are written to \b sector_map. Required for succeeding resolve calls \param p0 Composite of the first edge vertex \param p1 Composite of the second edge vertex \return Number of sectors found intersecting the specified edge
				MF_DECLARE	(count_t)		lookupSphere(const C center[3], const C&radius);
				MF_DECLARE	(ObjMap<Def>*)	lookupCover(const C lower_corner[3], const C upper_corner[3]);	//!< Tries to find the closest sector entirely covering the specified volume \param lower_corner Lower corner coordinates of the box \param upper_corner Upper corner coordinates of the box \return Smallest Sectors that covers the entire specified box. The result may not entirely cover the box if no such sector exists.
				MF_DECLARE	(count_t)		resolveQuads(Container::Buffer<MeshQuad<Def>*>&face_out);					//!< Copies all distinct quad pointers from the found sectors (if any) to the specified buffer. This method requires the result of either lookup(), lookupSphere(), or lookupEdge() to work properly \param face_out Out buffer for found quads. The buffer will not be reset before writing \return Number of quad pointers written to the buffer
				MF_DECLARE	(count_t)		resolveTriangles(Container::Buffer<MeshTriangle<Def>*>&face_out);			//!< Copies all distinct triangle pointers from the found sectors (if any) to the specified buffer. This method requires the result of either lookup(), lookupSphere(), or lookupEdge() to work properly \param face_out Out buffer for found triangles. The buffer will not be reset before writing  \return Number of triangle pointers written to the buffer                                                                                                                                                                                                                                                                                                      
				MF_DECLARE	(count_t)		resolveEdges(Container::Buffer<MeshEdge<Def>*>&edge_out);					//!< Copies all distinct edge pointers from the found sectors (if any) to the specified buffer. This method requires the result of either lookup(), lookupSphere(), or lookupEdge() to work properly \param edge_out Out buffer for found edges. The buffer will not be reset before writing \return Number of edge pointers written to the buffer
				MF_DECLARE	(count_t)		resolveVertices(Container::Buffer<MeshVertex<Def>*>&vertex_out);			//!< Copies all distinct vertex pointers from the found sectors (if any) to the specified buffer. This method requires the result of either lookup(), lookupSphere(), or lookupEdge() to work properly \param edge_out Out buffer for found vertices. The buffer will not be reset before writing \return Number of vertex pointers written to the buffer
			};


		template <class FloatType=float>
			class Point:public M::TVec3<FloatType>	//! General usage 3d point structure
			{
			public:
				typedef FloatType			Float;
				index_t						index;		//!< custom client application usage

				MF_CONSTRUCTOR				Point();
				MF_CONSTRUCTOR				Point(const Float&x, const Float&y, const Float&z);
				MF_CONSTRUCTOR				Point(const M::TVec3<Float>&position);
				MFUNC	(bool)				operator>(const M::TVec3<C>&other)	const;	//!< Vector order priorizing x over y and y over z
				MFUNC	(bool)				operator<(const M::TVec3<C>&other)	const;	//!< Vector order priorizing x over y and y over z
				MF_DECLARE	(String)		ToString()	const
											{
												return M::Vec::toString(*this);
											}
			};
	
		template <class FloatType=float>
			class NormalPoint:public Point<FloatType>	//! Point including normal
			{
			public:
				typedef FloatType			Float;
				M::TVec3<Float>				normal;		//!< Point normal

				MF_CONSTRUCTOR				NormalPoint();
				MF_CONSTRUCTOR				NormalPoint(const Float&x, const Float&y, const Float&z,const Float&nx, const Float&ny, const Float&nz);
				MF_CONSTRUCTOR				NormalPoint(const M::TVec3<Float>&position, const M::TVec3<Float>&normal);
				MFUNC(bool)					operator>(const NormalPoint<C>&other)	const;	//!< Point order priorizing position over normal
				MFUNC(bool)					operator<(const NormalPoint<C>&other)	const;	//!< Point order priorizing position over normal
			};
	

		template <class FloatType=float>
			class AbstractSphere : public M::Sphere<FloatType>	//! Sphere definition
			{
			public:
				typedef M::Sphere<FloatType>	Super;
				typedef FloatType			Float;
			
				MF_DECLARE	(void)			resolveIndentation(const AbstractSphere<Float>&remote, Float&indentation, M::TVec3<Float>&indentation_vector, bool verbose)	const;	//!< Attempts to determine the direction and intensity of an intersection between the local sphere and a remote abstract geometry  \param remote Geometry to determine the indentation/intersection of \param indentation_vector Out vector that the local sphere would have to be moved by to deintersect the two geometries. The resulting vector is of length 0 if the two geometries don't intersect.
				MF_DECLARE	(void)			resolveIndentation(const AbstractCylinder<Float>&remote, Float&indentation, M::TVec3<Float>&indentation_vector, bool verbose)	const;	//!< Attempts to determine the direction and intensity of an intersection between the local sphere and a remote abstract geometry  \param remote Geometry to determine the indentation/intersection of \param indentation_vector Out vector that the local sphere would have to be moved by to deintersect the two geometries. The resulting vector is of length 0 if the two geometries don't intersect.
				MF_DECLARE(bool)			intersects(const AbstractSphere<Float>&remote)		const;
				MF_DECLARE(bool)			intersects(const AbstractCylinder<Float>&remote)	const;
			};
	
		template <class FloatType=float>
			class AbstractCylinder	//! Cylinder definition
			{
			public:
				typedef FloatType			Float;
			
				Point<Float>				p0,			//!< Lower circle center (point)
											p1;			//!< Upper circle center (point)
				Float						radius;		//!< Cylinder radius (scalar)
			
				MF_CONSTRUCTOR				AbstractCylinder(const Float&radius=1);
				MF_CONSTRUCTOR				AbstractCylinder(const M::TVec3<Float>&p0, const M::TVec3<Float>&p1, const Float&radius=1);

				MF_DECLARE	(void)			resolveIndentation(const AbstractSphere<Float>&remote, Float&indentation, M::TVec3<Float>& indentation_vector, bool verbose)	const;	//!< Attempts to determine the direction and intensity of an intersection between the local geometry and a remote abstract geometry  \param remote Geometry to determine the indentation/intersection of \param indentation_vector Out vector that the local cylinder would have to be moved by to deintersect the two geometries. The resulting vector is of length 0 if the two geometries don't intersect.
				MF_DECLARE	(void)			resolveIndentation(const AbstractCylinder<Float>&remote, Float&indentation, M::TVec3<Float>& indentation_vector, bool verbose)	const;//!< Attempts to determine the direction and intensity of an intersection between the local and a remote abstract geometry  \param remote Geometry to determine the indentation/intersection of \param indentation_vector Out vector that the local cylinder would have to be moved by to deintersect the two geometries. The resulting vector is of length 0 if the two geometries don't intersect.
				MF_DECLARE(bool)			intersects(const AbstractSphere<Float>&remote)		const;
				MF_DECLARE(bool)			intersects(const AbstractCylinder<Float>&remote)	const;
				MFUNC(bool)					Contains(const M::TVec3<C>&point)	const;
			};
			
			
			

		template <class FloatType=float>
			class AbstractHull	//! Collection of abstract spheres and cylinders
			{
			public:
				typedef FloatType			Float;
			
				Ctr::Array<AbstractSphere<Float> >spheres;
				Ctr::Array<AbstractCylinder<Float> >cylinders;
		
				MF_CONSTRUCTOR				AbstractHull(count_t spheres=0, count_t cylinders=0);
				virtual						~AbstractHull()	{}

				MF_DECLARE(bool)			intersects(const AbstractHull<Float>&remote)		const;
				MF_DECLARE(bool)			intersects(const AbstractSphere<Float>&remote)		const;
				MF_DECLARE(bool)			intersects(const AbstractCylinder<Float>&remote)	const;
				MF_DECLARE(void)			resolveIndentation(const AbstractSphere<Float>&remote, Float&indentation, M::TVec3<Float>& indentation_vector, bool verbose)	const;	//!< Attempts to determine the direction and intensity of an intersection between the local geometry and a remote abstract geometry  \param remote Geometry to determine the indentation/intersection of \param indentation_vector Out vector that the local cylinder would have to be moved by to deintersect the two geometries. The resulting vector is of length 0 if the two geometries don't intersect.
				MF_DECLARE(void)			resolveIndentation(const AbstractCylinder<Float>&remote, Float&indentation, M::TVec3<Float>& indentation_vector, bool verbose)	const;//!< Attempts to determine the direction and intensity of an intersection between the local and a remote abstract geometry  \param remote Geometry to determine the indentation/intersection of \param indentation_vector Out vector that the local cylinder would have to be moved by to deintersect the two geometries. The resulting vector is of length 0 if the two geometries don't intersect.
				MF_DECLARE(bool)			detectMaximumIndentation(const AbstractHull<Float>&remote, M::TVec3<>&indentation_out, bool verbose)	const;	//!< Detects the maximum indentation/intersection vector of the local and a remote abstract hull \param remote Hull to determine the indentation/intersection of \param indentation_out Out vector that the local hull would have to be moved by to deintersect the two hulls. The resulting vector is of length 0 if the two hulls don't intersect.
				MFUNC(void)					translate(const M::TVec3<C>&delta);	//!< Translates the entire hull (all spheres and cylinders)
				MF_DECLARE	(String)		ToString()	const;				//!< Generates a string representation of the local hull
				void						adoptData(AbstractHull<Float>&other)	{spheres.adoptData(other.spheres); cylinders.adoptData(other.cylinders);}
				void						swap(AbstractHull<Float>&other)			{spheres.swap(other.spheres); cylinders.swap(other.cylinders);}
			};
	
	
		template <class Float>
			MF_DECLARE (void)				makeSphere(const Float&radius,AbstractHull<Float>&out_hull);	//!< Generates an abstract hull that contains just one sphere (at the point of origin) of the specified radius
		template <class Float>
			MF_DECLARE (void)				makeSphere(const M::TVec3<Float>&center, const Float&radius,AbstractHull<Float>&out_hull);	//!< Generates an abstract hull that contains just one sphere at the specified center of the specified radius

		template <class Float>
			MF_DECLARE (void)				makeCapsule(const M::TVec3<Float>&p0, const M::TVec3<Float>&p1, const Float&radius,AbstractHull<Float>&out_hull);	//!< Generates an abstract hull consisting of two spheres and a connecting cylinder \param p0 Center of the first sphere/cylinder circle \param p1 Center of the second sphere/cylinder circle \param radius Radius of the spheres and cylinder \return new hull containing a cylinder and two spheres


		template <typename Float=float>
			class ConvexHullBuilder	//! Helper class to iterativly build a convex hull
			{
			public:
					struct TTriangle
					{
						union
						{
							struct
							{
								UINT32			v0,
												v1,
												v2,
												n0,
												n1,
												n2;
							};
							struct
							{
								UINT32			v[3],
												n[3];
							};
						};
						M::TVec3<Float>			normal;
						bool					flagged;
						M::TVec3<UINT32>			link;
					
						MF_DECLARE (TTriangle&)		Set(UINT32 v0_, UINT32 v1_, UINT32 v2_, UINT32 n0_, UINT32 n1_, UINT32 n2_)
												{
													v0 = v0_;
													v1 = v1_;
													v2 = v2_;
													n0 = n0_;
													n1 = n1_;
													n2 = n2_;
													flagged = false;
													return *this;
												}
					};
				
					struct TEdge
					{
						UINT32					v0,
												v1,
												t0,
												t1;
						BYTE					index;
					};
				
					struct TPoint
					{
						M::TVec3<Float>			vector;
						UINT32					index0,
												index1;
						template <typename T>
						MF_DECLARE (TPoint&)	Set(const M::TVec3<T>&v, UINT32 i0, UINT32 i1)
												{
													vector = v;
													index0 = i0;
													index1 = i1;
													return *this;
												}
					};
				
					Container::Buffer<TPoint>		buffer;	//!< Points that have not yet been inserted into the hull
					Container::Buffer<TPoint>		vertices;	//!< Mapped hull point
					Container::Buffer<TTriangle>	triangles;
					Container::Buffer<TEdge>		edges;	//non-persistent
				
					MF_DECLARE (void)				UpdateNormal(TTriangle&triangle)	const;
				
					MF_DECLARE (void)				VerifyIntegrity()	const;
					MF_DECLARE (bool)				DetectTedrahedron();			//!< Detects and creates a tedrahedron if possible
				
					MF_DECLARE (void)				Reset();						//!< Resets the local hull building process
				template <typename T>
					MF_DECLARE (void)				Include(const M::TVec3<T>&point);		//!< Includes a point into the convex hull.
				template <typename T>
					MF_DECLARE (void)				Include(const Point<T>&point);	//!< Includes a point into the convex hull.
				
				template <class Def>
					MF_DECLARE (void)				ExportToMesh(Mesh<Def>&mesh)	const;	//!< Exports the current convex hull to a mesh
			};
		
	
		template <class C>
			class SplineQuad	//!< A spline based quad. Constructable from points and normals
			{
			protected:
			static	MF_DECLARE (void)		resolveDeltaVectors(const C p0[3], const C n0[3], const C p1[3], const C n1[3], const C&control_factor, C c0[3], C c1[3]);
		
			public:
					C			control[4][4][3];	//!< control points of the final spline quad. p0 maps to control[0][0], p1 to control[3][0], p2 to control[3][3], and p3 to control[0][3]
				
					/**
						@brief Resolves a point on the loaded quad curve

						@param x [in] X coordinate of the requested point in the range [0,1]
						@param y [in] Y coordinate of the requested point in the range [0,1]
						@param out [out] Resulting 3d coordinates
					*/
			MF_DECLARE	(void)		resolve(const C&x, const C&y, C out[3])	const;
					/**
						@brief Resolves the X direction vector of a point on the loaded quad curve
						@param x [in] X coordinate of the requested point in the range [0,1]
						@param y [in] Y coordinate of the requested point in the range [0,1]
						@param out [out] Resulting 3d direction vector. Not normalized.
					*/
			MF_DECLARE	(void)		resolveDirectionX(const C&x, const C&y, C out[3]) const;
					/**
						@brief Resolves the Y direction vector of a point on the loaded quad curve
						@param x [in] X coordinate of the requested point in the range [0,1]
						@param y [in] Y coordinate of the requested point in the range [0,1]
						@param out [out] Resulting 3d direction vector. Not normalized.
					*/
			MF_DECLARE	(void)		resolveDirectionY(const C&x, const C&y, C out[3]) const;
					/**
						@brief (Re)Builds the local spline quad using four points and their respective normals.

						Points should be oriented in counterclockwise order


						@param p0 3d Position of the first point
						@param n0 3d Normal of the first point (not required to be normalized. Must not be 0)
						@param p1 3d Position of the second point
						@param n1 3d Normal of the second point (not required to be normalized. Must not be 0)
						@param p2 3d Position of the third point
						@param n2 3d Normal of the third point (not required to be normalized. Must not be 0)
						@param p3 3d Position of the fourth point
						@param n3 3d Normal of the fourth point (not required to be normalized. Must not be 0)
						@param control_factor Relative distance between a point's direct and intermediate controls.
									When applied this value is multipled with the distance between each two opposing points.
									Typically in the range (0,0.5). Alternate values may cause undesired results
					*/
			MF_DECLARE	(void)		build(const C p0[3], const C n0[3], const C p1[3], const C n1[3], const C p2[3], const C n2[3], const C p3[3], const C n3[3], const C&control_factor=0.45);
			};

	}


}

#include "object.tpl.h"

#endif

