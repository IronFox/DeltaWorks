#ifndef engine_aspectH
#define engine_aspectH

/******************************************************************

Camera-structures (in general aspects) to simplify usage of
different types of cameras.

******************************************************************/


#include <math/matrix.h>
#include <math/object.h>
#include "enumerations.h"
//#include <general/undef.h>

namespace Engine
{
	using namespace DeltaWorks;

template <class>	class	Aspect;
template <class>	class	OrthographicAspect;
template <class>	class	Camera;
template <class>	class	AngularCamera;
template <class>	class	VectorCamera;


template <class C=float>
class	Frustum		//! Volume description of the visible area.
{
private:
		MF_DECLARE(void)	defNormal(const M::TVec3<C>&p0, const M::TVec3<C>&p1, const M::TVec3<C>&p2, M::TVec3<C>&target);
template <typename Subtype>
		MF_DECLARE(void)	defNormalST(const M::TVec3<C>&p0, const M::TVec3<C>&p1, const M::TVec3<C>&p2, M::TVec3<C>&target);
		MF_STATIC(1,bool)	intersects(const C0&x0, const C0&y0, const C0&z0,const C0&x1, const C0&y1, const C0&z1,const M::TVec3<C>&p0, const M::TVec3<C>&p1, const M::TVec3<C>&p2, const M::TVec3<C>&p3);
		MF_STATIC(1,bool)	intersects(const M::Box<C0>&box, const M::TVec3<C>&p0, const M::TVec3<C>&p1, const M::TVec3<C>&p2, const M::TVec3<C>&p3);
		MF_STATIC(1,bool)	TestTriangle(const M::TVec3<C0>&p0,const M::TVec3<C0>&p1,const M::TVec3<C0>&p2,const M::TVec3<C0>&e0,const M::TVec3<C0>&d);

public:
	union
	{
		struct
		{
			M::TVec3<C>		corner[8];	//!< volume corners. each contains an R3 vector
		};
		struct
		{
			M::TVec3<C>		nearCorner[4],		//!< near volume corners. each contains an R3 vector
							farCorner[4];			//!< far volume corners. each contains an R3 vector
		};
		struct
		{
			M::TVec3<C>		near_bottom_left,	//!< volume corner in R3
							near_bottom_right,	//!< volume corner in R3
							near_top_right,		//!< volume corner in R3
							near_top_left,		//!< volume corner in R3
							far_bottom_left,	//!< volume corner in R3
							far_bottom_right,	//!< volume corner in R3
							far_top_right,		//!< volume corner in R3
							far_top_left;		//!< volume corner in R3
		};
	};
	union
	{
		M::TVec3<C>			normal[6];			//!< normalized volume face normals (pointing out)
		struct
		{
			M::TVec3<C>		near_normal,		//!< normalized volume face normal (pointing out)
							far_normal,			//!< normalized volume face normal (pointing out)
							bottom_normal,		//!< normalized volume face normal (pointing out)
							right_normal,		//!< normalized volume face normal (pointing out)
							top_normal,			//!< normalized volume face normal (pointing out)
							left_normal;		//!< normalized volume face normal (pointing out)
		};
	};
		enum Visibility		//! Visibility check result
		{
			NotVisible,				//!< Checked entity is not visible
			PartiallyVisible,		//!< Checked entity is partially visible (Due to the nature of the approximation, this result does not exclude full visibility)
			FullyVisible			//!< Checked entity is fully visible
		};

MFUNC3	(bool)				visible(const C0&x, const C1&y, const C2&z) const;		//!< Determines visibility of a point \return true if the specified point lies within the local frustum
MFUNC2	(bool)				visible(const M::TVec3<C0>&center, const C1&radius)	const;	//!< Determines visibility of a sphere \param center Center of the sphere \param radius Radius of the sphere \return true if the specified sphere is inside or intersects the volume, false otherwise
MFUNC2	(bool)				IsVisible(const M::TVec3<C0>&center, const C1&radius)	const;	//!< Identical to visible()
MFUNC1	(bool)				IsVisible(const M::Sphere<C0>&sphere)	const	{return IsVisible(sphere.center,sphere.radius);}
MFUNC1	(bool)				IsVisible_UnpreciseExpensiveDoNotUse(const M::Box<C0>&)	const;
MFUNC2	(Visibility)		CheckSphereVisibility(const M::TVec3<C0>&center, const C1&radius)	const;	//!< Determines visibility of a sphere \param center Center of the sphere \param radius Radius of the sphere \return sphere visibility
MFUNC1	(Visibility)		CheckSphereVisibility(const M::Sphere<C0>&sphere)	const	{return CheckSphereVisibility(sphere.center,sphere.radius);}
MF_DECLARE (Visibility)		CheckBoxVisibility_UnpreciseExpensiveDoNotUse(const M::Box<C>&)	const;	 //!< Determines visibility of a box \return box visibility

MFUNC2	(bool)				IntersectsCone(const M::TVec3<C0>&center, const C1&radius) const;	//!< Similar to visible(), however ignoring the znear and zfar planes, making the frustum infinitly long
MF_DECLARE(void)			UpdateNormals();								//!< Recalculate normals from the modified volume edges
MF_DECLARE(void)			UpdateNormalsP();								
template <typename SubType>
MF_DECLARE(void)			UpdateNormalsST();								
MF_DECLARE(String)			ToString()	const;
MF_DECLARE(void)			FlipNormals();

MFUNC1	(Frustum<C>&)		operator=(const Frustum<C0>&other);
};



namespace GlobalAspectConfiguration
{
extern	bool					worldZIsUp,			//!< Declares whether y or z should point up. Default is false, causing y to point up
								loadAsProjection;		//!< Declares that the entire aspect (any camera, view and projection) should be loaded into the projection matrix
};

template <class C=float>
class	Aspect		//! Generalized aspect container. It contains various matrices, the current center location and the region on the screen
{
private:
	static	Frustum<C>			result;
public:
	typedef Frustum<C>			Volume;		//!< Visual volume type
	typedef C					Type;				//!< Local type


