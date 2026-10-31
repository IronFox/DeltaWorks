#ifndef bz2H
#define bz2H
#include <stddef.h>

namespace BZ2
{
		/*!
			\brief Compresses generic data using BZ2 compression
			
			\param source Pointer to the first byte of the data that should be compressed
			\param source_size Size in bytes of the data to compress
			\param target Pointer to the first byte of the destination space
			\param target_space Target buffer space
			\return Compressed size or 0 if compression failed
		*/
        size_t    	compress(const void*source, size_t source_size, void*target, size_t target_space);
        size_t		decompress(const void*source, size_t source_size, void*target, size_t target_space);
        const char* errorStr();
        int         errorCode();
};

#endif
