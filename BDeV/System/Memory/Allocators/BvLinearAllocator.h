#pragma once


#include "BDeV/System/Memory/BvMemoryCommon.h"


class BvLinearAllocator
{
	BV_NOCOPYMOVE(BvLinearAllocator);
public:
	BvLinearAllocator(void* pStart, void* pEnd);
	~BvLinearAllocator();

	void* Allocate(size_t size, size_t alignment = kDefaultAlignmentSize, size_t alignmentOffset = 0);
	void Free(void* pPtr);

	void Reset();

	size_t GetAllocationSize(void* pPtr) const;

	void Debug();

private:
	char* m_pStart = nullptr;
	char* m_pEnd = nullptr;
	char* m_pCurrent = nullptr;
};