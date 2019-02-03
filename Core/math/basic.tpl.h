#ifndef basic_mathTplH
#define basic_mathTplH

/******************************************************************

Collection of basic template functions.

******************************************************************/


namespace Math
{



	namespace ByReference
	{

		MFUNC	(C)			frac(const C&x)
		{
			return x - floor(x);
		}

	
		MFUNC	(C)		gaussian(const C&t)
		{
			if (t <= -2)
				return 0;
			if (t <= -1)
				return 1.0/6*cubic(2+t);
			if (t <= 0)
				return 1.0/6*(4-6*t*t-3*t*t*t);
			if (t <= 1)
				return 1.0/6*(4-6*t*t+3*t*t*t);
			if (t <= 2)
				return 1.0/6*cubic(2-t);
			return 0;
		}
		
		MFUNC (bool)		nearingZero(const C&value)
		{
			return vabs(value)<=GetError<C>();
		}
		
		MFUNC (bool)		nearingOne(const C&value)
		{
			return vabs(value-1) <= GetError<C>();
		}

		MFUNC2 (bool)		Similar(const C0&v0, const C1&v1)
		{
			return vabs(v0-v1) <= GetError<C0>();
		}
		
		MFUNC3 (bool)		Similar(const C0&v0, const C1&v1, const C2&tolerance)
		{
			return vabs(v0-v1) <= tolerance;
		}
		

		MFUNC	(C)	rnd(const C&f)
		{
			C a = (C)floor(f);
			if (a+(C)0.5 < f)
				a++;
			return a;
		}
	
		MFUNC3 (C0)          clamped(const C0&v, const C1&min, const C2&max)
		{
			if (v < (C0)min)
				return (C0)min;
			if (v > (C0)max)
				return (C0)max;
			return v;
		}
		MFUNC3 (C0)          clamp(const C0&v, const C1&min, const C2&max)
		{
			if (v < (C0)min)
				return (C0)min;
			if (v > (C0)max)
				return (C0)max;
			return v;
		}
		MFUNC (C)          clamp01(const C&v)
		{
			if (v < (C)0)
				return (C)0;
			if (v > (C)1)
				return (C)1;
			return v;
		}
		MFUNC  (int)        Sign(const C&f)
		{
			if (f < -GetError<C>()/100)
				return -1;
			if (f <= GetError<C>()/100)
				return 0;
			return 1;
		}
		MFUNC3 (C0)           normalDistribution(const C0&x, const C1&mean, const C2&deviation)
		{
			return (C0)1.0/(deviation*(C0)2.506628274631000502415765284811)*vexp(-sqr(x-mean)/((C0)2.0*deviation*deviation));
		}
		MFUNC2 (C0)        aligned(const C0&i, const C1&step)
		{
			if (i%step)
				return i+step-i%step;
			return i;
		}
		MFUNC (C)       vabs(const C&v)
		{
			return fabs(v);
		}


		MF_SPECIALIZED (int)       vabs<int>(const int&v)
		{
			#if SYSTEM==WINDOWS
				return abs(v);
			#else
				return v>0?v:-v;
			#endif
		}
			
		MF_SPECIALIZED (long int)   vabs<long int>(const long int&v)
		{
			#if SYSTEM==WINDOWS
				return labs(v);
			#else
				return v>0?v:-v;
			#endif
		}
		
		MF_SPECIALIZED (long double)  vabs<long double>(const long double&v)
		{
			return fabsl(v);
		}

		MF_SPECIALIZED(half)  vabs<half>(const half&v)
		{
			return v.Abs();
		}

		MFUNC (C)             vsqrt(const C&v)
		{
			return sqrt(v);
		}
			
		MF_SPECIALIZED (long double)   vsqrt<long double>(const long double&v)
		{
			return sqrtl(v);
		}
			
		MF_SPECIALIZED (int)   vsqrt<int>(const int&v)
		{
			return (int)sqrt((float)v);
		}
		
		MFUNC (C)             vlog(const C&v)
		{
			return log(v);
		}
			
		MF_SPECIALIZED (long double)   vlog<long double>(const long double&v)
		{
			return logl(v);
		}
		
