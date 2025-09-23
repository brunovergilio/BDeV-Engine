#pragma once


#include "BDeV/Core/System/Memory/BvMemory.h"


class BvFreeListAllocator
{
	BV_NOCOPYMOVE(BvFreeListAllocator);
public:
	BvFreeListAllocator() = default;
	BvFreeListAllocator(void* pStart, void* pEnd);
	BvFreeListAllocator(size_t size);
	~BvFreeListAllocator();

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


class BvGrowableFreeListAllocator
{
	BV_NOCOPYMOVE(BvGrowableFreeListAllocator);
public:
	BvGrowableFreeListAllocator() = default;
	BvGrowableFreeListAllocator(void* pStart, void* pEnd, size_t growSize = 0);
	BvGrowableFreeListAllocator(size_t maxSize, size_t growSize = 0);
	~BvGrowableFreeListAllocator();

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