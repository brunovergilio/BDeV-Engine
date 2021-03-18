#pragma once


#include "BvCore/System/Memory/BvMemory.h"


class BvStackAllocator
{
public:
	BvStackAllocator(void* pStart, void* pEnd);
	~BvStackAllocator();

	void* Allocate(size_t size, size_t alignment, size_t offset = 0);
	void Free(void* ptr);

	void Debug();

private:
	char* m_pStart = nullptr;
	char* m_pEnd = nullptr;
	char* m_pCurrent = nullptr;
};