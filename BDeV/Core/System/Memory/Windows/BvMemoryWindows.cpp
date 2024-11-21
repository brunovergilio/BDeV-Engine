#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BDeV/Core/System/Memory/BvMemory.h"
#include "BDeV/Core/System/BvPlatformHeaders.h"
#include "BDeV/Core/System/Process/BvProcess.h"


void* BvVirtualMemory::Reserve(size_t size)
{
	return VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_NOACCESS);
}


bool BvVirtualMemory::Commit(void* pAddress, size_t size)
{
	return VirtualAlloc(pAddress, size, MEM_COMMIT, PAGE_READWRITE) != nullptr;
}


void* BvVirtualMemory::ReserveAndCommit(size_t size, bool useLargePage)
{
	return VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT | (useLargePage ? MEM_LARGE_PAGES : 0), PAGE_READWRITE);
}


bool BvVirtualMemory::Decommit(void* pAddress, size_t size)
{
#pragma warning(push)
#pragma warning(disable:6250)
	// From https://github.com/microsoft/mimalloc/issues/311:
	// This warning is wrong for us: we use VirtualFree to specifically decommit memory here (not to free the address range).
	// So, the code is good : -) msdn
	return VirtualFree(pAddress, size, MEM_DECOMMIT) == TRUE;
#pragma warning(pop)
}


bool BvVirtualMemory::Release(void* pAddress)
{
	return VirtualFree(pAddress, 0, MEM_RELEASE) == TRUE;
}


size_t BvVirtualMemory::GetPageSize()
{
	return BvSystem::GetSystemInfo().m_PageSize;
}


size_t BvVirtualMemory::GetLargePageSize()
{
	return BvSystem::GetSystemInfo().m_LargePageSize;
}