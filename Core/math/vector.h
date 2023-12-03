#ifndef vector_mathH
#define vector_mathH

/******************************************************************

Collection of template vector-math-functions.

******************************************************************/
#include "../global_root.h"
#include "../global_string.h"
#include "basic.h"

namespace DeltaWorks
{
	namespace Math
	{
		template <count_t Value0, count_t Value1>
			struct GreaterOrEqual
			{
				static const bool eval=Value0>=Value1;
			};

		template <typename T=float>
			struct TVec0
			{
				typedef typename TypeInfo<T>::UnionCompatibleBase
										Type;
				static	const count_t	Dimensions=0;

				Type				v[0];	//!< Standard accessor
			};

		template <typename T=float>
			struct TVec1
			{
				typedef typename TypeInfo<T>::UnionCompatibleBase
					Type;
				static	const count_t	Dimensions = 1;

				union
				{
					Type			v[1];	//!< Standard accessor
					Type			x;	//!< 1D coordinate accessor
					Type			alpha;	//!< Color accessor
					Type			r;	//!< Color accessor
					Type			red;	//!< Color accessor
				};
			};

		template <typename T=float>
			struct TVec2
			{
				typedef typename TypeInfo<T>::UnionCompatibleBase
					Type;
				static	const count_t	Dimensions = 2;

				union
				{
					Type			v[2];	//!< Standard accessor
					struct
					{
						Type		x, y;	//!< 2D coordinate accessors
					};
					struct
					{
						Type		r, g;	//!< Color accessors
					};
					struct
					{
						Type		red, green;	//!< Color accessors
					};
					struct
					{
						Type		luminance, alpha;	//!< Color accessors
					};
					//T				xy[2];	//!< Named accessor
					//T				la[2];	//!< Color accessor
				};
			};

		template <typename T=float>
			struct TVec3
			{
				typedef typename TypeInfo<T>::UnionCompatibleBase Type;
				static	const count_t	Dimensions = 3;

				union
				{
					Type			v[3];	//!< Standard accessor
					struct
					{
						Type		x, y, z;	//!< 3D coordinate accessors
					};
					struct
					{
						TVec2<T>	xy;
					};
					struct
					{
						TVec2<T>	rg;
					};
					struct
					{
						Type		x_;
						TVec2<T>	yz;
					};
					struct
					{
						Type		red, green, blue;	//!< Color accessors
					};
					struct
					{
						Type		r, g, b;	//!< Color accessors
					};
				};
			};
		template <typename T=float>
			struct TVec4
			{
				typedef typename TypeInfo<T>::UnionCompatibleBase Type;
				static	const count_t	Dimensions = 4;

				union
				{
					Type			v[4];	//!< Standard accessor
					struct
					{
						Type		x,y,z,w;	//!< 4D coordinate accessors
					};
					struct
					{
						TVec2<T>	xy,zw;
					};
					struct
					{
						TVec2<T>	rg,ba;
					};
					struct
					{
						Type		x_0;
						TVec3<T>	yzw; //!< yzw accessor
					};
					struct
					{
						Type		x_1;
						TVec2<T>	yz;
					};
					struct
					{
						Type		x_2;
						TVec2<T>	gb;
					};
					struct
					{
						Type		red, green, blue, alpha;	//!< Color accessors
					};
					struct
					{
						Type		r, g, b, a;	//!< Color accessors
					};
					TVec3<T>		rgb;		//!< Color accessor
					TVec3<T>		xyz;		//!< Vector accessor
					Type			rgba[4];	//!< Color accessor
				};
			};


		template <typename T, count_t D>	//due to subsequent specializations for D=0, D=1, D=2, D=3, and D=4, D must be > 4 here
			struct TVec
			{
				typedef typename TypeInfo<T>::UnionCompatibleBase Type;
				static	const count_t	Dimensions = D;

				union
				{
					Type			v[D];	//!< Standard accessor
					struct
					{
						Type		x, y, z, w;	//!< 4D coordinate accessors
					};
					struct
					{
						TVec2<T>	xy,zw; //!< xy and zw accessors
					};
					struct
					{
						TVec3<T>	xyz;	//!< xyz accessor
					};
					struct
					{
						Type		x_;
						TVec3<T>	yzw; //!< yzw accessor
					};
					struct
					{
						Type		red, green, blue, alpha;	//!< Color accessors
					};
					struct
					{
						Type		r, g, b, a;	//!< Color accessors
					};
					TVec3<T>		rgb;		//!< Color accessor
					TVec4<T>		rgba;	//!< Color accessor
					TVec4<T>		xyzw;	//!< Vector accessor
				};
			};




		template <typename T>
			struct TVec<T,0>:public TVec0<T>
			{};

		template <typename T>
			struct TVec<T,1>:public TVec1<T>
			{};
			

		template <typename T>
			struct TVec<T,2>:public TVec2<T>
			{};

		template <typename T>
			struct TVec<T,3>:public TVec3<T>
			{};
			

		template <typename T>
			struct TVec<T,4>:public TVec4<T>
			{};



		template <typename T>
			inline hash_t Hash(const TVec2<T>&v)
			{
				HashValue rs;
				rs.AddGeneric(v.x);
				rs.AddGeneric(v.y);
				return rs;
			}
		template <typename T>
			inline hash_t Hash(const TVec3<T>&v)
			{
				HashValue rs;
				rs.AddGeneric(v.x);
				rs.AddGeneric(v.y);
				rs.AddGeneric(v.z);
				return rs;
			}
		template <typename T>
			inline hash_t Hash(const TVec4<T>&v)
			{
				HashValue rs;
				rs.AddGeneric(v.x);
				rs.AddGeneric(v.y);
				rs.AddGeneric(v.z);
				rs.AddGeneric(v.w);
				return rs;
			}

		template <typename T, count_t D>
			inline hash_t Hash(const TVec<T,D>&v)
			{
				HashValue rs;
				for (index_t i = 0; i < D; i++)
					rs.AddGeneric(v.v[i]);
				return rs;
			}


		template <typename T>
			inline void Serialize(IWriteStream&s, const TVec2<T>&v)
			{
				using Serialization::Serialize;
				Serialize(s,v.x);
				Serialize(s,v.y);
			}
		template <typename T>
			inline void Deserialize(IReadStream&s, TVec2<T>&v)
			{
				using Serialization::Deserialize;
				Deserialize(s,v.x);
				Deserialize(s,v.y);
			}

		template <typename T>
			inline void Serialize(IWriteStream&s, const TVec3<T>&v)
			{
				using Serialization::Serialize;
				Serialize(s,v.x);
				Serialize(s,v.y);
				Serialize(s,v.z);
			}
		template <typename T>
			inline void Deserialize(IReadStream&s, TVec3<T>&v)
			{
				using Serialization::Deserialize;
				Deserialize(s,v.x);
				Deserialize(s,v.y);
				Deserialize(s,v.z);
			}


		template <typename T>
			inline void Serialize(IWriteStream&s, const TVec4<T>&v)
			{
				using Serialization::Serialize;
				Serialize(s,v.x);
				Serialize(s,v.y);
				Serialize(s,v.z);
				Serialize(s,v.w);
			}
		template <typename T>
			inline void Deserialize(IReadStream&s, TVec4<T>&v)
			{
				using Serialization::Deserialize;
				Deserialize(s,v.x);
				Deserialize(s,v.y);
				Deserialize(s,v.z);
				Deserialize(s,v.w);
			}


		template <typename T=float>
			class TFloatRange
			{
			public:
				typedef typename TypeInfo<T>::UnionCompatibleBase
					Type;
				typedef TFloatRange<T>	Self;

				union
				{
					struct
					{
						Type			min,max;
					};
					Type				extreme[2];
				};


				static const TFloatRange<T>	Invalid;


				bool					IsValid() const { return max >= min; }

				void					AssertIsNotNaN() const
				{
					ASSERT__(!isnan(min));
					ASSERT__(!isnan(max));
				}

				bool					operator==(const Self&other) const
				{
					return min == other.min && max == other.max;
				}
				bool					operator!=(const Self&other) const
				{
					return !operator==(other);
				}

				template <typename T1>
					TFloatRange<T>&		operator=(const TFloatRange<T1>&other)
										{
											min = (T)other.min;
											max = (T)other.max;
											return *this;
										}
									
				MF_DECLARE(void)		Set(const T&min, const T& max)
										{
											this->min = min;
											this->max = max;
										}
				template<typename T0>
					MF_DECLARE(void)	Set(const TFloatRange<T0>&other)
										{
											min = (T)other.min;
											max = (T)other.max;
										}
				MF_DECLARE(void)		SetCenter(const T&center)
										{
											T	ext = (max - min)/(T)2;
											SetCenter(center,ext);
										}
				MF_DECLARE(void)		SetCenter(const T&center, const T&ext)
										{
											min = center - ext;
											max = center + ext;
										}
				template <typename T0>
					MF_DECLARE(void)	SetMinAndMax(const T0&value)
										{
											min = max = (T)value;
										}
				MF_DECLARE(Self)		Scaled(const T&by)	const //! Scales the range from its central extent
										{
											T	c = center();
											T	ext  = (max - c);
											Self rs;
											rs.SetCenter(c,ext*by);
											return rs;
										}

