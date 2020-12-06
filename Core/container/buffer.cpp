#include "../global_root.h"
#include "buffer.h"

namespace DeltaWorks
{

	namespace Container
	{
		template class Buffer<int>;
		template class Buffer0<int>;
		template class CommonBufferStorage<int>;
		template class WriteBuffer<>;
	}
}

