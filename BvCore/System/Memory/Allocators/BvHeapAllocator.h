#pragma once


#include "BvCore/System/Memory/BvMemory.h"


class BvHeapAllocator
{
public:
	BvHeapAllocator(void* pStart, void* pEnd);
	~BvHeapAllocator();

	void* Allocate(size_t size, size_t alignment, size_t offset = 0);
	void Free(void* ptr);

	void Debug();

private:
	char* m_pStart = nullptr;
	char* m_pEnd = nullptr;
};