#include "BvHeapAllocator.h"
#include "BDeV/Core/System/Debug/BvDebug.h"
#include "BDeV/Core/System/Threading/BvProcess.h"


// The block header will store the block size and a
// bit flag to determine whether it's in use or not
constexpr size_t kBlockHeaderSize = sizeof(size_t);
// The block footer will hold the original memory address
// This will be used when merging a freed block with its
// previous memory block
constexpr size_t kBlockFooterSize = sizeof(size_t);
constexpr size_t kBlockInfoSize = kBlockHeaderSize + kBlockFooterSize;
constexpr u64 kMsb = u64(1ull << 63);
constexpr u64 kRevMsb = ~kMsb;


void GetBlockInfo(size_t* pMemory, size_t& size, bool& inUse)
{
	size = pMemory[0] & kRevMsb;
	inUse = (pMemory[0] & kMsb) != 0;
}


void SetBlockInfo(size_t* pMemory, size_t size, bool inUse)
{
	MemType mem{ pMemory };
	size_t inUseBit = (inUse ? 1 : 0);
	constexpr auto kBitsToShift = sizeof(size_t) * CHAR_BIT - 1ull;

	// Set Header
	mem.pAsSizeTPtr[0] = size | (inUseBit << kBitsToShift);
	// Set Footer
	mem.asSizeT += kBlockHeaderSize + size;
	mem.pAsSizeTPtr[0] = reinterpret_cast<size_t>(pMemory);
}


// ===============================================
// Heap Allocator
// ===============================================
BvHeapAllocator::BvHeapAllocator(void* pStart, void* pEnd)
	: m_pStart(reinterpret_cast<char*>(pStart)), m_pEnd(reinterpret_cast<char*>(pEnd))
{
	const size_t size = size_t(m_pEnd - m_pStart);
	BvAssert(size > kBlockInfoSize, "Total memory size is smaller than a single memory block!");
	SetBlockInfo(reinterpret_cast<size_t*>(m_pStart), size - kBlockInfoSize, false);
}


BvHeapAllocator::BvHeapAllocator(size_t size)
	: m_pStart(reinterpret_cast<char*>(BvMemory::Allocate(std::max(size, kBlockInfoSize + kPointerSize)))),
	m_pEnd(m_pStart + std::max(size, kBlockInfoSize + kPointerSize)), m_HasOwnMemory(true)
{
	SetBlockInfo(reinterpret_cast<size_t*>(m_pStart), std::max(size, kBlockInfoSize + kPointerSize) - kBlockInfoSize, false);
}


BvHeapAllocator::~BvHeapAllocator()
{
	if (m_HasOwnMemory)
	{
		BvMemory::Free(m_pStart);
	}
}


void* BvHeapAllocator::Allocate(size_t size, size_t alignment, size_t alignmentOffset /*= 0*/)
{
	size = RoundToNearestPowerOf2(size, kPointerSize) + alignment + alignmentOffset + kPointerSize;
	
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
		MemType alignedMem{ BvMemory::AlignMemory(mem.pAsCharPtr + alignmentOffset + kPointerSize, alignment) };
		// Offset it back
		alignedMem.pAsCharPtr -= alignmentOffset;
		// Store the offset to the misaligned memory address
		alignedMem.pAsSizeTPtr[-1] = mem.asSizeT;
		
		mem.pAsVoidPtr = alignedMem.pAsVoidPtr;
	}

	return mem.pAsVoidPtr;
}


void BvHeapAllocator::Free(void* pMem)
{
	BvAssert(pMem != nullptr, "Trying to free nullptr");

	MemType mem{ pMem };
	// Retrieve the original address
	mem.pAsVoidPtr = reinterpret_cast<void*>(mem.pAsSizeTPtr[-1]);
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
	--mem.pAsSizeTPtr;
	mem.pAsVoidPtr = reinterpret_cast<void*>(*mem.pAsSizeTPtr);

	mem.pAsCharPtr -= kBlockHeaderSize;
	
	size_t blockSize;
	bool inUse;
	GetBlockInfo(mem.pAsSizeTPtr, blockSize, inUse);

	return blockSize;
}


// ===============================================
// Growable Heap Allocator
// ===============================================
BvGrowableHeapAllocator::BvGrowableHeapAllocator(size_t maxSize, size_t growSize)
{
	auto& systemInfo = BvProcess::GetSystemInfo();
	m_GrowSize = growSize > 0 ? RoundToNearestPowerOf2(growSize, systemInfo.m_PageSize) : systemInfo.m_PageSize;
	maxSize = RoundToNearestPowerOf2(maxSize, m_GrowSize);
	m_pVirtualStart = reinterpret_cast<char*>(BvVirtualMemory::Reserve(maxSize));
	m_pVirtualEnd = m_pVirtualStart + maxSize;
	m_pStart = m_pEnd = m_pUsedAddressEnd = m_pVirtualStart;
}


BvGrowableHeapAllocator::~BvGrowableHeapAllocator()
{
	BvVirtualMemory::Release(m_pVirtualStart);
}


