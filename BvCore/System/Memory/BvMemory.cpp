#include "BvMemory.h"


#include "BvCore/System/Threading/BvSync.h"


BvMemoryAllocator<BvDefaultAllocator, BvNoLock, BvNoBoundsChecker, BvNoMemoryMarker, BvNoMemoryTracker> g_DefaultAllocator;
BvMemoryAllocatorBase* g_pCurrentDefaultAllocator = &g_DefaultAllocator;


void SetDefaultAllocator(BvMemoryAllocatorBase& defaultAllocator)
{
    BvAssert(&defaultAllocator != nullptr, "Default allocator can't be nullptr.");
    g_pCurrentDefaultAllocator = &defaultAllocator;
}


BvMemoryAllocatorBase& GetDefaultAllocator()
{
    return *g_pCurrentDefaultAllocator;
}