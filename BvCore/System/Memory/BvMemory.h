#pragma once


#include "BvCore/BvPlatform.h"
#include "BvCore/BvDebug.h"
#include <memory>


constexpr size_t kDefaultAlignSize = 16;


void * BvAlign(void * pAddress, const size_t alignment);
void * BvAlloc(const size_t size, const size_t alignment = kDefaultAlignSize);
void BvFree(void * pAddress);


BV_INLINE void * BvAlign(void * pAddress, const size_t alignment)
{
	BvAssertMsg(pAddress != nullptr, "Address has to be valid");
	BvAssertMsg((alignment & (alignment - 1)) == 0, "Alignment has to be a power of 2");

	const size_t mask = alignment - 1;
	return reinterpret_cast<void *>((reinterpret_cast<size_t>(pAddress) + mask) & ~mask);
}


BV_INLINE void * BvAlloc(const size_t size, const size_t alignment)
{
	//return aligned_alloc(size, alignment);
	//The extra bytes are for storing the alignment done on the address
	constexpr size_t extraBytes = sizeof(size_t);
	void * pAddress = malloc(size + alignment + extraBytes);

	union
	{
		void * pAsVoidPtr;
		size_t * pAsSizeTPtr;
		size_t asSizeT;
	} memOriginal{ pAddress }, memAligned{ nullptr };

	memAligned.pAsVoidPtr = BvAlign(reinterpret_cast<void *>(memOriginal.asSizeT + extraBytes), alignment);
	memAligned.pAsSizeTPtr[-1] = memOriginal.asSizeT;

	return memAligned.pAsVoidPtr;
}


BV_INLINE void BvFree(void * pAddress)
{
	//aligned_free(pAddress);
	if (!pAddress)
	{
		return;
	}

	union
	{
		void * pAsVoidPtr;
		size_t * pAsSizeTPtr;
		size_t asSizeT;
	} mem{ pAddress };

	mem.asSizeT = mem.pAsSizeTPtr[-1];

	free(mem.pAsVoidPtr);
}


template<class Type>
Type GetNextPowerOf2(const Type value)
{
	Type result = 0;
	Type count = sizeof(Type) * 8;
	for (Type i = 0; i < count; i++)
	{
		result = static_cast<Type>(1) << i;
		if (result > value)
		{
			return result;
		}
	}

	return 0;
}