				MF_DECLARE(void)		Scale(const T&by)	//! Scales the range from its central extent
										{
											T	c = center();
											T	ext  = (max - c);
											SetCenter(c,ext*by);
										}
				MF_DECLARE(void)		Scale(const T&scaleCenter, const T&by)	//! Scales the range from the specified center
										{
											min = (min - scaleCenter)*by + scaleCenter;
											max = (max - scaleCenter)*by + scaleCenter;
										}
				MF_DECLARE(void)		Expand(const T& by)	//! Expands the range. @b min is expected to be less than @b max
										{
											min -= by;
											max += by;
										}
				MF_DECLARE(void)		ConstrainBy(const TFloatRange<T>&constraint)	//! Modifies the local range so that it lies within the specified constraint range
										{
											min = Math::clamp(min,constraint.min,constraint.max);
											max = Math::clamp(max,constraint.min,constraint.max);
										}
				MF_DECLARE(T)			center()	const	//! Retrieves the center of the local range
										{
											return (min+max)/(T)2;
										}
				MF_DECLARE(T)			GetCenter()	const	//! @copydoc center()
										{
											return center();
										}
				MF_DECLARE(T)			GetExtent()	const	//! Retrieves the difference between max and min
										{
											return max-min;
										}
				template<typename T0>
					MF_DECLARE(bool)	Contains(const T0&value)	const //! Checks if the specified value lies within the local range
										{
											return (T)value >= min && (T)value <= max;
										}
				template<typename T0>
					MF_DECLARE(bool)	Contains(const TFloatRange<T0>&range)	const //! Checks if the specified range lies within the local range
										{
											return (T)range.min >= min && (T)range.max <= max;
										}
				template<typename T0>
					MF_DECLARE(void)	Include(const T0&value)	//!< Expands the local range so that it includes the specified value
										{
											this->min = M::Min(this->min,value);
											this->max = M::Max(this->max,value);
										}
				template<typename T0, typename T1>
					MF_DECLARE(void)	Include(const T0&min, const T1&max)	//!< Expands the local range so that it includes the specified range
										{
											this->min = M::Min(this->min,min);
											this->max = M::Max(this->max,max);
										}
				template<typename T0>
					MF_DECLARE(void)	Include(const TFloatRange<T0>&other)	//!< Expands the local range so that it includes the specified other range
										{
											this->min = M::Min(this->min,other.min);
											this->max = M::Max(this->max,other.max);
										}
				template <typename T0>
					MF_DECLARE(void)	Translate(const T0&delta)
										{
											min = (T)(min + delta);
											max = (T)(max + delta);
										}
				template <typename T0,typename T1>
					MF_DECLARE(void)	Relativate(const T0& absolute, T1&relative_out)	const
										{
											relative_out = (T1)(((T)absolute-min)/GetExtent());
										}
				template <typename T0>
					MF_DECLARE(T)		Relativate(const T0& absolute)	const
										{
											return (T)(((T)absolute-min)/GetExtent());
										}
				template <typename T0,typename T1>
					MF_DECLARE(void)	RelativateRange(const TFloatRange<T0>& absolute, TFloatRange<T1>&relative_out)	const
										{
											Relativate(absolute.min,relative_out.min);
											Relativate(absolute.max,relative_out.max);
										}
				template <typename T0,typename T1>
					MF_DECLARE(void)	Derelativate(const T0& relative, T1&absolute_out)	const
										{
											absolute_out = (T1)(min + relative*GetExtent());
										}
				template <typename T0>
					MF_DECLARE(T)		Derelativate(const T0& relative)	const
										{
											return (T)(min + (T)relative*GetExtent());
										}
				template <typename T0,typename T1>
					MF_DECLARE(void)	DerelativateRange(const TFloatRange<T0>& relative, TFloatRange<T1>&absolute_out)	const
										{
											Derelativate(relative.min,absolute_out.min);
											Derelativate(relative.max,absolute_out.max);
										}

				
				template <typename T0,typename T1>
					MF_DECLARE(void)	MakeRelative(const T0& absolute, T1&relative_out)	const
										{
											relative_out = (T1)(T)((absolute-min)/GetExtent());
										}
				template <typename T0,typename T1>
					MF_DECLARE(void)	MakeRangeRelative(const TFloatRange<T0>& absolute, TFloatRange<T1>&relative_out)	const
										{
											Relativate(absolute.min,relative_out.min);
											Relativate(absolute.max,relative_out.max);
										}
				template <typename T0,typename T1>
					MF_DECLARE(void)	MakeAbsolute(const T0& relative, T1&absolute_out)	const
										{
											absolute_out = (T1)(min + relative*GetExtent());
										}
				template <typename T0,typename T1>
					MF_DECLARE(void)	MakeRangeAbsolute(const TFloatRange<T0>& relative, TFloatRange<T1>&absolute_out)	const
										{
											Derelativate(relative.min,absolute_out.min);
											Derelativate(relative.max,absolute_out.max);
										}
				T						Clamp(const T&value)	const
										{
											return Math::clamp(value,min,max);
										}
				
				template <typename T1>
					MF_DECLARE(bool)	Intersects(const TFloatRange<T1>&other)	const
										{
											return min <= other.max && max >= other.min;
										}
				template <typename T1>
					MF_DECLARE(void)	operator*=(T1 factor)
										{
											min *= factor;
											max *= factor;
										}
				template <typename T1>
					MF_DECLARE(void)	operator/=(T1 factor)
										{
											min /= factor;
											max /= factor;
										}

				friend String			ToString(const Self&r)
										{
											using StringConversion::ToString;
											return "["+ToString(r.min)+","+ToString(r.max)+"]("+ToString(r.GetExtent())+")";
										}

				friend void Serialize(IWriteStream&s, const Self&v)
				{
					using Serialization::Serialize;
					Serialize(s,v.min);
					Serialize(s,v.max);
				}
				friend void Deserialize(IReadStream&s, Self&v)
				{
					using Serialization::Deserialize;
					Deserialize(s,v.min);
					Deserialize(s,v.max);
				}
	
			};

		template <typename T>
			const TFloatRange<T>	TFloatRange<T>::Invalid = {TypeInfo<T>::max, TypeInfo<T>::min};


		template <typename T=int>
			class TIntRange
			{
			public:
				typedef typename TypeInfo<T>::UnionCompatibleBase
					Type;
				typedef TIntRange<T>	Self;

				union
				{
					struct
					{
						Type			start,
										end;
					};
					Type				extreme[2];
				};

				static const TIntRange<T>		Invalid;


				bool					IsValid() const { return end >= start; }
				bool					IsEmpty() const { return end == start; }
				bool					IsNotEmpty() const { return end > start; }

				template <typename T1>
					TIntRange<T>&		operator=(const TIntRange<T1>&other)
										{
											start = (T)other.start;
											end = (T)other.end;
											return *this;
										}

				void					operator+=(T delta)
				{
					start += delta;
					end += delta;
				}
				void					operator-=(T delta)
				{
					start -= delta;
					end -= delta;
				}

				Self					GetLowerHalf() const {return Self{start,GetCenter()};}
				Self					GetUpperHalf() const {return Self{GetCenter(),end};}
									
				MF_DECLARE(void)		Set(const T&start, const T& end)
										{
											this->start = start;
											this->end = end;
										}
				template<typename T0>
					MF_DECLARE(void)	Set(const TIntRange<T0>&other)
										{
											start = (T)other.start;
											end = (T)other.end;
										}
				template <typename T0>
					MF_DECLARE(void)	SetEmpty(const T0&offset)
										{
											start = end = (T)offset;
										}
				MF_DECLARE(void)		Expand(const T& by)	//! Expands the range. @b min is expected to be less than @b max
										{
											start -= by;
											end += by;
										}
				MF_DECLARE(void)		ConstrainBy(const TIntRange<T>&constraint)	//! Modifies the local range so that it lies within the specified constraint range
										{
											start = Math::clamp(start,constraint.start,constraint.end);
											end = Math::clamp(end,constraint.start,constraint.end);
										}
				MF_DECLARE(T)			GetExtent()	const
										{
											return end - start;
										}
				MF_DECLARE(T)			GetCenter()	const
										{
											return (end + start) / 2;
										}
				template<typename T0>
					MF_DECLARE(bool)	Contains(const T0&value)	const //! Checks if the specified value lies within the local range
										{
											return (T)value >= start && (T)value < end;
										}
				template<typename T0>
					MF_DECLARE(bool)	Contains(const TIntRange<T0>&range)	const //! Checks if the specified range lies within the local range
										{
											return (T)range.start >= start && (T)range.end <= end;
										}
				template<typename T0>
					MF_DECLARE(void)	Include(const T0&value)	//!< Expands the local range so that it includes the specified value
										{
											start = Min(start,value);
											end = Max(end,value+1);
										}
				template<typename T0>
					MF_DECLARE(void)	Include(const TIntRange<T0>&other)	//!< Expands the local range so that it includes the specified other range
										{
											start = Min(start,other.start);
											end = Max(end,other.end);
										}
				template <typename T0>
					MF_DECLARE(void)	Translate(const T0&delta)
										{
											start = (T)(start + delta);
											end = (T)(end + delta);
										}
				template <typename T1>
					MF_DECLARE(bool)	Intersects(const TIntRange<T1>&other)	const
										{
											return start < other.end && end > other.start;
										}
				template <typename T1>
					MF_DECLARE(Self)	Intersect(const TIntRange<T1>&other)	const
										{
											Self rs;
											rs.start = Max(start,other.start);
											rs.end = Min(end,other.end);
											return rs;
										}