		MFUNC2 (C0)             vpow(const C0&b,const C1&e)
		{
			return pow(b,(C0)e);
		}
	
		MF_SPECIALIZED (long double)   vpow<long double, long double>(const long double&b,const long double&e)
		{
			return powl(b,e);
		}
		
		MFUNC (C)             vexp(const C&e)
		{
			return exp(e);
		}
		
		MF_SPECIALIZED (long double)  vexp<long double>(const long double&e)
		{
			return expl(e);
		}
		
			
		
		MFUNC (C)             vlog10(const C&v)
		{
			return log10(v);
		}
		
		MF_SPECIALIZED (long double)   vlog10<long double>(const long double&v)
		{
			return log10l(v);
		}
		

		MFUNC (C)        vatan(const C&x)
		{
			return atan(x);
		}
			

		MF_SPECIALIZED (long double)        vatan<long double>(const long double&x)
		{
			return atanl(x);
		}
		
		MFUNC2 (C0)        vatan2(const C0&y, const C1&x)
		{
			return atan2(y,(C0)x);
		}
		
		MF_SPECIALIZED (long double)        vatan2<long double, long double>(const long double&y, const long double&x)
		{
			return atan2l(y,x);
		}
		

		MFUNC (C)        vsin(const C&v)
		{
			return sin(v);
		}
			
		MF_SPECIALIZED (long double)    vsin<long double>(const long double&v)
		{
			return sinl(v);
		}

		MFUNC (C)        vcos(const C&v)
		{
			return cos(v);
		}
		
		MF_SPECIALIZED (long double)  vcos<long double>(const long double&v)
		{
			return cosl(v);
		}

		MFUNC (C)        vasin(const C&v)
		{
			return asin(v);
		}

		MF_SPECIALIZED (long double) vasin<long double>(const long double&v)
		{
			return asinl(v);
		}
		
		MFUNC (C)        vacos(const C&v)
		{
			return acos(v);
		}
		
		MF_SPECIALIZED (long double) vacos<long double>(const long double&v)
		{
			return acosl(v);
		}


		MFUNC (C)        vtan(const C&v)
		{
			return tan(v);
		}
		
		MF_SPECIALIZED (long double)     vtan<long double>(const long double&v)
		{
			return tanl(v);
		}


		MFUNC2 (C0)    vmin(const C0&value0, const C1&value1)
		{
			return value0<(C0)value1?value0:(C0)value1;
		}

		MFUNC2 (C0)    vmax(const C0&value0, const C1&value1)
		{
			return value0>(C0)value1?value0:(C0)value1;
		}
	
		MFUNC3 (C0)    vmin(const C0&value0, const C1&value1, const C2&value2)
		{
			if (value0 < value1)
			{
				if (value0 < value2)
					return value0;
				return value2;
			}
			if (value1 < value2)
				return value1;
			return value2;
		}

		MFUNC3 (C0)    vmax(const C0&value0, const C1&value1, const C2&value2)
		{
			if (value0 > value1)
			{
				if (value0 > value2)
					return value0;
				return value2;
			}
			if (value1 > value2)
				return value1;
			return value2;
		}
		

		MFUNC2 (C0)     loop_exp(const C0&b, const C1&e)
		{
			C0 result(1);
			for (C1 i = 0; i < e; i++)
				result*=b;
			return result;
		}
		

		MFUNC3 (bool)				sqrEquationHasResults(const C0&a, const C1&b, const C2&c)
		{
			C0 root = b*b - 4*a*c;
			if (root < 0)
				return false;
			return vabs(a) > GetError<C0>()
					||
					vabs(b) > GetError<C1>();
		}

		MFUNC4 (BYTE)     solveSqrEquation(const C0&a, const C1&b, const C2&c, C3 rs[2])
		{
			C3 root = b*b - 4*a*c;
			if (root < 0)
				return 0;
			if (vabs(a) <= GetError<C0>())
			{
				if (vabs(b) <= GetError<C1>())
					return 0;
				rs[0] = -c/b;
				return 1;
			}
			root = vsqrt(root);
			rs[0] = (-b-root)/(2*a);
			rs[1] = (-b+root)/(2*a);
			return rs[0] != rs[1]?2:1;
		}
	
