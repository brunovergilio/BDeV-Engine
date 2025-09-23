#include "BvLinearAllocator.h"
#include "BDeV/Core/System/Process/BvProcess.h"


// ===============================================
// Linear Allocator
// ===============================================
BvLinearAllocator::BvLinearAllocator(void* pStart, void* pEnd)
	: m_pStart(reinterpret_cast<char*>(pStart)), m_pEnd(reinterpret_cast<char*>(pEnd)),
	m_pCurrent(m_pStart)
{
}


BvLinearAllocator::BvLinearAllocator(size_t size)
	: m_pStart(reinterpret_cast<char*>(BvMemory::Allocate(std::max(size, kPointerSize)))),
	m_pEnd(m_pStart + std::max(size, kPointerSize)), m_pCurrent(m_pStart), m_HasOwnMemory(true)
{
}


BvLinearAllocator::~BvLinearAllocator()
{
	if (m_HasOwnMemory)
	{
		BvMemory::Free(m_pStart);
	}
}


void BvLinearAllocator::Set(void* pStart, void* pEnd)
{
	BV_ASSERT(m_pStart == nullptr, "Memory already set");
	m_pStart = reinterpret_cast<char*>(pStart);
	m_pEnd = reinterpret_cast<char*>(pEnd);
	m_pCurrent = m_pStart;
}


void BvLinearAllocator::Set(size_t size)
{
	BV_ASSERT(m_pStart == nullptr, "Memory already set");
	m_pStart = reinterpret_cast<char*>(BvMemory::Allocate(size));
	m_pEnd = m_pStart + size;
	m_pCurrent = m_pStart;
	m_HasOwnMemory = true;
}


void* BvLinearAllocator::Allocate(size_t size, size_t alignment, size_t alignmentOffset /*= 0*/)
{
	size = RoundToNearestPowerOf2(size + std::max(alignment, kPointerSize) + alignmentOffset, kPointerSize) + (kPointerSize << 1);

	// Make sure we're not going out of bounds
	if (m_pCurrent + size > m_pEnd)
	{
		return nullptr;
	}

	MemType mem{ m_pCurrent };

	// Store the future m_pCurrent in the current address
	*mem.pAsSizeTPtr = reinterpret_cast<size_t>(m_pCurrent + size);

	// Align the pointer with the added offset
	MemType alignedMem{ BvMemory::AlignMemory(mem.pAsCharPtr + alignmentOffset + (kPointerSize << 1), alignment) };
	// Offset it back
	alignedMem.pAsCharPtr -= alignmentOffset;
	// Store the original memory address
	alignedMem.pAsSizeTPtr[-1] = mem.asSizeT;

	// Move the current pointer forward
	m_pCurrent += size;

	// Return the allocated address
	return alignedMem.pAsVoidPtr;
}


size_t BvLinearAllocator::GetAllocationSize(void* pMem) const
{
	MemType mem{ pMem };
	// Retrieve the original address
	MemType original{ reinterpret_cast<void*>(mem.pAsSizeTPtr[-1]) };
	// Get the size and move the pointer to the end of this allocation
	MemType end{ reinterpret_cast<void*>(*original.pAsSizeTPtr) };

	return end.asSizeT - mem.asSizeT;
}


void BvLinearAllocator::Reset()
{
	m_pCurrent = m_pStart;
}


// ===============================================
// Growable Linear Allocator
// ===============================================
BvGrowableLinearAllocator::BvGrowableLinearAllocator(void* pStart, void* pEnd, size_t growSize)
	: m_pVirtualStart(reinterpret_cast<char*>(pStart)), m_pVirtualEnd(reinterpret_cast<char*>(pEnd)),
	m_pStart(m_pVirtualStart), m_pEnd(m_pVirtualStart), m_pCurrent(m_pVirtualStart)
{
	auto& systemInfo = BvSystem::GetSystemInfo();
	m_GrowSize = growSize > 0 ? RoundToNearestPowerOf2(growSize, systemInfo.m_PageSize) : systemInfo.m_PageSize;
}


BvGrowableLinearAllocator::BvGrowableLinearAllocator(size_t maxSize, size_t growSize)
	: m_HasOwnMemory(true)
{
	auto& systemInfo = BvSystem::GetSystemInfo();
	m_GrowSize = growSize > 0 ? RoundToNearestPowerOf2(growSize, systemInfo.m_PageSize) : systemInfo.m_PageSize;
	maxSize = RoundToNearestPowerOf2(maxSize, m_GrowSize);
	m_pVirtualStart = reinterpret_cast<char*>(BvVirtualMemory::Reserve(maxSize));
	m_pVirtualEnd = m_pVirtualStart + maxSize;
	m_pStart = m_pEnd = m_pCurrent = m_pVirtualStart;
}


