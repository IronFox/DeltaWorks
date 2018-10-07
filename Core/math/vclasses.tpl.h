#ifndef vclassesTplH
#define vclassesTplH



    template <class C, count_t Len>MF_DECLARE (void) VecN<C,Len>::clear()
    {
        VecUnroll<Len>::clear(*this);
    }

    template <class C, count_t Len> MF_DECLARE(String) VecN<C,Len>::ToString()       const
    {
        return VecUnroll<Len>::toString(*this);
    }

    template <class C, count_t Len>MF_DECLARE (C) VecN<C,Len>::GetLength()                const
    {
        return VecUnroll<Len>::length(*this);
    }
	
    template <class C, count_t Len>MF_DECLARE (C) VecN<C,Len>::sqr()                const
    {
        return VecUnroll<Len>::dot(*this);
    }

    template <class C, count_t Len>MF_DECLARE (C) VecN<C,Len>::summary()               const
    {
        return VecUnroll<Len>::sum(*this);
    }

    template <class C, count_t Len>MF_INIT VecN<C,Len> MF_CC VecN<C,Len>::normalized()     const
    {
        VecN<C,Len> result;
        VecUnroll<Len>::copy(*this,result);
        VecUnroll<Len>::normalize(result);
        return result;
    }

    template <class C, count_t Len>
    MF_VNT MF_INIT VecN<C,Eval<Len,Len0>::Min> MF_CC VecN<C,Len>::operator+ (const TVec<C0,Len0>&other) const
    {
	    VecN<C,Eval<Len,Len0>::Min> result;
        VecUnroll<Eval<Len,Len0>::Min>::add(v,other.v,result.v);
		return result;
    }
    
    template <class C, count_t Len> MF_INIT VecN<C,Len> MF_CC VecN<C,Len>::operator+(const C&scalar) const
    {
        VecN<C,Len> result;
        VecUnroll<Len>::addVal(*this,scalar,result);
        return result;
    }
    
    template <class C, count_t Len>
    MF_VNT MF_DECLARE (void) VecN<C,Len>::operator+=(const TVec<C0,Len0>&other)
    {
		VecUnroll< Eval<Len,Len0>::Min >::add(v,other.v);
    }
    
    template <class C, count_t Len> MF_DECLARE (void) VecN<C,Len>::operator+=(const C&scalar)
    {
        VecUnroll<Len>::addVal(*this,scalar);
    }

    template <class C, count_t Len> MF_INIT VecN<C,Len> MF_CC VecN<C,Len>::operator* (const C&factor) const
    {
        VecN<C,Len> result;
        VecUnroll<Len>::multiply(*this,factor,result);
        return result;
    }
    
  //  template <class C, count_t Len>
  //  MF_VNT MF_DECLARE (C) VecN<C,Len>::operator*(const TVec<C0,Len0>&other) const
  //  {
		//return VecUnroll<Eval<Len,Len0>::Min>::dot(v,other.v);
  //  }
    
    template <class C, count_t Len>
    template <class C0> MF_DECLARE (void)                VecN<C,Len>::operator*=(const C0&factor)
    {
        VecUnroll<Len>::multiply(*this,factor);
    }
    
    template <class C, count_t Len>
    template <class C0> MF_INIT VecN<C,Len> MF_CC        VecN<C,Len>::operator/(const C0&factor) const
    {
        VecN<C,Len> result;
        VecUnroll<Len>::divide(*this,factor,result);
        return result;
    }
    
    template <class C, count_t Len>
    template <class C0> MF_DECLARE  (void)                VecN<C,Len>::operator/=(const C0&factor)
    {
        VecUnroll<Len>::divide(*this,factor);
    }


    template <class C, count_t Len>
    MF_VNT MF_INIT VecN<C,Eval<Len,Len0>::Min> MF_CC VecN<C,Len>::operator- (const TVec<C0,Len0>&other) const
    {
	    VecN<C,Eval<Len,Len0>::Min> result;
        VecUnroll<Eval<Len,Len0>::Min>::subtract(v,other.v,result.v);
		return result;
    }
    
    template <class C, count_t Len> MF_INIT VecN<C,Len> MF_CC VecN<C,Len>::operator-(const C&scalar) const
    {
        VecN<C,Len> result;
        VecUnroll<Len>::subtractValue(v,scalar,result.v);
        return result;
    }

    template <class C, count_t Len>    MF_INIT  VecN<C,Len> MF_CC   VecN<C,Len>::operator-()          const
    {
        VecN<C,Len>   result;
        VecUnroll<Len>::multiply(v,(C)-1,result.v);
        return result;
    }
    
    template <class C, count_t Len>
    MF_VNT MF_DECLARE (void) VecN<C,Len>::operator-=(const TVec<C0,Len0>&other)
    {
        VecUnroll<Eval<Len,Len0>::Min>::subtract(v,other.v);
    }
    
    template <class C, count_t Len> MF_DECLARE (void) VecN<C,Len>::operator-=(const C&scalar)
    {
        VecUnroll<Len>::subVal(*this,scalar);
    }

    template <class C, count_t Len>
    MF_VNT MF_INIT Vec3<C> MF_CC VecN<C,Len>::operator| (const TVec<C0,Len0>&other) const
    {
        Vec3<C>rs;
        Vec::cross(xyz,other.xyz,rs);
        return rs;
    }


    template <class C, count_t Len>
    MF_VNT MF_DECLARE (void) VecN<C,Len>::operator= (const TVec<C0,Len0>&other)
    {
		VecUnroll<Eval<Len,Len0>::Min>::copy(other.v,v);
    }

    template <class C, count_t Len>
    template <class C0> MF_DECLARE (void) VecN<C,Len>::operator= (const TVec2<C0>&other)
    {
        VecUnroll<Eval<Len,2>::Min>::copy(other.v,v);
    }


    template <class C, count_t Len>
    template <class C0> MF_DECLARE (void) VecN<C,Len>::operator= (const TVec3<C0>&other)
    {
        VecUnroll<Eval<Len,3>::Min>::copy(other.v,v);
    }

    template <class C, count_t Len>
    template <class C0> MF_DECLARE (void) VecN<C,Len>::operator= (const TVec4<C0>&other)
    {
        VecUnroll<Eval<Len,4>::Min>::copy(other.v,v);
    }

    template <class C, count_t Len>
    MF_VNT MF_DECLARE (bool) VecN<C,Len>::operator==(const TVec<C0,Len0>&other) const
    {
		return VecUnroll<Eval<Len,Len0>::Min>::equal(v,other.v);
		//return VecUnroll<Eval<Len,Len0>::Min>::Similar(v,other.v,GetError<C>());
    }

    template <class C, count_t Len>
    MF_VNT MF_DECLARE (bool) VecN<C,Len>::operator!=(const TVec<C0,Len0>&other) const
    {
		return !operator==(other);
    }

    template <class C, count_t Len>
    MFUNC1 (bool) VecN<C,Len>::operator> (const TVec<C0,Len>&other) const
    {
		return VecUnroll<Len>::compare(v,other.v,GetError<C>())>0;
    }

    template <class C, count_t Len>
    MFUNC1 (bool) VecN<C,Len>::operator< (const TVec<C0,Len>&other) const
    {
		return VecUnroll<Len>::compare(v,other.v,GetError<C>())<0;
    }

	template <class C, count_t Len>
	MFUNC1 (char) VecN<C,Len>::CompareTo (const TVec<C0,Len>&other) const
	{
		return VecUnroll<Len>::compare(v,other.v,GetError<C>());
	}

