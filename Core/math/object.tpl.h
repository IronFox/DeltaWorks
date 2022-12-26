#ifndef object_mathTplH
#define object_mathTplH

/******************************************************************

Collection of template object-functions.
Also provides template-structures for abstract objects.


******************************************************************/

namespace DeltaWorks
{

	namespace Math	//! Collection of geometry related mathematical functions and classes
	{




MFUNC7
	(bool)		_oDetSphereCollision(const C0 p[3], const C1 v[3], const C2&r0, const C3 q[3], const C4 w[3], const C5&r1, C6&t)
	{
		C0	d[3],pd[3];
		_sub(v,w,d);
		_sub(p,q,pd);
		C0	a = _dot(d),
			b = 2*(_dot(p,d)-_dot(q,d)),
			c = _dot(pd)-sqr(r0+r1);
		C0 rs[2];
		BYTE rs_num = solveSqrEquation(a,b,c,rs);
		if (!rs_num)
			return false;
		t = smallestPositiveResult(rs,rs_num);
		return true;
	}
		
MFUNC7
	(bool)		_oDetCircleCollision(const C0 p[2], const C1 v[2], const C2&r0, const C3 q[2], const C4 w[2], const C5&r1, C6&t)
	{
		C0	d[2],pd[2];
		_sub2(v,w,d);
		_sub2(p,q,pd);
		C0	a = _dot2(d),
			b = 2*(_dot2(p,d)-_dot2(q,d)),
			c = _dot2(pd)-sqr(r0+r1);
		C0 rs[2];
		BYTE rs_num = solveSqrEquation(a,b,c,rs);
		if (!rs_num)
			return false;
		t = smallestPositiveResult(rs,rs_num);
		return true;
	}

MFUNC4
	(bool)		_oDetectOpticalSphereIntersection(const C0 center[3], const C1&radius, const C2 b[3], const C3 d[3])
	{
		C0	delta[3];
		_sub(b,center,delta);
			
		C0	pa = 1,
			pb = 2*_dot(d,delta),
			pc = _dot(delta)-sqr(radius),
			rs[2];
			
		BYTE num_rs = solveSqrEquation(pa,pb,pc,rs);
		return num_rs > 0;
	}

MFUNC5
	(bool)		_oDetectOpticalSphereIntersection(const C0 center[3], const C1&radius, const C2 b[3], const C3 d[3], C4&distance)
	{
		C0	delta[3];
		_sub(b,center,delta);
			
		C0	pa = 1,
			pb = 2*_dot(d,delta),
			pc = _dot(delta)-sqr(radius),
			rs[2];
			
		BYTE num_rs = solveSqrEquation(pa,pb,pc,rs);
		if (!num_rs)
			return false;
		C0	alpha = smallestPositiveResult(rs,num_rs);
		if (alpha >= distance)
			return false;
		distance = alpha;
		return true;
	}
		
MFUNC4 (void)			_oTriangleNormal(const M::TVec3<C0>&p0, const M::TVec3<C1>&p1, const M::TVec3<C2>&p2, M::TVec3<C3>&result)
{
	M::TVec3<C3>	v={p1.x-p0.x,p1.y-p0.y,p1.z-p0.z},
				w={p2.x-p0.x,p2.y-p0.y,p2.z-p0.z};

	result.x = v.y*w.z - v.z*w.y;
	result.y = v.z*w.x - v.x*w.z;
	result.z = v.x*w.y - v.y*w.x;

}
	
MFUNC4 (void)			_oAddTriangleNormal(const M::TVec3<C0>&p0, const M::TVec3<C1>&p1, const M::TVec3<C2>&p2, M::TVec3<C3>&inout)
{
	M::TVec3<C3>	v={p1.x-p0.x,p1.y-p0.y,p1.z-p0.z},
				w={p2.x-p0.x,p2.y-p0.y,p2.z-p0.z};

	inout.x += v.y*w.z - v.z*w.y;
	inout.y += v.z*w.x - v.x*w.z;
	inout.z += v.x*w.y - v.y*w.x;
}
	

	

MFUNC3 (C0)				_oTriangleSize(const C0*v1, const C1*v2, const C2*v3)
{
	C0 buffer[9];
	_subtract(v2,v1,buffer);
	_subtract(v3,v1,&buffer[3]);
	_cross(buffer,&buffer[3],&buffer[6]);
	return _length(&buffer[6])/2;
}
	
MFUNC3	(char)			_oTriangleOrientation2(const C0 p0[2], const C1 p1[2], const C2 p2[2])
{
	return M::Sign( (p1[0]-p0[0])*(p2[1]-p0[1]) - (p2[0]-p0[0])*(p1[1]-p0[1]) );
}

MFUNC3 (C0)		_oSignedTriangleSize2(const C0 p0[2], const C1 p1[2], const C2 p2[2])
{
	return	(p0[0]*p1[1]+p1[0]*p2[1]+p2[0]*p0[1]
			-p1[0]*p0[1]-p2[0]*p1[1]-p0[0]*p2[1])/2;
}


	
MFUNC2 (C0)		_oZeroTriangleSize2(const C0 p0[2], const C1 p1[2])
{
	return (p0[0]*p1[1] - p0[1]*p1[0])/2;
}


	
MFUNC6 (void)		_oTriangleCoord(const C0 v0[3], const C1 v1[3], const C2 v2[3], const C3&x, const C4&y, C5 out[3])
{
	out[0] = v0[0] + (v1[0]-v0[0])*x + (v2[0]-v0[0])*y;
	out[1] = v0[1] + (v1[1]-v0[1])*x + (v2[1]-v0[1])*y;
	out[2] = v0[2] + (v1[2]-v0[2])*x + (v2[2]-v0[2])*y;
}
	



MFUNC5 (C4)			_oIntersectPoint(const C0 base[3], const C1 normal[3], const C2 vbase[3], const C3 vdir[3], C4 out[3])
{
	C4	dif[3];
	_subtract(base,vbase,dif);
	C4	alpha = _dot(normal,vdir);
	if (!alpha)
		return 1.0e10;
	alpha = _dot(normal,dif)/alpha;
	_multAdd(vbase,vdir,alpha,out);
	return alpha;
}

MFUNC10
(bool)		_oDetVolumeVertexIntersection(const C0 p[3], const C1 v0t0[3], const C2 v1t0[3], const C3 v2t0[3], const C4 v0t1[3], const C5 v1t1[3], const C6 v2t1[3], C7&time, C8&x, C9&y)
{
	C0	n0[3],n1[3],nd[3],bd[3];
	_oTriangleNormal(v0t0,v1t0,v2t0,n0);
	_oTriangleNormal(v0t1,v1t1,v2t1,n1);
	_sub(n1,n0,nd);
	_sub(v0t1,v0t0,bd);
	C0	dt0 = _dot(n0,p),
		dt1 = _dot(nd,p),
		dt2 = _dot(n0,v0t0),
		dt3 = _dot(n0,bd),
		dt4 = _dot(nd,v0t0),
		dt5 = _dot(nd,bd),
		rs[2];
	BYTE num_rs = solveSqrEquation(-dt5,dt1-dt3-dt4,dt0-dt2,rs);
	if (!num_rs)
		return false;
	if (rs[0] > rs[1])
		swp(rs[0],rs[1]);
	if (rs[0] >= 0 && rs[0] <= 1)
		time = rs[0];
	else
		if (num_rs==2 && rs[1] >= 0 && rs[1] <= 1)
			time = rs[1];
		else
			return false;

	C0		p0[3],p1[3],p2[3],d0[3],d1[3],n[3];
	_interpolate(v0t0,v0t1,time,p0);
	_interpolate(v1t0,v1t1,time,p1);
	_interpolate(v2t0,v2t1,time,p2);
	_mad(n0,nd,time,n);
	_sub(p1,p0,d0);
	_sub(p2,p0,d1);
	_cross(n,d0,n0);
	_cross(n,d1,n1);
	C0	sub0 = _dot(n0,d1),
		sub1 = _dot(n1,d0);
	if (vabs(sub0) < Math::GetError<C0>() || vabs(sub1) < Math::GetError<C0>())
		return false;
	_sub(p,p0,bd);
	x = _dot(n0,bd)/sub0;
	y = _dot(n1,bd)/sub1;
	return true;
}

MFUNC6 (bool)		_oDetTriangleEdgeIntersection(const C0 t0[3], const C1 t1[3], const C2 t2[3], const C3 e0[3], const C4 e1[3], C5 out[3])
{
	C5	d[3],d0[3],d1[3],n[3],dif[3],n0[3],n1[3],cross_point[3];

	_subtract(e1,e0,d);
	_subtract(t1,t0,d0);
	_subtract(t2,t0,d1);
	_cross(d0,d1,n);
	_subtract(t0,e0,dif);
	C5	sub_alpha = _dot(n,d);
	if (!sub_alpha)
		return false;
	C5	alpha = _dot(n,dif)/sub_alpha;
	_cross(n,d0,n0);
	_cross(n,d1,n1);
	_multAdd(e0,d,alpha,cross_point);
	_subtract(cross_point,t0,dif);
	C5	beta = _dot(n0,dif)/_dot(n0,d1),
		gamma = _dot(n1,dif)/_dot(n1,d0);
	_v3(out, gamma, beta, alpha);
	return true;
}
	
MFUNC6
	(bool)		_oDetTriangleRayIntersection(const C0 t0[3], const C1 t1[3], const C2 t2[3], const C3 b[3], const C4 f[3], C5 out[3])
{
	C5	//d[3],
		d0[3],d1[3],n[3],dif[3],n0[3],n1[3],cross_point[3];

	//_subtract(e1,e0,d);
	_subtract(t1,t0,d0);
	_subtract(t2,t0,d1);
	_cross(d0,d1,n);
	_subtract(t0,b,dif);
	C5	sub_alpha = _dot(n,f);
	if (!sub_alpha)
		return false;
	C5	alpha = _dot(n,dif)/sub_alpha;
	_cross(n,d0,n0);
	_cross(n,d1,n1);
	_multAdd(b,f,alpha,cross_point);
	_subtract(cross_point,t0,dif);
	C5	beta = _dot(n0,dif)/_dot(n0,d1),
		gamma = _dot(n1,dif)/_dot(n1,d0);
	_v3(out, gamma, beta, alpha);
	return true;
}
	
MFUNC1
	(bool)		_oValidTriangleEdgeIntersection(const C0 factors[3])
	{
		return factors[0]>=-Math::GetError<C0>() && factors[1] >= -Math::GetError<C0>() && factors[0]+factors[1]<=(C0)1+Math::GetError<C0>() && factors[2] >= -Math::GetError<C0>() && factors[2] <= (C0)1+Math::GetError<C0>();
	}
	
MFUNC4 (void)		_oProject(const C0 base[3], const C1 normal[3], const C2 p[3], C3 out[3])
{
	C3 alpha = (_dot(normal,base) - _dot(normal,p))/ _dot(normal);
	_mad(p,normal,alpha,out);
}

MFUNC3 (C0)			_oProjectionFactor(const C0 base[3], const C1 normal[3], const C2 p[3])
{
	return (_dot(normal,base) - _dot(normal,p))/ _dot(normal);
}



MFUNC6 (bool)			_oDetectIntersection(const C0 p0[3], const C1 p1[3], const C2 p2[3], const C3 b[3], const C4 d[3], C5&distance)
{
	C5	d0[3] = {p1[0]-p0[0], p1[1]-p0[1], p1[2]-p0[2]},
				//_subtract(p1,p0,d0);
		d1[3] = {p2[0]-p0[0], p2[1]-p0[1], p2[2]-p0[2]},
				//_subtract(p2,p0,d1);
		n[3] = {d0[1]*d1[2] - d0[2]*d1[1],
				d0[2]*d1[0] - d0[0]*d1[2],
				d0[0]*d1[1] - d0[1]*d1[0]};
				//_cross(d0,d1,n);

//			dif[3],n0[3],n1[3],cross_point[3];



	C5	sub_alpha = n[0]*d[0]+n[1]*d[1]+n[2]*d[2];
				//_dot(n,d);
	if (!sub_alpha)
		return false;
	C5	dif[3] = {p0[0]-b[0],p0[1]-b[1],p0[2]-b[2]};
				//_subtract(p0,b,dif);
	C5	alpha = (n[0]*dif[0]+n[1]*dif[1]+n[2]*dif[2])/sub_alpha;
				//_dot(n,dif)/sub_alpha;
	C5 n0[3] = {n[1]*d0[2] - n[2]*d0[1],
				n[2]*d0[0] - n[0]*d0[2],
				n[0]*d0[1] - n[1]*d0[0]},
				//_cross(n,d0,n0);
		n1[3] = {n[1]*d1[2] - n[2]*d1[1],
				n[2]*d1[0] - n[0]*d1[2],
				n[0]*d1[1] - n[1]*d1[0]},
				//_cross(n,d1,n1);
		cross_point[3] ={b[0]+d[0]*alpha,
						b[1]+d[1]*alpha,
						b[2]+d[2]*alpha};
				//_multAdd(b,d,alpha,cross_point);
	_subtract(cross_point,p0,dif);
	C5	beta = (n0[0]*dif[0]+n0[1]*dif[1]+n0[2]*dif[2])/(n0[0]*d1[0]+n0[1]*d1[1]+n0[2]*d1[2]),
				//_dot(n0,dif)/_dot(n0,d1),
		gamma = (n1[0]*dif[0]+n1[1]*dif[1]+n1[2]*dif[2])/(n1[0]*d0[0]+n1[1]*d0[1]+n1[2]*d0[2]);
				//_dot(n1,dif)/_dot(n1,d0);
	if (beta > -Math::GetError<C5>() && gamma > -Math::GetError<C5>() && beta + gamma < (C5)1+ Math::GetError<C5>())
	{
		distance = alpha;
		return true;
	}
	return false;
}
	
MFUNC6 (bool)			_oDetectOpticalIntersection(const M::TVec3<C0>&p0, const M::TVec3<C1>&p1, const M::TVec3<C2>&p2, const M::TVec3<C3>&b, const M::TVec3<C4>&d, C5&distance)
{
	M::TVec3<C5>	d0 = {p1.x-p0.x, p1.y-p0.y, p1.z-p0.z},
				//_subtract(p1,p0,d0);
				d1 = {p2.x-p0.x, p2.y-p0.y, p2.z-p0.z},
				//_subtract(p2,p0,d1);
				n = {	d0.y*d1.z - d0.z*d1.y,
						d0.z*d1.x - d0.x*d1.z,
						d0.x*d1.y - d0.y*d1.x};
				//_cross(d0,d1,n);

//			dif[3],n0[3],n1[3],cross_point[3];



	C5	sub_alpha = n.x*d.x+n.y*d.y+n.z*d.z;
				//_dot(n,d);
	if (!sub_alpha)
		return false;
	M::TVec3<C5>	dif = {p0.x-b.x,p0.y-b.y,p0.z-b.z};
				//_subtract(p0,b,dif);
	C5	alpha = (n.x*dif.x+n.y*dif.y+n.z*dif.z)/sub_alpha;
				//_dot(n,dif)/sub_alpha;
	if (alpha < 0 || alpha > distance)
		return false;
	M::TVec3<C5>	n0 = {	n.y*d0.z - n.z*d0.y,
						n.z*d0.x - n.x*d0.z,
						n.x*d0.y - n.y*d0.x},
				//_cross(n,d0,n0);
				n1 = {	n.y*d1.z - n.z*d1.y,
						n.z*d1.x - n.x*d1.z,
						n.x*d1.y - n.y*d1.x},
				//_cross(n,d1,n1);
				cross_point ={	b.x+d.x*alpha,
								b.y+d.y*alpha,
								b.z+d.z*alpha};
				//_multAdd(b,d,alpha,cross_point);
	M::Vec::subtract(cross_point,p0,dif);
	C5	beta = (n0.x*dif.x + n0.y*dif.y + n0.z*dif.z) / (n0.x*d1.x + n0.y*d1.y + n0.z*d1.z),
				//_dot(n0,dif)/_dot(n0,d1),
		gamma = (n1.x*dif.x + n1.y*dif.y + n1.z*dif.z) / (n1.x*d0.x + n1.y*d0.y + n1.z*d0.z);
				//_dot(n1,dif)/_dot(n1,d0);
	if (beta > -Math::GetError<C5>() && gamma > -Math::GetError<C5>() && beta + gamma < (C5)1+ Math::GetError<C5>())
	{
		distance = alpha;
		return true;
	}
	return false;
}
	
MFUNC6 (bool)			_oDetectOpticalQuadIntersection(const C0 p0[3], const C1 p1[3], const C2 p2[3], const C3 b[3], const C4 d[3], C5&distance)
{
	C5	d0[3] = {p1[0]-p0[0], p1[1]-p0[1], p1[2]-p0[2]},
				//_subtract(p1,p0,d0);
		d1[3] = {p2[0]-p0[0], p2[1]-p0[1], p2[2]-p0[2]},
				//_subtract(p2,p0,d1);
		n[3] = {d0[1]*d1[2] - d0[2]*d1[1],
				d0[2]*d1[0] - d0[0]*d1[2],
				d0[0]*d1[1] - d0[1]*d1[0]};
				//_cross(d0,d1,n);

//			dif[3],n0[3],n1[3],cross_point[3];



	C5	sub_alpha = n[0]*d[0]+n[1]*d[1]+n[2]*d[2];
				//_dot(n,d);
	if (!sub_alpha)
		return false;
	C5	dif[3] = {p0[0]-b[0],p0[1]-b[1],p0[2]-b[2]};
				//_subtract(p0,b,dif);
	C5	alpha = (n[0]*dif[0]+n[1]*dif[1]+n[2]*dif[2])/sub_alpha;
				//_dot(n,dif)/sub_alpha;
	if (alpha < 0 || alpha > distance)
		return false;
	C5 n0[3] = {n[1]*d0[2] - n[2]*d0[1],
				n[2]*d0[0] - n[0]*d0[2],
				n[0]*d0[1] - n[1]*d0[0]},
				//_cross(n,d0,n0);
		n1[3] = {n[1]*d1[2] - n[2]*d1[1],
				n[2]*d1[0] - n[0]*d1[2],
				n[0]*d1[1] - n[1]*d1[0]},
				//_cross(n,d1,n1);
		cross_point[3] ={b[0]+d[0]*alpha,
						b[1]+d[1]*alpha,
						b[2]+d[2]*alpha};
				//_multAdd(b,d,alpha,cross_point);
	_subtract(cross_point,p0,dif);
	C5	beta = (n0[0]*dif[0]+n0[1]*dif[1]+n0[2]*dif[2])/(n0[0]*d1[0]+n0[1]*d1[1]+n0[2]*d1[2]),
				//_dot(n0,dif)/_dot(n0,d1),
		gamma = (n1[0]*dif[0]+n1[1]*dif[1]+n1[2]*dif[2])/(n1[0]*d0[0]+n1[1]*d0[1]+n1[2]*d0[2]);
				//_dot(n1,dif)/_dot(n1,d0);
	if (beta > -Math::GetError<C5>() && gamma > -Math::GetError<C5>() && beta < (C5)1+ Math::GetError<C5>() && gamma < (C5)1+ Math::GetError<C5>())
	{
		distance = alpha;
		return true;
	}
	return false;
}

MFUNC2 (void) _oDetDimension(const C0*vertex, C1*dimensions, unsigned band)
{
	for (unsigned el = 0; el < band; el++)
		if (vertex[el] < dimensions[el])
			dimensions[el] = vertex[el];
		else
			if (vertex[el] > dimensions[el+band])
				dimensions[el+band] = vertex[el];
}

MFUNC2V (void) _oDetDimensionV(const C0*vertex, C1*dimensions)
{
	for (index_t el = 0; el < Dimensions; el++)
		if (vertex[el] < dimensions[el])
			dimensions[el] = vertex[el];
		else
			if (vertex[el] > dimensions[el+Dimensions])
				dimensions[el+Dimensions] = vertex[el];
}


MFUNC2 (void) _oDetDimension(const M::TVec3<C0>&vertex, M::Box<C1>&dimensions)
{
	dimensions.Include(vertex);
}
	
MFUNC3 (void) _oDetDimension(const M::TVec3<C0>&vertex, M::TVec3<C1>&lower, M::TVec3<C2>&upper)
{
	if (vertex.x < lower.x)
		lower.x = vertex.x;
	else
		if (vertex.x > upper.x)
			upper.x = vertex.x;
	if (vertex.y < lower.y)
		lower.y = vertex.y;
	else
		if (vertex.y > upper.y)
			upper.y = vertex.y;
	if (vertex.z < lower.z)
		lower.z = vertex.z;
	else
		if (vertex.z > upper.z)
			upper.z = vertex.z;
}

MFUNC6 (bool)		_oDetEdgeIntersection2(const C0 a0[2], const C1 a1[2], const C2 b0[2], const C3 b1[2], C4&alpha, C5&beta)
{
	if (a0 == a1 || a0 == b0 || a0 == b1 || a1 == b0 || a1 == b1 || b0 == b1) //pointer-equations. checking the actual positions it too time-consuming
		return false;
	C4	d0x = b0[0]-a0[0],
		d0y = b0[1]-a0[1],
		d1x = a1[0]-a0[0],
		d1y = a1[1]-a0[1],
		d2x = b1[0]-b0[0],
		d2y = b1[1]-b0[1],
		sub = d2x*d1y-d2y*d1x,
		error = Math::GetError<C4>();
	if (vabs(sub)<error)
		return false;
	C4 	a = (d0y*d2x-d0x*d2y)/sub,
		b = (d0y*d1x-d0x*d1y)/sub;
	alpha = a;
	beta = b;
			
	return a >= -error && a <= (C4)1+error && b >= -error && b <= (C4)1+error;
}

MFUNC6	(bool)		_oDetEdgeIntersection(const C0 local0[3], const C1 local1[3], const C2 other0[3], const C3*other1, C4&out_local_intersection, C5&out_other_intersection)
{
	C4	d0[3],d1[3],n[3],n0[3],n1[3],x0[3],x1[3];
	_subtract(local1,local0,d0);
	_subtract(other1,other0,d1);
	_cross(d0,d1,n);
	_cross(n,d0,n0);
	_cross(n,d1,n1);
	C0	sub0 = _dot(d1,n0),
		sub1 = _dot(d0,n1);
	if (!sub0 || !sub1)
		return false;
	C4	fc0 = (_dot(local0,n0) - _dot(other0,n0)) / sub0,
		fc1 = (_dot(other0,n1) - _dot(local0,n1)) / sub1;
	if (fc0 < 0 || fc0 > 1 || fc1 < 0 || fc1 > 1)
		return false;
	out_local_intersection = fc0;
	out_other_intersection = fc1;



	_mad(local0,d0,fc0,x0);				//I'll remove this part later. For testing purpose only.
	_mad(other0,d1,fc1,x1);
	if (_distance(x0,x1) > GLOBAL_ERROR)
		FATAL__("internal intersection-error: "+M::Vec::toString(x0)+" differs from "+M::Vec::toString(x1)+" for case: "+M::Vec::toString(local0)+"->"+M::Vec::toString(local1)+" and "+M::Vec::toString(other0)+"->"+M::Vec::toString(other1));
//	_center(x0,x1,out_position);

	return true;
}

MFUNC9	(bool)		_oDetEdgeIntersection(const C0*local0, const C1*local1, const C2*prev0, const C3*prev1, const C4*next0, const C5*next1, C6&out_time, C7&out_local_intersection, C8&out_other_intersection)
{
	C6	local_delta[3],delta0[3],delta1[3],now0[3],now1[3];
	_subtract(local1,local0,local_delta);
	_subtract(next0,prev0,delta0);
	_subtract(next1,prev1,delta1);

	C6	a	=	_crossDot(local_delta,delta0,delta1),
		b	=	_crossDot(local_delta,prev0,delta1)	+ _crossDot(local_delta,delta0,prev1)
				- _crossDot(local_delta,delta0,local0) - _crossDot(local_delta,local0,delta1),
		c	=	_crossDot(local_delta,prev0,prev1)	- _crossDot(local_delta,prev0,local0) - _crossDot(local_delta,local0,prev1),
		rs[2];
	BYTE m_result = solveSqrEquation(a,b,c,rs);
	if (!m_result)
		return false;

	if (m_result==2)
	{
		if (rs[0] > rs[1])
			swap(rs[0],rs[1]);
		for (BYTE k = 0; k < 2; k++)
			if (rs[k] >= 0 && rs[k] <= 1)
			{
				out_time = rs[k];
				_interpolate(prev0,next0,out_time,now0);
				_interpolate(prev1,next1,out_time,now1);
				if (_oDetEdgeIntersection(local0,local1,now0,now1,out_local_intersection, out_other_intersection))
					return true;
			}
		return false;
	}
	out_time = rs[0];
	if (out_time < 0 || out_time > 1)
		return false;
	_interpolate(prev0,next0,out_time,now0);
	_interpolate(prev1,next1,out_time,now1);
	return _oDetEdgeIntersection(local0,local1,now0,now1,out_local_intersection, out_other_intersection);
}

MFUNC7	(bool)		_oDetFaceEdgeIntersection(const C0*local0, const C1*local1, const C2*local2, const C3*prev, const C4*next, C5&out_time, C6*out_position)
{
	C6	n[3],d0[3],d1[3],n0[3],n1[3],dif[3];
	_subtract(local1,local0,d0);
	_subtract(local2,local0,d1);
	_cross(d0,d1,n);
	C0	div = _dot(next,n) - _dot(prev,n);
	if (!div)
		return false;
	out_time = (_dot(local0,n) - _dot(prev,n)) / div;
	if (out_time < 0 || out_time > 1)
		return false;
	_cross(n,d0,n0);
	_cross(n,d1,n1);
	_interpolate(prev,next,out_time,out_position);
	_subtract(out_position,local0,dif);
	C5	beta = _dot(n0,dif)/_dot(n0,d1),
		gamma = _dot(n1,dif)/_dot(n1,d0);
	return !(beta < 0 || gamma < 0 || beta + gamma > 1);
}


template <class Def, class IndexType>
	MF_DECLARE (void)		_oMakeGraph(Mesh<TFaceGraphDef<Def> >&object, const Ctr::ArrayData<IndexType>&triangle_index_field, const Ctr::ArrayData<IndexType>&quad_index_field)
	{
		object.clear();
		typedef MeshVertex<TFaceGraphDef<Def> > Vtx;
		typedef MeshEdge<TFaceGraphDef<Def> >	Edg;
		typedef MeshTriangle<TFaceGraphDef<Def> >	Tri;
		typedef MeshQuad<TFaceGraphDef<Def> >	Quad;
		
		IndexType min,max;
			
		if (triangle_index_field.GetLength())
			max = min = triangle_index_field[0];
		else
			if (quad_index_field.GetLength())
				max = min = quad_index_field[0];
			else
				return;
			
		for (index_t i = 1; i < triangle_index_field.GetLength(); i++)
		{
			min = vmin(min,triangle_index_field[i]);
			max = vmax(max,triangle_index_field[i]);
		}
		for (index_t i = 0; i < quad_index_field.GetLength(); i++)
		{
			min = vmin(min,quad_index_field[i]);
			max = vmax(max,quad_index_field[i]);
		}
		object.vertex_field.SetSize(max+1-min);
		object.triangle_field.SetSize(triangle_index_field.GetLength()/3);
		object.quad_field.SetSize(quad_index_field.GetLength()/4);
		object.vregion.first = min;
		object.vregion.last = max;
			
		for (index_t i = 0; i < object.vertex_field.GetLength(); i++)
		{
			object.vertex_field[i].index = i+min;
			object.vertex_field[i].marked = false;
		}
			
		for (index_t i = 0; i < object.triangle_field.GetLength(); i++)
		{
			Tri&t = object.triangle_field[i];
			t.v0 = object.vertex_field+(triangle_index_field[i*3]-min);
			t.v1 = object.vertex_field+(triangle_index_field[i*3+1]-min);
			t.v2 = object.vertex_field+(triangle_index_field[i*3+2]-min);
			t.marked = false;
			t.index = i;
			t.attrib = 0;
			t.n0 = t.v0->findFace(t.v1);
			t.n1 = t.v1->findFace(t.v2);
			t.n2 = t.v2->findFace(t.v0);
			t.n0.setNeighbor(t.v1,&t);
			t.n1.setNeighbor(t.v2,&t);
			t.n2.setNeighbor(t.v0,&t);
			t.v0->Insert(&t);
			t.v1->Insert(&t);
			t.v2->Insert(&t);
		}
		for (index_t i = 0; i < object.quad_field.GetLength(); i++)
		{
			Quad&q = object.quad_field[i];
			q.v0 = object.vertex_field + (quad_index_field[i*4]-min);
			q.v1 = object.vertex_field + (quad_index_field[i*4+1]-min);
			q.v2 = object.vertex_field + (quad_index_field[i*4+2]-min);
			q.v3 = object.vertex_field + (quad_index_field[i*4+3]-min);
			q.marked = false;
			q.attrib = 0;
			q.index = i;
			q.n0 = q.v0->findFace(q.v1);
			q.n1 = q.v1->findFace(q.v2);
			q.n2 = q.v2->findFace(q.v3);
			q.n3 = q.v3->findFace(q.v0);
			q.n0.setNeighbor(q.v1,&q);
			q.n1.setNeighbor(q.v2,&q);
			q.n2.setNeighbor(q.v3,&q);
			q.n3.setNeighbor(q.v0,&q);
			q.v0->Insert(&q);
			q.v1->Insert(&q);
			q.v2->Insert(&q);
			q.v3->Insert(&q);
		}
	}
	
template <class Def, class IndexType>
	MF_DECLARE (void)		_oMakeGraph(DynamicMesh<TFaceGraphDef<Def> >&object, const Ctr::ArrayData<IndexType>&triangle_index_field, const Ctr::ArrayData<IndexType>&quad_index_field)
	{
		object.clear();
		typedef MeshVertex<TFaceGraphDef<Def> > Vtx;
		typedef MeshEdge<TFaceGraphDef<Def> >	Edg;
		typedef MeshTriangle<TFaceGraphDef<Def> >	Tri;
		typedef MeshQuad<TFaceGraphDef<Def> >	Quad;
		
		IndexType min,max;
			
		if (triangle_index_field.GetLength())
			max = min = triangle_index_field[0];
		else
			if (quad_index_field.GetLength())
				max = min = quad_index_field[0];
			else
				return;
			
		for (index_t i = 1; i < triangle_index_field.GetLength(); i++)
		{
			min = vmin(min,triangle_index_field[i]);
			max = vmax(max,triangle_index_field[i]);
		}
		for (index_t i = 0; i < quad_index_field.GetLength(); i++)
		{
			min = vmin(min,quad_index_field[i]);
			max = vmax(max,quad_index_field[i]);
		}
		object.vregion.first = min;
		object.vregion.last = max;
			
		for (index_t i = 0; i < max-min+1; i++)
		{
			Vtx&v = *object.vertices.Append();
			v.index = i+min;
			v.marked = false;
		}
			
		IndexType*p = triangle_index_field;
		for (index_t i = 0; i < triangle_index_field.GetLength()/3; i++)
		{
			Tri&t = *object.triangles.Append();
			t.v0 = object.vertices[(*p++)-min];
			t.v1 = object.vertices[(*p++)-min];
			t.v2 = object.vertices[(*p++)-min];
			t.marked = false;
			t.index = i;
			t.attrib = 0;
			t.n0 = t.v0->findFace(t.v1);
			t.n1 = t.v1->findFace(t.v2);
			t.n2 = t.v2->findFace(t.v0);
			t.n0.setNeighbor(t.v1,&t);
			t.n1.setNeighbor(t.v2,&t);
			t.n2.setNeighbor(t.v0,&t);
			t.v0->Insert(&t);
			t.v1->Insert(&t);
			t.v2->Insert(&t);
		}
		p = quad_index_field;
		for (index_t i = 0; i < quad_index_field.GetLength()/4; i++)
		{
			Quad&q = *object.quads.Append();
			q.v0 = object.vertices[(*p++)-min];
			q.v1 = object.vertices[(*p++)-min];
			q.v2 = object.vertices[(*p++)-min];
			q.v3 = object.vertices[(*p++)-min];
			q.marked = false;
			q.attrib = 0;
			q.index = i;
			q.n0 = q.v0->findFace(q.v1);
			q.n1 = q.v1->findFace(q.v2);
			q.n2 = q.v2->findFace(q.v3);
			q.n3 = q.v3->findFace(q.v0);
			q.n0.setNeighbor(q.v1,&q);
			q.n1.setNeighbor(q.v2,&q);
			q.n2.setNeighbor(q.v3,&q);
			q.n3.setNeighbor(q.v0,&q);
			q.v0->Insert(&q);
			q.v1->Insert(&q);
			q.v2->Insert(&q);
			q.v3->Insert(&q);
		}
	}	
	

template <class Def, class IndexType> MF_DECLARE (void) _oMakeTriangleGraph(Mesh<TGraphDef<Def> >&object, const Ctr::ArrayData<IndexType>&field)
{
	object.clear();
	typedef MeshVertex<TGraphDef<Def> > Vtx;
	typedef MeshEdge<TGraphDef<Def> >	Edg;
	typedef MeshTriangle<TGraphDef<Def> >	Tri;
	typedef MeshQuad<TGraphDef<Def> >	Quad;
		
	IndexType min((IndexType)-1),max(0);
	for (index_t i = 0; i < field.GetLength(); i++)
	{
		min = vmin(min,field[i]);
		max = vmax(max,field[i]);
	}
	object.vertex_field.SetSize(max+1-min);
	object.triangle_field.SetSize(field.GetLength()/3);
	object.vregion.first = min;
	object.vregion.last = max;

	M::Vector3<Edg>	edge_buffer;

	for (index_t i = 0; i < object.vertex_field.GetLength(); i++)
	{
		object.vertex_field[i].index = i+min;
		object.vertex_field[i].marked = false;
	}
	for (index_t i = 0; i < object.triangle_field.GetLength(); i++)
	{
		object.triangle_field[i].marked = false;
		object.triangle_field[i].attrib = 0;
		object.triangle_field[i].index = i;
		for (BYTE k = 0; k < 3; k++)
		{
			object.triangle_field[i].n[k] = NULL;
			object.triangle_field[i].vertex[k] = &object.vertex_field[field[i*3+k]-min];
		}
		for (BYTE k = 0; k < 3; k++)
		{
			Vtx*vtx0 = object.triangle_field[i].vertex[k],
				*vtx1 = object.triangle_field[i].vertex[(k+1)%3];
			Edg*edge(NULL),*current(vtx0->first);
			while (current && !edge)
			{
				char index = current->GetIndexOf(vtx0);
				if (index == -1)
					FATAL__("edge-extraction-error for "+IntToStr(i)+" "+IntToStr(k));
				if (current->vertex[!index] == vtx1)
					edge = current;
				else
					current = current->next[index];
			}
			if (!edge)
			{
				edge = edge_buffer.Append();
				edge->v0 = object.triangle_field[i].vertex[k];
				edge->v1 = object.triangle_field[i].vertex[(k+1)%3];
				edge->n[0].triangle = &object.triangle_field[i];
				edge->n[1].triangle = NULL;
				edge->n[0].is_quad = false;
				edge->n[1].is_quad = false;
				edge->next[0] = NULL;
				edge->next[1] = NULL;
				edge->marked = false;

				vtx0->degree++;
				vtx1->degree++;
				if (vtx0->last)
				{
					char index = vtx0->last->GetIndexOf(vtx0);
					if (index == -1)
						FATAL__("linkage broken");
					vtx0->last->next[index] = edge;
					vtx0->last = edge;
				}
				else
					vtx0->first = vtx0->last = edge;

				if (vtx1->last)
				{
					char index = vtx1->last->GetIndexOf(vtx1);
					if (index == -1)
						FATAL__("linkage broken");
					vtx1->last->next[index] = edge;
					vtx1->last = edge;
				}
				else
					vtx1->first = vtx1->last = edge;
			}
			else
			{
				char index = edge->triangle[0]->GetIndexOf(edge->v0);
				if (index == -1)
					FATAL__("broken index");
				edge->triangle[0]->n[index] = &object.triangle_field[i];
				object.triangle_field[i].nt[k] = edge->triangle[0];
				object.triangle_field[i].is_quad[k] = false;
				edge->n[1].triangle = &object.triangle_field[i];
			}
			object.triangle_field[i].edge[k] = edge;
		}
	}

	object.edge_field.SetSize(edge_buffer.Count());
	for (index_t i = 0; i < object.edge_field.GetLength(); i++)
		edge_buffer[i]->index = i;

	for (index_t i = 0; i < object.edge_field.GetLength(); i++)
	{
		object.edge_field[i] = *edge_buffer[i];
		if (object.edge_field[i].next[0])
			object.edge_field[i].next[0] = &object.edge_field[object.edge_field[i].next[0]->index];
		if (object.edge_field[i].next[1])
			object.edge_field[i].next[1] = &object.edge_field[object.edge_field[i].next[1]->index];
	}
	for (index_t i = 0; i < object.triangle_field.GetLength(); i++)
		for (BYTE k = 0; k < 3; k++)
			object.triangle_field[i].edge[k] = &object.edge_field[object.triangle_field[i].edge[k]->index];
	for (index_t i = 0; i < object.vertex_field.GetLength(); i++)
		if (object.vertex_field[i].first)
		{
			object.vertex_field[i].first = &object.edge_field[object.vertex_field[i].first->index];
			object.vertex_field[i].last = &object.edge_field[object.vertex_field[i].last->index];
		}
		else
			if (object.vertex_field[i].last)
				FATAL__("data shot");



	for (index_t i = 0; i < object.edge_field.GetLength(); i++)
		for (BYTE k = 0; k < 2; k++)
		{
			if (object.edge_field[i].next[k] && (index_t)(object.edge_field[i].next[k]-object.edge) >= object.edge_field.GetLength())
				FATAL__("object shot");
			if (object.edge_field[i].next[k] == &object.edge_field[i])
				FATAL__("selfrefering");
			if (_oIndexOf(&object.edge_field[i],object.edge_field[i].vertex[k])==-1)
				FATAL__("vertex-linkage shot");
		}

	if (!object.valid())
		FATAL__("object invalid");

//	cout << "len: "<<len<<endl;
//	cout << "vertices: "<<object.vertex_field.GetLength()<<endl;
//	cout << "edges: "<<object.edge_field.GetLength()<<endl;
//	cout << "faces: "<<object.face_field.GetLength()<<endl;

}
	
template <class Def, class IndexType>
	MF_DECLARE (bool)		_oMakeGraph(Mesh<TFaceGraphDef<Def> >&object, const Ctr::ArrayData<IndexType>&index_field, const Ctr::ArrayData<IndexType>&tsegment_field, const Ctr::ArrayData<IndexType>&qsegment_field)
	{
		object.clear();
		typedef MeshVertex<TFaceGraphDef<Def> > Vtx;
		typedef MeshEdge<TFaceGraphDef<Def> >	Edg;
		typedef MeshTriangle<TFaceGraphDef<Def> >	Tri;
		typedef MeshQuad<TFaceGraphDef<Def> >	Quad;
		
		IndexType min,max;
			
		if (index_field.GetLength())
			max = min = index_field[0];
		else
			return true;
			
		for (index_t i = 1; i < index_field.GetLength(); i++)
		{
			min = vmin(min,index_field[i]);
			max = vmax(max,index_field[i]);
		}
		object.vertex_field.SetSize(max+1-min);
		{
			count_t	triangles = 0,
					indices = 0;
			if (tsegment_field.GetLength())
			{
				triangles = tsegment_field[0]/3;
				indices += tsegment_field[0];
				if (qsegment_field[0]%3)
					return false;					
			}
			for (index_t k = 1; k < tsegment_field.GetLength(); k++)
			{
				triangles += tsegment_field[k]-2;
				indices += tsegment_field[k];
			}
			object.triangle_field.SetSize(triangles);

			count_t	quads= 0;

			if (qsegment_field.GetLength())
			{
				quads += qsegment_field[0]/4;
				indices += qsegment_field[0];
				if (qsegment_field[0]%4)
					return false;
			}
			for (index_t k = 1; k < qsegment_field.GetLength(); k++)
			{
				quads += (qsegment_field[k]-2)/2;
				indices += qsegment_field[k];
				if (qsegment_field[k]%2)
					return false;
			}
			object.quad_field.SetSize(quads);
			if (indices != index_field.GetLength())
				return false;
		}
		object.vregion.first = min;
		object.vregion.last = max;
			
		for (index_t i = 0; i < object.vertex_field.GetLength(); i++)
		{
			object.vertex_field[i].index = i+min;
			object.vertex_field[i].marked = false;
		}
			
		IndexType*index = index_field;
		if (tsegment_field.GetLength())
		{
			Tri*t = object.triangle_field;
			for (index_t i = 0; i < tsegment_field[0]/3; i++)
			{
				t->v0 = object.vertex_field+((*index++)-min);
				t->v1 = object.vertex_field+((*index++)-min);
				t->v2 = object.vertex_field+((*index++)-min);
				t->marked = false;
				t->index = t-object.triangle_field;
				t->attrib = 0;
				t->n0 = t->v0->findFace(t->v1);
				t->n1 = t->v1->findFace(t->v2);
				t->n2 = t->v2->findFace(t->v0);
				t->n0.setNeighbor(t->v1,t);
				t->n1.setNeighbor(t->v2,t);
				t->n2.setNeighbor(t->v0,t);
				t->v0->Insert(t);
				t->v1->Insert(t);
				t->v2->Insert(t);
				t++;
			}
			for (index_t j = 1; j < tsegment_field.GetLength(); j++)
			{
				IndexType	i0 = (*index++),
							i1 = (*index++);
				bool invert(false);
				for (index_t i = 2; i < tsegment_field[j]; i++)
				{
					IndexType i2 = (*index++);
					if (!invert)
					{
						t->v0 = object.vertex_field+(i0-min);
						t->v1 = object.vertex_field+(i1-min);
						t->v2 = object.vertex_field+(i2-min);
					}
					else
					{
						t->v0 = object.vertex_field+(i0-min);
						t->v2 = object.vertex_field+(i1-min);
						t->v1 = object.vertex_field+(i2-min);
					}
					t->marked = false;
					t->index = t-object.triangle_field;
					t->attrib = 0;
					t->n0 = t->v0->findFace(t->v1);
					t->n1 = t->v1->findFace(t->v2);
					t->n2 = t->v2->findFace(t->v0);
					t->n0.setNeighbor(t->v1,t);
					t->n1.setNeighbor(t->v2,t);
					t->n2.setNeighbor(t->v0,t);
					t->v0->Insert(t);
					t->v1->Insert(t);
					t->v2->Insert(t);
					t++;
						
					i0 = i1;
					i1 = i2;
					invert = !invert;
				}
			}
			ASSERT_CONCLUSION(object.triangle_field,t)
		}
		if (qsegment_field.GetLength())
		{
			Quad*q = object.quad_field;

			for (index_t i = 0; i < qsegment_field[0]/4; i++)
			{
				q->v0 = object.vertex_field + ((*index++)-min);
				q->v1 = object.vertex_field + ((*index++)-min);
				q->v2 = object.vertex_field + ((*index++)-min);
				q->v3 = object.vertex_field + ((*index++)-min);
				q->marked = false;
				q->attrib = 0;
				q->index = i;
				q->n0 = q->v0->findFace(q->v1);
				q->n1 = q->v1->findFace(q->v2);
				q->n2 = q->v2->findFace(q->v3);
				q->n3 = q->v3->findFace(q->v0);
				q->n0.setNeighbor(q->v1,q);
				q->n1.setNeighbor(q->v2,q);
				q->n2.setNeighbor(q->v3,q);
				q->n3.setNeighbor(q->v0,q);
				q->v0->Insert(q);
				q->v1->Insert(q);
				q->v2->Insert(q);
				q->v3->Insert(q);
				q++;
			}
			for (index_t j = 1; j < qsegment_field.GetLength(); j++)
			{
				IndexType	i0 = (*index++),
							i1 = (*index++);
				for (index_t i = 2; i < qsegment_field[j]/2; i++)
				{
					IndexType	i2 = (*index++),
								i3 = (*index++);
					q->v0 = object.vertex_field+(i0-min);
					q->v1 = object.vertex_field+(i1-min);
					q->v2 = object.vertex_field+(i2-min);
					q->v3 = object.vertex_field+(i3-min);
					q->marked = false;
					q->index = q-object.quad_field;
					q->attrib = 0;
					q->n0 = q->v0->findFace(q->v1);
					q->n1 = q->v1->findFace(q->v2);
					q->n2 = q->v2->findFace(q->v3);
					q->n3 = q->v3->findFace(q->v0);
					q->n0.setNeighbor(q->v1,q);
					q->n1.setNeighbor(q->v2,q);
					q->n2.setNeighbor(q->v3,q);
					q->n3.setNeighbor(q->v0,q);
					q->v0->Insert(q);
					q->v1->Insert(q);
					q->v2->Insert(q);
					q->v3->Insert(q);
					q++;
						
					i0 = i2;
					i1 = i3;
				}
			}
			ASSERT_CONCLUSION(object.quad_field,q)
		}
		ASSERT_CONCLUSION(index_field,index)
		return true;
	}
	
template <class Def, class IndexType>
	MF_DECLARE (bool)		_oMakeGraph(DynamicMesh<TFaceGraphDef<Def> >&object, const Ctr::ArrayData<IndexType>&index_field, const Ctr::ArrayData<IndexType>&tsegment_field, const Ctr::ArrayData<IndexType>&qsegment_field)
	{
		object.clear();
		typedef MeshVertex<TFaceGraphDef<Def> > Vtx;
		typedef MeshEdge<TFaceGraphDef<Def> >	Edg;
		typedef MeshTriangle<TFaceGraphDef<Def> >	Tri;
		typedef MeshQuad<TFaceGraphDef<Def> >	Quad;
		
		IndexType min,max;
			
		if (index_field.GetLength())
			max = min = index_field[0];
		else
			return true;
			
		for (index_t i = 1; i < index_field.GetLength(); i++)
		{
			min = vmin(min,index_field[i]);
			max = vmax(max,index_field[i]);
		}

		{
			index_t	indices = 0;
			if (tsegment_field.GetLength())
			{
				indices = tsegment_field[0];
				if (qsegment_field[0]%3)
					return false;					
			}
			for (index_t k = 1; k < tsegment_field.GetLength(); k++)
			{
				indices += tsegment_field[k];
			}

			if (qsegment_field.GetLength())
			{
				indices += qsegment_field[0];
				if (qsegment_field[0]%4)
					return false;
			}
			for (index_t k = 1; k < qsegment_field.GetLength(); k++)
			{
				indices += qsegment_field[k];
				if (qsegment_field[k]%2)
					return false;
			}
			if (indices != index_field.GetLength())
				return false;
		}
		object.vregion.first = min;
		object.vregion.last = max;
			
		for (index_t i = 0; i < max-min+1; i++)
		{
			MeshVertex<TFaceGraphDef<Def> >*vtx = object.vertices.Append();
			vtx->index = i+min;
			vtx->marked = false;
		}
			
		const IndexType*index = index_field;
		if (tsegment_field.GetLength())
		{
			for (index_t i = 0; i < tsegment_field[0]/3; i++)
			{
				Tri*t = object.triangles.Append();
				t->v0 = object.vertices.GetChar((*index++)-min);
				t->v1 = object.vertices.GetChar((*index++)-min);
				t->v2 = object.vertices.GetChar((*index++)-min);
				t->marked = false;
				t->index = object.triangles-1;
				t->attrib = 0;
				t->n0 = t->v0->findFace(t->v1);
				t->n1 = t->v1->findFace(t->v2);
				t->n2 = t->v2->findFace(t->v0);
				t->n0.setNeighbor(t->v1,t);
				t->n1.setNeighbor(t->v2,t);
				t->n2.setNeighbor(t->v0,t);
				t->v0->Insert(t);
				t->v1->Insert(t);
				t->v2->Insert(t);
			}
			for (index_t j = 1; j < tsegment_field.GetLength(); j++)
			{
				IndexType	i0 = (*index++),
							i1 = (*index++);
				bool invert(false);
				for (index_t i = 2; i < tsegment_field[j]; i++)
				{
					Tri*t = object.triangles.Append();
					IndexType i2 = (*index++);
					if (!invert)
					{
						t->v0 = object.vertices.GetChar(i0-min);
						t->v1 = object.vertices.GetChar(i1-min);
						t->v2 = object.vertices.GetChar(i2-min);
					}
					else
					{
						t->v0 = object.vertices.GetChar(i0-min);
						t->v2 = object.vertices.GetChar(i1-min);
						t->v1 = object.vertices.GetChar(i2-min);
					}
					t->marked = false;
					t->index = object.triangles-1;
					t->attrib = 0;
					t->n0 = t->v0->findFace(t->v1);
					t->n1 = t->v1->findFace(t->v2);
					t->n2 = t->v2->findFace(t->v0);
					t->n0.setNeighbor(t->v1,t);
					t->n1.setNeighbor(t->v2,t);
					t->n2.setNeighbor(t->v0,t);
					t->v0->Insert(t);
					t->v1->Insert(t);
					t->v2->Insert(t);
						
					i0 = i1;
					i1 = i2;
					invert = !invert;
				}
			}
		}
		if (qsegment_field.GetLength())
		{

			for (index_t i = 0; i < qsegment_field[0]/4; i++)
			{
				Quad*q = object.quads.Append();
				q->v0 = object.vertices.GetChar((*index++)-min);
				q->v1 = object.vertices.GetChar((*index++)-min);
				q->v2 = object.vertices.GetChar((*index++)-min);
				q->v3 = object.vertices.GetChar((*index++)-min);
				q->marked = false;
				q->attrib = 0;
				q->index = i;
				q->n0 = q->v0->findFace(q->v1);
				q->n1 = q->v1->findFace(q->v2);
				q->n2 = q->v2->findFace(q->v3);
				q->n3 = q->v3->findFace(q->v0);
				q->n0.setNeighbor(q->v1,q);
				q->n1.setNeighbor(q->v2,q);
				q->n2.setNeighbor(q->v3,q);
				q->n3.setNeighbor(q->v0,q);
				q->v0->Insert(q);
				q->v1->Insert(q);
				q->v2->Insert(q);
				q->v3->Insert(q);
			}
			for (index_t j = 1; j < qsegment_field.GetLength(); j++)
			{
				IndexType	i0 = (*index++),
							i1 = (*index++);
				for (index_t i = 2; i < qsegment_field[j]/2; i++)
				{
					IndexType	i2 = (*index++),
								i3 = (*index++);
					Quad*q = object.quads.Append();
					q->v0 = object.vertices.GetChar(i0-min);
					q->v1 = object.vertices.GetChar(i1-min);
					q->v2 = object.vertices.GetChar(i2-min);
					q->v3 = object.vertices.GetChar(i3-min);
					q->marked = false;
					q->index = object.quads-1;
					q->attrib = 0;
					q->n0 = q->v0->findFace(q->v1);
					q->n1 = q->v1->findFace(q->v2);
					q->n2 = q->v2->findFace(q->v3);
					q->n3 = q->v3->findFace(q->v0);
					q->n0.setNeighbor(q->v1,q);
					q->n1.setNeighbor(q->v2,q);
					q->n2.setNeighbor(q->v3,q);
					q->n3.setNeighbor(q->v0,q);
					q->v0->Insert(q);
					q->v1->Insert(q);
					q->v2->Insert(q);
					q->v3->Insert(q);
						
					i0 = i2;
					i1 = i3;
				}
			}
		}
		ASSERT_CONCLUSION(index_field,index)
		return true;
	}
	
	
	
	
template <class Element> MF_DECLARE (count_t)	_oCountUnmarked(const Ctr::BasicVector<Element>&list)
{
	count_t count(0);
	for (index_t i = 0; i < list.Count(); i++)
		count += !list[i]->marked;
	return count;
}
	
template <class Def> MF_DECLARE (String)	_oCheckNeighborIntegrity(const DynamicMesh<TFaceGraphDef<Def> >&object)
{
	for (index_t i = 0; i < object.edges; i++)
		if (!object.vertices(object.edges[i].v0) || !object.vertices(object.edges[i].v1))
			return "Vertex linkage of edge #"+IntToStr(i+1)+"/"+IntToStr(object.edges)+" broken";
	for (index_t i = 0; i < object.triangles; i++)
		if (!object.vertices(object.edges[i].v0) || !object.vertices(object.edges[i].v1))
			return "Linkage of edge #"+IntToStr(i+1)+"/"+IntToStr(object.edges)+" broken";
		
	
	
	return "";
}
	
namespace Obj
{
	template <typename F, typename I>
	bool	TriangulateF( const count_t numVertices, const std::function<TVec2<F>(index_t)>&getVertex, BasicVector<I>&target )
	{
		Array<TVec2<F> >temp(numVertices);	//this is likely the most efficient approach, what with the constant searching
		for (index_t i = 0; i < numVertices; i++)
			temp[i] = getVertex(i);
		return Triangulate(temp,target);
	}

