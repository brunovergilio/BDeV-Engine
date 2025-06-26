#pragma once


#include "BDeV/Core/System/Memory/BvMemory.h"


class BvPoolAllocator
{
	BV_NOCOPYMOVE(BvPoolAllocator);
public:
	BvPoolAllocator() = default;
	BvPoolAllocator(void* pStart, void* pEnd, size_t elementSize, size_t alignment);
	BvPoolAllocator(size_t size, size_t elementSize, size_t alignment);
	~BvPoolAllocator();

	void Set(void* pStart, void* pEnd, size_t elementSize, size_t alignment);
	void Set(size_t size, size_t elementSize, size_t alignment);

	// The parameters are provided so the class can be used as a template parameter
	void* Allocate(size_t size = 0, size_t alignment = 0, size_t alignmentOffset = 0);
	void Free(void* pMem);
	BV_INLINE size_t GetAllocationSize(void*) const { return m_ElementSize; }

private:
	void* m_pBaseAddress = nullptr;
	void* m_pFreeList = nullptr;
	size_t m_ElementSize = 0;
	bool m_HasOwnMemory = false;
};


class BvGrowablePoolAllocator
{
	BV_NOCOPYMOVE(BvGrowablePoolAllocator);
public:
	BvGrowablePoolAllocator() = default;
	BvGrowablePoolAllocator(void* pStart, void* pEnd, size_t growSize, size_t elementSize, size_t alignment);
	BvGrowablePoolAllocator(size_t maxSize, size_t growSize, size_t elementSize, size_t alignment);
	~BvGrowablePoolAllocator();

	// The parameters are provided so the class can be used as a template parameter
	void* Allocate(size_t size = 0, size_t alignment = 0, size_t alignmentOffset = 0);
	void Free(void* pMem);
	BV_INLINE size_t GetAllocationSize(void*) const { return m_ElementSize; }
	
	void Purge();

private:
	bool CommitMemory();

private:
	char* m_pVirtualBase = nullptr;
	char* m_pVirtualStart = nullptr;
	char* m_pVirtualEnd = nullptr;
	char* m_pCurrent = nullptr;
	size_t m_GrowSize = 0;

	void* m_pFreeList = nullptr;
	size_t m_ElementSize = 0;

	bool m_HasOwnMemory = false;
};