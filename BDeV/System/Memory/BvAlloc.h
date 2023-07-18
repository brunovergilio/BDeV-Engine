#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Utils/BvUtils.h"


template<size_t N>
class BvStackMemoryArea final
{
	BV_NOCOPYMOVE(BvStackMemoryArea);
	
public:
	BvStackMemoryArea() {}
	~BvStackMemoryArea() {}

	operator char*() { return m_Data; }
	size_t GetSize() const { return N; }

private:
	char m_Data[N];
};


class BvHeapMemoryArea final
{
	BV_NOCOPYMOVE(BvHeapMemoryArea);

public:
	static void* Alloc(size_t size, size_t alignment = kDefaultAlignmentSize);
	static void Free(void* pMem);
};


class BvVirtualMemoryArea final
{
	BV_NOCOPYMOVE(BvVirtualMemoryArea);

public:
	BvVirtualMemoryArea();
	~BvVirtualMemoryArea();

	static void* Reserve(size_t size);
	static void Commit(void* pAddress, size_t size);
	static void* ReserveAndCommit(size_t size, bool useLargePage = false);
	static void Decommit(void* pAddress, size_t size);
	static void Release(void* pAddress);

	void* ReservePages(size_t pageCount);
	void CommitPage(size_t pageIndex);
	void* ReserveAndCommitPages(size_t pageCount, bool useLargePage = false);
	void DecommitPage(size_t pageIndex);

private:
	void* m_pAddress{};
	size_t m_PageCount{};
};