	M::TMatrix4<Type>			view,					//!< 4x4 out view matrix (primary). Set to identity by default
								viewInvert,			//!< 4x4 in view matrix (inverted). Set to identity by default
								projection,				//!< 4x4 out projection matrix (primary). Set to identity by default
								projectionInvert;		//!< 4x4 in projection matrix (inverted). Set to identity by default
	M::TVec3<Type>					location;				//!< Central aspect location. Set to (0,0,0) by default

	M::TFloatRect				region;					//!< Aspect screen region (set to 0,0,1,1 by default)
	eDepthTest					depthTest;	//!< Depth test configuration of this aspect ( Engine::VisualEnum::NormalDepthTest by default)

	static const Aspect<C>		identity;	//!< identity aspect using normal depth test
	MF_CONSTRUCTOR				Aspect();
	MF_CONSTRUCTOR				Aspect(VisualEnum::eDepthTest dtest);

	MF_DECLARE(void)			UpdateInvert();														//!< Updates viewInvert and projectionInvert from view and projection
	MFUNC3(void)				Translate(const C0&x, const C1&y, const C2&z);						//!< Moves the aspect depending on the current view orientation. Auto updates viewInvert. \param x Translation in X-direction (horizontal) \param y Translation in Y-direction (vertical) \param Translation in Z-direction (depth)
	MFUNC1(void)				Translate(const M::TVec3<C0>&delta);										//!< Moves the aspect depending on the current view orientation. Auto updates viewInvert. \param delta 3 component movement vector (x,y,z)
	MFUNC4(void)				Rotate(const C0&angle, const C1&x, const C2&y, const C3&z);			//!< Rotates the view matrix about an arbitrary axis. The axis components are not required to form a normalized vector. Auto updates viewInvert. \param Angle to Rotate by (0...360) \param x X-component of the rotation axis \param y Y-component of the rotation axis. \param z Z-component of the rotation axis
	MFUNC2(void)				Rotate(const C0&angle, const M::TVec3<C1>&v);								//!< Rotates the view matrix about an arbitrary axis. The axis is not required to form a normalized vector. Auto updates viewInvert. \param Angle to Rotate by (0...360) \param v Rotation axis (x,y,z)
	MFUNC1(void)				Rotate(const M::TVec4<C0>&quaternion);										//!< Rotates the view matrix via an arbitrary quaternion. Auto updates viewInvert. \param quaternion Quaternion to use for the rotation. The quaternion is required to be normalized.
	MFUNC2(bool)				PointToScreen(const M::TVec3<C0>&point, M::TVec2<C1>&screenPoint)				const;	//!< Projects a 3d point to 2d screen coordinates (using both view and projection transformations). \param point Point to transform \param screenPoint 2 component out vector to store the final screen position in. \return true if the projected point lies within the projection depth range, false otherwise.
	MFUNC2(bool)				VectorToScreen(const M::TVec3<C0>&vector, M::TVec2<C1>&screenPoint)			const;	//!< Projects a 3d vector to 2d screen coordinates (using both view and projection transformations). The vector is rotated and projected but not Translated. \param vector Vector to Project \param screenPoint 2 component out vector to store the final screen position in. \return true if the vector could be projected.
	MFUNC3(void)				ScreenToVector(const M::TVec2<C0>&point, M::TVec3<C1>&position, M::TVec3<C2>&direction)	const;	//!< Reverse projects a point on the screen to a point and a direction vector. \param point Screen point to reverse Project. \param position 3 component out vector to write the final point coordinates to. \param direction 3 component out vector to write the final viewing direction to.
	MFUNC4(void)				ScreenToVector(const C0&x, const C1&y, M::TVec3<C2>&position, M::TVec3<C3>&direction)	const;	//!< Reverse projects a point on the screen to a point and a direction vector. \param x Screen point(x) to reverse Project. \param y Screen point(y) to reverse Project. \param position 3 component out vector to write the final point coordinates to. \param direction 3 component out vector to write the final viewing direction to.
	MFUNC2(bool)				Project(const M::TVec3<C0>&point, M::TVec3<C1>&projected)							const;	//!< Transforms the specified point and projectes it to the screen.
	MFUNC2(bool)				ProjectToRegion(const M::TVec3<C0>&point, M::TVec3<C1>&projected)					const;	//!< Executes Project() and transforms the result (if any) out of the current region. The resulting screen range is still [-1,-1,-1],[1,1,1]. Note that z-values are not affected by the regional projection
	MFUNC2(void)				ReverseProject(const M::TVec3<C0>&point, M::TVec3<C1>&position)					const;	//!< Reverse projects a point on the screen to its point of origin. \param point 3 component screen point to reverse Project.\param position 3 component out vector to write the final point coordinates to. 
	MF_DECLARE(const Volume&)	ResolveVolume() 													const;	//!< Extracts a visual volume from the current matrix content. \return Const reference to a global visual volume object that contains the local visual volume.
	MF_DECLARE(const Volume&)	ResolveFrustum()													const;	//!< Identical to ResolveVolume()
	MF_DECLARE(const Volume&)	GetFrustum()														const;	//!< Identical to ResolveVolume()
	MF_DECLARE(void)			ResolveVolume(Volume&v) 											const;	//!< Extracts a visual volume from the current matrix content. @param v [out] Reference to a global visual volume object that should contain the local visual volume.
	MF_DECLARE(void)			ResolveFrustum(Volume&v)											const;	//!< Identical to ResolveVolume()
	MF_DECLARE(void)			GetFrustum(Volume&v)												const;	//!< Identical to ResolveVolume()
	template <typename SubType>
		MF_DECLARE(void)		ResolveVolumeST(Volume&v)											const;	//!< Similar however potential more precise version of ResolveVolume(). Normal calculation is done using the specified sub type instead (double recommended here)
	MF_DECLARE(String)			ToString()															const;	//!< Creates a string representation of the local aspect. \return String representation.
	MF_DECLARE(const M::TVec3<C>&)	GetAbsoluteLocation()													const;	//!< Returns the absolute realworld coordinates of the local aspect object
	MF_DECLARE(void)			LoadIdentity();														//!< Resets all matrices and vectors to identity. New aspects automatically load identity into every matrix.

