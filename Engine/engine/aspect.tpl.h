#ifndef engine_aspectTplH
#define engine_aspectTplH

/******************************************************************

Camera-structures (in general aspects) to simplify usage of
different types of cameras.

******************************************************************/

#include <math/object.h>

namespace Engine
{

	template <typename C>
	/*static*/ const Aspect<C>		Aspect<C>::identity;

template <class C>
MFUNC2 (bool) Frustum<C>::visible(const M::TVec3<C0>&center, const C1&radius) const
{
	return	(M::Vec::planePointDistanceN(near_bottom_left,bottom_normal,center)<radius)
			&&
			(M::Vec::planePointDistanceN(near_bottom_right,right_normal,center)<radius)
			&&
			(M::Vec::planePointDistanceN(near_top_right,top_normal,center)<radius)
			&&
			(M::Vec::planePointDistanceN(near_top_left,left_normal,center)<radius)
			&&
			(M::Vec::planePointDistanceN(near_bottom_left,near_normal,center)<radius)
			&&
			(M::Vec::planePointDistanceN(far_bottom_left,far_normal,center)<radius);
}

template <class C>
	MFUNC2 (bool) Frustum<C>::IntersectsCone(const M::TVec3<C0>&center, const C1&radius) const
		{
			return	(M::Vec::planePointDistanceN(near_bottom_left,bottom_normal,center)<radius)
					&&
					(M::Vec::planePointDistanceN(near_bottom_right,right_normal,center)<radius)
					&&
					(M::Vec::planePointDistanceN(near_top_right,top_normal,center)<radius)
					&&
					(M::Vec::planePointDistanceN(near_top_left,left_normal,center)<radius);
		}


template <class C> MFUNC2 (typename Frustum<C>::Visibility) Frustum<C>::CheckSphereVisibility(const M::TVec3<C0>&center, const C1&radius) const
	{
		const M::TVec3<C>* base[6] =
			{
				&near_bottom_left,
				&far_bottom_left,
				&near_bottom_left,
				&near_bottom_right,
				&near_top_right,
				&near_top_left
			};
		bool fully_visible = true;
		for (BYTE k = 0; k < 6; k++)
		{
			C0 dist = M::Vec::planePointDistance(* (base[k]),normal[k],center);
			if (dist > -radius)
				fully_visible = false;
			if (dist > radius)
				return NotVisible;
		}
		return fully_visible?FullyVisible:PartiallyVisible;
	}


template <class C> MFUNC3 (bool) Frustum<C>::visible(const C0&x, const C1&y, const C2&z) const
	{
		M::TVec3<C0> center={x,y,z};
		C error = M::GetError<C>();
		return	(M::Vec::planePointDistanceN(near_bottom_left,bottom_normal,center)<=error)
				&&
				(M::Vec::planePointDistanceN(near_bottom_right,right_normal,center)<=error)
				&&
				(M::Vec::planePointDistanceN(near_top_right,top_normal,center)<=error)
				&&
				(M::Vec::planePointDistanceN(near_top_left,left_normal,center)<=error)
				&&
				(M::Vec::planePointDistanceN(near_bottom_left,near_normal,center)<=error)
				&&
				(M::Vec::planePointDistanceN(far_bottom_left,far_normal,center)<=error);
	}
	
	
	
template <class C>
	/*static*/ MFUNC1(bool) Frustum<C>::TestTriangle(const M::TVec3<C0>&p0,const M::TVec3<C0>&p1,const M::TVec3<C0>&p2,const M::TVec3<C0>&e0,const M::TVec3<C0>&d)
	{
		M::TVec3<C0> rs;
		return
			M::Obj::detTriangleRayIntersection(p0, p1, p2, e0, d, rs)
			&& 
			rs.z >= -M::GetError<C0>() 
			&& 
			rs.z <= (C0)1+M::GetError<C0>() 
			&& 
			rs.x >= M::GetError<C0>() 
			&& 
			rs.y >= M::GetError<C0>() 
			&& 
			rs.x+rs.y <= (C0)1+M::GetError<C0>();
	}

template <class C>
	MFUNC1 (bool) Frustum<C>::intersects(const C0&x0, const C0&y0, const C0&z0,const C0&x1, const C0&y1, const C0&z1,const M::TVec3<C>&p0, const M::TVec3<C>&p1, const M::TVec3<C>&p2, const M::TVec3<C>&p3)
	{
		M::TVec3<C0>	e0 = {x0,y0,z0},
//						e1 = {x1,y1,z1},
						d = {x1-x0,y1-y0,z1-z0};
						
		return TestTriangle(p0,p1,p2,e0,d) || TestTriangle(p0,p2,p3,e0,d);
	}
	

template <class C> MFUNC1 (bool) Frustum<C>::intersects(const M::Box<C0>&box, const M::TVec3<C>&p0, const M::TVec3<C>&p1, const M::TVec3<C>&p2, const M::TVec3<C>&p3)
	{
		return	intersects(box.x.min,box.y.min,box.z.min,box.x.max,box.y.min,box.z.min,p0,p1,p2,p3)
				||
				intersects(box.x.min,box.y.min,box.z.min,box.x.min,box.y.max,box.z.min,p0,p1,p2,p3)
				||
				intersects(box.x.min,box.y.min,box.z.min,box.x.min,box.y.min,box.z.max,p0,p1,p2,p3)
				||
				intersects(box.x.max,box.y.min,box.z.min,box.x.max,box.y.max,box.z.min,p0,p1,p2,p3)
				||
				intersects(box.x.max,box.y.min,box.z.min,box.x.max,box.y.min,box.z.max,p0,p1,p2,p3)
				||
				intersects(box.x.min,box.y.max,box.z.min,box.x.min,box.y.max,box.z.max,p0,p1,p2,p3)
				||
				intersects(box.x.min,box.y.max,box.z.min,box.x.max,box.y.max,box.z.min,p0,p1,p2,p3)
				||
				intersects(box.x.min,box.y.min,box.z.max,box.x.min,box.y.max,box.z.max,p0,p1,p2,p3)
				||
				intersects(box.x.min,box.y.min,box.z.max,box.x.max,box.y.min,box.z.max,p0,p1,p2,p3)
				||
				intersects(box.x.min,box.y.max,box.z.max,box.x.max,box.y.max,box.z.max,p0,p1,p2,p3)
				||
				intersects(box.x.max,box.y.min,box.z.max,box.x.max,box.y.max,box.z.max,p0,p1,p2,p3)
				||
				intersects(box.x.max,box.y.max,box.z.min,box.x.max,box.y.max,box.z.max,p0,p1,p2,p3);
	}


template <class C> 
	MFUNC1 (bool) Frustum<C>::IsVisible_UnpreciseExpensiveDoNotUse(const M::Box<C0>&box)	const
	{
		return CheckBoxVisibility_UnpreciseExpensiveDoNotUse(box) != NotVisible;
	}

	
template <class C>
	MF_DECLARE (typename Frustum<C>::Visibility) Frustum<C>::CheckBoxVisibility_UnpreciseExpensiveDoNotUse(const M::Box<C>&box)	const
	{
		bool all_in_box=true;
		for (BYTE k = 0; k < 8; k++)
		{
			all_in_box = box.Contains(corner[k]);
			//!M::Vec::oneLess(corner[k],box.axis[k].min) && !M::Vec::oneGreater(corner[k],box.axis[k].max);
			if (!all_in_box)
				break;
		}
		if (all_in_box)
			return PartiallyVisible;
		if (
			visible(box.x.min,box.y.min,box.z.min)
			&&
			visible(box.x.min,box.y.min,box.z.max)
			&&
			visible(box.x.min,box.y.max,box.z.max)
			&&
			visible(box.x.min,box.y.max,box.z.min)
			&&
			visible(box.x.max,box.y.max,box.z.max)
			&&
			visible(box.x.max,box.y.max,box.z.min)
			&&
			visible(box.x.max,box.y.min,box.z.min)
			&&
			visible(box.x.max,box.y.min,box.z.max))
			return FullyVisible;
		
		// tested for box in frustum and frustum in box. now to check for other intersection. if no intersecion occurs then the box is invisible
		
		//test if edges of the frustum intersect box faces:
		
		//Vec(lower_corner,box);
		//_c3(upper_corner,box+3);
		
		if (_oIntersectsBox(near_bottom_left,near_bottom_right,box))
			return PartiallyVisible;
		if (_oIntersectsBox(near_bottom_left,near_top_left,box))
			return PartiallyVisible;
		if (_oIntersectsBox(near_top_left,near_top_right,box))
			return PartiallyVisible;
		if (_oIntersectsBox(near_top_right,near_bottom_right,box))
			return PartiallyVisible;
			
		if (_oIntersectsBox(far_bottom_left,far_bottom_right,box))
			return PartiallyVisible;
		if (_oIntersectsBox(far_bottom_left,far_top_left,box))
			return PartiallyVisible;
		if (_oIntersectsBox(far_top_left,far_top_right,box))
			return PartiallyVisible;
		if (_oIntersectsBox(far_top_right,far_bottom_right,box))
			return PartiallyVisible;
			
		if (_oIntersectsBox(far_top_right,near_top_right,box))
			return PartiallyVisible;
		if (_oIntersectsBox(far_top_left,near_top_left,box))
			return PartiallyVisible;
		if (_oIntersectsBox(far_bottom_left,near_bottom_left,box))
			return PartiallyVisible;
		if (_oIntersectsBox(far_bottom_right,near_bottom_right,box))
			return PartiallyVisible;
			
		if (intersects(box,near_bottom_left,near_bottom_right,near_top_right,near_top_left))
			return PartiallyVisible;
		if (intersects(box,far_bottom_left,far_bottom_right,far_top_right,far_top_left))
			return PartiallyVisible;
		if (intersects(box,far_bottom_left,far_bottom_right,near_bottom_right,near_bottom_left))
			return PartiallyVisible;
		if (intersects(box,far_bottom_left,far_top_left,near_top_left,near_bottom_left))
			return PartiallyVisible;
		if (intersects(box,far_top_left,far_top_right,near_top_right,near_top_left))
			return PartiallyVisible;
		if (intersects(box,far_top_right,far_bottom_right,near_bottom_right,near_top_right))
			return PartiallyVisible;
		
		return NotVisible;
	}
	


template <class C>
	MF_DECLARE(String)		Frustum<C>::ToString()	const
	{
		String rs = "volume:\r\nnear: "+M::Vec::toString(near_bottom_left)+", "+M::Vec::toString(near_bottom_right)+", "+M::Vec::toString(near_top_right)+", "+M::Vec::toString(near_top_left)+"\r\n";
		rs += "far: "+M::Vec::toString(far_bottom_left)+", "+M::Vec::toString(far_bottom_right)+", "+M::Vec::toString(far_top_right)+", "+M::Vec::toString(far_top_left)+"\r\n";
		rs += "normals: left="+M::Vec::toString(left_normal)+", right="+M::Vec::toString(right_normal)+", bottom="+M::Vec::toString(bottom_normal)+", top="+M::Vec::toString(top_normal);
		return rs;
	}

template <class C>
	MFUNC2 (bool)	Frustum<C>::IsVisible(const M::TVec3<C0>&center, const C1&radius)	const
	{
		return visible(center,radius);
	}


template <class C>
	MF_DECLARE(void) Frustum<C>::defNormal(const M::TVec3<C>&p0, const M::TVec3<C>&p1, const M::TVec3<C>&p2, M::TVec3<C>&out)
	{
		M::TVec3<C> edge0,edge1;
		M::Vec::sub(p1,p0,edge0);
		M::Vec::sub(p2,p0,edge1);
		M::Vec::cross(edge0,edge1,out);
		M::Vec::normalize0(out);
	}

template <class C>
	template <typename SubType>
		MF_DECLARE (void) Frustum<C>::defNormalST(const M::TVec3<C>&p0, const M::TVec3<C>&p1, const M::TVec3<C>&p2, M::TVec3<C>&out)
		{
			M::TVec3<SubType>	edge0,edge1,tmp;
			M::Vec::sub(p1,p0,edge0);
			M::Vec::sub(p2,p0,edge1);
			M::Vec::cross(edge0,edge1,tmp);
			M::Vec::normalize0(tmp);
			M::Vec::copy(tmp,out);
		}
template <class C>
	MF_DECLARE(void)	Frustum<C>::FlipNormals()
	{
		M::Vec::mult(normal[0],-1);
		M::Vec::mult(normal[1],-1);
		M::Vec::mult(normal[2],-1);
		M::Vec::mult(normal[3],-1);
		M::Vec::mult(normal[4],-1);
		M::Vec::mult(normal[5],-1);


	}

template <class C>
	MF_DECLARE(void) Frustum<C>::UpdateNormals()
	{
		defNormal(corner[0],corner[4],corner[1],bottom_normal);
		defNormal(corner[1],corner[5],corner[2],right_normal);
		defNormal(corner[2],corner[6],corner[3],top_normal);
		defNormal(corner[3],corner[7],corner[4],left_normal);
		//defNormal(corner[0],corner[1],corner[2],near_normal);	//near field may be too small
		{
			M::TVec3<C>	far_center = far[0],
						near_center = near[0];

			M::Vec::add(near_center,near[1]);
			M::Vec::add(near_center,near[2]);
			M::Vec::add(near_center,near[3]);
			M::Vec::div(near_center,(C)4);
	
			M::Vec::add(far_center,far[1]);
			M::Vec::add(far_center,far[2]);
			M::Vec::add(far_center,far[3]);
			M::Vec::div(far_center,(C)4);
	
			M::Vec::sub(near_center,far_center,near_normal);
			M::Vec::normalize0(near_normal);
		}
	
		defNormal(corner[7],corner[6],corner[5],far_normal);
	}

template <class C>
	template <typename SubType>
		MF_DECLARE(void) Frustum<C>::UpdateNormalsST()
		{
			defNormalST<SubType>(corner[0],corner[4],corner[1],bottom_normal);
			defNormalST<SubType>(corner[1],corner[5],corner[2],right_normal);
			defNormalST<SubType>(corner[2],corner[6],corner[3],top_normal);
			defNormalST<SubType>(corner[3],corner[7],corner[4],left_normal);
			defNormalST<SubType>(corner[0],corner[1],corner[2],near_normal);
			defNormalST<SubType>(corner[7],corner[6],corner[5],far_normal);
		}

template <class C>
	MF_DECLARE(void) Frustum<C>::UpdateNormalsP()
	{
		defNormal(corner[0],corner[4],corner[1],bottom_normal);
		defNormal(corner[1],corner[5],corner[2],right_normal);
		defNormal(corner[2],corner[6],corner[3],top_normal);
		defNormal(corner[3],corner[7],corner[4],left_normal);
		defNormal(corner[0],corner[1],corner[2],near_normal);
		defNormal(corner[7],corner[6],corner[5],far_normal);
	}

template <class C>
	MFUNC1(Frustum<C>&)	Frustum<C>::operator=(const Frustum<C0>&other)
		{
			for (BYTE k = 0; k < 8; k++)
				M::Vec::copy(other.corner[k],corner[k]);
			for (BYTE k = 0; k < 6; k++)
				M::Vec::copy(other.normal[k],normal[k]);
			return *this;
		}


template <class C> MF_CONSTRUCTOR Aspect<C>::Aspect():region(0,0,1,1),depthTest(NormalDepthTest)
{
	LoadIdentity();
}

template <class C> MF_CONSTRUCTOR Aspect<C>::Aspect(VisualEnum::eDepthTest dtest):region(0,0,1,1),depthTest(dtest)
{
	LoadIdentity();
}


template <class C> MF_DECLARE(void)	Aspect<C>::LoadIdentity()
{
	view = M::Matrix<C>::eye4;
	viewInvert = M::Matrix<C>::eye4;
	projectionInvert = M::Matrix<C>::eye4;
	projection = M::Matrix<C>::eye4;
	M::Vec::clear(location);
}

template <class C> MF_DECLARE(void) Aspect<C>::UpdateInvert()
{
	M::Mat::invertSystem(view,viewInvert);
	M::Mat::invert(projection,projectionInvert);
}

template <class C> MF_DECLARE(const M::TVec3<C>&) Aspect<C>::GetAbsoluteLocation()	const
{
	return viewInvert.w.xyz;
}


template <class C>
MFUNC3 (void) Aspect<C>::Translate(const C0&x, const C1&y, const C2&z)
{
	M::TVec3<C> vec={x,y,z},out;
	M::Mat::rotate(view,vec,out);
	M::Vec::add(ref3(view+12),out);
	M::Mat::invertSystem(view,viewInvert);
}


template <class C>
MFUNC1 (void) Aspect<C>::Translate(const M::TVec3<C0>&delta)
{
	M::TVec3<C> out;
	M::Mat::rotate(view,delta,out);
	M::Vec::add(ref3(view+12),out);
	M::Mat::invertSystem(view,viewInvert);
}

template <class C>
MFUNC4 (void) Aspect<C>::Rotate(const C0&angle, const C1&x, const C2&y, const C3&z)
{
	M::TVec4<C> a = {x,y,z,angle},q[4];
	M::Vec::normalize0(a);	//normalize first 3 components
	_q2Quaternion(a,q);
	Rotate(q);
}

template <class C>
MFUNC2 (void) Aspect<C>::Rotate(const C0&angle, const M::TVec3<C1>&v)
{
	M::TVec4<C> a = {v.x,v.y,v.z,angle},q[4];
	M::Vec::normalize0(a);	//normalize first 3 components
	_q2Quaternion(a,q);
	Rotate(q);
}

template <class C>
MFUNC1 (void) Aspect<C>::Rotate(const M::TVec4<C0>&quaternion)
{
	_qRotateSystemCW(quaternion,view);
	M::Mat::invertSystem(view,viewInvert);
}

template <class C>
MFUNC2 (bool) Aspect<C>::PointToScreen(const M::TVec3<C0>&point, M::TVec2<C1>&screenPoint)	const
{
	M::TVec4<C>	temp0,temp1;
	M::Mat::Transform(view,point,temp0.xyz);
	temp0.w = 1;
	M::Mat::Mult(projection,temp0,temp1);
	using std::fabs;
	if (fabs(temp1.w) <= TypeInfo<C>::error)
		return false;
	M::Vec::divide(temp1.xy,temp1.w,screenPoint);
	return temp1.z >= -temp1.w && temp1.z <= temp1.w;	//[-1,+1]
}
template <class C>
MFUNC2 (bool) Aspect<C>::Project(const M::TVec3<C0>&point, M::TVec3<C1>&projected)	const
{
	M::TVec4<C>	temp0,temp1;
	M::Mat::Transform(view,point,temp0.xyz);
	temp0.w = 1;
	M::Mat::Mult(projection,temp0,temp1);
	using std::fabs;
	if (fabs(temp1.w) < M::GetError<C>())
		return false;
	M::Vec::divide(temp1.xyz,temp1.w,projected);
	return projected.z >= -1 && projected.z <= 1;
}

template <class C>
MFUNC2 (bool) Aspect<C>::ProjectToRegion(const M::TVec3<C0>&point, M::TVec3<C1>&projected)	const
{
	bool rs = Project(point,projected);
	region.Derelativate(C(projected.x * (C1)0.5+(C1)0.5),C(projected.y * (C1)0.5+(C1)0.5),projected.x,projected.y);
	projected.x = projected.x * (C1)2-(C1)1;
	projected.y = projected.y * (C1)2-(C1)1;
	return rs;
}

template <class C>
MFUNC2 (bool) Aspect<C>::VectorToScreen(const M::TVec3<C0>&vector, M::TVec2<C1>&screenPoint)	const
{
	M::TVec4<C>	temp0,temp1;
	M::Mat::rotate(view,vector,temp0.xyz);
	temp0.w = 1;
	M::Mat::Mult(projection,temp0,temp1);
	if (vabs(temp1.w) < GetError<C>())
		return false;
	M::Vec::divide(temp1.xy,temp1.w,screenPoint);
	return temp1.w >= 0;
}

template <class C>
	MFUNC4 (void)				Aspect<C>::ScreenToVector(const C0&x, const C1&y, M::TVec3<C2>&position, M::TVec3<C3>&direction)	const
		{
			M::TVec2<C>	point = {x,y};
			ScreenToVector(point,position,direction);
		}

template <class C>
MFUNC3 (void) Aspect<C>::ScreenToVector(const M::TVec2<C0>&point, M::TVec3<C1>&position, M::TVec3<C2>&direction)	const
{
	//_c3(&viewInvert[12],position);
	M::TVec4<C>	p0 = {point.x,point.y,-1,1},
				temp;
	M::Mat::Mult(projectionInvert,p0,temp);
	using std::fabs;
	if (fabs(temp.w) > M::GetError<C>())
		M::Vec::div(temp.xyz,temp.w);
	M::Mat::rotate(viewInvert,temp.xyz,direction);
	M::Mat::Transform(viewInvert,temp.xyz,position);
}

template <class C>
MFUNC2 (void)				Aspect<C>::ReverseProject(const M::TVec3<C0>&point, M::TVec3<C1>&position)					const
	{
		M::TVec4<C>		p0 = {point.x,point.y,point.z,1},
						temp;
		M::Mat::Mult(projectionInvert,p0,temp);
		if (vabs(temp.w) > GetError<C>())
			M::Vec::div(temp.xyz,temp.w);
		M::Mat::Transform(viewInvert,temp.xyz,position);
	}


template <class C>
	MF_DECLARE (const Frustum<C>&)	Aspect<C>::ResolveFrustum() const
	{
		return ResolveVolume();
	}

template <class C>
	MF_DECLARE (const Frustum<C>&)	Aspect<C>::GetFrustum() const
	{
		return ResolveVolume();
	}
	

template <class C>
	MF_DECLARE (const Frustum<C>&) Aspect<C>::ResolveVolume() const
	{
		ResolveVolume(result);
		return result;
	}


template <class C>
	MF_DECLARE (void) Aspect<C>::ResolveFrustum(Volume&result) const
	{
		ResolveVolume(result);
	}

template <class C>
	MF_DECLARE (void) Aspect<C>::GetFrustum(Volume&result) const
	{
		ResolveVolume(result);
	}

template <class C>
	template <typename SubType>
		MF_DECLARE (void) Aspect<C>::ResolveVolumeST(Volume&result_) const
		{
			Frustum<SubType>	result;

			M::TVec4<SubType>	temp;

			M::TVec4<SubType>	p = {-1,-1,-1,1};

							M::Mat::Mult(projectionInvert,p,temp);
							if (temp.w)
								M::Vec::divide(temp.xyz,temp.w);
							M::Mat::Transform(viewInvert,temp.xyz,result.corner[0]);
			M::Vec::def(p.xy,1,-1);
							M::Mat::Mult(projectionInvert,p,temp);
							if (temp.w)
								M::Vec::divide(temp.xyz,temp.w);
							M::Mat::Transform(viewInvert,temp.xyz,result.corner[1]);
			M::Vec::def(p.xy,1,1);
							M::Mat::Mult(projectionInvert,p,temp);
							if (temp.w)
								M::Vec::divide(temp.xyz,temp.w);
							M::Mat::Transform(viewInvert,temp.xyz,result.corner[2]);
			M::Vec::def(p.xy,-1,1);
							M::Mat::Mult(projectionInvert,p,temp);
							if (temp.w)
								M::Vec::divide(temp.xyz,temp.w);
							M::Mat::Transform(viewInvert,temp.xyz,result.corner[3]);


			M::Vec::def(p.xyz,-1,-1,1);
							M::Mat::Mult(projectionInvert,p,temp);
							if (temp.w)
								M::Vec::divide(temp.xyz,temp.w);
							M::Mat::Transform(viewInvert,temp.xyz,result.corner[4]);
			M::Vec::def(p.xy,1,-1);
							M::Mat::Mult(projectionInvert,p,temp);
							if (temp.w)
								M::Vec::divide(temp.xyz,temp.w);
							M::Mat::Transform(viewInvert,temp.xyz,result.corner[5]);
			M::Vec::def(p.xy,1,1);
							M::Mat::Mult(projectionInvert,p,temp);
							if (temp.w)
								M::Vec::divide(temp.xyz,temp.w);
							M::Mat::Transform(viewInvert,temp.xyz,result.corner[6]);
			M::Vec::def(p.xy,-1,1);
							M::Mat::Mult(projectionInvert,p,temp);
							if (temp.w)
								M::Vec::divide(temp.xyz,temp.w);
							M::Mat::Transform(viewInvert,temp.xyz,result.corner[7]);

			result.updateNormalsP();
			result_ = result;
		}

template <class C>
	MF_DECLARE (void) Aspect<C>::ResolveVolume(Volume&result) const
	{
		M::TVec4<C>	temp;

		M::TVec4<C>	p = {-1,-1,-1,1};
						M::Mat::Mult(projectionInvert,p,temp);
						if (temp.w)
							M::Vec::divide(temp.xyz,temp.w);
						M::Mat::Transform(viewInvert,temp.xyz,result.corner[0]);
		M::Vec::def(p.xy,1,-1);
						M::Mat::Mult(projectionInvert,p,temp);
						if (temp.w)
							M::Vec::divide(temp.xyz,temp.w);
						M::Mat::Transform(viewInvert,temp.xyz,result.corner[1]);
		M::Vec::def(p.xy,1,1);
						M::Mat::Mult(projectionInvert,p,temp);
						if (temp.w)
							M::Vec::divide(temp.xyz,temp.w);
						M::Mat::Transform(viewInvert,temp.xyz,result.corner[2]);
		M::Vec::def(p.xy,-1,1);
						M::Mat::Mult(projectionInvert,p,temp);
						if (temp.w)
							M::Vec::divide(temp.xyz,temp.w);
						M::Mat::Transform(viewInvert,temp.xyz,result.corner[3]);


		M::Vec::def(p.xyz,-1,-1,1);
						M::Mat::Mult(projectionInvert,p,temp);
						if (temp.w)
							M::Vec::divide(temp.xyz,temp.w);
						M::Mat::Transform(viewInvert,temp.xyz,result.corner[4]);
		M::Vec::def(p.xy,1,-1);
						M::Mat::Mult(projectionInvert,p,temp);
						if (temp.w)
							M::Vec::divide(temp.xyz,temp.w);
						M::Mat::Transform(viewInvert,temp.xyz,result.corner[5]);
		M::Vec::def(p.xy,1,1);
						M::Mat::Mult(projectionInvert,p,temp);
						if (temp.w)
							M::Vec::divide(temp.xyz,temp.w);
						M::Mat::Transform(viewInvert,temp.xyz,result.corner[6]);
		M::Vec::def(p.xy,-1,1);
						M::Mat::Mult(projectionInvert,p,temp);
						if (temp.w)
							M::Vec::divide(temp.xyz,temp.w);
						M::Mat::Transform(viewInvert,temp.xyz,result.corner[7]);

		result.UpdateNormals();
	}

template <class C> MF_DECLARE (String) Aspect<C>::ToString() const
{
	return "view\n"+M::Mat::Raw::ToString4x4(view)+"\nprojection\n"+M::Mat::Raw::ToString4x4(projection);
}



template <class C> MF_CONSTRUCTOR OrthographicAspect<C>::OrthographicAspect(VisualEnum::eDepthTest depthTest /*= VisualEnum::NoDepthTest*/):Aspect<C>(depthTest)
{
	M::Vec::clear(location);
	M::Mat::Eye(orientation);
}


MFUNC (void) OrthographicAspect<C>::UpdateProjection(const C&aspect, const C&zoom, const C&zNear, const C&zFar)
{
	C	scale = (C)1/(zFar-zNear),		//endless 0
		offset = -(zNear+zFar)/2*scale,	//endless -0.5
		pa = (C)1.0/region.GetPixelAspect()/aspect;

	M::Vec::def(projection.x,	pa*zoom,0,0,0);
	M::Vec::def(projection.y,	0,zoom,0,0);
	M::Vec::def(projection.z,	0,0,-scale,0);
	M::Vec::def(projection.w,	0,0,offset,1);
	M::Mat::invert(projection,projectionInvert);
}

//MFUNC (void) OrthographicAspect<C>::UpdateProjection2(const C&aspect, const C&zoom, const C&zNear, const C&zFar)
//{
//	C	scale = (C)1/(zFar-zNear),		//endless 0
//		offset = -(zNear+zFar)/2*scale,	//endless -0.5
//		pa = (C)1.0/region.GetPixelAspect()/aspect;
//
//	M::Vec::def(projection.x,	pa*zoom,0,0,0);
//	M::Vec::def(projection.y,	0,zoom,0,0);
//	M::Vec::def(projection.z,	0,0,-scale*0.5f,0);
//	M::Vec::def(projection.w,	0,0,offset+0.5f,1);
//	M::Mat::invert(projection,projectionInvert);
//}




template <class C>
MFUNC2 (void) OrthographicAspect<C>::AlterDepthRange(const C0&zNear, const C1&zFar)
{
	projection.z.z = (C)-1/(zFar-zNear);
	projection.w.z = (zNear+zFar)/2*projection[10];
	M::Mat::invert(projection,projectionInvert);
}


template <class C>
MFUNC1 (void) OrthographicAspect<C>::UpdateProjection(const M::Rect<C0>&area, const C&zNear, const C&zFar)
{
	UpdateProjection(area.x.min,area.y.min,area.x.max,area.y.max,zNear,zFar);
}

template <class C>
MFUNC1 (void) OrthographicAspect<C>::UpdateProjection2(const M::Rect<C0>&area, const C&zNear, const C&zFar)
{
	UpdateProjection2(area.x.min,area.y.min,area.x.max,area.y.max,zNear,zFar);
}


MFUNC (void) OrthographicAspect<C>::UpdateProjection(const C&left, const C&bottom, const C&right, const C&top,const C&zNear, const C&zFar)
{
	C	xscale = (C)1/(right-left)*2,
		yscale = (C)1/(top-bottom)*2,
		zscale = (C)1/(zFar-zNear)*2,
		xoffset = (C)-(left)*xscale-1,
		yoffset = (C)-(bottom)*yscale-1,
		zoffset = (C)-(zNear+zFar)/2*zscale;

	M::Vec::def(projection.x,	xscale,0,0,0);
	M::Vec::def(projection.y, 0,yscale,0,0);
	M::Vec::def(projection.z, 0,0,-zscale,0);
	M::Vec::def(projection.w, xoffset,yoffset,zoffset,1);


	M::Mat::invert(projection,projectionInvert);
}

MFUNC (void) OrthographicAspect<C>::UpdateProjection2(const C&left, const C&bottom, const C&right, const C&top,const C&zNear, const C&zFar)
{
	C	xscale = (C)1/(right-left)*2,
		yscale = (C)1/(top-bottom)*2,
		zscale = (C)1/(zFar-zNear)*2,
		xoffset = (C)-(left)*xscale-1,
		yoffset = (C)-(bottom)*yscale-1,
		zoffset = (C)-(zNear+zFar)/2*zscale;

	M::Vec::def(projection.x,	xscale,0,0,0);
	M::Vec::def(projection.y, 0,yscale,0,0);
	M::Vec::def(projection.z, 0,0,-zscale*(C)0.5,0);
	M::Vec::def(projection.w, xoffset,yoffset,zoffset+(C)0.5,1);


	M::Mat::invert(projection,projectionInvert);
}



template <class C> MF_DECLARE (void) OrthographicAspect<C>::UpdateView()
{
	viewInvert.x.xyz = orientation.x;
	viewInvert.y.xyz = orientation.y;
	viewInvert.z.xyz = orientation.z;
	viewInvert.w.xyz = location;
	M::Mat::invertSystem(viewInvert,view);
}
template <class C> MF_DECLARE (void) OrthographicAspect<C>::UpdateScaledView()
{
	viewInvert.x.xyz = orientation.x;
	viewInvert.y.xyz = orientation.y;
	viewInvert.z.xyz = orientation.z;
	viewInvert.w.xyz = location;
	M::Mat::invert(viewInvert,view);
}


template <class C> MF_CONSTRUCTOR Camera<C>::Camera()
{
	M::Vec::clear(location);
	M::Vec::clear(retraction);
	M::Mat::Eye(orientation);

	system = orientation;
	vfov = 90;
}

template <class C> MF_DECLARE (void) Camera<C>::UpdateProjection(const C&aspect, const C&zNear, const C&zFar, const C&vFov)
{
	vfov = vFov;
	C	extent = (C)1.0/M::vtan(vfov*M_PI/180/2),
		pa = (C)1.0/region.GetPixelAspect()/aspect;

	M::Vec::def(projection.x,	pa*extent,0,0,0);
	M::Vec::def(projection.y,	0,extent,0,0);
	M::Vec::def(projection.z,	0,0,(C)((C)zNear+zFar)/((C)zNear-zFar),-1);	//endless -1, -1
	M::Vec::def(projection.w,	0,0,(C)2.0*zNear*zFar/(zNear-zFar),0);	//endless 0
	M::Mat::invert(projection,projectionInvert);
}

template <class C> MF_DECLARE (void) Camera<C>::UpdateProjection2(const C&aspect, const C&zNear, const C&zFar, const C&vFov)
{
	vfov = vFov;
	C	extent = (C)1.0/M::vtan(vfov*M_PI/180/2),
		pa = (C)1.0/region.GetPixelAspect()/aspect;

	M::Vec::def(projection.x,	pa*extent,0,0,0);
	M::Vec::def(projection.y,	0,extent,0,0);
	M::Vec::def(projection.z,	0,0,(C)((C)zFar)/((C)zNear-zFar),-1);	//endless -1, -1
	M::Vec::def(projection.w,	0,0,(C)zNear*zFar/(zNear-zFar),0);	//endless 0
	M::Mat::invert(projection,projectionInvert);
}




template <class C>
MFUNC2 (void) Camera<C>::ExtractDepthRange(C0&zNear, C1&zFar)	const
{
	C	z0 = projection.z.z,	// = (zNear+zFar)/(zNear-zFar)
		z1 = projection.w.z;	// = 2*zNear*zFar/(zNear-zFar)
	
	/*
	z0 = (zNear+zFar)/(zNear-zFar)
	=> (zNear-zFar)*z0 = zNear+zFar
	=> z0*zNear - z0*zFar = zNear+zFar
	=> z0*zNear - zNear = zFar + z0*zFar
	=> zNear * (z0-1) = zFar * (z0+1)
	=> zNear = zFar * (z0+1)/(z0-1)
	
	f0 := (z0+1)/(z0-1)
	
	z1 = 2*zNear*zFar / (zNear-zFar)
	=> z1 = 2*zFar*f0*zFar/(zFar*f0 - zFar)
	=> z1 = 2*zFar²*f0/(zFar*(f0 - 1))
	=> z1 * zFar * (f0-1) = 2*zFar²*f0
	=> z1 * (f0-1) = 2*zFar*f0
	=> zFar = z1 * (f0-1) / (2*f0)
	
	
	=> zNear = z1 * (f0-1)/(2*f0)*f0
	=> zNear = z1 * (f0-1)/2
	*/

	
	C f0 = (z0+1)/(z0-1);
	
	zNear = (C0)(z1*(f0-1)/2);
	zFar = (C1)(z1*(f0-1)/(2*f0));
}


template <class C>
MFUNC2 (void) Camera<C>::AlterDepthRange(const C0&zNear, const C1&zFar)
{
	projection.z.z = (C)((C)zNear+zFar)/((C)zNear-zFar);
	projection.w.z = (C)2.0*zNear*zFar/(zNear-zFar);
	M::Mat::invert(projection,projectionInvert);
}


template <class C> MF_DECLARE (void) Camera<C>::UpdateView()
{
	if (GlobalAspectConfiguration::worldZIsUp)
	{
		view.x.xyz = orientation.x;
		view.y.xyz = orientation.z;
		view.z.xyz = orientation.y;
		M::Vec::mult(view.y.xyz,-1);
	}
	else
	{
		view.x.xyz = orientation.x;
		view.y.xyz = orientation.y;
		view.z.xyz = orientation.z;
	}
	view.w.xyz = retraction;
	M::Mat::resetBottomRow(view);
	M::TVec3<C> temp;
	M::Mat::rotate(view,location,temp);
	M::Vec::subtract(view.w.xyz,temp);

	M::Mat::invertSystem(view,viewInvert);
}


template <class C>
MFUNC3 (void) Camera<C>::Translate(const C0&x, const C1&y, const C2&z)
{
	M::TVec3<C>	vec = {x,y,z},out;
	M::Mat::rotate(viewInvert,vec,out);
	M::Vec::add(location,out);
	M::Vec::add(viewInvert.w.xyz,out);
	M::Vec::subtract(view.w.xyz,vec);
}

template <class C>
MFUNC1 (void) Camera<C>::Translate(const M::TVec3<C0>& delta)
{
	M::TVec3<C>	out;
	M::Mat::rotate(viewInvert,delta,out);
	M::Vec::add(location,out);
	M::Vec::add(viewInvert.w.xyz,out);
	M::Vec::subtract(view.w.xyz,delta);
}


template <class C>
MFUNC3 (void) Camera<C>::SetPosition(const C0&x, const C1&y, const C2&z)
{
	M::Vec::def(location,x,y,z);
	UpdateView();
}

template <class C>
MFUNC1 (void) Camera<C>::SetPosition(const M::TVec3<C0>&position)
{
	M::Vec::copy(position,location);
	UpdateView();			//could be removed
}

template <class C> MF_CONSTRUCTOR AngularCamera<C>::AngularCamera()
{
	M::Vec::clear(angle);
}

template <class C> MF_DECLARE (void) AngularCamera<C>::UpdateView()
{
	C	a0 = angle.x*M_PI/180,
		a1 = angle.y*M_PI/180,
		a2 = angle.z*M_PI/180;
	if (GlobalAspectConfiguration::worldZIsUp)
	{
		swp(a1,a2);
	}

	M::TMatrix3<C>	mbuffer0,
				mbuffer1,
				mbuffer2,
				mbuffer3;

	M::Vec::def(mbuffer0.x, cos(a1),0,-sin(a1));
	M::Vec::def(mbuffer0.y, 0, 1, 0);
	M::Vec::def(mbuffer0.z, sin(a1), 0, cos(a1));

	M::Vec::def(mbuffer1.x, 1,0,0);
	M::Vec::def(mbuffer1.y, 0, cos(a0), -sin(a0));
	M::Vec::def(mbuffer1.z, 0, sin(a0), cos(a0));

	M::Vec::def(mbuffer2.x, cos(a2),-sin(a2),0);
	M::Vec::def(mbuffer2.y, sin(a2), cos(a2), 0);
	M::Vec::def(mbuffer2.z, 0, 0, 1);

	M::Mat::Mult(mbuffer1,mbuffer2,mbuffer3);
	M::Mat::Mult(mbuffer0,mbuffer3,system);


	M::Mat::transpose(system,orientation);
	Camera<C>::UpdateView();
}

template <class C>
MFUNC3 (void) AngularCamera<C>::TranslatePlanar(const C0&x, const C1&y, const C2&z)
{
	C a =angle.y;
	M::TVec3<C>	delta;
	if (GlobalAspectConfiguration::worldZIsUp)
	{
		a = (C)180.0-angle.z;
		M::Vec::def (delta ,vcos(a*M_PI/180)*x+vsin(a*M_PI/180)*y,-vsin(a*M_PI/180)*x+vcos(a*M_PI/180)*y, z);
	}
	else
		M::Vec::def (delta ,vcos(a*M_PI/180)*x+vsin(a*M_PI/180)*z,y,-vsin(a*M_PI/180)*x+vcos(a*M_PI/180)*z);

	M::Vec::add(location,delta);
	UpdateView(); //could be removed...
}

template <class C>
MFUNC1 (void) AngularCamera<C>::TranslatePlanar(const M::TVec3<C0>&delta)
{
	TranslatePlanar(delta.x,delta.y,delta.z);
}

template <class C>
MFUNC3 (void) AngularCamera<C>::AlterAngles(const C0&alpha, const C1&beta, const C2&gamma)
{
	angle.x += alpha;
	angle.y += beta;
	angle.z += gamma;
	UpdateView();
}

template <class C>
MFUNC1 (void) AngularCamera<C>::LookAtPlanar(const M::TVec3<C0>&vector)
{
	angle.y = M::Vec::angle360(location.x-vector.x,location.z-vector.z);
	UpdateView();
}

template <class C>
MFUNC1 (void) AngularCamera<C>::LookAt(const M::TVec3<C0>&vector)
{
	M::TVec3<C>	delta;
	M::Vec::sub(location,vector,delta);
	if (GlobalAspectConfiguration::worldZIsUp)
	{
		angle.z = 90.f + M::Vec::angle360(delta.x,delta.y);
		angle.x = vatan2(delta.z,vsqrt(sqr(delta.x)+sqr(delta.y)))*(C)180/M_PI;
		angle.y = 0;
	}
	else
	{
		angle.y = 90.f - M::Vec::angle360(delta.x,delta.z);
		angle.x = vatan2(delta.y,vsqrt(sqr(delta.x)+sqr(delta.z)))*(C)180/M_PI;
		angle.z = 0;
	}
	UpdateView();

	//const M::TVec3<>&	view = GetViewingDirection();
	//if (!M::Vec::similar(view,delta))
	//	FATAL__("Expected similarity");
}


template <class C>
MFUNC1 (void) AngularCamera<C>::AlterAngles(const M::TVec3<C0>&delta)
{
	M::Vec::add(angle,delta);
	UpdateView();
}

template <class C> MF_DECLARE(VectorCamera<C>) AngularCamera<C>::ToVectorCamera()
{
	VectorCamera<C> result(*this);
	result.direction = system.z;
	result.upAxis = system.y;

	if (GlobalAspectConfiguration::worldZIsUp)
	{
		result.direction.z =-result.direction.z;
		std::swap(result.direction.y,result.direction.z);
		result.upAxis.z = -result.upAxis.z;
		std::swap(result.upAxis.y,result.upAxis.z);
	}

	return result;
}

template <class C>
	MF_CONSTRUCTOR VectorCamera<C>::VectorCamera(const Camera<C>&c):Camera<C>(c)
	{
		M::Vec::def(direction,0,0,-1);
		M::Vec::def(upAxis,0,1,0);
		vsystem = system;
	}


template <class C> MF_CONSTRUCTOR VectorCamera<C>::VectorCamera()
{
	M::Vec::def(direction,0,0,-1);
	M::Vec::def(upAxis,0,1,0);
	vsystem = system;
}

template <class C>
MFUNC3 (void) VectorCamera<C>::TranslatePlanar(const C0&x, const C1&y, const C2&z)
{
	M::TVec3<C>	temp = {x,y,z};
	TranslatePlanar(temp);
}

template <class C>
MFUNC1 (void) VectorCamera<C>::TranslatePlanar(const M::TVec3<C0>&delta)
{
	M::TVec3<C>	temp0,temp1;
	M::Mat::Mult(orientation,delta,temp0);
	M::Mat::rotate(view,temp0,temp1);
	M::Vec::add(location,temp1);
	view.w.xyz = location;
	M::Mat::invertSystem(view,viewInvert);
}

template <class C>
MFUNC2 (void) VectorCamera<C>::Align(const M::TVec3<C0>&dir, const M::TVec3<C1>&vert)
{
	M::Vec::copy(dir,direction);
	M::Vec::copy(vert,upAxis);
	UpdateView();
}


template <class C>
	MF_DECLARE (void) VectorCamera<C>::AdjustUpAxis()
	{
		M::TVec3<C>	n0,n1;
		M::Vec::cross(direction,upAxis,n0);
		M::Vec::cross(n0,direction,n1);
		M::Vec::normalize0(n1);
		if (M::Vec::dot(n1,upAxis)>0)
			upAxis = n1;
		else
			M::Vec::mult(n1,-1,upAxis);
	}

template <class C>
	template <typename T>
		MF_DECLARE (void) VectorCamera<C>::SetDirection(const M::TVec3<T>&d, bool doUpdate)
		{
			M::Vec::copy(d,direction);
			/*C	intensity = vabs(_intensity(upAxis,direction));
			if (intensity > 0.9)
				AdjustUpAxis();*/
			if (doUpdate)
				UpdateView();
		}

template <class C>
	template <typename T>
		MF_DECLARE (void) VectorCamera<C>::SetUpAxis(const M::TVec3<T>&v,bool doUpdate)
		{
			M::Vec::copy(v,upAxis);
			/*C	intensity = vabs(_intensity(v,direction));
			if (intensity > 0.99)
				return;*/
			/*if (intensity>0.9)
				adjustVertical();*/
			if (doUpdate)
				UpdateView();
		}

template <class C> MF_DECLARE (void) VectorCamera<C>::UpdateView()
{
	if (GlobalAspectConfiguration::worldZIsUp)
	{
		M::TVec3<C> direction = this->direction;
		M::TVec3<C> upAxis = this->upAxis;
		std::swap(direction.y,direction.z);	direction.z =-direction.z;
		std::swap(upAxis.y,upAxis.z); upAxis.z =-upAxis.z;

		system.z = direction;
		M::Vec::cross(upAxis,direction,system.x);
		M::Vec::cross(direction,system.x,system.y);
	}
	else
	{
		system.z = direction;
		M::Vec::cross(upAxis,direction,system.x);
		M::Vec::cross(direction,system.x,system.y);

	}
	vsystem.y = upAxis;
	M::Vec::cross(upAxis,direction,vsystem.x);
	M::Vec::cross(vsystem.x,upAxis,vsystem.z);
	M::Vec::normalize(vsystem.x);
	M::Vec::normalize(vsystem.y);
	M::Vec::normalize(vsystem.z);


	M::Vec::normalize(system.x);
	M::Vec::normalize(system.y);
	M::Vec::normalize(system.z);

	M::Mat::transpose(system,orientation);
	Camera<C>::UpdateView();
}

template <class C>
MFUNC2 (void) VectorCamera<C>::RotatePlanar(const C0&alpha, const C1&beta)
{
	M::TVec3<C>	axis0;
	M::TMatrix3<C>	matrix,temp;
	M::Vec::cross(upAxis,direction,axis0);
	M::Vec::normalize0(axis0);
	//_v3(axis0,-1,0,0);
	M::Mat::BuildRotationMatrix(-alpha,axis0,matrix);
	//_q2RotMatrixCCW(alpha,axis0,matrix);
	M::Mat::Mult(matrix,direction,axis0);
	//__mult3(matrix,orientation,temp);

	M::Mat::BuildRotationMatrix(beta,upAxis,matrix);
	M::Mat::Mult(matrix,axis0,direction);
	//__mult3(matrix,temp,orientation);
	
	
	UpdateView();
}

template <class C>
MFUNC1 (void) VectorCamera<C>::RotateDirectional(const C0&alpha, bool rebuild)
{
	M::TMatrix3<C>		matrix,tempm;
	M::TVec3<C>		v=direction;

	M::Vec::normalize0(v);
	M::Mat::BuildRotationMatrix(alpha,v,matrix);
    //__rotationMatrix(alpha, matrix, 3);
	M::Mat::Mult(matrix,upAxis);
	//__mult3(matrix,orientation,tempm);
	//_c9(tempm,orientation);
	if (rebuild)
		UpdateView();
}

template <class C> MF_DECLARE (AngularCamera<C>) VectorCamera<C>::ToAngularCamera()
{
//tbc - complicated ...

	return AngularCamera<C>();

}


template <class C>
Frustum<C>	Aspect<C>::result;



MF_DECLARE (RECT) rect(int left, int top, int right, int bottom)
{
	RECT rs;
	rs.left = left;
	rs.top = top;
	rs.right = right;
	rs.bottom = bottom;
	return rs;
}

}

#endif
