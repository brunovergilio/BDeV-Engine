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
	// The total size will be => [allocation + alignment + (2) u32 bytes + offset]
	// One u32 will store the offset to the next element, while the other stores
	// the offset to the aligned memory, so we can properly free it
	alignment = std::max(alignment, kDefaultAlignSize);
	size = RoundToNearestMultipleP2(size, kMinAllocationSize) + sizeof(u64) + alignment;

	// Make sure we're not going out of bounds
	if (m_pCurrent + size >= m_pEnd)
	{
		return nullptr;
	}

	MemType mem{ m_pCurrent };
	
	// Store the offset to the future m_pCurrent in the current address
	*mem.pAsUIntPtr = u32(size);

	// Align the pointer with the added offset and the 2 u32 bytes
	mem.pAsVoidPtr = BvAlign(mem.pAsCharPtr + sizeof(u64) + offset, alignment);
	// Offset it back
	mem.pAsCharPtr -= offset;

	// Store the offset to the misaligned memory address
	mem.pAsUIntPtr[-1] = u32(mem.pAsCharPtr - m_pCurrent);

	// Move the pointer forward
	m_pCurrent += size;

	// Return the aligned address
	return mem.pAsVoidPtr;
}


void BvStackAllocator::Free(void* ptr)
{
	BvAssert(ptr != nullptr, "Trying to free nullptr");

	MemType mem{ ptr };
	// Move the pointer back by the offset to the misaligned memory address
	mem.pAsCharPtr -= mem.pAsUIntPtr[-1];

	// The misaligned address contains the offset in bytes to the next
	// allocation, so we can get the current pointer offset
	auto pCurrent = mem.pAsCharPtr + *mem.pAsUIntPtr;

	// If the current pointer address is not the same as m_pCurrent,
	// then we know there's been another allocation after it, so we
	// can't roll the memory back - could do an if condition, but
	// will assert instead
	BvAssert(m_pCurrent == pCurrent, "Stack allocator not freeing the last allocation");
	m_pCurrent = mem.pAsCharPtr;
}


size_t BvStackAllocator::GetAllocationSize(void* pMem) const
{
	MemType mem{ pMem };
	// Retrieve the original address
	MemType original{ mem.pAsCharPtr - mem.pAsUIntPtr[-1] };
	// Get the size and move the pointer to the end of this allocation
	MemType end{ original.pAsCharPtr + *original.pAsUIntPtr };

	return end.asSizeT - mem.asSizeT;
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