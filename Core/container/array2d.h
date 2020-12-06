#pragma once

#include "array.h"
#include "../math/sizeND.h"

namespace DeltaWorks
{

	namespace Container
	{
		/**
		@brief Two dimensional, row-major array
		
		The Array2D class maps 2d access to a single-dimensional field. Internally it behaves like a normal Ctr::Array instance, but it provides some 2d access helper method.
		*/
		template <class C, class MyStrategy=typename Strategy::StrategySelector<C>::Default>
			class Array2D:public Ctr::Array<C,MyStrategy>
			{
				typedef Ctr::Array<C, MyStrategy>	Super;

			public:
				typedef Array2D<C,MyStrategy>	Self;

				typedef Size2D<count_t>		Size;
				typedef Index2D<index_t>	Index;

				using Super::Fill;

				template <bool IsHorizontal>
					class Axis;

				template <bool IsHorizontal>
					class Iterator
					{
						index_t	at;
						friend class Axis<IsHorizontal>;

						/**/			Iterator(index_t at):at(at)	{}
					public:
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
						bool			operator==(const Iterator<IsHorizontal>&other) const {return at == other.at;}
						bool			operator!=(const Iterator<IsHorizontal>&other) const {return at != other.at;}
						bool			operator<=(const Iterator<IsHorizontal>&other) const {return at <= other.at;}
						bool			operator>=(const Iterator<IsHorizontal>&other) const {return at >= other.at;}
						bool			operator<(const Iterator<IsHorizontal>&other) const {return at < other.at;}
						bool			operator>(const Iterator<IsHorizontal>&other) const {return at > other.at;}
					};
			
				template <bool IsHorizontal>
					class Axis
					{
						const index_t		extent;

						friend class Self;//Array2D<C,Strategy>;

						/**/			Axis(index_t extent):extent(extent)	{}
					public:
						typedef Iterator<IsHorizontal>	iterator,const_iterator;

						constexpr iterator	begin() const {return 0;}
						iterator		end() const {return extent;}
						count_t			size() const {return extent;}
					};	


				Array2D():w(0) {}
				Array2D(index_t width, index_t height):Super(width*height),w(width) {}
				Array2D(const Size&res):Super(res.CountElements()),w(res.width) {}
				Array2D(index_t width, index_t height, const C&initial):Super(width*height),w(width)
				{
					Fill(initial);
				}
				Array2D(const Size&res, const C&initial):Super(res.CountElements()),w(res.width)
				{
					Fill(initial);
				}

				template <class T, class OtherStrategy>
					Array2D(const Array2D<T,OtherStrategy>&other):Super(other),w(other.w)
					{}
					
				#if __ARRAY_RVALUE_REFERENCES__
					Array2D(const Self&other):Super(other),w(other.w)	{}
					Array2D(Self&&other):Super(std::move( other )),w(other.w)
					{}
					void operator=(Self&&other)
					{
						adoptData(other);
					}
				#endif

				void operator=(const Self&other)
				{
					Super::operator=(other);
					w = other.w;
				}
				
				index_t		GetWidth()	const	//! Retrieves this array's width \return width
				{
					return w;
				}
				index_t		GetHeight() const	//! Retrieves this array's height \return height
				{
					return w?Ctr::Array<C,MyStrategy>::elements/w:0;
				}
				Size		GetSize() const {return Size(GetWidth(),GetHeight());}

				Axis<true>	Horizontal() const {return Axis<true>(GetWidth());}
				Axis<false>	Vertical() const {return Axis<false>(GetHeight());}


				void		InsertRowBefore(index_t rowIndex)
				{
					const count_t h = GetHeight();
					rowIndex = std::min(rowIndex,h);
					Array2D<C,MyStrategy> temp(GetWidth(),h+1);
					MyStrategy::moveElements(Super::pointer(),temp.pointer(),w*rowIndex);
					MyStrategy::moveElements(Super::pointer()+rowIndex*w,temp.pointer()+(rowIndex+1)*w,w*(h - rowIndex));
					swap(temp);
				}

				void		InsertColumnBefore(index_t colIndex)
				{
					const count_t h = GetHeight();
					colIndex = std::min(colIndex,GetWidth());

					const count_t w1 = w+1;
					Array2D<C,MyStrategy> temp(w1,h);
					for (index_t r = 0; r < h; r++)
					{
						C*row0 = Super::pointer()+r*w;
						C*row1 = temp.pointer()+r*w1;
						MyStrategy::moveElements(row0,row1,colIndex);
						MyStrategy::moveElements(row0+colIndex,row1+colIndex+1,w-colIndex);
					}
					swap(temp);
				}

