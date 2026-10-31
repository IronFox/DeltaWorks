#include "../global_root.h"
#include "bz2.h"
#include <bzlib.h>

static int last_code;


static bool resolveError(int code)
{
    if (code == BZ_OK)
        return false;
    last_code = code;
    return true;
}

size_t BZ2::compress(const void*source, size_t source_size, void*target, size_t target_space)
{
	unsigned int size = unsigned(target_space);
    if (resolveError(BZ2_bzBuffToBuffCompress((char*)target,&size,const_cast<char*>((const char*)source),unsigned(source_size),9,0,30)))
        target_space = 0;
	else
		target_space = size;
    return target_space;
}

size_t BZ2::decompress(const void*source, size_t source_size, void*target, size_t target_space)
{
	unsigned int size = unsigned(target_space);
    if (resolveError(BZ2_bzBuffToBuffDecompress((char*)target,&size,const_cast<char*>((const char*)source),unsigned(source_size),0,0)))
		target_space = 0;
	else
		target_space = size;
	return target_space;
}

const char* BZ2::errorStr()
{
    #define ecase(error)    case error: return "BZ2_error: '"#error"'";
    switch (last_code)
    {
        ecase(BZ_CONFIG_ERROR)
        ecase(BZ_SEQUENCE_ERROR)
        ecase(BZ_PARAM_ERROR)
        ecase(BZ_MEM_ERROR)
        ecase(BZ_DATA_ERROR)
        ecase(BZ_DATA_ERROR_MAGIC)
        ecase(BZ_IO_ERROR)
        ecase(BZ_UNEXPECTED_EOF)
        ecase(BZ_OUTBUFF_FULL)
    }
    #undef ecase
    return "BZ2_error: unknown error";
};
