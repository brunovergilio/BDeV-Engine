#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Utils/BvUtils.h"


template<size_t N>
class BvStackMemory final
{
	BV_NOCOPYMOVE(BvStackMemory);

public:
	BvStackMemory() {}
	~BvStackMemory() {}

	operator char* () { return m_Data; }
	size_t GetSize() const { return N; }

private:
	char m_Data[N];
};


class BvHeapMemory final
{
public:
	static void* Alloc(size_t size, size_t alignment = kDefaultAlignmentSize, size_t alignmentOffset = 0);
	static void Free(void* pAddress);
};


class BvVirtualMemory final
{
public:
	static void* Reserve(size_t size);
	static void Commit(void* pAddress, size_t size);
	static void* ReserveAndCommit(size_t size, bool useLargePage = false);
	static void Decommit(void* pAddress, size_t size);
	static void Release(void* pAddress);
};


class BvHeapMemoryArea final
{
	BV_NOCOPYMOVE(BvHeapMemoryArea);

public:
	BvHeapMemoryArea();
	~BvHeapMemoryArea();

	void* Alloc(size_t size, size_t alignment = kDefaultAlignmentSize);
	void Free(void* pMem);
};


class BvVirtualMemoryArea final
{
	BV_NOCOPYMOVE(BvVirtualMemoryArea);

public:
	BvVirtualMemoryArea(size_t reservePageCount, size_t initialPageCommitCount = 0, bool useLargePage = false);
	~BvVirtualMemoryArea();

	void* AllocatePage();
	void* ReserveAndCommitPages(size_t pageCount, bool useLargePage = false);
	void DecommitPage(size_t pageIndex);

private:
	void* m_pAddress{};
	size_t m_PageCount{};
};