	template <typename F, typename I>
	bool	Triangulate( const ArrayRef<TVec2<F> >&vertices, BasicVector<I>&target )
	{
		target.reset();
		if (vertices.Count() < 3)
			return false;
		Vector<I>	indices;
		for (index_t i = 0; i < vertices.Count(); i++)
			indices<<i;

		typedef TVec2<F>	Vertex;
		typedef F			Float;
		typedef I			Index;

		Float size(0);
		for (index_t i = 0; i < vertices.Count()-1; i++)
			size += Obj::zeroTriangleSize(vertices[i],vertices[i+1]);
		size += Obj::zeroTriangleSize(vertices.Last(),vertices.First());

		const int orientation = M::Sign(size);
		bool failed(false);
		
		index_t		current_index(0),
					walk(0);
		while (indices.Count() > 3 && !failed)
		{
			walk = 0;
			bool looping=true;
			Index		index0,index1,index2;

			while (looping)
			{
				walk++;
				if (walk > vertices.Count())
				{
					looping = false;
					failed = true;
					continue;
				}
				index0 = indices[current_index];
				index1 = indices[(current_index+1)%indices.Count()];
				index2 = indices[(current_index+2)%indices.Count()];
				current_index++;
				if (current_index >= indices.Count())
					current_index = 0;
				if (M::Sign(Obj::signedTriangleSize(vertices[index0],vertices[index1],vertices[index2])) != orientation)
					continue;
				bool hit(false);
				Float	dummy;
				const Vertex &p0 = vertices[index2],	&p1 = vertices[indices[(current_index+2)%indices.Count()]],	&p2=vertices[index0];
				const int thisOrientation = M::Sign(Obj::signedTriangleSize(p0,p1,p2));
				if (thisOrientation != orientation && thisOrientation != 0)
					continue;

				for (index_t i = 0; i < indices.Count() && !hit; i++)
				{
					const index_t i0 = (indices[i]);
					const index_t i1 = (indices[(i+1)%(indices.Count())]);
					if (i0 != index0 && i0 != index2 && i1 != index0 && i1 != index2)
					{
						bool cut = Obj::detEdgeIntersection(vertices[i0],vertices[i1],vertices[index0],vertices[index2],dummy,dummy);
						hit = cut;
					}
				}
				
				looping = hit;
			}
			if (!failed)
			{
				target << index0 << index1 << index2;
				indices.Erase(current_index);
				if (current_index >= indices.Count())
					current_index = 0;
			}
		}
		if (indices.Count() == 3)
		{
			target << indices.First() << indices[1] << indices[2];
		}
		return !failed;
	}

	MFUNC4 (bool)		DetectOpticalBoxIntersection(const M::Box<C0>&box, const M::TVec3<C1>&b, const M::TVec3<C2>&d, C3&distance)
	{
		if (distance <= 0)
			return false;
		bool result = false;
		static const C3 error = TypeInfo<C2>::error;
		for (BYTE k = 0; k < 3; k++)
			if (d.v[k])
			{
				const BYTE	i = (k+1)%3,
							j = (k+2)%3;
				const C3 minDist = (C3)(box.axis[k].min-b.v[k]);
				if (minDist > error)
				{
					//b is less than minimum on this axis
					if (d.v[k] < 0)
						continue;	//nothing to find on this axis

					const C3 f = minDist /(d.v[k]);
					if (f >= distance)
						continue;	//cube side is too far away to intersect

					const C3 x = b.v[i]+(d.v[i])*f;
					const C3 y = b.v[j]+(d.v[j])*f;

					if (box.axis[i].Contains(x)
						&&
						box.axis[j].Contains(y))
					{
						distance = f;
						result = true;
					}
					continue;
				}


				const C3 maxDist = (C3)(b.v[k] - box.axis[k].max);
				if (maxDist > error)
				{
					//b is greater than maximum on this axis
					if (d.v[k] > 0)
						continue;	//nothing to find on this axis

					const C3 f = maxDist /(-d.v[k]);
					if (f >= distance)
						continue;	//cube side is too far away to intersect

					const C3 x = b.v[i]+(d.v[i])*f;
					const C3 y = b.v[j]+(d.v[j])*f;

					if (box.axis[i].Contains(x)
						&&
						box.axis[j].Contains(y))
					{
						distance = f;
						result = true;
					}
					continue;
				}

				distance = 0;
				return true;
			}
		return result;
	}

	MFUNC3	(bool)		DetectSphereEdgeIntersection(const M::Sphere<C0>&sphere, const M::TVec3<C1>&e0, const M::TVec3<C2>&e1)
	{
		return DetectSphereEdgeIntersection(sphere.center,sphere.radius,e0,e1);
	}