		MFUNC3 (BYTE)     solveSqrEquation(const C0&p, const C1&q, C2 rs[2])
		{
			C2 root = p*p - 4*q;
			if (root < 0)
				return 0;
			
			root = vsqrt(root);
			rs[0] = (-p-root)/2;
			rs[1] = (-p+root)/2;
			return Similar(rs[0],rs[1])?1:2;
		}

		MFUNC (C)     sqr(const C&f)
		{
			return f*f;
		}

		MFUNC (C)     cubic(const C&f)
		{
			return f*f*f;
		}
	
		MFUNC (C)     cosFactor(const C&f)
		{
			return (cos((1-f)*M_PI)+1)/2;
		}

		MFUNC (C)     cubicFactor(const C&f)
		{
			return f*f*((C)3 - (C)2*f);
		}




		MFUNC3 (C0)          linearStep(const C0&v, const C1&lower, const C2&upper)
		{
			if (upper<=lower)
				return 0.5;
			return clamped((v-lower)/(upper-lower),0,1);
		}
	
		MFUNC3 (C0)          cosStep(const C0&v, const C1&lower, const C2&upper)
		{
			return cosFactor(linearStep(v,lower,upper));
		}
	
		MFUNC3 (C0)          cubicStep(const C0&v, const C1&lower, const C2&upper)
		{
			return cubicFactor(linearStep(v,lower,upper));
		}



		MFUNC (C)     notZero(const C&f)
		{
			return f?f:0.01;
		}
		MFUNC (bool) dividable(const C&value, const C&by)
		{
			return (int)((float)value/(float)by) == (float)value/(float)by;
		}

	}
	
	MFUNC	(C)			frac(C x)
	{
		return x - floor(x);
	}


	MFUNC (bool)		nearingZero(C value)
	{
		return vabs(value)<=GetError<C>();
	}

	MFUNC (bool)		nearingOne(C value)
	{
		return vabs(value-1) <= GetError<C>();
	}
	

	MFUNC2 (bool)		Similar(C0 v0, C1 v1)
	{
		return vabs(v0-v1) <= GetError<C0>();
	}

	MFUNC3 (bool)		Similar(C0 v0, C1 v1, C2 tolerance)
	{
		return vabs(v0-v1) <= tolerance;
	}

		


	MFUNC   (C)	rnd(C f)
	{
		C a = (C)floor(f);
		if (a+(C)0.5 < f)
			a++;
		return a;
	}



	#ifdef _MSC_VER
		MF_DECLARE (float)	fmax(float a, float b)
		{
			return a > b?a:b;
		}
	#endif


    template <class Type, class UType> static
		MF_DECLARE  (String) int2str(Type value)
    {
        char    buffer[256],
                *end = buffer+sizeof(buffer)-1,
                *c = end;
        (*end) = 0;
        bool negative = value < 0;
        if (negative)
            value*=-1;
        UType   uval = value;
        while (uval && c != buffer)
        {
            (*--c) = '0'+(uval%10);
            uval/=10;
        }
        if (c == end)
            (*--c) = '0';
        if (negative && c != buffer)
            (*--c) = '-';
        return c;
    }
    

    template <class Type>
	static	MF_DECLARE(String) uint2str(Type value)
    {
        char    buffer[256],
                *end = buffer+sizeof(buffer)-1,
                *c = end;
        (*end) = 0;
        while (value && c != buffer)
        {
            (*--c) = '0'+(value%10);
            value/=10;
        }
        if (c == end)
            (*--c) = '0';
        return c;
    }



    MF_DECLARE (void)      toggleBool(bool&b)
    {
        b = !b;
    }
	
	MF_SPECIALIZED	(String) v2str<bool>(const bool&value)
	{
		return value?"true":"false";
	
	}

    MFUNC	(String) v2str(const C&value)
    {
        return value;
    }


    

    MF_SPECIALIZED	(String) v2str<long long>(const long long&value)
    {
        return int2str<long long,unsigned long long>(value);
    }

    MF_SPECIALIZED	(String) v2str<unsigned long long>(const unsigned long long&value)
    {
        return uint2str<unsigned long long>(value);
    }

