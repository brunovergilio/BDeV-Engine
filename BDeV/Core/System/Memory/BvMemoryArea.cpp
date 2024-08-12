#include "BvMemoryArea.h"
#include "BDeV/Core/System/Memory/BvMemory.h"
#include "BDeV/Core/System/Threading/BvProcess.h"


namespace Internal
{
	BvStackAreaImpl::BvStackAreaImpl(void* pMem, size_t size)
		: m_pStart(reinterpret_cast<char*>(pMem)), m_pEnd(m_pStart + size)
	{
	}


	BvStackAreaImpl::~BvStackAreaImpl()
	{
	}
}


BvHeapArea::BvHeapArea(size_t size)
	: m_pStart(reinterpret_cast<char*>(BvMemory::Allocate(size))), m_pEnd(m_pStart + size)
{
}


BvHeapArea::~BvHeapArea()
{
	BvMemory::Free(m_pStart);
}


BvPageArea::BvPageArea(u32 maxPageCount, u32 initialCommitPageCount)
	: m_pStart(reinterpret_cast<char*>(BvVirtualMemory::Reserve(maxPageCount * BvVirtualMemory::GetPageSize()))),
	m_pCurr(m_pStart), m_pEnd(m_pStart + maxPageCount * BvVirtualMemory::GetPageSize())
{
	if (initialCommitPageCount)
	{
		size_t commitSize = BvVirtualMemory::GetPageSize() * initialCommitPageCount;
		BvVirtualMemory::Commit(m_pStart, commitSize);
		m_pCurr += commitSize;
	}
}


BvPageArea::~BvPageArea()
{
}


char* BvPageArea::CommitPages(u32 pageCount)
{
	auto pageSize = BvVirtualMemory::GetPageSize();
	auto availableSize = size_t(m_pEnd - m_pCurr);
	if (availableSize / pageSize < pageCount)
	{
		return nullptr;
	}

	auto commitSize = pageSize * pageCount;
	auto pMem = m_pCurr;
	BvVirtualMemory::Commit(m_pCurr, commitSize);
	m_pCurr += commitSize;

	return pMem;
}