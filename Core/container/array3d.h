#pragma once

#include "array.h"
#include "../math/sizeND.h"

namespace DeltaWorks
{
	namespace Container
	{


		/**
		@brief Three dimensional, row-major array
		
		The Array3D class maps 3d access to a single-dimensional field. Internally it behaves like a normal Ctr::Array instance, but it provides some 3d access helper method.
		*/
		template <typename T, class MyStrategy=typename Strategy::StrategySelector<T>::Default>
			class Array3D:public Ctr::Array<T,MyStrategy>
			{
				typedef Ctr::Array<T, MyStrategy>	Super;
			protected:
				using Super::data;
			public:
				typedef Array3D<T,MyStrategy>	Self;
				typedef Size3D<count_t>	Size;
				typedef Index3D<index_t> Index;

				using Super::Count;
				using Super::Fill;

				template <int AxisIndex>
					class Axis;

				template <int AxisIndex>
					class Iterator
					{
						index_t	at;
						friend class Axis<AxisIndex>;

						/**/			Iterator(index_t at):at(at)	{}
					public:
						typedef Iterator<AxisIndex>	Self;
						Iterator		operator++(int) {Iterator rs(*this); at++; return rs;}
						Iterator&		operator++() {at++; return *this;}
						Iterator		operator--(int) {Iterator rs(*this); at--; return rs;}
						Iterator&		operator--() {at--; return *this;}
						index_t			operator*() const {return at;}
						bool			operator==(index_t other) const {return at == other;}
						bool			operator!=(index_t other) const {return at != other;}
						bool			operator<=(index_t other) const {return at <= other;}
						bool			operator>=(index_t other) const {return at >= other;}
						bool			operator<(index_t other) const {return at < other;}
						bool			operator>(index_t other) const {return at > other;}
						bool			operator==(const Self&other) const {return at == other.at;}
						bool			operator!=(const Self&other) const {return at != other.at;}
						bool			operator<=(const Self&other) const {return at <= other.at;}
						bool			operator>=(const Self&other) const {return at >= other.at;}
						bool			operator<(const Self&other) const {return at < other.at;}
						bool			operator>(const Self&other) const {return at > other.at;}
					};
			
				template <int AxisIndex>
					class Axis
					{
						const index_t		extent;

						friend class Self;

						/**/			Axis(index_t extent):extent(extent)	{}
					public:
						typedef Iterator<AxisIndex>	iterator,const_iterator;

						constexpr iterator	begin() const {return 0;}
						iterator		end() const {return extent;}
						count_t			size() const {return extent;}
					};	


				Array3D():w(0),h(0) {}
				Array3D(const Size&res):Super(res.CountElements()),w(res.width),h(res.height) {}
				Array3D(count_t width, count_t height, count_t depth):Super(width*height*depth),w(width),h(height) {}
				Array3D(const Size&res, const T&initial):Super(res.CountElements()),w(res.width),h(res.height)
				{
					Fill(initial);
				}
				Array3D(count_t width, count_t height, count_t depth, const T&initial):Super(width*height*depth),w(width),h(height)
				{
					Fill(initial);
				}

				template <class T, class OtherStrategy>
					Array3D(const Array3D<T,OtherStrategy>&other):Super(other),w(other.w),h(other.h)
					{}
					
				#if __ARRAY_RVALUE_REFERENCES__
					Array3D(const Self&other):Super(other),w(other.w),h(other.h)	{}
					Array3D(Self&&other):Super(std::move( other )),w(other.w),h(other.h)
					{
						other.w = 0;
						other.h = 0;
					}
					void operator=(Self&&other)
					{
						adoptData(other);
					}
				#endif

				void operator=(const Self&other)
				{
					Super::operator=(other);
					w = other.w;
					h = other.h;
				}
				
				count_t		GetWidth()	const	//! Retrieves this array's width \return width
				{
					return w;
				}
				count_t		GetDepth()	const	//! Retrieves this array's depth \return depth
				{
					return w&&h?Super::elements/w/h:0;
				}
				count_t		GetHeight() const	//! Retrieves this array's height \return height
				{
					return h;
				}
				Size		GetSize() const {return Size(GetWidth(),GetHeight(),GetDepth());}

				Axis<0>		Horizontal() const {return Axis<0>(GetWidth());}
				Axis<1>		Vertical() const {return Axis<1>(GetHeight());}
				Axis<2>		Deep() const {return Axis<2>(GetDepth());}

				index_t		ToLinearIndex(const Index&idx) const
				{
					return ToLinearIndex(idx.x,idx.y,idx.z);
				}
				index_t		ToLinearIndexNoCheck(const Index&idx) const
				{
					return ToLinearIndexNoCheck(idx.x,idx.y,idx.z);
				}


