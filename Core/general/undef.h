#undef  near
//#undef  far	//this breaks some windows kit includes
#undef  index
#undef  max
#undef  min

#undef Bool
//#undef Status

#undef CONFIG_STACK_DEBUG

#undef None
#undef Status
#undef Complex

#define NOMINMAX

#ifndef global_constants_defined
	#define global_constants_defined
	const int None = 0;
#endif