	/**
	Calculates the normalized looking direction.
	Points along the central direction the camera is looking.
	Simple negation of the view invert z axis.

	@return Forward viewing direction (inherently normalized)
	*/
	MF_DECLARE(M::TVec3<C>)		GetForwardDirection()		const	{M::TVec3<C> rs = {-viewInvert.z.x,-viewInvert.z.y,-viewInvert.z.z}; return rs;}

};




template <class C=float>
class	OrthographicAspect:public Aspect<C>	//! Orthographic aspect for non central projecting viewports
{
public:
	typedef Aspect<C> Super;

	using	Super::view;
	using	Super::viewInvert;
	using	Super::projection;
	using	Super::projectionInvert;
	using	Super::region;
	using	Super::location;
	using	Super::depthTest;

	M::TMatrix3<C>			orientation;
		
	MF_CONSTRUCTOR			OrthographicAspect(VisualEnum::eDepthTest depthTest = VisualEnum::NoDepthTest);

MF_DECLARE(void)			UpdateProjection(const C&pixelAspect, const C&zoom, const C&zNear, const C&zFar); 							//!< Create the local projection matrix via aspect, zoom, znear and zfar. Also updates the projection invert. \param pixelAspect Global pixel aspect. Should not contain the local region aspect. \param zoom Aspect zoom. \param zNear Near z plane. \param zFar Far z plane.
MF_DECLARE(void)			UpdateProjection(const C&left, const C&bottom, const C&right, const C&top,const C&zNear, const C&zFar);	//!< Create the local projection matrix via borders. Also updates the projection invert. \param left Left border. \param bottom Bottom border. \param right Right border. \param top Top border. \param zNear Near z plane. \param zFar Far z plane.
MFUNC1 (void)				UpdateProjection(const M::Rect<C0>&area, const C&zNear, const C&zFar);											//!< Create the local projection matrix via borders. Also updates the projection invert. \param area Planar viewing area \param zNear Near z plane. \param zFar Far z plane.
MF_DECLARE(void)			UpdateProjection2(const C&left, const C&bottom, const C&right, const C&top,const C&zNear, const C&zFar);	//!< DX11 variance
MFUNC1 (void)				UpdateProjection2(const M::Rect<C0>&area, const C&zNear, const C&zFar);											//!< DX11 variance
MFUNC2 (void)				AlterDepthRange(const C0&zNear, const C1&zFar);														//!< Changes the depth range of the local projection matrix. Also updates the projection invert. \param zNear New near z plane. \param zFar New far z plane.
MF_DECLARE(void)			UpdateView();																							//!< (Re)assembles the view matrix using the local orientation matrix. Also updates the view invert.
MF_DECLARE(void)			UpdateScaledView();																					//!< Identical to UpdateView() except that it allows orientation to consist of non-normalized vectors.
};


template <class C=float>
class	Camera:public Aspect<C>	//! Generalized camera object
{
public:
	typedef Aspect<C> Super;

