#include "BvAllocWindows.h"
#include "BDeV/System/Debug/BvDebug.h"
#include <Windows.h>


void* BvVMReserve(size_t size)
{
	auto pMem = VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_NOACCESS);
	if (!pMem)
	{
		BV_OS_ERROR();
	}
	return nullptr;
}


void BvVMCommit(void* pAddress, size_t size)
{
	auto pMem = VirtualAlloc(pAddress, size, MEM_COMMIT, PAGE_READWRITE);
	if (!pMem)
	{
		BV_OS_ERROR();
	}
}


void* BvVMReserveAndCommit(size_t size)
{
	auto pMem = VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!pMem)
	{
		BV_OS_ERROR();
	}

	return pMem;
}


void BvVMDecommit(void* pAddress, size_t size)
{
# pragma warning(push)
# pragma warning(disable:6250)
	// From https://github.com/microsoft/mimalloc/issues/311:
	// This warning is wrong for us: we use VirtualFree to specifically decommit memory here (not to free the address range).
	// So, the code is good : -) msdn
	auto result = VirtualFree(pAddress, size, MEM_DECOMMIT);
# pragma warning( pop )
	if (!result)
	{
		BV_OS_ERROR();
	}
}


void BvVMRelease(void* pAddress)
{
	auto result = VirtualFree(pAddress, 0, MEM_RELEASE);
	if (!result)
	{
		BV_OS_ERROR();
	}
}