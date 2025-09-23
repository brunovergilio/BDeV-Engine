#include "BvStackAllocator.h"
#include "BDeV/Core/System/Process/BvProcess.h"


// ===============================================
// Stack Allocator
// ===============================================
BvStackAllocator::BvStackAllocator(void* pStart, void* pEnd)
	: m_pStart(reinterpret_cast<char*>(pStart)), m_pEnd(reinterpret_cast<char*>(pEnd)),
	m_pCurrent(reinterpret_cast<char*>(pStart))
{
}


BvStackAllocator::BvStackAllocator(size_t size)
	: m_pStart(reinterpret_cast<char*>(BvMemory::Allocate(std::max(size, kPointerSize)))),
	m_pEnd(m_pStart + std::max(size, kPointerSize)), m_pCurrent(m_pStart), m_HasOwnMemory(true)
{
}


BvStackAllocator::~BvStackAllocator()
{
	if (m_HasOwnMemory)
	{
		BvMemory::Free(m_pStart);
	}
}


void BvStackAllocator::Set(void* pStart, void* pEnd)
{
	BV_ASSERT(m_pStart == nullptr, "Memory already set");
	m_pStart = reinterpret_cast<char*>(pStart);
	m_pEnd = reinterpret_cast<char*>(pEnd);
	m_pCurrent = m_pStart;
}


void BvStackAllocator::Set(size_t size)
{
	BV_ASSERT(m_pStart == nullptr, "Memory already set");
	m_pStart = reinterpret_cast<char*>(BvMemory::Allocate(size));
	m_pEnd = m_pStart + size;
	m_pCurrent = m_pStart;
	m_HasOwnMemory = true;
}


void* BvStackAllocator::Allocate(size_t size, size_t alignment, size_t alignmentOffset /*= 0*/)
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

	// Move the pointer forward
	m_pCurrent += size;

	// Return the aligned address
	return alignedMem.pAsVoidPtr;
}


void BvStackAllocator::Free(void* pMem)
{
	BV_ASSERT(pMem != nullptr, "Trying to free nullptr");

	MemType mem{ pMem };

	// Get the original memory address
	mem.pAsVoidPtr = reinterpret_cast<void*>(mem.pAsSizeTPtr[-1]);

	// The original address contains the next address data
	MemType current{ reinterpret_cast<void*>(*mem.pAsSizeTPtr) };

	// If the current pointer address is not the same as m_pCurrent,
	// then we know there's been another allocation after it, so we
	// can't roll the memory back - could do an if condition, but
	// will assert instead
	BV_ASSERT(m_pCurrent == current.pAsCharPtr, "Stack allocator not freeing the last allocation");
	m_pCurrent = mem.pAsCharPtr;
}


size_t BvStackAllocator::GetAllocationSize(void* pMem) const
{
	MemType mem{ pMem };
	// Retrieve the original address
	MemType original{ reinterpret_cast<void*>(mem.pAsSizeTPtr[-1]) };
	// Get the size and move the pointer to the end of this allocation
	MemType end{ reinterpret_cast<void*>(*original.pAsSizeTPtr) };

	return end.asSizeT - mem.asSizeT;
}


// ===============================================
// Growable Stack Allocator
// ===============================================
BvGrowableStackAllocator::BvGrowableStackAllocator(void* pStart, void* pEnd, size_t growSize)
	: m_pVirtualStart(reinterpret_cast<char*>(pStart)), m_pVirtualEnd(reinterpret_cast<char*>(pEnd)),
	m_pStart(m_pVirtualStart), m_pEnd(m_pVirtualStart), m_pCurrent(m_pVirtualStart)
{
	auto& systemInfo = BvSystem::GetSystemInfo();
	m_GrowSize = growSize > 0 ? RoundToNearestPowerOf2(growSize, systemInfo.m_PageSize) : systemInfo.m_PageSize;
}


BvGrowableStackAllocator::BvGrowableStackAllocator(size_t maxSize, size_t growSize /*= 0*/)
	: m_HasOwnMemory(true)
{
	auto& systemInfo = BvSystem::GetSystemInfo();
	m_GrowSize = growSize > 0 ? RoundToNearestPowerOf2(growSize, systemInfo.m_PageSize) : systemInfo.m_PageSize;
	maxSize = RoundToNearestPowerOf2(maxSize, m_GrowSize);
	m_pVirtualStart = reinterpret_cast<char*>(BvVirtualMemory::Reserve(maxSize));
	m_pVirtualEnd = m_pVirtualStart + maxSize;
	m_pStart = m_pEnd = m_pCurrent = m_pVirtualStart;
}


BvGrowableStackAllocator::~BvGrowableStackAllocator()
{
	if (m_HasOwnMemory)
	{
		BvVirtualMemory::Release(m_pVirtualStart);
	}
}


void BvGrowableStackAllocator::Set(void* pStart, void* pEnd, size_t growSize)
{
	BV_ASSERT(m_pStart == nullptr, "Memory already set");
	auto& systemInfo = BvSystem::GetSystemInfo();
	m_GrowSize = growSize > 0 ? RoundToNearestPowerOf2(growSize, systemInfo.m_PageSize) : systemInfo.m_PageSize;
	m_pVirtualStart = reinterpret_cast<char*>(pStart);
	m_pVirtualEnd = reinterpret_cast<char*>(pEnd);
	m_pStart = m_pEnd = m_pCurrent = m_pVirtualStart;
}


void BvGrowableStackAllocator::Set(size_t maxSize, size_t growSize)
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


void* BvGrowableStackAllocator::Allocate(size_t size, size_t alignment /*= kDefaultAlignmentSize*/, size_t alignmentOffset /*= 0*/)
{
	size = RoundToNearestPowerOf2(size + std::max(alignment, kPointerSize) + alignmentOffset, kPointerSize) + (kPointerSize << 1);

	// Make sure we're not going out of bounds; if we are, try committing more memory
	if (m_pCurrent + size > m_pEnd && !CommitMemory(size))
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

	// Move the pointer forward
	m_pCurrent += size;

	// Return the aligned address
	return alignedMem.pAsVoidPtr;
}


void BvGrowableStackAllocator::Free(void* pMem)
{
	BV_ASSERT(pMem != nullptr, "Trying to free nullptr");

	MemType mem{ pMem };

	// Get the original memory address
	mem.pAsVoidPtr = reinterpret_cast<void*>(mem.pAsSizeTPtr[-1]);

	// The original address contains the next address data
	MemType current{ reinterpret_cast<void*>(*mem.pAsSizeTPtr) };

	// If the current pointer address is not the same as m_pCurrent,
	// then we know there's been another allocation after it, so we
	// can't roll the memory back - could do an if condition, but
	// will assert instead
	BV_ASSERT(m_pCurrent == current.pAsCharPtr, "Stack allocator not freeing the last allocation");
	m_pCurrent = mem.pAsCharPtr;
}


size_t BvGrowableStackAllocator::GetAllocationSize(void* pMem) const
{
	MemType mem{ pMem };
	// Retrieve the original address
	MemType original{ reinterpret_cast<void*>(mem.pAsSizeTPtr[-1]) };
	// Get the size and move the pointer to the end of this allocation
	MemType end{ reinterpret_cast<void*>(*original.pAsSizeTPtr) };

	return end.asSizeT - mem.asSizeT;
}


void BvGrowableStackAllocator::Purge()
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


bool BvGrowableStackAllocator::CommitMemory(size_t size)
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