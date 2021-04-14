#pragma once


#include "BvCore/System/Memory/BvMemoryCommon.h"


class BvPoolAllocator
{
	BV_NOCOPYMOVE(BvPoolAllocator);
public:
	BvPoolAllocator(void* pStart, void* pEnd, size_t elementSize, size_t alignment);
	~BvPoolAllocator();

	// The parameters are provided so the class can be used as a template
	void* Allocate(size_t size = 0, size_t alignment = 0, size_t offset = 0);
	void Free(void* ptr);

	BV_INLINE size_t GetAllocationSize(void*) const { return m_ElementSize; }

	void Debug();

private:
	char* m_pMem = nullptr;
	size_t m_ElementSize = 0;
};