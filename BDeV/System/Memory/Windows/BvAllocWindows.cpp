#include "BDeV/System/Memory/BvAlloc.h"
#include "BDeV/System/Debug/BvDebug.h"
#include <Windows.h>


void* BvHeapMemory::Alloc(size_t size, size_t alignment)
{
	return _aligned_malloc(size, alignment);
}


void BvHeapMemory::Free(void* pMem)
{
	_aligned_free(pMem);
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