#include "BvMemoryCommon.h"
#include "BDeV/System/Memory/Allocators/BvDefaultAllocator.h"
#include "BDeV/System/Memory/Utilities/BvBoundsChecker.h"
#include "BDeV/System/Memory/Utilities/BvMemoryMarker.h"
#include "BDeV/System/Memory/Utilities/BvMemoryTracker.h"
#include "BDeV/System/Threading/BvSync.h"


void* BvAlignMemory(void* pAddress, size_t alignment)
{
	BvAssert(pAddress != nullptr, "Address has to be valid");
	BvAssert((alignment & (alignment - 1)) == 0, "Alignment has to be a power of 2");

	const size_t mask = alignment - 1;
	return reinterpret_cast<void*>((reinterpret_cast<size_t>(pAddress) + mask) & (~mask));
}


void* BvMAlloc(size_t size, size_t alignment, size_t alignmentOffset)
{
	// We adjust the alignment to the minimum required (if needed)
	alignment = std::max(alignment, kDefaultAlignmentSize);

	// Allocate memory using malloc - the total size will be the requested size, plus
	// the alignment and alignment offset, and we also add another kPointerSize bytes
	// in order to store the pointer back to the original address when freeing it
	MemType mem{ malloc(size + alignment + alignmentOffset + kPointerSize) };

	// We align the memory with the added offset and kPointerSize
	MemType alignedMem{ BvAlignMemory(mem.pAsCharPtr + alignmentOffset + kPointerSize, alignment) };

	// We move back the offset bytes
	alignedMem.pAsCharPtr -= alignmentOffset;
	// And then we set the originally allocated memory address
	alignedMem.pAsSizeTPtr[-1] = mem.asSizeT;

	// return the memory containing the offset bytes
	return alignedMem.pAsVoidPtr;
}


void BvFree(void* pAddress)
{
	if (pAddress)
	{
		// We take the returned address and move back size_t* bytes,
		// since that's where we stored our original pointer
		MemType mem{ pAddress };

		// We free the address that was stored as a size_t value,
		// so we need to cast it back to void*
		free(reinterpret_cast<void*>(mem.pAsSizeTPtr[-1]));
	}
}


BvMemoryAllocator<BvDefaultAllocator, BvNoLock, BvNoBoundsChecker, BvNoMemoryMarker, BvNoMemoryTracker> g_DefaultAllocator;
std::atomic<IBvMemoryAllocator*> g_pCurrentDefaultAllocator = &g_DefaultAllocator;


void SetDefaultAllocator(IBvMemoryAllocator* defaultAllocator)
{
	BvAssert(defaultAllocator != nullptr, "Default allocator can't be nullptr.");
	g_pCurrentDefaultAllocator = defaultAllocator;
}


IBvMemoryAllocator* GetDefaultAllocator()
{
	return g_pCurrentDefaultAllocator;
}