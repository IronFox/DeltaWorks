#pragma once

namespace DeltaWorks
{
	#ifdef _DEBUG
	#define __INDEX_DBG_RANGE_CHECK__		//!< Checks index ranges in debug mode
	#endif


	template <typename T>
		class Size2D;
	template <typename T>
		class Size3D;

	template <typename T>
		class Index2D
		{
		public:
			typedef Index2D<T>	Self;
			typedef Size2D<T>	Size;
			
			T			x,
						y;

			explicit	Index2D(T diagonal=0):x(diagonal),y(diagonal)	{}
			/**/		Index2D(T x, T y):x(x),y(y)	{}
			bool		operator==(const Self&other) const {return x == other.x && y == other.y;}
			bool		operator!=(const Self&other) const {return !operator==(other);}
			Self		operator+(const Size&) const;
			Size		operator-(const Self&) const;
			void		Set(T x, T y)
			{
				this->x = x;
				this->y = y;
			}
			/**
			Iterates over all immediate neighbors of the local vector, including diagonal neighbors and itself.
			All created indexes are passed to the provided callback function.
			Indexes outside the range [0,limit.*) are omitted.
			@param function Callback function accepting a single Index parameter. Executed once for each matching index
			@param limit Size constraint. No index is generated where any coordinate is greater or equal to the respective size dimension
			*/
			template <typename F>
			void		IterateNeighborhood(F function, const Size&limit) const;
		};

	template <typename T>
		class Size2D
		{
		public:
			typedef Size2D<T>	Self;
			typedef Index2D<T>	Index;

			T			width,
						height;

			explicit	Size2D(T size=0):width(size),height(size)	{}
			/**/		Size2D(T w, T h):width(w),height(h)	{}
			bool		operator==(const Self&other) const {return width == other.width && height == other.height;}
			bool		operator!=(const Self&other) const {return !operator==(other);}
			count_t		CountElements() const {return width*height;}
			Self		operator-(const Index2D<T>&idx) const
			{
				return Self(width - idx.x,height - idx.y);
			}
			void		Limit(const Self&other)
			{
				using std::min;
				width = min(width,other.width);
				height = min(height,other.height);
			}
			void		Set(T width, T height)
			{
				this->width = width;
				this->height = height;
			}
			void		Set(T diagonalSize)
			{
				width = diagonalSize;
				height = diagonalSize;
			}
			bool		IsInBounds(const Index&idx) const
			{
				return idx.x < width && idx.y < height;
			}
			index_t		ToLinearIndexNoCheck(const Index&idx) const
			{
				return idx.y*width+idx.x;
			}
			index_t		ToLinearIndex(const Index&idx) const
			{
				#ifdef __INDEX_DBG_RANGE_CHECK__
					if (!IsInBounds(idx))
						FATAL__("Index out of bounds");
				#endif
				return ToLinearIndexNoCheck(idx);
			}


			Index		ToVectorIndex(index_t linear) const
			{
				#ifdef __INDEX_DBG_RANGE_CHECK__
					if (linear >= CountElements())
						FATAL__("Index out of bounds");
				#endif
				return ToVectorIndexNoCheck(linear);
			}

			Index		ToVectorIndexNoCheck(index_t linear) const
			{
				return Index(linear % width, linear / width);
			}




			friend Self max(const Self&a, const Self&b)
			{
				using std::max;
				return Self(max(a.width,b.width),max(a.height,b.height));
			}
			friend Self min(const Self&a, const Self&b)
			{
				using std::min;
				return Self(min(a.width,b.width),min(a.height,b.height));
			}
		};

	template <typename T>
		class Index3D
		{
		public:
			typedef Index3D<T>	Self;
			typedef Size3D<T>	Size;

			T			x,
						y,
						z;

			explicit	Index3D(T diagonal=0):x(diagonal),y(diagonal),z(diagonal)	{}
			/**/		Index3D(T x, T y, T z):x(x),y(y),z(z)	{}
			bool		operator==(const Self&other) const {return x == other.x && y == other.y && z == other.z;}
			bool		operator!=(const Self&other) const {return !operator==(other);}
			Self		operator+(const Size&) const;
			Size		operator-(const Self&) const;
			void		Set(T x, T y, T z)
			{
				this->x = x;
				this->y = y;
				this->z = z;
			}
			/**
			Iterates over all immediate neighbors of the local vector, including diagonal neighbors and itself.
			All created indexes are passed to the provided callback function.
			Indexes outside the range [0,limit.*) are omitted.
			@param function Callback function accepting a single Index parameter. Executed once for each matching index
			@param limit Size constraint. No index is generated where any coordinate is greater or equal to the respective size dimension
			*/
			template <typename F>
			void		IterateNeighborhood(F function, const Size&limit) const;
		};

