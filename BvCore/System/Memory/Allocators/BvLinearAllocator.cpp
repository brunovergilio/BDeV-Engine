#include "BvLinearAllocator.h"


BvLinearAllocator::BvLinearAllocator(void* pStart, void* pEnd)
	: m_pStart(reinterpret_cast<char*>(pStart)), m_pEnd(reinterpret_cast<char*>(pEnd)),
	m_pCurrent(reinterpret_cast<char*>(pStart))
{
}


BvLinearAllocator::~BvLinearAllocator()
{
}


void* BvLinearAllocator::Allocate(size_t size, size_t alignment, size_t offset /*= 0*/)
{
	// The total size will be the (rounded) size of the allocation + the
	// alignment + u32 bytes. We only use the extra u32 bytes here to
	// track the allocations for debugging purposes
	size = RoundToNearestMultiple(size, 4ull) + sizeof(u32) + alignment;

	// Make sure we're not going out of bounds
	if (m_pCurrent + size >= m_pEnd)
	{
		return nullptr;
	}

	MemType mem{ m_pCurrent };
	*mem.pAsUIntPtr = u32(size);

	// Move the pointer forward
	m_pCurrent += size;

	// Return the aligned address
	return BvAlign(mem.pAsVoidPtr, alignment);
}


void BvLinearAllocator::Free(void* /*ptr*/)
{
}


void BvLinearAllocator::Reset()
{
	m_pCurrent = m_pStart;
}


void BvLinearAllocator::Debug()
{
	MemType mem{ m_pStart };
	while (mem.pAsCharPtr < m_pCurrent)
	{
		printf("(0x%p) Block Size: %lu\n", mem.pAsVoidPtr, *mem.pAsUIntPtr);
		mem.pAsCharPtr += *mem.pAsUIntPtr;
	}
}