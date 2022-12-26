#include "../global_root.h"
#include "vectorList.h"

namespace DeltaWorks
{

	namespace Container
	{
		template class PreAllocVector<int>;
		template class Vector<int>;
		template class CommonVectorStorage<int>;
		template class WriteBuffer<>;
	}
}

