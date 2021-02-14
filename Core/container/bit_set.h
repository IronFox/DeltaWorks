#pragma once

#include "array.h"

namespace DeltaWorks
{
	
	namespace Container
	{
		/**
		Auto-resizing compact field of bits, cast to booleans.
		All allocated bits are zero by default
		*/
		class BitSet
		{
		public:
			/**/			BitSet()	{}
			/**/			BitSet(count_t initialSize) :data(CalcDataSize(initialSize)),bitSize(initialSize)
			{
				data.Fill(0);
			}


			/**
			Updates the local data to contain the given amount of bits, growing or shrinking internal
			storage as needed. Any bits still contained by the new field maintain their setting
			@param newSize New number of bits to store
			*/
			void			ResizePreserveContent(count_t newSize)
			{
				count_t oldElements = data.Count();
				count_t newElements = CalcDataSize(newSize);
				data.ResizePreserveContent(newElements);
				for (index_t i = oldElements; i < newElements; i++)
					data[i] = 0;
				bitSize = newSize;
			}

			/**
			Updates the local data to contain the given amount of bits, growing or shrinking internal
			storage as needed. All bits are reset to 0 (false) upon completion
			@param newSize New number of bits to store
			*/
			void			ResizeZero(count_t newSize)
			{
				data.SetSize(CalcDataSize(newSize));
				data.Fill(0);
				bitSize = newSize;
			}

			/**
			Resizes the local storage to 0. Any set bits are purged
			*/
			void			Clear();

			/**
			Flush the internal storage to all 1s.
			*/
			void			ZeroAll();

			/**
			Updates all bits to either 1 or 0 depending on setting
			*/
			void			SetAll(bool toTrue);

			/**
			Sets all bits also set in the remote bit set.
			Resizes the local set if necessary
			*/
			void			Join(const BitSet&other);

			/**
			Retains only bits as true that are also set in the remote bit set.
			Does not resize the local bit set even if all bits
			are set to 0 (false).
			*/
			void			Intersect(const BitSet&other);

			/**
			Inverts the local bit set
			*/
			void			Invert();

			/**
			Checks whether the given bit index is currently set to 1.
			If the index is outside the valid range, the result is always false
			*/
			bool			IsSet(index_t bitIndex) const;

			/**
			Sets the specified bit index.
			If the index is outside the valid range, then the internal storage is
			expanded only if @param toTrue is true.
			*/
			void			Set(index_t bitIndex, bool toTrue=true);

			/**
			Fetches the number of stored bits
			*/
			count_t			size() const {return bitSize;}

			/**
			@copydoc size()
			*/
			count_t			Count() const {return bitSize;}


		private:
			static count_t	CalcDataSize(count_t numBits);

			count_t			bitSize;
			Array<size_t>	data;
		};
	}
}
