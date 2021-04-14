#pragma once


#include "BvCore/System/Memory/BvMemoryCommon.h"


class BvLinearAllocator
{
	BV_NOCOPYMOVE(BvLinearAllocator);
public:
	BvLinearAllocator(void* pStart, void* pEnd);
	~BvLinearAllocator();

	void* Allocate(size_t size, size_t alignment, size_t offset = 0);
	void Free(void* ptr);

	void Reset();

	size_t GetAllocationSize(void* pMem) const;

	void Debug();

private:
	char* m_pStart = nullptr;
	char* m_pEnd = nullptr;
	char* m_pCurrent = nullptr;
};