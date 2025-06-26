#include "BvPoolAllocator.h"
#include "BDeV/Core/System/Process/BvProcess.h"


void MakeFreeList(void* pFreeList, void* pEnd, size_t elementSize)
{
	MemType curr{ pFreeList }, end{ pEnd };

	// Traverse the addresses and set up the linked list
	void** pWalker = nullptr;
	while (curr.pAsCharPtr < end.pAsCharPtr)
	{
		pWalker = (void**)curr.pAsVoidPtr;
		*pWalker = curr.pAsCharPtr + elementSize;
		curr.pAsCharPtr += elementSize;
	}
	// The last element shouldn't point to anything
	*pWalker = nullptr;
}


// ===============================================
// Pool Allocator
// ===============================================
BvPoolAllocator::BvPoolAllocator(void* pStart, void* pEnd, size_t elementSize, size_t alignment)
	: m_pFreeList(BvMemory::AlignMemory(pStart, alignment)), m_ElementSize(std::max(kPointerSize, RoundToNearestPowerOf2(elementSize, alignment)))
{
	MakeFreeList(m_pFreeList, pEnd, m_ElementSize);
}


BvPoolAllocator::BvPoolAllocator(size_t size, size_t elementSize, size_t alignment)
	: m_pBaseAddress(BvMemory::Allocate(RoundToNearestPowerOf2(size, alignment), alignment)), m_pFreeList(m_pBaseAddress),
	m_ElementSize(std::max(kPointerSize, RoundToNearestPowerOf2(elementSize, alignment))), m_HasOwnMemory(true)
{
	MakeFreeList(m_pFreeList, reinterpret_cast<char*>(m_pFreeList) + RoundToNearestPowerOf2(size, alignment), m_ElementSize);
}


BvPoolAllocator::~BvPoolAllocator()
{
	if (m_HasOwnMemory)
	{
		BvMemory::Free(m_pBaseAddress);
	}
}


void BvPoolAllocator::Set(void* pStart, void* pEnd, size_t elementSize, size_t alignment)
{
	BV_ASSERT(m_pFreeList == nullptr, "Memory already set");
	m_pFreeList = BvMemory::AlignMemory(pStart, alignment);
	m_ElementSize = std::max(kPointerSize, RoundToNearestPowerOf2(elementSize, alignment));
	MakeFreeList(m_pFreeList, pEnd, m_ElementSize);
}


void BvPoolAllocator::Set(size_t size, size_t elementSize, size_t alignment)
{
	BV_ASSERT(m_pFreeList == nullptr, "Memory already set");
	m_pBaseAddress = BvMemory::Allocate(RoundToNearestPowerOf2(size, alignment), alignment);
	m_pFreeList = m_pBaseAddress;
	m_ElementSize = std::max(kPointerSize, RoundToNearestPowerOf2(elementSize, alignment));
	m_HasOwnMemory = true;
	MakeFreeList(m_pFreeList, reinterpret_cast<char*>(m_pFreeList) + RoundToNearestPowerOf2(size, alignment), m_ElementSize);
}


void* BvPoolAllocator::Allocate(size_t, size_t, size_t)
{
	// If we're out of blocks, return nullptr
	if (!m_pFreeList)
	{
		return nullptr;
	}

	// Take a free block from the beginning of the list
	void* pBlock = m_pFreeList;
	m_pFreeList = *((void**)pBlock);

	return pBlock;
}


void BvPoolAllocator::Free(void* pMem)
{
	BV_ASSERT(pMem != nullptr, "Trying to free nullptr");

	// Make the free pointer point to the beginning of the list
	void** pHead = (void**)pMem;
	*pHead = m_pFreeList;
	// Make the free pointer the beginning of the list
	m_pFreeList = pMem;
}


// ===============================================
// Growable Pool Allocator
// ===============================================
BvGrowablePoolAllocator::BvGrowablePoolAllocator(void* pStart, void* pEnd, size_t growSize, size_t elementSize, size_t alignment)
{
	auto& systemInfo = BvSystem::GetSystemInfo();
	m_GrowSize = growSize > 0 ? RoundToNearestPowerOf2(growSize, systemInfo.m_PageSize) : systemInfo.m_PageSize;
	size_t maxSize = size_t(pEnd) - size_t(pStart);

	auto numPages = maxSize / systemInfo.m_PageSize;
	auto metadataSize = sizeof(u32) * numPages;

	m_pVirtualBase = reinterpret_cast<char*>(pStart);
	BvVirtualMemory::Commit(m_pVirtualBase, m_GrowSize);
	m_pVirtualStart = reinterpret_cast<char*>(RoundToNearestPowerOf2(size_t(m_pVirtualBase) + metadataSize, alignment));
	m_pVirtualEnd = m_pVirtualBase + maxSize;
	m_pCurrent = m_pVirtualBase + m_GrowSize;
	m_ElementSize = std::max(kPointerSize, RoundToNearestPowerOf2(elementSize, alignment));
	MakeFreeList(m_pVirtualStart, m_pCurrent, m_ElementSize);

	// First page will always be allocated since we need to store metadata information
	// to be able to decommit pages later on
	u32* pPageCount = reinterpret_cast<u32*>(m_pVirtualBase);
	*pPageCount = 1;
}


