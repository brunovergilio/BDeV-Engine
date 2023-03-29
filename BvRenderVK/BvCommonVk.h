#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/System/Debug/BvDebug.h"

// Platform-dependent stuff
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	#define VK_USE_PLATFORM_WIN32_KHR
#else
	// Other platforms
#endif // #if (BV_PLATFORM == BV_PLATFORM_WIN32)


// Defines for the Vulkan Memory Allocator so it uses functions loaded from Volk
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0


#include "Volk/volk.h"