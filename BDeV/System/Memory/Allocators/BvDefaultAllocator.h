#pragma once


#include "BDeV/System/Memory/BvMemoryCommon.h"


class BvDefaultAllocator
{
	BV_NOCOPYMOVE(BvDefaultAllocator);

public:
	BvDefaultAllocator();
	~BvDefaultAllocator();

	void* Allocate(size_t size, size_t alignment = kDefaultAlignmentSize, size_t alignmentOffset = 0);
	void Free(void* ptr);

	size_t GetAllocationSize(void* pMem);

private:
};