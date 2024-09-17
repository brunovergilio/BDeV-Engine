#include "BvMemory.h"


void* BvMemory::Allocate(size_t size, size_t alignment, size_t alignmentOffset)
{
	BvAssert(size > 0, "Size can't be 0");
	BvAssert((alignment & (alignment - 1)) == 0, "Alignment has to be a power of 2");

	// Allocate memory using malloc - the total size will be the requested size, plus
	// the alignment and alignment offset, and we also add another (kPointerSize * 2)
	// bytes, that way we can store the total allocation size as well as the
	// original address, in order to free it later
	auto totalSize = RoundToNearestMultiple(size + alignment + alignmentOffset, kPointerSize) + (kPointerSize << 1);
	MemType unalignedMem{ std::malloc(totalSize) };

	// We align the memory with the added offset and kPointerSize
	MemType alignedMemWithOffset{ AlignMemory(unalignedMem.pAsCharPtr + alignmentOffset + (kPointerSize << 1), alignment) };

	// We move back the offset bytes
	alignedMemWithOffset.pAsCharPtr -= alignmentOffset;
	// And then we set the originally allocated memory address
	alignedMemWithOffset.pAsSizeTPtr[-1] = unalignedMem.asSizeT;

	// Store the size in the address returned by malloc()
	unalignedMem.pAsSizeTPtr[0] = totalSize;

	// return the memory containing the offset bytes
	return alignedMemWithOffset.pAsVoidPtr;
}


void BvMemory::Free(void* pMem)
{
	if (pMem)
	{
		// Note: if the address was allocated with an alignment offset greater than 0,
		// that offset has to be subtracted from the memory address before it is
		// passed to this function

		// We take the returned address and move back size_t* bytes,
		// since that's where we stored our original pointer
		MemType alignedMemWithOffset{ pMem };

		// We free the address that was stored as a size_t value,
		// so we need to cast it back to void*
		std::free(reinterpret_cast<void*>(alignedMemWithOffset.pAsSizeTPtr[-1]));
	}
}


size_t BvMemory::GetAllocationSize(void* pMem)
{
	MemType address{ pMem };

	// We got back and get the original address
	address.asSizeT = address.pAsSizeTPtr[-1];

	return address.pAsSizeTPtr[0];
}


void* BvMemory::AlignMemory(void* pMem, size_t alignment)
{
	BvAssert(pMem != nullptr, "Address has to be valid");
	BvAssert((alignment & (alignment - 1)) == 0, "Alignment has to be a power of 2");

	const size_t mask = alignment - 1;
	return reinterpret_cast<void*>((reinterpret_cast<size_t>(pMem) + mask) & (~mask));
}