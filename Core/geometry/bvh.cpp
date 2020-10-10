#include "../global_root.h"
#include "bvh.h"

namespace DeltaWorks
{
	template BVH<int,1>;
	template BVH<int, 2>;
	template SphereBVH <int, 1>;
	template SphereBVH <int, 2>;

	template BVH<void*,1>;
	template BVH<void*, 2>;
	template SphereBVH <void*, 1>;
	template SphereBVH <void*, 2>;
}

