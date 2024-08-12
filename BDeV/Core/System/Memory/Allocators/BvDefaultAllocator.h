#pragma once


#include "BDeV/Core/System/Memory/BvMemory.h"


class BvDefaultAllocator
{
	BV_NOCOPYMOVE(BvDefaultAllocator);
public:
	BvDefaultAllocator(void* pStart, void* pEnd);
	~BvDefaultAllocator();

	void* Allocate(size_t size, size_t alignment = kDefaultAlignmentSize, size_t alignmentOffset = 0);
	void Free(void* pMem);
	size_t GetAllocationSize(void* pMem) const;
};