				friend String			ToString(const TIntRange<T>&r)
										{
											using StringConversion::ToString;
											return "["+ToString(r.start)+","+ToString(r.end)+")";
										}
				friend void Serialize(IWriteStream&s, const Self&v)
				{
					using Serialization::Serialize;
					Serialize(s,v.start);
					Serialize(s,v.end);
				}
				friend void Deserialize(IReadStream&s, Self&v)
				{
					using Serialization::Deserialize;
					Deserialize(s,v.start);
					Deserialize(s,v.end);
				}

			};

		template <typename T>
			const TIntRange<T>	TIntRange<T>::Invalid = {TypeInfo<T>::max, TypeInfo<T>::min};





		/**
		@brief Constructs a new float range
		*/
		template <typename T>
			inline constexpr TFloatRange<T>	FloatRange(const T&min, const T&max)
			{
				return TFloatRange<T> {min,max};
				//return result;
			}

		/**
		@brief Constructs a new float range
		*/
		template <typename T>
			inline TIntRange<T>	IntRange(const T&start, const T&end)
			{
				TIntRange<T> result = {start,end};
				return result;
			}


		#define DECLARE_FLOAT_RANGE_CONSTRUCTOR(_TYPE_)	inline TFloatRange<_TYPE_>	Range(const _TYPE_&min, const _TYPE_&max)	{TFloatRange<_TYPE_> result = {min,max}; return result;}
		#define DECLARE_INT_RANGE_CONSTRUCTOR(_TYPE_)	inline TIntRange<_TYPE_>	Range(const _TYPE_&start, const _TYPE_&end)	{TIntRange<_TYPE_> result = {start,end}; return result;}

		DECLARE_FLOAT_RANGE_CONSTRUCTOR(half);
		DECLARE_FLOAT_RANGE_CONSTRUCTOR(float);
		DECLARE_FLOAT_RANGE_CONSTRUCTOR(double);
		DECLARE_FLOAT_RANGE_CONSTRUCTOR(long double);

		DECLARE_INT_RANGE_CONSTRUCTOR(char);
		DECLARE_INT_RANGE_CONSTRUCTOR(short);
		DECLARE_INT_RANGE_CONSTRUCTOR(long);
		DECLARE_INT_RANGE_CONSTRUCTOR(int);
		DECLARE_INT_RANGE_CONSTRUCTOR(long long);

		/**
		@brief Constructs a new centered range
		*/
		template <typename T>
			inline TFloatRange<T>	CenterRange(const T&center, const T&extent)
			{
				TFloatRange<T> result = { center - extent, center + extent };
				return result;
			}


		template <typename T>
			inline constexpr TFloatRange<T>	MaxInvalidRange()
			{
				return FloatRange(std::numeric_limits<T>::max(),std::numeric_limits<T>::lowest());
			}

		template <typename T>
			class Quad
			{
			public:
				typedef typename TypeInfo<T>::UnionCompatibleBase
					Type;
				typedef Quad<T>	Self;

				union
				{
					struct
					{
						Type			left,
										bottom,
										right,
										top;
					};
					Type				value[4];
				};

				/**/					Quad(){}
				/**/					Quad(const T&v):left(v),bottom(v),right(v),top(v)	{}
				/**/					Quad(const T&left_, const T&bottom_, const T&right_, const T&top_)
											:left(left_),bottom(bottom_),right(right_),top(top_)	{}

				/**
				@brief Updates all values of the local quad
				*/
				MF_DECLARE(void)		Set(const T&left, const T& bottom, const T& right, const T& top)
										{
											this->left = left;
											this->right = right;
											this->bottom = bottom;
											this->top = top;
										}

				template <typename T0>
					MF_DECLARE(void)	SetAll(const T0&value)
										{
											left = (T)value;
											right = (T)value;
											bottom = (T)value;
											top = (T)value;
										}
				template <typename T1>
					MF_DECLARE(void)	operator*=(T1 factor)
										{
											left *= factor;
											right *= factor;
											bottom *= factor;
											top *= factor;
										}
				T						Max() const
				{
					using std::max;
					return max(max(left,right),max(top,bottom));
				}
				T						Min() const
				{
					using std::min;
					return min(min(left,right),min(top,bottom));
				}

				bool					operator==(const Self&other) const
				{
					return left == other.left && bottom == other.bottom && right == other.right && top == other.top;
				}
				bool					operator!=(const Self&other) const
				{
					return !operator==(other);
				}

			};

		inline	Quad<bool>	operator&&(const Quad<bool>&a, const Quad<bool>&b)
		{
			return Quad<bool>(a.left && b.left, a.bottom && b.bottom, a.right && b.right,a.top && b.top);
		}
		inline	Quad<bool>	operator||(const Quad<bool>&a, const Quad<bool>&b)
		{
			return Quad<bool>(a.left || b.left, a.bottom || b.bottom, a.right || b.right,a.top || b.top);
		}

		template <typename T=float>
			class Rect	//! General purpose rectangle
			{
			public:
				typedef typename TFloatRange<T>::Type	Type;	//union compatible. all references must use this
				union
				{
					struct
					{
						TFloatRange<T>	x,y;
					};
					TFloatRange<T>		axis[2];
				};

				typedef Rect<T>			Self;
				static Self				Invalid;

				/**/					Rect(){}
				/**/					Rect(const T&left, const T& bottom, const T& right, const T& top)
										{
											Set(left,bottom,right,top);
										}
				/**/					Rect(const TVec2<T>&min, const TVec2<T>&max)
										{
											Set(min,max);
										}
				/**/					Rect(const TVec2<T>&min_and_max)
										{
											SetMinAndMax(min_and_max);
										}
				/**/					Rect(const TFloatRange<T>&_x, const TFloatRange<T>&_y):x(_x),y(_y)	{}
				template<typename T1>
					/**/				Rect(const Rect<T1>&other)
											:x(other.x),y(other.y)	{}

				template <typename T1>
					Rect<T>&			operator=(const Rect<T1>&other)
										{
											x = other.x;
											y = other.y;
											return *this;
										}

				T						GetVolume() const {return x.GetExtent() * y.GetExtent();}

				void					AssertIsNotNaN() const
				{
					x.AssertIsNotNaN();
					y.AssertIsNotNaN();
				}
				bool					operator==(const Self&other) const
				{
					return x == other.x && y == other.y;
				}
				bool					operator!=(const Self&other) const
				{
					return !operator==(other);
				}
									
