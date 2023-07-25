#include "BvMemoryCommon.h"
#include "BDeV/System/Memory/Allocators/BvDefaultAllocator.h"
#include "BDeV/System/Memory/Utilities/BvBoundsChecker.h"
#include "BDeV/System/Memory/Utilities/BvMemoryMarker.h"
#include "BDeV/System/Memory/Utilities/BvMemoryTracker.h"
#include "BDeV/System/Threading/BvSync.h"
#include "BDeV/System/Memory/BvAlloc.h"


BvMemoryAllocator<BvDefaultAllocator, BvNoLock, BvNoBoundsChecker, BvNoMemoryMarker, BvNoMemoryTracker> g_DefaultAllocator;
IBvMemoryAllocator* GetDefaultAllocator()
{
	return &g_DefaultAllocator;
}


void* operator new(std::size_t count)
{
	BvAssert(false, "This shouldn't be called! Use BV_NEW instead!");
	return BvHeapMemory::Alloc(count);
}

void* operator new[](std::size_t count)
{
	BvAssert(false, "This shouldn't be called! Use BV_NEW instead!");
	return BvHeapMemory::Alloc(count);
}

void* operator new(std::size_t count, std::align_val_t al)
{
	BvAssert(false, "This shouldn't be called! Use BV_NEW instead!");
	return BvHeapMemory::Alloc(count, (size_t)al);
}

void* operator new[](std::size_t count, std::align_val_t al)
{
	BvAssert(false, "This shouldn't be called! Use BV_NEW instead!");
	return BvHeapMemory::Alloc(count, (size_t)al);
}

void operator delete(void* ptr)
{
	BvHeapMemory::Free(ptr);
}

void operator delete[](void* ptr)
{
	BvHeapMemory::Free(ptr);
}

void operator delete(void* ptr, std::align_val_t al)
{
	BvHeapMemory::Free(ptr);
}

void operator delete[](void* ptr, std::align_val_t al)
{
	BvHeapMemory::Free(ptr);
}


// Custom
void* operator new  (std::size_t count, const BvSourceInfo& sourceInfo)
{
	PrintF("Default allocation at %s() (%s), line %llu\n", sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);
	return BvHeapMemory::Alloc(count);
}

void* operator new[](std::size_t count, const BvSourceInfo& sourceInfo)
{
	PrintF("Default allocation at %s() (%s), line %llu\n", sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);
	return BvHeapMemory::Alloc(count);
}

void* operator new  (std::size_t count, std::align_val_t al, const BvSourceInfo& sourceInfo)
{
	PrintF("Default allocation at %s() (%s), line %llu\n", sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);
	return BvHeapMemory::Alloc(count, (size_t)al);
}

void* operator new[](std::size_t count, std::align_val_t al, const BvSourceInfo& sourceInfo)
{
	PrintF("Default allocation at %s() (%s), line %llu\n", sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);
	return BvHeapMemory::Alloc(count, (size_t)al);
}

void operator delete  (void* ptr, const BvSourceInfo& sourceInfo)
{
	BvHeapMemory::Free(ptr);
}

void operator delete[](void* ptr, const BvSourceInfo& sourceInfo)
{
	BvHeapMemory::Free(ptr);
}

void operator delete  (void* ptr, std::align_val_t al, const BvSourceInfo& sourceInfo)
{
	BvHeapMemory::Free(ptr);
}

void operator delete[](void* ptr, std::align_val_t al, const BvSourceInfo& sourceInfo)
{
	BvHeapMemory::Free(ptr);
}