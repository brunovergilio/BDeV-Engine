#pragma once


#include "BDeV/System/Memory/BvMemoryCommon.h"


class BvPoolAllocator
{
	BV_NOCOPYMOVE(BvPoolAllocator);
public:
	BvPoolAllocator(void* pStart, void* pEnd, size_t elementSize, size_t alignment);
	~BvPoolAllocator();

	// The parameters are provided so the class can be used as a template parameter
	void* Allocate(size_t size = 0, size_t alignment = 0, size_t alignmentOffset = 0);
	void Free(void* pPtr);

	BV_INLINE size_t GetAllocationSize(void*) const { return m_ElementSize; }

	void Debug();

private:
	void* m_pHead = nullptr;
	size_t m_ElementSize = 0;
};