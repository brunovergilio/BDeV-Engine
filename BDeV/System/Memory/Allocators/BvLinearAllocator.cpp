#include "BvLinearAllocator.h"


BvLinearAllocator::BvLinearAllocator(void* pStart, void* pEnd)
	: m_pStart(reinterpret_cast<char*>(pStart)), m_pEnd(reinterpret_cast<char*>(pEnd)),
	m_pCurrent(reinterpret_cast<char*>(pStart))
{
}


BvLinearAllocator::~BvLinearAllocator()
{
}


void* BvLinearAllocator::Allocate(size_t size, size_t alignment, size_t alignmentOffset /*= 0*/)
{
	// The total size will be => [allocation + alignment + alignmentOffset + kPointerSize]
	// The extra 2 kPointerSize bytes will store the address to the next element and
	// the original memory address, so it can have its size calculated
	alignment = std::max(alignment, kDefaultAlignmentSize);
	size += alignment + alignmentOffset + (kPointerSize << 1);

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

	// Move the current pointer forward
	m_pCurrent += size;

	// Return the allocated address
	return alignedMem.pAsVoidPtr;
}


void BvLinearAllocator::Free(void* /*pPtr*/)
{
}


void BvLinearAllocator::Reset()
{
	m_pCurrent = m_pStart;
}


size_t BvLinearAllocator::GetAllocationSize(void* pPtr) const
{
	MemType mem{ pPtr };
	// Retrieve the original address
	MemType original{ reinterpret_cast<void*>(mem.pAsSizeTPtr[-1]) };
	// Get the size and move the pointer to the end of this allocation
	MemType end{ reinterpret_cast<void*>(*original.pAsSizeTPtr) };

	return end.asSizeT - mem.asSizeT;
}


void BvLinearAllocator::Debug()
{
	MemType mem{ m_pStart };
	while (mem.pAsCharPtr < m_pCurrent)
	{
		MemType next{ reinterpret_cast<void*>(*mem.pAsSizeTPtr) };
		auto size = next.asSizeT - mem.asSizeT;
		printf("(0x%p) Block Size: %llu\n", mem.pAsVoidPtr, size);
		mem.pAsCharPtr += size;
	}
}