	template <typename T>
		class Size3D
		{
		public:
			typedef Size3D<T>	Self;
			typedef Index3D<T>	Index;

			T			width,
						height,
						depth;

			explicit	Size3D(T size=0):width(size),height(size),depth(size)	{}
			/**/		Size3D(T w, T h, T d):width(w),height(h),depth(d)	{}
			bool		operator==(const Self&other) const {return width == other.width && height == other.height && depth == other.depth;}
			bool		operator!=(const Self&other) const {return !operator==(other);}
			count_t		CountElements() const {return width*height*depth;}
			Self		operator-(const Index3D<T>&idx) const
			{
				return Self(width - idx.x,height - idx.y, depth - idx.z);
			}
			void		Limit(const Self&other)
			{
				using std::min;
				width = min(width,other.width);
				height = min(height,other.height);
				depth = min(depth,other.depth);
			}
			void		Set(T width, T height, T depth)
			{
				this->width = width;
				this->height = height;
				this->depth = depth;
			}
			void		Set(T diagonalSize)
			{
				width = diagonalSize;
				height = diagonalSize;
				depth = diagonalSize;
			}

			bool		IsInBounds(const Index&idx) const
			{
				return idx.x < width && idx.y < height && idx.z < depth;
			}

			index_t		ToLinearIndexNoCheck(const Index&idx) const
			{
				return idx.z*width*height + idx.y*width + idx.x;
			}
			index_t		ToLinearIndex(const Index&idx) const
			{
				#ifdef __INDEX_DBG_RANGE_CHECK__
					if (!IsInBounds(idx))
						FATAL__("Index out of bounds");
				#endif
				return ToLinearIndexNoCheck(idx);
			}

			Index		ToVectorIndex(index_t linear) const
			{
				#ifdef __INDEX_DBG_RANGE_CHECK__
					if (linear >= CountElements())
						FATAL__("Index out of bounds");
				#endif
				return ToVectorIndexNoCheck(linear);
			}

			Index		ToVectorIndexNoCheck(index_t linear) const
			{
				const count_t slice = width*height;
				return Index(linear % width, (linear%slice) / width, linear / slice);
			}

			friend Self max(const Self&a, const Self&b)
			{
				using std::max;
				return Self(max(a.width,b.width),max(a.height,b.height),max(a.depth,b.depth));
			}
			friend Self min(const Self&a, const Self&b)
			{
				using std::min;
				return Self(min(a.width,b.width),min(a.height,b.height),min(a.depth,b.depth));
			}
		};

	template <typename T>
		Index2D<T>	Index2D<T>::operator+(const Size2D<T>&s) const
		{
			return Self(x + s.width, y + s.height);
		}
	template <typename T>
		Size2D<T>	Index2D<T>::operator-(const Index2D<T>&other) const
		{
			return Size2D<T>(
				x > other.x ? x - other.x : 0,
				y > other.y ? y - other.y : 0
				);
		}

	template <typename T>
		template <typename F>
			void		Index2D<T>::IterateNeighborhood(F function, const Size&limit) const
			{
				const T xBegin = x > 1 ? x -1 : 0;
				const T yBegin = y > 1 ? y -1 : 0;
				const T xEnd = x+2 <= limit.width ? x+2 : limit.width;
				const T yEnd = y+2 <= limit.height ? y+2 : limit.height;
				for (T xi = xBegin; xi < xEnd; xi++)
				for (T yi = yBegin; yi < yEnd; yi++)
					function(Self(xi,yi));
			}
	template <typename T>
		template <typename F>
			void		Index3D<T>::IterateNeighborhood(F function, const Size&limit) const
			{
				const T xBegin = x > 1 ? x -1 : 0;
				const T yBegin = y > 1 ? y -1 : 0;
				const T zBegin = z > 1 ? z -1 : 0;
				const T xEnd = x+2 <= limit.width ? x+2 : limit.width;
				const T yEnd = y+2 <= limit.height ? y+2 : limit.height;
				const T zEnd = z+2 <= limit.depth ? z+2 : limit.depth;
				for (T xi = xBegin; xi < xEnd; xi++)
				for (T yi = yBegin; yi < yEnd; yi++)
				for (T zi = zBegin; zi < zEnd; zi++)
					function(Self(xi,yi,zi));
			}


	template <typename T>
		Index3D<T>	Index3D<T>::operator+(const Size3D<T>&s) const
		{
			return Self(x + s.width, y + s.height, z + s.depth);
		}
	template <typename T>
		Size3D<T>	Index3D<T>::operator-(const Index3D<T>&other) const
		{
			return Size3D<T>(
				x > other.x ? x - other.x : 0,
				y > other.y ? y - other.y : 0,
				z > other.z ? z - other.z : 0
				);
		}
	
}
