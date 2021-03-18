#include "BvStackAllocator.h"

BvStackAllocator::BvStackAllocator(void* pStart, void* pEnd)
	: m_pStart(reinterpret_cast<char*>(pStart)), m_pEnd(reinterpret_cast<char*>(pEnd)),
	m_pCurrent(reinterpret_cast<char*>(pStart))
{
}


BvStackAllocator::~BvStackAllocator()
{
}


void* BvStackAllocator::Allocate(size_t size, size_t alignment, size_t offset /*= 0*/)
{
	// The total size will be the (rounded) size of the allocation + the alignment + (2) u32 bytes
	// One u32 will store the offset to the latest pointer, while the other stores the
	// offset to the aligned memory, so we can properly free it
	size = RoundToNearestMultiple(size, 4ull) + sizeof(u64) + alignment;

	// Make sure we're not going out of bounds
	if (m_pCurrent + size >= m_pEnd)
	{
		return nullptr;
	}

	MemType mem{ m_pCurrent };
	
	// Store the offset to the future m_pCurrent in the current address
	*mem.pAsUIntPtr = u32(size);

	// Align the memory and take into account the 2 extra u32 bytes
	mem.pAsVoidPtr = BvAlign(mem.pAsCharPtr + sizeof(u64), alignment);

	// Store the offset to the misaligned memory address
	mem.pAsUIntPtr[-1] = u32(mem.pAsCharPtr - m_pCurrent);

	// Move the pointer forward
	m_pCurrent += size;

	// Return the aligned address
	return mem.pAsVoidPtr;
}


void BvStackAllocator::Free(void* ptr)
{
	BvAssertMsg(ptr != nullptr, "Trying to free nullptr");

	MemType mem{ ptr };
	// Move the pointer back by the offset to the misaligned memory address
	mem.pAsCharPtr = mem.pAsCharPtr - mem.pAsUIntPtr[-1];

	// The misaligned address contains the offset in bytes to the next
	// allocation, so we can get the current pointer offset
	auto pCurrent = mem.pAsCharPtr + *mem.pAsUIntPtr;

	// If the current pointer address is not the same as m_pCurrent,
	// then we know there's been another allocation after it, so we
	// can't roll the memory back - could do an if condition, but
	// will assert instead
	BvAssertMsg(m_pCurrent == pCurrent, "Stack allocator not freeing the last allocation");
	m_pCurrent = mem.pAsCharPtr;
}


void BvStackAllocator::Debug()
{
	MemType mem{ m_pStart };
	while (mem.pAsCharPtr < m_pCurrent)
	{
		printf("(0x%p) Block Size: %lu\n", mem.pAsVoidPtr, *mem.pAsUIntPtr);
		mem.pAsCharPtr += *mem.pAsUIntPtr;
	}
}