		using	Super::view;
		using	Super::viewInvert;
		using	Super::projection;
		using	Super::projectionInvert;
		using	Super::region;
		using	Super::location;
		using	Super::depthTest;

		M::TVec3<C>			retraction;			//!< Retraction vector. The retraction causes the camera to move away from its control center. Use negative z-coordinates for a 3rd person like camera.
		M::TMatrix3<C>		orientation,
							system;				//!< 3x3 system matrix. This matrix is not needed in the local camera object but by all derived camera objects.
		C					vfov;					//!< Vertical field of view (in degrees).



MF_CONSTRUCTOR				Camera();
MF_DECLARE(void)			UpdateProjection(const C&pixelAspect, const C&zNear=0.1, const C&zFar=100, const C&vFov=45);	//!< Create the local projection matrix via aspect, znear, zfar and vertical field of view. Also updates the projection invert. \param pixelAspect Global pixel aspect. Should not contain the local region aspect. \param zNear Near z plane. \param zFar Far z plane. \param vFov Vertical field of view in degrees.
MF_DECLARE(void)			UpdateProjection2(const C&pixelAspect, const C&zNear=0.1, const C&zFar=100, const C&vFov=45);	//!< Create the local projection matrix via aspect, znear, zfar and vertical field of view. Also updates the projection invert. \param pixelAspect Global pixel aspect. Should not contain the local region aspect. \param zNear Near z plane. \param zFar Far z plane. \param vFov Vertical field of view in degrees.
/**
Internally calls UpdateProjection2(), then modifies the resulting projection matrix such that only the specified window
is actually rendered. This feature can be used to render extremely high resolution images in iterative segments
@param left Left window edge coordinate in [0,1]
@param bottom Bottom window edge coordinate in [0,1]
@param width Horizontal window size in [0,1]
@param height Vertical window size in [0,1]
*/
MF_DECLARE(void)			UpdateProjectionSegment(const C&pixelAspect, const C&left, const C&bottom, const C&width, const C&height, const C&zNear=0.1, const C&zFar=100, const C&vFov=45);
MFUNC2	(void)				AlterDepthRange(const C0&zNear, const C1&zFar);											//!< Changes the depth range of the local projection matrix. Also updates the projection invert. \param zNear New near z plane. \param zFar New far z plane.
MF_DECLARE(void)			UpdateView();																				//!< (Re)assembles the view matrix using the local orientation matrix. Also updates the view invert.
MFUNC3	(void)				Translate(const C0&x, const C1&y, const C2&z);						//!< Moves the camera depending on its current orientation. Auto updates viewInvert. \param x Translation in X-direction (horizontal) \param y Translation in Y-direction (vertical) \param Translation in Z-direction (depth)
MFUNC1	(void)				Translate(const M::TVec3<C0>&delta);										//!< Moves the camera depending on its current orientation. Auto updates viewInvert. \param delta 3 component movement vector (x,y,z)
MFUNC3	(void)				SetPosition(const C0&x, const C1&y, const C2&z);							//!< Redefines the camera center and rebuilds. \param x Camera x-coordinate \param y Camera y-coordinate \param z Camera z-coordinate
MFUNC1	(void)				SetPosition(const M::TVec3<C0>&position);										//!< Redefines the camera center and rebuilds. \param position New camera position (x,y,z)
MFUNC2	(void) 				ExtractDepthRange(C0&zNear, C1&zFar)	const;						//!< Extracts the zNear and zFar values from the projection matrix
};

template <class C=float>
class	AngularCamera:public Camera<C>	//! Camera object using angles to determine the viewing direction
{
public:
	typedef Camera<C> Super;

