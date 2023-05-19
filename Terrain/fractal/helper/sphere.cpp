#include <global_root.h>
#include "sphere.h"


const double ::Sphere::relative_height_step = 2.36e-4;

::Sphere::Sphere():built(false)
{}

::Sphere::Sphere(const Engine::Frustum<>&frustum, double radius, double height, const M::TVec3<double>&delta)
{
	apply(frustum,radius,height, delta);
}

void			::Sphere::apply(const Frustum<>&frustrum_, double radius_, double height_,const M::TVec3<double>&delta_)
{
	frustum = frustrum_;
	delta = delta_;
	height = height_;
	radius = radius_;
	
	M::TVec3<double>	axis;
	M::Vec::div(delta,height,axis);
	M::TMatrix4<>		system;
	M::Vec::mult(axis,-(height-radius),world_offset);
	M::Mat::makeAxisSystem(world_offset,axis,2,world_system);
	world_matrix.x = world_system.x.xyz;
	world_matrix.y = world_system.y.xyz;
	world_matrix.z = world_system.z.xyz;
	
	M::Mat::invertSystem(world_system,system);
	

	M::TVec3<> axis0,axis1;
	
	M::Vec::sub(frustum.far_bottom_right,frustum.near_bottom_right,axis0);
	M::Vec::sub(frustum.far_top_left,frustum.near_top_left,axis1);
	M::Mat::rotate(system,axis0);
	M::Mat::rotate(system,axis1);
	M::Vec::center(axis0,axis1,axis);
	
	y_fov = M::Vec::angle(axis0,axis1)*M_PI/180/2*1.35;
	view_x = M::Vec::angle2PI(axis.y,axis.x);
	view_y = atan2(M::vsqrt(M::Sqr(axis.x)+M::Sqr(axis.y)),-axis.z);
	float  fc = M::Min(M::vabs(view_y/M_PI*2),1.0f);
	x_fov = y_fov/M::Sqr(0.01+fc*0.99)*0.65;
	
}
/*
TTracedVertex	Sphere::trace(float x, float y)	const
{
	double	planet_relative[3],
			p[3],d[3],b0[3],b1[3];
	_mad(bb,bx,x,p);
	_mad(p,by,y);
	_mad(db,dx,x,d);
	_mad(d,dy,y);
	//ShowMessage(_toString(bb)+" x"+_toString(bx)+" y"+_toString(by)+ " ("+String(x)+", "+String(y)+")");
	_normalize0(d);
	_add(p,delta);
	TTracedVertex vertex;
	vertex.range = height;
	if (vertex.isset=rayCast(p,d,radius,vertex.range))
	{
		//ShowMessage(String(vertex.range)+"/"+String(height-radius));
		double b[3];
		_mad(p,d,vertex.range,planet_relative);
		_sub(planet_relative,delta,b);
		_c3(b,vertex.position);
		double len = _length(planet_relative);
		_div(planet_relative,len,vertex.normal);

		//ShowMessage(_toString(vertex.position)+" n"+_toString(vertex.normal));
	}
	return vertex;
}

void			Sphere::render(float x, float y, float rx, float ry)
{
	glBegin(GL_QUADS);
		renderRecursive(x,y,rx,ry,trace(x,y),trace(x+rx,y),trace(x+rx,y+ry),trace(x,y+ry),0);
	glEnd();
}
						
void			Sphere::renderRecursive(float x, float y, float rx, float ry, const Vertex&v0, const Vertex&v1, const Vertex&v2, const Vertex&v3, unsigned depth)
{
	if (depth>3 && v0.isset && v1.isset && v2.isset && v3.isset)
	{
		glNormal3fv(v0.normal);
		glVertex3fv(v0.position);
		glNormal3fv(v1.normal);
		glVertex3fv(v1.position);
		glNormal3fv(v2.normal);
		glVertex3fv(v2.position);
		glNormal3fv(v3.normal);
		glVertex3fv(v3.position);
		return;
	}
	
	if (depth < 8)
	{
		Vertex	middle = trace(x+rx/2,y+ry/2),
				left = trace(x,y+ry/2),
				right = trace(x+rx,y+ry/2),
				top = trace(x+rx/2,y+ry),
				bottom = trace(x+rx/2,y);
		renderRecursive(x,y,rx/2,ry/2,v0,bottom,middle,left,depth+1);
		renderRecursive(x+rx/2,y,rx/2,ry/2,bottom,v1,right,middle,depth+1);
		renderRecursive(x+rx/2,y+ry/2,rx/2,ry/2,middle,right,v2,top,depth+1);
		renderRecursive(x,y+ry/2,rx/2,ry/2,left,middle,top,v3,depth+1);
	}
}
*/