	#undef VCHAR
	#undef VNAME
	#undef VTYPE
	#if defined(_MSC_VER) && !defined(__CUDACC__)
		#define VCHAR(type)	MF_SPECIALIZED	(char) vChar<type>() {return #type[0]; }		MF_SPECIALIZED (char) vChar<volatile type>() {return #type[0]; }
		#define VNAME(type)	MF_SPECIALIZED	(const char*) vName<type>() {return #type; }	MF_SPECIALIZED (const char*) vName<volatile type>() {return #type; }
	#else
		#define VCHAR(type)	MF_SPECIALIZED	(char) vChar<type>() {return #type[0]; }
		#define VNAME(type)	MF_SPECIALIZED	(const char*) vName<type>() {return #type; }
	#endif
	#define VTYPE(type)	VCHAR(type) VNAME(type)
	
	VTYPE(double)
	VTYPE(float)
	VTYPE(int)
	VTYPE(unsigned)
	VTYPE(short)
	VTYPE(bool)
	VTYPE(BYTE)
	VTYPE(char)
	VTYPE(long)
	
	
	VNAME(long double)
	VNAME(unsigned short)
	VNAME(unsigned long)
	VNAME(long long)
	VNAME(unsigned long long)
	VNAME(String)
	VNAME(const char*)
	
	
	#undef VCHAR
	#undef VNAME
	#undef VTYPE
	

	MF_SPECIALIZED	(char) vChar<long long>() {return 'L'; }
	MF_SPECIALIZED	(char) vChar<long double>() {return 'D'; }
	MF_SPECIALIZED	(char) vChar<unsigned long long>() {return 'U'; }
	
	
	template <typename T>
		constexpr T GetError()
		{
			return std::numeric_limits<T>::epsilon()*10;
		}


    MFUNC3	(C0)          clamped(C0 v, C1 min, C2 max)
    {
        if (v < (C0)min)
            return (C0)min;
        if (v > (C0)max)
            return (C0)max;
        return v;
    }


	MFUNC3 (C0)          clamp(C0 v, C1 min, C2 max)
	{
		if (v < (C0)min)
			return (C0)min;
		if (v > (C0)max)
			return (C0)max;
		return v;
	}
	MFUNC (C)          clamp01(C v)
	{
		if (v < (C)0)
			return (C)0;
		if (v > (C)1)
			return (C)1;
		return v;
	}

    MFUNC (int)        Sign(C f)
    {
        if (f < -GetError<C>()/100)
            return -1;
        if (f <= GetError<C>()/100)
            return 0;
        return 1;
    }

    
	MFUNC3 (C0)           normalDistribution(C0 x, C1 mean, C2 deviation)
	{
		return ((C0)1.0/(deviation*((C0)2.506628274631000502415765284811)))*vexp(-sqr(x-mean)/(((C0)2.0)*deviation*deviation));
	}

    

    
    MFUNC2	(C0)        aligned(C0 i, C1 step)
    {
        if (i%step)
            return i+step-i%step;
        return i;
    }
    

    MFUNC	(C)       vabs(C v)
    {
		using std::fabs;
        return fabs(v);
    }

    MF_SPECIALIZED	(int)       vabs<int>(int v)
    {
        #if SYSTEM==WINDOWS
            return abs(v);
        #else
            return v>0?v:-v;
        #endif
    }


    MF_SPECIALIZED	(long int)   vabs<long int>(long int v)
    {
        #if SYSTEM==WINDOWS
            return labs(v);
        #else
            return v>0?v:-v;
        #endif
    }
	

	MF_SPECIALIZED(half)  vabs<half>(half v)
	{
		return v.Abs();
	}


    MF_SPECIALIZED	(long double)  vabs<long double>(long double v)
    {
        return fabsl(v);
    }



    MFUNC	(C)             vsqrt(C v)
    {
        return sqrt(v);
    }


    MF_SPECIALIZED	(long double)   vsqrt<long double>(long double v)
    {
        return sqrtl(v);
    }


    MF_SPECIALIZED	(int)   vsqrt<int>(int v)
    {
        return (int)sqrt((float)v);
    }


    MFUNC	(C)             vlog(C v)
    {
        return log(v);
    }

	
	

    MF_SPECIALIZED	(long double)   vlog<long double>(long double v)
	{
		return logl(v);
	}