BvGrowablePoolAllocator::BvGrowablePoolAllocator(size_t maxSize, size_t growSize, size_t elementSize, size_t alignment)
	: m_HasOwnMemory(true)
{
	auto& systemInfo = BvSystem::GetSystemInfo();
	m_GrowSize = growSize > 0 ? RoundToNearestPowerOf2(growSize, systemInfo.m_PageSize) : systemInfo.m_PageSize;
	maxSize = RoundToNearestPowerOf2(maxSize, m_GrowSize);

	auto numPages = maxSize / systemInfo.m_PageSize;
	auto metadataSize = sizeof(u32) * numPages;

	m_pVirtualBase = reinterpret_cast<char*>(BvVirtualMemory::Reserve(maxSize));
	BvVirtualMemory::Commit(m_pVirtualBase, m_GrowSize);
	m_pVirtualStart = reinterpret_cast<char*>(RoundToNearestPowerOf2(size_t(m_pVirtualBase) + metadataSize, alignment));
	m_pVirtualEnd = m_pVirtualBase + maxSize;
	m_pCurrent = m_pVirtualBase + m_GrowSize;
	m_ElementSize = std::max(kPointerSize, RoundToNearestPowerOf2(elementSize, alignment));
	MakeFreeList(m_pVirtualStart, m_pCurrent, m_ElementSize);

	// First page will always be allocated since we need to store metadata information
	// to be able to decommit pages later on
	u32* pPageCount = reinterpret_cast<u32*>(m_pVirtualBase);
	*pPageCount = 1;
}


BvGrowablePoolAllocator::~BvGrowablePoolAllocator()
{
	if (m_HasOwnMemory)
	{
		BvVirtualMemory::Release(m_pVirtualBase);
	}
}


void* BvGrowablePoolAllocator::Allocate(size_t size /*= 0*/, size_t alignment /*= 0*/, size_t alignmentOffset /*= 0*/)
{
	// If we're out of blocks, return nullptr
	if (!m_pFreeList && !CommitMemory())
	{
		return nullptr;
	}

	// Take a free block from the beginning of the list
	void* pBlock = m_pFreeList;
	m_pFreeList = *((void**)pBlock);

	// We update the metadata for the page we just acquired memory from
	auto& systemInfo = BvSystem::GetSystemInfo();
	MemType mem{ pBlock }, start{ m_pVirtualStart };
	auto metadataPageIndex = (mem.asSizeT - start.asSizeT) / systemInfo.m_PageSize;
	u32* pMetadata = reinterpret_cast<u32*>(m_pVirtualBase) + metadataPageIndex;
	++(*pMetadata);

	return pBlock;
}


void BvGrowablePoolAllocator::Free(void* pMem)
{
	BV_ASSERT(pMem != nullptr, "Trying to free nullptr");

	// Make the free pointer point to the beginning of the list
	void** pHead = (void**)pMem;
	*pHead = m_pFreeList;
	// Make the free pointer the beginning of the list
	m_pFreeList = pMem;

	// We update the metadata for the page we just released memory to
	auto& systemInfo = BvSystem::GetSystemInfo();
	MemType mem{ pMem }, start{ m_pVirtualStart };
	auto metadataPageIndex = (mem.asSizeT - start.asSizeT) / systemInfo.m_PageSize;
	u32* pMetadata = reinterpret_cast<u32*>(m_pVirtualBase) + metadataPageIndex;
	--(*pMetadata);
}


void BvGrowablePoolAllocator::Purge()
{
	if (!m_pFreeList)
	{
		return;
	}

	auto& systemInfo = BvSystem::GetSystemInfo();
	MemType start{ m_pVirtualStart }, end{ m_pVirtualEnd }, addressToFree{ m_pVirtualStart };
	auto numPages = (end.asSizeT - start.asSizeT) / systemInfo.m_PageSize;
	u32* pMetadata = reinterpret_cast<u32*>(m_pVirtualBase);
	u32 lastPageInUse = 0;
	for (auto i = 0u; i < numPages; ++i)
	{
		if (pMetadata[i] > 0)
		{
			lastPageInUse = i;
		}
	}

	addressToFree.pAsCharPtr = m_pVirtualStart + systemInfo.m_PageSize * (lastPageInUse + 1);
	auto decommitSize = end.asSizeT - addressToFree.asSizeT;
	if (decommitSize)
	{
		BvVirtualMemory::Decommit(addressToFree.pAsVoidPtr, decommitSize);
		m_pCurrent = addressToFree.pAsCharPtr;
	}
}


bool BvGrowablePoolAllocator::CommitMemory()
{
	// Round the size needed to a multiple of the grow size
	// Check if it's more than we reserved; if it is we can't allocate
	if (m_pCurrent + m_GrowSize > m_pVirtualEnd)
	{
		return false;
	}

	BvVirtualMemory::Commit(m_pCurrent, m_GrowSize);

	m_pFreeList = m_pCurrent;
	MakeFreeList(m_pCurrent, m_pCurrent + m_GrowSize, m_ElementSize);

	// Increase the physical end address value
	m_pCurrent += m_GrowSize;

	return true;
}