SphereSector::SphereSector():display_list(0),abegin(0),aend(2*M_PI),rbegin(0),rend(M_PI)
{}

SphereSector::~SphereSector()
{
	if (display_list)
	{
		glDeleteLists(display_list,1);
	}
}

bool			SphereSector::isVisible(::Sphere*super, double distance)		const
{
	float dist = (abegin+aend)/2-super->view_x;
	if (dist > M_PI)
		dist -= 2*M_PI;
	if (dist < -M_PI)
		dist += 2*M_PI;
	if (M::vabs(dist)-(aend-abegin)/2 > super->x_fov)
		return false;
	
	
	dist = (rbegin+rend)/2-super->view_y;
	if (dist > 0)
	{
		if (M::vabs(dist)-(rend-rbegin)/2 > super->y_fov)
			return false;
	}
	else
	{
		float effective_fov = super->y_fov/((super->height-super->radius)/distance);
		if (M::vabs(dist)-(rend-rbegin)/2 > effective_fov)
			return false;
	
	}
	return true;
	
	


	M::TVec3<>	sphere_center;
	float	sphere_radius;
	M::TVec3<>	p0,
			p1,
			p2,
			p3,
			center;
	super->radialPoint(abegin,rbegin,distance,p0);
	super->radialPoint(aend,rbegin,distance,p1);
	super->radialPoint(aend,rend,distance,p2);
	super->radialPoint(abegin,rend,distance,p3);
	super->radialPoint((abegin+aend)/2,(rbegin+rend)/2,distance,sphere_center);
	sphere_radius = M::vsqrt(M::vmax(M::vmax(M::Vec::quadraticDistance(p0,sphere_center),M::Vec::quadraticDistance(p1,sphere_center)), M::vmax(M::Vec::quadraticDistance(p2,sphere_center),M::Vec::quadraticDistance(p3,sphere_center))));
	M::Mat::Transform(super->world_system,sphere_center,center);
	bool rs = super->frustum.IsVisible(center,sphere_radius);
	return rs;
	
}


static inline unsigned xres(double r, double range)
{
	ASSERT__(!isnan(range));
	unsigned result = (unsigned)(sin(r)/range *1e9*2);
	if (result < 2)
		result = 2;
	return result;
}

void		SphereSector::renderOutline(::Sphere*super, float r, float g, float b, double distance)
{
	glColor3f(r,g,b);
	glBegin(GL_LINE_LOOP);
		for (unsigned i = 0; i < 20; i++)
		{
			float fx = abegin+(float)((float)i/19.0f)*(aend-abegin);
			super->radialPoint(fx,rbegin,distance);
		}
		
		for (unsigned i = 0; i < 20; i++)
		{
			float fx = abegin+(1.0f-(float)((float)i/19.0f))*(aend-abegin);
			super->radialPoint(fx,rend,distance);
		}
	glEnd();
	
	glColor4f(r,g,b,0.3);
	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	glCallList(display_list);
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
}


void		SphereSector::build(double distance, ::Sphere*super, float resolution)
{
	//static const unsigned xres = 30;
	unsigned yres = M::vmax((unsigned)(10*resolution),2);
	double	r = rbegin,
			h = super->radius+distance,
			rstep = (rend-rbegin)/(double)yres,
			rad_res = 0.1;
	ASSERT_IS_CONSTRAINED__(rbegin,0,M_PI);
	ASSERT_IS_CONSTRAINED__(rend,0,M_PI);
	ASSERT_IS_CONSTRAINED__(rstep,0,M_PI);
	if (!display_list)
		display_list = glGenLists(1);
	if (!display_list)
		FATAL__("failed to generate display list");

	//cout << "begin constructing segment "<<abegin<<", "<<aend<<" / "<<rbegin<<", "<<rend<<endl;
	glNewList(display_list,GL_COMPILE);
		unsigned res0= (unsigned)M::vmax(M::Round(0.4*M_PI*r/rad_res*resolution),2);
		//xres(r,distance);
		for (unsigned strip = 0; strip < yres; strip++)
		{
			double r2 = r+rstep;
			{
				unsigned	
							res1 = (unsigned)M::vmax(M::Round(0.4*M_PI*r2/rad_res*resolution),2);
				glBegin(GL_TRIANGLE_STRIP);
					//float down_sample = (float)res1/res0;
					for (unsigned i = 0; i < res1; i++)
					{
						float	fx1 = abegin+(float)i/(res1-1)*(aend-abegin),
								fx0 = abegin+(float)M::Round((float)i*res0/res1)/(res0-1)*(aend-abegin);
						super->radialPoint(fx1,r2,distance);
						super->radialPoint(fx0,r,distance);
					}
					/*for (unsigned i = res0; i < res1; i++)
					{
						float	fx0 = abegin+(aend-abegin),
								fx1 = abegin+(float)i/(res1-1)*(aend-abegin);
						super->radialPoint(fx1,r2);
						super->radialPoint(fx0,r);
					}*/
				glEnd();
				res0 = res1;
			//ShowMessage(String(r)+" - "+String(r2));
			}

			r = r2;
		}

	glEndList();
}