	using	Super::view;
	using	Super::viewInvert;
	using	Super::projection;
	using	Super::projectionInvert;
	using	Super::region;
	using	Super::location;
	using	Super::depthTest;
		
	using	Super::retraction;
	using	Super::vfov;
	using	Super::orientation;
	using	Super::system;

	M::TVec3<C>			angle;			//!< Current angles about the x, y, and z axes (in that order).

MF_CONSTRUCTOR				AngularCamera();
MF_DECLARE(void)			UpdateView();													//!< (Re)assembles the view matrix using the local angles. Also updates the view invert.
MFUNC3	(void)				TranslatePlanar(const C0&x, const C1&y, const C2&z);		//!< Moves the camera depending on its current viewing direction. The angle around the x-axis is ignored. Auto updates viewInvert. \param x Translation in X-direction (horizontal) \param y Translation in Y-direction (vertical) \param Translation in Z-direction (depth)
MFUNC1	(void)				TranslatePlanar(const M::TVec3<C0>&delta);					//!< Moves the camera depending on its current viewing direction. The angle around the x-axis is ignored. Auto updates viewInvert. \param delta 3 component movement vector (x,y,z)
MFUNC3	(void)				AlterAngles(const C0&alpha, const C1&beta, const C2&gamma);	//!< Alters the local viewing angles and rebuilds. \param alpha Alpha angle (about the x-axis) delta. \param beta Beta angle (around the y-axis) delta. \param gamma Gamma angle (around the z-axis) delta.
MFUNC1	(void)				AlterAngles(const M::TVec3<C0>&delta);						//!< Alters the local viewing angles and rebuilds. \param delta 3 component angle delta vector (alpha, beta, gamma).
MFUNC1	(void)				LookAtPlanar(const M::TVec3<C0>&point);						//!< Rotates the camera towards the specified point. Does not alter the angle around the x-axis. \param point 3d point to look at.
MFUNC1	(void)				LookAt(const M::TVec3<C0>&point);							//!< Rotates the camera towards the specified point. \param point 3d point to look at.
MF_DECLARE(VectorCamera<C>)	ToVectorCamera();											//!< Converts the local angular camera to a vector camera. \return Vector camera object matching the local position and orientation.
};


template <class C=float>
class	VectorCamera:public Camera<C>	//! Camera object using vectors to determine the viewing direction
{
	typedef Camera<C> Super;
private:
	MF_DECLARE(void)	AdjustUpAxis();

public:
	using	Super::view;
	using	Super::viewInvert;
	using	Super::projection;
	using	Super::projectionInvert;
	using	Super::region;
	using	Super::location;
	using	Super::depthTest;
		