				void		EraseRow(index_t rowIndex)
				{
					const count_t h = GetHeight();
					if (rowIndex >= h)
						return;
					Array2D<C,MyStrategy> temp(GetWidth(),h-1);
					MyStrategy::moveElements(Super::pointer(),temp.pointer(),w*(rowIndex));
					MyStrategy::moveElements(Super::pointer()+(rowIndex+1)*w,temp.pointer()+(rowIndex)*w,w*(h - rowIndex-1));
					swap(temp);
				}

				void		EraseColumn(index_t colIndex)
				{
					const count_t h = GetHeight();
					if (colIndex >= GetWidth())
						return;
					const count_t w1 = w-1;
					Array2D<C,MyStrategy> temp(w1,h);
					for (index_t r = 0; r < h; r++)
					{
						C*row0 = Super::pointer()+r*w;
						C*row1 = temp.pointer()+r*w1;
						MyStrategy::moveElements(row0,row1,colIndex);
						MyStrategy::moveElements(row0+colIndex+1,row1+colIndex,w-colIndex-1);
					}
					swap(temp);
				}

				void		CopyRowTo(index_t row, Ctr::ArrayData<C>&target) const
				{
					#ifdef __ARRAY_DBG_RANGE_CHECK__
						if (row >= GetHeight())
							FATAL__("Index out of bounds");
					#endif
					target.SetSize(w);
					MyStrategy::copyElements(Super::pointer()+row*w,target.pointer(),w);
				}

				void		CopyColumnTo(index_t col, Ctr::ArrayData<C>&target) const
				{
					#ifdef __ARRAY_DBG_RANGE_CHECK__
						if (col >= w)
							FATAL__("Index out of bounds");
					#endif
					count_t h = GetHeight();
					target.SetSize(h);
					for (index_t i= 0; i < h; i++)
					{
						target[i] = Super::data[ToLinearIndexNoCheck(col,i)];
					}
				}

				void		CopyRowFrom(index_t row, const Ctr::ArrayData<C>&source)
				{
					#ifdef __ARRAY_DBG_RANGE_CHECK__
						if (row >= GetHeight())
							FATAL__("Index out of bounds");
						ASSERT__(source.GetLength() == w);
					#endif
					MyStrategy::copyElements(source.pointer(),Super::pointer()+row*w,w);
				}

				void		CopyColumnFrom(index_t col, const Ctr::ArrayData<C>&source)
				{
					count_t h = GetHeight();
					#ifdef __ARRAY_DBG_RANGE_CHECK__
						if (col >= w)
							FATAL__("Index out of bounds");
						ASSERT__(source.GetLength() == h);
					#endif
					for (index_t i= 0; i < h; i++)
					{
						Super::data[ToLinearIndexNoCheck(col,i)] = source[i];
					}
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
					if (offset.x >= other.GetWidth() || offset.y >= other.GetHeight())
					{
						SetSize(0,0);
						return;
					}
					extent.Limit(other.GetSize() - offset);
					SetSize(extent);
					const Index end = offset + extent;
					for (index_t x = offset.x; x < end.x; x++)
						for (index_t y = offset.y; y < end.y; y++)
							data[ToLinearIndexNoCheck(x - offset.x,y-offset.y)] = other.data[other.ToLinearIndexNoCheck(x,y)];
				}

				index_t		ToLinearIndex(const Index&idx) const
				{
					return ToLinearIndex(idx.x,idx.y);
				}
				index_t		ToLinearIndexNoCheck(const Index&idx) const
				{
					return ToLinearIndexNoCheck(idx.x,idx.y);
				}

				index_t		ToLinearIndex(index_t x, index_t y) const
				{
					#ifdef __ARRAY_DBG_RANGE_CHECK__
						if (x >= w || y >= GetHeight())
							FATAL__("Index out of bounds");
					#endif
					return ToLinearIndexNoCheck(x,y);
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
					return Index(linear % w, linear / w);
				}

				index_t		ToLinearIndexNoCheck(index_t x, index_t y) const
				{
					return y*w+x;
				}