SpherePhase::SpherePhase():dbegin(0),dend(1)
{}

void	SpherePhase::build(::Sphere*super, float resolution)
{
	double horizon = asin((double)super->radius/((double)super->radius+dend)),
			step = horizon*0.999/(double)radial_sectors;


	for (unsigned i = 0; i < angular_sectors; i++)
	{
		double radial = 0;
		for (unsigned j = 0; j < radial_sectors; j++)
		{
			//cout << "building radial segment "<<j<<" at distance "<<dend<<" ["<<radial*180/M_PI<<", "<<(radial+step)*180/M_PI<<"]/"<<horizon*180/M_PI<<endl;
			sector[i][j].rbegin = radial;
			radial += step;
			sector[i][j].rend = radial;
			sector[i][j].abegin = (double)i*2.0*M_PI/angular_sectors;
			sector[i][j].aend = (double)(i+1)*2.0*M_PI/angular_sectors;
			sector[i][j].build(dend /*(dbegin+dend)/2*/, super, resolution);
			if (!sector[i][j].display_list)
				FATAL__("sector "+String(i)+"."+String(j)+" failed to build");
		}
	}
}

void	SpherePhase::render(::Sphere*super)
{
	for (unsigned j = radial_sectors-1; j < radial_sectors; j--)
		for (unsigned i = 0; i < angular_sectors; i++)
			if (sector[i][j].isVisible(super,dend))
			{
				if (!sector[i][j].display_list)
				{
					ErrMessage("sector "+String(i)+"."+String(j)+" has not been properly built");
					exit(0);
				}
				glCallList(sector[i][j].display_list);
			}
}



void	SpherePhase::renderOutline(::Sphere*super, float r, float g, float b)
{
	for (unsigned j = radial_sectors-1; j < radial_sectors; j--)
		for (unsigned i = 0; i < angular_sectors; i++)
			if (sector[i][j].isVisible(super,dend))
				sector[i][j].renderOutline(super,r,g,b,dend);
}













void			::Sphere::checkDisplayLists(const String&domain)
{
	for (unsigned i = 0; i < ARRAYSIZE(phase); i++)
		for (unsigned j = 0; j < ARRAYSIZE(phase[i].sector); j++)
			for (unsigned k = 0; k < ARRAYSIZE(phase[i].sector[j]); k++)
				if (!phase[i].sector[j][k].display_list)
				{
					ErrMessage("display list in phase "+String(i)+"-"+String(j)+"."+String(k)+" is 0 ("+domain+" 0x"+IntToHex((int)(__int64)this,8)+")");
					exit(0);
				}

}

void			::Sphere::build(double root_step, float resolution)
{
	double distance = 0,
			dstep = root_step;
	for (unsigned i = 0; i < ARRAYSIZE(phase); i++)
	{
		phase[i].dbegin = distance;
		distance += dstep;
		phase[i].dend = distance;
		dstep *= 1.8;
		phase[i].build(this, resolution);
	}

	built = true;
}

void			::Sphere::buildFromRadius(double radius, float resolution)
{
	this->radius = radius;
	build(relative_height_step*radius, resolution);
}


void			::Sphere::renderCircular(unsigned min_level)
{
	//glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	
	double root_step = relative_height_step*radius;

	if (!built)
	{
		build(root_step,1);
	}


	int index = (unsigned)(log(M::vabs((height-radius)/root_step))/log(1.8));
	if (index < (int)min_level)
		index = (int)min_level;
	if (index >= ARRAYSIZE(phase))
		index = ARRAYSIZE(phase)-1;
	
	//glDisable(GL_CULL_FACE);
	//glDisable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glPushMatrix();
	glMultMatrixf(world_system.v);
		phase[index].render(this);
	glPopMatrix();
	//glEnable(GL_CULL_FACE);
	//glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	
}

void			::Sphere::renderOutline(float r, float g, float b)
{
	double root_step = relative_height_step*radius;
	
	if (!built)
	{
		build(root_step,1);
	}


	int index = (unsigned)(log(M::vabs((height-radius)/root_step))/log(1.8));
	if (index < 0)
		index = 0;
	if (index >= ARRAYSIZE(phase))
		index = ARRAYSIZE(phase)-1;
	
	glPushMatrix();
	glMultMatrixf(world_system.v);
		phase[index].renderOutline(this,r,g,b);
	glPopMatrix();
}