	MFUNC4	(bool)		DetectOpticalSphereIntersection(const M::Sphere<C0>&sphere, const M::TVec3<C1>&b, const M::TVec3<C2>&d, C3&distance)
	{
		if (distance <= 0)
			return false;
		M::TVec3<C0>	delta;
		Vec::sub(b,sphere.center,delta);

		const C3 dd = (C3)Vec::dot(delta);
		const C3 r2 = M::sqr((C3)sphere.radius);

		if (dd <= r2)
		{
			distance = 0;
			return true;
		}

		C3	pa = (C3)1,
			pb = (C3)2*(C3)Vec::dot(d,delta),
			pc = dd-r2,
			rs[2];
			
		BYTE num_rs = solveSqrEquation(pa,pb,pc,rs);
		if (!num_rs)
			return false;
		C3	alpha = smallestPositiveResult(rs,num_rs);
		if (alpha >= distance)
			return false;
		distance = alpha;
		return true;
	}
}
	
MFUNC3 (bool)		_oIntersectsBox(const M::TVec3<C0>&p0, const M::TVec3<C1>&p1, const M::Box<C2>&box)
{
	if (box.Contains(p0) || box.Contains(p1))
		return true;

	for (BYTE k = 0; k < 3; k++)
		if (p1.v[k] != p0.v[k])
		{
			BYTE	i = (k+1)%3,
					j = (k+2)%3;
			C2 f = (box.axis[k].min-p0.v[k])/(p1.v[k]-p0.v[k]),x,y;
			if (f >= -Math::GetError<C2>() && f <= (C2)1+Math::GetError<C2>())
			{
				x = p0.v[i]+(p1.v[i]-p0.v[i])*f;
				y = p0.v[j]+(p1.v[j]-p0.v[j])*f;
				if (box.axis[i].Contains(x)
					&&
					box.axis[j].Contains(y))
					return true;
			}
			f = (box.axis[k].max-p0.v[k])/(p1.v[k]-p0.v[k]);
			if (f >= 0 && f <= 1)
			{
				x = p0.v[i]+(p1.v[i]-p0.v[i])*f;
				y = p0.v[j]+(p1.v[j]-p0.v[j])*f;
				if (box.axis[i].Contains(x)
					&&
					box.axis[j].Contains(y))
					return true;
			}
		}
	return false;
}


MFUNC2 (bool)		_oTriangulate(const Ctr::ArrayData<M::TVec2<C0> >&vertex_field, Mesh<C1>&target)
{
	return _oTriangulate(vertex_field.pointer(),vertex_field.Count(),target);
}

template <typename ContainerT, typename Index>
	MF_DECLARE	(bool)	_oTriangulate3(const ContainerT&vertex_field, Container::BasicVector<Index>&target)
	{
		target.reset();
		if (vertex_field.size() < 3)
			return false;
		//ReferenceVector<MeshVertex<C1> >	triangles;

		Container::PreAllocVector<Index>	indices;
		/*target.Resize(vertices,0,0,0);*/
		for (index_t i = 0; i < vertex_field.Count(); i++)
		{
			indices<<i;
			/*M::Vec::def(target.vertex_field[i].position,vertex[i].x,vertex[i].y,0);
			target.vertex_field[i].index = i;
			target.vertex_field[i].marked = false;*/
		}

		typedef decltype(vertex_field.operator[](index_t(0)))	Vertex;
		//typedef decltype(vertex_field.operator[](index_t(0)).x)	Float;
		typedef float Float;

		Float size(0);//,delta[2];
		for (index_t i = 0; i < vertex_field.size()-1; i++)
		{
			size += Obj::zeroTriangleSize(vertex_field[i].xy,vertex_field[i+1].xy);
		}
		size += Obj::zeroTriangleSize(vertex_field.Last().xy,vertex_field.First().xy);
		int orientation = M::Sign(size);
		bool failed(false);
		
		index_t		current_index(0),
					walk(0);
		while (indices.Count() > 3 && !failed)
		{
			walk = 0;
			bool looping=true;
			//const Vertex*v0,*v1,*v2;
			Index		index0,index1,index2;

			while (looping)
			{
				walk++;
				if (walk > vertex_field.size())
				{
					//ShowMessage("looped :P");
					looping = false;
					failed = true;
					continue;
				}
				index0 = indices[current_index];
				index1 = indices[(current_index+1)%indices.Count()];
				index2 = indices[(current_index+2)%indices.Count()];
				/*v0 = &(vertex_field[index0]);
				v1 = &(vertex_field[index1]);
				v2 = &(vertex_field[index2]);*/
				current_index++;
				if (current_index >= indices.Count())
					current_index = 0;
				if (M::Sign(Obj::signedTriangleSize(vertex_field[index0].xy,vertex_field[index1].xy,vertex_field[index2].xy)) != orientation)
					continue;
				bool hit(false);
				Float	dummy;
				Vertex &p0 = vertex_field[index2],	&p1 = vertex_field[indices[(current_index+2)%indices.Count()]],	&p2=vertex_field[index0];
				if (M::Sign(Obj::signedTriangleSize(p0.xy,p1.xy,p2.xy)) != orientation)
					continue;

				for (index_t i = 0; i < indices.Count()-1 && !hit; i++)
					if (i <current_index || i > current_index+1)
						hit = Obj::detEdgeIntersection(vertex_field[(indices[i])].xy,vertex_field[(indices[i+1])].xy,vertex_field[index0].xy,vertex_field[index2].xy,dummy,dummy);
				hit = hit || Obj::detEdgeIntersection(vertex_field[(indices.Last())].xy,vertex_field[(indices.First())].xy,vertex_field[index0].xy,vertex_field[index2].xy,dummy,dummy);
				
				looping = hit;
			}
			if (!failed)
			{
				target << index0 << index1 << index2;
				indices.Erase(current_index);
				if (current_index >= indices.Count())
					current_index = 0;
			}
		}
		if (indices.Count() == 3)
		{
			target << indices.First() << indices[1] << indices[2];
		}
		return !failed;
	}

	
MFUNC2 (bool)		_oTriangulate(const M::TVec2<C0>*vertex, count_t vertices, Mesh<C1>&target)
{
	target.clear();
	if (vertices < 3)
		return false;
	Container::PreAllocVector<index_t>					indices;
	Container::PreAllocVector<MeshVertex<C1>*>		triangles;

	target.Resize(vertices,0,0,0);
	for (index_t i = 0; i < vertices; i++)
	{
		indices<<i;
		M::Vec::def(target.vertex_field[i].position,vertex[i].x,vertex[i].y,0);
		target.vertex_field[i].index = i;
		target.vertex_field[i].marked = false;
	}


	C0 size(0);//,delta[2];
	for (index_t i = 0; i < vertices-1; i++)
	{
		MeshVertex<C1>	*current = target.vertex_field+indices[i],
						*next = target.vertex_field+indices[i+1];
		size += Obj::zeroTriangleSize(current->position.xy,next->position.xy);
	}
	size += Obj::zeroTriangleSize(target.vertex_field[indices.Last()].position.xy,target.vertex_field[indices.First()].position.xy);
	char orientation = M::Sign(size);
	bool failed(false);
		
	index_t		current_index(0),
				walk(0);
	while (indices.Count() > 3 && !failed)
	{
		walk = 0;
		bool looping=true;
		MeshVertex<C1>*v0=nullptr,*v1= nullptr,*v2= nullptr;

		while (looping)
		{
			walk++;
			if (walk > vertices)
			{
				//ShowMessage("looped :P");
				looping = false;
				failed = true;
				continue;
			}
			index_t		index0 = indices[current_index],
						index1 = indices[(current_index+1)%indices.Count()],
						index2 = indices[(current_index+2)%indices.Count()];
			v0 = target.vertex_field+index0;
			v1 = target.vertex_field+index1;
			v2 = target.vertex_field+index2;
			current_index++;
			if (current_index >= indices.Count())
				current_index = 0;
			if (M::Sign(Obj::signedTriangleSize(v0->position.xy,v1->position.xy,v2->position.xy)) != orientation)
				continue;
			bool hit(false);
			C0	dummy;
			M::TVec3<typename C1::Type> &p0 = v2->position,&p1 = target.vertex_field[indices[(current_index+2)%indices.Count()]].position,&p2=v0->position;
			if (M::Sign(Obj::signedTriangleSize(p0.xy,p1.xy,p2.xy)) != orientation)
				continue;

			for (index_t i = 0; i < indices.Count()-1 && !hit; i++)
				if (i <current_index || i > current_index+1)
					hit = Obj::detEdgeIntersection(vertex[(indices[i])],vertex[(indices[i+1])],vertex[v0->index],vertex[v2->index],dummy,dummy);
			hit = hit || Obj::detEdgeIntersection(vertex[(indices.Last())],vertex[(indices.First())],vertex[v0->index],vertex[v2->index],dummy,dummy);
				
			looping = hit;
		}
		if (!failed)
		{
			triangles.append(v0);
			triangles.append(v1);
			triangles.append(v2);
			
			indices.Erase(current_index);
			if (current_index >= indices.Count())
				current_index = 0;
		}
	}
	if (indices.Count() == 3)
	{
		triangles.append(target.vertex_field+indices.First());
		triangles.append(target.vertex_field+indices[1]);
		triangles.append(target.vertex_field+indices[2]);
	}
	target.triangle_field.SetSize(triangles.Count()/3);
	for (index_t i = 0; i < target.triangle_field.GetLength(); i++)
	{
		target.triangle_field[i].v0 = triangles[i*3];
		target.triangle_field[i].v1 = triangles[i*3+1];
		target.triangle_field[i].v2 = triangles[i*3+2];
		target.triangle_field[i].index = i;
		target.triangle_field[i].marked = false;
	}
	return !failed;
}
	
	
template <class Def>
	MF_DECLARE (String)		TMeshFaceLink<Def>::ToString(bool full)	const
	{
		if (!triangle)
			return "NULL";
		return is_quad?quad->ToString(full):triangle->ToString(full);
	}
	
template <class Def>
	MF_DECLARE (char) TMeshFaceLink<Def>::GetIndexOf(const MeshVertex<Def>*vertex)	const
	{
		if (!triangle)
			return -1;
		if (is_quad)
			return quad->GetIndexOf(vertex);
		return triangle->GetIndexOf(vertex);
	}
		
template <class Def>
	MF_DECLARE (char) TMeshFaceLink<Def>::GetIndexOf(const MeshEdge<Def>*edge)	const	//if supported
	{
		if (!triangle)
			return -1;
		if (is_quad)
			return quad->GetIndexOf(edge);
		return triangle->GetIndexOf(edge);
	}
		
template <class Def>
	MF_DECLARE (BYTE) TMeshFaceLink<Def>::requireIndexOf(const MeshVertex<Def>*vertex)	const
	{
		if (!triangle)
			FATAL__("requesting index of vertex but link is undefined");
		if (is_quad)
			return quad->requireIndexOf(vertex);
		return triangle->requireIndexOf(vertex);
	}
		
template <class Def>
	MF_DECLARE (BYTE) TMeshFaceLink<Def>::requireIndexOf(const MeshEdge<Def>*edge)	const	//if supported
	{
		if (!triangle)
			FATAL__("requesting index of vertex but link is undefined");
		if (is_quad)
			return quad->requireIndexOf(edge);
		return triangle->requireIndexOf(edge);
	}
	
template <class Def>
	MF_DECLARE (bool)		TMeshFaceLink<Def>::marked()	const
	{
		if (!triangle)
			return false;
		if (is_quad)
			return quad->marked;
		return triangle->marked;
	}
	
template <class Def>
	MF_DECLARE (void)		TMeshFaceLink<Def>::mark()
	{
		if (!triangle)
			return;
		if (is_quad)
			quad->marked = true;
		else
			triangle->marked = true;
	}

template <class Def>
	MF_DECLARE	(const TMeshFaceLink<Def>&)	TMeshFaceLink<Def>::neighbor(BYTE index)	const
	{
		if (triangle)
			if (is_quad)
				return quad->n[index];
			else
				return triangle->n[index];
			
		static TMeshFaceLink<Def>	empty;
		empty.Unset();
		return empty;
	}
	
template <class Def> MFUNC1
	(void) TMeshFaceLink<Def>::unsetNeighbor(C0*t)
	{
		if (!triangle)
			return;
		if (is_quad)
		{
			if (quad->n0.triangle == (MeshTriangle<Def>*)t)
			{
				quad->n0.Unset();
				return;
			}
			if (quad->n1.triangle == (MeshTriangle<Def>*)t)
			{
				quad->n1.Unset();
				return;
			}
			if (quad->n2.triangle == (MeshTriangle<Def>*)t)
			{
				quad->n2.Unset();
				return;
			}
			if (quad->n3.triangle == (MeshTriangle<Def>*)t)
			{
				quad->n3.Unset();
				return;
			}
			return;
		}
		if (triangle->n0.triangle == (MeshTriangle<Def>*)t)
		{
			triangle->n0.Unset();
			return;
		}
		if (triangle->n1.triangle == (MeshTriangle<Def>*)t)
		{
			triangle->n1.Unset();
			return;
		}
		if (triangle->n2.triangle == (MeshTriangle<Def>*)t)
		{
			triangle->n2.Unset();
			return;
		}
	}

template <class Def> MFUNC2
	(void) TMeshFaceLink<Def>::replaceNeighbor(C0*t,C1*with)
	{
		if (!triangle)
			return;
		if (is_quad)
		{
			if (quad->n0.triangle == (MeshTriangle<Def>*)t)
			{
				quad->n0.Set(with);
				return;
			}
			if (quad->n1.triangle == (MeshTriangle<Def>*)t)
			{
				quad->n1.Set(with);
				return;
			}
			if (quad->n2.triangle == (MeshTriangle<Def>*)t)
			{
				quad->n2.Set(with);
				return;
			}
			if (quad->n3.triangle == (MeshTriangle<Def>*)t)
			{
				quad->n3.Set(with);
				return;
			}
			return;
		}
		if (triangle->n0.triangle == (MeshTriangle<Def>*)t)
		{
			triangle->n0.Set(with);
			return;
		}
		if (triangle->n1.triangle == (MeshTriangle<Def>*)t)
		{
			triangle->n1.Set(with);
			return;
		}
		if (triangle->n2.triangle == (MeshTriangle<Def>*)t)
		{
			triangle->n2.Set(with);
			return;
		}
	}
		
template <class Def> MFUNC1
	(void) TMeshFaceLink<Def>::replaceNeighbor(C0*t,const TMeshFaceLink<Def>&with)
	{
		if (!triangle)
			return;
		if (is_quad)
		{
			if (quad->n0.triangle == (MeshTriangle<Def>*)t)
			{
				quad->n0 = with;
				return;
			}
			if (quad->n1.triangle == (MeshTriangle<Def>*)t)
			{
				quad->n1 = with;
				return;
			}
			if (quad->n2.triangle == (MeshTriangle<Def>*)t)
			{
				quad->n2 = with;
				return;
			}
			if (quad->n3.triangle == (MeshTriangle<Def>*)t)
			{
				quad->n3 = with;
				return;
			}
			return;
		}
		if (triangle->n0.triangle == (MeshTriangle<Def>*)t)
		{
			triangle->n0 = with;
			return;
		}
		if (triangle->n1.triangle == (MeshTriangle<Def>*)t)
		{
			triangle->n1 = with;
			return;
		}
		if (triangle->n2.triangle == (MeshTriangle<Def>*)t)
		{
			triangle->n2 = with;
			return;
		}
	}
	
template <class Def>
	MF_DECLARE	(const TMeshFaceLink<Def>&)	TMeshFaceLink<Def>::getNextAround(const MeshVertex<Def>*vertex)	const
	{
		if (triangle)
			if (is_quad)
			{
				char index = quad->GetIndexOf(vertex);
				if (index != -1)
					return quad->next[index];
			}
			else
			{
				char index = triangle->GetIndexOf(vertex);
				if (index != -1)
					return triangle->next[index];
			}
			
			
		static TMeshFaceLink<Def>	empty;
		empty.Unset();
		return empty;
	}

template <class Def>
	MF_DECLARE	(const TMeshFaceLink<Def>&)	TMeshFaceLink<Def>::getNextAround(BYTE index)	const
	{
		if (triangle)
			if (is_quad)
				return quad->next[index];
			else
				return triangle->next[index];
		
			
		static TMeshFaceLink<Def>	empty;
		empty.Unset();
		return empty;
	}


template <class Def> MFUNC1	(void) TMeshFaceLink<Def>::setNeighbor(MeshVertex<Def>*v,C0*face)
	{
		if (!triangle)
			return;
		if (is_quad)
		{
			char index = quad->GetIndexOf(v);
			if (index != -1)
				quad->n[index].Set(face);
		}
		else
		{
			char index = triangle->GetIndexOf(v);
			if (index != -1)
				triangle->n[index].Set(face);
		}
	}

	
template <class Def>
	MF_DECLARE (void) TMeshFaceLink<Def>::Set(MeshQuad<Def>*q)
	{
		quad = q;
		is_quad = true;
	}
	
template <class Def>
	MF_DECLARE (void) TMeshFaceLink<Def>::Set(MeshTriangle<Def>*t)
	{
		triangle = t;
		is_quad = false;
	}
	
template <class Def>
	MF_DECLARE (void) TMeshFaceLink<Def>::Unset()
	{
		triangle = NULL;
	}

	

template <class Def> MF_DECLARE (char) MeshEdge<Def>::GetIndexOf(const MeshVertex<Def>*v)	const
{
	if (v0 == v)
		return 0;
	if (v1 == v)
		return 1;
	return -1;
}
	
template <class Def> MF_DECLARE (char) MeshEdge<Def>::GetIndexOf(const MeshTriangle<Def>*t)		const
{
	if (n[0].triangle == t)
		return 0;
	if (n[1].triangle == t)
		return 1;
	return -1;	
}
	
template <class Def> MF_DECLARE (char) MeshEdge<Def>::GetIndexOf(const MeshQuad<Def>*t)	const
{
	if (n[0].quad == t)
		return 0;
	if (n[1].quad == t)
		return 1;
	return -1;	
}
	
template <class Def> MF_DECLARE (BYTE) MeshEdge<Def>::requireIndexOf(const MeshVertex<Def>*v)	const
{
	if (v0 == v)
		return 0;
	if (v1 == v)
		return 1;
	FATAL__("index lookup failed");
	return 0;
}
	
template <class Def> MF_DECLARE (BYTE) MeshEdge<Def>::requireIndexOf(const MeshTriangle<Def>*t)		const
{
	if (n[0].triangle == t)
		return 0;
	if (n[1].triangle == t)
		return 1;
	FATAL__("index lookup failed");
	return 0;
}
	
template <class Def> MF_DECLARE (BYTE) MeshEdge<Def>::requireIndexOf(const MeshQuad<Def>*t)	const
{
	if (n[0].quad == t)
		return 0;
	if (n[1].quad == t)
		return 1;
	FATAL__("index lookup failed");
	return 0;
}	
	
template <class Def> MF_DECLARE (char) MeshEdge<Def>::FindAndReplace(MeshVertex<Def>*v, MeshVertex<Def>*with)
{
	if (v0 == v)
	{
		v0 = with;
		return 0;
	}
	if (v1 == v)
	{
		v1 = with;
		return 1;
	}
	return -1;
}
	
template <class Def> MF_DECLARE (char) MeshEdge<Def>::FindAndReplace(MeshQuad<Def>*v, MeshQuad<Def>*with)
{
	if (n[0].quad == v)
	{
		n[0].quad = with;
		n[0].is_quad = true;
		if (!with)
		{
			n[0] = n[1];
			n[1].triangle = NULL;
		}
		return 0;
	}
	if (n[1].quad == v)
	{
		n[1].quad = with;
		n[1].is_quad = true;
		return 1;
	}
	return -1;
}
	
template <class Def> MF_DECLARE (char) MeshEdge<Def>::FindAndReplace(MeshQuad<Def>*v, MeshTriangle<Def>*with)
{
	if (n[0].quad == v)
	{
		n[0].triangle = with;
		n[0].is_quad = false;
		if (!with)
		{
			n[0] = n[1];
			n[1].triangle = NULL;
		}
		return 0;
	}
	if (n[1].quad == v)
	{
		n[1].triangle = with;
		n[1].is_quad = false;
		return 1;
	}
	return -1;
}
	
template <class Def> MF_DECLARE (char) MeshEdge<Def>::FindAndReplace(MeshTriangle<Def>*v, MeshTriangle<Def>*with)
{
	if (n[0].triangle == v)
	{
		n[0].triangle = with;
		n[0].is_quad = false;
		if (!with)
		{
			n[0] = n[1];
			n[1].triangle = NULL;
		}
		return 0;
	}
	if (n[1].triangle == v)
	{
		n[1].triangle = with;
		n[1].is_quad = false;
		return 1;
	}
	return -1;
}
	
	
	
template <class Def> MF_DECLARE (char) MeshEdge<Def>::FindAndReplace(MeshTriangle<Def>*v, MeshQuad<Def>*with)
{
	if (n[0].triangle == v)
	{
		n[0].quad = with;
		n[0].is_quad = true;
		if (!with)
		{
			n[0] = n[1];
			n[1].triangle = NULL;
		}
		return 0;
	}
	if (n[1].triangle == v)
	{
		n[1].quad = with;
		n[1].is_quad = true;
		return 1;
	}
	return -1;
}


template <class Def> MF_DECLARE (char) MeshTriangle<Def>::GetIndexOf(const MeshVertex<Def>*v)	const
{
	if (v0 == v)
		return 0;
	if (v1 == v)
		return 1;
	if (v2 == v)
		return 2;
	return -1;
}
	
template <class Def> MF_DECLARE (BYTE) MeshTriangle<Def>::requireIndexOf(const MeshVertex<Def>*v)	const
{
	if (v0 == v)
		return 0;
	if (v1 == v)
		return 1;
	if (v2 == v)
		return 2;
	FATAL__("index lookup failed");
	return 0;
}
	
template <class Def> MF_DECLARE (char) MeshTriangle<Def>::FindAndReplace(MeshVertex<Def>*v, MeshVertex<Def>*with)
{
	if (v0 == v)
	{
		v0 = with;
		return 0;
	}
	if (v1 == v)
	{
		v1 = with;
		return 1;
	}
	if (v2 == v)
	{
		v2 = with;
		return 2;
	}
	return -1;
}
	
template <class Def> MF_DECLARE (char) MeshQuad<Def>::GetIndexOf(const MeshVertex<Def>*v)	const
{
	if (v0 == v)
		return 0;
	if (v1 == v)
		return 1;
	if (v2 == v)
		return 2;
	if (v3 == v)
		return 3;
	return -1;
}
	
template <class Def> MF_DECLARE (BYTE) MeshQuad<Def>::requireIndexOf(const MeshVertex<Def>*v)	const
{
	if (v0 == v)
		return 0;
	if (v1 == v)
		return 1;
	if (v2 == v)
		return 2;
	if (v3 == v)
		return 3;
	FATAL__("index lookup failed");
	return 0;
}
	
	
template <class Def> MF_DECLARE (char) MeshQuad<Def>::FindAndReplace(MeshVertex<Def>*v, MeshVertex<Def>*with)
{
	if (v0 == v)
	{
		v0 = with;
		return 0;
	}
	if (v1 == v)
	{
		v1 = with;
		return 1;
	}
	if (v2 == v)
	{
		v2 = with;
		return 2;
	}
	if (v3 == v)
	{
		v3 = with;
		return 3;
	}
	return -1;
}
	
template <class Def>
	MF_CONSTRUCTOR TGraphDefVertex<Def>::TGraphDefVertex():first(NULL),last(NULL),degree(0)
	{}
		
template <class Def>
	MF_DECLARE	(void)	TGraphDefVertex<Def>::Insert(MeshEdge<TGraphDef<Def> >*edge)
	{
		degree++;
		if (last)
		{
			char index = last->GetIndexOf((const MeshVertex<TGraphDef<Def> >*)this);
			if (index == -1)
				FATAL__("linkage broken");
			last->next[index] = edge;
			last = edge;
		}
		else
			first = last = edge;
		char index = edge->GetIndexOf((const MeshVertex<TGraphDef<Def> >*)this);
		if (index == -1)
			FATAL__("bad insert. edge is not linked to local vertex");
		edge->next[index] = NULL;
	}
		
template <class Def>
	MF_DECLARE (MeshEdge<TGraphDef<Def> >*)	TGraphDefVertex<Def>::findEdgeTo(MeshVertex<TGraphDef<Def> >*vertex)	const
	{
		MeshEdge<TGraphDef<Def> >	*current = first;

		while (current)
		{
			char index = current->GetIndexOf((const MeshVertex<TGraphDef<Def> >*)this);
			if (index == -1)
				FATAL__("linkage broken");
			if (current->vertex[!index] == vertex)
				return current;
			current = current->next[index];
		}
		return NULL;
	}
	
template <class Def>
	MF_CONSTRUCTOR TFaceGraphDefVertex<Def>::TFaceGraphDefVertex():degree(0)
	{
		first.triangle = NULL;
		last.triangle = NULL;
	}
		
template <class Def>
	MF_DECLARE (void)	TFaceGraphDefVertex<Def>::Insert(MeshQuad<TFaceGraphDef<Def> >*quad)
	{
		degree++;
		if (last.triangle)
		{
			if (last.is_quad)
			{
				BYTE index = last.quad->requireIndexOf((const MeshVertex<TFaceGraphDef<Def> >*)this);
				last.quad->next[index].Set(quad);
				last = last.quad->next[index];
			}
			else
			{
				BYTE index = last.triangle->requireIndexOf((const MeshVertex<TFaceGraphDef<Def> >*)this);
				last.triangle->next[index].Set(quad);
				last = last.triangle->next[index];
			}
		}
		else
		{
			first.Set(quad);
			last = first;
		}

		BYTE index = quad->requireIndexOf((const MeshVertex<TFaceGraphDef<Def> >*)this);
		quad->next[index].triangle = NULL;
	}
		
template <class Def>
	MF_DECLARE (void)	TFaceGraphDefVertex<Def>::Insert(MeshTriangle<TFaceGraphDef<Def> >*triangle)
	{
		degree++;
		if (last.triangle)
		{
			if (last.is_quad)
			{
				BYTE index = last.quad->requireIndexOf((const MeshVertex<TFaceGraphDef<Def> >*)this);
				last.quad->next[index].Set(triangle);
				last = last.quad->next[index];
			}
			else
			{
				BYTE index = last.triangle->requireIndexOf((const MeshVertex<TFaceGraphDef<Def> >*)this);
				last.triangle->next[index].Set(triangle);
				last = last.triangle->next[index];
			}
		}
		else
		{
			first.Set(triangle);
			last = first;
		}
			
		BYTE index = triangle->requireIndexOf((const MeshVertex<TFaceGraphDef<Def> >*)this);
		triangle->next[index].triangle = NULL;
	}
		
		
template <class Def>
	MF_DECLARE (TMeshFaceLink<TFaceGraphDef<Def> >) TFaceGraphDefVertex<Def>::findFace(const MeshVertex<TFaceGraphDef<Def> >*v) const
	{
		FaceLink	current = first;
		while (current)
		{
			if (current.GetIndexOf(v) != -1)
				return current;
			current = current.getNextAround((const MeshVertex<TFaceGraphDef<Def> >*)this);
		}
		FaceLink result;
		result.Unset();
		return result;
	}
	
template <class Def>
	MF_DECLARE (TMeshFaceLink<TFaceGraphDef<Def> >) TFaceGraphDefVertex<Def>::findFace(const MeshVertex<TFaceGraphDef<Def> >*v, BYTE index) const
	{
		FaceLink	current = first;
		while (current)
		{
			if (current.is_quad)
			{
				if (current.quad->vertex[index] == v)
					return current;
			}
			else
				if (current.triangle->vertex[index] == v)
					return current;
			current = current.getNextAround((const MeshVertex<TFaceGraphDef<Def> >*)this);
		}
		FaceLink result;
		result.Unset();
		return result;
	}
		
		
template <class Def>
	MF_DECLARE (bool)	TFaceGraphDefVertex<Def>::unlink(MeshTriangle<TFaceGraphDef<Def> >*triangle)
	{
		TMeshFaceLink<TFaceGraphDef<Def> >	link = first,
											prev;
		if (!link.triangle)
			return false;
		BYTE index = triangle->requireIndexOf((const MeshVertex<TFaceGraphDef<Def> >*)this);
		prev.triangle = NULL;
		while (link)
		{
			if (link.triangle == triangle)
			{
				if (prev.triangle)
					if (prev.is_quad)
						prev.quad->next[prev.quad->requireIndexOf((const MeshVertex<TFaceGraphDef<Def> >*)this)] = triangle->next[index];
					else
						prev.triangle->next[prev.triangle->requireIndexOf((const MeshVertex<TFaceGraphDef<Def> >*)this)] = triangle->next[index];
				else
					first = triangle->next[index];
				triangle->next[index].Unset();
				if (last.triangle == link.triangle)
					last = prev;
				degree--;
				return true;
			}
			prev = link;
			if (link.is_quad)
				link = link.quad->next[link.quad->requireIndexOf((const MeshVertex<TFaceGraphDef<Def> >*)this)];
			else
				link = link.triangle->next[link.triangle->requireIndexOf((const MeshVertex<TFaceGraphDef<Def> >*)this)];
		}
		return false;
	}
		
template <class Def>
	MF_DECLARE (bool)	TFaceGraphDefVertex<Def>::unlink(MeshQuad<TFaceGraphDef<Def> >*quad)
	{
		TMeshFaceLink<TFaceGraphDef<Def> >	link = first,
											prev;
		if (!link.triangle)
			return false;
		BYTE index = quad->requireIndexOf((const MeshVertex<TFaceGraphDef<Def> >*)this);
		prev.triangle = NULL;
		while (link.triangle)
		{
			if (link.quad == quad)
			{
				if (prev.triangle)
					if (prev.is_quad)
						prev.quad->next[prev.quad->requireIndexOf((const MeshVertex<TFaceGraphDef<Def> >*)this)] = quad->next[index];
					else
						prev.triangle->next[prev.triangle->requireIndexOf((const MeshVertex<TFaceGraphDef<Def> >*)this)] = quad->next[index];
				else
					first = quad->next[index];
				quad->next[index].triangle = NULL;
				if (last.triangle == link.triangle)
					last = prev;
				degree--;
				return true;
			}
			prev = link;
			if (link.is_quad)
				link = link.quad->next[link.quad->requireIndexOf((const MeshVertex<TFaceGraphDef<Def> >*)this)];
			else
				link = link.triangle->next[link.triangle->requireIndexOf((const MeshVertex<TFaceGraphDef<Def> >*)this)];
		}
		return false;
	}
		
template <class Def>
	MF_DECLARE (void)		_oUnlinkFromVertices(MeshTriangle<TFaceGraphDef<Def> >*triangle)
	{
		triangle->v0->unlink(triangle);
		triangle->v1->unlink(triangle);
		triangle->v2->unlink(triangle);
	}
	
template <class Def>
	MF_DECLARE (void)		_oUnlinkFromVertices(MeshQuad<TFaceGraphDef<Def> >*quad)
	{
		quad->v0->unlink(quad);
		quad->v1->unlink(quad);
		quad->v2->unlink(quad);
		quad->v3->unlink(quad);
	}
	
template <class Def>
	MF_DECLARE (void)		_oUnlinkFromVertices(MeshTriangle<TFaceGraphDef<Def> >*triangle, MeshVertex<TFaceGraphDef<Def> >*except)
	{
		if (triangle->v0 != except)
			triangle->v0->unlink(triangle);
		if (triangle->v1 != except)
			triangle->v1->unlink(triangle);
		if (triangle->v2 != except)
			triangle->v2->unlink(triangle);
	}
	
template <class Def>
	MF_DECLARE (void)		_oUnlinkFromVertices(MeshQuad<TFaceGraphDef<Def> >*quad, MeshVertex<TFaceGraphDef<Def> >*except)
	{
		if (quad->v0 != except)
			quad->v0->unlink(quad);
		if (quad->v1 != except)
			quad->v1->unlink(quad);
		if (quad->v2 != except)
			quad->v2->unlink(quad);
		if (quad->v3 != except)
			quad->v3->unlink(quad);
	}
	
#ifndef __CUDACC__
template <class Def>
	MF_DECLARE (void)		_oUnlink(MeshEdge<TGraphDef<Def> >*edge)
	{
		if (edge->v0)
			edge->v0->unlink(edge);
		if (edge->v1)
			edge->v1->unlink(edge);
		edge->n[0].triangle = NULL;
		edge->n[1].triangle = NULL;
	}
#endif
	
		
template <class Def>
	MF_DECLARE (void)		_oLinkToVertices(MeshEdge<TGraphDef<Def> >*edge)
	{
		edge->v0->Insert(edge);
		edge->v1->Insert(edge);
	}
	
template <class Def>
	MF_DECLARE (void)		_oLinkToVertices(MeshTriangle<TFaceGraphDef<Def> >*triangle)
	{
		triangle->v0->Insert(triangle);
		triangle->v1->Insert(triangle);
		triangle->v2->Insert(triangle);
	}

#ifndef	__CUDACC__
template <class Def>
	MF_DECLARE (void)		_oLinkToVertices(MeshQuad<TFaceGraphDef<Def> >*quad)
	{
		quad->v0->Insert(quad);
		quad->v1->Insert(quad);
		quad->v2->Insert(quad);
		quad->v3->Insert(quad);
	}
#endif
	
template <class Def>
	MF_DECLARE (void)		MeshEdge<Def>::linkUnary(MeshTriangle<Def>*triangle)
	{
		n[0].triangle = triangle;
		n[0].is_quad = false;
		n[1].triangle = NULL;
	}
		
template <class Def>
	MF_DECLARE (void)		MeshEdge<Def>::linkUnary(MeshQuad<Def>*quad)
	{
		n[0].quad = quad;
		n[0].is_quad = true;
		n[1].triangle = NULL;
	}
		
		
template <class Def>
	MF_DECLARE (bool) TFaceGraphDefVertex<Def>::walkLeft(TMeshFaceLink<TFaceGraphDef<Def> > face, Ctr::ArrayData<TMeshFaceLink<TFaceGraphDef<Def> > >&fbuffer, count_t&fcnt)
	{
		while (face && !face.marked())
		{
			char index1 = face.GetIndexOf((MeshVertex<TFaceGraphDef<Def> >*)this);
			if (index1 == -1)	//path lost
				FATAL__("path lost. "+face.ToString()+" does not contain v"+IntToStr(((MeshVertex<TFaceGraphDef<Def> >*)this)->index));

			fbuffer[fcnt++] = face;
			face.mark();
			if (face.is_quad)
				face = face.quad->n[(index1+3)%4];
			else
				face = face.triangle->n[(index1+2)%3];
		}
		return true;
	}					
	
#ifndef	__CUDACC__

template <class Def>
	MF_DECLARE (count_t)	TFaceGraphDefVertex<Def>::bypass(DynamicMesh<TFaceGraphDef<Def> >&object)
	{
		typedef	MeshEdge<TFaceGraphDef<Def> >		Edge;
		typedef	MeshQuad<TFaceGraphDef<Def> >		Quad;
		typedef	MeshTriangle<TFaceGraphDef<Def> >	Triangle;
		typedef	MeshVertex<TFaceGraphDef<Def> >		Vertex;
		typedef TMeshFaceLink<TFaceGraphDef<Def> >	Link;
			
		if (!degree)
			return 0;
			
		Vertex*self = (Vertex*)this;
		//bool debug = true;
		//cout << "bypassing v"<<self->index<<endl;
			
		count_t result = 0;
		static Ctr::Array<Link>	fbuffer;
			
		if (fbuffer.GetLength() < degree*2)
			fbuffer.SetSize(degree*2);
		const count_t offset = degree;
		count_t fentries(0);	//number of faces that lack a right neighbor
		Link current = first;	//try to find islands
		while (current)
		{
// 				cout << "linked: "<<current.ToString()<<endl;
			BYTE vindex = current.requireIndexOf((Vertex*)this);
			if (!current.marked() && !current.neighbor(vindex))	//left oriented edge
			{
// 					cout << "open: "<<current.ToString()<<endl;
				fbuffer[offset+fentries++] = current;
			}
			current = current.getNextAround(vindex);
		}
			
		for (index_t i = 0; i < fentries; i++)
		{
			current = fbuffer[offset+i];
// 				cout << "processing "<<current.ToString()<<endl;
			char vindex = current.GetIndexOf((Vertex*)this);
			if (vindex < 0 || current.marked())
				continue;
			count_t	fcnt = 0;
			if (!walkLeft(current,fbuffer,fcnt))	//found island
				FATAL__("orientation error ("+IntToStr(result)+")");
// 				if (debug)
// 				{
// 					cout << "faces found: "<<endl;
// 					for (unsigned i = 0; i < fcnt; i++)
// 						cout << " - "<<fbuffer[i].ToString()<<endl;
// 				}
			if (fcnt)
			{
				Vertex*pivot;
				if (fbuffer[0].is_quad)
				{
					Quad*q = fbuffer[0].quad;
// 						cout << q->ToString()<<" unset in "<<__LINE__<<endl;
						
					BYTE me = q->requireIndexOf((Vertex*)this);
					pivot = q->vertex[(me+1)%4];
					Triangle*t = object.triangles.Append();
					t->index = object.triangles-1;
					t->marked = false;
					t->attrib = 0;
					t->v0 = pivot;
					t->v1 = q->vertex[(me+2)%4];
					t->v2 = q->vertex[(me+3)%4];
						
					t->n0 = q->n[(me+1)%4];
					t->n1 = q->n[(me+2)%4];
					t->n2 = q->n[(me+3)%4];
					t->n0.replaceNeighbor(q,t);
					t->n1.replaceNeighbor(q,t);
					t->n2.replaceNeighbor(q,t);
					q->n[me].unsetNeighbor(q);
						
// 						cout << t->ToString()<<" created in "<<__LINE__<<" replacing "<<q->ToString()<<endl;
// 						cout << "  neighbors: \n    "<<t->n0.ToString()<<"\n    "<<t->n1.ToString()<<"\n    "<<t->n2.ToString()<<endl;
						
					_oUnlinkFromVertices(q,(Vertex*)this);
					_oLinkToVertices(t);
						
				}
				else
				{
					Triangle*t = fbuffer[0].triangle;
// 						cout << t->ToString()<<" unset in "<<__LINE__<<endl;
					BYTE me = t->requireIndexOf((Vertex*)this);
					t->n[(me+2)%3].replaceNeighbor(t,t->n[(me+1)%3]);
					t->n[(me+1)%3].replaceNeighbor(t,t->n[(me+2)%3]);
					t->n[me].unsetNeighbor(t);
					pivot = t->vertex[(me+1)%3];
					_oUnlinkFromVertices(t,(Vertex*)this);
				}
				result++;
					
				for (index_t i = 1; i < fcnt; i++)
				{
					if (fbuffer[i].is_quad)
					{
						Quad*q = fbuffer[i].quad;
// 							cout << q->ToString()<<" processed in "<<__LINE__<<endl;
						BYTE index = q->requireIndexOf((Vertex*)this);
						q->vertex[index]->unlink(q);
						q->vertex[index] = pivot;
						pivot->Insert(q);
						q->marked = false;
					}
					else
					{
						Triangle*t = fbuffer[i].triangle;
// 							cout << t->ToString()<<" processed in "<<__LINE__<<endl;
						BYTE index = t->requireIndexOf((Vertex*)this);
						t->vertex[index]->unlink(t);
						t->vertex[index] = pivot;
						pivot->Insert(t);
						t->marked = false;
					}
				}
			}
			//if (!object.checkIntegrity())
				//FATAL__("integrity exception");
			
		}
			
			
		if (!fentries)
		{
			count_t fcnt = 0;
			if (!walkLeft(first,fbuffer,fcnt))	//found fan
				FATAL__("orientation error");
				
			typename Def::Type	weight = 0;
			index_t begin = 0;
			for (index_t i = 0; i < fcnt; i++)
				if (fbuffer[i].is_quad)
				{
					BYTE me = fbuffer[i].quad->requireIndexOf((Vertex*)this);
					if (fbuffer[i].quad->vertex[(me+1)%4]->weight > weight)
					{
						weight = fbuffer[i].quad->vertex[(me+1)%4]->weight;
						begin = i;
					}
				}
				else
				{
					BYTE me = fbuffer[i].triangle->requireIndexOf((Vertex*)this);
					
					if (fbuffer[i].triangle->vertex[(me+1)%3]->weight > weight)
					{
						weight = fbuffer[i].triangle->vertex[(me+1)%3]->weight;
						begin = i;
					}
				}
							
					
				
			if (fcnt)
			{
				Vertex*pivot;
				Edge*previous_edge;
				if (fbuffer[begin].is_quad)
				{
					Quad*q = fbuffer[begin].quad;
// 						cout << q->ToString()<<" processed in "<<__LINE__<<endl;
					BYTE me = q->requireIndexOf((Vertex*)this);
					pivot = q->vertex[(me+1)%4];
					Triangle*t = object.triangles.Append();
					t->index = object.triangles-1;
					t->marked = false;
					t->attrib = 0;
					t->v0 = pivot;
					t->v1 = q->vertex[(me+2)%4];
					t->v2 = q->vertex[(me+3)%4];
						
					t->n0 = q->n[(me+1)%4];
					t->n1 = q->n[(me+2)%4];
					t->n2 = q->n[(me+3)%4];
					t->n0.replaceNeighbor(q,t);
					t->n1.replaceNeighbor(q,t);
					t->n2.replaceNeighbor(q,t);
					q->n[me].unsetNeighbor(q);
					
// 						cout << t->ToString()<<" created in "<<__LINE__<<" replacing "<<q->ToString()<<endl;
						
					_oUnlinkFromVertices(q,(Vertex*)this);
					_oLinkToVertices(t);
				}
				else
				{
					Triangle*t = fbuffer[begin].triangle;
// 						cout << t->ToString()<<" unset in "<<__LINE__<<endl;
					BYTE me = t->requireIndexOf((Vertex*)this);
					t->n[(me+2)%3].replaceNeighbor(t,t->n[(me+1)%3]);
					t->n[(me+1)%3].replaceNeighbor(t,t->n[(me+2)%3]);
					t->n[me].unsetNeighbor(t);	//?
					pivot = t->vertex[(me+1)%3];
						
					_oUnlinkFromVertices(t,(Vertex*)this);
				}
				result+=2;
				for (index_t j = 1; j < fcnt-1; j++)
				{
					index_t i = (j+begin)%fcnt;
					if (fbuffer[i].is_quad)
					{
						Quad*q = fbuffer[i].quad;
// 							cout << q->ToString()<<" processed in "<<__LINE__<<endl;
						BYTE index = q->requireIndexOf((Vertex*)this);
						q->vertex[index]->unlink(q);
						q->vertex[index] = pivot;
						pivot->Insert(q);
						q->marked = false;
					}
					else
					{
						Triangle*t = fbuffer[i].triangle;
// 							cout << t->ToString()<<" processed in "<<__LINE__<<endl;
						BYTE index = t->requireIndexOf((Vertex*)this);
						t->vertex[index]->unlink(t);
						t->vertex[index] = pivot;
						pivot->Insert(t);
						t->marked = false;
					}
				}
				index_t end = (begin+fcnt-1)%fcnt;
				if (fbuffer[end].is_quad)
				{
					Quad*q = fbuffer[end].quad;
// 						cout << q->ToString()<<" processed in "<<__LINE__<<endl;
					BYTE me = q->requireIndexOf((Vertex*)this);
					Triangle*t = object.triangles.Append();
					t->index = object.triangles-1;
					t->marked = false;
					t->attrib = 0;
					t->v0 = pivot;
					t->v1 = q->vertex[(me+1)%4];
					t->v2 = q->vertex[(me+2)%4];
						
					t->n0 = q->n[me];
					t->n1 = q->n[(me+1)%4];
					t->n2 = q->n[(me+2)%4];
					t->n0.replaceNeighbor(q,t);
					t->n1.replaceNeighbor(q,t);
					t->n2.replaceNeighbor(q,t);
					q->n[(me+3)%4].unsetNeighbor(q);
					
// 						cout << t->ToString()<<" created in "<<__LINE__<<" replacing "<<q->ToString()<<endl;

						
					_oUnlinkFromVertices(q,(Vertex*)this);
					_oLinkToVertices(t);
				}
				else
				{
					Triangle*t = fbuffer[end].triangle;
// 						cout << t->ToString()<<" unset in "<<__LINE__<<endl;
					BYTE me = t->requireIndexOf((Vertex*)this);
					t->n[me].replaceNeighbor(t,t->n[(me+1)%3]);
					t->n[(me+1)%3].replaceNeighbor(t,t->n[me]);
					t->n[(me+2)%3].unsetNeighbor(t);
					_oUnlinkFromVertices(t,(Vertex*)this);
				}
			}
			//if (!object.checkIntegrity())
				//FATAL__("integrity exception");
		}
		first.Unset();
		last.Unset();
		degree = 0;
		return result;
	}
#endif
	
template <class Def>
	MF_DECLARE	(String)		TFaceGraphDefTriangle<Def>::ToString(bool full)	const
	{
 		typedef MeshVertex<TFaceGraphDef<Def> >		Vertex;
 		typedef MeshTriangle<TFaceGraphDef<Def> >	Triangle;
 		typedef MeshQuad<TFaceGraphDef<Def> >		Quad;
 		const Triangle*t = (const Triangle*)this;
 		if (!full)
 			return "t"+IntToStr(t->index);
 		return "t"+IntToStr(t->index)+" {v"+IntToStr(t->v0->index)+", v"+IntToStr(t->v1->index)+", v"+IntToStr(t->v2->index)+"} {"+n0.ToString(false)+", "+n1.ToString(false)+", "+n2.ToString(false)+"}";
	}
	
template <class Def>
	MF_DECLARE	(String)		TFaceGraphDefQuad<Def>::ToString(bool full)	const
	{
 		typedef MeshVertex<TFaceGraphDef<Def> >		Vertex;
 		typedef MeshTriangle<TFaceGraphDef<Def> >	Triangle;
 		typedef MeshQuad<TFaceGraphDef<Def> >		Quad;
 		const Quad*t = (const Quad*)this;
 		if (!full)
 			return "q"+IntToStr(t->index);
 		return "q"+IntToStr(t->index)+" {v"+IntToStr(t->v0->index)+", v"+IntToStr(t->v1->index)+", v"+IntToStr(t->v2->index)+", v"+IntToStr(t->v3->index)+"} {"+n0.ToString(false)+", "+n1.ToString(false)+", "+n2.ToString(false)+", "+n3.ToString(false)+"}";
	}

	
template <class Def> MF_DECLARE (char) TFaceGraphDefTriangle<Def>::GetIndexOf(const MeshQuad<TFaceGraphDef<Def> >*q)	const
{
	if (n0.quad == q)
		return 0;
	if (n1.quad == q)
		return 1;
	if (n2.quad == q)
		return 2;
	return -1;
}
	
template <class Def> MF_DECLARE (BYTE) TFaceGraphDefTriangle<Def>::requireIndexOf(const MeshQuad<TFaceGraphDef<Def> >*q)	const
{
	if (n0.quad == q)
		return 0;
	if (n1.quad == q)
		return 1;
	if (n2.quad == q)
		return 2;
	FATAL__("linkage broken");
}
	
template <class Def> MF_DECLARE (char) TFaceGraphDefTriangle<Def>::GetIndexOf(const MeshTriangle<TFaceGraphDef<Def> >*t)		const
{
	if (n0.triangle == t)
		return 0;
	if (n1.triangle == t)
		return 1;
	if (n2.triangle == t)
		return 2;
	return -1;
}
	
	
template <class Def> MF_DECLARE (char) TFaceGraphDefTriangle<Def>::indexOfTriangle(const MeshTriangle<TFaceGraphDef<Def> >*t)		const
{
	if (n0.triangle == t)
		return 0;
	if (n1.triangle == t)
		return 1;
	if (n2.triangle == t)
		return 2;
	return -1;
}
	
template <class Def> MF_DECLARE (BYTE) TFaceGraphDefTriangle<Def>::requireIndexOf(const MeshTriangle<TFaceGraphDef<Def> >*t)		const
{
	if (n0.triangle == t)
		return 0;
	if (n1.triangle == t)
		return 1;
	if (n2.triangle == t)
		return 2;
	FATAL__("linkage broken");
}
	
template <class Def> MF_DECLARE (BYTE) TFaceGraphDefTriangle<Def>::requireIndexOfTriangle(const MeshTriangle<TFaceGraphDef<Def> >*t)		const
{
	if (n0.triangle == t)
		return 0;
	if (n1.triangle == t)
		return 1;
	if (n2.triangle == t)
		return 2;
	FATAL__("linkage broken");
}
	


template <class Def> MF_DECLARE (char) TFaceGraphDefTriangle<Def>::FindAndReplace(MeshQuad<TFaceGraphDef<Def> >*v, MeshQuad<TFaceGraphDef<Def> >*with)
{
	if (n0.quad == v)
	{
		n0.quad = with;
		n0.is_quad = true;
		return 0;
	}
	if (n1.quad == v)
	{
		n1.quad = with;
		n1.is_quad = true;
		return 1;
	}
	if (n2.quad == v)
	{
		n2.quad = with;
		n2.is_quad = true;
		return 2;
	}
	return -1;
}
	
template <class Def> MF_DECLARE (char) TFaceGraphDefTriangle<Def>::FindAndReplace(MeshQuad<TFaceGraphDef<Def> >*v, MeshTriangle<TFaceGraphDef<Def> >*with)
{
	if (n0.quad == v)
	{
		n0.triangle = with;
		n0.is_quad = false;
		return 0;
	}
	if (n1.quad == v)
	{
		n1.triangle = with;
		n1.is_quad = false;
		return 1;
	}
	if (n2.quad == v)
	{
		n2.triangle = with;
		n2.is_quad = false;
		return 2;
	}
	return -1;
}
	
template <class Def> MF_DECLARE (char) TFaceGraphDefTriangle<Def>::FindAndReplace(MeshTriangle<TFaceGraphDef<Def> >*v, MeshTriangle<TFaceGraphDef<Def> >*with)
{
	if (n0.triangle == v)
	{
		n0.triangle = with;
		n0.is_quad = false;
		return 0;
	}
	if (n1.triangle == v)
	{
		n1.triangle = with;
		n1.is_quad = false;
		return 1;
	}
	if (n2.triangle == v)
	{
		n2.triangle = with;
		n2.is_quad = false;
		return 2;
	}
	return -1;
}
	
template <class Def> MF_DECLARE (char) TFaceGraphDefTriangle<Def>::FindAndReplace(MeshTriangle<TFaceGraphDef<Def> >*v, MeshQuad<TFaceGraphDef<Def> >*with)
{
	if (n0.triangle == v)
	{
		n0.quad = with;
		n0.is_quad = true;
		return 0;
	}
	if (n1.triangle == v)
	{
		n1.quad = with;
		n1.is_quad = true;
		return 1;
	}
	if (n2.triangle == v)
	{
		n2.quad = with;
		n2.is_quad = true;
		return 2;
	}
	return -1;
}
	

	

	
template <class Def> MF_DECLARE (char) TFaceGraphDefQuad<Def>::GetIndexOf(const MeshQuad<TFaceGraphDef<Def> >*q)	const
{
	if (n0.quad == q)
		return 0;
	if (n1.quad == q)
		return 1;
	if (n2.quad == q)
		return 2;
	if (n3.quad == q)
		return 3;
	return -1;
}
	
template <class Def> MF_DECLARE (BYTE) TFaceGraphDefQuad<Def>::requireIndexOf(const MeshQuad<TFaceGraphDef<Def> >*q)	const
{
	if (n0.quad == q)
		return 0;
	if (n1.quad == q)
		return 1;
	if (n2.quad == q)
		return 2;
	if (n3.quad == q)
		return 3;
	FATAL__("linkage broken");
}
	
template <class Def> MF_DECLARE (char) TFaceGraphDefQuad<Def>::GetIndexOf(const MeshTriangle<TFaceGraphDef<Def> >*t)		const
{
	if (n0.triangle == t)
		return 0;
	if (n1.triangle == t)
		return 1;
	if (n2.triangle == t)
		return 2;
	if (n3.triangle == t)
		return 3;
	return -1;
}
	
template <class Def> MF_DECLARE (BYTE) TFaceGraphDefQuad<Def>::requireIndexOf(const MeshTriangle<TFaceGraphDef<Def> >*t)		const
{
	if (n0.triangle == t)
		return 0;
	if (n1.triangle == t)
		return 1;
	if (n2.triangle == t)
		return 2;
	if (n3.triangle == t)
		return 3;
	return 255;
}
	

template <class Def> MF_DECLARE (char) TFaceGraphDefQuad<Def>::FindAndReplace(MeshQuad<TFaceGraphDef<Def> >*v, MeshQuad<TFaceGraphDef<Def> >*with)
{
	if (n0.quad == v)
	{
		n0.quad = with;
		n0.is_quad = true;
		return 0;
	}
	if (n1.quad == v)
	{
		n1.quad = with;
		n1.is_quad = true;
		return 1;
	}
	if (n2.quad == v)
	{
		n2.quad = with;
		n2.is_quad = true;
		return 2;
	}
	if (n3.quad == v)
	{
		n3.quad = with;
		n3.is_quad = true;
		return 3;
	}
	return -1;
}
	
template <class Def> MF_DECLARE (char) TFaceGraphDefQuad<Def>::FindAndReplace(MeshQuad<TFaceGraphDef<Def> >*v, MeshTriangle<TFaceGraphDef<Def> >*with)
{
	if (n0.quad == v)
	{
		n0.triangle = with;
		n0.is_quad = false;
		return 0;
	}
	if (n1.quad == v)
	{
		n1.triangle = with;
		n1.is_quad = false;
		return 1;
	}
	if (n2.quad == v)
	{
		n2.triangle = with;
		n2.is_quad = false;
		return 2;
	}
	if (n3.quad == v)
	{
		n3.triangle = with;
		n3.is_quad = false;
		return 3;
	}
	return -1;
}
	
template <class Def> MF_DECLARE (char) TFaceGraphDefQuad<Def>::FindAndReplace(MeshTriangle<TFaceGraphDef<Def> >*v, MeshTriangle<TFaceGraphDef<Def> >*with)
{
	if (n0.triangle == v)
	{
		n0.triangle = with;
		n0.is_quad = false;
		return 0;
	}
	if (n1.triangle == v)
	{
		n1.triangle = with;
		n1.is_quad = false;
		return 1;
	}
	if (n2.triangle == v)
	{
		n2.triangle = with;
		n2.is_quad = false;
		return 2;
	}
	if (n3.triangle == v)
	{
		n3.triangle = with;
		n3.is_quad = false;
		return 3;
	}
	return -1;
}
	
template <class Def> MF_DECLARE (char) TFaceGraphDefQuad<Def>::FindAndReplace(MeshTriangle<TFaceGraphDef<Def> >*v, MeshQuad<TFaceGraphDef<Def> >*with)
{
	if (n0.triangle == v)
	{
		n0.quad = with;
		n0.is_quad = true;
		return 0;
	}
	if (n1.triangle == v)
	{
		n1.quad = with;
		n1.is_quad = true;
		return 1;
	}
	if (n2.triangle == v)
	{
		n2.quad = with;
		n2.is_quad = true;
		return 2;
	}
	if (n3.triangle == v)
	{
		n3.quad = with;
		n3.is_quad = true;
		return 3;
	}
	return -1;
}
	

	
	
#ifndef	__CUDACC__

	
	
template <class Def>
	MF_DECLARE	(String)		TGraphDefTriangle<Def>::ToString(bool full)	const
	{
 		typedef MeshVertex<TFaceGraphDef<Def> >		Vertex;
 		typedef MeshTriangle<TFaceGraphDef<Def> >	Triangle;
 		typedef MeshQuad<TFaceGraphDef<Def> >		Quad;
 		const Triangle*t = (const Triangle*)this;
 		if (!full)
 			return "t"+IntToStr(t->index);
 		return "t"+IntToStr(t->index)+" {v"+IntToStr(t->v0->index)+", v"+IntToStr(t->v1->index)+", v"+IntToStr(t->v2->index)+"} {"+n0.ToString(false)+", "+n1.ToString(false)+", "+n2.ToString(false)+"} {e"+IntToStr(e0->index)+", e"+IntToStr(e1->index)+", e"+IntToStr(e2->index)+"}";
	}
	
template <class Def>
	MF_DECLARE	(String)		TGraphDefQuad<Def>::ToString(bool full)	const
	{
 		typedef MeshVertex<TFaceGraphDef<Def> >		Vertex;
 		typedef MeshTriangle<TFaceGraphDef<Def> >	Triangle;
 		typedef MeshQuad<TFaceGraphDef<Def> >		Quad;
 		const Quad*t = (const Quad*)this;
 		if (!full)
 			return "q"+IntToStr(t->index);
 		return "q"+IntToStr(t->index)+" {v"+IntToStr(t->v0->index)+", v"+IntToStr(t->v1->index)+", v"+IntToStr(t->v2->index)+", v"+IntToStr(t->v3->index)+"} {"+n0.ToString(false)+", "+n1.ToString(false)+", "+n2.ToString(false)+", "+n3.ToString(false)+" {e"+IntToStr(e0->index)+", e"+IntToStr(e1->index)+", e"+IntToStr(e2->index)+", e"+IntToStr(e3->index)+"}";
	}

	
#endif
	
	
	
template <class Def> MF_DECLARE (char) TGraphDefTriangle<Def>::GetIndexOf(const MeshQuad<TGraphDef<Def> >*q)	const
{
	if (n0.quad == q)
		return 0;
	if (n1.quad == q)
		return 1;
	if (n2.quad == q)
		return 2;
	return -1;
}
	
template <class Def> MF_DECLARE (BYTE) TGraphDefTriangle<Def>::requireIndexOf(const MeshQuad<TGraphDef<Def> >*q)	const
{
	if (n0.quad == q)
		return 0;
	if (n1.quad == q)
		return 1;
	if (n2.quad == q)
		return 2;
	FATAL__("linkage broken");
}
	
template <class Def> MF_DECLARE (char) TGraphDefTriangle<Def>::GetIndexOf(const MeshTriangle<TGraphDef<Def> >*t)		const
{
	if (n0.triangle == t)
		return 0;
	if (n1.triangle == t)
		return 1;
	if (n2.triangle == t)
		return 2;
	return -1;
}
	
template <class Def> MF_DECLARE (BYTE) TGraphDefTriangle<Def>::requireIndexOf(const MeshTriangle<TGraphDef<Def> >*t)		const
{
	if (n0.triangle == t)
		return 0;
	if (n1.triangle == t)
		return 1;
	if (n2.triangle == t)
		return 2;
	FATAL__("linkage broken");
}
	
template <class Def> MF_DECLARE (char) TGraphDefTriangle<Def>::GetIndexOf(const MeshEdge<TGraphDef<Def> >*e)	const
{
	if (edge[0] == e)
		return 0;
	if (edge[1] == e)
		return 1;
	if (edge[2] == e)
		return 2;
	return -1;
}
	
template <class Def> MF_DECLARE (BYTE) TGraphDefTriangle<Def>::requireIndexOf(const MeshEdge<TGraphDef<Def> >*e)	const
{
	if (edge[0] == e)
		return 0;
	if (edge[1] == e)
		return 1;
	if (edge[2] == e)
		return 2;
	FATAL__("linkage broken");
}

template <class Def> MF_DECLARE (char) TGraphDefTriangle<Def>::FindAndReplace(MeshQuad<TGraphDef<Def> >*v, MeshQuad<TGraphDef<Def> >*with)
{
	if (n0.quad == v)
	{
		n0.quad = with;
		n0.is_quad = true;
		return 0;
	}
	if (n1.quad == v)
	{
		n1.quad = with;
		n1.is_quad = true;
		return 1;
	}
	if (n2.quad == v)
	{
		n2.quad = with;
		n2.is_quad = true;
		return 2;
	}
	return -1;
}
	
template <class Def> MF_DECLARE (char) TGraphDefTriangle<Def>::FindAndReplace(MeshQuad<TGraphDef<Def> >*v, MeshTriangle<TGraphDef<Def> >*with)
{
	if (n0.quad == v)
	{
		n0.triangle = with;
		n0.is_quad = false;
		return 0;
	}
	if (n1.quad == v)
	{
		n1.triangle = with;
		n1.is_quad = false;
		return 1;
	}
	if (n2.quad == v)
	{
		n2.triangle = with;
		n2.is_quad = false;
		return 2;
	}
	return -1;
}
	
template <class Def> MF_DECLARE (char) TGraphDefTriangle<Def>::FindAndReplace(MeshTriangle<TGraphDef<Def> >*v, MeshTriangle<TGraphDef<Def> >*with)
{
	if (n0.triangle == v)
	{
		n0.triangle = with;
		n0.is_quad = false;
		return 0;
	}
	if (n1.triangle == v)
	{
		n1.triangle = with;
		n1.is_quad = false;
		return 1;
	}
	if (n2.triangle == v)
	{
		n2.triangle = with;
		n2.is_quad = false;
		return 2;
	}
	return -1;
}
	
template <class Def> MF_DECLARE (char) TGraphDefTriangle<Def>::FindAndReplace(MeshTriangle<TGraphDef<Def> >*v, MeshQuad<TGraphDef<Def> >*with)
{
	if (n0.triangle == v)
	{
		n0.quad = with;
		n0.is_quad = true;
		return 0;
	}
	if (n1.triangle == v)
	{
		n1.quad = with;
		n1.is_quad = true;
		return 1;
	}
	if (n2.triangle == v)
	{
		n2.quad = with;
		n2.is_quad = true;
		return 2;
	}
	return -1;
}
	
template <class Def> MF_DECLARE (char) TGraphDefTriangle<Def>::FindAndReplace(MeshEdge<TGraphDef<Def> >*e, MeshEdge<TGraphDef<Def> >*with)
{
	if (edge[0] == e)
	{
		edge[0] = with;
		return 0;
	}
	if (edge[1] == e)
	{
		edge[1] = with;
		return 1;
	}
	if (edge[2] == e)
	{
		edge[2] = with;
		return 2;
	}
	return -1;
}	
	

	
	

	
template <class Def> MF_DECLARE (char) TGraphDefQuad<Def>::GetIndexOf(const MeshQuad<TGraphDef<Def> >*q)	const
{
	if (n0.quad == q)
		return 0;
	if (n1.quad == q)
		return 1;
	if (n2.quad == q)
		return 2;
	if (n3.quad == q)
		return 3;
	return -1;
}
	
template <class Def> MF_DECLARE (BYTE) TGraphDefQuad<Def>::requireIndexOf(const MeshQuad<TGraphDef<Def> >*q)	const
{
	if (n0.quad == q)
		return 0;
	if (n1.quad == q)
		return 1;
	if (n2.quad == q)
		return 2;
	if (n3.quad == q)
		return 3;
	FATAL__("linkage broken");
}
	
template <class Def> MF_DECLARE (char) TGraphDefQuad<Def>::GetIndexOf(const MeshTriangle<TGraphDef<Def> >*t)		const
{
	if (n0.triangle == t)
		return 0;
	if (n1.triangle == t)
		return 1;
	if (n2.triangle == t)
		return 2;
	if (n3.triangle == t)
		return 3;
	return -1;
}
	
template <class Def> MF_DECLARE (BYTE) TGraphDefQuad<Def>::requireIndexOf(const MeshTriangle<TGraphDef<Def> >*t)		const
{
	if (n0.triangle == t)
		return 0;
	if (n1.triangle == t)
		return 1;
	if (n2.triangle == t)
		return 2;
	if (n3.triangle == t)
		return 3;
	FATAL__("linkage broken");
}
	
template <class Def> MF_DECLARE (char) TGraphDefQuad<Def>::GetIndexOf(const MeshEdge<TGraphDef<Def> >*e)	const
{
	if (edge[0] == e)
		return 0;
	if (edge[1] == e)
		return 1;
	if (edge[2] == e)
		return 2;
	if (edge[3] == e)
		return 3;
	return -1;
}

template <class Def> MF_DECLARE (BYTE) TGraphDefQuad<Def>::requireIndexOf(const MeshEdge<TGraphDef<Def> >*e)	const
{
	if (edge[0] == e)
		return 0;
	if (edge[1] == e)
		return 1;
	if (edge[2] == e)
		return 2;
	if (edge[3] == e)
		return 3;
	FATAL__("linkage broken");
}

template <class Def> MF_DECLARE (char) TGraphDefQuad<Def>::FindAndReplace(MeshQuad<TGraphDef<Def> >*v, MeshQuad<TGraphDef<Def> >*with)
{
	if (n0.quad == v)
	{
		n0.quad = with;
		n0.is_quad = true;
		return 0;
	}
	if (n1.quad == v)
	{
		n1.quad = with;
		n1.is_quad = true;
		return 1;
	}
	if (n2.quad == v)
	{
		n2.quad = with;
		n2.is_quad = true;
		return 2;
	}
	if (n3.quad == v)
	{
		n3.quad = with;
		n3.is_quad = true;
		return 3;
	}
	return -1;
}
	
template <class Def> MF_DECLARE (char) TGraphDefQuad<Def>::FindAndReplace(MeshQuad<TGraphDef<Def> >*v, MeshTriangle<TGraphDef<Def> >*with)
{
	if (n0.quad == v)
	{
		n0.triangle = with;
		n0.is_quad = false;
		return 0;
	}
	if (n1.quad == v)
	{
		n1.triangle = with;
		n1.is_quad = false;
		return 1;
	}
	if (n2.quad == v)
	{
		n2.triangle = with;
		n2.is_quad = false;
		return 2;
	}
	if (n3.quad == v)
	{
		n3.triangle = with;
		n3.is_quad = false;
		return 3;
	}
	return -1;
}
	
template <class Def> MF_DECLARE (char) TGraphDefQuad<Def>::FindAndReplace(MeshTriangle<TGraphDef<Def> >*v, MeshTriangle<TGraphDef<Def> >*with)
{
	if (n0.triangle == v)
	{
		n0.triangle = with;
		n0.is_quad = false;
		return 0;
	}
	if (n1.triangle == v)
	{
		n1.triangle = with;
		n1.is_quad = false;
		return 1;
	}
	if (n2.triangle == v)
	{
		n2.triangle = with;
		n2.is_quad = false;
		return 2;
	}
	if (n3.triangle == v)
	{
		n3.triangle = with;
		n3.is_quad = false;
		return 3;
	}
	return -1;
}
	
template <class Def> MF_DECLARE (char) TGraphDefQuad<Def>::FindAndReplace(MeshTriangle<TGraphDef<Def> >*v, MeshQuad<TGraphDef<Def> >*with)
{
	if (n0.triangle == v)
	{
		n0.quad = with;
		n0.is_quad = true;
		return 0;
	}
	if (n1.triangle == v)
	{
		n1.quad = with;
		n1.is_quad = true;
		return 1;
	}
	if (n2.triangle == v)
	{
		n2.quad = with;
		n2.is_quad = true;
		return 2;
	}
	if (n3.triangle == v)
	{
		n3.quad = with;
		n3.is_quad = true;
		return 3;
	}
	return -1;
}
	
template <class Def> MF_DECLARE (char) TGraphDefQuad<Def>::FindAndReplace(MeshEdge<TGraphDef<Def> >*e, MeshEdge<TGraphDef<Def> >*with)
{
	if (edge[0] == e)
	{
		edge[0] = with;
		return 0;
	}
	if (edge[1] == e)
	{
		edge[1] = with;
		return 1;
	}
	if (edge[2] == e)
	{
		edge[2] = with;
		return 2;
	}
	if (edge[3] == e)
	{
		edge[3] = with;
		return 3;
	}
	return -1;
}	
		
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
#if 0



// index-functions
template <class Def> MFMODS (char)	_oIndexOf(ObjFace<Def>*face, MeshVertex<Def>*vertex)
{
	if (face->v0 == vertex)
		return 0;
	if (face->v1 == vertex)
		return 1;
	if (face->v2 == vertex)
		return 2;
	return -1;
}

template <class Def> MFMODS (char)	_oIndexOf(MeshEdge<Def>*edge, MeshVertex<Def>*vertex)
{
	if (edge->v0 == vertex)
		return 0;
	if (edge->v1 == vertex)
		return 1;
	return -1;
}

template <class Def> MFMODS (char)	_oIndexOf(MeshEdge<Def>*edge, ObjFace<Def>*face)
{
	if (edge->face[0] == face)
		return 0;
	if (edge->face[1] == face)
		return 1;
	return -1;
}

MFUNC	(char)	_oIndexOf(ObjFace<TGraphDef<C> >*face, MeshEdge<TGraphDef<C> >*edge)
{
	if (face->edge[0] == edge)
		return 0;
	if (face->edge[1] == edge)
		return 1;
	if (face->edge[2] == edge)
		return 2;
	return -1;
}
	
MFUNC	(char)	_oIndexOf(ObjFace<TGraphDef<C> >*face, ObjFace<TGraphDef<C> >*neighbor)
{
	if (face->n0 == neighbor)
		return 0;
	if (face->n1 == neighbor)
		return 1;
	if (face->n2 == neighbor)
		return 2;
	return -1;
}

	
template <class Def> MFMODS (char)	_oReplace(ObjFace<Def>*face, MeshVertex<Def>*vertex, MeshVertex<Def>*with)
{
	char rs = _oIndexOf(face,vertex);
	if (rs != -1)
		face->vertex[rs] = with;
	return rs;
}
	
template <class Def> MFMODS (char)	_oReplace(MeshEdge<Def>*edge, MeshVertex<Def>*vertex, MeshVertex<Def>*with)
{
	char rs = _oIndexOf(edge,vertex);
	if (rs != -1)
		edge->vertex[rs] = with;
	return rs;
}
	
template <class Def> MFMODS (char)	_oReplace(MeshEdge<Def>*edge, ObjFace<Def>*face, ObjFace<Def>*with)
{
	char rs = _oIndexOf(edge,face);
	if (rs != -1)
		edge->face[rs] = with;
	return rs;
}
	
MFUNC	(char)	_oReplace(ObjFace<TGraphDef<C> >*face, MeshEdge<TGraphDef<C> >*edge, MeshEdge<TGraphDef<C> >*with)
{
	char rs = _oIndexOf(face,edge);
	if (rs != -1)
		face->edge[rs] = with;
	return rs;
}

MFUNC	(char)	_oReplace(ObjFace<TGraphDef<C> >*face, ObjFace<TGraphDef<C> >*neighbor,ObjFace<TGraphDef<C> >*with)
{
	char rs = _oIndexOf(face,neighbor);
	if (rs != -1)
		face->n[rs] = with;
	return rs;
}

#endif

MFUNC2
	(void)				_oSphere(TriangleMesh<C0>&obj,const C1&radius, count_t iterations)
	{
		obj.vertices = (iterations-1)*(iterations-1)*3*12;
		obj.band = 6;
		obj.data.SetSize(obj.vertices*obj.band);
			
		//unsigned cnt = 0;
		M::TVec3<C0> p[4],c,n;
		C0	*out = obj.data.pointer();

		for (BYTE d = 0; d < 3; d++)
		{
			BYTE	vx = (d+1)%3,
					vy = (d+2)%3;
			for (index_t x = 1; x < iterations; x++)
				for (index_t y = 1; y < iterations; y++)
				{
					p[0].v[vx] = 1-(C0)(x-1)/(C0)(iterations-1)*2;
					p[0].v[vy] = 1-(C0)(y-1)/(C0)(iterations-1)*2;
					p[0].v[d]	= 1;
					p[1].v[vx] = 1-(C0)(x)/(C0)(iterations-1)*2;
					p[1].v[vy] = 1-(C0)(y-1)/(C0)(iterations-1)*2;
					p[1].v[d]	= 1;
					p[2].v[vx] = 1-(C0)(x)/(C0)(iterations-1)*2;
					p[2].v[vy] = 1-(C0)(y)/(C0)(iterations-1)*2;
					p[2].v[d]	= 1;
					p[3].v[vx] = 1-(C0)(x-1)/(C0)(iterations-1)*2;
					p[3].v[vy] = 1-(C0)(y)/(C0)(iterations-1)*2;
					p[3].v[d]	= 1;

					for (BYTE k = 0; k < 3; k++)
					{
						M::Vec::divide(p[k],M::Vec::length(p[k]),n);
						M::Vec::multiply(n,radius,c);
						M::Vec::copy(c,M::Vec::ref3(out));	out+=3;
						M::Vec::copy(n,M::Vec::ref3(out)); out+=3;
					}
					for (BYTE k = 2; k < 5; k++)
					{
						M::Vec::divide(p[k%4],M::Vec::length(p[k%4]),n);
						M::Vec::multiply(n,radius,c);
						M::Vec::copy(c,M::Vec::ref3(out));	out+=3;
						M::Vec::copy(n,M::Vec::ref3(out)); out+=3;
					}


					for (BYTE k = 2; k < 3; k--)
					{
						p[k].v[d] = -1;
						M::Vec::divide(p[k],M::Vec::length(p[k]),n);
						M::Vec::multiply(n,radius,c);
						M::Vec::copy(c,M::Vec::ref3(out));	out+=3;
						M::Vec::copy(n,M::Vec::ref3(out)); out+=3;
					}
					for (BYTE k = 4; k > 1; k--)
					{
						p[k%4].v[d] = -1;
						M::Vec::divide(p[k%4],M::Vec::length(p[k%4]),n);
						M::Vec::multiply(n,radius,c);
						M::Vec::copy(c,M::Vec::ref3(out));	out+=3;
						M::Vec::copy(n,M::Vec::ref3(out));	out+=3;
					}
				}
		}
		ASSERT_CONCLUSION(obj.data,out);
	}

MFUNC	(TriangleMesh<C>*)	_oSphere(const C&radius, count_t iterations)
{
	TriangleMesh<C>*result = SignalNew(new TriangleMesh<C>((iterations-1)*(iterations-1)*3*12,6));
	count_t cnt = 0;
	C p[4][3],c[3],n[3];

		for (BYTE d = 0; d < 3; d++)
		{
			BYTE vx = (d+1)%3,
					vy = (d+2)%3;
			for (index_t x = 1; x < iterations; x++)
				for (index_t y = 1; y < iterations; y++)
				{
					p[0][vx] = 1-(C)(x-1)/(C)(iterations-1)*2;
					p[0][vy] = 1-(C)(y-1)/(C)(iterations-1)*2;
					p[0][d]	= 1;
					p[1][vx] = 1-(C)(x)/(C)(iterations-1)*2;
					p[1][vy] = 1-(C)(y-1)/(C)(iterations-1)*2;
					p[1][d]	= 1;
					p[2][vx] = 1-(C)(x)/(C)(iterations-1)*2;
					p[2][vy] = 1-(C)(y)/(C)(iterations-1)*2;
					p[2][d]	= 1;
					p[3][vx] = 1-(C)(x-1)/(C)(iterations-1)*2;
					p[3][vy] = 1-(C)(y)/(C)(iterations-1)*2;
					p[3][d]	= 1;

					for (BYTE k = 0; k < 3; k++)
					{
						_divide(p[k],_length(p[k]),n);
						_multiply(n,radius,c);
						_c3(c,&result->data[3*cnt++]);
						_c3(n,&result->data[3*cnt++]);
					}
					for (BYTE k = 2; k < 5; k++)
					{
						_divide(p[k%4],_length(p[k%4]),n);
						_multiply(n,radius,c);
						_c3(c,&result->data[3*cnt++]);
						_c3(n,&result->data[3*cnt++]);
					}


					for (BYTE k = 2; k < 3; k--)
					{
						p[k][d] = -1;
						_divide(p[k],_length(p[k]),n);
						_multiply(n,radius,c);
						_c3(c,&result->data[3*cnt++]);
						_c3(n,&result->data[3*cnt++]);
					}
					for (BYTE k = 4; k > 1; k--)
					{
						p[k%4][d] = -1;
						_divide(p[k%4],_length(p[k%4]),n);
						_multiply(n,radius,c);
						_c3(c,&result->data[3*cnt++]);
						_c3(n,&result->data[3*cnt++]);
					}
				}
		}
	return result;
}




template <class Def> MF_CONSTRUCTOR  TriangleMesh<Def>::TriangleMesh(count_t v, unsigned b):vertices(v),band(b),data(v*b)
{}

template <class C> TriangleMesh<C>::~TriangleMesh()
{}
	
template <class Def>
	MF_DECLARE	(void)	DynamicMesh<Def>::clear()
	{
		vertices.clear();
		edges.clear();
		quads.clear();
		triangles.clear();
	}
	
template <class Def>
	MF_DECLARE	(void)	DynamicMesh<Def>::determineVertexWeights()	//GraphDef or FaceGraphDef only
	{
			
		Ctr::Array<Point<typename Def::Type> >	face_normals(triangles+quads),
											side_face_normals(triangles*3+quads*4),
											vertex_normals(vertices),
											outer_vertex_normals(vertices);
		Ctr::Array<typename Def::Type>			side_face_size(triangles*3+quads*4),
											face_size(triangles+quads);

		for (index_t i = 0; i < vertex_normals.Count(); i++)
		{
			_clear(vertex_normals[i].vector);
			_clear(outer_vertex_normals[i].vector);
			vertices[i]->weight = 0;
		}
			
			
		Point<typename Def::Type>	*p = face_normals,
									*sp = side_face_normals;
		typename Def::Type			*f = face_size,
									*sf = side_face_size;
		for (index_t i = 0; i < triangles; i++)
		{
			MeshTriangle<Def>*t = triangles[i];
			typename Def::Type normal[3];
			_oTriangleNormal(t->v0->position,t->v1->position,t->v2->position,normal);
			(*f) = _length(normal);
			if (*f > Math::GetError<typename Def::Type>())
				_div(normal,*f,p->vector);
			else
				_clear(p->vector);
			_add(vertex_normals[t->v0->index].vector,normal);
			_add(vertex_normals[t->v1->index].vector,normal);
			_add(vertex_normals[t->v2->index].vector,normal);
				
			for (BYTE k = 0; k < 3; k++)
			{
				if (!t->n[k])
				{
					typename Def::Type vec[3],side_normal[3];
					_sub(t->vertex[(k+1)%3]->position,t->vertex[k]->position,vec);
					_cross(vec,p->vector,side_normal);
					(*sf) = _length(side_normal);
					if (*sf > Math::GetError<typename Def::Type>())
						_div(side_normal,*sf,sp->vector);
					else
						_clear(sp->vector);
					_add(outer_vertex_normals[t->vertex[k]->index].vector,side_normal);
					_add(outer_vertex_normals[t->vertex[(k+1)%3]->index].vector,side_normal);
				}
				sf++; sp++;
			}
			p++; f++;
				
		}

		for (index_t i = 0; i < quads; i++)
		{
			MeshQuad<Def>*q = quads[i];
			typename Def::Type normal[3];
			if (!q->v0 || !q->v1 || !q->v2 || !q->v3)
				FATAL__("linkage error");
			for (BYTE k = 0; k < 4; k++)
				if (!vertices(q->vertex[k]))
					FATAL__("vertex "+IntToStr(k)+" corrupted");
			_oTriangleNormal(q->v0->position,q->v1->position,q->v2->position,normal);
			_oAddTriangleNormal(q->v0->position,q->v2->position,q->v3->position,normal);
			(*f) = _length(normal);
			if (*f > Math::GetError<typename Def::Type>())
				_div(normal,*f,p->vector);
			else
				_clear(p->vector);
			_add(vertex_normals[q->v0->index].vector,normal);
			_add(vertex_normals[q->v1->index].vector,normal);
			_add(vertex_normals[q->v2->index].vector,normal);
			_add(vertex_normals[q->v3->index].vector,normal);
				
			for (BYTE k = 0; k < 4; k++)
			{
				if (!q->n[k])
				{
					typename Def::Type vec[3],side_normal[3];
					_sub(q->vertex[(k+1)%4]->position,q->vertex[k]->position,vec);
					_cross(vec,p->vector,side_normal);
					(*sf) = _length(side_normal);
					if (*sf > Math::GetError<typename Def::Type>())
						_div(side_normal,*sf,sp->vector);
					else
						_clear(sp->vector);
					_add(outer_vertex_normals[q->vertex[k]->index].vector,side_normal);
					_add(outer_vertex_normals[q->vertex[(k+1)%4]->index].vector,side_normal);
				}
				sf++; sp++;
			}
			p++; f++;
		}
			
		ASSERT_CONCLUSION(face_normals,p)
		ASSERT_CONCLUSION(side_face_normals,sp)
		ASSERT_CONCLUSION(face_size,f)
		ASSERT_CONCLUSION(side_face_size,sf)

		for (index_t i = 0; i < vertex_normals.Count(); i++)
		{
			_normalize0(vertex_normals[i].vector);
			_normalize0(outer_vertex_normals[i].vector);
		}
			
			
		p = face_normals;
		sp = side_face_normals;
		f = face_size;
		sf = side_face_size;
		for (index_t i = 0; i < triangles; i++)
		{
			MeshTriangle<Def>*t = triangles[i];
			for (BYTE k = 0; k < 3; k++)
			{
				typename Def::Type	level = (-_dot(p->vector,vertex_normals[t->vertex[k]->index].vector)+1)/2*(*f);
				if (level > t->vertex[k]->weight)
					t->vertex[k]->weight = level;
					
				if (!t->n[k])
				{
					level = (-_dot(sp->vector,outer_vertex_normals[t->vertex[k]->index].vector)+1)/2*sqr(*sf)*300;
					if (level > t->vertex[k]->weight)
						t->vertex[k]->weight = level;
					level = (-_dot(sp->vector,outer_vertex_normals[t->vertex[(k+1)%3]->index].vector)+1)/2*sqr(*sf)*300;
					if (level > t->vertex[(k+1)%3]->weight)
						t->vertex[(k+1)%3]->weight = level;
				}
				sf++;
				sp++;
			}
			f++;
			p++;
		}
			
		for (index_t i = 0; i < quads; i++)
		{
			MeshQuad<Def>*t = quads[i];
			for (BYTE k = 0; k < 4; k++)
			{
				typename Def::Type	level = (-_dot(p->vector,vertex_normals[t->vertex[k]->index].vector)+1)/2*(*f);
				if (level > t->vertex[k]->weight)
					t->vertex[k]->weight = level;
						
				if (!t->n[k])
				{
					level = (-_dot(sp->vector,outer_vertex_normals[t->vertex[k]->index].vector)+1)/2*sqr(*sf)*300;
					if (level > t->vertex[k]->weight)
						t->vertex[k]->weight = level;
					level = (-_dot(sp->vector,outer_vertex_normals[t->vertex[(k+1)%4]->index].vector)+1)/2*sqr(*sf)*300;
					if (level > t->vertex[(k+1)%4]->weight)
						t->vertex[(k+1)%4]->weight = level;
				}
				sf++;
				sp++;						
			}
			f++;
			p++;
		}
			
		ASSERT_CONCLUSION(face_normals,p)
		ASSERT_CONCLUSION(side_face_normals,sp)
		ASSERT_CONCLUSION(face_size,f)
		ASSERT_CONCLUSION(side_face_size,sf)
	}


template <class Def>MF_CONSTRUCTOR Mesh<Def>::Mesh():_map(NULL)
{}

template <class Def>MF_CONSTRUCTOR Mesh<Def>::Mesh(const Mesh<Def>&other):_map(NULL)
{
	operator=(other);
}
template <class Def>
template <class T> MF_CONSTRUCTOR Mesh<Def>::Mesh(const Mesh<T>&other):_map(NULL)
{
	operator=(other);
}
	
template <class Def> template <typename T0, typename T1>
	MF_DECLARE	(bool)			Mesh<Def>::detectOpticalIntersection(const T0 b[3], const T1 d[3], float&distance)	const
	{
		bool result = false;
		for (index_t i = 0; i < triangle_field.GetLength(); i++)
			if (_oDetectOptiocalIntersection(triangle_field[i].v0->position,triangle_field[i].v1->position,triangle_field[i].v2->position,b,d,distance))
				result = true;
		for (index_t i = 0; i < quad_field.GetLength(); i++)
			if (_oDetectOptiocalIntersection(quad_field[i].v0->position,quad_field[i].v1->position,quad_field[i].v2->position,b,d,distance)
				||
				_oDetectOptiocalIntersection(quad_field[i].v0->position,quad_field[i].v2->position,quad_field[i].v3->position,b,d,distance))
				result = true;
		return result;
	}
		
template <class Def>
	MF_DECLARE	(void)	Mesh<Def>::verifyIntegrity()									const
	{
		if (!valid(true))
			FATAL__(_error);
	}

template <class Def>
	MF_DECLARE	(bool) Mesh<Def>::valid(bool check_ranges)	const
	{
		for (index_t i = 0; i < triangle_field.GetLength(); i++)
			for (BYTE k = 0; k < 3; k++)
			{
				if (check_ranges && (index_t)(triangle_field[i].vertex[k]-vertex_field) >= vertex_field.GetLength())
				{
					_error = "Vertex link "+String(k)+" of triangle "+String(i)+" is broken";
					return false;
				}
				if (triangle_field[i].vertex[k] == triangle_field[i].vertex[(k+1)%3])
				{
					_error = "Vertex link "+String(k)+" of triangle "+String(i)+" is identical to successor => triangle collapsed";
					return false;
				}
			}
		for (index_t i = 0; i < quad_field.GetLength(); i++)
			for (BYTE k = 0; k < 4; k++)
			{
				if (check_ranges && (index_t)(quad_field[i].vertex[k]-vertex_field) >= vertex_field.GetLength())
				{
					_error = "Vertex link "+String(k)+" of quad "+String(i)+" is broken";
					return false;
				}
				if (quad_field[i].vertex[k] == quad_field[i].vertex[(k+1)%4] || quad_field[i].vertex[k] == quad_field[i].vertex[(k+2)%4])
				{
					_error = "Vertex link "+String(k)+" of quad "+String(i)+" is identical to successor => triangle collapsed";
					return false;
				}
			}
		for (index_t i = 0; i < edge_field.GetLength(); i++)
		{
			for (BYTE k = 0; k < 2; k++)
			{
				if (check_ranges && (index_t)(edge_field[i].vertex[k]-vertex_field) >= vertex_field.GetLength())
				{
					_error = "Vertex link "+String(k)+" of edge "+String(i)+" is broken";
					return false;
				}
			}
			if (!IsValid(edge_field[i].n[0],false,true)
				||
				!IsValid(edge_field[i].n[1],true,true))
			{
				_error = "Neighbor link(s) of edge "+String(i)+" is/are broken";
				return false;
			}
				
			if (edge_field[i].v0 == edge_field[i].v1)
			{
				_error = "Vertex links of edge "+String(i)+" are identical => edge collapsed";
				return false;
			}
		}
		_error = "";
		return true;
	}
	
template <class Def> MF_DECLARE	(bool) Mesh<Def>::IsEmpty()																	const
{
	return !vertex_field.GetLength();	//no vertices = no edges && no faces
}
	
template <class Def> MF_DECLARE	(bool) Mesh<Def>::IsValid(const TMeshFaceLink<Def>&link, bool may_be_null, bool may_be_marked)	const
{
	if (!link.triangle)
		return may_be_null;
	if (link.is_quad)
		return quad_field.Owns(link.quad) && (may_be_marked || !link.quad->marked);
	return triangle_field.Owns(link.triangle) && (may_be_marked || !link.triangle->marked);
}
	
template <class Def> MF_DECLARE	(sindex_t)	Mesh<Def>::linkToIndex(const TMeshFaceLink<Def>&link)	const
{
	if (!link.triangle)
		return TypeInfo<sindex_t>::max;
		
	if (link.is_quad)
		return -1-(sindex_t)(link.quad-quad_field);
	return link.triangle-triangle_field;
}
	
template <class Def> MF_DECLARE	(bool) Mesh<Def>::indexToLink(sindex_t index, TMeshFaceLink<Def>&link)
{
	if (index >= 0)
	{
		if ((index_t)index < triangle_field.GetLength())
		{
			link.triangle = triangle_field + index;
			link.is_quad = false;
			return true;
		}
		link.triangle = NULL;
		return false;
	}
		
	index = -index-1;

	if ((index_t)index < quad_field.GetLength())
	{
		link.quad = quad_field + index;
		link.is_quad = true;
		return true;
	}
	link.triangle = NULL;
	return false;
}
	
template <class Def>
	MF_DECLARE	(void)				Mesh<Def>::invert(unsigned selection)
	{
		if (selection&O_TRIANGLES)
			for (index_t i = 0; i < triangle_field.GetLength(); i++)
				swp(triangle_field[i].v0,triangle_field[i].v1);
		if (selection&O_QUADS)
			for (index_t i = 0; i < quad_field.GetLength(); i++)
			{
				swp(quad_field[i].v0, quad_field[i].v3);
				swp(quad_field[i].v1, quad_field[i].v2);
			}
		if (selection&O_EDGES)
			for (index_t i = 0; i < edge_field.GetLength(); i++)
				swp(edge_field[i].v0, edge_field[i].v1);
	}

template <class Def>
	MF_DECLARE	(bool)			Mesh<Def>::isConvex(bool update_edges)
	{
		return false;	//later
			
			
			
	
		
		
		
		
		
	}
		
template <class Def>
	MF_DECLARE	(void)			Mesh<Def>::generateEdges()
	{
		if (edge_field.GetLength())
			return;
		//typedef TGraphDef<TDef<typename Def::Type> >	GraphDef;

		Ctr::Array<PreAllocVector<index_t,2>	>		vertex_edge_list(vertex_field.GetLength());
		PreAllocVector<MeshEdge<Def> >			edge_buffer;

		/*Ctr::Array<MeshVertex<GraphDef> >	vtx(vertex_field.GetLength());
		for (index_t i = 0; i < vertex_field.GetLength(); i++)
		{
			vtx[i].marked = false;
			vtx[i].index = i;
		}*/

			
			
		//Ctr::Array<MeshTriangle<GraphDef> >	tri(triangle_field.GetLength());
		//Ctr::Array<MeshQuad<GraphDef> >		quad(quad_field.GetLength());

		for (index_t i = 0; i < triangle_field.GetLength(); i++)
		{
			MeshTriangle<Def>&t = triangle_field[i];
			/*t.v0 = vtx + (triangle_field[i].v0-vertex_field);
			t.v1 = vtx + (triangle_field[i].v1-vertex_field);
			t.v2 = vtx + (triangle_field[i].v2-vertex_field);
			t.marked = false;
			t.n0.triangle = NULL;
			t.n1.triangle = NULL;
			t.n2.triangle = NULL;*/
			
			for (BYTE k = 0; k < 3; k++)
			{
					MeshVertex<Def>	*vtx0 = t.vertex[k],
									*vtx1 = t.vertex[(k+1)%3];
				index_t		vtx_i0 = vtx0 - vertex_field,
							vtx_i1 = vtx1 - vertex_field;

				MeshEdge<Def>*found = NULL;

				for (index_t j = 0; j < vertex_edge_list[vtx_i0].Count(); j++)
				{
					MeshEdge<Def>&e = edge_buffer[vertex_edge_list[vtx_i0][j]];
					if (e.vertex[!e.requireIndexOf(vtx0)] == vtx1)
					{
						found = &e;
						break;
					}
				}

				if (!found)
				{
					index_t new_edge_i = edge_buffer.GetLength();
					MeshEdge<Def>&edge = edge_buffer.Append();
					edge.index = new_edge_i;
					edge.v0 = vtx0;
					edge.v1 = vtx1;
					edge.linkUnary(&t);
					edge.marked = false;

					vertex_edge_list[vtx_i0] << new_edge_i;
					vertex_edge_list[vtx_i1] << new_edge_i;
				}
				else
				{
					//must be triangle at this point
					/*char index = found->n[0].triangle->GetIndexOf(edge->v0);
					if (index == -1)
						FATAL__("broken index");*/
					//edge->n[0].triangle->n[index].triangle = &t;
					//edge->n[0].triangle->n[index].is_quad = false;
					//t.n[k].triangle = edge->n[0].triangle;
					//t.n[k].is_quad = false;
					found->n[1].triangle = &t;
					found->n[1].is_quad = false;
				}
				//t.edge[k] = edge;
			}
		}
			
		for (index_t i = 0; i < quad_field.GetLength(); i++)
		{
			MeshQuad<Def>&q = quad_field[i];
			/*q.v0 = vtx + (quad_field[i].v0-vertex_field);
			q.v1 = vtx + (quad_field[i].v1-vertex_field);
			q.v2 = vtx + (quad_field[i].v2-vertex_field);
			q.v3 = vtx + (quad_field[i].v3-vertex_field);*/
			//q.marked = false;
			/*q.n0.triangle = NULL;
			q.n1.triangle = NULL;
			q.n2.triangle = NULL;
			q.n3.triangle = NULL;*/
			
			for (BYTE k = 0; k < 4; k++)
			{
				MeshVertex<Def>	*vtx0 = q.vertex[k],
								*vtx1 = q.vertex[(k+1)%4];

				index_t		vtx_i0 = vtx0 - vertex_field,
							vtx_i1 = vtx1 - vertex_field;

				MeshEdge<Def>*found = NULL;

				for (index_t j = 0; j < vertex_edge_list[vtx_i0].Count(); j++)
				{
					MeshEdge<Def>&e = edge_buffer[vertex_edge_list[vtx_i0][j]];
					if (e.vertex[!e.requireIndexOf(vtx0)] == vtx1)
					{
						found = &e;
						break;
					}
				}

//					MeshEdge<GraphDef>	*edge = vtx0->findEdgeTo(vtx1);
				if (!found)
				{
					index_t new_edge_i = edge_buffer.GetLength();
					MeshEdge<Def>&edge = edge_buffer.Append();
					edge.index = new_edge_i;
					edge.v0 = vtx0;
					edge.v1 = vtx1;
					edge.linkUnary(&q);
					edge.marked = false;
						
					vertex_edge_list[vtx_i0] << new_edge_i;
					vertex_edge_list[vtx_i1] << new_edge_i;
				}
				else
				{
					/*if (found->n[0].is_quad)
					{
						char index = found->n[0].quad->GetIndexOf(edge->v0);
						if (index == -1)
							FATAL__("broken index");
						//found->n[0].quad->n[index].quad = &q;
						//edge->n[0].quad->n[index].is_quad = true;
						//q.n[k].quad = edge->n[0].quad;
						//q.n[k].is_quad = true;
					}
					else
					{
						char index = found->n[0].triangle->GetIndexOf(edge->v0);
						if (index == -1)
							FATAL__("broken index");
						edge->n[0].triangle->n[index].quad = &q;
						edge->n[0].triangle->n[index].is_quad = true;
						q.n[k].triangle = edge->n[0].triangle;
						q.n[k].is_quad = false;
					}*/
					found->n[1].quad = &q;
					found->n[1].is_quad = true;
				}
				//q.edge[k] = edge;
			}
		}
			
		edge_buffer.copyToArray(edge_field);
		/*
		for (index_t i = 0; i < edge_field.GetLength(); i++)
		{
			MeshEdge<Def>&target = edge_field[i];
			MeshEdge<GraphDef>*source = edge_buffer[i];
			target.index = i;
			target.marked = false;
			target.v0 = vertex_field + (source->v0-vtx);
			target.v1 = vertex_field + (source->v1-vtx);
			target.n[0].is_quad = source->n[0].is_quad;
			target.n[1].is_quad = source->n[1].is_quad;
			if (target.n[0].is_quad)
				target.n[0].quad = quad_field + (source->n[0].quad-quad);
			else
				target.n[0].triangle = triangle_field + (source->n[0].triangle-tri);
			if (source->n[1].triangle)
			{
				if (target.n[1].is_quad)
					target.n[1].quad = quad_field + (source->n[1].quad-quad);
				else
					target.n[1].triangle = triangle_field + (source->n[1].triangle-tri);
			}
			else
				target.n[1].triangle = NULL;
		}
		*/
		if (!valid())
			FATAL__(errorStr());
	}
	
template <class Def>
	MF_DECLARE	(void)			Mesh<Def>::toGraph(Mesh<TFaceGraphDef<Def> >&object, bool include_edges)		const
	{
		object.clear();
		if (!quad_field.GetLength() && !triangle_field.GetLength())
			return;

		typedef MeshVertex<TFaceGraphDef<Def> > Vtx;
		typedef MeshEdge<TFaceGraphDef<Def> >	Edg;
		typedef MeshTriangle<TFaceGraphDef<Def> >	Tri;
		typedef MeshQuad<TFaceGraphDef<Def> >	Quad;
			
			
		object.vertex_field.SetSize(vertex_field.GetLength());
		object.triangle_field.SetSize(triangle_field.GetLength());
		object.quad_field.SetSize(quad_field.GetLength());
		if (include_edges)
			object.edge_field.SetSize(edge_field.GetLength());
			
		object.vregion.first = 0;
		object.vregion.last = vertex_field.GetLength()-1;
			
		(*(typename Def::Mesh*)&object) = *this;
			
		for (index_t i = 0; i < vertex_field.GetLength(); i++)
		{
			Vtx&v = object.vertex_field[i];
			v.index = i;
			v.marked = false;
			_c3(vertex_field[i].position,v.position);
			(*(typename Def::Vertex*)&v) = vertex_field[i];
		}
		for (index_t i = 0; i < object.triangle_field.GetLength(); i++)
		{
			Tri&t = object.triangle_field[i];
			const MeshTriangle<Def>&source = triangle_field[i];
			t.v0 = object.vertex_field + (source.v0-vertex_field);
			t.v1 = object.vertex_field + (source.v1-vertex_field);
			t.v2 = object.vertex_field + (source.v2-vertex_field);
			t.marked = false;
			t.index = i;
			t.attrib = 0;
			t.n0 = t.v0->findFace(t.v1);
			t.n1 = t.v1->findFace(t.v2);
			t.n2 = t.v2->findFace(t.v0);
			t.n0.setNeighbor(t.v1,&t);
			t.n1.setNeighbor(t.v2,&t);
			t.n2.setNeighbor(t.v0,&t);
			t.v0->Insert(&t);
			t.v1->Insert(&t);
			t.v2->Insert(&t);
			(*(typename Def::Triangle*)&t) = source;
		}
		for (index_t i = 0; i < object.quad_field.GetLength(); i++)
		{
			Quad&q = object.quad_field[i];
			const MeshQuad<Def>&source = quad_field[i];
			q.v0 = object.vertex_field + (source.v0-vertex_field);
			q.v1 = object.vertex_field + (source.v1-vertex_field);
			q.v2 = object.vertex_field + (source.v2-vertex_field);
			q.v3 = object.vertex_field + (source.v3-vertex_field);
			q.marked = false;
			q.attrib = 0;
			q.index = i;
			q.n0 = q.v0->findFace(q.v1);
			q.n1 = q.v1->findFace(q.v2);
			q.n2 = q.v2->findFace(q.v3);
			q.n3 = q.v3->findFace(q.v0);
			q.n0.setNeighbor(q.v1,&q);
			q.n1.setNeighbor(q.v2,&q);
			q.n2.setNeighbor(q.v3,&q);
			q.n3.setNeighbor(q.v0,&q);
			q.v0->Insert(&q);
			q.v1->Insert(&q);
			q.v2->Insert(&q);
			q.v3->Insert(&q);
			(*(typename Def::Quad*)&q) = source;
		}
		if (include_edges)
			for (index_t i = 0; i < object.edge_field.GetLength(); i++)
			{
				Edg&e = object.edge_field[i];
				const MeshEdge<Def>&source = edge_field[i];
				e.v0 = object.vertex_field + (source.v0-vertex_field);
				e.v1 = object.vertex_field + (source.v1-vertex_field);
				e.marked = false;
				e.index = i;
				e.n0.is_quad = source.n0.is_quad;
				e.n1.is_quad = source.n1.is_quad;
				if (e.n0.is_quad)
					e.n0.quad = object.quad_field + (source.n0.quad-quad_field);
				else
					e.n0.triangle = object.triangle_field + (source.n0.triangle-triangle_field);
				if (source.n1.triangle)
				{
					if (e.n1.is_quad)
						e.n1.quad = object.quad_field + (source.n1.quad-quad_field);
					else
						e.n1.triangle = object.triangle_field + (source.n1.triangle-triangle_field);
				}
				else
					e.n1.triangle = NULL;
				(*(typename Def::Edge*)&e) = source;
			}
			
		if (!object.valid())
			FATAL__("object invalid");
	}
		
template <class Def>
	MF_DECLARE	(void) Mesh<Def>::toGraph(DynamicMesh<TFaceGraphDef<Def> >&object, bool include_edges)const
	{
		object.clear();
		if (!quad_field.GetLength() && !triangle_field.GetLength())
			return;

		typedef MeshVertex<TFaceGraphDef<Def> > Vtx;
		typedef MeshEdge<TFaceGraphDef<Def> >	Edg;
		typedef MeshTriangle<TFaceGraphDef<Def> >	Tri;
		typedef MeshQuad<TFaceGraphDef<Def> >	Quad;
			
			
			
		object.vregion.first = 0;
		object.vregion.last = vertex_field.GetLength()-1;
			
		(*(typename Def::Mesh*)&object) = *this;
			
		for (index_t i = 0; i < vertex_field.GetLength(); i++)
		{
			Vtx*v = object.vertices.Append();
			v->index = i;
			v->marked = false;
			_c3(vertex_field[i].position,v->position);
			(*(typename Def::Vertex*)v) = vertex_field[i];
		}
		for (index_t i = 0; i < triangle_field.GetLength(); i++)
		{
			Tri*t = object.triangles.Append();
			const MeshTriangle<Def>&source = triangle_field[i];
			t->v0 = object.vertices[source.v0-vertex_field];
			t->v1 = object.vertices[source.v1-vertex_field];
			t->v2 = object.vertices[source.v2-vertex_field];
			t->marked = false;
			t->index = i;
			t->attrib = 0;
			t->n0 = t->v0->findFace(t->v1);
			t->n1 = t->v1->findFace(t->v2);
			t->n2 = t->v2->findFace(t->v0);
			t->n0.setNeighbor(t->v1,t);
			t->n1.setNeighbor(t->v2,t);
			t->n2.setNeighbor(t->v0,t);				
			t->v0->Insert(t);
			t->v1->Insert(t);
			t->v2->Insert(t);
			(*(typename Def::Triangle*)t) = source;
		}
		for (index_t i = 0; i < quad_field.GetLength(); i++)
		{
			Quad&q = *object.quads.Append();
			const MeshQuad<Def>&source = quad_field[i];
			q.v0 = object.vertices.GetChar(source.v0-vertex_field);
			q.v1 = object.vertices.GetChar(source.v1-vertex_field);
			q.v2 = object.vertices.GetChar(source.v2-vertex_field);
			q.v3 = object.vertices.GetChar(source.v3-vertex_field);
			q.marked = false;
			q.attrib = 0;
			q.index = i;
			q.n0 = q.v0->findFace(q.v1);
			q.n1 = q.v1->findFace(q.v2);
			q.n2 = q.v2->findFace(q.v3);
			q.n3 = q.v3->findFace(q.v0);
			q.n0.setNeighbor(q.v1,&q);
			q.n1.setNeighbor(q.v2,&q);
			q.n2.setNeighbor(q.v3,&q);
			q.n3.setNeighbor(q.v0,&q);				
			q.v0->Insert(&q);
			q.v1->Insert(&q);
			q.v2->Insert(&q);
			q.v3->Insert(&q);
			(*(typename Def::Quad*)&q) = source;
		}
		if (include_edges)
			for (index_t i = 0; i < edge_field.GetLength(); i++)
			{
				Edg&e = *object.edges.Append();
				const MeshEdge<Def>&source = edge_field[i];
				e.v0 = object.vertices.GetChar(source.v0-vertex_field);
				e.v1 = object.vertices.GetChar(source.v1-vertex_field);
				e.marked = false;
				e.index = i;
				e.n0.is_quad = source.n0.is_quad;
				e.n1.is_quad = source.n1.is_quad;
				if (e.n0.is_quad)
					e.n0.quad = object.quads.GetChar(source.n0.quad-quad_field);
				else
					e.n0.triangle = object.triangles.GetChar(source.n0.triangle-triangle_field);
				if (source.n1.triangle)
				{
					if (e.n1.is_quad)
						e.n1.quad = object.quads.GetChar(source.n1.quad-quad_field);
					else
						e.n1.triangle = object.triangles.GetChar(source.n1.triangle-triangle_field);
				}
				else
					e.n1.triangle = NULL;
				(*(typename Def::Edge*)&e) = source;
			}
			
	}

template <class Def>
	MF_DECLARE	(void)			Mesh<Def>::toGraph(Mesh<TGraphDef<Def> >&object)		const
	{
		object.clear();
		if (!quad_field.GetLength() && !triangle_field.GetLength())
			return;

		typedef MeshVertex<TGraphDef<Def> > Vtx;
		typedef MeshEdge<TGraphDef<Def> >	Edg;
		typedef MeshTriangle<TGraphDef<Def> >	Tri;
		typedef MeshQuad<TGraphDef<Def> >	Quad;
			
			
		object.vertex_field.SetSize(vertex_field.GetLength());
		object.triangle_field.SetSize(triangle_field.GetLength());
		object.quad_field.SetSize(quad_field.GetLength());
		object.edge_field.SetSize(edge_field.GetLength());
			
		object.vregion.first = 0;
		object.vregion.last = vertex_field.GetLength()-1;
			
		(*(typename Def::Mesh*)&object) = *this;
			
		for (index_t i = 0; i < vertex_field.GetLength(); i++)
		{
			Vtx&v = object.vertex_field[i];
			v.index = i;
			v.marked = false;
			_c3(vertex_field[i].position,v.position);
			(*(typename Def::Vertex*)&v) = vertex_field[i];
		}
		for (index_t i = 0; i < object.triangle_field.GetLength(); i++)
		{
			Tri&t = object.triangle_field[i];
			const MeshTriangle<Def>&source = triangle_field[i];
			t.v0 = object.vertex_field + (source.v0-vertex_field);
			t.v1 = object.vertex_field + (source.v1-vertex_field);
			t.v2 = object.vertex_field + (source.v2-vertex_field);
			t.marked = false;
			t.index = i;
			t.attrib = 0;
			t.n0.triangle = NULL;
			t.n1.triangle = NULL;
			t.n2.triangle = NULL;
			t.e0 = NULL;
			t.e1 = NULL;
			t.e2 = NULL;
			(*(typename Def::Triangle*)&t) = source;
		}
		for (index_t i = 0; i < object.quad_field.GetLength(); i++)
		{
			Quad&q = object.quad_field[i];
			const MeshQuad<Def>&source = quad_field[i];
			q.v0 = object.vertex_field + (source.v0-vertex_field);
			q.v1 = object.vertex_field + (source.v1-vertex_field);
			q.v2 = object.vertex_field + (source.v2-vertex_field);
			q.v3 = object.vertex_field + (source.v3-vertex_field);
			q.marked = false;
			q.attrib = 0;
			q.index = i;
			q.n0.triangle = NULL;
			q.n1.triangle = NULL;
			q.n2.triangle = NULL;
			q.n3.triangle = NULL;
			q.e0 = NULL;
			q.e1 = NULL;
			q.e2 = NULL;
			q.e3 = NULL;
			(*(typename Def::Quad*)&q) = source;
		}
			
		for (index_t i = 0; i < object.edge_field.GetLength(); i++)
		{
			Edg&e = object.edge_field[i];
			const MeshEdge<Def>&source = edge_field[i];
			e.v0 = object.vertex_field + (source.v0-vertex_field);
			e.v1 = object.vertex_field + (source.v1-vertex_field);
			e.marked = false;
			e.index = i;
			e.n0.is_quad = source.n0.is_quad;
			e.n1.is_quad = source.n1.is_quad;
			if (e.n0.is_quad)
				e.n0.quad = object.quad_field + (source.n0.quad-quad_field);
			else
				e.n0.triangle = object.triangle_field + (source.n0.triangle-triangle_field);
			if (source.n1.triangle)
			{
				if (e.n1.is_quad)
					e.n1.quad = object.quad_field + (source.n1.quad-quad_field);
				else
					e.n1.triangle = object.triangle_field + (source.n1.triangle-triangle_field);
			}
			else
				e.n1.triangle = NULL;
			e.next[0] = NULL;
			e.next[1] = NULL;
				
			e.v0->Insert(&e);
			e.v1->Insert(&e);
				
			if (e.n1.triangle)
			{
				if (e.n0.is_quad)
				{
					char	index0 = e.n0.quad->GetIndexOf(e.v0),
							index1 = e.n0.quad->GetIndexOf(e.v1);
					if (index0 == -1 || index1 == -1)
						FATAL__("linkage broken");
					if (index1 == (index0+1)%4)
					{
						e.n0.quad->n[index0] = e.n1;
						e.n0.quad->edge[index0] = &e;
					}
					else
					{
						e.n0.quad->n[index1] = e.n1;
						e.n0.quad->edge[index1] = &e;
					}
				}
				else
				{
					char	index0 = e.n0.triangle->GetIndexOf(e.v0),
							index1 = e.n0.triangle->GetIndexOf(e.v1);
					if (index0 == -1 || index1 == -1)
						FATAL__("linkage broken");
					if (index1 == (index0+1)%3)
					{
						e.n0.triangle->n[index0] = e.n1;
						e.n0.triangle->edge[index0] = &e;
					}
					else
					{
						e.n0.triangle->n[index1] = e.n1;
						e.n0.triangle->edge[index1] = &e;
					}
				}
					
				if (e.n1.is_quad)
				{
					char	index0 = e.n1.quad->GetIndexOf(e.v0),
							index1 = e.n1.quad->GetIndexOf(e.v1);
					if (index0 == -1 || index1 == -1)
						FATAL__("linkage broken");
					if (index1 == (index0+1)%4)
					{
						e.n1.quad->n[index0] = e.n0;
						e.n1.quad->edge[index0] = &e;
					}
					else
					{
						e.n1.quad->n[index1] = e.n0;
						e.n1.quad->edge[index1] = &e;
					}
				}
				else
				{
					char	index0 = e.n1.triangle->GetIndexOf(e.v0),
							index1 = e.n1.triangle->GetIndexOf(e.v1);
					if (index0 == -1 || index1 == -1)
						FATAL__("linkage broken");
					if (index1 == (index0+1)%3)
					{
						e.n1.triangle->n[index0] = e.n0;
						e.n1.triangle->edge[index0] = &e;
					}
					else
					{
						e.n1.triangle->n[index1] = e.n0;
						e.n1.triangle->edge[index1] = &e;
					}
				}
			}
			else
			{
				if (e.n0.is_quad)
				{
					char	index0 = e.n0.quad->GetIndexOf(e.v0),
							index1 = e.n0.quad->GetIndexOf(e.v1);
					if (index0 == -1 || index1 == -1)
						FATAL__("linkage broken");
					if (index1 == (index0+1)%4)
						e.n0.quad->edge[index0] = &e;
					else
						e.n0.quad->edge[index1] = &e;
				}
				else
				{
					char	index0 = e.n0.triangle->GetIndexOf(e.v0),
							index1 = e.n0.triangle->GetIndexOf(e.v1);
					if (index0 == -1 || index1 == -1)
						FATAL__("linkage broken");
					if (index1 == (index0+1)%3)
						e.n0.triangle->edge[index0] = &e;
					else
						e.n0.triangle->edge[index1] = &e;
				}
			}
			(*(typename Def::Edge*)&e) = source;
		}
			
		if (!object.valid())
			FATAL__("object invalid");
	}
	
	
template <class Def>
	MF_DECLARE	(void)			Mesh<Def>::toGraph(DynamicMesh<TGraphDef<Def> >&object)		const
	{
		object.clear();
		if (!quad_field.GetLength() && !triangle_field.GetLength())
			return;

		typedef MeshVertex<TGraphDef<Def> > Vtx;
		typedef MeshEdge<TGraphDef<Def> >	Edg;
		typedef MeshTriangle<TGraphDef<Def> >	Tri;
		typedef MeshQuad<TGraphDef<Def> >	Quad;
			
			
		object.vregion.first = 0;
		object.vregion.last = vertex_field.GetLength()-1;
			
		(*(typename Def::Mesh*)&object) = *this;
			
		for (index_t i = 0; i < vertex_field.GetLength(); i++)
		{
			Vtx*v = object.vertices.Append();
			v->index = i;
			v->marked = false;
			_c3(vertex_field[i].position,v->position);
			(*(typename Def::Vertex*)v) = vertex_field[i];
		}
		for (index_t i = 0; i < triangle_field.GetLength(); i++)
		{
			Tri*t = object.triangles.Append();
			const MeshTriangle<Def>&source = triangle_field[i];
			t->v0 = object.vertices[(source.v0-vertex_field)];
			t->v1 = object.vertices[(source.v1-vertex_field)];
			t->v2 = object.vertices[(source.v2-vertex_field)];
			t->marked = false;
			t->index = i;
			t->attrib = 0;
			t->n0.triangle = NULL;
			t->n1.triangle = NULL;
			t->n2.triangle = NULL;
			t->e0 = NULL;
			t->e1 = NULL;
			t->e2 = NULL;
			(*(typename Def::Triangle*)t) = source;
		}
		for (index_t i = 0; i < quad_field.GetLength(); i++)
		{
			Quad*q = object.quads.Append();
			const MeshQuad<Def>&source = quad_field[i];
			q->v0 = object.vertices[source.v0-vertex_field];
			q->v1 = object.vertices[source.v1-vertex_field];
			q->v2 = object.vertices[source.v2-vertex_field];
			q->v3 = object.vertices[source.v3-vertex_field];
			q->marked = false;
			q->index = i;
			q->attrib = 0;
			q->n0.triangle = NULL;
			q->n1.triangle = NULL;
			q->n2.triangle = NULL;
			q->n3.triangle = NULL;
			q->e0 = NULL;
			q->e1 = NULL;
			q->e2 = NULL;
			q->e3 = NULL;
			(*(typename Def::Quad*)q) = source;
		}
			
		for (index_t i = 0; i < edge_field.GetLength(); i++)
		{
			Edg*e = object.edges.Append();
			const MeshEdge<Def>&source = edge_field[i];
			e->v0 = object.vertices[source.v0-vertex_field];
			e->v1 = object.vertices[source.v1-vertex_field];
			e->marked = false;
			e->index = i;
			e->n0.is_quad = source.n0.is_quad;
			e->n1.is_quad = source.n1.is_quad;
			if (e->n0.is_quad)
				e->n0.quad = object.quads[source.n0.quad-quad_field];
			else
				e->n0.triangle = object.triangles[source.n0.triangle-triangle_field];
			if (source.n1.triangle)
			{
				if (e->n1.is_quad)
					e->n1.quad = object.quads[source.n1.quad-quad_field];
				else
					e->n1.triangle = object.triangles[source.n1.triangle-triangle_field];
			}
			else
				e->n1.triangle = NULL;
			e->next[0] = NULL;
			e->next[1] = NULL;
				
			e->v0->Insert(e);
			e->v1->Insert(e);
				
			if (e->n1.triangle)
			{
				if (e->n0.is_quad)
				{
					char	index0 = e->n0.quad->GetIndexOf(e->v0),
							index1 = e->n0.quad->GetIndexOf(e->v1);
					if (index0 == -1 || index1 == -1)
						FATAL__("linkage broken");
					if (index1 == (index0+1)%4)
					{
						e->n0.quad->n[index0] = e->n1;
						e->n0.quad->edge[index0] = e;
					}
					else
					{
						e->n0.quad->n[index1] = e->n1;
						e->n0.quad->edge[index1] = e;
					}
				}
				else
				{
					char	index0 = e->n0.triangle->GetIndexOf(e->v0),
							index1 = e->n0.triangle->GetIndexOf(e->v1);
					if (index0 == -1 || index1 == -1)
						FATAL__("linkage broken");
					if (index1 == (index0+1)%3)
					{
						e->n0.triangle->n[index0] = e->n1;
						e->n0.triangle->edge[index0] = e;
					}
					else
					{
						e->n0.triangle->n[index1] = e->n1;
						e->n0.triangle->edge[index1] = e;
					}
				}
					
				if (e->n1.is_quad)
				{
					char	index0 = e->n1.quad->GetIndexOf(e->v0),
							index1 = e->n1.quad->GetIndexOf(e->v1);
					if (index0 == -1 || index1 == -1)
						FATAL__("linkage broken");
					if (index1 == (index0+1)%4)
					{
						e->n1.quad->n[index0] = e->n0;
						e->n1.quad->edge[index0] = e;
					}
					else
					{
						e->n1.quad->n[index1] = e->n0;
						e->n1.quad->edge[index1] = e;
					}
				}
				else
				{
					char	index0 = e->n1.triangle->GetIndexOf(e->v0),
							index1 = e->n1.triangle->GetIndexOf(e->v1);
					if (index0 == -1 || index1 == -1)
						FATAL__("linkage broken");
					if (index1 == (index0+1)%3)
					{
						e->n1.triangle->n[index0] = e->n0;
						e->n1.triangle->edge[index0] = e;
					}
					else
					{
						e->n1.triangle->n[index1] = e->n0;
						e->n1.triangle->edge[index1] = e;
					}
				}
			}
			else
			{
				if (e->n0.is_quad)
				{
					char	index0 = e->n0.quad->GetIndexOf(e->v0),
							index1 = e->n0.quad->GetIndexOf(e->v1);
					if (index0 == -1 || index1 == -1)
						FATAL__("linkage broken");
					if (index1 == (index0+1)%4)
						e->n0.quad->edge[index0] = e;
					else
						e->n0.quad->edge[index1] = e;
				}
				else
				{
					char	index0 = e->n0.triangle->GetIndexOf(e->v0),
							index1 = e->n0.triangle->GetIndexOf(e->v1);
					if (index0 == -1 || index1 == -1)
						FATAL__("linkage broken");
					if (index1 == (index0+1)%3)
						e->n0.triangle->edge[index0] = e;
					else
						e->n0.triangle->edge[index1] = e;
				}
			}				
			(*(typename Def::Edge*)e) = source;
		}
	}
		
template <class Def>
	MF_DECLARE	(typename Def::Type)	Mesh<Def>::getVolume(bool alternate)										const
	{
		if (!alternate)
		{
			typename Def::Type sum(0);
			for (index_t i = 0; i < triangle_field.GetLength(); i++)
				sum += Obj::zeroTetrahedronVolume(triangle_field[i].v0->position,triangle_field[i].v1->position,triangle_field[i].v2->position);
			for (index_t i = 0; i < quad_field.GetLength(); i++)
			{
				sum += Obj::zeroTetrahedronVolume(quad_field[i].v0->position,quad_field[i].v1->position,quad_field[i].v2->position);
				sum += Obj::zeroTetrahedronVolume(quad_field[i].v0->position,quad_field[i].v2->position,quad_field[i].v3->position);
			}
			return sum;
		}
		
		Ctr::Array<typename Def::Type>	volume_field(triangle_field.GetLength()+2*quad_field.GetLength());
		if (!volume_field.GetLength())
			return 0;
		typename Def::Type	*p = volume_field.pointer();
		for (index_t i = 0; i < triangle_field.GetLength(); i++)
			(*p++) = Obj::zeroTetrahedronVolume(triangle_field[i].v0->position,triangle_field[i].v1->position,triangle_field[i].v2->position);
		for (index_t i = 0; i < quad_field.GetLength(); i++)
		{
			(*p++) = Obj::zeroTetrahedronVolume(quad_field[i].v0->position,quad_field[i].v1->position,quad_field[i].v2->position);
			(*p++) = Obj::zeroTetrahedronVolume(quad_field[i].v0->position,quad_field[i].v2->position,quad_field[i].v3->position);
		}
		if (p != volume_field+volume_field.GetLength())
			FATAL__("volume calculation error");
		typename Def::Type	*begin = volume_field.pointer(),
							*end = &volume_field.Last();
		while (begin < end)	//sort field so that it begins with positive volumes and ends with negative volumes
		{
			while (*begin >= 0 && begin <=end)
				begin++;
			while (*end < 0 && end >= begin)
				end--;
			if (begin < end)
				swp(*begin,*end);
		}	//begin now points to the first negative element, end to the last positive
		typename Def::Type	sum(0);
		begin = volume_field.pointer();
		end = &volume_field.Last();
		while (begin < end)
		{
			sum += (*begin++);
			sum += (*end--);
		}
		if (begin == end)
			sum += (*begin);
		return sum;
			
			
			
		/*
		typedef Def::Type	*positive = volume_field,
							*negative = begin;
		end = volume_field+volume_field.GetLength();
			
		while (positive != begin && negative != end)
		{
			sum += (*positive++);
			sum += (*negative++);
		}
			
		while (positive != begin)
			sum += (*positive++);
		while (negative != end)
			sum += (*negative++);*/
	}
		
	
template <class Def> MF_DECLARE	(void)	Mesh<Def>::unmark(unsigned selection)
{
	if (selection&O_VERTICES)
		for (index_t i = 0; i < vertex_field.GetLength(); i++)
			vertex_field[i].marked = false;
	if (selection&O_TRIANGLES)
		for (index_t i = 0; i < triangle_field.GetLength(); i++)
			triangle_field[i].marked = false;
	if (selection&O_QUADS)
		for (index_t i = 0; i < quad_field.GetLength(); i++)
			quad_field[i].marked = false;
	if (selection&O_EDGES)
		for (index_t i = 0; i < edge_field.GetLength(); i++)
			edge_field[i].marked = false;
}
	

template <class Def> MF_DECLARE	(void) Mesh<Def>::correct(unsigned selection)
{
			
	Ctr::Array<MeshTriangle<Def> >	new_triangles;
	Ctr::Array<MeshQuad<Def> >	new_quads;
		
	bool 	replace_triangles(false),
			replace_quads(false);
		
		
	if (selection&O_TRIANGLES)
	{
		count_t marked = 0;
			
		for (index_t i = 0; i < triangle_field.GetLength(); i++)
		{
			MeshTriangle<Def>&triangle = triangle_field[i];
			triangle.marked = false;
			for (BYTE k = 0; k < 3; k++)
				triangle.marked |=	(!vertex_field.Owns(triangle.vertex[k])
									||
									triangle_field[i].vertex[k] == triangle_field[i].vertex[(k+1)%3]);
			marked += triangle.marked;
		}
			
		if (marked)
		{
			replace_triangles = true;
			new_triangles.SetSize(triangle_field.GetLength()-marked);
			MeshTriangle<Def>*at = new_triangles.pointer();
				
			for (index_t i = 0; i < triangle_field.GetLength(); i++)
				if (!triangle_field[i].marked)
					(*at++) = triangle_field[i];
		}
	}
				
	if (selection&O_QUADS)
	{
		count_t marked = 0;
				
		for (index_t i = 0; i < quad_field.GetLength(); i++)
		{
			MeshQuad<Def>&quad = quad_field[i];
			quad.marked = false;
			for (BYTE k = 0; k < 4; k++)
				quad.marked |=	(!vertex_field.Owns(quad_field[i].vertex[k])
								||
								quad_field[i].vertex[k] == quad_field[i].vertex[(k+1)%4]
								||
								quad_field[i].vertex[k] == quad_field[i].vertex[(k+2)%4]);
			marked += quad.marked;
		}
			
		if (marked)
		{
			replace_quads = true;
			new_quads.SetSize(quad_field.GetLength()-marked);
			MeshQuad<Def>*at = new_quads.pointer();
				
			for (index_t i = 0; i < quad_field.GetLength(); i++)
				if (!quad_field[i].marked)
					(*at++) = quad_field[i];
		}
	}
		
	if ((selection&O_EDGES) || replace_quads || replace_triangles)
	{
		count_t marked = 0;
		for (index_t i = 0; i < edge_field.GetLength(); i++)
		{
			MeshEdge<Def>&edge = edge_field[i];
			edge.marked = (	!vertex_field.Owns(edge.v0)
							||
							!vertex_field.Owns(edge.v1));
								
			if (!edge.marked)
			{
				if (!IsValid(edge.n[0],false,false))
				{
					edge.n[0] = edge.n[1];
					edge.n[1].triangle = NULL;
					edge.marked = !IsValid(edge.n[0]);
				}
				else
					if (!IsValid(edge.n[1],true,false))
						edge.n[1].triangle = NULL;
			}
			marked += edge.marked;
		}
			
		if (marked)
		{
			Ctr::Array<MeshEdge<Def> >	new_edges(edge_field.GetLength()-marked);
			MeshEdge<Def>*at = new_edges.pointer();
			for (index_t i = 0; i < edge_field.GetLength(); i++)
				if (!edge_field[i].marked)
					(*at++) = edge_field[i];
			edge_field.adoptData(new_edges);
		}
	}
		
	if (replace_quads)
	{
		for (index_t i = 0; i < edge_field.GetLength(); i++)
		{
			MeshEdge<Def>&edge=edge_field[i];
			if (edge.n[0].is_quad)
				edge.n[0].quad = new_quads + (int)(edge.n[0].quad - quad_field);
			if (edge.n[1].is_quad && edge.n[1].quad)
				edge.n[1].quad = new_quads + (int)(edge.n[1].quad - quad_field);
		}
		quad_field.adoptData(new_quads);
	}
		
	if (replace_triangles)
	{
		for (index_t i = 0; i < edge_field.GetLength(); i++)
		{
			MeshEdge<Def>&edge=edge_field[i];
			if (!edge.n[0].is_quad)
				edge.n[0].triangle = new_triangles + (int)(edge.n[0].triangle - triangle_field);
			if (!edge.n[1].is_quad && edge.n[1].triangle)
				edge.n[1].triangle = new_triangles + (int)(edge.n[1].triangle - triangle_field);
		}
		triangle_field.adoptData(new_triangles);
	}
}
	
template <class Def> MF_DECLARE	(String) Mesh<Def>::errorStr() const
{
	return _error;
}


template <class Def> MF_DECLARE	(String) Mesh<Def>::difference(Mesh<Def>&other, const String&intend)
{
	String rs;
	#if 0
	#define COMPARE(cnt,token){\
			if (cnt != other.cnt)\
				rs+="\n"+intend+#cnt+" "+IntToStr(cnt)+" != "+IntToStr(other.cnt);\
			if ((token == other.token) && token)\
				rs+="\n"+intend+#token+": pointers equal ("+IntToHex((int)token,8)+" == "+IntToHex((int)other.token,8)+")";}

	COMPARE(vertex_field.GetLength(),vertex);
	COMPARE(edge_field.GetLength(),edge);
	COMPARE(face_field.GetLength(),face);
	#undef COMPARE
	#endif
	count_t difference(0);
	for (index_t i = 0; i < vertex_field.GetLength()&&i < other.vertex_field.GetLength(); i++)
		if (!_similar(vertex_field[i].position,other.vertex_field[i].position))
		{
			difference++;
			if (difference == 1)
				rs+="\n"+intend+"vertex difference{"+M::Vec::toString(vertex_field[i].position)+" != "+M::Vec::toString(other.vertex_field[i].position);
			if (difference < 5)
				rs+=String(", ")+M::Vec::toString(vertex_field[i].position)+" != "+M::Vec::toString(other.vertex_field[i].position);
		}
	if (difference >= 5)
		rs+="...";
	if (difference)
		rs+="}";
	return rs;
}


template <class Def>
	MF_DECLARE	(void) Mesh<Def>::buildMap(BYTE tag, unsigned max_depth)
	{
		if (_map)
			_map->reset(max_depth);
		else
			_map = SignalNew(new ObjMap<Def>(max_depth));
		_map->map(*this,tag);
	}
	
template <class Def>
	MF_DECLARE	(void)		Mesh<Def>::adoptData(Mesh<Def>&other)
	{
		vertex_field.adoptData(other.vertex_field);
		edge_field.adoptData(other.edge_field);
		quad_field.adoptData(other.quad_field);
		triangle_field.adoptData(other.triangle_field);
		if (_map)
			Discard(_map);
		_map = other._map;
		other._map = NULL;
		(*(typename Def::Mesh*)this) = other;
	}

template <class Def>
	MF_DECLARE	(void)		Mesh<Def>::swap(Mesh<Def>&other)
	{
		vertex_field.swap(other.vertex_field);
		edge_field.swap(other.edge_field);
		quad_field.swap(other.quad_field);
		triangle_field.swap(other.triangle_field);
		swp(_map,other._map);
		(*(typename Def::Mesh*)this) = other;
	}

template <class Def>MF_DECLARE	(Mesh<Def>&) Mesh<Def>::operator=(const Mesh<Def>&other)
{
	vertex_field = other.vertex_field;
	edge_field = other.edge_field;
	quad_field = other.quad_field;
	triangle_field = other.triangle_field;

		
	for (index_t i = 0; i < edge_field.GetLength(); i++)
	{
		edge_field[i].v0 = vertex_field + (other.edge_field[i].v0-other.vertex_field);
		edge_field[i].v1 = vertex_field + (other.edge_field[i].v1-other.vertex_field);
		if (other.edge_field[i].n[0].is_quad)
			edge_field[i].n[0].Set(quad_field + (other.edge_field[i].n[0].quad-other.quad_field));
		else
			edge_field[i].n[0].Set(triangle_field + (other.edge_field[i].n[0].triangle-other.triangle_field));
		if (other.edge_field[i].n[1])
		{
			if (other.edge_field[i].n[1].is_quad)
				edge_field[i].n[1].Set(quad_field + (other.edge_field[i].n[1].quad-other.quad_field));
			else
				edge_field[i].n[1].Set(triangle_field + (other.edge_field[i].n[1].triangle-other.triangle_field));
		}
	}
	for (index_t i = 0; i < triangle_field.GetLength(); i++)
	{
		triangle_field[i].v0 = vertex_field + (other.triangle_field[i].v0-other.vertex_field);
		triangle_field[i].v1 = vertex_field + (other.triangle_field[i].v1-other.vertex_field);
		triangle_field[i].v2 = vertex_field + (other.triangle_field[i].v2-other.vertex_field);
	}
	for (index_t i = 0; i < quad_field.GetLength(); i++)
	{
		quad_field[i].v0 = vertex_field + (other.quad_field[i].v0-other.vertex_field);
		quad_field[i].v1 = vertex_field + (other.quad_field[i].v1-other.vertex_field);
		quad_field[i].v2 = vertex_field + (other.quad_field[i].v2-other.vertex_field);
		quad_field[i].v3 = vertex_field + (other.quad_field[i].v3-other.vertex_field);
	}
	flushMap();
	return *this;
}

template <class Def>
	MF_DECLARE(void)	Mesh<Def>::flushMap()
	{
		if (_map)
			Discard(_map);
		_map = NULL;
	}
	
template <class Def>
template <class T> MF_DECLARE	(Mesh<Def>&) Mesh<Def>::operator=(const Mesh<T>&other)
{
	vertex_field = other.vertex_field;
	edge_field = other.edge_field;
	quad_field = other.quad_field;
	triangle_field = other.triangle_field;

		
	for (index_t i = 0; i < edge_field.GetLength(); i++)
	{
		edge_field[i].v0 = vertex_field + (other.edge_field[i].v0-other.vertex_field);
		edge_field[i].v1 = vertex_field + (other.edge_field[i].v1-other.vertex_field);
		if (edge_field[i].n[0].is_quad)
			edge_field[i].n[0].quad = quad_field + (other.edge_field[i].quad[0]-other.quad_field);
		else
			edge_field[i].n[0].triangle = triangle_field + (other.edge_field[i].triangle[0]-other.triangle_field);
		if (other.edge_field[i].triangle[1])
		{
			if (edge_field[i].n[1].is_quad)
				edge_field[i].n[1].quad = quad_field + (other.edge_field[i].quad[1]-other.quad_field);
			else
				edge_field[i].n[1].triangle = triangle_field + (other.edge_field[i].triangle[1]-other.triangle_field);
		}
	}
	for (index_t i = 0; i < triangle_field.GetLength(); i++)
	{
		triangle_field[i].v0 = vertex_field + (other.triangle_field[i].v0-other.vertex_field);
		triangle_field[i].v1 = vertex_field + (other.triangle_field[i].v1-other.vertex_field);
		triangle_field[i].v2 = vertex_field + (other.triangle_field[i].v2-other.vertex_field);
	}
	for (index_t i = 0; i < quad_field.GetLength(); i++)
	{
		quad_field[i].v0 = vertex_field + (other.quad_field[i].v0-other.vertex_field);
		quad_field[i].v1 = vertex_field + (other.quad_field[i].v1-other.vertex_field);
		quad_field[i].v2 = vertex_field + (other.quad_field[i].v2-other.vertex_field);
		quad_field[i].v3 = vertex_field + (other.quad_field[i].v3-other.vertex_field);
	}
	flushMap();
	return *this;
}
	

template <class Def>Mesh<Def>::~Mesh()
{
	flushMap();
}

template <class Def>
	MF_DECLARE	(void) Mesh<Def>::clear()
	{
		vertex_field.free();
		edge_field.free();
		triangle_field.free();
		quad_field.free();
		flushMap();
	}
	
template <class Def>
	MF_DECLARE	(void)	Mesh<Def>::_mergeIn(MeshVertex<Def>*&voffset,MeshEdge<Def>*&eoffset,MeshTriangle<Def>*&toffset,MeshQuad<Def>*&qoffset,const Mesh<Def>&obj)
	{
		for (index_t i = 0; i < obj.vertex_field.GetLength(); i++)
			voffset[i] = obj.vertex_field[i];
		for (index_t i = 0; i < obj.edge_field.GetLength(); i++)
		{
			eoffset[i] = obj.edge_field[i];
			eoffset[i].v0 = voffset + (obj.edge_field[i].v0-obj.vertex_field);
			eoffset[i].v1 = voffset + (obj.edge_field[i].v1-obj.vertex_field);
		}
			
		for (index_t i = 0; i < obj.triangle_field.GetLength(); i++)
		{
			toffset[i] = obj.triangle_field[i];
			toffset[i].v0 = voffset + (obj.triangle_field[i].v0-obj.vertex_field);
			toffset[i].v1 = voffset + (obj.triangle_field[i].v1-obj.vertex_field);
			toffset[i].v2 = voffset + (obj.triangle_field[i].v2-obj.vertex_field);
		}
			
		for (index_t i = 0; i < obj.quad_field.GetLength(); i++)
		{
			qoffset[i] = obj.quad_field[i];
			qoffset[i].v0 = voffset + (obj.quad_field[i].v0-obj.vertex_field);
			qoffset[i].v1 = voffset + (obj.quad_field[i].v1-obj.vertex_field);
			qoffset[i].v2 = voffset + (obj.quad_field[i].v2-obj.vertex_field);
			qoffset[i].v3 = voffset + (obj.quad_field[i].v3-obj.vertex_field);
		}
			
		for (index_t i = 0; i < obj.edge_field.GetLength(); i++)
		{
			if (obj.edge_field[i].n[0].is_quad)
				eoffset[i].n[0].Set(qoffset+(obj.edge_field[i].n[0].quad-obj.quad_field));
			else
				eoffset[i].n[0].Set(toffset+(obj.edge_field[i].n[0].triangle-obj.triangle_field));
				
			if (obj.edge_field[i].n[1])
				if (obj.edge_field[i].n[1].is_quad)
					eoffset[i].n[1].Set(qoffset+(obj.edge_field[i].n[1].quad-obj.quad_field));
				else
					eoffset[i].n[1].Set(toffset+(obj.edge_field[i].n[1].triangle-obj.triangle_field));
		}
			
		voffset += obj.vertex_field.GetLength();
		eoffset += obj.edge_field.GetLength();
		toffset += obj.triangle_field.GetLength();
		qoffset += obj.quad_field.GetLength();
	}

template <class Def>
	MF_DECLARE	(void)	Mesh<Def>::join(const Ctr::Array<Mesh<Def> >&others)
	{
		join(others.pointer(),others.Count());
	}

		
template <class Def>
	MF_DECLARE	(void) Mesh<Def>::join(const Mesh<Def>*other_objects, count_t count)
	{
		count_t	join_vertices = 0,
				join_edges = 0,
				join_triangles = 0,
				join_quads = 0;
		for (index_t i = 0; i < count; i++)
		{
			join_vertices += other_objects[i].vertex_field.GetLength();
			join_edges += other_objects[i].edge_field.GetLength();
			join_triangles += other_objects[i].triangle_field.GetLength();
			join_quads += other_objects[i].quad_field.GetLength();
		}
		if (!join_vertices)
			return;
			
		Ctr::Array<MeshVertex<Def> >	vfield(vertex_field.GetLength()+join_vertices);
		Ctr::Array<MeshEdge<Def> >		efield(edge_field.GetLength()+join_edges);
		Ctr::Array<MeshTriangle<Def> >	tfield(triangle_field.GetLength()+join_triangles);
		Ctr::Array<MeshQuad<Def> >		qfield(quad_field.GetLength()+join_quads);
			
		MeshVertex<Def>*voffset = vfield;
		MeshEdge<Def>*eoffset = efield;
		MeshTriangle<Def>*toffset = tfield;
		MeshQuad<Def>*qoffset = qfield;
			
		_mergeIn(voffset,eoffset,toffset,qoffset,*this);
		for (index_t i = 0; i < count; i++)
			_mergeIn(voffset,eoffset,toffset,qoffset,other_objects[i]);
			
		ASSERT_CONCLUSION(vfield,voffset);
		ASSERT_CONCLUSION(efield,eoffset);
		ASSERT_CONCLUSION(tfield,toffset);
		ASSERT_CONCLUSION(qfield,qoffset);
			
		vertex_field.adoptData(vfield);
		edge_field.adoptData(efield);
		quad_field.adoptData(qfield);
		triangle_field.adoptData(tfield);
	}
	
template <class Def>MF_DECLARE	(void) Mesh<Def>::vertexDimensions(M::Box<typename Def::Type>&field)	const
{
	if (!vertex_field.GetLength())
		return;
	field.upper = field.lower = vertex_field.First().position;
	//cout << "Init: " << M::Vec::toString(field.min)<< " < "<<M::Vec::toString(field.max)<<endl;
	for (index_t i = 1; i < vertex_field.GetLength(); i++)
	{
		_oDetDimension(vertex_field[i].position,field);
		//ASSERT__(!M::Vec::oneLess(vertex_field[i].position,field.min));
		//ASSERT__(!M::Vec::oneGreater(vertex_field[i].position,field.max));
		//cout << "Included "<<M::Vec::toString(vertex_field[i].position)<<" => " << M::Vec::toString(field.min)<< " < "<<M::Vec::toString(field.max)<<endl;
			
	}
}
	
template <class Def> template <typename T>
	MF_DECLARE	(void)	Mesh<Def>::scale(const T&factor)
	{
		for (index_t i = 0; i < vertex_field.GetLength(); i++)
			M::Vec::mult(vertex_field[i].position,factor);
	}
	


template <class Def>MF_DECLARE	(void) Mesh<Def>::Resize(count_t vcnt, count_t ecnt, count_t tcnt, count_t qcnt)
{
	vertex_field.SetSize(vcnt);
	edge_field.SetSize(ecnt);
	triangle_field.SetSize(tcnt);
	quad_field.SetSize(qcnt);
}

	
template <class Def>MF_DECLARE	 (String) Mesh<Def>::ToString() const
{
	return "Mesh (vertices: "+String(vertex_field.GetLength())+"; edges: "+String(edge_field.GetLength())+"; triangles: "+String(triangle_field.GetLength())+"; quads: "+String(quad_field.GetLength())+"; valid: "+String((int)valid())+")";
}

template <class Def>
	MF_DECLARE	(void)		Mesh<Def>::generateNormals()
	{
		Concurrency::parallel_for(index_t(0),vertex_field.GetLength(),[this](index_t i)
		{
			M::Vec::clear(vertex_field[i].normal);
		});
		for (index_t i = 0; i < triangle_field.GetLength(); i++)
		{
			MeshTriangle<Def>&t = triangle_field[i];
			Obj::triangleNormal(t.v0->position,t.v1->position,t.v2->position,t.normal);
			M::Vec::add(t.v0->normal,t.normal);
			M::Vec::add(t.v1->normal,t.normal);
			M::Vec::add(t.v2->normal,t.normal);
			M::Vec::normalize0(t.normal);
		}
		
		for (index_t i = 0; i < quad_field.GetLength(); i++)
		{
			MeshQuad<Def>&q = quad_field[i];
			Obj::triangleNormal(q.v0->position,q.v1->position,q.v2->position,q.normal);
			_oAddTriangleNormal(q.v0->position,q.v2->position,q.v3->position,q.normal);
			_add(q.v0->normal,q.normal);
			_add(q.v1->normal,q.normal);
			_add(q.v2->normal,q.normal);
			_add(q.v3->normal,q.normal);
			_normalize0(q.normal);
		}
		for (index_t i = 0; i < vertex_field.GetLength(); i++)
			_normalize0(vertex_field[i].normal);
	}
	
template <class Def>
	MF_DECLARE	(void)		Mesh<Def>::generateVertexNormals()
	{
		for (index_t i = 0; i < vertex_field.GetLength(); i++)
			M::Vec::clear(vertex_field[i].normal);
		for (index_t i = 0; i < triangle_field.GetLength(); i++)
		{
			MeshTriangle<Def>&t = triangle_field[i];
			M::TVec3<typename Def::Type>	normal;
			Obj::triangleNormal(t.v0->position,t.v1->position,t.v2->position,normal);
			M::Vec::add(t.v0->normal,normal);
			M::Vec::add(t.v1->normal,normal);
			M::Vec::add(t.v2->normal,normal);
		}
		
		for (index_t i = 0; i < quad_field.GetLength(); i++)
		{
			MeshQuad<Def>&q = quad_field[i];
			M::TVec3<typename Def::Type>	normal;
			Obj::triangleNormal(q.v0->position,q.v1->position,q.v2->position,normal);
			_oAddTriangleNormal(q.v0->position,q.v2->position,q.v3->position,normal);
			M::Vec::add(q.v0->normal,normal);
			M::Vec::add(q.v1->normal,normal);
			M::Vec::add(q.v2->normal,normal);
			M::Vec::add(q.v3->normal,normal);
		}
		for (index_t i = 0; i < vertex_field.GetLength(); i++)
			M::Vec::normalize0(vertex_field[i].normal);
	}
		
template <class Def>
	MF_DECLARE	(void)		Mesh<Def>::generateFaceNormals()
	{
		for (index_t i = 0; i < triangle_field.GetLength(); i++)
		{
			MeshTriangle<Def>&t = triangle_field[i];
			Obj::triangleNormal(t.v0->position,t.v1->position,t.v2->position,t.normal);
			M::Vec::normalize0(t.normal);
		}
		
		for (index_t i = 0; i < quad_field.GetLength(); i++)
		{
			MeshQuad<Def>&q = quad_field[i];
			Obj::triangleNormal(q.v0->position,q.v1->position,q.v2->position,q.normal);
			_oAddTriangleNormal(q.v0->position,q.v2->position,q.v3->position,q.normal);
			M::Vec::normalize0(q.normal);
		}
	}

template <class Vertex, typename TX, typename TY, typename TZ>
	static inline void _AddVertex(Container::PreAllocVector<Vertex>&buffer, const TX&x, const TY&y, const TZ&z)
	{
		Vertex&v = buffer.Append();
		v.index = buffer.Count()-1;
		v.marked = false;
		M::Vec::def(v.position,x,y,z);
	}

template <class Def>
	MF_DECLARE	(void)						Mesh<Def>::buildSphere(count_t resolution/*=50*/)
	{
		typedef typename Def::Type Float;

		flushMap();

		PreAllocVector<Vertex>	vertexBuffer;
		PreAllocVector<UINT32> triangleBuffer, quadBuffer;

		count_t yres = std::max((count_t)1,resolution/2);
			
		_AddVertex(vertexBuffer,0,0,-1);
		for (index_t iy = 1; iy < yres; iy++)
		{
			Float ay = Float(iy) / Float(yres) * (Float)M_PI - (Float)M_PI / (Float)2;
			Float sny = vsin(ay);
			Float csy = vcos(ay);
				
			index_t offset = vertexBuffer.Count();

			for (index_t ix = 0; ix < resolution; ix++)
			{
				Float ax = Float(ix) / Float(resolution) * Float(M_PI*2);
				Float sn = vsin(ax);
				Float cs = vcos(ax);

				_AddVertex(vertexBuffer,cs * csy, sn * csy, sny);

				if (iy == 1)
					//triangleBuffer << 0 << (UINT32)(offset + ix) << (UINT32)(offset + (ix + 1)%resolution);
					triangleBuffer << 0 << (UINT32)(offset + (ix + 1)%resolution) << (UINT32)(offset + ix);
				else
				{
					index_t off0 = offset - resolution;
					//quadBuffer  << (UINT32)(off0 + (ix + 1)%resolution) << (UINT32)(off0 + ix) << (UINT32)(offset + ix) << (UINT32)(offset + (ix + 1)%resolution);
					quadBuffer  << (UINT32)(off0 + ix)<< (UINT32)(off0 + (ix + 1)%resolution)  << (UINT32)(offset + (ix + 1)%resolution)<< (UINT32)(offset + ix) ;
				}
			}

		}

		index_t end = vertexBuffer.Count();

		_AddVertex(vertexBuffer,0, 0, 1);

		index_t off0 = end - resolution;
		for (index_t ix = 0; ix < resolution; ix++)
		{
//				triangleBuffer << (UINT32)(off0 + (ix + 1)%resolution) << (UINT32)(off0 + ix) << (UINT32)end;
			triangleBuffer << (UINT32)(off0 + ix)<< (UINT32)(off0 + (ix + 1)%resolution)  << (UINT32)end;
		}

		vertexBuffer.copyToArray(vertex_field);
		quad_field.SetSize(quadBuffer.Count()/4);
		triangle_field.SetSize(triangleBuffer.Count()/3);
		edge_field.SetSize(0);
		for (index_t i = 0; i < quad_field.Count(); i++)
		{
			quad_field[i].v0 = vertex_field + quadBuffer[i*4];
			quad_field[i].v1 = vertex_field + quadBuffer[i*4+1];
			quad_field[i].v2 = vertex_field + quadBuffer[i*4+2];
			quad_field[i].v3 = vertex_field + quadBuffer[i*4+3];
		}
		for (index_t i = 0; i < triangle_field.Count(); i++)
		{
			triangle_field[i].v0 = vertex_field + triangleBuffer[i*3];
			triangle_field[i].v1 = vertex_field + triangleBuffer[i*3+1];
			triangle_field[i].v2 = vertex_field + triangleBuffer[i*3+2];
		}
			
		#ifdef _DEBUG
			verifyIntegrity();
		#endif
	}

template <class Def>
	MF_DECLARE	(void)						Mesh<Def>::buildCylinder(count_t resolution/*=50*/)
	{
		typedef typename Def::Type Float;

		flushMap();

		PreAllocVector<Vertex>	vertexBuffer;
		PreAllocVector<UINT32> quadBuffer;

		{
			index_t offset = vertexBuffer.Count();

			for (index_t ix = 0; ix < resolution; ix++)
			{
				Float ax = Float(ix) / Float(resolution) * Float(M_PI*2);
				Float sn = vsin(ax);
				Float cs = vcos(ax);

				_AddVertex(vertexBuffer,cs, sn, 0);
				_AddVertex(vertexBuffer,cs, sn, 1);

				{
					index_t off0 = offset - resolution;
					//quadBuffer  << (UINT32)(off0 + (ix + 1)%resolution) << (UINT32)(off0 + ix) << (UINT32)(offset + ix) << (UINT32)(offset + (ix + 1)%resolution);
					quadBuffer << (UINT32)(ix * 2 + 1) 
						<< (UINT32)(ix*2)
						<< (UINT32)(((ix + 1) % resolution) * 2)
						<< (UINT32)(((ix + 1)%resolution) * 2+1)
						;
				}
			}

		}

		index_t end = vertexBuffer.Count();

		vertexBuffer.copyToArray(vertex_field);
		quad_field.SetSize(quadBuffer.Count()/4);
		triangle_field.Clear();
		edge_field.SetSize(0);
		for (index_t i = 0; i < quad_field.Count(); i++)
		{
			quad_field[i].v0 = vertex_field + quadBuffer[i*4];
			quad_field[i].v1 = vertex_field + quadBuffer[i*4+1];
			quad_field[i].v2 = vertex_field + quadBuffer[i*4+2];
			quad_field[i].v3 = vertex_field + quadBuffer[i*4+3];
		}
		#ifdef _DEBUG
			verifyIntegrity();
		#endif
	}


	
template <class Def>
	MF_DECLARE	(void)		Mesh<Def>::buildCube()
	{
		typedef typename Def::Type Float;

		flushMap();

		vertex_field.SetSize(8);
		quad_field.SetSize(6);
		triangle_field.SetSize(0);
		edge_field.SetSize(12);

		M::Vec::def(vertex_field[0].position,-1,-1,+1);
		M::Vec::def(vertex_field[1].position,+1,-1,+1);
		M::Vec::def(vertex_field[2].position,+1,-1,-1);
		M::Vec::def(vertex_field[3].position,-1,-1,-1);
		M::Vec::def(vertex_field[4].position,-1,+1,+1);
		M::Vec::def(vertex_field[5].position,+1,+1,+1);
		M::Vec::def(vertex_field[6].position,+1,+1,-1);
		M::Vec::def(vertex_field[7].position,-1,+1,-1);

		for (index_t i = 0; i < vertex_field.GetLength(); i++)
		{
			vertex_field[i].index = i;
			vertex_field[i].marked = false;
		}


		quad_field[0].v0 = vertex_field + 0;
		quad_field[0].v1 = vertex_field + 1;
		quad_field[0].v2 = vertex_field + 5;
		quad_field[0].v3 = vertex_field + 4;

		quad_field[1].v0 = vertex_field + 1;
		quad_field[1].v1 = vertex_field + 2;
		quad_field[1].v2 = vertex_field + 6;
		quad_field[1].v3 = vertex_field + 5;

		quad_field[2].v0 = vertex_field + 2;
		quad_field[2].v1 = vertex_field + 3;
		quad_field[2].v2 = vertex_field + 7;
		quad_field[2].v3 = vertex_field + 6;

		quad_field[3].v0 = vertex_field + 3;
		quad_field[3].v1 = vertex_field + 0;
		quad_field[3].v2 = vertex_field + 4;
		quad_field[3].v3 = vertex_field + 7;

		quad_field[4].v0 = vertex_field + 3;
		quad_field[4].v1 = vertex_field + 2;
		quad_field[4].v2 = vertex_field + 1;
		quad_field[4].v3 = vertex_field + 0;

		quad_field[5].v0 = vertex_field + 4;
		quad_field[5].v1 = vertex_field + 5;
		quad_field[5].v2 = vertex_field + 6;
		quad_field[5].v3 = vertex_field + 7;

		for (index_t i = 0; i < quad_field.GetLength(); i++)
		{
			quad_field[i].index = i;
			quad_field[i].marked = false;
		}

		edge_field[0].v0 = vertex_field + 0;
		edge_field[0].v1 = vertex_field + 1;
		edge_field[0].n[0].Set(quad_field+0);
		edge_field[0].n[1].Set(quad_field+4);

		edge_field[1].v0 = vertex_field + 1;
		edge_field[1].v1 = vertex_field + 2;
		edge_field[1].n[0].Set(quad_field+1);
		edge_field[1].n[1].Set(quad_field+4);

		edge_field[2].v0 = vertex_field + 2;
		edge_field[2].v1 = vertex_field + 3;
		edge_field[2].n[0].Set(quad_field+2);
		edge_field[2].n[1].Set(quad_field+4);

		edge_field[3].v0 = vertex_field + 3;
		edge_field[3].v1 = vertex_field + 0;
		edge_field[3].n[0].Set(quad_field+3);
		edge_field[3].n[1].Set(quad_field+4);


		edge_field[4].v0 = vertex_field + 4;
		edge_field[4].v1 = vertex_field + 5;
		edge_field[4].n[0].Set(quad_field+0);
		edge_field[4].n[1].Set(quad_field+5);

		edge_field[5].v0 = vertex_field + 5;
		edge_field[5].v1 = vertex_field + 6;
		edge_field[5].n[0].Set(quad_field+1);
		edge_field[5].n[1].Set(quad_field+5);

		edge_field[6].v0 = vertex_field + 6;
		edge_field[6].v1 = vertex_field + 7;
		edge_field[6].n[0].Set(quad_field+2);
		edge_field[6].n[1].Set(quad_field+5);

		edge_field[7].v0 = vertex_field + 7;
		edge_field[7].v1 = vertex_field + 4;
		edge_field[7].n[0].Set(quad_field+3);
		edge_field[7].n[1].Set(quad_field+5);


		edge_field[8].v0 = vertex_field + 1;
		edge_field[8].v1 = vertex_field + 5;
		edge_field[8].n[0].Set(quad_field+0);
		edge_field[8].n[1].Set(quad_field+1);

		edge_field[9].v0 = vertex_field + 2;
		edge_field[9].v1 = vertex_field + 6;
		edge_field[9].n[0].Set(quad_field+1);
		edge_field[9].n[1].Set(quad_field+2);

		edge_field[10].v0 = vertex_field + 3;
		edge_field[10].v1 = vertex_field + 7;
		edge_field[10].n[0].Set(quad_field+2);
		edge_field[10].n[1].Set(quad_field+3);

		edge_field[11].v0 = vertex_field + 0;
		edge_field[11].v1 = vertex_field + 4;
		edge_field[11].n[0].Set(quad_field+3);
		edge_field[11].n[1].Set(quad_field+0);

		for (index_t i = 0; i < edge_field.GetLength(); i++)
		{
			edge_field[i].index = i;
			edge_field[i].marked = false;
		}


		verifyIntegrity();
	}

	
template <class Def> template <typename T>
	MF_DECLARE	(void)		Mesh<Def>::buildFromFunction(const T&min_x, const T&max_x, count_t xres, const T&min_z, const T&max_z, count_t zres, T (*function)(const T&x, const T&y))
	{
		typedef typename Def::Type Float;
		vertex_field.SetSize((xres+1)*(zres+1));
		quad_field.SetSize(xres*zres);
		triangle_field.free();
		edge_field.free();
			
		T	range_x = (Float)(max_x-min_x),
			range_z = (Float)(max_z-min_z);
		for (index_t x = 0; x <= xres; x++)
			for (index_t z = 0; z <= zres; z++)
			{
				T	fx = min_x+(T)x/(T)xres*range_x,
					fz = min_z+(T)z/(T)zres*range_z;
				M::Vec::def(vertex_field[x*(zres+1)+z].position,fx,function(fx,fz),fz);
			}
			
		for (index_t x = 0; x < xres; x++)
			for (index_t z = 0; z < zres; z++)
			{
				MeshQuad<Def>&q = quad_field[x*zres+z];
				q.v0 = vertex_field+(x*(zres+1)+z);
				q.v1 = vertex_field+(x*(zres+1)+z+1);
				q.v2 = vertex_field+((x+1)*(zres+1)+z+1);
				q.v3 = vertex_field+((x+1)*(zres+1)+z);
			}
		ASSERT__(valid());
	}
	
	
	
	

/*ObjDataVertex<Def>*	exportVertices();
		ObjDataEdge<Def>*		exportEdges();
		ObjDataFace<Def>*		exportFaces();*/


template <class Def>ObjMap<Def>::ObjMap(const M::Box<C>&dimension, unsigned Level):level(Level),keep_dimensions(true)
{
	dim = dimension;
	for (BYTE k = 0; k < 8; k++)
		child[k] = NULL;
}

template <class Def>ObjMap<Def>::ObjMap(unsigned Level, bool keep):level(Level),keep_dimensions(keep)
{
	for (BYTE k = 0; k < 8; k++)
		child[k] = NULL;
}

#ifndef __CUDACC__

template <class Def>ObjMap<Def>::ObjMap(const ObjMap<Def>&other):DynamicMesh<Def>(other),level(other.level),keep_dimensions(other.keep_dimensions)
{
	_c6(other.dim,dim);
		
		
	for (BYTE k = 0; k < 8; k++)
		if (other.child[k])
			child[k] = SignalNew(new ObjMap<Def>(*other.child[k]));
		else
			child[k] = NULL;
}
#endif

template <class Def>ObjMap<Def>::~ObjMap()
{
	for (BYTE k = 0; k < 8; k++)
		if (child[k])
			Discard(child[k]);
}

template <class Def>MF_DECLARE	(void) ObjMap<Def>::map(Mesh<Def>&object, BYTE tag)
{
	/*ObjBuffer<MeshVertex<Def> >vertexBuffer[8];
	ObjBuffer<MeshEdge<Def> >	edge_buffer[8];
	ObjBuffer<ObjFace<Def> >	face_buffer[8];*/

	if (empty() && !keep_dimensions)
	{
		M::Box<C>	new_dim;
		if ((tag & O_VERTICES) && object.vertex_field.GetLength())
		{
			new_dim.SetMinAndMax(object.vertex_field[0].position);
			for (index_t i = 1; i < object.vertex_field.GetLength(); i++)
				new_dim.Include(object.vertex_field[i].position);
		}
		elif ((tag & O_EDGES) && object.edge_field.GetLength())
		{
			new_dim.SetMinAndMax(object.edge_field[0].v0->position);
			for (BYTE k = 0; k < 2; k++)
				for (index_t i = 0; i < object.edge_field.GetLength(); i++)
					new_dim.Include(object.edge_field[i].vertex[k]->position);
		}
		elif ((tag & O_TRIANGLES) && object.triangle_field.GetLength())
		{
			new_dim.SetMinAndMax(object.triangle_field[0].v0->position);
			for (BYTE k = 0; k < 3; k++)
				for (index_t i = 0; i < object.triangle_field.GetLength(); i++)
					new_dim.Include(object.triangle_field[i].vertex[k]->position);
		}
		elif ((tag & O_QUADS) && object.quad_field.GetLength())
		{
			new_dim.SetMinAndMax(object.quad_field[0].v0->position);
			for (BYTE k = 0; k < 4; k++)
				for (index_t i = 0; i < object.quad_field.GetLength(); i++)
					new_dim.Include(object.quad_field[i].vertex[k]->position);
		}
		else
			return;
				
		/*_sub(dim+3,dim,new_dim);
		for (BYTE k = 0; k < 3; k++)
			if (!new_dim[k])
			{
				new_dim[k] = 1;
				ShowMessage("fixing dimension "+String(k));
			}
		_div(new_dim,10);
		_sub(dim,new_dim);
		_add(dim+3,new_dim);*/
			
		C range = M::Vec::length(new_dim.GetExtent());
		for (BYTE k = 0; k < 3; k++)
		{
			if (new_dim.axis[k].max == new_dim.axis[k].min)
			{
				new_dim.axis[k].max+=range/100;
				new_dim.axis[k].min-=range/50;
			}
			C dif = new_dim.axis[k].GetExtent();
			new_dim.axis[k].Expand(dif/100);
		}
		dim = new_dim;

		for (BYTE k = 0; k < 8; k++)
			if (child[k])
			{
				Discard(child[k]);
				child[k] = NULL;
			}
		/*
		
		_sub(dim+3,dim,new_dim);
		for (BYTE k = 0; k < 3; k++)
			if (!new_dim[k])
			{
				new_dim[k] = 1;
				ShowMessage("fixing dimension "+String(k));
			}
		_div(new_dim,10);
		_sub(dim,new_dim);
		_add(dim+3,new_dim);*/
			
	}
	count_t	nvc = vertex_field.GetLength() + object.vertex_field.GetLength(),
			nec = edge_field.GetLength() + object.edge_field.GetLength(),
			ntc = triangle_field.GetLength() + object.triangle_field.GetLength(),
			nqc = quad_field.GetLength() + object.quad_field.GetLength();

	if (object.vertex_field.GetLength())
	{
		Ctr::Array<MeshVertex<Def>*> list(nvc);
		MeshVertex<Def>**c = list.pointer();
		for (index_t i = 0; i < vertex_field.GetLength(); i++)
		{
			(*c++) = vertex_field[i];
		}
		for (index_t i = 0; i < object.vertex_field.GetLength(); i++)
		{
			(*c) = &object.vertex_field[i];
			(*c)->marked = false;
			c++;
		}
		ASSERT_CONCLUSION(list,c);
		vertex_field.adoptData(list);
	}
	if (object.edge_field.GetLength())
	{
		Ctr::Array<MeshEdge<Def>*>list(nec);
		MeshEdge<Def>**c = list.pointer();
		for (index_t i = 0; i < edge_field.GetLength(); i++)
			(*c++) = edge_field[i];
		for (index_t i = 0; i < object.edge_field.GetLength(); i++)
		{
			(*c) = &object.edge_field[i];
			(*c)->marked = false;
			c++;
		}
		ASSERT_CONCLUSION(list,c);
		edge_field.adoptData(list);
	}
	if (object.triangle_field.GetLength())
	{
		Ctr::Array<MeshTriangle<Def>*>list(ntc);
		MeshTriangle<Def>**c = list.pointer();
		for (index_t i = 0; i < triangle_field.GetLength(); i++)
			(*c++) = triangle_field[i];
		for (index_t i = 0; i < object.triangle_field.GetLength(); i++)
		{
			(*c) = &object.triangle_field[i];
			(*c)->marked = false;
			c++;
		}
		ASSERT_CONCLUSION(list,c);
		triangle_field.adoptData(list);
	}
	if (object.quad_field.GetLength())
	{
		Ctr::Array<MeshQuad<Def>*>list(nqc);
		MeshQuad<Def>**c = list.pointer();
		for (index_t i = 0; i < quad_field.GetLength(); i++)
			(*c++) = quad_field[i];
		for (index_t i = 0; i < object.quad_field.GetLength(); i++)
		{
			(*c) = &object.quad_field[i];
			(*c)->marked = false;
			c++;
		}
		ASSERT_CONCLUSION(list,c);
		quad_field.adoptData(list);
	}
	Ctr::Array<MeshVertex<Def>*>		vbuffer[8];
	Ctr::Array<MeshEdge<Def>*>			ebuffer[8];
	Ctr::Array<MeshTriangle<Def>*>		tbuffer[8];
	Ctr::Array<MeshQuad<Def>*>			qbuffer[8];
	for (BYTE k = 0; k < 8; k++)
	{
		vbuffer[k].SetSize(vertex_field.GetLength());
		ebuffer[k].SetSize(edge_field.GetLength());
		tbuffer[k].SetSize(triangle_field.GetLength());
		qbuffer[k].SetSize(quad_field.GetLength());
	}
		
	recursiveMap(vbuffer,ebuffer,tbuffer,qbuffer,tag);
/*
	for (BYTE k = 0; k < 8; k++)
	{
		if (!child[k])
		{
			BYTE p[3] = {k / 4,
							k % 4 / 2,
							k % 4 % 2};
			C d[6];
			for (BYTE k = 0; k < 3; k++)
			{
				d[k] = dim[k] + (dim[3+k] - dim[k])/2*p[k];
				d[k+3] = d[k] + (dim[3+k] - dim[k])/2;
			}
			child[k] = new ObjMap<C>(d, level-1);
		}
		child[k]->recursiveMap(this,tag);
	}
*/
}
	

template <class Def>MF_DECLARE	(void) ObjMap<Def>::recursiveMap(Ctr::ArrayData<MeshVertex<Def>*> vbuffer[8], Ctr::ArrayData<MeshEdge<Def>*> ebuffer[8], Ctr::ArrayData<MeshTriangle<Def>*> tbuffer[8],Ctr::ArrayData<MeshQuad<Def>*> qbuffer[8],BYTE tag)
{
	if (!this)
		ShowMessage("child does not exist");
//	ShowMessage(level);
	count_t items = quad_field.GetLength()+triangle_field.GetLength()+vertex_field.GetLength()+edge_field.GetLength();
	if (items < COMPLEXITY_CONST)
		level = 0;
	if (!level || !items)
		return;

	for (BYTE k = 0; k < 8; k++)
	{
		if (!child[k])
		{
			BYTE p[3] = {	(BYTE)(k / 4),
							(BYTE)(k % 4 / 2),
							(BYTE)(k % 4 % 2)};
			M::Box<C>	d;
			for (BYTE j = 0; j < 3; j++)
			{
				d.axis[j].min = dim.axis[j].min + (dim.axis[j].GetExtent())/2*p[j];
				d.axis[j].max = d.axis[j].min + (dim.axis[j].GetExtent())/2;
			}
			child[k] = SignalNew(new ObjMap<Def>(d, level-1));
		}
	}
		
	count_t	vbuffered[8] = {0,0,0,0, 0,0,0,0},
			ebuffered[8] = {0,0,0,0, 0,0,0,0},
			tbuffered[8] = {0,0,0,0, 0,0,0,0},
			qbuffered[8] = {0,0,0,0, 0,0,0,0};
		
	bool Set[8];
	M::TVec3<C>	half = dim.center();

	if (tag & O_VERTICES)
		for (index_t i = 0; i < vertex_field.GetLength(); i++)
			for (BYTE k = 0; k < 8; k++)
				if (child[k]->dim.Contains(vertex_field[i]->position))
					vbuffer[k][vbuffered[k]++] = vertex_field[i];

	if (tag & O_EDGES)
		for (index_t i = 0; i < edge_field.GetLength(); i++)
		{
			VecUnroll<8>::clear(Set);
			BYTE define[3];

			for (BYTE j = 0; j < 2; j++)
				if (dim.Contains(edge_field[i]->vertex[j]->position))
				{
					for (BYTE k = 0; k < 3; k++)
						define[k] = edge_field[i]->vertex[j]->position.v[k] > half.v[k];
//					ShowMessage(_toString(define));
					BYTE k = define[0] *4+ define[1] *2 + define[2];
					Set[k] = true;

				}
			M::TVec3<C>	dir;
			M::Vec::subtract(edge_field[i]->v1->position, edge_field[i]->v0->position, dir);
			for (BYTE k = 0; k < 3; k++)
				if (dir.v[k])
				{
					BYTE	x = (k+1)%3,
							y = (k+2)%3;
					M::TVec3<C>	corner[2] = {dim.min(),dim.max()};
					for (BYTE j = 0; j < 2; j++)
					{
						define[k] = j;
						C		alpha = (corner[j].v[k]-edge_field[i]->v0->position.v[k])/dir.v[k];
						if (alpha > 1 || alpha < 0)
							continue;
						C		vx = edge_field[i]->v0->position.v[x] + dir.v[x] *alpha,
								vy = edge_field[i]->v0->position.v[y] + dir.v[y] *alpha;
						if (!dim.axis[x].Contains(vx) || !dim.axis[y].Contains(vy))
							continue;
						define[x] = vx > half.v[x];
						define[y] = vy > half.v[y];
						Set[define[0]*4 + define[1]*2 + define[2]] = true;
					}
					C	alpha = (half.v[k] - edge_field[i]->v0->position.v[k]) / dir.v[k];
					if (alpha > 1 || alpha < 0)
						continue;
					C	vx = edge_field[i]->v0->position.v[x] + dir.v[x] *alpha,
						vy = edge_field[i]->v0->position.v[y] + dir.v[y] *alpha;
					if (!dim.axis[x].Contains(vx) || !dim.axis[y].Contains(vy))
						continue;
					define[k] = 0;
					define[x] = vx > half.v[x];
					define[y] = vy > half.v[y];
					Set[define[0] *4 + define[1]*2 + define[2]] = true;
					define[k] = 1;
					Set[define[0] *4 + define[1]*2 + define[2]] = true;
				}
			for (BYTE k = 0; k < 8; k++)
				if (Set[k])
					ebuffer[k][ebuffered[k]++] = edge_field[i];
		}

	if (tag & O_TRIANGLES)
		for (index_t i = 0; i < triangle_field.GetLength(); i++)
		{
			memset(Set,0,sizeof(Set));
			BYTE define[3];

			for (BYTE j = 0; j < 3; j++)
				if (!dim.Contains(triangle_field[i]->vertex[j]->position))
					continue;
				else
				{
					for (BYTE k = 0; k < 3; k++)
						define[k] = triangle_field[i]->vertex[j]->position.v[k] > half.v[k];
					Set[define[0] *4+ define[1] *2 + define[2]] = true;
				}
			for (BYTE h = 0; h < 3; h++)
			{
				M::TVec3<C>	dir;
				M::Vec::subtract(triangle_field[i]->vertex[(h+1)%3]->position,triangle_field[i]->vertex[h]->position,dir);
				for (BYTE k = 0; k < 3; k++)
					if (dir.v[k])
					{
						BYTE x = (k+1)%3,
								y = (k+2)%3;
						M::TVec3<C>	corner[2] = {dim.min(),dim.max()};

						for (BYTE j = 0; j < 2; j++)
						{
							define[k] = j;
							C	alpha = (corner[j].v[k]-triangle_field[i]->vertex[h]->position.v[k])/dir.v[k];
							if (alpha > 1 || alpha < 0)
								continue;
							C	vx = triangle_field[i]->vertex[h]->position.v[x] + dir.v[x] *alpha,
								vy = triangle_field[i]->vertex[h]->position.v[y] + dir.v[y] *alpha;
							if (!dim.axis[x].Contains(vx) || !dim.axis[y].Contains(vy))
								continue;
							define[x] = vx > half.v[x];
							define[y] = vy > half.v[y];
							Set[define[0]*4 + define[1]*2 + define[2]] = true;
						}
						C	alpha = (half.v[k] - triangle_field[i]->vertex[h]->position.v[k]) / dir.v[k];
						if (alpha > 1 || alpha < 0)
							continue;
						C	vx = triangle_field[i]->vertex[h]->position.v[x] + dir.v[x] *alpha,
							vy = triangle_field[i]->vertex[h]->position.v[y] + dir.v[y] *alpha;
						if (!dim.axis[x].Contains(vx) || !dim.axis[y].Contains(vy))
							continue;
						define[k] = 0;
						define[x] = vx > half.v[x];
						define[y] = vy > half.v[y];
						Set[define[0] *4 + define[1]*2 + define[2]] = true;
						define[k] = 1;
						Set[define[0] *4 + define[1]*2 + define[2]] = true;
					}
			}

			M::TVec3<C>	dir0,dir1,normal;
			M::Vec::subtract(triangle_field[i]->v1->position , triangle_field[i]->v0->position, dir0);
			M::Vec::subtract(triangle_field[i]->v2->position , triangle_field[i]->v0->position, dir1);
			M::Vec::cross(dir0,dir1,normal);
			C	dot = M::Vec::dot(triangle_field[i]->v0->position, normal);

			for (BYTE k = 0; k < 3; k++)
				for (BYTE fx = 0; fx < 3; fx++)
					for (BYTE fy = 0; fy < 3; fy++)
					{
						M::TVec3<C> a,d={0,0,0};
						a = dim.min();

						BYTE	x = (k+1)%3,
								y = (k+2)%3;
						a.v[x] += (dim.axis[x].GetExtent())/2 * fx;
						a.v[y] += (dim.axis[y].GetExtent())/2 * fy;
						d.v[k] =	dim.axis[k].GetExtent(); //so now we got the base point and direction;
						M::TVec3<C>	n0,n1,cross_point,dif;
						C	div = M::Vec::dot(d, normal);
						if (!div)
							continue;
						C	alpha = (dot - M::Vec::dot(a, normal)) / div;
						if (alpha < 0 || alpha > 1)
							continue;
						M::Vec::cross(normal, dir0, n0);
						M::Vec::cross(normal, dir1, n1);
						M::Vec::mad(a,d,alpha,cross_point);
						M::Vec::subtract(cross_point, triangle_field[i]->v0->position, dif);
						C	beta = M::Vec::dot(n0, dif) / M::Vec::dot(n0, dir1),
							gamma= M::Vec::dot(n1, dif) / M::Vec::dot(n1, dir0);
						if (beta < 0 || gamma < 0 || beta+gamma > 1)
							continue;
						define[k] = cross_point.v[k] > half.v[k];
						for (BYTE ix = 0; ix < 2; ix++)
							for (BYTE iy = 0; iy < 2; iy++)
							{
								define[x] = fx -1+ix;
								define[y] = fy -1+iy;
								if (define[x] < 2 && define[y] < 2)
									Set[define[0] *4 + define[1] *2 + define[2]] = true;
							}
					}
			for (BYTE k = 0; k < 8; k++)
				if (Set[k])
					tbuffer[k][tbuffered[k]++] = triangle_field[i];

		}
			
			
	if (tag & O_QUADS)
		for (index_t i = 0; i < quad_field.GetLength(); i++)
		{
			memset(Set,0,sizeof(Set));
			BYTE define[3];

			for (BYTE j = 0; j < 4; j++)
				if (!dim.Contains(quad_field[i]->vertex[j]->position))
					continue;
				else
				{
					for (BYTE k = 0; k < 3; k++)
						define[k] = quad_field[i]->vertex[j]->position.v[k] > half.v[k];
					Set[define[0] *4+ define[1] *2 + define[2]] = true;
				}
			for (BYTE h = 0; h < 4; h++)
			{
				M::TVec3<C>	dir;
				M::Vec::subtract(quad_field[i]->vertex[(h+1)%3]->position,quad_field[i]->vertex[h]->position,dir);
				for (BYTE k = 0; k < 3; k++)
					if (dir.v[k])
					{
						BYTE	x = (k+1)%3,
								y = (k+2)%3;
						M::TVec3<C>	corner[2] = {dim.min(),dim.max()};

						for (BYTE j = 0; j < 2; j++)
						{
							define[k] = j;
							C	alpha = (corner[j].v[k]-quad_field[i]->vertex[h]->position.v[k])/dir.v[k];
							if (alpha > 1 || alpha < 0)
								continue;
							C	vx = quad_field[i]->vertex[h]->position.v[x] + dir.v[x] *alpha,
								vy = quad_field[i]->vertex[h]->position.v[y] + dir.v[y] *alpha;
							if (!dim.axis[x].Contains(vx) || !dim.axis[y].Contains(vy))
								continue;
							define[x] = vx > half.v[x];
							define[y] = vy > half.v[y];
							Set[define[0]*4 + define[1]*2 + define[2]] = true;
						}
						C	alpha = (half.v[k] - quad_field[i]->vertex[h]->position.v[k]) / dir.v[k];
						if (alpha > 1 || alpha < 0)
							continue;
						C	vx = quad_field[i]->vertex[h]->position.v[x] + dir.v[x] *alpha,
							vy = quad_field[i]->vertex[h]->position.v[y] + dir.v[y] *alpha;
						if (!dim.axis[x].Contains(vx) || !dim.axis[y].Contains(vy))
							continue;
						define[k] = 0;
						define[x] = vx > half.v[x];
						define[y] = vy > half.v[y];
						Set[define[0] *4 + define[1]*2 + define[2]] = true;
						define[k] = 1;
						Set[define[0] *4 + define[1]*2 + define[2]] = true;
					}
			}

			{
				M::TVec3<C>	dir0,dir1,normal;

				M::Vec::subtract(quad_field[i]->v1->position , quad_field[i]->v0->position, dir0);
				M::Vec::subtract(quad_field[i]->v2->position , quad_field[i]->v0->position, dir1);
				M::Vec::cross(dir0,dir1,normal);
				C	dot = M::Vec::dot(quad_field[i]->v0->position, normal);

				for (BYTE k = 0; k < 3; k++)
					for (BYTE fx = 0; fx < 3; fx++)
						for (BYTE fy = 0; fy < 3; fy++)
						{
							M::TVec3<C> a,d={0,0,0};
							a = dim.min();
							BYTE	x = (k+1)%3,
									y = (k+2)%3;
							a.v[x] += (dim.axis[x].GetExtent())/2 * fx;
							a.v[y] += (dim.axis[y].GetExtent())/2 * fy;
							d.v[k] =	dim.axis[k].GetExtent(); //so now we got the base point and direction;
							M::TVec3<C>	n0,n1,cross_point,dif;
							C	div = M::Vec::dot(d, normal);
							if (!div)
								continue;
							C	alpha = (dot - M::Vec::dot(a, normal)) / div;
							if (alpha < 0 || alpha > 1)
								continue;
							M::Vec::cross(normal, dir0, n0);
							M::Vec::cross(normal, dir1, n1);
							M::Vec::multAdd(a,d,alpha,cross_point);
							M::Vec::subtract(cross_point, quad_field[i]->v0->position, dif);
							C	beta = M::Vec::dot(n0, dif) / M::Vec::dot(n0, dir1),
								gamma= M::Vec::dot(n1, dif) / M::Vec::dot(n1, dir0);
							if (beta < 0 || gamma < 0 || beta+gamma > 1)
								continue;
							define[k] = cross_point.v[k] > half.v[k];
							for (BYTE ix = 0; ix < 2; ix++)
								for (BYTE iy = 0; iy < 2; iy++)
								{
									define[x] = fx -1+ix;
									define[y] = fy -1+iy;
									if (define[x] < 2 && define[y] < 2)
										Set[define[0] *4 + define[1] *2 + define[2]] = true;
								}
						}
							
				M::Vec::subtract(quad_field[i]->v2->position , quad_field[i]->v0->position, dir0);
				M::Vec::subtract(quad_field[i]->v3->position , quad_field[i]->v0->position, dir1);
				M::Vec::cross(dir0,dir1,normal);
				dot = M::Vec::dot(quad_field[i]->v0->position, normal);

				for (BYTE k = 0; k < 3; k++)
					for (BYTE fx = 0; fx < 3; fx++)
						for (BYTE fy = 0; fy < 3; fy++)
						{
							M::TVec3<C> a,d={0,0,0};
							a = dim.min();
							BYTE	x = (k+1)%3,
									y = (k+2)%3;
							a.v[x] += (dim.axis[x].GetExtent())/2 * fx;
							a.v[y] += (dim.axis[y].GetExtent())/2 * fy;
							d.v[k] =	dim.axis[k].GetExtent(); //so now we got the base point and direction;
							M::TVec3<C>	n0,n1,cross_point,dif;
							C	div = M::Vec::dot(d, normal);
							if (!div)
								continue;
							C	alpha = (dot - M::Vec::dot(a, normal)) / div;
							if (alpha < 0 || alpha > 1)
								continue;
							M::Vec::cross(normal, dir0, n0);
							M::Vec::cross(normal, dir1, n1);
							M::Vec::multAdd(a,d,alpha,cross_point);
							M::Vec::subtract(cross_point, quad_field[i]->v0->position, dif);
							C	beta = M::Vec::dot(n0, dif) / M::Vec::dot(n0, dir1),
								gamma= M::Vec::dot(n1, dif) / M::Vec::dot(n1, dir0);
							if (beta < 0 || gamma < 0 || beta+gamma > 1)
								continue;
							define[k] = cross_point.v[k] > half.v[k];
							for (BYTE ix = 0; ix < 2; ix++)
								for (BYTE iy = 0; iy < 2; iy++)
								{
									define[x] = fx -1+ix;
									define[y] = fy -1+iy;
									if (define[x] < 2 && define[y] < 2)
										Set[define[0] *4 + define[1] *2 + define[2]] = true;
								}
						}							
			}
			for (BYTE k = 0; k < 8; k++)
				if (Set[k])
					qbuffer[k][qbuffered[k]++] = quad_field[i];

		}
			
			
	for (BYTE k = 0; k < 8; k++)
	{
		child[k]->vertex_field.appendCopy(vbuffer[k],vbuffered[k]);
		child[k]->edge_field.appendCopy(ebuffer[k],ebuffered[k]);
		child[k]->triangle_field.appendCopy(tbuffer[k],tbuffered[k]);
		child[k]->quad_field.appendCopy(qbuffer[k],qbuffered[k]);
	}
	for (BYTE k = 0; k < 8; k++)
	{
		if (!child[k])
			FATAL__("bad child");
		child[k]->recursiveMap(vbuffer,ebuffer,tbuffer,qbuffer,tag);
	}
}

template <class Def>MF_DECLARE	(void) ObjMap<Def>::recursiveMap(ObjMap<Def>*source,BYTE tag)
{
	static Ctr::Array<MeshVertex<Def>*>		temp_vertex_field;
	static Ctr::Array<MeshEdge<Def>*>		temp_edge_field;
	static Ctr::Array<MeshQuad<Def>*>		temp_quad_field;
	static Ctr::Array<MeshTriangle<Def>*>		temp_triangle_field;
		
	if (temp_vertex_field.GetLength() < source->vertex_field.GetLength())
		temp_vertex_field.SetSize(source->vertex_field.GetLength());
	if (temp_edge_field.GetLength() < source->edge_field.GetLength())
		temp_edge_field.SetSize(source->edge_field.GetLength());
	if (temp_triangle_field.GetLength() < source->triangle_field.GetLength())
		temp_triangle_field.SetSize(source->triangle_field.GetLength());
	if (temp_quad_field.GetLength() < source->quad_field.GetLength())
		temp_quad_field.SetSize(source->quad_field.GetLength());
		
	count_t	temp_vertex_count = 0,
			temp_edge_count = 0,
			temp_quad_count = 0,
			temp_triangle_count = 0;

	if (tag & O_VERTICES)
		for (index_t i = 0; i < source->vertex_field.GetLength(); i++)
			if (_allGreater(source->vertex_field[i]->position,dim) && _allLess(source->vertex_field[i]->position,dim+3))
				temp_vertex_field[temp_vertex_count++] = source->vertex_field[i];
	if (tag & O_EDGES)
		for (index_t i = 0; i < source->edge_field.GetLength(); i++)
		{
			bool added = false;
			for (BYTE j = 0; j < 2; j++)	//vertices
				if (_allGreater(source->edge_field[i]->vertex[j]->position,dim) && _allLess(source->edge_field[i]->vertex[j]->position,dim+3))
				{
					temp_edge_field[temp_edge_count++] = source->edge_field[i];
					added = true;
				}
			C	dir[3];
			_subtract(source->edge_field[i]->v1->position, source->edge_field[i]->v0->position, dir);
			for (BYTE k = 0; k < 3 && !added; k++) //edge
				if (dir[k])
					for (BYTE j = 0; j < 2 && !added; j++)
					{
						C	alpha = (dim[j*3+k]-source->edge_field[i]->v0->position[k])/dir[k];
						if (alpha > 1 || alpha < 0)
							continue;
						BYTE x = (k+1)%3,
								y = (k+2)%3;
						C	vx = source->edge_field[i]->v0->position[x] + dir[x] *alpha,
									vy = source->edge_field[i]->v0->position[y] + dir[y] *alpha;
						if (vx >= dim[x] && vx <= dim[3+x] && vy >= dim[y] && vy <= dim[3+y])
						{
							added = true;
							temp_edge_field[temp_edge_count++] = source->edge_field[i];
						}
					}
		}
	if (tag & O_TRIANGLES)
		for (index_t i = 0; i < source->triangle_field.GetLength(); i++)
		{
			bool added = false;
			for (BYTE j = 0; j < 3; j++) //vertices
				if (_allGreater(source->triangle_field[i]->vertex[j]->position,dim) && _allLess(source->triangle_field[i]->vertex[j]->position,dim+3))
				{
					temp_triangle_field[temp_triangle_count++] = source->triangle_field[i];
					added = true;
				}
			for (BYTE k = 0; k < 3 && !added; k++) //edges
				for (BYTE e = 0; e < 3 && !added; e++)
				{
					C	dir[3];
					_subtract(source->triangle_field[i]->vertex[(e+1)%3]->position, source->triangle_field[i]->vertex[e]->position, dir);

					if (dir[k])
						for (BYTE j = 0; j < 2 && !added; j++)
						{
							C alpha = (dim[j*3+k]-source->triangle_field[i]->vertex[e]->position[k])/dir[k];
							if (alpha > 1 || alpha < 0)
								continue;
							BYTE	x = (k+1)%3,
									y = (k+2)%3;
							C		vx = source->triangle_field[i]->vertex[e]->position[x] + dir[x] *alpha,
									vy = source->triangle_field[i]->vertex[e]->position[y] + dir[y] *alpha;
							if (vx >= dim[x] && vx <= dim[3+x] && vy >= dim[y] && vy <= dim[3+y])
							{
								added = true;
								temp_triangle_field[temp_triangle_count++] = source->triangle_field[i];
							}
						}
				}

			C	dir0[3],dir1[3],normal[3];
			_subtract(source->triangle_field[i]->v1->position , source->triangle_field[i]->v0->position, dir0);
			_subtract(source->triangle_field[i]->v2->position , source->triangle_field[i]->v0->position, dir1);
			_cross(dir0,dir1,normal);
			C	dot = _dot(source->triangle_field[i]->v0->position, normal);

			for (BYTE k = 0; k < 3 && !added; k++)
				for (BYTE fx = 0; fx < 2; fx++)
					for (BYTE fy = 0; fy < 2; fy++)
					{
						C	a[3],d[3]={0,0,0};
						_c3(dim,a);
						BYTE x = (k+1)%3,
								y = (k+2)%3;
						a[x] += (dim[3+x] - dim[x]) * fx;
						a[y] += (dim[3+y] - dim[y]) * fy;
						d[k] =	dim[3+k] - dim[k]; //so now we got the base point and direction;
						C	div = _dot(d, normal);
						if (!div)
							continue;
						C	alpha = (dot - _dot(a, normal)) / div;
						if (alpha < 0 || alpha > 1)
							continue;
						C	n0[3],n1[3],cross_point[3],dif[3];
						_cross(normal, dir0, n0);
						_cross(normal, dir1, n1);
						_mad(a,d,alpha,cross_point);
						_subtract(cross_point, source->triangle_field[i]->v0->position, dif);
						C	beta = _dot(n0, dif) / _dot(n0, dir1),
									gamma= _dot(n1, dif) / _dot(n1, dir0);
						if (beta < 0 || gamma < 0 || beta+gamma > 1)
							continue;
						added = true;
						temp_triangle_field[temp_triangle_count++] = source->triangle_field[i];
					}
		}
			
	if (tag & O_QUADS)
		for (index_t i = 0; i < source->quad_field.GetLength(); i++)
		{
			bool added = false;
			for (BYTE j = 0; j < 3; j++) //vertices
				if (_allGreater(source->quad_field[i]->vertex[j]->position,dim) && _allLess(source->quad_field[i]->vertex[j]->position,dim+3))
				{
					temp_quad_field[temp_quad_count++] = source->quad_field[i];
					added = true;
				}
			for (BYTE k = 0; k < 3 && !added; k++) //edges
				for (BYTE e = 0; e < 3 && !added; e++)
				{
					C	dir[3];
					_subtract(source->quad_field[i]->vertex[(e+1)%3]->position, source->quad_field[i]->vertex[e]->position, dir);

					if (dir[k])
						for (BYTE j = 0; j < 2 && !added; j++)
						{
							C alpha = (dim[j*3+k]-source->quad_field[i]->vertex[e]->position[k])/dir[k];
							if (alpha > 1 || alpha < 0)
								continue;
							BYTE	x = (k+1)%3,
									y = (k+2)%3;
							C		vx = source->quad_field[i]->vertex[e]->position[x] + dir[x] *alpha,
									vy = source->quad_field[i]->vertex[e]->position[y] + dir[y] *alpha;
							if (vx >= dim[x] && vx <= dim[3+x] && vy >= dim[y] && vy <= dim[3+y])
							{
								added = true;
								temp_quad_field[temp_quad_count++] = source->quad_field[i];
							}
						}
				}

			C	dir0[3],dir1[3],normal[3];
			_subtract(source->quad_field[i]->v1->position , source->quad_field[i]->v0->position, dir0);
			_subtract(source->quad_field[i]->v2->position , source->quad_field[i]->v0->position, dir1);
			_cross(dir0,dir1,normal);
			C	dot = _dot(source->quad_field[i]->v0->position, normal);

			for (BYTE k = 0; k < 3 && !added; k++)
				for (BYTE fx = 0; fx < 2; fx++)
					for (BYTE fy = 0; fy < 2; fy++)
					{
						C	a[3],d[3]={0,0,0};
						_c3(dim,a);
						BYTE x = (k+1)%3,
								y = (k+2)%3;
						a[x] += (dim[3+x] - dim[x]) * fx;
						a[y] += (dim[3+y] - dim[y]) * fy;
						d[k] =	dim[3+k] - dim[k]; //so now we got the base point and direction;
						C	div = _dot(d, normal);
						if (!div)
							continue;
						C	alpha = (dot - _dot(a, normal)) / div;
						if (alpha < 0 || alpha > 1)
							continue;
						C	n0[3],n1[3],cross_point[3],dif[3];
						_cross(normal, dir0, n0);
						_cross(normal, dir1, n1);
						_mad(a,d,alpha,cross_point);
						_subtract(cross_point, source->quad_field[i]->v0->position, dif);
						C	beta = _dot(n0, dif) / _dot(n0, dir1),
									gamma= _dot(n1, dif) / _dot(n1, dir0);
						if (beta < 0 || gamma < 0 || beta+gamma > 1)
							continue;
						added = true;
						temp_quad_field[temp_quad_count++] = source->quad_field[i];
					}
						
			_subtract(source->quad_field[i]->v2->position , source->quad_field[i]->v0->position, dir0);
			_subtract(source->quad_field[i]->v3->position , source->quad_field[i]->v0->position, dir1);
			_cross(dir0,dir1,normal);
			dot = _dot(source->quad_field[i]->v0->position, normal);

			for (BYTE k = 0; k < 3 && !added; k++)
				for (BYTE fx = 0; fx < 2; fx++)
					for (BYTE fy = 0; fy < 2; fy++)
					{
						C	a[3],d[3]={0,0,0};
						_c3(dim,a);
						BYTE x = (k+1)%3,
								y = (k+2)%3;
						a[x] += (dim[3+x] - dim[x]) * fx;
						a[y] += (dim[3+y] - dim[y]) * fy;
						d[k] =	dim[3+k] - dim[k]; //so now we got the base point and direction;
						C	div = _dot(d, normal);
						if (!div)
							continue;
						C	alpha = (dot - _dot(a, normal)) / div;
						if (alpha < 0 || alpha > 1)
							continue;
						C	n0[3],n1[3],cross_point[3],dif[3];
						_cross(normal, dir0, n0);
						_cross(normal, dir1, n1);
						_mad(a,d,alpha,cross_point);
						_subtract(cross_point, source->quad_field[i]->v0->position, dif);
						C	beta = _dot(n0, dif) / _dot(n0, dir1),
									gamma= _dot(n1, dif) / _dot(n1, dir0);
						if (beta < 0 || gamma < 0 || beta+gamma > 1)
							continue;
						added = true;
						temp_quad_field[temp_quad_count++] = source->quad_field[i];
					}
		}			
		
	vertex_field.append(temp_vertex_field,temp_vertex_count);
	edge_field.append(temp_edge_field,temp_edge_count);
	triangle_field.append(temp_triangle_field,temp_triangle_count);
	quad_field.append(temp_quad_field,temp_quad_count);

	if (level && !empty())
		for (BYTE k = 0; k < 8; k++)
		{
			if (!child[k])
			{
				BYTE p[3] = {(BYTE)(k / 4),
							(BYTE)(k % 4 / 2),
							(BYTE)(k % 4 % 2)};
				C	d[6];
				for (BYTE j = 0; j < 3; j++)
				{
					d[j] = dim[j] + (dim[3+j] - dim[j])/2*p[j];
					d[j+3] = d[j] + (dim[3+j] - dim[j])/2;
				}
				child[k] = SignalNew(new ObjMap<Def>(d, level-1));
			}
			child[k]->recursiveMap(this,tag);
		}
}

template <class Def>MF_DECLARE	(bool) ObjMap<Def>::empty() const
{
	return !vertex_field.GetLength() && !edge_field.GetLength() && !triangle_field.GetLength() && !quad_field.GetLength();
}


template <class Def>MF_DECLARE	(void) ObjMap<Def>::drop(index_t index, BYTE tag)
{
	count_t remaining;
	bool changed = false;
	if (tag & O_VERTICES)
	{
		remaining	= vertex_field.GetLength();
		for (index_t i = 0; i < vertex_field.GetLength(); i++)
			if (vertex_field[i]->index == index)
				remaining--;
		if (remaining != vertex_field.GetLength())
		{
			Ctr::Array<MeshVertex<Def>*>list(remaining);
			index_t at = 0;
			if (remaining)
				for (index_t i = 0; i < vertex_field.GetLength(); i++)
					if (vertex_field[i]->index != index)
						list[at++] = vertex_field[i];
			vertex_field.adoptData(list);
			changed = true;
		}
	}
	if (tag & O_EDGES)
	{
		remaining = edge_field.GetLength();
		for (index_t i = 0; i < edge_field.GetLength(); i++)
			if (edge_field[i]->index == index)
				remaining--;
		if (remaining != edge_field.GetLength())
		{
			Ctr::Array<MeshEdge<Def>*>list(remaining);
			index_t at = 0;
			if (remaining)
				for (index_t i = 0; i < edge_field.GetLength(); i++)
					if (edge_field[i]->index != index)
						list[at++] = edge_field[i];
			edge_field.adoptData(list);
			changed = true;
		}
	}
	if (tag & O_TRIANGLES)
	{
		remaining = triangle_field.GetLength();
		for (index_t i = 0; i < triangle_field.GetLength(); i++)
			if (triangle_field[i]->index == index)
				remaining--;
		if (remaining != triangle_field.GetLength())
		{
			Ctr::Array<MeshTriangle<Def>*>list(remaining);
			index_t at = 0;
			if (remaining)
				for (index_t i = 0; i < triangle_field.GetLength(); i++)
					if (triangle_field[i]->index != index)
						list[at++] = triangle_field[i];
			triangle_field.adoptData(list);
			changed = true;
		}
	}
	if (tag & O_QUADS)
	{
		remaining = quad_field.GetLength();
		for (index_t i = 0; i < quad_field.GetLength(); i++)
			if (quad_field[i]->index == index)
				remaining--;
		if (remaining != quad_field.GetLength())
		{
			Ctr::Array<MeshQuad<Def>*>list(remaining);
			index_t at = 0;
			if (remaining)
				for (index_t i = 0; i < quad_field.GetLength(); i++)
					if (quad_field[i]->index != index)
						list[at++] = quad_field[i];
			quad_field.adoptData(list);
			changed = true;
		}
	}		
	if (!changed)
		return;
	if (empty())
	{
		for (BYTE k = 0; k < 8; k++)
			if (child[k])
			{
				Discard(child[k]);
				child[k] = NULL;
			}
	}
	else
		if (level)
			for (BYTE k = 0; k < 8; k++)
				child[k]->drop(index,tag);
}

template <class Def>
	MF_DECLARE	(void)			ObjMap<Def>::reset(unsigned depth, bool keep /*=false*/)
	{
		clear();
		level = depth;
		keep_dimensions = keep;
	}

template <class Def>MF_DECLARE	(void) ObjMap<Def>::clear()
{
	for (BYTE k = 0; k < 8; k++)
		if (child[k])
		{
			Discard(child[k]);
			child[k] = NULL;
		}
	vertex_field.free();
	edge_field.free();
	quad_field.free();
	triangle_field.free();
}

template <class Def>MF_DECLARE	(count_t) ObjMap<Def>::recursiveLookupEdge(const C*p0, const C*p1)
{
	count_t cnt = Count();
	if (!cnt)
		return 0;
	if (!level)
	{
		sector_map << this;
		return cnt;
	}
	const C*edge[2] = {p0,p1};
	C		half[3];
	_center(dim,dim+3,half);
	bool Set[8] = {false,false,false,false,false,false,false,false};
	BYTE define[3];

	for (BYTE j = 0; j < 2; j++)
			if (_allGreater(edge[j],dim) && _allLess(edge[j],dim+3))
			{
				for (BYTE k = 0; k < 3; k++)
					define[k] = edge[j][k] > half[k];
				BYTE k = define[0] *4+ define[1] *2 + define[2];
				Set[k] = true;
			}
	C	dir[3];
	_subtract(p1, p0, dir);
	for (BYTE k = 0; k < 3; k++)
		if (dir[k])
		{
			BYTE x = (k+1)%3,
					y = (k+2)%3;
			for (BYTE j = 0; j < 2; j++)
			{
					define[k] = j;
					C alpha = (dim[j*3+k]-p0[k])/dir[k];
					if (alpha > 1 || alpha < 0)
					continue;
					C		vx = p0[x] + dir[x] *alpha,
						vy = p0[y] + dir[y] *alpha;
					if (vx < dim[x] || vx > dim[3+x] || vy < dim[y] || vy > dim[3+y])
						continue;
					define[x] = vx > half[x];
					define[y] = vy > half[y];
					Set[define[0]*4 + define[1]*2 + define[2]] = true;
			}
			C	alpha = (half[k] - p0[k]) / dir[k];
			if (alpha > 1 || alpha < 0)
				continue;
			C		vx = p0[x] + dir[x] *alpha,
					vy = p0[y] + dir[y] *alpha;
			if (vx < dim[x] || vx > dim[3+x] || vy < dim[y] || vy > dim[3+y])
				continue;
			define[k] = 0;
			define[x] = vx > half[x];
			define[y] = vy > half[y];
			Set[define[0] *4 + define[1]*2 + define[2]] = true;
			define[k] = 1;
			Set[define[0] *4 + define[1]*2 + define[2]] = true;
		}
	index_t at = sector_map.Count();
	for (BYTE k = 0; k < 8; k++)
		if (Set[k])
			if (child[k]->recursiveLookupEdge(p0,p1) == cnt)
			{
				sector_map.Truncate(at);
				sector_map<< this;
				return cnt;
			}
	return 0;
}
	
template <class Def>MF_DECLARE	(count_t) ObjMap<Def>::recursiveLookupSphere(const C*p, const C&r)
{
	count_t cnt = Count();
	if (!cnt)
		return 0;
	if (!level)
	{
		sector_map << this;
		return cnt;
	}
		
	C		half[3];
	_center(dim,dim+3,half);
	bool Set[8] = {false,false,false,false,false,false,false,false};
	//BYTE define[3];
	static const BYTE fc[3] = {4,2,1};
		
	for (BYTE k = 0; k < 3; k++)
	{
		BYTE	other0 = (k+1)%3,
				other1 = (k+2)%3;
		//ShowMessage("p is "+_toString(p)+" k is "+String(k)+" r is "+String(r)+" dim[k] is "+String(dim[k]));
			
		if (p[k] >= dim[k] && p[k] <= dim[k+3])
		{
			if (sqr(p[other0]-dim[other0])+sqr(p[other1]-dim[other1])<= r*r)
			{
				BYTE	id = 0;
				if (p[k] >= half[k])
					id += fc[k];
				Set[id] = true;
			}
			if (sqr(p[other0]-half[other0])+sqr(p[other1]-dim[other1])<= r*r)
			{
				BYTE	id = 0;
				if (p[k] >= half[k])
					id += fc[k];
				Set[id] = true;
				Set[id+fc[other0]] = true;
			}
			if (sqr(p[other0]-dim[other0+3])+sqr(p[other1]-dim[other1])<= r*r)
			{
				BYTE	id = 0;
				if (p[k] >= half[k])
					id += fc[k];
				Set[id+fc[other0]] = true;
			}
				
			if (sqr(p[other0]-dim[other0])+sqr(p[other1]-dim[other1+3])<= r*r)
			{
				BYTE	id = 0;
				if (p[k] >= half[k])
					id += fc[k];
				Set[id+fc[other1]] = true;
			}
			if (sqr(p[other0]-half[other0])+sqr(p[other1]-dim[other1+3])<= r*r)
			{
				BYTE	id = 0;
				if (p[k] >= half[k])
					id += fc[k];
				Set[id+fc[other1]] = true;
				Set[id+fc[other0]+fc[other1]] = true;
			}
			if (sqr(p[other0]-dim[other0+3])+sqr(p[other1]-dim[other1+3])<= r*r)
			{
				BYTE	id = 0;
				if (p[k] >= half[k])
					id += fc[k];
				Set[id+fc[other0]+fc[other1]] = true;
			}
				
			if (sqr(p[other0]-dim[other0])+sqr(p[other1]-half[other1])<= r*r)
			{
				BYTE	id = 0;
				if (p[k] >= half[k])
					id += fc[k];
				Set[id] = true;
				Set[id+fc[other1]] = true;
			}
			if (sqr(p[other0]-half[other0])+sqr(p[other1]-half[other1])<= r*r)
			{
				BYTE	id = 0;
				if (p[k] >= half[k])
					id += fc[k];
				Set[id] = true;
				Set[id+fc[other0]] = true;
				Set[id+fc[other1]] = true;
				Set[id+fc[other0]+fc[other1]] = true;
			}
			if (sqr(p[other0]-dim[other0+3])+sqr(p[other1]-half[other1])<= r*r)
			{
				BYTE	id = 0;
				if (p[k] >= half[k])
					id += fc[k];
				Set[id+fc[other0]] = true;
				Set[id+fc[other0]+fc[other1]] = true;
			}
		}
			
			
		if (p[k]+r > dim[k] && p[k]-r < dim[k])
		{
			//ShowMessage("in one");
			if (p[other0] >= dim[other0] && p[other0] <= dim[other0+3]
				&& p[other1] >= dim[other1] && p[other1] <= dim[other1+3])
			{
				bool	u0 = p[other0] > half[other0],
						u1 = p[other1] > half[other1];
				BYTE	id = 0;
				if (u0)
					id += fc[other0];
				if (u1)
					id += fc[other1];
				Set[id] = true;
			}
		}
		
		if (p[k]+r > half[k] && p[k]-r < half[k])
		{
			//ShowMessage("in half");
			if (p[other0] >= dim[other0] && p[other0] <= dim[other0+3]
				&& p[other1] >= dim[other1] && p[other1] <= dim[other1+3])
			{
				bool	u0 = p[other0] > half[other0],
						u1 = p[other1] > half[other1];
				BYTE	id = 0;
				if (u0)
					id += fc[other0];
				if (u1)
					id += fc[other1];
				Set[id] = true;
				Set[id+fc[k]] = true;
			}
		}
			
		if (p[k]+r > dim[k+3] && p[k]-r < dim[k+3])
		{
			//ShowMessage("in two");
			
			if (p[other0] >= dim[other0] && p[other0] <= dim[other0+3]
				&& p[other1] >= dim[other1] && p[other1] <= dim[other1+3])
			{
				bool	u0 = p[other0] > half[other0],
						u1 = p[other1] > half[other1];
				BYTE	id = 0;
				if (u0)
					id += fc[other0];
				if (u1)
					id += fc[other1];
				Set[id+fc[k]] = true;
			}
		}
	}
		
	//simple cases:
	Set[0] = Set[0] || _distanceSquare(p,dim)<r*r;
	if (_distanceSquare(p,half)<r*r)
		memset(Set,true,sizeof(Set));
	Set[7] = Set[7] || _distanceSquare(p,dim+3)<r*r;

	//lower cases
	Set[fc[0]] = Set[fc[0]] || sqr(p[0]-dim[3])+sqr(p[1]-dim[1])+sqr(p[2]-dim[2]) < r*r;
	Set[fc[1]] = Set[fc[1]] || sqr(p[0]-dim[0])+sqr(p[1]-dim[4])+sqr(p[2]-dim[2]) < r*r;
	Set[fc[2]] = Set[fc[2]] || sqr(p[0]-dim[0])+sqr(p[1]-dim[1])+sqr(p[2]-dim[5]) < r*r;
		
	//upper cases
	Set[fc[0]+fc[1]] = Set[fc[0]+fc[1]] || sqr(p[0]-dim[3])+sqr(p[1]-dim[4])+sqr(p[2]-dim[2]) < r*r;
	Set[fc[0]+fc[2]] = Set[fc[0]+fc[2]] || sqr(p[0]-dim[3])+sqr(p[1]-dim[1])+sqr(p[2]-dim[5]) < r*r;
	Set[fc[1]+fc[2]] = Set[fc[1]+fc[2]] || sqr(p[0]-dim[0])+sqr(p[1]-dim[4])+sqr(p[2]-dim[5]) < r*r;
		

	index_t at = sector_map.Count();
	for (BYTE k = 0; k < 8; k++)
		if (Set[k])
			if (child[k]->recursiveLookupSphere(p,r) == cnt)
			{
				sector_map.Truncate(at);
				sector_map<< this;
				return cnt;
			}
	//if (!sector_map.Count())
		//ShowMessage("none found");
	return 0;
}

template <class Def>MF_DECLARE	(bool) ObjMap<Def>::recursiveLookup(const C*lower_corner, const C*upper_corner)
{
	count_t cnt = Count();
	if (!cnt || _oneGreater(lower_corner,dim+3) || _oneLess(upper_corner,dim))
		return false;
	if (!level || (_allGreater(dim,lower_corner) && _allLess(dim+3,upper_corner)))
	{
		sector_map << this;
		return true;
	}
	bool rs = false;
	for (BYTE k = 0; k < 8; k++)
		rs |= child[k]->recursiveLookup(lower_corner,upper_corner);
	return rs;
}

template <class Def>MF_DECLARE	(count_t) ObjMap<Def>::lookupEdge(const C*p0, const C*p1)
{
	sector_map.reset();
	recursiveLookupEdge(p0,p1);
	return sector_map.Count();
}

template <class Def>MF_DECLARE	(count_t) ObjMap<Def>::lookupSphere(const C*p, const C&r)
{
	sector_map.reset();
	recursiveLookupSphere(p,r);
	return sector_map.Count();
}

template <class Def>MF_DECLARE	(count_t) ObjMap<Def>::lookup(const C*lower_corner, const C*upper_corner)
{
	sector_map.reset();
	recursiveLookup(lower_corner,upper_corner);
	return sector_map.Count();
}

template <class Def>MF_DECLARE	(ObjMap<Def>*) ObjMap<Def>::lookupCover(const C*lower_corner, const C*upper_corner)
{
	if (!level || empty())
		return this;
	for (BYTE k = 0; k < 8; k++)
		if (_allGreater(lower_corner,child[k]->dim) && _allLess(upper_corner,child[k]->dim+3))
			return child[k]->lookupCover(lower_corner,upper_corner);
	return this;
}

template <class Def>MF_DECLARE	(count_t) ObjMap<Def>::countSubMaps() const
{
	count_t cnt = 1;
	for (BYTE k = 0; k < 8; k++)
		if (child[k])
			cnt += child[k]->countSubMaps();
	return cnt;
}

template <class Def>MF_DECLARE	(count_t) ObjMap<Def>::Count() const
{
	return vertex_field.GetLength()+edge_field.GetLength()+triangle_field.GetLength()+quad_field.GetLength();
}


template <class Def>
MF_DECLARE	(count_t) ObjMap<Def>::resolveTriangles(Container::PreAllocVector<MeshTriangle<Def>*>&face_out)
{
	#ifndef THROW_BUFFER_ERROR
		overflow = false;
	#endif
	bad_hits = 0;
	index_t begin = face_out.Count();
	count_t total=0;
	for (index_t i = 0; i < sector_map.Count(); i++)
	{
		ObjMap<Def>*pntr = sector_map[i];
		total += pntr->triangle_field.GetLength();
		for (index_t j = 0; j < pntr->triangle_field.GetLength(); j++)
			if (!pntr->triangle_field[j]->marked)
			{
				face_out << pntr->triangle_field[j];
				pntr->triangle_field[j]->marked = true;
			}
			else
				bad_hits++;
	}
	for (index_t i = begin; i < face_out.Count(); i++)
		face_out[i]->marked = false;
	if (total && begin==face_out.Count())
		FATAL__("at least one resulting triangle expected");
	return face_out.Count()-begin;
}
	
template <class Def>
MF_DECLARE	(count_t) ObjMap<Def>::resolveQuads(Container::PreAllocVector<MeshQuad<Def>*>&face_out)
{
	#ifndef THROW_BUFFER_ERROR
		overflow = false;
	#endif
	bad_hits = 0;
	index_t begin = face_out.Count();
	for (index_t i = 0; i < sector_map.Count(); i++)
	{
		ObjMap<Def>*pntr = sector_map[i];
		for (index_t j = 0; j < pntr->quad_field.GetLength(); j++)
			if (!pntr->quad_field[j]->marked)
			{
				face_out << pntr->quad_field[j];
				pntr->quad_field[j]->marked = true;
			}
			else
				bad_hits++;
	}
	for (index_t i = begin; i < face_out.GetLength(); i++)
		face_out[i]->marked = false;
	return face_out.Count()-begin;
}
	

template <class Def>
MF_DECLARE	(count_t) ObjMap<Def>::resolveEdges(Container::PreAllocVector<MeshEdge<Def>*>&edge_out)
{
	#ifndef THROW_BUFFER_ERROR
		overflow = false;
	#endif
	bad_hits = 0;
	index_t begin = edge_out.Count();
	for (index_t i = 0; i < sector_map.Count(); i++)
	{
		ObjMap<Def>*pntr = sector_map[i];
		for (index_t j = 0; j < pntr->edge_field.GetLength(); j++)
			if (!pntr->edge_field[j]->marked)
			{
				edge_out << pntr->edge_field[j];
				pntr->edge_field[j]->marked = true;
			}
			else
				bad_hits++;
	}
	for (index_t i = begin; i < edge_out.Count(); i++)
		edge_out[i]->marked = false;
	return edge_out.Count()-begin;
}

template <class Def>
MF_DECLARE	(count_t) ObjMap<Def>::resolveVertices(Container::PreAllocVector<MeshVertex<Def>*>&vertex_out)
{
	#ifndef THROW_BUFFER_ERROR
		overflow = false;
	#endif
	bad_hits = 0;
	index_t begin = vertex_out.Count();
		
	for (index_t i = 0; i < sector_map.Count(); i++)
	{
		ObjMap<Def>*pntr = sector_map[i];
		for (index_t j = 0; j < pntr->vertex_field.GetLength(); j++)
			if (!pntr->vertex_field[j]->marked)
			{
				vertex_out << pntr->vertex_field[j];
				pntr->vertex_field[j]->marked = true;
			}
			else
				bad_hits++;
	}
	for (index_t i = begin; i < vertex_out.Count(); i++)
		vertex_out[i]->marked = false;
	return vertex_out.Count()-begin;
}

template <class Def> String Mesh<Def>::_error;
	
	

template <class FloatType>
	MF_CONSTRUCTOR Point<FloatType>::Point()
	{}
	
template <class FloatType>
	MF_CONSTRUCTOR Point<FloatType>::Point(const Float&x, const Float&y, const Float&z)
	{
		M::Vec::def(*this,x,y,z);
	}

template <class FloatType>
	MF_CONSTRUCTOR Point<FloatType>::Point(const M::TVec3<Float>&position_):M::TVec3<Float>(position_)
	{}

template <class FloatType> template <typename T>
	MF_DECLARE	(bool)				Point<FloatType>::operator>(const M::TVec3<T>&other)	const
	{
		return M::Vec::compare(*this,other)>0;
	}
		
template <class FloatType> template <typename T>
	MF_DECLARE	(bool)				Point<FloatType>::operator<(const M::TVec3<T>&other)	const
	{
		return M::Vec::compare(*this,other)<0;
	}
		
	
template <class FloatType>
	MF_CONSTRUCTOR NormalPoint<FloatType>::NormalPoint()
	{}
		
template <class FloatType>
	MF_CONSTRUCTOR NormalPoint<FloatType>::NormalPoint(const Float&x, const Float&y, const Float&z,const Float&nx, const Float&ny, const Float&nz):Point<FloatType>(x,y,z)
	{
		M::Vec::def(normal,nx,ny,nz);
	}
		
template <class FloatType>
	MF_CONSTRUCTOR NormalPoint<FloatType>::NormalPoint(const M::TVec3<Float>&position, const M::TVec3<Float>&normal_):Point<FloatType>(position)
	{
		normal = normal_;
	}

template <class FloatType> template <typename T>
	MF_DECLARE	(bool)		NormalPoint<FloatType>::operator>(const NormalPoint<T>&other)	const
	{
		char rs = M::Vec::compare(*this,other);
		if (rs > 0)
			return true;
		if (rs < 0)
			return false;
		return M::Vec::compare(normal,other.normal)>0;
	}
		
template <class FloatType> template <typename T>
	MF_DECLARE	(bool)		NormalPoint<FloatType>::operator<(const NormalPoint<T>&other)	const
	{
		char rs = M::Vec::compare(*this,other);
		if (rs < 0)
			return true;
		if (rs > 0)
			return false;
		return M::Vec::compare(normal,other.normal)<0;
	}

	

template <class FloatType>
	MF_DECLARE	(bool) AbstractSphere<FloatType>::intersects(const AbstractSphere<Float>&remote)	const
	{
		Float distance2 = M::Vec::quadraticDistance(Super::center,remote.center);
		if (distance2 > sqr(Super::radius + remote.radius))
			return false;
		return true;
	}


template <class FloatType>
	MF_DECLARE	(bool) AbstractSphere<FloatType>::intersects(const AbstractCylinder<Float>&remote)	const
	{
		M::TVec3<Float> v;
		M::Vec::sub(remote.p1,remote.p0,v);
		Float alpha = (M::Vec::dot(Super::center,v)-M::Vec::dot(remote.p0,v))/M::Vec::dot(v);
		if (alpha < -Math::GetError<Float>() || alpha > (Float)1+Math::GetError<Float>())
			return false;
		M::TVec3<Float> p;
		M::Vec::mad(remote.p0,v,alpha,p);
		Float distance = M::Vec::distance(Super::center,p);
		if (distance > Super::radius+remote.radius)
			return false;
		return true;
	}

template <class FloatType>
	MF_DECLARE	(void) AbstractSphere<FloatType>::resolveIndentation(const AbstractSphere<Float>&remote, Float&indentation, M::TVec3<Float>&indentation_vector, bool verbose)	const
	{
		if (verbose)
			ShowMessage("testing local sphere "+M::Vec::toString(Super::center)+" r"+FloatToStr(Super::radius)+"\n with remote sphere "+M::Vec::toString(remote.center)+" r"+FloatToStr(remote.radius));

		Float distance = M::Vec::distance(Super::center,remote.center);
		if (distance > Super::radius + remote.radius)
			return;
		Float local_indentation = (Float)1-(distance-remote.radius)/Super::radius;
		if (local_indentation > indentation)
		{
			indentation = local_indentation;
			M::Vec::sub(remote.center,Super::center,indentation_vector);
			M::Vec::mult(indentation_vector,indentation/distance);
		}
	}


template <class FloatType>
	MF_DECLARE	(void) AbstractSphere<FloatType>::resolveIndentation(const AbstractCylinder<Float>&remote, Float&indentation, M::TVec3<Float>&indentation_vector, bool verbose)	const
	{
		if (verbose)
			ShowMessage("testing local sphere "+M::Vec::toString(Super::center)+" r"+FloatToStr(Super::radius)+"\n with remote cylinder "+M::Vec::toString(remote.p0)+" "+M::Vec::toString(remote.p1)+" r"+FloatToStr(remote.radius));

		M::TVec3<Float> v;
		M::Vec::sub(remote.p1,remote.p0,v);
		Float alpha = (M::Vec::dot(Super::center,v)-M::Vec::dot(remote.p0,v))/M::Vec::dot(v);
		if (alpha < -Math::GetError<Float>() || alpha > (Float)1+Math::GetError<Float>())
			return;
		M::TVec3<Float> p;
		M::Vec::mad(remote.p0,v,alpha,p);
		Float distance = M::Vec::distance(Super::center,p);
		if (distance > Super::radius+remote.radius)
			return;
		Float local_indentation = (Float)1-(distance-remote.radius)/Super::radius;
		if (local_indentation > indentation)
		{
			indentation = local_indentation;
			M::Vec::sub(p,Super::center,indentation_vector);
			M::Vec::mult(indentation_vector,indentation/distance);
		}
	}
	
	

template <class FloatType>
	MF_CONSTRUCTOR AbstractCylinder<FloatType>::AbstractCylinder(const Float&radius_):p0(0,0,0),p1(0,1,0),radius(radius_)
	{}

template <class FloatType>
	MF_CONSTRUCTOR AbstractCylinder<FloatType>::AbstractCylinder(const M::TVec3<Float>&p0_, const M::TVec3<Float>&p1_, const Float&radius_):p0(p0_),p1(p1_),radius(radius_)
	{}


template <class FloatType>
	MF_DECLARE	(bool)  AbstractCylinder<FloatType>::intersects(const AbstractSphere<Float>&remote)	const
	{
		M::TVec3<Float> v;
		M::Vec::sub(p1,p0,v);
		Float alpha = (M::Vec::dot(remote.center,v)-M::Vec::dot(p0,v))/M::Vec::dot(v);
		if (alpha < -Math::GetError<Float>() || alpha > (Float)1+Math::GetError<Float>())
			return false;
		M::TVec3<Float> p;
		M::Vec::mad(p0,v,alpha,p);
		Float distance = M::Vec::distance(remote.center,p);
		if (distance > radius+remote.radius)
			return false;
		return true;
	}

template <class FloatType>
	MF_DECLARE	(bool) AbstractCylinder<FloatType>::intersects(const AbstractCylinder<Float>&remote)	const
	{
		M::TVec3<Float> v,w,h,n;
		M::Vec::sub(p1,p0,v);
		M::Vec::sub(remote.p1,remote.p0,w);
		M::Vec::cross(v,w,h);
		if (M::Vec::zero(h))
			return false;
		M::Vec::cross(h,w,n);
		Float vn = M::Vec::dot(v,n);
		if (vabs(vn) <= Math::GetError<Float>())
			return false;
		Float alpha = (M::Vec::dot(remote.p0,n)-M::Vec::dot(p0,n))/vn;
		if (alpha < -Math::GetError<Float>() || alpha > (Float)1+Math::GetError<Float>())
			return false;
		M::Vec::cross(h,v,n);
		Float wn = M::Vec::dot(w,n);
		if (vabs(wn) <= Math::GetError<Float>())
			return false;
		Float beta = (M::Vec::dot(p0,n)-M::Vec::dot(remote.p0,n))/wn;
		if (beta < -Math::GetError<Float>() || beta > (Float)1+Math::GetError<Float>())
			return false;
		M::TVec3<Float> lp,rp;
		M::Vec::mad(p0,v,alpha,lp);
		M::Vec::mad(remote.p0,w,beta,rp);
		Float distance = M::Vec::distance(lp,rp);
		if (distance > radius+remote.radius)
			return false;
		return true;
	}
	
	
template <class FloatType>
	MF_DECLARE	(void) AbstractCylinder<FloatType>::resolveIndentation(const AbstractSphere<Float>&remote, Float&indentation, M::TVec3<Float>&indentation_vector, bool verbose)	const
	{
		if (verbose)
			ShowMessage("testing local cylinder "+M::Vec::toString(p0)+" "+M::Vec::toString(p1)+" r"+FloatToStr(radius)+"\n with remote sphere "+M::Vec::toString(remote.center)+" r"+FloatToStr(remote.radius));

		M::TVec3<Float> v;
		M::Vec::sub(p1,p0,v);
		Float alpha = (M::Vec::dot(remote.center,v)-M::Vec::dot(p0,v))/M::Vec::dot(v);
		if (alpha < -Math::GetError<Float>() || alpha > (Float)1+Math::GetError<Float>())
			return;
		M::TVec3<Float> p;
		M::Vec::mad(p0,v,alpha,p);
		Float distance = M::Vec::distance(remote.center,p);
		if (distance > radius+remote.radius)
			return;
		Float local_indentation = (Float)1-(distance-remote.radius)/radius;
		if (local_indentation > indentation)
		{
			indentation = local_indentation;
			M::Vec::sub(remote.center,p,indentation_vector);
			M::Vec::mult(indentation_vector,indentation/distance);
		}
	}

template <class FloatType>
	MF_DECLARE	(void) AbstractCylinder<FloatType>::resolveIndentation(const AbstractCylinder<Float>&remote, Float&indentation, M::TVec3<Float>&indentation_vector, bool verbose)	const
	{
		if (verbose)
			ShowMessage("testing local cylinder "+M::Vec::toString(p0)+" "+M::Vec::toString(p1)+" r"+FloatToStr(radius)+"\n with remote cylinder "+M::Vec::toString(remote.p0)+" "+M::Vec::toString(remote.p1)+" r"+FloatToStr(remote.radius));
		M::TVec3<Float> v,w,h,n;
		M::Vec::sub(p1,p0,v);
		M::Vec::sub(remote.p1,remote.p0,w);
		M::Vec::cross(v,w,h);
		if (M::Vec::zero(h))
			return;
		M::Vec::cross(h,w,n);
		Float vn = M::Vec::dot(v,n);
		if (vabs(vn) <= Math::GetError<Float>())
			return;
		Float alpha = (M::Vec::dot(remote.p0,n)-M::Vec::dot(p0,n))/vn;
		if (alpha < -Math::GetError<Float>() || alpha > (Float)1+Math::GetError<Float>())
			return;
		M::Vec::cross(h,v,n);
		Float wn = M::Vec::dot(w,n);
		if (vabs(wn) <= Math::GetError<Float>())
			return;
		Float beta = (M::Vec::dot(p0,n)-M::Vec::dot(remote.p0,n))/wn;
		if (beta < -Math::GetError<Float>() || beta > (Float)1+Math::GetError<Float>())
			return;
		M::TVec3<Float> lp,rp;
		M::Vec::mad(p0,v,alpha,lp);
		M::Vec::mad(remote.p0,w,beta,rp);
		Float distance = M::Vec::distance(lp,rp);
		if (distance > radius+remote.radius)
			return;
		Float local_indentation = (Float)1-(distance-remote.radius)/(radius);
		if (local_indentation > indentation)
		{
			indentation = local_indentation;
			M::Vec::sub(rp,lp,indentation_vector);
			M::Vec::mult(indentation_vector,indentation/distance);
		}
	}
	



template <class FloatType> AbstractHull<FloatType>::AbstractHull(count_t spheres_, count_t cylinders_):spheres(spheres_),cylinders(cylinders_)
{}
	


template <class FloatType> 
	MF_DECLARE	(bool)			AbstractHull<FloatType>::intersects(const AbstractSphere<Float>&remote)	const
	{
		for (index_t i = 0; i < spheres.Count(); i++)
			if (spheres[i].intersects(remote))
				return true;
		for (index_t i = 0; i < cylinders.Count(); i++)
			if (cylinders[i].intersects(remote))
				return true;
		return false;
	}
template <class FloatType> 
	MF_DECLARE	(bool)			AbstractHull<FloatType>::intersects(const AbstractCylinder<Float>&remote)	const
	{
		for (index_t i = 0; i < spheres.Count(); i++)
			if (spheres[i].intersects(remote))
				return true;
		for (index_t i = 0; i < cylinders.Count(); i++)
			if (cylinders[i].intersects(remote))
				return true;
		return false;
	}


	
template <class FloatType> 
	MF_DECLARE	(bool)			AbstractHull<FloatType>::intersects(const AbstractHull<Float>&remote)	const
	{
		for (index_t j = 0; j < remote.spheres.Count(); j++)
			if (intersects(remote.spheres[j]))
				return true;
		for (index_t j = 0; j < remote.cylinders.Count(); j++)
			if (intersects(remote.cylinders[j]))
				return true;
		return false;
	}
	

template <class FloatType> 
	MF_DECLARE	(void)			AbstractHull<FloatType>::resolveIndentation(const AbstractSphere<Float>&remote, Float&indentation, M::TVec3<Float>& indentation_out, bool verbose)	const
	{
		for (index_t i = 0; i < spheres.Count(); i++)
			spheres[i].resolveIndentation(remote,indentation,indentation_out,verbose);
		for (index_t i = 0; i < cylinders.Count(); i++)
			cylinders[i].resolveIndentation(remote,indentation,indentation_out,verbose);
	}
template <class FloatType> 
	MF_DECLARE	(void)			AbstractHull<FloatType>::resolveIndentation(const AbstractCylinder<Float>&remote, Float&indentation, M::TVec3<Float>& indentation_out, bool verbose)	const
	{
		for (index_t i = 0; i < spheres.Count(); i++)
			spheres[i].resolveIndentation(remote,indentation,indentation_out,verbose);
		for (index_t i = 0; i < cylinders.Count(); i++)
			cylinders[i].resolveIndentation(remote,indentation,indentation_out,verbose);
	}


	
template <class FloatType> 
	MF_DECLARE	(bool) AbstractHull<FloatType>::detectMaximumIndentation(const AbstractHull<Float>&remote, M::TVec3<float>&indentation_out, bool verbose)	const
	{
		if (verbose)
			ShowMessage("local: "+ToString()+"\nremote: "+remote.ToString());
		float indentation(0);

		for (index_t j = 0; j < remote.spheres.Count(); j++)
			resolveIndentation(remote.spheres[j],indentation,indentation_out,verbose);
		for (index_t j = 0; j < remote.cylinders.Count(); j++)
			resolveIndentation(remote.cylinders[j],indentation,indentation_out,verbose);

		if (verbose)
			ShowMessage("final maximum indentation is "+FloatToStr(indentation));
			
		return indentation > 0;
	}
	
template <class FloatType>
	MF_DECLARE	(String) AbstractHull<FloatType>::ToString() const
	{
		String rs;
		for (index_t i = 0; i < spheres.Count(); i++)
			rs+=" ("+IntToStr(i)+") sphere: "+M::Vec::toString(spheres[i].center)+" r"+String(spheres[i].radius)+"\n";
		for (index_t i = 0; i < cylinders.Count(); i++)
			rs+=" ("+IntToStr(i+spheres.Count())+") cylinder: "+M::Vec::toString(cylinders[i].p0)+" "+M::Vec::toString(cylinders[i].p1)+" r"+String(spheres[i].radius)+"\n";

		return rs;
	}
	
template <class FloatType>
	MFUNC(void)	AbstractHull<FloatType>::translate(const M::TVec3<C>&delta)
	{
		for (index_t i = 0; i < spheres.Count(); i++)
			M::Vec::add(spheres[i].center,delta);
		for (index_t i = 0; i < cylinders.Count(); i++)
		{
			M::Vec::add(cylinders[i].p0,delta);
			M::Vec::add(cylinders[i].p1,delta);
		}
	}





template <class Float>
	MF_DECLARE	(void)		makeSphere(const Float&radius,AbstractHull<Float>&result)
	{
		result.cylinders.free();
		result.spheres.SetSize(1);
		M::Vec::clear(result.spheres[0].center);
		result.spheres[0].radius = radius;
	}
	
template <class Float>
	MF_DECLARE	(void)		makeSphere(const M::TVec3<Float>&center, const Float&radius,AbstractHull<Float>&result)
	{
		result.cylinders.free();
		result.spheres.SetSize(1);
		result.spheres[0].radius = radius;
		result.spheres[0].center = center;
	}
	
template <class Float>
	MF_DECLARE	(void)		makeCapsule(const M::TVec3<Float>&p0, const M::TVec3<Float>&p1, const Float&radius,AbstractHull<Float>&result)
	{
		result.cylinders.SetSize(1);
		result.spheres.SetSize(2);
		result.spheres[0] = AbstractSphere<Float>(p0,radius);
		result.spheres[1] = AbstractSphere<Float>(p1,radius);
		result.cylinders[0] = AbstractCylinder<Float>(p0,p1,radius);
	}


template <class Def> Container::PreAllocVector<ObjMap<Def>*>	ObjMap<Def>::sector_map;
	
	
template <typename Float>
	MF_DECLARE	(void)			ConvexHullBuilder<Float>::Reset()
	{
		buffer.reset();
		vertices.reset();
		triangles.reset();
	}
	
template <typename Float>
	MF_DECLARE	(void)			ConvexHullBuilder<Float>::UpdateNormal(TTriangle&triangle)	const
	{
		_oTriangleNormal(vertices[triangle.v0].vector,vertices[triangle.v1].vector,vertices[triangle.v2].vector,triangle.normal);
		M::Vec::normalize0(triangle.normal);
	}
	
template <typename Float>
	MF_DECLARE	(bool)			ConvexHullBuilder<Float>::DetectTedrahedron()
	{
		if (buffer.GetLength()<4)
			return false;
		
		for (index_t i = 1; i < buffer.GetLength()-2; i++)
			for (index_t j = i+1; j < buffer.GetLength()-1; j++)
				for (index_t k = j+1; k < buffer.GetLength(); k++)
				{
					Float vol = Obj::TetrahedronVolume(buffer[0].vector,buffer[i].vector,buffer[j].vector,buffer[k].vector);
					/*if (vabs(vol) <= vmax(_distance(buffer[0].vector,buffer[i].vector),_distance(buffer[j].vector,buffer[k].vector))/2)
						continue;*/
					vertices << buffer[0] << buffer[i] << buffer[j] << buffer[k];
					buffer.Erase(k);
					buffer.Erase(j);
					buffer.Erase(i);
					buffer.Erase(index_t(0));
					if (vol > 0)
					{
						updateNormal(triangles.Append().Set(0,1,2,1,2,3));
						updateNormal(triangles.Append().Set(1,0,3,0,3,2));
						updateNormal(triangles.Append().Set(2,1,3,0,1,3));
						updateNormal(triangles.Append().Set(0,2,3,0,2,1));
					}
					else
					{
						updateNormal(triangles.Append().Set(0,2,1,3,2,1));
						updateNormal(triangles.Append().Set(1,3,0,2,3,0));
						updateNormal(triangles.Append().Set(2,3,1,3,1,0));
						updateNormal(triangles.Append().Set(0,3,2,1,2,0));
					}
					return true;
				}
		return false;
	}
	
template <typename Float> template <typename T>
	MF_DECLARE	(void)			ConvexHullBuilder<Float>::Include(const M::TVec3<T>&point)
	{
		//static Log	logfile("include.log",true);
		ASSERT2__((triangles.Count()>0)==(vertices.Count()>0),triangles.Count(),vertices.Count());
		if (!vertices.Count())
		{
			buffer.Append().Set(point,0,0);
			if (!DetectTedrahedron())
			{
				return;
			}
			for (index_t i = 0; i < buffer.GetLength(); i++)
				Include(buffer[i].vector);
			buffer.reset();
			ASSERT2__((triangles.Count()>0)==(vertices.Count()>0),triangles.Count(),vertices.Count());
		}
		else
		{
			/*verifyIntegrity();
			logfile<< nl<<"begin"<<nl;
			logfile<< "vertices:"<<nl;
			for (unsigned i = 0; i < vertices.Count(); i++)
				logfile << " "<<i<<": "<<_toString(vertices[i].vector)<<nl;
			logfile<< "triangles:"<<nl;
			for (unsigned i = 0; i < triangles.Count(); i++)
				logfile << " "<<i<<": v"<<_toString(triangles[i].v)<<" n"<<_toString(triangles[i].n)<<nl;
				
			logfile<< "attempting to include point ("<<_toString(point)<<")"<<nl;*/
			ASSERT__(triangles.Count()>2);
			M::TVec3<Float> axis,plane_normal,c;
			M::Vec::center(vertices[triangles.First().v0].vector,vertices[triangles.First().v1].vector,vertices[triangles.First().v2].vector,c);
			M::Vec::sub(point,c,axis);
			M::Vec::cross(triangles.First().normal,axis,plane_normal);
			if (M::Vec::similar(plane_normal,M::Vector3<Float>::zero))
			{
				M::TVec3<Float> v2;
				M::Vec::def(v2,vabs(axis.y)+1,axis.z,axis.x);
				M::Vec::cross(v2,axis,plane_normal);
			}
			M::Vec::normalize0(plane_normal);
			//logfile<< "plane is "<<_toString(plane_normal)<<nl;
				
			UINT32 face = 0,first=0,prev=UNSIGNED_UNDEF;
			count_t steps(0);
			while (true)
			{
				//logfile<< " now analysing face "<<face<<"/"<<triangles.Count()<<nl;
				ASSERT__(steps++<=triangles.Count());
				const TTriangle&t = triangles[face];
				M::TVec3<Float> axis;
				M::Vec::sub(point,vertices[t.v0].vector,axis);
				if (M::Vec::dot(axis,t.normal)>0)
				{
					//logfile<< " found viewing triangle"<<nl;
					break;
				}
					
				BYTE edge = 255;
				for (BYTE k = 0; k < 3; k++)
				{
					//logfile<< "  checking edge to neighbor "<<k<<nl;
					if (t.n[k] == prev)
					{
						//logfile<< "   edge leads back "<<nl;
						continue;
					}
					const M::TVec3<Float>	&p0 = vertices[t.v[k]].vector,
										&p1 = vertices[t.v[(k+1)%3]].vector;
					M::TVec3<Float>		dif,
										vdir;
					M::Vec::sub(p1,p0,vdir);
					M::Vec::sub(point,p0,dif);
					Float alpha = M::Vec::dot(plane_normal,vdir);
					if (vabs(alpha)<=0)
						continue;
					alpha = M::Vec::dot(plane_normal,dif)/alpha;
					if (alpha >= 0 && alpha <= 1)
					{
						edge = k;
						//logfile<< "   following this edge to face "<<t.n[k]<<nl;
						break;
					}
				}
				if (edge == 255)
				{
					ErrMessage("Unable to find jump edge");
					return;
				}
				prev = face;
				face = t.n[edge];
				if (face == first)
				{
					//logfile<< " full circle. aborting"<<nl;
					return;
				}
			}
			//now i have the face to start from in face
			//logfile<< "viewing triangle is "<<face<<nl;
				
			edges.reset();
			Queue<UINT32>	queue;
			queue << face;
			count_t flagged = 1;
				
			for (index_t i = 0; i < triangles.Count(); i++)
				if (triangles[i].flagged)
					FATAL__("triangle "+String(i)+" flagged before flagging");
				
			steps = 0;
			triangles[face].flagged = true;
			while (!queue.IsEmpty())
			{
				ASSERT__(steps++<=triangles.Count());
				queue >> face;
				TTriangle&t = triangles[face];
				//logfile<< " walking from face "<<face<<nl;
					
				for (BYTE k = 0; k < 3; k++)
				{
					//logfile<< "  analysing edge "<<k<<nl;
					ASSERT2__(t.n[k]<triangles.Count(),t.n[k],triangles.Count());
					if (triangles[t.n[k]].flagged)
					{
						//logfile<< "  neighbor "<<t.n[k]<<" already flagged"<<nl;
						continue;
					}
					const TTriangle&n = triangles[t.n[k]];

					
					M::TVec3<Float> axis;
					M::Vec::sub(point,vertices[n.v0].vector,axis);
					ASSERT__(!M::Vec::similar(point,vertices[n.v0].vector));
					ASSERT__(!M::Vec::similar(point,vertices[n.v1].vector));
					ASSERT__(!M::Vec::similar(point,vertices[n.v2].vector));
					if (M::Vec::dot(axis,n.normal)<=0)	//got shadow edge
					{
						//logfile<< " shadow edge found"<<nl;
						TEdge&edge = edges.Append();
						edge.v0 = t.v[k];
						edge.v1 = t.v[(k+1)%3];
						edge.t0 = face;
						edge.t1 = t.n[k];
						edge.index = k;
					}
					else
					{
						//logfile<< " enqueueing neighbor"<<nl;
						queue << t.n[k];
						triangles[t.n[k]].flagged = true;
						flagged++;
					}
				}
			}
			if (!edges.Count())
			{
				/*Log	error("error.log",true);
				error<<"object.vertex_field.Resize("<<vertices.Count()<<");"<<nl;
				error<<"object.triangle_field.Resize("<<triangles.Count()<<");"<<nl;
				for (unsigned i = 0; i < vertices.Count(); i++)
					error << "_v3(object.vertex_field["<<i<<"].position,"<<vertices[i].vector[0]<<", "<<vertices[i].vector[1]<<", "<<vertices[i].vector[2]<<");"<<nl;
				for (unsigned i = 0; i < triangles.Count(); i++)
					error << "_v3(object.triangle_field["<<i<<"].vertex,object.vertex_field+"<<triangles[i].v0<<", object.vertex_field+"<<triangles[i].v1<<", object.vertex_field+"<<triangles[i].v2<<");"<<nl;
					
				error << "_v3(point,"<<point[0]<<", "<<point[1]<<", "<<point[2]<<");"<<nl;*/
				return;
				
				
			}
			ASSERT__(edges.Count()>0);
			ASSERT2__(flagged < triangles.Count(),flagged,triangles.Count());
			PreAllocVector<TTriangle>	new_triangles;
			//logfile<< "queue ran empty after "<<counter<<" iterations"<<nl;
				
				
			UINT32 new_index = vertices.Count();
			vertices.Append().Set(point,0,0);
			//logfile<< "new point inserted to a total of "<<vertices.Count()<<" points"<<nl;
				
			/*for (unsigned i = 0; i < vertices.Count(); i++)
				vertices[i].index0 = vertices[i].index1 = UNSIGNED_UNDEF;*/
				
			for (index_t i = 0; i < edges.Count(); i++)
			{
				const TEdge&edge = edges[i];
				//logfile<< " inserting new face for edge "<<i<<"/"<<edges.Count()<<nl;
				//logfile<< "  "<<edge.v0<<"-"<<edge.v1<<nl;
				//ASSERT__(!triangles[edge.t1].flagged);
				updateNormal(new_triangles.Append().Set(edge.v0,edge.v1,new_index,edge.t1,UNSIGNED_UNDEF,UNSIGNED_UNDEF));
					
				triangles[edge.t0].link.v[edge.index] = new_triangles.Count()-1;
					
				/*ASSERT__(vertices[edge.v0].index0 == UNSIGNED_UNDEF);
				ASSERT__(vertices[edge.v1].index1 == UNSIGNED_UNDEF);*/
				vertices[edge.v0].index0 = new_triangles.Count()-1;
				vertices[edge.v1].index1 = new_triangles.Count()-1;
				//logfile<< "  i0["<<edge.v0<<"] <= "<<new_triangles.Count()-1<<nl;
				//logfile<< "  i1["<<edge.v1<<"] <= "<<new_triangles.Count()-1<<nl;
					
			}
				

			for (index_t i = 0; i < edges.Count(); i++)
			{
				const TEdge&edge = edges[i];
				//logfile<< " linking new face of edge "<<i<<"/"<<edges.Count()<<nl;
				new_triangles[i].n1 = vertices[edges[i].v1].index0;
				new_triangles[i].n2 = vertices[edges[i].v0].index1;
				//logfile<< "  i0["<<edge.v1<<"] => "<<new_triangles[i].n1<<nl;
				//logfile<< "  i1["<<edge.v0<<"] => "<<new_triangles[i].n2<<nl;
			}
				
			for (index_t i = 0; i < triangles.Count(); i++)
				if (!triangles[i].flagged)
				{
					//logfile<< " copying preserved face "<<i<<"/"<<triangles.Count()<<nl;
					M::Vec::set(triangles[i].link,new_triangles.Count());
					new_triangles << triangles[i];
					new_triangles.Last().flagged = false;
					new_triangles.Last().link.x = i;
				}
			for (index_t i = 0; i < edges.Count(); i++)
				if (new_triangles[i].n0 != UNSIGNED_UNDEF)
					for (BYTE j = 0; j < 3; j++)
					{
						if (triangles[new_triangles[i].n0].v[(j+1)%3] == new_triangles[i].v0)
						{
							new_triangles[i].n0 = triangles[new_triangles[i].n0].link.v[j];
							break;
						}
					}
			for (index_t i = edges.Count(); i < new_triangles.Count(); i++)
				for (BYTE k = 0; k < 3; k++)
				{
					if (new_triangles[i].n[k] != UNSIGNED_UNDEF)
						for (BYTE j = 0; j < 3; j++)
							if (triangles[new_triangles[i].n[k]].v[(j+1)%3] == new_triangles[i].v[k])
							{
								//logfile<< " replacing neighbor "<<k<<" of triangle "<<i<<"/"<<triangles.Count()<<" with link "<<j<<" of old triangle "<<new_triangles[i].n[k]<<nl;
								new_triangles[i].n[k] = triangles[new_triangles[i].n[k]].link.v[j];
								break;
							}
				}
									

													
			triangles.adoptData(new_triangles);
/*
			//logfile<< "vertices:"<<nl;
			for (unsigned i = 0; i < vertices.Count(); i++)
				logfile << " "<<i<<": "<<_toString(vertices[i].vector)<<nl;
			//logfile<< "triangles ("+String(edges.Count())+" are new):"<<nl;
			for (unsigned i = 0; i < triangles.Count(); i++)
				logfile << " "<<i<<": v"<<_toString(triangles[i].v)<<" n"<<_toString(triangles[i].n)<<nl;
			*/
			//logfile<< "verifying integrity"<<nl;
				
			//verifyIntegrity();
			ASSERT2__((triangles.Count()>0)==(vertices.Count()>0),triangles.Count(),vertices.Count());
		}
	}
	
template <typename Float>
template <class Def>
	MF_DECLARE	(void)			ConvexHullBuilder<Float>::ExportToMesh(Mesh<Def>&object)	const
	{
		Ctr::Array<UINT32>	vmap(vertices.Count());
		vmap.Fill(UNSIGNED_UNDEF);
		for (index_t i = 0; i < triangles.Count(); i++)
		{
			vmap[triangles[i].v0] = 0;
			vmap[triangles[i].v1] = 0;
			vmap[triangles[i].v2] = 0;
		}
		count_t vcnt = 0;
		for (index_t i = 0; i < vertices.Count(); i++)
			if (!vmap[i])
				vcnt++;
		object.vertex_field.SetSize(vcnt);
		vcnt = 0;
		for (index_t i = 0; i < vertices.Count(); i++)
			if (!vmap[i])
			{
				vmap[i] = vcnt;
				_c3(vertices[i].vector,object.vertex_field[vcnt++].position);
			}
		object.quad_field.SetSize(0);
		object.triangle_field.SetSize(triangles.Count());
		for (index_t i = 0; i < triangles.Count(); i++)
		{
			object.triangle_field[i].v0 = object.vertex_field+vmap[triangles[i].v0];
			object.triangle_field[i].v1 = object.vertex_field+vmap[triangles[i].v1];
			object.triangle_field[i].v2 = object.vertex_field+vmap[triangles[i].v2];
		}
	}

	
template <typename Float>
	MF_DECLARE	(void)			ConvexHullBuilder<Float>::VerifyIntegrity()	const
	{
		for (index_t i = 0; i < triangles.Count(); i++)
		{
			const TTriangle&t = triangles[i];
			for (BYTE k = 0; k < 3; k++)
			{
				if (t.v[k] >= vertices.Count())
					FATAL__("Invalid vertex index ("+String(t.v[k])+"/"+String(vertices.Count())+") of vertex "+String(k)+" of triangle "+String(i)+"/"+String(triangles.Count()));
				for (BYTE j = k+1; j < 3; j++)
					if (t.v[k] == t.v[j])
						FATAL__("Vertices "+String(k)+", "+String(j)+" collapsed ("+String(t.v[k])+") of triangle "+String(i)+"/"+String(triangles.Count()));
					
				if (t.n[k] >= triangles.Count())
					FATAL__("Invalid neighbor index ("+String(t.n[k])+"/"+String(triangles.Count())+") of neighbor "+String(k)+" of triangle "+String(i)+"/"+String(triangles.Count()));
				const TTriangle&n = triangles[t.n[k]];
				BYTE found = 255;
				for (BYTE j = 0; j < 3; j++)
					if (n.n[j] == i)
						found = j;
				if (found==255)
					FATAL__("Neighbor "+String(k)+" ("+String(t.n[k])+"/"+String(triangles.Count())+") does not know triangle "+String(i)+"/"+String(triangles.Count()));
				if (n.v[found] != t.v[(k+1)%3] || n.v[(found+1)%3] != t.v[k])
					FATAL__("Vertices to neighbor "+String(k)+" ("+String(t.n[k])+"/"+String(triangles.Count())+") do not match for triangle "+String(i)+"/"+String(triangles.Count())
							+" ("+String(n.v[(found+1)%3])+", "+String(n.v[found])+ "["+String(n.v[(found+2)%3])+ "] !="+String(t.v[k])+", "+String(t.v[(k+1)%3])+ ")");
					
				for (BYTE j = k+1; j < 3; j++)
					if (t.n[k] == t.n[j])
						FATAL__("Neighbors "+String(k)+", "+String(j)+" collapsed ("+String(t.n[k])+") of triangle "+String(i)+"/"+String(triangles.Count()));
			}
		}
		
	}
	
template <typename Float> template <typename T>
	MF_DECLARE	(void)			ConvexHullBuilder<Float>::Include(const Point<T>&point)
	{
		Include(point.vector);
	}
	

	
MFUNC
	(void)		SplineQuad<C>::resolve(const C&x, const C&y, C out[3])	const
	{
		C	ix = (C)1-x,
			x3 = x*x*x,
			x2 = x*x*ix*(C)3,
			x1 = x*ix*ix*(C)3,
			x0 = ix*ix*ix,
			iy = (C)1-y,
			y3 = y*y*y,
			y2 = y*y*iy*(C)3,
			y1 = y*iy*iy*(C)3,
			y0 = iy*iy*iy;
			
		out[0] =	control[0][0][0]*x0*y0 + control[1][0][0]*x1*y0 + control[2][0][0]*x2*y0 + control[3][0][0]*x3*y0
					+
					control[0][1][0]*x0*y1 + control[1][1][0]*x1*y1 + control[2][1][0]*x2*y1 + control[3][1][0]*x3*y1
					+
					control[0][2][0]*x0*y2 + control[1][2][0]*x1*y2 + control[2][2][0]*x2*y2 + control[3][2][0]*x3*y2
					+
					control[0][3][0]*x0*y3 + control[1][3][0]*x1*y3 + control[2][3][0]*x2*y3 + control[3][3][0]*x3*y3;
			
		out[1] =	control[0][0][1]*x0*y0 + control[1][0][1]*x1*y0 + control[2][0][1]*x2*y0 + control[3][0][1]*x3*y0
					+
					control[0][1][1]*x0*y1 + control[1][1][1]*x1*y1 + control[2][1][1]*x2*y1 + control[3][1][1]*x3*y1
					+
					control[0][2][1]*x0*y2 + control[1][2][1]*x1*y2 + control[2][2][1]*x2*y2 + control[3][2][1]*x3*y2
					+
					control[0][3][1]*x0*y3 + control[1][3][1]*x1*y3 + control[2][3][1]*x2*y3 + control[3][3][1]*x3*y3;
			
		out[2] =	control[0][0][2]*x0*y0 + control[1][0][2]*x1*y0 + control[2][0][2]*x2*y0 + control[3][0][2]*x3*y0
					+
					control[0][1][2]*x0*y1 + control[1][1][2]*x1*y1 + control[2][1][2]*x2*y1 + control[3][1][2]*x3*y1
					+
					control[0][2][2]*x0*y2 + control[1][2][2]*x1*y2 + control[2][2][2]*x2*y2 + control[3][2][2]*x3*y2
					+
					control[0][3][2]*x0*y3 + control[1][3][2]*x1*y3 + control[2][3][2]*x2*y3 + control[3][3][2]*x3*y3;
	}
		
MFUNC
	(void)		SplineQuad<C>::resolveDirectionX(const C&x, const C&y, C out[3]) const
	{
		C	ix = (C)1-x,
			x2 = x*x,
			x1 = x*ix*(C)2,
			x0 = ix*ix,
			iy = (C)1-y,
			y3 = y*y*y,
			y2 = y*y*iy*(C)3,
			y1 = y*iy*iy*(C)3,
			y0 = iy*iy*iy;
			
		out[0] =	(control[1][0][0]-control[0][0][0])*x0*y0 + (control[2][0][0]-control[1][0][0])*x1*y0 + (control[3][0][0]-control[2][0][0])*x2*y0
					+
					(control[1][1][0]-control[0][1][0])*x0*y1 + (control[2][1][0]-control[1][1][0])*x1*y1 + (control[3][1][0]-control[2][1][0])*x2*y1
					+
					(control[1][2][0]-control[0][2][0])*x0*y2 + (control[2][2][0]-control[1][2][0])*x1*y2 + (control[3][2][0]-control[2][2][0])*x2*y2
					+
					(control[1][3][0]-control[0][3][0])*x0*y3 + (control[2][3][0]-control[1][3][0])*x1*y3 + (control[3][3][0]-control[2][3][0])*x2*y3;
			
		out[1] =	(control[1][0][1]-control[0][0][1])*x0*y0 + (control[2][0][1]-control[1][0][1])*x1*y0 + (control[3][0][1]-control[2][0][1])*x2*y0
					+
					(control[1][1][1]-control[0][1][1])*x0*y1 + (control[2][1][1]-control[1][1][1])*x1*y1 + (control[3][1][1]-control[2][1][1])*x2*y1
					+
					(control[1][2][1]-control[0][2][1])*x0*y2 + (control[2][2][1]-control[1][2][1])*x1*y2 + (control[3][2][1]-control[2][2][1])*x2*y2
					+
					(control[1][3][1]-control[0][3][1])*x0*y3 + (control[2][3][1]-control[1][3][1])*x1*y3 + (control[3][3][1]-control[2][3][1])*x2*y3;
			
		out[2] =	(control[1][0][2]-control[0][0][2])*x0*y0 + (control[2][0][2]-control[1][0][2])*x1*y0 + (control[3][0][2]-control[2][0][2])*x2*y0
					+
					(control[1][1][2]-control[0][1][2])*x0*y1 + (control[2][1][2]-control[1][1][2])*x1*y1 + (control[3][1][2]-control[2][1][2])*x2*y1
					+
					(control[1][2][2]-control[0][2][2])*x0*y2 + (control[2][2][2]-control[1][2][2])*x1*y2 + (control[3][2][2]-control[2][2][2])*x2*y2
					+
					(control[1][3][2]-control[0][3][2])*x0*y3 + (control[2][3][2]-control[1][3][2])*x1*y3 + (control[3][3][2]-control[2][3][2])*x2*y3;
	}
	
MFUNC
	(void)		SplineQuad<C>::resolveDirectionY(const C&x, const C&y, C out[3]) const
	{
		C	ix = (C)1-x,
			x3 = x*x*x,
			x2 = x*x*ix*(C)3,
			x1 = x*ix*ix*(C)3,
			x0 = ix*ix*ix,
			iy = (C)1-y,
			y2 = y*y,
			y1 = y*iy*(C)2,
			y0 = iy*iy;
			
		out[0] =	(control[0][1][0]-control[0][0][0])*x0*y0 + (control[0][2][0]-control[0][1][0])*x0*y1 + (control[0][3][0]-control[0][2][0])*x0*y2
					+
					(control[1][1][0]-control[1][0][0])*x1*y0 + (control[1][2][0]-control[1][1][0])*x1*y1 + (control[1][3][0]-control[1][2][0])*x1*y2
					+
					(control[2][1][0]-control[2][0][0])*x2*y0 + (control[2][2][0]-control[2][1][0])*x2*y1 + (control[2][3][0]-control[2][2][0])*x2*y2
					+
					(control[3][1][0]-control[3][0][0])*x3*y0 + (control[3][2][0]-control[3][1][0])*x3*y1 + (control[3][3][0]-control[3][2][0])*x3*y2;
			
		out[1] =	(control[0][1][1]-control[0][0][1])*x0*y0 + (control[0][2][1]-control[0][1][1])*x0*y1 + (control[0][3][1]-control[0][2][1])*x0*y2
					+
					(control[1][1][1]-control[1][0][1])*x1*y0 + (control[1][2][1]-control[1][1][1])*x1*y1 + (control[1][3][1]-control[1][2][1])*x1*y2
					+
					(control[2][1][1]-control[2][0][1])*x2*y0 + (control[2][2][1]-control[2][1][1])*x2*y1 + (control[2][3][1]-control[2][2][1])*x2*y2
					+
					(control[3][1][1]-control[3][0][1])*x3*y0 + (control[3][2][1]-control[3][1][1])*x3*y1 + (control[3][3][1]-control[3][2][1])*x3*y2;
			
		out[2] =	(control[0][1][2]-control[0][0][2])*x0*y0 + (control[0][2][2]-control[0][1][2])*x0*y1 + (control[0][3][2]-control[0][2][2])*x0*y2
					+
					(control[1][1][2]-control[1][0][2])*x1*y0 + (control[1][2][2]-control[1][1][2])*x1*y1 + (control[1][3][2]-control[1][2][2])*x1*y2
					+
					(control[2][1][2]-control[2][0][2])*x2*y0 + (control[2][2][2]-control[2][1][2])*x2*y1 + (control[2][3][2]-control[2][2][2])*x2*y2
					+
					(control[3][1][2]-control[3][0][2])*x3*y0 + (control[3][2][2]-control[3][1][2])*x3*y1 + (control[3][3][2]-control[3][2][2])*x3*y2;
			

	}
		