	MFUNC2 (C0)             vpow(C0 b,C1 e)
	{
		return pow(b,(C0)e);
	}

    MF_SPECIALIZED	(long double)   vpow<long double, long double>(long double b,long double e)
	{
		return powl(b,e);
	}

	MFUNC (C)             vexp(C e)
	{
		return exp(e);
	}

    MF_SPECIALIZED	(long double)   vexp<long double>(long double e)
	{
		return expl(e);
	}


	template <typename T, bool IsFloat /*=false*/, bool IsSigned /*=false*/>
		class TPow10
		{
			typedef TPow10<T,IsFloat,IsSigned>	Self;
		public:
			typedef typename TypeInfo<T>::GreaterType::Type	ReturnType;			
			static ReturnType Execute(T val)
			{
				if (val == 0)
					return (ReturnType)1;
				if (val == 1)
					return (ReturnType)10;
				ReturnType rs = Self::Execute(val>>1);
				rs *= rs;
				if (val%2)
					rs *= 10;
				return rs;
			}

		};

	template <typename T, bool IsFloat /*=false*/>
		class TPow10<T, IsFloat,true>	//signed integer
		{
			typedef TPow10<T,IsFloat,true>	Self;
			typedef TPow10<typename TypeInfo<T>::UnsignedType::Type,IsFloat,false> Calculator;
		public:
			typedef double	ReturnType;

			static ReturnType Execute(T val)
			{
				if (val < 0)
					return 1.0 / (double)Calculator::Execute(-val);
				return (double)Calculator::Execute(val);
			}

		};

	template <typename T, bool IsSigned>
		class TPow10<T, true,IsSigned>
		{
			typedef TPow10<T,true,IsSigned>	Self;
		public:
			typedef T	ReturnType;

			static ReturnType Execute(T val)
			{
				return ::pow((T)10,val);
			}

		};

	template <typename T>
	auto            Pow10(T e) -> typename TPow10<T, TypeInfo<T>::is_float, TypeInfo<T>::is_signed>::ReturnType
	{
		return TPow10<T, TypeInfo<T>::is_float, TypeInfo<T>::is_signed>::Execute(e);
	}


	template <typename T, typename U>
	T            PowU(T base, U exponent)
	{
		switch (exponent)
		{
			case 0:
				return (T)1;
			case 1:
				return base;
			case 2:
				return base * base;
			case 3:
				return base * base * base;
		}
		const U exp2 = exponent >> 1;
		const T sub = PowU(base,exp2);
		if (exponent & 1)
		{
			return sub * sub * base;
		}
		return sub * sub;
	}


	template <typename T, typename I>
	T            PowI(T base, I exponent)
	{
		if (exponent < 0)
			return (T)1 / PowU(base,static_cast<typename TypeInfo<I>::UnsignedType::Type>(-exponent));
		return PowU(base,static_cast<typename TypeInfo<I>::UnsignedType::Type>(exponent));
	}



	MFUNC (C)             vlog10(C v)
	{
		return log10(v);
	}

    MF_SPECIALIZED	(long double)   vlog10<long double>(long double v)
	{
		return log10l(v);
	}


	MFUNC (C)        vatan(C x)
	{
		return atan(x);
	}

    MF_SPECIALIZED	(long double)        vatan<long double>(long double x)
	{
		return atanl(x);
	}

	MFUNC2 (C0)        vatan2(C0 y, C1 x)
	{
		return atan2(y,(C0)x);
	}

    MF_SPECIALIZED	(long double)        vatan2<long double, long double>(long double y, long double x)
	{
		return atan2l(y,x);
	}

	MFUNC (C)        vsin(C v)
	{
		return sin(v);
	}

    MF_SPECIALIZED	(long double)    vsin<long double>(long double v)
	{
		return sinl(v);
	}

	MFUNC (C)        vcos(C v)
	{
		return cos(v);
	}

    MF_SPECIALIZED	(long double)  vcos<long double>(long double v)
	{
		return cosl(v);
	}

	MFUNC (C)        vasin(C v)
	{
		return asin(v);
	}

    MF_SPECIALIZED	(long double) vasin<long double>(long double v)
	{
		return asinl(v);
	}

