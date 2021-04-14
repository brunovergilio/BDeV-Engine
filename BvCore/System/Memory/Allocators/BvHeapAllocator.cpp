#include "BvHeapAllocator.h"
#include "BvCore/Utils/BvDebug.h"


constexpr size_t kBlockHeaderSize = sizeof(size_t);
constexpr size_t kBlockFooterSize = sizeof(size_t);
constexpr size_t kBlockInfoSize = kBlockHeaderSize + kBlockFooterSize;
constexpr u64 kMsb = u64(1ull << 63);
constexpr u64 kRevMsb = ~kMsb;


void GetBlockInfo(size_t* pMemory, size_t& size, bool& inUse);
void SetBlockInfo(size_t* pMemory, size_t size, bool inUse);


BvHeapAllocator::BvHeapAllocator(void* pStart, void* pEnd)
	: m_pStart(reinterpret_cast<char*>(pStart)), m_pEnd(reinterpret_cast<char*>(pEnd))
{
	const size_t size = size_t(m_pEnd - m_pStart);
	BvAssert(size > kBlockInfoSize, "Total memory size is smaller than a single memory block!");
	SetBlockInfo(reinterpret_cast<size_t*>(m_pStart), size - kBlockInfoSize, false);
}


BvHeapAllocator::~BvHeapAllocator()
{

}


void* BvHeapAllocator::Allocate(size_t size, size_t alignment, size_t offset /*= 0*/)
{
	alignment = std::max(alignment, kDefaultAlignSize);
	size = RoundToNearestMultipleP2(size, kMinAllocationSize) + sizeof(u32) + alignment;
	
	size_t blockSize = 0;
	bool blockInUse = true;

	MemType curr{ m_pStart }, end{ m_pEnd }, mem{ nullptr };

	while (curr.pAsCharPtr < end.pAsCharPtr)
	{
		GetBlockInfo(curr.pAsSizeTPtr, blockSize, blockInUse);

		if (!blockInUse && blockSize >= size)
		{
			mem.pAsCharPtr = curr.pAsCharPtr;
			break;
		}

		curr.pAsCharPtr += kBlockInfoSize + blockSize;
	}

	if (mem.pAsVoidPtr != nullptr)
	{
		size_t remainingBlockSize = blockSize - size;
		// Can't create a free block with the remaining piece, so just use the whole thing
		if (remainingBlockSize <= kBlockInfoSize)
		{
			size = blockSize;
		}
		// If there's enough space for a new free block, set one up
		else
		{
			MemType pNewBlock{ mem.pAsVoidPtr };
			pNewBlock.asSizeT += kBlockInfoSize + size;
			SetBlockInfo(pNewBlock.pAsSizeTPtr, remainingBlockSize - kBlockInfoSize, false);
		}

		// Mark the allocated block as used
		SetBlockInfo(mem.pAsSizeTPtr, size, true);
		
		// Move the address by the block header size
		mem.pAsCharPtr += kBlockHeaderSize;

		// Align the pointer with the added offset
		MemType alignedMem{ BvAlign(mem.pAsCharPtr + sizeof(u32) + offset, alignment) };
		// Offset it back
		alignedMem.pAsCharPtr -= offset;
		// Store the offset to the misaligned memory address
		alignedMem.pAsUIntPtr[-1] = u32(alignedMem.pAsCharPtr - mem.pAsCharPtr);
		
		mem.pAsVoidPtr = alignedMem.pAsVoidPtr;
	}

	return mem.pAsVoidPtr;
}


void BvHeapAllocator::Free(void* ptr)
{
	BvAssert(ptr != nullptr, "Trying to free nullptr");

	MemType mem{ ptr };
	// Retrieve the misaligned address
	mem.asSizeT = mem.asSizeT - size_t(mem.pAsUIntPtr[-1]);
	// Now go back kBlockHeaderSize bytes to retrieve the block info
	mem.pAsCharPtr -= kBlockHeaderSize;

	size_t blockSize;
	bool inUse;
	GetBlockInfo(mem.pAsSizeTPtr, blockSize, inUse);
	SetBlockInfo(mem.pAsSizeTPtr, blockSize, false);

	MemType next{ mem.pAsCharPtr + kBlockInfoSize + blockSize };
	// Check if we're not going ahead of our last memory address
	if (next.pAsCharPtr < m_pEnd)
	{
		size_t nextBlockSize;
		GetBlockInfo(next.pAsSizeTPtr, nextBlockSize, inUse);
		
		// Try to merge with the next block
		if (!inUse)
		{
			blockSize += nextBlockSize + kBlockInfoSize;
			SetBlockInfo(mem.pAsSizeTPtr, blockSize, false);
		}
	}

	// Check if we're not going behind our first memory address
	MemType prev{ mem.pAsVoidPtr };
	if (prev.pAsCharPtr > m_pStart)
	{
		prev.asSizeT = prev.pAsSizeTPtr[-1];

		size_t prevBlockSize;
		GetBlockInfo(prev.pAsSizeTPtr, prevBlockSize, inUse);

		// Try to merge with the previous block
		if (!inUse)
		{
			blockSize += prevBlockSize + kBlockInfoSize;
			SetBlockInfo(prev.pAsSizeTPtr, blockSize, false);
		}
	}
}


size_t BvHeapAllocator::GetAllocationSize(void* pMem) const
{
	MemType mem{ pMem };
	// Retrieve the original address
	MemType original{ mem.pAsCharPtr - mem.pAsUIntPtr[-1] - kBlockHeaderSize };
	
	size_t blockSize;
	bool inUse;
	GetBlockInfo(original.pAsSizeTPtr, blockSize, inUse);
	original.pAsCharPtr += kBlockHeaderSize;

	return blockSize - (mem.asSizeT - original.asSizeT);
}


void BvHeapAllocator::Debug()
{
	MemType curr{ m_pStart };
	MemType end{ m_pEnd };
	size_t size; bool inUse;
	while (curr.asSizeT < end.asSizeT)
	{
		GetBlockInfo(curr.pAsSizeTPtr, size, inUse);
		if (curr.pAsCharPtr > m_pStart)
		{
			printf("Prev(0x%p) ", (void*)curr.pAsSizeTPtr[-1]);
		}
		printf("(0x%p) Block Size: %llu - In use: %s\n", curr.pAsVoidPtr, size, inUse ? "Yes" : "No");
		curr.asSizeT += size + kBlockInfoSize;
	}
}


void GetBlockInfo(size_t* pMemory, size_t& size, bool& inUse)
{
	size = pMemory[0] & kRevMsb;
	inUse = (pMemory[0] & kMsb) != 0;
}


void SetBlockInfo(size_t* pMemory, size_t size, bool inUse)
{
	MemType mem{ pMemory };
	u64 inUseBit = (inUse ? 1 : 0);

	// Set Header
	mem.pAsSizeTPtr[0] = size | (inUseBit << 63);
	// Set Footer
	mem.asSizeT += kBlockHeaderSize + size;
	mem.pAsSizeTPtr[0] = reinterpret_cast<size_t>(pMemory);
}