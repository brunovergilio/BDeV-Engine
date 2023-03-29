#pragma once


#include "BDeV/System/Memory/BvMemoryCommon.h"


class BV_API BvDefaultAllocator
{
	BV_NOCOPYMOVE(BvDefaultAllocator);

public:
	BvDefaultAllocator();
	~BvDefaultAllocator();

	void* Allocate(size_t size, size_t alignment = kDefaultAlignmentSize, size_t alignmentOffset = 0);
	void Free(void* ptr);

	BV_INLINE static size_t GetAllocationSize(void* pMem) { return 0; }

private:
};