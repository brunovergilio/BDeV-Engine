#pragma once


#include "BDeV/Core/System/Memory/BvMemory.h"


class BvHeapAllocator
{
	BV_NOCOPYMOVE(BvHeapAllocator);
public:
	BvHeapAllocator() = default;
	BvHeapAllocator(void* pStart, void* pEnd);
	BvHeapAllocator(size_t size);
	~BvHeapAllocator();

	void Set(void* pStart, void* pEnd);
	void Set(size_t size);

	void* Allocate(size_t size, size_t alignment = kDefaultAlignmentSize, size_t alignmentOffset = 0);
	void Free(void* pMem);
	size_t GetAllocationSize(void* pMem) const;

private:
	char* m_pStart = nullptr;
	char* m_pEnd = nullptr;
	bool m_HasOwnMemory = false;
};


class BvGrowableHeapAllocator
{
	BV_NOCOPYMOVE(BvGrowableHeapAllocator);
public:
	BvGrowableHeapAllocator() = default;
	BvGrowableHeapAllocator(void* pStart, void* pEnd, size_t growSize = 0);
	BvGrowableHeapAllocator(size_t maxSize, size_t growSize = 0);
	~BvGrowableHeapAllocator();

	void Set(void* pStart, void* pEnd, size_t growSize = 0);
	void Set(size_t maxSize, size_t growSize = 0);

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
	char* m_pUsedAddressEnd = nullptr;
	size_t m_GrowSize = 0;
	bool m_HasOwnMemory = false;
};