				index_t		ToLinearIndex(index_t x, index_t y, index_t z) const
				{
					#ifdef __ARRAY_DBG_RANGE_CHECK__
						if (x >= w || y >= h || z >= GetDepth())
							FATAL__("Index out of bounds");
					#endif
					return ToLinearIndexNoCheck(x,y,z);
				}

				Index		ToVectorIndex(index_t linear) const
				{
					#ifdef __ARRAY_DBG_RANGE_CHECK__
						if (linear >= Count())
							FATAL__("Index out of bounds");
					#endif
					return ToVectorIndexNoCheck(linear);
				}

				Index		ToVectorIndexNoCheck(index_t linear) const
				{
					const count_t slice = w*h;
					return Index(linear % w, (linear%slice) / w, linear / slice);
				}


				index_t		ToLinearIndexNoCheck(index_t x, index_t y, index_t z) const
				{
					return z*w*h + y*w + x;
				}

				

				void		SetSize(const Size&res)
				{
					SetSize(res.width,res.height,res.depth);
				}
				void		SetSize(count_t width, count_t height, count_t depth)	//! Resizes the local 2d array to match the specified dimensions. The local array content is lost if the array's total size is changed
				{
					Super::SetSize(width*height*depth);
					w = width;
					h = height;
				}
				void		Set(index_t x, index_t y, index_t z, const T&value)
				{
					Super::data[ToLinearIndex(x,y,z)] = value;
				}
			
				T&			Get(index_t x, index_t y, index_t z)
				{
					return Super::data[ToLinearIndex(x,y,z)];
				}
				const T&			Get(index_t x, index_t y, index_t z) const
				{
					return Super::data[ToLinearIndex(x,y,z)];
				}
				T&			Get(const Iterator<0>&x, const Iterator<1>&y, const Iterator<2>&z)
				{
					return Get(*x,*y,*z);
				}

				const T&	Get(const Index&idx) const
				{
					return Get(idx.x, idx.y, idx.z);
				}
				T&	Get(const Index&idx)
				{
					return Get(idx.x, idx.y, idx.z);
				}

				/**
				Copies up to @a extent many elements along each axis from @a other, starting at @a offset.
				The local array will be resized to match the copied elements
				@param other Array to copy from
				@param offset Starting index to copy from (inclusive) in @a other. Mapped to (0,0) in the local array. May exceed valid range, resulting in an empty array
				@param extent Number of elements to copy along each axis. May exceed valid range, resulting in reduced numbers
				*/
				void		ResizeToAndCopyBlock(const Self&other, const Index&offset, Size extent)
				{
					if (offset.x >= other.GetWidth() || offset.y >= other.GetHeight() || offset.z >= other.GetDepth())
					{
						SetSize(0,0,0);
						return;
					}
					extent.Limit(other.GetSize() - offset);
					SetSize(extent);
					const Index end = offset + extent;
					for (index_t x = offset.x; x < end.x; x++)
						for (index_t y = offset.y; y < end.y; y++)
							for (index_t z = offset.z; z < end.z; z++)
								data[ToLinearIndexNoCheck(x - offset.x,y-offset.y,z-offset.z)] = other.data[other.ToLinearIndexNoCheck(x,y,z)];
				}

			
				void	adoptData(Self&other)	//! Adopts pointer and size and sets both NULL of the specified origin array.
				{
					w = other.w;
					h = other.h;
					Super::adoptData(other);
					other.w = 0;
					other.h = 0;
				}
				void	swap(Self&other)
				{
					Super::swap(other);
					swp(w,other.w);
					swp(h,other.h);
				}
				friend void swap(Self&a, Self&b)
				{
					a.swap(b);
				}

				bool	operator==(const Self&other) const
				{
					return w == other.w && h == other.h && Super::operator==(other);
				}

				bool	operator!=(const Self&other) const
				{
					return !operator==(other);
				}

				friend void			Serialize(IWriteStream&s, const Self&v)
				{
					using Serialization::Serialize;
					s.WriteSize(v.w);
					s.WriteSize(v.h);
					Serialize(s,(const Super&)v);
				}
				friend void			Deserialize(IReadStream&s, Self&v)
				{
					using Serialization::Deserialize;
					s.ReadSize(v.w);
					s.ReadSize(v.h);
					Deserialize(s,(Super&)v);
				}

			protected:
				
				count_t	w,h;
				
				using Super::SetSize;
				using Super::ResizePreserveContent;
				using Super::erase;
				using Super::Append;

			};
	}
}

DECLARE_T2__(Container::Array3D,typename,class,SwapStrategy);