BvGrowableLinearAllocator::~BvGrowableLinearAllocator()
{
	if (m_HasOwnMemory)
	{
		BvVirtualMemory::Release(m_pVirtualStart);
	}
}


void BvGrowableLinearAllocator::Set(void* pStart, void* pEnd, size_t growSize)
{
	BV_ASSERT(m_pStart == nullptr, "Memory already set");
	auto& systemInfo = BvSystem::GetSystemInfo();
	m_GrowSize = growSize > 0 ? RoundToNearestPowerOf2(growSize, systemInfo.m_PageSize) : systemInfo.m_PageSize;
	m_pVirtualStart = reinterpret_cast<char*>(pStart);
	m_pVirtualEnd = reinterpret_cast<char*>(pEnd);
	m_pStart = m_pEnd = m_pCurrent = m_pVirtualStart;
}


void BvGrowableLinearAllocator::Set(size_t maxSize, size_t growSize)
{
	BV_ASSERT(m_pStart == nullptr, "Memory already set");
	auto& systemInfo = BvSystem::GetSystemInfo();
	m_GrowSize = growSize > 0 ? RoundToNearestPowerOf2(growSize, systemInfo.m_PageSize) : systemInfo.m_PageSize;
	maxSize = RoundToNearestPowerOf2(maxSize, m_GrowSize);
	m_pVirtualStart = reinterpret_cast<char*>(BvVirtualMemory::Reserve(maxSize));
	m_pVirtualEnd = m_pVirtualStart + maxSize;
	m_pStart = m_pEnd = m_pCurrent = m_pVirtualStart;
	m_HasOwnMemory = true;
}


void* BvGrowableLinearAllocator::Allocate(size_t size, size_t alignment /*= kDefaultAlignmentSize*/, size_t alignmentOffset /*= 0*/)
{
	size = RoundToNearestPowerOf2(size + std::max(alignment, kPointerSize) + alignmentOffset, kPointerSize) + (kPointerSize << 1);

	// Make sure we're not going out of bounds; if we are, try committing more memory
	if (m_pCurrent + size > m_pEnd && !CommitMemory(size - size_t(m_pEnd - m_pCurrent)))
	{
		return nullptr;
	}

	MemType mem{ m_pCurrent };

	// Store the future m_pCurrent in the current address
	*mem.pAsSizeTPtr = reinterpret_cast<size_t>(m_pCurrent + size);

	// Align the pointer with the added offset
	MemType alignedMem{ BvMemory::AlignMemory(mem.pAsCharPtr + alignmentOffset + (kPointerSize << 1), alignment) };
	// Offset it back
	alignedMem.pAsCharPtr -= alignmentOffset;
	// Store the original memory address
	alignedMem.pAsSizeTPtr[-1] = mem.asSizeT;

	// Move the current pointer forward
	m_pCurrent += size;

	// Return the allocated address
	return alignedMem.pAsVoidPtr;
}


size_t BvGrowableLinearAllocator::GetAllocationSize(void* pMem) const
{
	MemType mem{ pMem };
	// Retrieve the original address
	MemType original{ reinterpret_cast<void*>(mem.pAsSizeTPtr[-1]) };
	// Get the size and move the pointer to the end of this allocation
	MemType end{ reinterpret_cast<void*>(*original.pAsSizeTPtr) };

	return end.asSizeT - mem.asSizeT;
}


void BvGrowableLinearAllocator::Reset()
{
	m_pCurrent = m_pStart;
}


void BvGrowableLinearAllocator::Purge()
{
	MemType addressToFree{ m_pCurrent }, end{ m_pEnd };
	addressToFree.pAsVoidPtr = BvMemory::AlignMemory(addressToFree.pAsVoidPtr, m_GrowSize);

	size_t decommitSize = end.asSizeT - addressToFree.asSizeT;
	if (decommitSize)
	{
		BvVirtualMemory::Decommit(addressToFree.pAsVoidPtr, decommitSize);
		m_pEnd = addressToFree.pAsCharPtr;
	}
}


bool BvGrowableLinearAllocator::CommitMemory(size_t size)
{
	// Round the size needed to a multiple of the grow size
	auto sizeNeeded = RoundToNearestPowerOf2(size, m_GrowSize);
	// Check if it's more than we reserved; if it is we can't allocate
	if (m_pEnd + sizeNeeded > m_pVirtualEnd)
	{
		return false;
	}

	BvVirtualMemory::Commit(m_pEnd, sizeNeeded);

	// Increase the physical end address value
	m_pEnd += sizeNeeded;

	return true;
}