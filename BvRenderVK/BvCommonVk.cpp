#include "BvCommonVk.h"

#if (BV_PLATFORM == BV_PLATFORM_WIN32 && BV_COMPILER == BV_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable:4100 4189 4127 4324)

#define VOLK_IMPLEMENTATION
#include "Volk/volk.h"

#define VMA_IMPLEMENTATION
#include "VulkanMemoryAllocator/include/vk_mem_alloc.h"

#pragma warning(pop)
#endif