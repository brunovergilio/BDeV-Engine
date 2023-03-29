#include "BvMemory.h"


#include "BDeV/System/Threading/BvSync.h"


BvMemoryAllocator<BvDefaultAllocator, BvNoLock, BvNoBoundsChecker, BvNoMemoryMarker, BvNoMemoryTracker> g_DefaultAllocator;
BvIMemoryAllocator* g_pCurrentDefaultAllocator = &g_DefaultAllocator;


void SetDefaultAllocator(BvIMemoryAllocator& defaultAllocator)
{
    BvAssert(&defaultAllocator != nullptr, "Default allocator can't be nullptr.");
    g_pCurrentDefaultAllocator = &defaultAllocator;
}


BvIMemoryAllocator& GetDefaultAllocator()
{
    return *g_pCurrentDefaultAllocator;
}