				/**
				@brief Updates all values of the local rectangle
				*/
				MF_DECLARE(void)		Set(const T&left, const T& bottom, const T& right, const T& top)
										{
											x.Set(left,right);
											y.Set(bottom,top);
										}
				/**
				@brief Updates all values of the local rectangle
				*/
				MF_DECLARE(void)		Set(const TVec2<T>&min, const TVec2<T>&max)
										{
											x.Set(min.x,max.x);
											y.Set(min.y,max.y);
										}
				/**
				@brief Updates all values of the local rectangle
				*/
				MF_DECLARE(void)		SetMinAndMax(const TVec2<T>&min_and_max)
										{
											x.SetMinAndMax(min_and_max.x);
											y.SetMinAndMax(min_and_max.y);
										}
				/**
				@brief Relocates the center of the local rectangle.
				Width and height remain unchanged.
				*/
				MF_DECLARE(void)		SetCenter(const T&x, const T& y)
										{
											this->x.SetCenter(x);
											this->y.SetCenter(y);
										}
				/**
				@brief Updates all values of the local rectangle so that the center points to the specified x and y coordinates, and width and height match twice the specified extent
				@param x X coordinate of the new center
				@param y Y coordinate of the new center
				@param ext_x Extent in both directions along the x axis. Equals half the resulting width
				@param ext_y Extent in both directions along the y axis. Equals half the resulting height
				*/
				MF_DECLARE(void)		SetCenter(const T&x, const T& y, const T&ext_x, const T&ext_y)
										{
											this->x.SetCenter(x,ext_x);
											this->y.SetCenter(y,ext_y);
										}
				/**
				@brief Updates all values of the local rectangle so that the center points to the specified x and y coordinates, and width and height match twice the specified extent
				@param x X coordinate of the new center
				@param y Y coordinate of the new center
				@param ext Extent in both directions along the x and y axes. Equals half the resulting width/height
				*/
				MF_DECLARE(void)		SetCenter(const T&x, const T& y, const T&ext)
										{
											this->x.SetCenter(x,ext);
											this->y.SetCenter(y,ext);
										}
				/**
				@brief Updates all values of the local rect to the specified value
				*/
				MF_DECLARE(void)		SetMinAndMax(const T&value)
										{
											x.SetMinAndMax(value);
											y.SetMinAndMax(value);
										}
				/**
				@brief Updates all values of the local rect to the specified value
				*/
				MF_DECLARE(void)		SetAll(const TFloatRange<T>&range)
										{
											x = range;
											y = range;
										}
				/**
				@brief Updates all min values of the local rectangle to the specified value
				*/
				template <typename T0>
					MF_DECLARE(void)	SetAllMin(const T0&value)
										{
											x.min = (T)value;
											y.min = (T)value;
										}
				/**
				@brief Updates all max values of the local rectangle to the specified value
				*/
				template <typename T0>
					MF_DECLARE(void)	SetAllMax(const T0&value)
										{
											x.max = (T)value;
											y.max = (T)value;
										}
				/**
				@brief Updates the minimum values of the local rectangle
				*/
				MF_DECLARE(void)		SetMin(const TVec2<T>&min)
										{
											x.min = min.x;
											y.min = min.y;
										}
				/**
				@brief Updates the maximum values of the local rectangle
				*/
				MF_DECLARE(void)		SetMax(const TVec2<T>&max)
										{
											x.max = max.x;
											y.max = max.y;
										}
				/**
				@brief Updates the minimum values of the local rectangle
				*/
				template<typename T0, typename T1>
					MF_DECLARE(void)	SetMin(const T0&min_x, const T1&min_y)
										{
											x.min = (T)min_x;
											y.min = (T)min_y;
										}
				/**
				@brief Updates the maximum values of the local rectangle
				*/
				template<typename T0, typename T1>
					MF_DECLARE(void)	SetMax(const T0&max_x, const T1&max_y)
										{
											x.max = (T)max_x;
											y.max = (T)max_y;
										}

				MF_DECLARE(void)		Scale(const T&by)	//! Scales the rectangle from its central location
										{
											x.Scale(by);
											y.Scale(by);
										}

				MF_DECLARE(Self)		Scaled(const T&by)	const //! Scales the rectangle from its central location
										{
											return Self(x.Scaled(by), y.Scaled(by));
										}
				MF_DECLARE(void)		Scale(const TVec2<T>&scaleCenter, const T&by)	//! Scales the rectangle from its central location
										{
											x.Scale(scaleCenter.x,by);
											y.Scale(scaleCenter.y,by);
										}
				MF_DECLARE(Rect<T>&)	Expand(const T& by)	//! Expands the rectangle. @b bottom is expected to be less than @b top
										{
											x.Expand(by);
											y.Expand(by);
											return *this;
										}
				template <typename T2>
					MF_DECLARE(Rect<T>&)ExpandRelatively(const T2& by)	//! Expands the rectangle using a relative factor on the current size. @b bottom is expected to be less than @b top
										{
											x.Expand(by * x.GetExtent());
											y.Expand(by * y.GetExtent());
											return *this;
										}

				MF_DECLARE(void)		ConstrainBy(const Rect<T>&constraint)	//! Modifies the local rectangle so that it lies within the specified constraint rectangle. @b bottom is expected to be less than @b top
										{
											x.ConstrainBy(constraint.x);
											y.ConstrainBy(constraint.y);
										}
				/*!
				@brief	Calculate aspect of the rectangle
				@return Aspect
				*/
				MF_DECLARE(T)			GetPixelAspect()	const
										{
											return x.GetExtent()/y.GetExtent();
										}
				MF_DECLARE(T)			GetAspect()	const	/** @copydoc GetPixelAspect() */
										{
											return GetPixelAspect();
										}
				/*!
				@brief	Determines whether or not the specified point lies within the local rectangle
				@return true if the point lies in the rectangle
				*/
				template <typename T0>
					MF_DECLARE(bool)	Contains(const TVec2<T0>&p)	const
										{
											return x.Contains(p.x) && y.Contains(p.y);
										}
				/*!
				@brief	Determines whether or not the specified rect lies within the local rectangle
				@return true if the rectangle lies in the local rectangle
				*/
				template <typename T0>
					MF_DECLARE(bool)	Contains(const Rect<T0>&r)	const
										{
											return x.Contains(r.x) && y.Contains(r.y);
										}
		
				/*!
				@brief Modifies the local rectangle so that it Contains the specified point. @b top is expected to be greater or equal @b bottom
				*/
				template <typename T0, typename T1>
					MF_DECLARE(void)	Include(const T0&x, const T1&y)
										{
											this->x.Include(x);
											this->y.Include(y);
										}
				/*!
				@brief Modifies the local rectangle so that it Contains the specified point. @b top is expected to be greater or equal @b bottom
				*/
				template <typename T0>
					MF_DECLARE(void)	Include(const TVec2<T0>&p)
										{
											x.Include(p.x);
											y.Include(p.y);
										}
				/*!
				@brief Modifies the local rectangle so that it Contains the specified rectangle. @b top is expected to be greater or equal @b bottom
				*/
				template <typename T0>
					MF_DECLARE(void)	Include(const Rect<T0>&other)
										{
											x.Include(other.x);
											y.Include(other.y);
										}
				/*!
				@brief Modifies the given point so that it lies within the local rectangle
				*/
				template <typename T0>
					MF_DECLARE(void)	Clamp(TVec2<T0>&p) const
										{
											p.x = x.Clamp(p.x);
											p.y = y.Clamp(p.y);
										}
					/*!
					\brief	Determines whether or not the specified point lies within the local rectangle
					\param x X coordinate of the point
					\param y Y coordinate of the point
					\return true if the point lies in the rectangle
					*/
				template <typename T0, typename T1>
					MF_DECLARE(bool)	Contains(const T0& x, const T1& y)	const
										{
											return this->x.Contains(x) && this->y.Contains(y);
										}
				/*!
					\brief Calculates the width of the rectangle
					\return Rectangle width
				*/
				MF_DECLARE(T)			GetWidth()		const
										{
											return x.GetExtent();
										}
				/*!
					\brief Calculates the height of the rectangle
					\return Rectangle height
				*/
				MF_DECLARE(T)			GetHeight()	const
										{
											return y.GetExtent();
										}
									
				template <typename T0, typename T1>
					MF_DECLARE(void)	Translate(const T0&delta_x, const T1&delta_y)
										{
											x.Translate(delta_x);
											y.Translate(delta_y);
										}
									
				template <typename T0>
					MF_DECLARE(void)	Translate(const TVec2<T0>&delta)
										{
											x.Translate(delta.x);
											y.Translate(delta.y);
										}
			
				template <typename T0,typename T1>
					MF_DECLARE(void)	MakeRelative(const T& x, const T& y, T0&x_out, T1&y_out)	const
										{
											this->x.MakeRelative(x,x_out);
											this->y.MakeRelative(y,y_out);
										}
				template <typename T0>
					MF_DECLARE(void)	MakeRelative(const TVec2<T>&p, TVec2<T0>&out)	const
										{
											MakeRelative(p.x,p.y,out.x,out.y);
										}			
			
				template <typename T0,typename T1>
					MF_DECLARE(void)	Relativate(const T& x, const T& y, T0&x_out, T1&y_out)	const
										{
											MakeRelative(x,y,x_out,y_out);
										}
				template <typename T0>
					MF_DECLARE(void)	Relativate(const TVec2<T>&absolute, TVec2<T0>&relative_out)	const
										{
											MakeRelative(absolute.x,absolute.y,relative_out.x,relative_out.y);
										}			
				MF_DECLARE(TVec2<T>)	Relativate(const TVec2<T>&absolute)	const
										{
											TVec2<T>	relative_out;
											MakeRelative(absolute.x,absolute.y,relative_out.x,relative_out.y);
											return relative_out;
										}					

				template <typename T0,typename T1>
					MF_DECLARE(void)	Derelativate(const T& x, const T& y, T0&x_out, T1&y_out)	const
										{
											MakeAbsolute(x,y,x_out,y_out);
										}
			
				template <typename T0,typename T1>
					MF_DECLARE(void)	MakeAbsolute(const T& x, const T& y, T0&x_out, T1&y_out)	const
										{
											this->x.MakeAbsolute(x,x_out);
											this->y.MakeAbsolute(y,y_out);
										}
				template <typename T0,typename T1>
					MF_DECLARE(void)	Derelativate(const TVec2<T0>& relative, TVec2<T1>&absolute_out)	const
										{
											x.MakeAbsolute(relative.x,absolute_out.x);
											y.MakeAbsolute(relative.y,absolute_out.y);
										}
				template <typename T0,typename T1>
					MF_DECLARE(void)	MakeAbsolute(const TVec2<T0>& relative, TVec2<T1>&absolute_out)	const
										{
											x.MakeAbsolute(relative.x,absolute_out.x);
											y.MakeAbsolute(relative.y,absolute_out.y);
										}
			
	
				template <typename T0, typename T1>
					MF_DECLARE(void)	MakeRelative(const Rect<T0>&absolute, Rect<T1>&relative_out)	const
										{
											x.RelativateRange(absolute.x,relative_out.x);
											y.RelativateRange(absolute.y,relative_out.y);
										}			
			
