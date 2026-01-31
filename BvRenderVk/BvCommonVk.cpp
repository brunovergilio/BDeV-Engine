#include "BvCommonVk.h"

#if (BV_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable:4100 4189 4127 4324)
#endif

#define VOLK_IMPLEMENTATION
#include "Third Party/volk.h"

#define VMA_IMPLEMENTATION
#include "Third Party/vk_mem_alloc.h"

#if (BV_COMPILER_MSVC)
#pragma warning(pop)
#endif