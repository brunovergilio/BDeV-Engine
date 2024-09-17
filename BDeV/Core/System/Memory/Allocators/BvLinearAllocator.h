#pragma once


#include "BDeV/Core/System/Memory/BvMemory.h"


class BvLinearAllocator
{
	BV_NOCOPYMOVE(BvLinearAllocator);
public:
	BvLinearAllocator() = default;
	BvLinearAllocator(void* pStart, void* pEnd);
	BvLinearAllocator(size_t size);
	~BvLinearAllocator();

	void Set(void* pStart, void* pEnd);
	void Set(size_t size);

	void* Allocate(size_t size, size_t alignment = kDefaultAlignmentSize, size_t alignmentOffset = 0);
	BV_INLINE void Free(void* /*pMem*/) {}
	size_t GetAllocationSize(void* pMem) const;

	void Reset();

private:
	char* m_pStart = nullptr;
	char* m_pEnd = nullptr;
	char* m_pCurrent = nullptr;
	bool m_HasOwnMemory = false;
};


class BvGrowableLinearAllocator
{
	BV_NOCOPYMOVE(BvGrowableLinearAllocator);
public:
	BvGrowableLinearAllocator() = default;
	BvGrowableLinearAllocator(void* pStart, void* pEnd, size_t growSize = 0);
	BvGrowableLinearAllocator(size_t maxSize, size_t growSize = 0);
	~BvGrowableLinearAllocator();

	void Set(void* pStart, void* pEnd, size_t growSize = 0);
	void Set(size_t maxSize, size_t growSize = 0);

	void* Allocate(size_t size, size_t alignment = kDefaultAlignmentSize, size_t alignmentOffset = 0);
	BV_INLINE void Free(void* /*pMem*/) {}
	size_t GetAllocationSize(void* pMem) const;
	
	void Reset();
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
	bool m_HasOwnMemory = false;
};