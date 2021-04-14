#pragma once


#include "BvCore/System/Memory/BvMemoryCommon.h"


class BvDefaultAllocator
{
	BV_NOCOPYMOVE(BvDefaultAllocator);
public:
	BvDefaultAllocator();
	BvDefaultAllocator(void* pStart, void* pEnd);
	~BvDefaultAllocator();

	void* Allocate(size_t size, size_t alignment, size_t offset = 0);
	void Free(void* ptr);

	BV_INLINE size_t GetAllocationSize(void* pMem) const { return 0; }
};