				template <typename T0, typename T1>
					MF_DECLARE(void)	Relativate(const Rect<T0>&absolute, Rect<T1>&relative_out)	const
										{
											x.RelativateRange(absolute.x,relative_out.x);
											y.RelativateRange(absolute.y,relative_out.y);
										}			

				template <typename T0, typename T1>
					MF_DECLARE(void)	MakeAbsolute(const Rect<T0>&relative, Rect<T1>&absolute_out)	const
										{
											x.DerelativateRange(relative.x,absolute_out.x);
											y.DerelativateRange(relative.y,absolute_out.y);
										}			
			
				template <typename T0, typename T1>
					MF_DECLARE(void)	Derelativate(const Rect<T0>&relative, Rect<T1>&absolute_out)	const
										{
											x.DerelativateRange(relative.x,absolute_out.x);
											y.DerelativateRange(relative.y,absolute_out.y);
										}			

				bool					IsValid() const { return x.IsValid() && y.IsValid(); }


				MF_DECLARE(Type&)		left()
										{
											return x.min;
										}
				MF_DECLARE(Type&)		right()
										{
											return x.max;
										}
				MF_DECLARE(Type&)		bottom()
										{
											return y.min;
										}
				MF_DECLARE(Type&)		top()
										{
											return y.max;
										}

				MF_DECLARE(const Type&)	left()	const
										{
											return x.min;
										}
				MF_DECLARE(const Type&)	right()	const
										{
											return x.max;
										}
				MF_DECLARE(const Type&)	bottom()	const
										{
											return y.min;
										}
				MF_DECLARE(const Type&)	top()	const
										{
											return y.max;
										}

				MF_DECLARE(const Type&)	operator[](index_t index)	const
										{
											DBG_ASSERT_LESS__(index,4);
											return axis[index%2].extreme[index/2];
										}
				MF_DECLARE(Type&)		operator[](index_t index)
										{
											DBG_ASSERT_LESS__(index,4);
											return axis[index%2].extreme[index/2];
										}
			
				template <typename T0>
					MF_DECLARE(void)	GetMin(TVec2<T0>&out)	const
										{
											out.x = (T0)x.min;
											out.y = (T0)y.min;
										}
				template <typename T0>
					MF_DECLARE(void)	GetMax(TVec2<T0>&out)	const
										{
											out.x = (T0)x.max;
											out.y = (T0)y.max;
										}
				template <typename T0>
					MF_DECLARE(void)	GetCenter(TVec2<T0>&out)	const
										{
											out.x = (T0)x.center();
											out.y = (T0)y.center();
										}
				template <typename T0>
					MF_DECLARE(void)	GetExtent(TVec2<T0>&out)	const
										{
											out.x = (T0)x.GetExtent();
											out.y = (T0)y.GetExtent();
										}
				MF_DECLARE(TVec2<T>)	min()	const
										{
											TVec2<T> rs ={x.min,y.min};
											return rs;
										}
				MF_DECLARE(TVec2<T>)	max()	const
										{
											TVec2<T> rs ={x.max,y.max};
											return rs;
										}
				MF_DECLARE(TVec2<T>)	center()	const
										{
											TVec2<T> rs = {x.center(),y.center()};
											return rs;
										}
				MF_DECLARE(TVec2<T>)	GetExtent()	const
										{
											TVec2<T> rs = {x.GetExtent(),y.GetExtent()};
											return rs;
										}

				template <typename T1>
					MF_DECLARE(bool)	Intersects(const Rect<T1>&other)	const
										{
											return x.Intersects(other.x) && y.Intersects(other.y);
										}
			
				friend String			ToString(const Rect<T>&r)
										{
											return	"("+String(r.x.min)+", "+String(r.y.min)+") - ("+String(r.x.max)+", "+String(r.y.max)+")";
										}
				template <typename T1>
					MF_DECLARE(void)	operator*=(T1 factor)
										{
											x *= factor;
											y *= factor;
										}
				template <typename T1>
					MF_DECLARE(void)	operator/=(T1 factor)
										{
											x /= factor;
											y /= factor;
										}

				friend void Serialize(IWriteStream&s, const Self&v)
				{
					using Serialization::Serialize;
					Serialize(s,v.x);
					Serialize(s,v.y);
				}
				friend void Deserialize(IReadStream&s, Self&v)
				{
					using Serialization::Deserialize;
					Deserialize(s,v.x);
					Deserialize(s,v.y);
				}

			};
	
		typedef Rect<float>	TFloatRect;

		template <typename T>
			Rect<T>			Rect<T>::Invalid = Rect<T>(TFloatRange<T>::Invalid, TFloatRange<T>::Invalid);



		template <typename T=float>
			class Box //: public IToString	//! General purpose box
			{
			public:
				typedef typename TFloatRange<T>::Type	Type;	//union compatible. all references must use this

				typedef Box<T>			Self;
				union
				{
					struct
					{
						TFloatRange<T>	x, y, z;
					};
					TFloatRange<T>		axis[3];
				};



				/**/					Box(){}
				/**/					Box(const T&min_x, const T& min_y, const T& min_z, const T&max_x, const T& max_y, const T& max_z)
										{
											Set(min_x, min_y, min_z, max_x, max_y, max_z);
										}
				/**/					Box(const TVec3<T>&min, const TVec3<T>&max)
										{
											Set(min,max);
										}

				/**/					Box(const TVec3<T>&min_and_max)
										{
											SetMinAndMax(min_and_max);
										}
				constexpr				Box(const TFloatRange<T>&x, const TFloatRange<T>&y, const TFloatRange<T>&z)
										{
											this->x = x;
											this->y = y;
											this->z = z;
										}

				static const Self	Invalid;

				template<typename T1>
					/**/				Box(const Box<T1>&other)
										{
											x.Set(other.x);
											y.Set(other.y);
											z.Set(other.z);
										}

				template <typename T1>
					Box<T>&				operator=(const Box<T1>&other)
										{
											x.Set(other.x);
											y.Set(other.y);
											z.Set(other.z);
											return *this;
										}

				void					AssertIsNotNaN() const
				{
					x.AssertIsNotNaN();
					y.AssertIsNotNaN();
					z.AssertIsNotNaN();
				}
				bool					operator==(const Self&other) const
				{
					return x == other.x && y == other.y && z == other.z;
				}
				bool					operator!=(const Self&other) const
				{
					return !operator==(other);
				}
																		

