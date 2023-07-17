#pragma once


#include "BDeV/System/Memory/BvMemoryCommon.h"


class BvHeapAllocator
{
	BV_NOCOPYMOVE(BvHeapAllocator);
public:
	BvHeapAllocator(void* pStart, void* pEnd);
	~BvHeapAllocator();

	void* Allocate(size_t size, size_t alignment = kDefaultAlignmentSize, size_t alignmentOffset = 0);
	void Free(void* pPtr);

	size_t GetAllocationSize(void* pPtr) const;

	void Debug();

private:
	char* m_pStart = nullptr;
	char* m_pEnd = nullptr;
};