#pragma once


#include "BvCore/System/Memory/BvMemoryCommon.h"


class BvHeapAllocator
{
	BV_NOCOPYMOVE(BvHeapAllocator);
public:
	BvHeapAllocator(void* pStart, void* pEnd);
	~BvHeapAllocator();

	void* Allocate(size_t size, size_t alignment, size_t offset = 0);
	void Free(void* ptr);

	size_t GetAllocationSize(void* pMem) const;

	void Debug();

private:
	char* m_pStart = nullptr;
	char* m_pEnd = nullptr;
};