				void		SetSize(const Size&res)
				{
					SetSize(res.width,res.height);
				}
				void		SetSize(index_t width, index_t height)	//! Resizes the local 2d array to match the specified dimensions. The local array content is lost if the array's total size is changed
				{
					Super::SetSize(width*height);
					w = width;
				}
				void		Set(index_t x, index_t y, const C&value)	//! Updates a singular element at the specified position	\param x X coordinate. Must be less than GetWidth() \param y Y coordinate. Must be less than GetHeight() @param value Value to set \return Reference to the requested element
				{
					Super::data[ToLinearIndex(x,y)] = value;
				}
			
				C&			Get(const Index&idx)
				{
					return Get(idx.x,idx.y);
				}
				C&			Get(index_t x, index_t y)	//! Retrieves a singular element at the specified position	\param x X coordinate. Must be less than GetWidth() \param y Y coordinate. Must be less than GetHeight() \return Reference to the requested element
				{
					return Super::data[ToLinearIndex(x,y)];
				}
				C&			Get(const Iterator<true>&x, const Iterator<false>&y)	//! Retrieves a singular element at the specified position	\param x X coordinate. Must be less than GetWidth() \param y Y coordinate. Must be less than GetHeight() \return Reference to the requested element
				{
					return Get(*x,*y);
				}
			
				C&			Get(const Iterator<true>&x, index_t y)	//! Retrieves a singular element at the specified position	\param x X coordinate. Must be less than GetWidth() \param y Y coordinate. Must be less than GetHeight() \return Reference to the requested element
				{
					return Get(*x,y);
				}

				C&			Get(index_t x, const Iterator<false>&y)	//! Retrieves a singular element at the specified position	\param x X coordinate. Must be less than GetWidth() \param y Y coordinate. Must be less than GetHeight() \return Reference to the requested element
				{
					return Get(x,*y);
				}
		
				const C&	Get(const Index&idx) const
				{
					return Get(idx.x,idx.y);
				}
				const C&	Get(index_t x, index_t y)	const	//! Retrieves a singular element at the specified position \param x X coordinate. Must be less than GetWidth() \param y Y coordinate. Must be less than GetHeight() \return Reference to the requested element
				{
					#ifdef __ARRAY_DBG_RANGE_CHECK__
						if (x >= w || y >= GetHeight())
							FATAL__("Index out of bounds");
					#endif
					return Super::data[y*w+x];
				}
				const C&	Get(const Iterator<true>&x, const Iterator<false>&y)	const	//! Retrieves a singular element at the specified position \param x X coordinate. Must be less than GetWidth() \param y Y coordinate. Must be less than GetHeight() \return Reference to the requested element
				{
					return Get(*x,*y);
				}
			
				const C&	Get(const Iterator<true>&x, index_t y)	const //! Retrieves a singular element at the specified position	\param x X coordinate. Must be less than GetWidth() \param y Y coordinate. Must be less than GetHeight() \return Reference to the requested element
				{
					return Get(*x,y);
				}

				const C&	Get(index_t x, const Iterator<false>&y)	const //! Retrieves a singular element at the specified position	\param x X coordinate. Must be less than GetWidth() \param y Y coordinate. Must be less than GetHeight() \return Reference to the requested element
				{
					return Get(x,*y);
				}

				void	adoptData(Self&other)	//! Adopts pointer and size and sets both NULL of the specified origin array.
				{
					w = other.w;
					Super::adoptData(other);
					other.w = 0;
				}
				void	swap(Self&other)
				{
					Super::swap(other);
					swp(w,other.w);
				}
				friend void swap(Self&a, Self&b)
				{
					a.swap(b);
				}

				bool	operator==(const Self&other) const
				{
					return w == other.w && Super::operator==(other);
				}

				bool	operator!=(const Self&other) const
				{
					return w != other.w || Super::operator!=(other);
				}

				friend void			Serialize(IWriteStream&s, const Self&v)
				{
					using Serialization::Serialize;
					s.WriteSize(v.w);
					Serialize(s,(const Super&)v);
				}
				friend void			Deserialize(IReadStream&s, Self&v)
				{
					using Serialization::Deserialize;
					s.ReadSize(v.w);
					Deserialize(s,(Super&)v);
				}
			protected:
				index_t	w;
				
				using Super::SetSize;
				using Super::ResizePreserveContent;
				using Super::erase;
				using Super::Append;
				using Super::Count;

				using Super::data;
			};
	}
}

DECLARE_T2__(Container::Array2D,class,class,SwapStrategy);
