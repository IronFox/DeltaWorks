#ifndef vector_mathTplH
#define vector_mathTplH

/******************************************************************

Collection of template vector-math-functions.

******************************************************************/


namespace DeltaWorks
{
	namespace Math
	{
	
		template <typename T>
			MF_CONSTRUCTOR			Sphere<T>::Sphere(const T&radius):radius(radius)
			{
				Vec::clear(center);
			}
		template <typename T>
			MF_CONSTRUCTOR			Sphere<T>::Sphere(const TVec3<T>&center, const T&radius):center(center),radius(radius)
			{}
		template <typename T>
			MF_DECLARE(bool)		Sphere<T>::Intersects(const Sphere<T>&remote)		const
			{
				return Vec::quadraticDistance(center,remote.center) < sqr(radius + remote.radius);
			}
		template <typename T>
			MFUNC(void)				Sphere<T>::IncludeSquare(const TVec3<C>&point)	
			{
				radius = vmax(radius,Vec::quadraticDistance(center,point));
			}

		template <typename T>
			MFUNC(void)				Sphere<T>::Include(const TVec3<C>&sphereCenter, const T&sphereRadius)
			{
				T dist2 = Vec::quadraticDistance(center, sphereCenter);

				if (radius >= sphereRadius && dist2 <= sqr(radius - sphereRadius))
					return;
				radius = vsqrt(dist2) + sphereRadius;
			}

		template <typename T>
			MFUNC(void)				Sphere<T>::Include(const TVec3<C>& point)	
			{
				if (radius < 0) 
					Vec::copy(point, center); 
				radius = vmax(radius, Vec::distance(center, point)); 
			}

		template <typename T>
			MFUNC(void)				Sphere<T>::Merge(const TVec3<C>& point, const T&radius)
			{
				if (this->radius < 0)
				{
					Vec::copy(point,center); 
					this->radius = radius;
				}
				else
				{
					TVec3<T>	d;
					Vec::sub(point,center,d);
					Vec::normalize0(d);
					TVec3<T>	a,b;
					Vec::mad(center,d,-this->radius,a);
					Vec::mad(point,d,radius,b);
					Vec::center(a,b,center);
					this->radius = Vec::distance(a,b)/(T)2;
				}
			}

		template <typename T>
			MFUNC(bool)				Sphere<T>::Contains(const TVec3<C>&point)	const
			{
				return Vec::quadraticDistance(center,point) <= sqr(radius);
			}

		
		template <typename T>
			MFUNC(bool)				Sphere<T>::Contains(const Sphere<C>&sphere)	const
			{
				return radius >= sphere.radius 
					&& Vec::quadraticDistance(center, sphere.center) <= sqr(radius-sphere.radius); 
			}
		
		template <typename T>
			MF_DECLARE(T)			Sphere<T>::volume()	const
			{
				return (T)4/(T)3 * (T)M_PI * radius * radius * radius;
			}

		template <typename T>
			MF_DECLARE(T)			Sphere<T>::GetVolume()		const	
			{
				return volume();
			}

		template <typename T>
			template <typename T0>
				MF_DECLARE(void)	Sphere<T>::GetVolume(T0&result)	const
				{
					result = (T0)((T)4/(T)3 * (T)M_PI * radius * radius * radius);
				}

		template <typename T>
			template <typename T0>
				void				Sphere<T>::GetCenter(TVec3<T0>&out)	const
				{
					Vec::copy(center, out);
				}


}
}



#endif
