#ifndef sphereH
#define sphereH


#include <engine/renderer/opengl.h>
#include <engine/display.h>
#include <io/log_stream.h>

using namespace Engine;

//extern Display<OpenGL>				display;	



struct TTracedVertex	//! Point describing a traced point on the water surface of the respective planet
{
	bool							isset;			//!< True if this point is defined, false otherwise. If @b isset is false then the following variables are undefined.
	M::TVec3<>							position,	//!< Surface positiion of the traced point relative to the point of view
									normal;		//!< Normalized surface normal of the traced point
	double							range;			//!< Distance of this point from the point of view
};

class Sphere;

class SphereSector
{
public:
		GLuint						display_list;
		double						abegin,aend,	//!< Angular segment
									rbegin,rend;	//!< Radial segment (as seen from observer, NOT sphere center)
		
									SphereSector();
virtual								~SphereSector();
		bool						isVisible(::Sphere*super, double distance)		const;
		void						build(double distance, ::Sphere*super, float resolution);
		void						renderOutline(::Sphere*super, float r, float g, float b, double distance);
};

class SpherePhase
{
public:
		static const unsigned		radial_sectors = 4,
									angular_sectors = 16;
		double						dbegin,dend;		//!< Distance segment
		SphereSector				sector[angular_sectors][radial_sectors];
		
									SpherePhase();
		void						build(::Sphere*super, float resolution);
		void						render(::Sphere*super);
		void						renderOutline(::Sphere*super, float r, float g, float b);
};

class Sphere
{
private:
public:
static const double		relative_height_step;	//!< Base factor to determine the root height step of. Multiplied with radius

		typedef TTracedVertex	Vertex;
		bool			built;
		float			x_fov,	//!< Half  x field of view (in radians) as derived from the applied frustum
						y_fov,	//!< Half  y field of view (in radians) as derived from the applied frustum
						view_x,	//!< Horizontal view angle (0-2PI, in radians) as derived from the applied frustum
						view_y;	//!< Vertical view angle (0-2PI, in radians) as derived from the applied frustum
		M::TMatrix3<>		world_matrix;
		M::TVec3<>			world_offset;
		M::TMatrix4<>		world_system;
		SpherePhase	phase[16];
		M::TVec3<double>	delta;	//!< Vector from sphere center to point of view
		double			height,	//!< Camera distance from sphere center (length of delta)
						radius;	//!< Sphere radius
		Frustum<>		frustum;

						Sphere();
						Sphere(const Frustum<>&frustum, double radius, double height, const M::TVec3<double>&delta);
						/**
							@brief Updates the configuration to use for succeeding renderCircular() or renderOutline() calls
							
							@param frustrum_ Visible frustum to use for visibility checks
							@param radius_ Sphere radius
							@param height_ Observer distance from the sphere center (equals length(delta_))
							@param delta_ Absolute vector pointing from the sphere center to the observer
						*/
		void			apply(const Frustum<>&frustrum_, double radius_, double height_,const M::TVec3<double>&delta_);
		void			checkDisplayLists(const String&domain);
					
					
		
		/**
			@brief Determines the coordinates and normal from a point's radial coordinates
			
			@param angular Transversal angle (in radians)
			@param radial Longitudinal angle (in radians as seen from the observer, NOT sphere center)
			@param dh Height delta
			@param position [out] position vector
			@param normal [out] normal vector
		*/
inline	void			radialPoint(float angular, float radial, double dh, M::TVec3<>&position, M::TVec3<>&normal)
						{
							double	ca = cos(radial),
									h = dh+radius,
									p = -2*ca*h,
									q = h*h-radius*radius,
									rs[2];
							BYTE num_rs = M::solveSqrEquation(p,q,rs);
							if (!num_rs)
							{
								std::cout << "No results for sqr("<<p<<", "<<q<<") with h="<<h<<", ca="<<ca<<", radial="<<radial*180/M_PI<<", dh="<<dh<<std::endl;
								FATAL__("equation fault");
							}
							double	x = M::smallestPositiveResult(rs,num_rs),
									v = h-ca*x,
									beta = acos(v/radius);
							if (x > h)
							{
								std::cout << "Faulty result x="<<x<<" for sqr("<<p<<", "<<q<<") with h="<<h<<", ca="<<ca<<", radial="<<radial*180/M_PI<<", dh="<<dh<<", num_res="<<(int)num_rs<<", rs={"<<rs[0]<<", "<<rs[1]<<"}"<<std::endl;
								FATAL__("equation fault");
							}
							
							//std::cout << radial << " => x="<<x<<" => v/radius="<<v/radius<<" => beta="<<beta<<std::endl;
							float	sinr = sin(beta),
									sina = sin(angular),
									cosa = cos(angular),
									cosr = cos(beta),
									r = sinr*radius,
									z = (1.0f-cosr)*radius;//+(height-radius);
							M::Vec::def(normal,cosa*sinr,sina*sinr,cosr);
							M::Vec::def(position,cosa*r,sina*r,-z);
						}

inline	void			radialPoint(float angular, float radial, double dh, M::TVec3<>&position)
						{
							radialPoint(angular,radial,dh,position,M::Vector3<>::dummy);
						}

inline	void			radialPoint(float angular, float radial, double dh)
						{
							M::TVec3<> p,n;
							radialPoint(angular,radial,dh,p,n);
							glNormal3fv(n.v);
							glVertex3fv(p.v);
						}
		
						/**
							@brief (Re)builds the local display lists
							
							This method is invoked once automatically by renderCircularSector() and renderOutline() if necessary.
							
							@param root_step Absolute delta to apply to the first height step. Scaled by 1.8 during each iteration
						*/
		void			build(double root_step, float resolution);
		void			buildFromRadius(double radius, float resolution);
						
						
						/**
							@brief Central render call
							
							Invoke apply() whenever the camera or other relevant information has changed before calling renderCircular().
							renderCircular() automatically invokes build() if not done before. This may generate noticable lag as the assembly of display lists takes time.
							
							The method does not link any shaders or textures. Only glNormal() and glVertex() calls are used, relative to the determined world system.
						*/
		void			renderCircular(unsigned min_level=0);
						/**
							@brief Sector outline renderer
							
							Renders sector borders using the currently applied configuration.
							renderOutline() automatically invokes build() if not done before. This may generate noticable lag as the assembly of display lists takes time.
							
							@param r Red line color component
							@param g Green line color component
							@param b Blue line color component
						*/
		void			renderOutline(float r = 1, float g = 1, float b = 1);
};




#endif
