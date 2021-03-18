#include "BvPoolAllocator.h"


BvPoolAllocator::BvPoolAllocator(void* pStart, void* pEnd, size_t elementSize, size_t alignment)
{
	MemType start{ pStart }, end{ pEnd };
	start.pAsVoidPtr = BvAlign(start.pAsVoidPtr, alignment);
	m_pMem = start.pAsCharPtr;

	// Check how many elements we can store after the alignment
	size_t elementCount = (end.asSizeT - start.asSizeT) / alignment;

	// Traverse the addresses and set up the linked list
	MemType curr{ m_pMem };
	for (auto i = 0; i < elementCount - 1; i++)
	{
		// Make the current address store the value of the next pointer
		*curr.pAsSizeTPtr = curr.asSizeT + elementSize;
		// Increment the current address by the element size
		curr.pAsCharPtr += elementSize;
	}
	// The last element shouldn't point to anything
	*curr.pAsSizeTPtr = 0;
}


BvPoolAllocator::~BvPoolAllocator()
{
}


void* BvPoolAllocator::Allocate(size_t size /*= 0*/, size_t alignment /*= 0*/, size_t offset /*= 0*/)
{
	// Get the address (already aligned)
	MemType mem{ m_pMem };
	void* pMem = mem.pAsVoidPtr;

	if (mem.pAsVoidPtr)
	{
		// Move the pointer forward to the next element
		mem.asSizeT = *mem.pAsSizeTPtr;

		// Make the current pointer the new head
		m_pMem = mem.pAsCharPtr;
	}

	return pMem;
}


void BvPoolAllocator::Free(void* ptr)
{
	BvAssertMsg(ptr != nullptr, "Trying to free nullptr");

	MemType mem{ ptr };
	MemType head{ m_pMem };

	// Assign the head pointer's current address to the pointer
	// being returned to the pool
	*mem.pAsSizeTPtr = head.asSizeT;

	// Make the returned pointer the new head
	m_pMem = mem.pAsCharPtr;
}


void BvPoolAllocator::Debug()
{
	MemType mem{ m_pMem };
	while (mem.pAsVoidPtr)
	{
		printf("(0x%p) Block Size: Fixed\n", mem.pAsVoidPtr);
		mem.asSizeT = *mem.pAsSizeTPtr;
	}
}