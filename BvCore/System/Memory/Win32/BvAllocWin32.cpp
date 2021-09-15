#include "BvAllocWin32.h"
#include "BvCore/Utils/Win32/BvDebugWin32.h"
#include <Windows.h>


void* BvVMReserve(size_t size)
{
	auto pMem = VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_NOACCESS);
	if (!pMem)
	{
		BV_WIN32_ERROR();
	}
	return nullptr;
}


void BvVMCommit(void* pAddress, size_t size)
{
	auto pMem = VirtualAlloc(pAddress, size, MEM_COMMIT, PAGE_READWRITE);
	if (!pMem)
	{
		BV_WIN32_ERROR();
	}
}


void* BvVMReserveAndCommit(size_t size)
{
	auto pMem = VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!pMem)
	{
		BV_WIN32_ERROR();
	}

	return pMem;
}


void BvVMDecommit(void* pAddress, size_t size)
{
	auto result = VirtualFree(pAddress, size, MEM_DECOMMIT);
	if (!result)
	{
		BV_WIN32_ERROR();
	}
}


void BvVMRelease(void* pAddress)
{
	auto result = VirtualFree(pAddress, 0, MEM_RELEASE);
	if (!result)
	{
		BV_WIN32_ERROR();
	}
}