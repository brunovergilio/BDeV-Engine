#include "BDeV/System/Memory/BvAlloc.h"
#include "BDeV/System/Debug/BvDebug.h"
#include "BDeV/System/Memory/BvMemoryCommon.h"
#include <Windows.h>


void* BvHeapMemory::Alloc(size_t size, size_t alignment, size_t alignmentOffset)
{
	// Allocate memory using malloc - the total size will be the requested size, plus
	// the alignment and alignment offset, and we also add another kPointerSize bytes
	// in order to store the pointer back to the original address when freeing it
	MemType unalignedMem{ malloc(size + alignment + alignmentOffset + kPointerSize) };

	// We align the memory with the added offset and kPointerSize
	MemType alignedMemWithOffset{ AlignMemory(unalignedMem.pAsCharPtr + alignmentOffset + kPointerSize, alignment) };

	// We move back the offset bytes
	alignedMemWithOffset.pAsCharPtr -= alignmentOffset;
	// And then we set the originally allocated memory address
	alignedMemWithOffset.pAsSizeTPtr[-1] = unalignedMem.asSizeT;

	// return the memory containing the offset bytes
	return alignedMemWithOffset.pAsVoidPtr;
}


void BvHeapMemory::Free(void* pAddress)
{
	if (pAddress)
	{
		// We take the returned address and move back size_t* bytes,
		// since that's where we stored our original pointer
		MemType alignedMemWithOffset{ pAddress };

		// We free the address that was stored as a size_t value,
		// so we need to cast it back to void*
		free(reinterpret_cast<void*>(alignedMemWithOffset.pAsSizeTPtr[-1]));
	}
}


void* BvVirtualMemory::Reserve(size_t size)
{
	auto pMem = VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_NOACCESS);
	if (!pMem)
	{
		BV_OS_ERROR();
	}

	return pMem;
}


void BvVirtualMemory::Commit(void* pAddress, size_t size)
{
	auto pMem = VirtualAlloc(pAddress, size, MEM_COMMIT, PAGE_READWRITE);
	if (!pMem)
	{
		BV_OS_ERROR();
	}
}


void* BvVirtualMemory::ReserveAndCommit(size_t size, bool useLargePage)
{
	auto pMem = VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT | (useLargePage ? MEM_LARGE_PAGES : 0), PAGE_READWRITE);
	if (!pMem)
	{
		BV_OS_ERROR();
	}

	return pMem;
}


void BvVirtualMemory::Decommit(void* pAddress, size_t size)
{
#pragma warning(push)
#pragma warning(disable:6250)
	// From https://github.com/microsoft/mimalloc/issues/311:
	// This warning is wrong for us: we use VirtualFree to specifically decommit memory here (not to free the address range).
	// So, the code is good : -) msdn
	auto result = VirtualFree(pAddress, size, MEM_DECOMMIT);
#pragma warning(pop)
	if (!result)
	{
		BV_OS_ERROR();
	}
}


void BvVirtualMemory::Release(void* pAddress)
{
	auto result = VirtualFree(pAddress, 0, MEM_RELEASE);
	if (!result)
	{
		BV_OS_ERROR();
	}
}