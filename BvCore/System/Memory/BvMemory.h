#pragma once


#include "BvCore/System/Memory/BvAlloc.h"

#include "BvCore/System/Memory/Allocators/BvDefaultAllocator.h"
#include "BvCore/System/Memory/Allocators/BvHeapAllocator.h"
#include "BvCore/System/Memory/Allocators/BvLinearAllocator.h"
#include "BvCore/System/Memory/Allocators/BvStackAllocator.h"
#include "BvCore/System/Memory/Allocators/BvPoolAllocator.h"

#include "BvCore/System/Memory/Utilities/BvBoundsChecker.h"
#include "BvCore/System/Memory/Utilities/BvMemoryMarker.h"
#include "BvCore/System/Memory/Utilities/BvMemoryTracker.h"


void SetDefaultAllocator(BvMemoryAllocatorBase& defaultAllocator);
BvMemoryAllocatorBase& GetDefaultAllocator();