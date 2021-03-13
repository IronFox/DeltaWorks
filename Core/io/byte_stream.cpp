#include "../global_root.h"
#include "byte_stream.h"

namespace DeltaWorks
{



	bool ByteReadStream::_TryGetData(void*pntr, size_t size)
	{
		if (current + size > end)
			return false;
		memcpy(pntr, current, size);
		current += size;
		return true;
	}


	void ByteReadStream::_GetData(void*pntr, size_t size)
	{
		if (current + size > end)
			throw Except::Memory::SerializationFault(CLOCATION,"Reached end of stream");
		memcpy(pntr, current, size);
		current += size;
	}
}