				bool					IsValid() const { return x.IsValid() && y.IsValid() && z.IsValid(); }
				/**
				@brief Updates all values of the local box
				*/
				MF_DECLARE(void)		Set(const T&min_x, const T& min_y, const T& min_z, const T&max_x, const T& max_y, const T& max_z)
										{
											x.Set(min_x,max_x);
											y.Set(min_y,max_y);
											z.Set(min_z,max_z);
										}
				/**
				@brief Updates all values of the local rectangle
				*/
				MF_DECLARE(void)		Set(const TVec3<T>&min, const TVec3<T>&max)
										{
											x.Set(min.x,max.x);
											y.Set(min.y,max.y);
											z.Set(min.z,max.z);
										}
				/**
				@brief Updates all values of the local rectangle
				*/
				MFUNC(void)				SetMinAndMax(const TVec3<C>&min_and_max)
										{
											x.SetMinAndMax((T)min_and_max.x);
											y.SetMinAndMax((T)min_and_max.y);
											z.SetMinAndMax((T)min_and_max.z);
										}
				/**
				@brief Relocates the center of the local box.
				Width, height, and depth remain unchanged.
				*/
				MF_DECLARE(void)		SetCenter(const T&x, const T& y, const T&z)
										{
											this->x.SetCenter(x);
											this->y.SetCenter(y);
											this->z.SetCenter(z);
										}
				/**
				@brief Updates all values of the local box so that the center points to the specified x, y, z coordinates, and width, height, depth match twice the specified extent
				@param center New center
				@param ext Extent in both directions along the respective axis. Equals half the resulting width/height/depth
				*/
				template<typename T0, typename T1>
					MF_DECLARE(void)	SetCenter(const TVec3<T0>&center, const TVec3<T1>&ext)
										{
											x.SetCenter(center.x,ext.x);
											y.SetCenter(center.y,ext.y);
											z.SetCenter(center.z,ext.z);
										}
				/**
				@brief Updates all values of the local box so that the center points to the specified x, y, z coordinates, and width, height, depth match twice the specified extent
				@param center New center
				@param ext Extent in both directions along the respective axis. Equals half the resulting width/height/depth
				*/
				template<typename T0, typename T1>
					MF_DECLARE(Box<T>&)	SetCenter(const TVec3<T0>&center, const T1&ext)
										{
											x.SetCenter(center.x,ext);
											y.SetCenter(center.y,ext);
											z.SetCenter(center.z,ext);
											return *this;
										}
				/**
				@brief Updates all values of the local box to the specified value
				*/
				MF_DECLARE(void)		SetMinAndMax(const T&value)
										{
											x.SetMinAndMax(value);
											y.SetMinAndMax(value);
											z.SetMinAndMax(value);
										}
				/**
				@brief Updates all values of the local box to the specified value
				*/
				MF_DECLARE(void)		SetAll(const TFloatRange<T>&range)
										{
											x = range;
											y = range;
											z = range;
										}
				/**
				@brief Updates all min values of the local box to the specified value
				*/
				template <typename T0>
					MF_DECLARE(void)	SetAllMin(const T0&value)
										{
											x.min = (T)value;
											y.min = (T)value;
											z.min = (T)value;
										}
				/**
				@brief Updates all max values of the local box to the specified value
				*/
				template <typename T0>
					MF_DECLARE(void)	SetAllMax(const T0&value)
										{
											x.max = (T)value;
											y.max = (T)value;
											z.max = (T)value;
										}
				/**
				@brief Updates the minimum values of the local box
				*/
				MF_DECLARE(void)		SetMin(const TVec3<T>&min)
										{
											x.min = min.x;
											y.min = min.y;
											z.min = min.z;
										}
				/**
				@brief Updates the maximum values of the local box
				*/
				MF_DECLARE(void)		SetMax(const TVec3<T>&max)
										{
											x.max = max.x;
											y.max = max.y;
											z.max = max.z;
										}
				/**
				@brief Updates the minimum values of the local box
				*/
				template <typename T0, typename T1, typename T2>
					MF_DECLARE(void)	SetMin(const T0&min_x,const T1&min_y,const T2&min_z)
										{
											x.min = (T)min_x;
											y.min = (T)min_y;
											z.min = (T)min_z;
										}
				/**
				@brief Updates the maximum values of the local box
				*/
				template <typename T0, typename T1, typename T2>
					MF_DECLARE(void)	SetMax(const T0&max_x,const T1&max_y,const T2&max_z)
										{
											x.max = (T)max_x;
											y.max = (T)max_y;
											z.max = (T)max_z;
										}

				MF_DECLARE(Box<T>&)		Scale(const T&by)	//! Scales the box from its central location
										{
											x.Scale(by);
											y.Scale(by);
											z.Scale(by);
											return *this;
										}
				MF_DECLARE(Box<T>&)		Expand(const T& by)	//! Expands the box
										{
											x.Expand(by);
											y.Expand(by);
											z.Expand(by);
											return *this;
										}
									
				MF_DECLARE(Box<T>&)		ConstrainBy(const Box<T>&constraint)	//! Modifies the local box so that it lies within the specified constraint box
										{
											x.ConstrainBy(constraint.x);
											y.ConstrainBy(constraint.y);
											z.ConstrainBy(constraint.z);
											return *this;
										}
				/*!
				@brief	Determines whether or not the specified point lies within the local box
				@param p 3d point
				@return true if the point lies in the box
				*/
				template <typename T0>
					MF_DECLARE(bool)	Contains(const TVec3<T0>&p)	const
										{
											return x.Contains(p.x) && y.Contains(p.y) && z.Contains(p.z);
										}

				/*!
				@brief Modifies the given point so that it lies within the local box
				*/
				template <typename T0>
					MF_DECLARE(void)	Clamp(TVec3<T0>&p) const
										{
											p.x = x.Clamp(p.x);
											p.y = y.Clamp(p.y);
											p.z = z.Clamp(p.z);
										}

				/*!
				@brief	Determines whether or not the specified box lies within the local box
				@return true if the box lies in the box
				*/
				template <typename T0>
					MF_DECLARE(bool)	Contains(const Box<T0>&b)	const
										{
											return x.Contains(b.x) && y.Contains(b.y) && z.Contains(b.z);
										}
		
				/*!
				@brief Modifies the local box so that it Contains the specified point
				*/
				template <typename T0, typename T1, typename T2>
					MF_DECLARE(void)	Include(const T0&x, const T1&y, const T2&z)
										{
											this->x.Include(x);
											this->y.Include(y);
											this->z.Include(z);
										}
				/*!
				@brief Modifies the local box so that it Contains the specified point
				*/
				template <typename T0>
					MF_DECLARE(void)	Include(const TVec3<T0>&p)
										{
											x.Include(p.x);
											y.Include(p.y);
											z.Include(p.z);
										}
				/*!
				@brief Modifies the local box so that it Contains the specified other box
				*/
				template <typename T0>
					MF_DECLARE(void)	Include(const Box<T0>&other)
										{
											x.Include(other.x);
											y.Include(other.y);
											z.Include(other.z);
										}
		
				/*!
				\brief	Determines whether or not the specified point lies within the local rectangle
				\param x X coordinate of the point
				\param y Y coordinate of the point
				\return true if the point lies in the rectangle
				*/
				template <typename T0, typename T1, typename T2>
					MF_DECLARE(bool)	Contains(const T0& x, const T1& y, const T2& z)	const
										{
											return this->x.Contains(x) && this->y.Contains(y) && this->z.Contains(z);
										}
				/*!
				\brief Calculates the width of the box
				\return Box extent along the x axis
				*/
				MF_DECLARE(T)			GetWidth()		const
										{
											return x.GetExtent();
										}
				/*!
				\brief Calculates the height of the box
				\return Box extent along the y axis
				*/
				MF_DECLARE(T)			GetHeight()	const
										{
											return y.GetExtent();
										}
				/*!
				\brief Calculates the depth of the box
				\return Box extent along the z axis
				*/
				MF_DECLARE(T)			depth()	const
										{
											return z.GetExtent();
										}
				MF_DECLARE(T)			GetDepth()		const	/** @copydoc depth() */ {return depth();}
				MF_DECLARE(T)			volume()	const
										{
											return GetWidth() * GetHeight() * depth();
										}
				MF_DECLARE(T)			GetVolume()		const	/** @copydoc volume() */ {return volume();}
				template <typename T0>
					MF_DECLARE(void)	GetVolume(T0&result)	const
										{
											result = (T0)GetWidth() * (T0)GetHeight() * (T0)depth();
										}

									
				template <typename T0, typename T1, typename T2>
					MF_DECLARE(void)	Translate(const T0&delta_x, const T1&delta_y, const T2&delta_z)
										{
											x.Translate(delta_x);
											y.Translate(delta_y);
											z.Translate(delta_z);
										}
				template <typename T0>
					MF_DECLARE(void)	Translate(const TVec3<T0>&delta)
										{
											x.Translate(delta.x);
											y.Translate(delta.y);
											z.Translate(delta.z);
										}
			


				template <typename T0, typename T1>
					MF_DECLARE(void)	MakeRelative(const TVec3<T0>&absolute, TVec3<T1>&relative_out)	const
										{
											x.Relativate(absolute.x,relative_out.x);
											y.Relativate(absolute.y,relative_out.y);
											z.Relativate(absolute.z,relative_out.z);
										}			
			
				template <typename T0>
					MF_DECLARE(void)	Relativate(const TVec3<T>&absolute, TVec3<T0>&relative_out)	const
										{
											x.Relativate(absolute.x,relative_out.x);
											y.Relativate(absolute.y,relative_out.y);
											z.Relativate(absolute.z,relative_out.z);
										}			
			

				template <typename T0,typename T1>
					MF_DECLARE(void)	Derelativate(const TVec3<T0>& relative, TVec3<T1>&absolute_out)	const
										{
											x.MakeAbsolute(relative.x,absolute_out.x);
											y.MakeAbsolute(relative.y,absolute_out.y);
											z.MakeAbsolute(relative.z,absolute_out.z);
										}
				template <typename T0,typename T1>
					MF_DECLARE(void)	MakeAbsolute(const TVec3<T0>& relative, TVec3<T1>&absolute_out)	const
										{
											x.MakeAbsolute(relative.x,absolute_out.x);
											y.MakeAbsolute(relative.y,absolute_out.y);
											z.MakeAbsolute(relative.z,absolute_out.z);
										}
			
	
				template <typename T0, typename T1>
					MF_DECLARE(void)	MakeRelative(const Box<T0>&absolute, Box<T1>&relative_out)	const
										{
											x.RelativateRange(absolute.x,relative_out.x);
											y.RelativateRange(absolute.y,relative_out.y);
											z.RelativateRange(absolute.z,relative_out.z);
										}			
			
				template <typename T0, typename T1>
					MF_DECLARE(void)	Relativate(const Box<T0>&absolute, Box<T1>&relative_out)	const
										{
											x.RelativateRange(absolute.x,relative_out.x);
											y.RelativateRange(absolute.y,relative_out.y);
											z.RelativateRange(absolute.z,relative_out.z);
										}			