	MFUNC (C)        vacos(C v)
	{
		return acos(v);
	}

    MF_SPECIALIZED	(long double) vacos<long double>(long double v)
	{
		return acosl(v);
	}

	MFUNC (C)        vtan(C v)
	{
		return tan(v);
	}

    MF_SPECIALIZED	(long double)     vtan<long double>(long double v)
	{
		return tanl(v);
	}



	MFUNC2 (C0)    vmin(C0 value0, C1 value1)
	{
		return value0<(C0)value1?value0:(C0)value1;
	}

	MFUNC2 (C0)    vmax(C0 value0, C1 value1)
	{
		return value0>(C0)value1?value0:(C0)value1;
	}


	MFUNC3 (C0)    vmin(C0 value0, C1 value1, C2 value2)
	{
		if (value0 < value1)
		{
			if (value0 < value2)
				return value0;
			return value2;
		}
		if (value1 < value2)
			return value1;
		return value2;
	}

	MFUNC3 (C0)    vmax(C0 value0, C1 value1, C2 value2)
	{
		if (value0 > value1)
		{
			if (value0 > value2)
				return value0;
			return value2;
		}
		if (value1 > value2)
			return value1;
		return value2;
	}



	MFUNC2 (C0)     loop_exp(C0 b, C1 e)
	{
		C0 result(1);
		for (C1 i = 0; i < e; i++)
			result*=b;
		return result;
	}

    template <unsigned Value> MF_DECLARE (unsigned) factorial()
    {
        return Value*factorial<Value-1>();
    }

    MF_SPECIALIZED	(unsigned) factorial<0>()
    {
        return 1;
    }

    MF_DECLARE (unsigned) factorial(unsigned value)
    {
        unsigned result(1);
        for (unsigned i = 2; i <= value; i++)
            result*=i;
        return result;
    }

    MFUNCV	(C) resolveNURBSweight(unsigned element, const C&fc)
    {
    //    ShowMessage("final "+IntToStr(factorial<Dimensions-1>()/(factorial(element)*factorial(Dimensions-element-1))));
        return PowU(fc,element)*PowU(1-fc,Dimensions-element-1)*(C)factorial<Dimensions-1>()/(C)(factorial(element)*factorial(Dimensions-element-1));
    }

	MFUNC3 (bool)				sqrEquationHasResults(C0 a, C1 b, C2 c)
	{
		C0 root = b*b - 4*a*c;
		if (root < 0)
			return false;
		return vabs(a) > GetError<C0>()
				||
				vabs(b) > GetError<C1>();
	}

    MFUNC4	(BYTE)     solveSqrEquation(C0 a, C1 b, C2 c, C3 rs[2])
	{
		C3 root = b*b - 4*a*c;
		if (root < 0)
			return 0;
		if (vabs(a) <= GetError<C0>())
		{
			if (vabs(b) <= GetError<C1>())
				return 0;
			rs[0] = -c/b;
			return 1;
		}
		root = vsqrt(root);
		rs[0] = (-b-root)/(2*a);
		rs[1] = (-b+root)/(2*a);
		return rs[0] != rs[1]?2:1;
	}
	
	MFUNC3 (BYTE)     solveSqrEquation(C0 p, C1 q, C2 rs[2])
	{
		C2 root = p*p - 4*q;
		if (root < 0)
			return 0;
		
		root = vsqrt(root);
		rs[0] = (-p-root)/2;
		rs[1] = (-p+root)/2;
		return Similar(rs[0],rs[1])?1:2;
	}
	

	MFUNC   (C)           smallestPositiveResult(const C rs[2],BYTE num_res)
	{
		if (num_res == 1)
			return rs[0];
		if (rs[0] < rs[1] && rs[0] >= 0)
			return rs[0];
		if (rs[1] < rs[0] && rs[1] >= 0)
			return rs[1];
		if (rs[0] >= 0)
			return rs[0];
		return rs[1];
	}




	MFUNC (C)     sqr(C f)
	{
		return f*f;
	}

	MFUNC (C)     cubic(C f)
	{
		return f*f*f;
	}
	

	MFUNC (C)     cosFactor(C f)
	{
		return (cos((1-f)*M_PI)+1)/2;
	}

