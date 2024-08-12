#pragma once


#include "BDeV/Core/System/Memory/BvMemory.h"


class BvStackAllocator
{
	BV_NOCOPYMOVE(BvStackAllocator);
public:
	BvStackAllocator(void* pStart, void* pEnd);
	BvStackAllocator(size_t size);
	~BvStackAllocator();

	void* Allocate(size_t size, size_t alignment = kDefaultAlignmentSize, size_t alignmentOffset = 0);
	void Free(void* pMem);
	size_t GetAllocationSize(void* pMem) const;

private:
	char* m_pStart = nullptr;
	char* m_pEnd = nullptr;
	char* m_pCurrent = nullptr;
	bool m_HasOwnMemory = false;
};


class BvGrowableStackAllocator
{
	BV_NOCOPYMOVE(BvGrowableStackAllocator);
public:
	BvGrowableStackAllocator(size_t maxSize, size_t growSize = 0);
	~BvGrowableStackAllocator();

	void* Allocate(size_t size, size_t alignment = kDefaultAlignmentSize, size_t alignmentOffset = 0);
	void Free(void* pMem);
	size_t GetAllocationSize(void* pMem) const;

	void Purge();

private:
	bool CommitMemory(size_t size);

private:
	char* m_pVirtualStart = nullptr;
	char* m_pVirtualEnd = nullptr;
	char* m_pStart = nullptr;
	char* m_pEnd = nullptr;
	char* m_pCurrent = nullptr;
	size_t m_GrowSize = 0;
};