	//		void		resolveDirectionY(const C&x, const C&y, C out[3]) const;

		
MFUNC
	(void)		SplineQuad<C>::resolveDeltaVectors(const C p0[3], const C n0[3], const C p1[3], const C n1[3], const C&control_factor, C c0[3], C c1[3])
	{
		C delta[3],d0[3],d1[3],t[3];
		_sub(p1,p0,delta);
		C distance = _distance(p0,p1);

		_cross(delta,n0,t);
		_cross(n0,t,d0);
					
		_cross(delta,n1,t);
		_cross(n1,t,d1);
					
		_normalize(d0);
		_normalize(d1);
		_mult(d0,distance*control_factor,c0);
		_mult(d1,-distance*control_factor,c1);
	}
		
		
MFUNC
	(void)		SplineQuad<C>::build(const C p0[3], const C n0[3], const C p1[3], const C n1[3], const C p2[3], const C n2[3], const C p3[3], const C n3[3], const C&control_factor)
	{
		const C	*p[4] = {p0,p1,p2,p3},
				*n[4] = {n0,n1,n2,n3};
			
		C		vx00[3],vx01[3],
				vx10[3],vx11[3],
				vy00[3],vy01[3],
				vy10[3],vy11[3],
				vd00[3],vd11[3],
				vd10[3],vd01[3];
			
		resolveDeltaVectors(p0,n0,p1,n1,control_factor,vx00,vx10);
		resolveDeltaVectors(p3,n3,p2,n2,control_factor,vx01,vx11);
		resolveDeltaVectors(p0,n0,p3,n3,control_factor,vy00,vy01);
		resolveDeltaVectors(p1,n1,p2,n2,control_factor,vy10,vy11);
			
		resolveDeltaVectors(p0,n0,p2,n2,control_factor,vd00,vd11);
		resolveDeltaVectors(p1,n1,p3,n3,control_factor,vd10,vd01);
			
		_c3(p0,control[0][0]);
		_add(p0,vx00,control[1][0]);
		_add(p1,vx10,control[2][0]);
		_c3(p1,control[3][0]);
			
		_add(p0,vy00,control[0][1]);
		//_add(control[0][1],vx00,control[1][1]);
		_add(p0,vd00,control[1][1]);
		_add(p1,vy10,control[3][1]);
		_add(p1,vd10,control[2][1]);
		//_add(control[3][1],vx10,control[2][1]);
			
		_add(p3,vy01,control[0][2]);
		//_add(control[0][2],vx01,control[1][2]);
		_add(p3,vd01,control[1][2]);
		_add(p2,vy11,control[3][2]);
		//_add(control[3][2],vx11,control[2][2]);
		_add(p2,vd11,control[2][2]);
			
		_c3(p3,control[0][3]);
		_add(p3,vx01,control[1][3]);
		_add(p2,vx11,control[2][3]);
		_c3(p2,control[3][3]);
						
	}
	

}
}

#endif