	MFUNC (C)     cubicFactor(C f)
	{
		return f*f*((C)3 - (C)2*f);
	};

	MFUNC (C)	ReverseCubicFactor(C y)
	{
		static const C a = C(-2);
		static const C b = C(3);
		const C d = -y;


		static const C delta0 = b*b;
		const C delta1 = C(2) * b * b * b + C(27) * a * a * d;

		std::complex<C> c = std::complex<C>(delta1*delta1 - C(4) * delta0 * delta0 * delta0,0);

		c = (delta1 + sqrt(c)) / C(2);

		c = pow(c,C(1)/C(3));
		
		std::complex<C> u1 = std::complex<C>(1);
		std::complex<C> u2 = std::complex<C>(C(-1),C(sqrt(3.0)))/C(2);
		std::complex<C> u3 = std::complex<C>(C(-1),C(-sqrt(3.0)))/C(2);

		std::complex<C> rs1 = -C(1) / (C(3) * a) * (b + u1*c + delta0 / (u1 * c));
		std::complex<C> rs2 = -C(1) / (C(3) * a) * (b + u2*c + delta0 / (u2 * c));
		std::complex<C> rs3 = -C(1) / (C(3) * a) * (b + u3*c + delta0 / (u3 * c));

		float control = CubicFactor(rs3.real());

		return rs3.real();
	}
	

	MFUNC3 (C0)          linearStep(C0 v, C1 lower, C2 upper)
	{
		if (upper<=lower)
			return 0.5;
		return clamped((v-lower)/(upper-lower),0,1);
	}

	MFUNC3 (C0)          cosStep(C0 v, C1 lower, C2 upper)
	{
		return cosFactor(linearStep(v,lower,upper));
	}

	MFUNC3 (C0)          cubicStep(C0 v, C1 lower, C2 upper)
	{
		return cubicFactor(linearStep(v,lower,upper));
	}



	MFUNC (bool) dividable(C value, C by)
	{
		return (int)((float)value/(float)by) == (float)value/(float)by;
	}


	MFUNC (C)     notZero(C f)
	{
		return f?f:0.01;
	}
	
	MFUNC	(C)		gaussian(C t)
	{
		if (t <= -2)
			return 0;
		if (t <= -1)
			return 1.0/6*cubic(2+t);
		if (t <= 0)
			return 1.0/6*(4-6*t*t-3*t*t*t);
		if (t <= 1)
			return 1.0/6*(4-6*t*t+3*t*t*t);
		if (t <= 2)
			return 1.0/6*cubic(2-t);
		return 0;
	}



	
	MFUNC9	(void)	determineCubicParameters(C0 x1, C1 y1, C2 x2, C3 y2, C4 x3, C5 y3, C6&a, C7&b, C8&c)
	{
	/*
	f(x) = ax² + bx + c
	
	y1 = ax1² + bx1 + c
	=> c = y1 - ax1² - bx1
	=> y1-y2 + a*(x2²-x1²) + b*(x2-x1) = 0
	=> b(x2-x1) = y2-y1 + a(x1²-x2²)
	=> b	= (y2-y1 + a(x1²-x2²))/(x2-x1)
		= (y3-y2 + a(x2²-x3²))/(x3-x2)
	=> (y2-y1)(x3-x2) + a(x1²-x2²)(x3-x2) = (y3-y2)(x2-x1) + a(x2²-x3²)(x2-x1)
	=> a((x1²-x2²)(x3-x2) - (x2²-x3²)(x2-x1)) = (y3-y2)(x2-x1) - (y2-y1)(x3-x2) 
		(y3-y2)(x2-x1) - (y2-y1)(x3-x2) 
	=> a = -------------------------------------------------
		(x1²-x2²)(x3-x2) - (x2²-x3²)(x2-x1)		
	*/
		C6	sub = ((x1*x1 - x2*x2)*(x3-x2) - (x2*x2 - x3*x3)*(x2-x1));
		a = ((y3-y2)*(x2-x1) - (y2-y1)*(x3-x2)) / sub;
		b = (y2-y1 + a*(x1*x1-x2*x2))/(x2-x1);
		c = y1 - a*x1*x1 - b*x1;
	}


}
#endif
