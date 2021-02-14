#include "bit_set.h"


namespace DeltaWorks
{
	namespace Container
	{
		void			BitSet::Clear()
		{
			data.SetSize(0);
			bitSize = 0;
		}

		void			BitSet::ZeroAll()
		{
			data.Fill(0);
		}

		void			BitSet::Join(const BitSet&other)
		{
			if (other.bitSize > bitSize)
				ResizePreserveContent(other.bitSize);

			for (index_t i = 0; i < other.data.Count(); i++)
				data[i] |= other.data[i];
		}

		void			BitSet::Invert()
		{
			foreach (data,d)
				*d = ~*d;
			auto rest = bitSize%sizeof(size_t);
			if (rest)
				data.Last() &= (size_t(1)<<rest)-1;
		}


		void			BitSet::Intersect(const BitSet&other)
		{
			const count_t cnt = std::min(data.Count(),other.data.Count());
			for (index_t i = 0; i < cnt; i++)
				data[i] &= other.data[i];
		}


		void			BitSet::SetAll(bool toTrue)
		{
			if (!toTrue)
			{
				ZeroAll();
				return;
			}

			data.Fill(std::numeric_limits<size_t>::max());
			auto rest = bitSize%sizeof(size_t);
			if (rest)
			{
				size_t lastEntry = (size_t(1)<<rest)-1;
				data.Last() = lastEntry;
			}
		}

		bool			BitSet::IsSet(index_t bitIndex) const
		{
			return bitIndex < bitSize && 
				((data[bitIndex/sizeof(size_t)] >> (bitIndex % sizeof(size_t))) & 1) == 1;
		}

		void			BitSet::Set(index_t bitIndex, bool toTrue/*=true*/)
		{
			if (bitIndex >= bitSize)
			{
				if (!toTrue)
					return;
				ResizePreserveContent(bitIndex+1);
				data[bitIndex/sizeof(size_t)] |= (size_t(1) << (bitIndex % sizeof(size_t)));
				return;
			}
			if (toTrue)
				data[bitIndex/sizeof(size_t)] |= (size_t(1) << (bitIndex % sizeof(size_t)));
			else
				data[bitIndex/sizeof(size_t)] &= ~(size_t(1) << (bitIndex % sizeof(size_t)));
		}

		/*static*/ count_t	BitSet::CalcDataSize(count_t numBits)
		{
			return (numBits+sizeof(size_t)-1) / sizeof(size_t);
		}


	}

}

