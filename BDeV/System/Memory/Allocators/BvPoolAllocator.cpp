#include "BvPoolAllocator.h"


BvPoolAllocator::BvPoolAllocator(void* pStart, void* pEnd, size_t elementSize, size_t alignment)
	: m_pHead(BvMemory::AlignMemory(pStart, alignment)), m_ElementSize(elementSize)
{
	BvAssert(elementSize >= kPointerSize, "Element size must be at least the size of a pointer");

	MemType curr{ m_pHead }, end{ pEnd };

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


BvPoolAllocator::~BvPoolAllocator()
{
}


void* BvPoolAllocator::Allocate(size_t, size_t, size_t)
{
	// If we're out of blocks, return nullptr
	if (!m_pHead)
	{
		return nullptr;
	}

	// Take a free block from the beginning of the list
	void* pBlock = m_pHead;
	m_pHead = *((void**)pBlock);

	return pBlock;
}


void BvPoolAllocator::Free(void* pPtr)
{
	BvAssert(pPtr != nullptr, "Trying to free nullptr");

	// Make the free pointer point to the beginning of the list
	void** pHead = (void**)pPtr;
	*pHead = m_pHead;
	// Make the free pointer the beginning of the list
	m_pHead = pPtr;
}


void BvPoolAllocator::Debug()
{
	auto pWalker = (void**)m_pHead;
	while (pWalker)
	{
		BvConsole::PrintF("(0x%p) Block Size: Fixed\n", pWalker);
		pWalker = (void**)*pWalker;
	}
}