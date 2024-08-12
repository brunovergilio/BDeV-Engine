#include "BDeV/Core/System/Debug/BvDebug.h"
#include "BDeV/Core/System/Memory/BvMemory.h"
#include "BDeV/Core/System/Windows/BvWindowsHeader.h"
#include "BDeV/Core/System/Threading/BvProcess.h"


void* BvVirtualMemory::Reserve(size_t size)
{
	auto pMem = VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_NOACCESS);
	if (!pMem)
	{
		BvOSCrashIfFailed(pMem);
	}

	return pMem;
}


void BvVirtualMemory::Commit(void* pAddress, size_t size)
{
	auto pMem = VirtualAlloc(pAddress, size, MEM_COMMIT, PAGE_READWRITE);
	if (!pMem)
	{
		BvOSCrashIfFailed(pMem);
	}
}


void* BvVirtualMemory::ReserveAndCommit(size_t size, bool useLargePage)
{
	auto pMem = VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT | (useLargePage ? MEM_LARGE_PAGES : 0), PAGE_READWRITE);
	if (!pMem)
	{
		BvOSCrashIfFailed(pMem);
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
		BvOSCrashIfFailed(result);
	}
}


void BvVirtualMemory::Release(void* pAddress)
{
	auto result = VirtualFree(pAddress, 0, MEM_RELEASE);
	if (!result)
	{
		BvOSCrashIfFailed(result);
	}
}


size_t BvVirtualMemory::GetPageSize()
{
	return BvProcess::GetSystemInfo().m_PageSize;
}


size_t BvVirtualMemory::GetLargePageSize()
{
	return BvProcess::GetSystemInfo().m_LargePageSize;
}