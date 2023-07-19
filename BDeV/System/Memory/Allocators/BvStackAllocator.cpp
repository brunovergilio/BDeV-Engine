#include "BvStackAllocator.h"


BvStackAllocator::BvStackAllocator(void* pStart, void* pEnd)
	: m_pStart(reinterpret_cast<char*>(pStart)), m_pEnd(reinterpret_cast<char*>(pEnd)),
	m_pCurrent(reinterpret_cast<char*>(pStart))
{
}


BvStackAllocator::~BvStackAllocator()
{
}


void* BvStackAllocator::Allocate(size_t size, size_t alignment, size_t alignmentOffset /*= 0*/)
{
	// The total size will be => [allocation + alignment + alignmentOffset + kPointerSize]
	// The extra 2 kPointerSize bytes will store the address to the next element and
	// the original memory address, so it can have its size calculated
	size += alignment + (kPointerSize << 1);

	// Make sure we're not going out of bounds
	if (m_pCurrent + size >= m_pEnd)
	{
		return nullptr;
	}

	MemType mem{ m_pCurrent };
	
	// Store the future m_pCurrent in the current address
	*mem.pAsSizeTPtr = reinterpret_cast<size_t>(m_pCurrent + size);

	// Align the pointer with the added offset
	MemType alignedMem{ BvAlignMemory(mem.pAsCharPtr + alignmentOffset + (kPointerSize << 1), alignment) };
	// Offset it back
	alignedMem.pAsCharPtr -= alignmentOffset;
	// Store the original memory address
	alignedMem.pAsSizeTPtr[-1] = mem.asSizeT;

	// Move the pointer forward
	m_pCurrent += size;

	// Return the aligned address
	return alignedMem.pAsVoidPtr;
}


void BvStackAllocator::Free(void* pPtr)
{
	BvAssert(pPtr != nullptr, "Trying to free nullptr");

	MemType mem{ pPtr };

	// Get the original memory address
	mem.pAsVoidPtr = reinterpret_cast<void*>(mem.pAsSizeTPtr[-1]);

	// The original address contains the next address data
	MemType current{ reinterpret_cast<void*>(*mem.pAsSizeTPtr) };

	// If the current pointer address is not the same as m_pCurrent,
	// then we know there's been another allocation after it, so we
	// can't roll the memory back - could do an if condition, but
	// will assert instead
	BvAssert(m_pCurrent == current.pAsCharPtr, "Stack allocator not freeing the last allocation");
	m_pCurrent = mem.pAsCharPtr;
}


size_t BvStackAllocator::GetAllocationSize(void* pPtr) const
{
	MemType mem{ pPtr };
	// Retrieve the original address
	MemType original{ reinterpret_cast<void*>(mem.pAsSizeTPtr[-1]) };
	// Get the size and move the pointer to the end of this allocation
	MemType end{ reinterpret_cast<void*>(*original.pAsSizeTPtr) };

	return end.asSizeT - mem.asSizeT;
}


void BvStackAllocator::Debug()
{
	MemType mem{ m_pStart };
	while (mem.pAsCharPtr < m_pCurrent)
	{
		MemType next{ reinterpret_cast<void*>(*mem.pAsSizeTPtr) };
		auto size = next.asSizeT - mem.asSizeT;
		PrintF("(0x%p) Block Size: %llu\n", mem.pAsVoidPtr, size);
		mem.pAsCharPtr += size;
	}
}