	using	Super::retraction;
	using	Super::vfov;
	using	Super::orientation;
	using	Super::system;

		M::TVec3<C>			direction,				//!< Current viewing direction (points backwards)
							upAxis;					//!< Vertical orientation vector
		M::TMatrix3<C>		vsystem;				//!< Vector system matrix

MF_CONSTRUCTOR				VectorCamera();
MF_CONSTRUCTOR				VectorCamera(const Camera<C>&);
MF_DECLARE(void)			UpdateView();													//!< (Re)assembles the view matrix using the local vectors. Also updates the view invert.
MFUNC3(void)				TranslatePlanar(const C0&x, const C1&y, const C2&z);		//!< Moves the camera depending on its current viewing direction. Any up or down orientation of the direction vector is ignored. Auto updates viewInvert. \param x Translation in X-direction (horizontal) \param y Translation in Y-direction (upAxis) \param Translation in Z-direction (depth)
MFUNC1(void)				TranslatePlanar(const M::TVec3<C0>&delta);							//!< Moves the camera depending on its current viewing direction. Any up or down orientation of the direction vector is ignored. Auto updates viewInvert. \param delta Translation delta vector
MFUNC2(void)				RotatePlanar(const C0&alpha, const C1&beta);				//!< Rotates the local camera object to the left/right(beta) and up/down(alpha) and rebuilds. \param alpha Angle to Rotate up or down  \param beta Angle to Rotate about the vertical axis (to the left/right)
MFUNC1(void)				RotateDirectional(const C0&alpha, bool rebuild=true);		//!< Rotates the local camera object clockwise around the current viewing direction and rebuilds. \param alpha Angle to Rotate about the viewing axis \param rebuild Rebuild the local camera object after rotation
MFUNC1(void)				SetDirection(const M::TVec3<C0>&dir,bool doUpdate=true);			//!< Modifies the direction vector to match the specified one and rebuilds. \param dir New direction vector
MFUNC1(void)				SetUpAxis(const M::TVec3<C0>&vert,bool doUpdate=true);			//!< Modifies the vertical vector to match the specified one and rebuilds. \param vert New vertical orientation vector.
MFUNC2(void)				Align(const M::TVec3<C0>&dir, const M::TVec3<C1>&vert);					//!< Modifies both the direction and vertical vectors and rebuilds.  \param dir New direction vector \param vert New vertical orientation vector.
MF_DECLARE(AngularCamera<C>)ToAngularCamera();											//!< Converts the local vector camera to an angular camera. <b>Not implemented yet. Do not use</b>
MF_DECLARE(const VectorCamera<C>&)	ToVectorCamera() const {return *this;};
};

}

#include "aspect.tpl.h"


#endif
