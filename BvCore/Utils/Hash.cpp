#include "BvCore/Utils/Hash.h"


u32 FastHash(const void * const pBytes, const size_t size)
{
	const char * const pBytesAsChar = static_cast<const char * const>(pBytes);
	u32 hash = 5381;
	u32 c;

	for (u32 i = 0; i < size; i++)
	{
		c = static_cast<u32>(pBytesAsChar[i]);
		hash = ((hash << 5) + hash) + c;
	}

	return hash;
}