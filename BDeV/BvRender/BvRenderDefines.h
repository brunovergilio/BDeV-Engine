#pragma once


// Platform-dependent stuff
#if (BV_PLATFORM == BV_PLATFORM_WIN32)

#define VK_USE_PLATFORM_WIN32_KHR


#else

// Other platforms

#endif // #if (BV_PLATFORM == BV_PLATFORM_WIN32)


#include "BvCore/BvDefines.h"
#include "vulkan/vulkan.h"