/*
    template <class C, count_t Len>
    template <class C0, count_t Len0> MFMODS (C)    VecN<C,Len>::operator* (const TVec<C0,Len0>&other) const
    {
        if (Len < Len0)
            return _dotV<C,C0,Len>(v,other.v);
        return _dotV<C,C0,Len0>(v,other.v);
    }
*/
	template <class C, count_t Len> MF_DECLARE(typename TypeInfo<C>::UnionCompatibleBase &) VecN<C, Len>::operator[](index_t component)
    {
        return v[component];
    }
    
	template <class C, count_t Len> MF_DECLARE(const typename TypeInfo<C>::UnionCompatibleBase&) VecN<C, Len>::operator[](index_t component) const
    {
        return v[component];
    }


    // ----------------     vec3     ------------------------------------------------------------------------------




    template <class C>  MF_CONSTRUCTOR          Vec3<C>::Vec3()
	{
		x = 0;
		y = 0;
		z = 0;
	}

    template <class C> 
		MF_CONSTRUCTOR          Vec3<C>::Vec3(const C&value)
		{
			x = value;
			y = value;
			z = value;
		}

	template <class C>
		MFUNC1 (C)				Vec3<C>::operator()(const C0&x)					const
		{
			return x*x*this->x + x*this->y + this->z;
		}




    template <class C>
    MF_CONSTRUCTOR1          Vec3<C>::Vec3(const TVec3<C0>&other)
	{
		x = (C)other.x;
		y = (C)other.y;
		z = (C)other.z;
	}


    template <class C>
    MF_CONSTRUCTOR2          Vec3<C>::Vec3(const TVec2<C0>&other,const C1&z_)
	{
		x = (C)other.x;
		y = (C)other.y;
		z = z_;
	}





    template <class C>
    MF_CONSTRUCTOR1          Vec3<C>::Vec3(const C0 field[3])
	{
		x = (C)field[0];
		y = (C)field[1];
		z = (C)field[2];
	}




    template <class C>
    MF_CONSTRUCTOR3 Vec3<C>::Vec3(const C0&x_, const C1&y_, const C2&z_)
	{
		x = (C)x_;
		y = (C)y_;
		z = (C)z_;
	}




	template <class C>
	MFUNC1 (Vec3<C>)		Vec3<C>::operator&(const TVec3<C0>&other)		const
	{
		return Vec3<C>(x * other.x, y * other.y, z * other.z);
	}

	template <class C>
	MFUNC1 (void)		Vec3<C>::operator&=(const TVec3<C0>&other)
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;
	}


    template <class C>  MF_DECLARE  (Vec3<C>)    Vec3<C>::normalized()                const
    {
        C   len = Vec::length(*this);
        return Vec3<C>(x/len,y/len,z/len);
    }

    template <class C>  MF_DECLARE  (Vec3<C>)    Vec3<C>::normalized0()                const
    {
        C   len = Vec::length(*this);
		using std::fabs;
        if (fabs(len) > TypeInfo<C>::error)
            return Vec3<C>(x/len,y/len,z/len);
        return Vec3<C>(1,0,0);
    }
    
    
    template <class C>
    template <class C0> MF_DECLARE  (Vec3<C>)   Vec3<C>::operator+(const TVec3<C0>&other)    const
    {
        return Vec3<C>(x+other.x,y+other.y,z+other.z);
    }

    template <class C>  MF_DECLARE  (Vec3<C>)   Vec3<C>::operator+(const C&value)             const
    {
        return Vec3<C>(x+value,y+value,z+value);
    }

    template <class C>
    template <class C0> MF_DECLARE  (void)        Vec3<C>::operator+=(const TVec3<C0>&other)
    {
        x+=other.x;
        y+=other.y;
        z+=other.z;
    }

    template <class C>  MF_DECLARE  (void)        Vec3<C>::operator+=(const C&value)
    {
        x+=value;
        y+=value;
        z+=value;
    }

    //template <class C>
    //template <class C0> MF_DECLARE  (C)           Vec3<C>::operator*(const TVec3<C0>&other)    const
    //{
    //    return x*other.x + y*other.y + z*other.z;
    //}

    template <class C>  MF_DECLARE  (Vec3<C>)   Vec3<C>::operator*(const C&factor)            const
    {
        return Vec3<C>(x*factor,y*factor,z*factor);
    }

    template <class C>  MF_DECLARE  (void)        Vec3<C>::operator*=(const C&factor)
    {
        x*=factor;
        y*=factor;
        z*=factor;
    }

    template <class C>
    template <class C0> MF_DECLARE  (Vec3<C>)   Vec3<C>::operator/(const C0&factor)           const
    {
        return Vec3<C>(x/factor,y/factor,z/factor);
    }

    template <class C>
    template <class C0> MF_DECLARE  (void)        Vec3<C>::operator/=(const C0&factor)
    {
        x/=factor;
        y/=factor;
        z/=factor;
    }


    template <class C>
    template <class C0> MF_DECLARE  (Vec3<C>)   Vec3<C>::operator-(const TVec3<C0>&other)    const
    {
        return Vec3<C>(x-other.x,y-other.y,z-other.z);
    }

    template <class C>  MF_DECLARE  (Vec3<C>)   Vec3<C>::operator-(const C&value)             const
    {
        return Vec3<C>(x-value,y-value,z-value);
    }

    template <class C>
    template <class C0> MF_DECLARE  (void)        Vec3<C>::operator-=(const TVec3<C0>&other)
    {
        x-=other.x;
        y-=other.y;
        z-=other.z;
    }

    template <class C>  MF_DECLARE  (void)        Vec3<C>::operator-=(const C&value)
    {
        x-=value;
        y-=value;
        z-=value;
    }

    template <class C>  MF_DECLARE  (Vec3<C>)   Vec3<C>::operator-()                          const
    {
        return Vec3<C>(-x,-y,-z);
    }

    template <class C>
    template <class C0> MF_DECLARE  (Vec3<C>)   Vec3<C>::operator|(const TVec3<C0>&other)    const
    {
        return Vec3<C>(	y*other.z - z*other.y,
                            z*other.x - x*other.z,
                            x*other.y - y*other.x);
    }

    template <class C>
    template <class C0> MF_DECLARE  (void)        Vec3<C>::operator=(const TVec3<C0>&other)
    {
		x = (C)other.x;
		y = (C)other.y;
		z = (C)other.z;
    }

    template <class C>
    template <class C0> MF_DECLARE  (bool)        Vec3<C>::operator==(const TVec3<C0>&other)   const
    {
		return Vec::equal(*this,other);
        //return Math::sqr(x-other.x)+Math::sqr(y-other.y)+Math::sqr(z-other.z) <= GetError<C>()*GetError<C0>();
    }

    template <class C>
    template <class C0> MF_DECLARE  (bool)        Vec3<C>::operator!=(const TVec3<C0>&other)   const
    {
		return !operator==(other);
    }
	
    template <class C>
    MFUNC1  (bool)        Vec3<C>::operator>(const TVec3<C0>&other)    const
    {
        return Vec::compare(*this,other) > 0;
    }

    template <class C>
    MFUNC1  (bool)        Vec3<C>::operator<(const TVec3<C0>&other)    const
    {
		return Vec::compare(*this,other) < 0;
    }


	template <class C>  MF_DECLARE(typename Vec3<C>::Type&)          Vec3<C>::operator[](index_t component)
    {
        return v[component];
    }
    
	template <class C>  MF_DECLARE(const typename Vec3<C>::Type&)          Vec3<C>::operator[](index_t component) const
    {
        return v[component];
    }

    

    // ----------------     vec2     ------------------------------------------------------------------------------


    template <class C>  MF_CONSTRUCTOR              Vec2<C>::Vec2()
	{
		x = 0;
		y = 0;
	}


    template <class C>  MF_CONSTRUCTOR              Vec2<C>::Vec2(const C&value)
	{
		x = value;
		y = value;
	}


	template <class C>
    MF_CONSTRUCTOR1              Vec2<C>::Vec2(const TVec2<C0>&other)
	{
		x = (C)other.x;
		y = (C)other.y;
	}

    
    template <class C>
    MF_CONSTRUCTOR1              Vec2<C>::Vec2(const C0 field[2])
	{
		x = (C)field[0];
		y = (C)field[1];
	}

	template <class C>
    MF_CONSTRUCTOR1              Vec2<C>::Vec2(const std::initializer_list<C0>&field)
	{
		DBG_ASSERT__(field.size() == 2);
		x = (C)*field.begin();
		y = (C)*(field.begin()+1);
	}


    template <class C>
    MF_CONSTRUCTOR2     Vec2<C>::Vec2(const C0&x_, const C1&y_)
	{
		x = (C)x_;
		y = (C)y_;
	}


	template <class C>
	MFUNC1 (C)			Vec2<C>::operator()(const C0&x)					const
	{
		return this->x*x + y;
	}



    template <class C>  MF_DECLARE (Vec2<C>) Vec2<C>::normalized()                            const
    {
		return Vec2<C>(*this).normalize();
    }

	template <class C>  MF_DECLARE(Vec2<C>)			Vec2<C>::normalized0()							const
	{
		return Vec2<C>(*this).normalize0();
	}


    template <class C>
    MFUNC1  (Vec2<C>)   Vec2<C>::operator+(const TVec2<C0>&other)    const
    {
        return Vec2<C>(x+other.x,y+other.y);
    }

    template <class C>  MF_DECLARE  (Vec2<C>)   Vec2<C>::operator+(const C&value)             const
    {
        return Vec2<C>(x+value,y+value);
    }

    template <class C>
    MFUNC1  (void)        Vec2<C>::operator+=(const TVec2<C0>&other)
    {
        x+=other.x;
        y+=other.y;
    }

    template <class C>  MF_DECLARE  (void)        Vec2<C>::operator+=(const C&value)
    {
        x+=value;
        y+=value;
    }


    //template <class C>
    //MFUNC1  (C)           Vec2<C>::operator*(const TVec2<C0>&other)    const
    //{
    //    return x*other.x+y*other.y;
    //}

    template <class C>  MF_DECLARE  (Vec2<C>)   Vec2<C>::operator*(const C&factor)            const
    {
        return Vec2<C>(x*factor,y*factor);
    }

    template <class C>  MF_DECLARE  (void)        Vec2<C>::operator*=(const C&factor)
    {
        x*=factor;
        y*=factor;
    }

    template <class C>
    MFUNC1  (Vec2<C>)   Vec2<C>::operator/(const C0&factor)           const
    {
        return Vec2<C>(x/factor,y/factor);
    }

    template <class C>
    MFUNC1  (void)        Vec2<C>::operator/=(const C0&factor)
    {
        x/=factor;
        y/=factor;
    }

    template <class C>
    MFUNC1  (Vec2<C>)   Vec2<C>::operator-(const TVec2<C0>&other)    const
    {
        return Vec2<C>(x-other.x,y-other.y);
    }

    template <class C>  MF_DECLARE  (Vec2<C>)   Vec2<C>::operator-()                          const
    {
        return Vec2<C>(-x,-y);
    }

    template <class C>  MF_DECLARE  (Vec2<C>)   Vec2<C>::operator-(const C&value)             const
    {
        return Vec2<C>(x-value,y-value);
    }

    template <class C>
    MFUNC1  (void)        Vec2<C>::operator-=(const TVec2<C0>&other)
    {
        x-=other.x;
        y-=other.y;
    }

    template <class C>  MF_DECLARE  (void)        Vec2<C>::operator-=(const C&value)
    {
        x-=value;
        y-=value;
    }


    template <class C>
    MFUNC1  (void)        Vec2<C>::operator=(const TVec2<C0>&other)
    {
        Vec::copy(other,*this);
    }

    template <class C>
    MFUNC1  (bool)        Vec2<C>::operator==(const TVec2<C0>&other)   const
    {
		return Vec::equal(*this,other);
//        return Math::sqr(x-other.x)+Math::sqr(y-other.y) <= GetError<C>() * GetError<C0>();
    }

    template <class C>
    MFUNC1  (bool)        Vec2<C>::operator!=(const TVec2<C0>&other)   const
    {
		return !operator==(other);
    }

    template <class C>
    MFUNC1  (bool)        Vec2<C>::operator>(const TVec2<C0>&other)    const
    {
        return Vec::compare(*this,other)>0;
    }

    template <class C>
    MFUNC1  (bool)        Vec2<C>::operator<(const TVec2<C0>&other)    const
    {
		return Vec::compare(*this,other)<0;
    }



    template <class C>
    MFUNC1  (Vec2<C>)            Vec2<C>::operator&(const TVec2<C0>&other)        const
	{
		return Vec2<C>(x*other.x,y*other.y);
	}

	template <class C>
	MFUNC1 (void)		Vec2<C>::operator&=(const TVec2<C0>&other)
	{
		x *= other.x;
		y *= other.y;
	}


	template <class C>  MF_DECLARE(typename Vec2<C>::Type&)          Vec2<C>::operator[](index_t component)
    {
        return v[component];
    }

	template <class C>  MF_DECLARE(const typename Vec2<C>::Type&)          Vec2<C>::operator[](index_t component) const
    {
        return v[component];
    }



    // ----------------     vec4     ------------------------------------------------------------------------------



    template <class C>  MF_CONSTRUCTOR              Vec4<C>::Vec4()
    {
		x = 0;
		y = 0;
		z = 0;
		w = 0;
	}

    template <class C>  MF_CONSTRUCTOR              Vec4<C>::Vec4(const C&value)
	{
		x = value;
		y = value;
		z = value;
		w = value;
	}

    template <class C>
    MF_CONSTRUCTOR3              Vec4<C>::Vec4(const TVec2<C0>&other,const C1&z_, const C2&w_)
	{
		x = (C)other.x;
		y = (C)other.y;
		z = (C)z_;
		w = (C)w_;
	}

	template <class C>
	MF_CONSTRUCTOR3              Vec4<C>::Vec4(const C0&x_, const C1&y_, const TVec2<C2>&zw)
	{
		x = (C)x_;
		y = (C)y_;
		z = (C)zw.x;
		w = (C)zw.y;
	}

	template <class C>
	MF_CONSTRUCTOR2              Vec4<C>::Vec4(const C0&x_, const TVec3<C1>&yzw)
	{
		x = (C)x_;
		y = (C)yzw.x;
		z = (C)yzw.y;
		w = (C)yzw.z;
	}

    template <class C>
    MF_CONSTRUCTOR2              Vec4<C>::Vec4(const TVec2<C0>&xy,const TVec2<C1>&zw)
	{
		x = (C)xy.x;
		y = (C)xy.y;
		z = (C)zw.x;
		w = (C)zw.y;
	}
	
    template <class C>
    MF_CONSTRUCTOR2              Vec4<C>::Vec4(const TVec3<C0>&other, const C1&w_)
	{
		x = (C)other.x;
		y = (C)other.y;
		z = (C)other.z;
		w = (C)w_;
	}
	
    template <class C>
    MF_CONSTRUCTOR1              Vec4<C>::Vec4(const TVec4<C0>&other)
	{
		x = (C)other.x;
		y = (C)other.y;
		z = (C)other.z;
		w = (C)other.w;
	}
    
    template <class C>
    MF_CONSTRUCTOR1              Vec4<C>::Vec4(const C0 field[4])
	{
		x = (C)field[0];
		y = (C)field[1];
		z = (C)field[2];
		w = (C)field[3];
	}




    template <class C>
    MF_CONSTRUCTOR4 Vec4<C>::Vec4(const C0&x_, const C1&y_, const C2&z_, const C3&w_)
	{
		x = (C)x_;
		y = (C)y_;
		z = (C)z_;
		w = (C)w_;
	}





    template <class C>  MF_DECLARE  (Vec4<C>)    Vec4<C>::normalized()                            const
    {
        C   len = Vec::length(*this);
        return Vec4<C>(x/len,y/len,z/len,w/len);
    }

    template <class C>
    MFUNC1  (Vec4<C>)   Vec4<C>::operator+(const TVec4<C0>&other)    const
    {
        return Vec4<C>(x+other.x,y+other.y,z+other.z,w+other.w);
    }

    template <class C>  MF_DECLARE  (Vec4<C>)   Vec4<C>::operator+(const C&value)             const
    {
        return Vec4<C>(x+value,y+value,z+value,w+value);
    }



	template <class C>
	MFUNC1 (Vec4<C>)		Vec4<C>::operator&(const TVec4<C0>&other)		const
	{
		return Vec4<C>(x * other.x, y * other.y, z * other.z, w * other.w);
	}

	template <class C>
	MFUNC1 (void)		Vec4<C>::operator&=(const TVec4<C0>&other)
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;
		w *= other.w;
	}



    template <class C>
    MFUNC1  (void)        Vec4<C>::operator+=(const TVec4<C0>&other)
    {
        x+=other.x;
        y+=other.y;
        z+=other.z;
        w+=other.w;
    }

    template <class C>  MF_DECLARE  (void)        Vec4<C>::operator+=(const C&value)
    {
        x+=value;
        y+=value;
        z+=value;
        w+=value;
    }


    //template <class C>
    //MFUNC1  (C)           Vec4<C>::operator*(const TVec4<C0>&other)    const
    //{
    //    return x*other.x + y*other.y + z*other.z + w*other.w;
    //}

    template <class C>  MF_DECLARE  (Vec4<C>)   Vec4<C>::operator*(const C&factor)            const
    {
        return Vec4<C>(x*factor,y*factor,z*factor,w*factor);
    }

    template <class C>  MF_DECLARE  (void)        Vec4<C>::operator*=(const C&factor)
    {
        x*=factor;
        y*=factor;
        z*=factor;
        w*=factor;
    }

    template <class C>
    MFUNC1  (Vec4<C>)   Vec4<C>::operator/(const C0&factor)           const
    {
        return Vec4<C>(x/factor,y/factor,z/factor,w/factor);
    }

    template <class C>
    MFUNC1  (void)        Vec4<C>::operator/=(const C0&factor)
    {
        x/=factor;
        y/=factor;
        z/=factor;
        w/=factor;
    }


    template <class C>
    MFUNC1  (Vec4<C>)   Vec4<C>::operator-(const TVec4<C0>&other)    const
    {
        return Vec4<C>(x-other.x,y-other.y,z-other.z,w-other.w);
    }

    template <class C>  MF_DECLARE  (Vec4<C>)   Vec4<C>::operator-()                          const
    {
        return Vec4<C>(-x,-y,-z,-w);
    }

    template <class C>  MF_DECLARE  (Vec4<C>)   Vec4<C>::operator-(const C&value)             const
    {
        return Vec4<C>(x-value,y-value,z-value,w-value);
    }

    template <class C>
    MFUNC1  (void)        Vec4<C>::operator-=(const TVec4<C0>&other)
    {
        x-=other.x;
        y-=other.y;
        z-=other.z;
        w-=other.w;
    }

    template <class C>  MF_DECLARE  (void)        Vec4<C>::operator-=(const C&value)
    {
        x-=value;
        y-=value;
        z-=value;
        w-=value;
    }


    template <class C>
    MFUNC1  (Vec3<C>)   Vec4<C>::operator|(const TVec4<C0>&other)    const
    {
        return Vec3<C>(   y*other.z - z*other.y,
                            z*other.x - x*other.z,
                            x*other.y - y*other.x);
    }

    template <class C>
    MFUNC1  (void)        Vec4<C>::operator=(const TVec2<C0>&other)
    {
        Vec::copy(other,xy);
    }

    template <class C>
    MFUNC1  (void)        Vec4<C>::operator=(const TVec3<C0>&other)
    {
        Vec::copy(other,xyz);
    }
    template <class C>
    MFUNC1  (void)        Vec4<C>::operator=(const TVec4<C0>&other)
    {
        Vec::copy(other,*this);
    }

    template <class C>
    MFUNC1  (bool)        Vec4<C>::operator==(const TVec4<C0>&other)   const
    {
		return Vec::equal(*this,other);
        //return Math::sqr(x-other.x)+Math::sqr(y-other.y)+Math::sqr(z-other.z)+Math::sqr(w-other.w) <= GetError<C>()*GetError<C0>();
    }

    template <class C>
    MFUNC1  (bool)        Vec4<C>::operator!=(const TVec4<C0>&other)   const
    {
		return !operator==(other);
    }

    template <class C>
    MFUNC1  (bool)        Vec4<C>::operator>(const TVec4<C0>&other)    const
    {
		return Vec::compare(*this,other)>0;
    }

    template <class C>
    MFUNC1  (bool)        Vec4<C>::operator<(const TVec4<C0>&other)    const
    {
		return Vec::compare(*this,other)<0;
    }



	template <class C>  MF_DECLARE(typename Vec4<C>::Type&)          Vec4<C>::operator[](index_t component)
    {
        return v[component];
    }

	template <class C>  MF_DECLARE(const typename Vec4<C>::Type&)          Vec4<C>::operator[](index_t component) const
    {
        return v[component];
    }
	
	template <class C>  MF_DECLARE			(Vec3<C>)			Vec4<C>::vector(BYTE c0, BYTE c1, BYTE c2)		const
	{
		return Vec3<C>(v[c0],v[c1],v[c2]);
	}
	
	template <class C>  MF_DECLARE			(Vec2<C>)			Vec4<C>::vector(BYTE c0, BYTE c1)				const
	{
		return Vec2<C>(v[c0],v[c1]);
	}
	



    // ----------------     line     ------------------------------------------------------------------------------




    template <class C> MF_CONSTRUCTOR Line<C>::Line()
    {}

    template <class C> MF_CONSTRUCTOR Line<C>::Line(const Line<C>&line):position(line.position),direction(line.direction)
    {}
    
    template <class C> MF_CONSTRUCTOR2 Line<C>::Line(const TVec3<C0>&p, const TVec3<C1>&d):position(p),direction(d)
    {}

    template <class C> MF_CONSTRUCTOR1 Line<C>::Line(const Line<C0>&line):position(line.position),direction(line.direction)
    {}
    
    template <class C> MF_DECLARE(String)             Line<C>::ToString()
    {
        return position.ToString()+" -> "+direction.ToString();
    }
    
    template <class C> MFUNC1 (Vec3<C>) Line<C>::At(const C0&scalar) const
    {
        return Vec3<C>(position.x + direction.x * scalar,
                        position.y + direction.y * scalar,
                        position.z + direction.z * scalar);
    }
    template <class C> MFUNC1 (Vec3<C>) Line<C>::operator()(const C0&scalar) const
    {
        return Vec3<C>(position.x + direction.x * scalar,
                        position.y + direction.y * scalar,
                        position.z + direction.z * scalar);
    }

    template <class C> MFUNC1  (void) Line<C>::operator+=(const Line<C0>&line)
    {
        Vec::add(position,line.position);
        Vec::add(direction,line.direction);
    }

    template <class C> MFUNC1  (void) Line<C>::operator-=(const Line<C0>&line)
    {
        Vec::subtract(position,line.position);
        Vec::subtract(direction,line.direction);
    }

    template <class C> MFUNC1  (void) Line<C>::operator*=(const C0&scalar)
    {
//        _multiply(position.v,scalar); //???
        Vec::multiply(direction,scalar);
    }

    template <class C> MFUNC1  (void) Line<C>::operator/=(const C0&scalar)
    {
//        _divide(position.v,scalar); //???
        Vec::divide(direction,scalar);
    }

    template <class C> MFUNC1  (void) Line<C>::operator=(const Line<C0>&other)
    {
        Vec::copy(other.position,position);
        Vec::copy(other.direction,direction);
    }

    template <class C> MFUNC1 (Line<C>) Line<C>::operator+(const Line<C0>&f)  const
    {
        Line<C>  result;
        Vec::add(position,f.position,result.position);
        Vec::add(direction,f.direction,result.direction);
        return result;
    }

    template <class C> MFUNC1 (Line<C>) Line<C>::operator-(const Line<C0>&f)  const
    {
        Line<C>  result;
        Vec::subtract(position,f.position,result.position);
        Vec::subtract(direction,f.direction,result.direction);
        return result;
    }

    template <class C> MFUNC1 (Line<C>) Line<C>::operator*(const C0&scalar)       const
    {
        Line<C>  result;
//        _multiply(position.v,scalar,result.position.v); //???
        result.position = position;
        Vec::multiply(direction,scalar,result.direction);
        return result;
    }

    template <class C> MFUNC1 (Line<C>) Line<C>::operator/(const C0&scalar)      const
    {
        Line<C>  result;
//        _divide(position.v,scalar,result.position.v); //???
        result.position = position;
        Vec::divide(direction,scalar,result.direction);
        return result;
    }

    template <class C> MF_DECLARE (Line<C>) Line<C>::operator-()                    const
    {
        return Line<C>(position,-direction);
    }







#endif