void* BvGrowableHeapAllocator::Allocate(size_t size, size_t alignment, size_t alignmentOffset)
{
	size = RoundToNearestPowerOf2(size, kPointerSize) + alignment + alignmentOffset + kPointerSize;

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

	// If we don't have a free block or one that meets our needs, try committing more pages
	if (mem.pAsVoidPtr == nullptr)
	{
		if (CommitMemory(size))
		{
			// We store the original end address value
			mem.pAsCharPtr = end.pAsCharPtr;
			GetBlockInfo(mem.pAsSizeTPtr, blockSize, blockInUse);
		}
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

		auto pUsedAddressEnd = mem.pAsCharPtr + kBlockInfoSize + size;
		if (pUsedAddressEnd > m_pUsedAddressEnd)
		{
			m_pUsedAddressEnd = pUsedAddressEnd;
		}

		// Move the address by the block header size
		mem.pAsCharPtr += kBlockHeaderSize;

		// Align the pointer with the added offset
		MemType alignedMem{ BvMemory::AlignMemory(mem.pAsCharPtr + alignmentOffset + kPointerSize, alignment) };
		// Offset it back
		alignedMem.pAsCharPtr -= alignmentOffset;
		// Store the offset to the misaligned memory address
		alignedMem.pAsSizeTPtr[-1] = mem.asSizeT;

		mem.pAsVoidPtr = alignedMem.pAsVoidPtr;
	}

	return mem.pAsVoidPtr;
}


void BvGrowableHeapAllocator::Free(void* pMem)
{
	BvAssert(pMem != nullptr, "Trying to free nullptr");

	MemType mem{ pMem };
	// Retrieve the original address
	mem.pAsVoidPtr = reinterpret_cast<void*>(mem.pAsSizeTPtr[-1]);
	// Now go back kBlockHeaderSize bytes to retrieve the block info
	mem.pAsCharPtr -= kBlockHeaderSize;

	size_t blockSize;
	bool inUse;
	GetBlockInfo(mem.pAsSizeTPtr, blockSize, inUse);
	SetBlockInfo(mem.pAsSizeTPtr, blockSize, false);

	auto pUsedAddressEnd = mem.pAsCharPtr + kBlockInfoSize + blockSize;
	if (pUsedAddressEnd == m_pUsedAddressEnd)
	{
		m_pUsedAddressEnd = mem.pAsCharPtr;
	}

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

			// If we're coallescing with a previous block and this is
			// the used end address, we update it to the previous
			// block's address
			if (m_pUsedAddressEnd == mem.pAsCharPtr)
			{
				m_pUsedAddressEnd = prev.pAsCharPtr;
			}
		}
	}
}


size_t BvGrowableHeapAllocator::GetAllocationSize(void* pMem) const
{
	MemType mem{ pMem };
	// Retrieve the original address
	--mem.pAsSizeTPtr;
	mem.pAsVoidPtr = reinterpret_cast<void*>(*mem.pAsSizeTPtr);

	mem.pAsCharPtr -= kBlockHeaderSize;

	size_t blockSize;
	bool inUse;
	GetBlockInfo(mem.pAsSizeTPtr, blockSize, inUse);

	return blockSize;
}


void BvGrowableHeapAllocator::Purge()
{
	MemType addressToFree{ m_pUsedAddressEnd }, end{ m_pEnd };

	addressToFree.pAsVoidPtr = BvMemory::AlignMemory(addressToFree.pAsVoidPtr, m_GrowSize);
	size_t decommitSize = end.asSizeT - addressToFree.asSizeT;
	if (decommitSize)
	{
		BvVirtualMemory::Decommit(addressToFree.pAsVoidPtr, decommitSize);
		m_pEnd = addressToFree.pAsCharPtr;

		// If we still have any space left in the current page, we need to update the block info
		auto sizeLeft = addressToFree.asSizeT - size_t(m_pUsedAddressEnd);
		if (sizeLeft > 0)
		{
			size_t blockSize;
			bool blockInUse;

			// Try to update the next block's size
			if (sizeLeft >= kBlockInfoSize + kPointerSize)
			{
				MemType nextBlock{ m_pUsedAddressEnd };
				SetBlockInfo(nextBlock.pAsSizeTPtr, sizeLeft - kBlockInfoSize, false);
			}
			// Otherwise just update the last block's size
			else
			{
				MemType lastBlock{ m_pUsedAddressEnd };
				lastBlock.asSizeT = lastBlock.pAsSizeTPtr[-1];

				GetBlockInfo(lastBlock.pAsSizeTPtr, blockSize, blockInUse);
				SetBlockInfo(lastBlock.pAsSizeTPtr, blockSize + sizeLeft, blockInUse);
			}
		}
	}
}


bool BvGrowableHeapAllocator::CommitMemory(size_t size)
{
	// Round the size needed to a multiple of the grow size
	auto sizeNeeded = RoundToNearestPowerOf2(size + kBlockInfoSize, m_GrowSize);
	// Check if it's more than we reserved; if it is we can't allocate
	if (m_pEnd + sizeNeeded > m_pVirtualEnd)
	{
		return false;
	}

	BvVirtualMemory::Commit(m_pEnd, sizeNeeded);
	SetBlockInfo(reinterpret_cast<size_t*>(m_pEnd), sizeNeeded - kBlockInfoSize, false);

	// Increase the physical end address value
	m_pEnd += sizeNeeded;

	return true;
}