				template <typename T0, typename T1>
					MF_DECLARE(void)	MakeAbsolute(const Box<T0>&relative, Box<T1>&absolute_out)	const
										{
											x.DerelativateRange(relative.x,absolute_out.x);
											y.DerelativateRange(relative.y,absolute_out.y);
											z.DerelativateRange(relative.z,absolute_out.z);
										}			
			
				template <typename T0, typename T1>
					MF_DECLARE(void)	Derelativate(const Box<T0>&relative, Box<T1>&absolute_out)	const
										{
											x.DerelativateRange(relative.x,absolute_out.x);
											y.DerelativateRange(relative.y,absolute_out.y);
											z.DerelativateRange(relative.z,absolute_out.z);
										}			
				template <typename T0>
					void				GetMin(TVec3<T0>&out)	const
										{
											out.x = (T0)x.min;
											out.y = (T0)y.min;
											out.z = (T0)z.min;
										}
				template <typename T0>
					void				GetMax(TVec3<T0>&out)	const
										{
											out.x = (T0)x.max;
											out.y = (T0)y.max;
											out.z = (T0)z.max;
										}
				template <typename T0>
					void				GetCenter(TVec3<T0>&out)	const
										{
											out.x = (T0)x.center();
											out.y = (T0)y.center();
											out.z = (T0)z.center();
										}
					TVec3<T>			GetCenter() const {TVec3<T> rs; GetCenter(rs); return rs;}
				template <typename T0>
					void				GetExtent(TVec3<T0>&out)	const
										{
											out.x = (T0)x.GetExtent();
											out.y = (T0)y.GetExtent();
											out.z = (T0)z.GetExtent();
										}
				MF_DECLARE(TVec3<T>)	center()	const
										{
											TVec3<T> rs = {x.center(),y.center(),z.center()};
											return rs;
										}
				MF_DECLARE(TVec3<T>)	min()	const
										{
											TVec3<T> rs = {x.min,y.min,z.min};
											return rs;
										}
				MF_DECLARE(TVec3<T>)	max()	const
										{
											TVec3<T> rs = {x.max,y.max,z.max};
											return rs;
										}
				MF_DECLARE(TVec3<T>)	GetExtent()	const
										{
											TVec3<T> rs = {x.GetExtent(),y.GetExtent(),z.GetExtent()};
											return rs;
										}

				template <typename T1>
					MF_DECLARE(bool)	Intersects(const Box<T1>&other)	const
										{
											return x.Intersects(other.x) && y.Intersects(other.y) && z.Intersects(other.z);
										}
			
				friend String			ToString(const Box<T>&b)
										{
											return	"("+String(b.x.min)+", "+String(b.y.min)+", "+String(b.z.min)+") - ("+String(b.x.max)+", "+String(b.y.max)+", "+String(b.z.max)+")";
										}
				template <typename T1>
					MF_DECLARE(void)	operator*=(T1 factor)
										{
											x *= factor;
											y *= factor;
											z *= factor;
										}

				friend void Serialize(IWriteStream&s, const Self&v)
				{
					using Serialization::Serialize;
					Serialize(s,v.x);
					Serialize(s,v.y);
					Serialize(s,v.z);
				}
				friend void Deserialize(IReadStream&s, Self&v)
				{
					using Serialization::Deserialize;
					Deserialize(s,v.x);
					Deserialize(s,v.y);
					Deserialize(s,v.z);
				}
			};



		template <typename T>
			const Box<T>	Box<T>::Invalid = Box<T>(MaxInvalidRange<T>(),MaxInvalidRange<T>(),MaxInvalidRange<T>());

	

		/**
			@brief Predefined global 2d vectors
		*/
		template <class C=float>
			struct Vector2
			{
				static const TVec2<C>	x_axis,
										y_axis,
										negative_x_axis,
										negative_y_axis,
										zero,
										null,
										one,
										negative_one;
				static TVec2<C>			dummy;


			private:
										Vector2()	{};	//cannot be instantiated
			};

		/**
			@brief Predefined global 3d vectors
		*/
		template <class C=float>
			struct Vector3
			{
				static const TVec3<C>	x_axis,
										y_axis,
										z_axis,
										negative_x_axis,
										negative_y_axis,
										negative_z_axis,
										zero,
										null,
										one;
				static TVec3<C>			dummy;


			private:
										Vector3()	{};	//cannot be instantiated
			};

		/**
			@brief Predefined global 4d vectors
		*/
		template <class C=float>
			struct Vector4
			{
				static const TVec4<C>	x_axis,
										y_axis,
										z_axis,
										w_axis,
										negative_x_axis,
										negative_y_axis,
										negative_z_axis,
										negative_w_axis,
										zero,
										null,
										one;
				static TVec4<C>			dummy;


			private:
										Vector4()	{};	//cannot be instantiated
			};

		template <class C>
			const TVec2<C>		Vector2<C>::x_axis = {1,0};
		template <class C>
			const TVec2<C>		Vector2<C>::y_axis = {0,1};
		template <class C>
			const TVec2<C>		Vector2<C>::negative_x_axis = {-1,0};
		template <class C>
			const TVec2<C>		Vector2<C>::negative_y_axis = {0,-1};
		template <class C>
			const TVec2<C>		Vector2<C>::zero = {0,0};
		template <class C>
			const TVec2<C>		Vector2<C>::null = {0,0};
		template <class C>
			const TVec2<C>		Vector2<C>::one = {1,1};
		template <class C>
			const TVec2<C>		Vector2<C>::negative_one = {-1,-1};
		template <class C>
			TVec2<C>				Vector2<C>::dummy = {0,0};


		template <class C>
			const TVec3<C>		Vector3<C>::x_axis = {1,0,0};
		template <class C>
			const TVec3<C>		Vector3<C>::y_axis = {0,1,0};
		template <class C>
			const TVec3<C>		Vector3<C>::z_axis = {0,0,1};
		template <class C>
			const TVec3<C>		Vector3<C>::negative_x_axis = {-1,0,0};
		template <class C>
			const TVec3<C>		Vector3<C>::negative_y_axis = {0,-1,0};
		template <class C>
			const TVec3<C>		Vector3<C>::negative_z_axis = {0,0,-1};
		template <class C>
			const TVec3<C>		Vector3<C>::zero = {0,0,0};
		template <class C>
			const TVec3<C>		Vector3<C>::null = {0,0,0};
		template <class C>
			const TVec3<C>		Vector3<C>::one = {1,1,1};
		template <class C>
			TVec3<C>			Vector3<C>::dummy = {0,0,0};



		template <class C>
			const TVec4<C>		Vector4<C>::x_axis = {1,0,0,0};
		template <class C>
			const TVec4<C>		Vector4<C>::y_axis = {0,1,0,0};
		template <class C>
			const TVec4<C>		Vector4<C>::z_axis = {0,0,1,0};
		template <class C>
			const TVec4<C>		Vector4<C>::w_axis = {0,0,0,1};
		template <class C>
			const TVec4<C>		Vector4<C>::negative_x_axis = {-1,0,0,0};
		template <class C>
			const TVec4<C>		Vector4<C>::negative_y_axis = {0,-1,0,0};
		template <class C>
			const TVec4<C>		Vector4<C>::negative_z_axis = {0,0,-1,0};
		template <class C>
			const TVec4<C>		Vector4<C>::negative_w_axis = {0,0,0,-1};
		template <class C>
			const TVec4<C>		Vector4<C>::zero = {0,0,0,0};
		template <class C>
			const TVec4<C>		Vector4<C>::null = {0,0,0,0};
		template <class C>
			const TVec4<C>		Vector4<C>::one = {1,1,1,1};
		template <class C>
			TVec4<C>				Vector4<C>::dummy = {0,0,0,0};


	/**
		\brief Assembler style vector operators

		Though these functions may look primitive, they are highly optimized.
		<br />Function variances:<br /><ul>
		<li>_someFunc(parameter(...),result[3]) - 3 dimensional variance</li>
		<li>_someFunc(inout_parameter[3],parameter(...)) - in-place 3 dimensional variance</li>
		<li>_someFunc2(parameter(...),result[2]) - 2 dimensional variance</li>
		<li>_someFuncV(parameter(...),*result) - N-dimensional compile time variance (Number of components are specified in the last template parameter)</li>
		<li>_someFunc(parameter(...),*result,dimensions) - N-dimensional run-time variance</li></ul>
	*/



		#if __cplusplus > 199711L
			#define register      // Deprecated in C++11.
		#endif  // #if __cplusplus > 199711L
	



	
		template <typename T=float>
			class Sphere/*: public IToString*/	//! General purpose sphere - better not implement IToString due to possible IO-problems
			{
			public:
				TVec3<T>				center;		//!< Sphere center (point)
				T						radius;		//!< Sphere radius (scalar)
			
				MF_CONSTRUCTOR			Sphere(const T& radius = 1);
				MF_CONSTRUCTOR			Sphere(const TVec3<T>& center, const T& radius = 1);

				static Sphere<T>		Invalid()	{return Sphere((T)-1);}

				MF_DECLARE(bool)		Intersects(const Sphere<T>& remote)		const;
				/**
					@brief Extends radius so that the local sphere includes the specified point. The calculated distance from the local sphere center to the specified point is left squared thus increasing performance
				*/
				MFUNC(void)				IncludeSquare(const TVec3<C>& point);
				/**
				@brief Expands the local radius to include the specified sphere, if necessary. Does not modify the local sphere's center
				*/
				MFUNC(void)				Include(const TVec3<C>& sphereCenter, const T& sphereRadius);
				/**
				@brief Expands the local radius to include the specified sphere, if necessary. Does not modify the local sphere's center
				*/
				MFUNC(void)				Include(const Sphere<C>&sphere)	{ Include(sphere.center, sphere.radius); }
				/**
				@brief Extends radius so that the local sphere includes the specified point 
				*/
				MFUNC(void)				Include(const TVec3<C>& point);
				MFUNC(void)				Merge(const TVec3<C>& point, const T& radius);
				MFUNC(void)				Merge(const Sphere<C>&sphere)	{Merge(sphere.center,sphere.radius);}
				MFUNC(bool)				Contains(const TVec3<C>& point)	const;
				MFUNC(bool)				Contains(const Sphere<C>& sphere)	const;
				MF_DECLARE(T)			volume()	const;
				MF_DECLARE(T)			GetVolume()		const;
				template <typename T0>
				MF_DECLARE(void)	GetVolume(T0& result)	const;
				template <typename T0>
				void				GetCenter(TVec3<T0>& out)	const;

				friend String			ToString(const Sphere<T>&s)
										{
											using StringConversion::ToString;
											return ToString(s.center)+",r="+ToString(s.radius);
										}
			};

		namespace Vec
		{
			template <typename T>
				MF_DECLARE (TVec4<T>&) 	ref4(T*vec)
				{
					return *reinterpret_cast<TVec4<T>*>(vec);
				}


			template <typename T>
				MF_DECLARE (const TVec4<T>&) 	ref4(const T*vec)
				{
					return *reinterpret_cast<const TVec4<T>*>(vec);
				}

			template <typename T>
				MF_DECLARE (TVec3<T>&) 	ref3(T*vec)
				{
					return *reinterpret_cast<TVec3<T>*>(vec);
				}


			template <typename T>
				MF_DECLARE (const TVec3<T>&) 	ref3(const T*vec)
				{
					return *reinterpret_cast<const TVec3<T>*>(vec);
				}

			template <typename T>
				MF_DECLARE (TVec2<T>&) 	ref2(T*vec)
				{
					return *reinterpret_cast<TVec2<T>*>(vec);
				}


			template <typename T>
				MF_DECLARE (const TVec2<T>&) 	ref2(const T*vec)
				{
					return *reinterpret_cast<const TVec2<T>*>(vec);
				}

			template <typename T>
				MF_DECLARE (TVec1<T>&) 	ref1(T*vec)
				{
					return *reinterpret_cast<TVec1<T>*>(vec);
				}


			template <typename T>
				MF_DECLARE (const TVec1<T>&) 	ref1(const T*vec)
				{
					return *reinterpret_cast<const TVec1<T>*>(vec);
				}



			template <typename T0, typename T1, typename T2>
				MF_DECLARE(void)	def(TVec2<T0>&v, T1 x, T2 y)
				{
					v.x = (T0)x;
					v.y = (T0)y;
				}
			template <typename T0, typename T1, typename T2, typename T3>
				MF_DECLARE(void)	def(TVec3<T0>&v, T1 x, T2 y, T3 z)
				{
					v.x = (T0)x;
					v.y = (T0)y;
					v.z = (T0)z;
				}
			template <typename T0, typename T1, typename T2, typename T3, typename T4>
				MF_DECLARE(void)	def(TVec4<T0>&v, T1 x, T2 y, T3 z, T4 w)
				{
					v.x = (T0)x;
					v.y = (T0)y;
					v.z = (T0)z;
					v.w = (T0)w;
				}

			template <typename T0, typename T1, typename T2, typename T3>
				MF_DECLARE(void)	def3(T0*v, T1 x, T2 y, T3 z)
				{
					v[0] = (T0)x;
					v[1] = (T0)y;
					v[2] = (T0)z;
				}
			
			template <typename T>
				inline void	assertConsistency()
				{
					TVec1<T>	t1;
					TVec2<T>	t2;
					TVec3<T>	t3;
					TVec4<T>	t4;

					//ASSERT_EQUAL__((const void*)t0.v,(const void*)&t0);
					ASSERT_EQUAL__((const void*)t1.v,(const void*)&t1);
					ASSERT_EQUAL__((const void*)t2.v,(const void*)&t2);
					ASSERT_EQUAL__((const void*)t3.v,(const void*)&t3);
					ASSERT_EQUAL__((const void*)t4.v,(const void*)&t4);


					ASSERT_EQUAL__((const void*)(((const T*)&t2.y)-1),(const void*)&t2);
					ASSERT_EQUAL__((const void*)(((const T*)&t3.y)-1),(const void*)&t3);
					ASSERT_EQUAL__((const void*)(((const T*)&t4.y)-1),(const void*)&t4);

					ASSERT_EQUAL__((const void*)(((const T*)&t3.z)-2),(const void*)&t3);
					ASSERT_EQUAL__((const void*)(((const T*)&t4.z)-2),(const void*)&t4);

					ASSERT_EQUAL__((const void*)(((const T*)&t4.w)-3),(const void*)&t4);

					ASSERT_EQUAL__(&t1.x,&t1.alpha);

					ASSERT_EQUAL__(&t2.y,&t2.alpha);
					ASSERT_EQUAL__(&t2.x,&t2.luminance);

					//ASSERT_EQUAL__(t2.v,t2.la);
					//ASSERT_EQUAL__(t3.v,t3.rgb);
					ASSERT_EQUAL__((const void*)t4.v,(const void*)&t4.rgb);
					ASSERT_EQUAL__((const void*)(t4.v+2),(const void*)&t4.zw);
					ASSERT_EQUAL__((const void*)t4.v, (const void*)&t4.rgba);

					ASSERT_EQUAL__(&t3.z,&t3.blue);
					ASSERT_EQUAL__(&t3.y,&t3.green);
					ASSERT_EQUAL__(&t3.x,&t3.red);

					ASSERT_EQUAL__(&t4.w,&t4.alpha);
					ASSERT_EQUAL__(&t4.z,&t4.blue);
					ASSERT_EQUAL__(&t4.y,&t4.green);
					ASSERT_EQUAL__(&t4.x,&t4.red);

					ASSERT_EQUAL__((const T*)&ref4(t4.v),t4.v);
					ASSERT_EQUAL__((const T*)&ref3(t3.v),t3.v);
					ASSERT_EQUAL__((const T*)&ref2(t2.v),t2.v);
					ASSERT_EQUAL__((const T*)&ref1(t1.v),t1.v);
				};


			template <typename T>
				void HSL2RGB(const TVec3<T>&hsl, TVec3<T>&outRGB)
				{
					outRGB.r = hsl.z;   // default to gray
					outRGB.g = hsl.z;
					outRGB.b = hsl.z;
					T v = (hsl.z <= (T)0.5) ? (hsl.z * ((T)1.0 + hsl.y)) : (hsl.z + hsl.y - hsl.z * hsl.y);
					if (v > 0)
					{
						T m;
						T sv;
						int sextant;
						T fract, vsf, mid1, mid2;

						m = hsl.z + hsl.z - v;
						sv = (v - m) / v;
						T scaled = hsl.x * (T)6.0;
						fract = fmod(scaled,(T)1.0);
						vsf = v * sv * fract;
						mid1 = m + vsf;
						mid2 = v - vsf;
						switch ((int)scaled)
						{
						case 0:
							outRGB.r = v;
							outRGB.g = mid1;
							outRGB.b = m;
							break;
						case 1:
							outRGB.r = mid2;
							outRGB.g = v;
							outRGB.b = m;
							break;
						case 2:
							outRGB.r = m;
							outRGB.g = v;
							outRGB.b = mid1;
							break;
						case 3:
							outRGB.r = m;
							outRGB.g = mid2;
							outRGB.b = v;
							break;
						case 4:
							outRGB.r = mid1;
							outRGB.g = m;
							outRGB.b = v;
							break;
						case 5:
							outRGB.r = v;
							outRGB.g = m;
							outRGB.b = mid2;
							break;
						}
					}
			}
		}
		#include "vector_operations.h"




		template <typename T>
			inline String ToString(const TVec2<T>&v)
			{
				return Vec::toString(v);
			}
		template <typename T>
			inline String ToString(const TVec3<T>&v)
			{
				return Vec::toString(v);
			}
		template <typename T>
			inline String ToString(const TVec4<T>&v)
			{
				return Vec::toString(v);
			}

		template <typename T, count_t D>
			inline String ToString(const TVec<T,D>&v)
			{
				return Vec::toString(v);
			}

	}

	namespace Vec = Math::Vec;

}


#include "vector.tpl.h"
#endif
