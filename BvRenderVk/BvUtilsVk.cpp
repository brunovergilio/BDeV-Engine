#include "BvUtilsVk.h"
#include "BDeV/Core/Container/BvRobinMap.h"
#include "BDeV/Core/Container/BvString.h"
#include "BvTypeConversionsVk.h"


const char* const VkResultToString(const VkResult result)
{
#define VkStringifyCase(result) case result: return #result
	switch (result)
	{
		VkStringifyCase(VK_SUCCESS												);
		VkStringifyCase(VK_NOT_READY											);
		VkStringifyCase(VK_TIMEOUT												);
		VkStringifyCase(VK_EVENT_SET											);
		VkStringifyCase(VK_EVENT_RESET											);
		VkStringifyCase(VK_INCOMPLETE											);
		VkStringifyCase(VK_ERROR_OUT_OF_HOST_MEMORY								);
		VkStringifyCase(VK_ERROR_OUT_OF_DEVICE_MEMORY							);
		VkStringifyCase(VK_ERROR_INITIALIZATION_FAILED							);
		VkStringifyCase(VK_ERROR_DEVICE_LOST									);
		VkStringifyCase(VK_ERROR_MEMORY_MAP_FAILED								);
		VkStringifyCase(VK_ERROR_LAYER_NOT_PRESENT								);
		VkStringifyCase(VK_ERROR_EXTENSION_NOT_PRESENT							);
		VkStringifyCase(VK_ERROR_FEATURE_NOT_PRESENT							);
		VkStringifyCase(VK_ERROR_INCOMPATIBLE_DRIVER							);
		VkStringifyCase(VK_ERROR_TOO_MANY_OBJECTS								);
		VkStringifyCase(VK_ERROR_FORMAT_NOT_SUPPORTED							);
		VkStringifyCase(VK_ERROR_FRAGMENTED_POOL								);
		VkStringifyCase(VK_ERROR_UNKNOWN										);
		VkStringifyCase(VK_ERROR_OUT_OF_POOL_MEMORY								);
		VkStringifyCase(VK_ERROR_INVALID_EXTERNAL_HANDLE						);
		VkStringifyCase(VK_ERROR_FRAGMENTATION									);
		VkStringifyCase(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS					);
		VkStringifyCase(VK_ERROR_SURFACE_LOST_KHR								);
		VkStringifyCase(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR						);
		VkStringifyCase(VK_SUBOPTIMAL_KHR										);
		VkStringifyCase(VK_ERROR_OUT_OF_DATE_KHR								);
		VkStringifyCase(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR						);
		VkStringifyCase(VK_ERROR_VALIDATION_FAILED_EXT							);
		VkStringifyCase(VK_ERROR_INVALID_SHADER_NV								);
		VkStringifyCase(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT	);
		VkStringifyCase(VK_ERROR_NOT_PERMITTED_EXT								);
		VkStringifyCase(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT			);
		VkStringifyCase(VK_THREAD_IDLE_KHR										);
		VkStringifyCase(VK_THREAD_DONE_KHR										);
		VkStringifyCase(VK_OPERATION_DEFERRED_KHR								);
		VkStringifyCase(VK_OPERATION_NOT_DEFERRED_KHR							);
		VkStringifyCase(VK_ERROR_COMPRESSION_EXHAUSTED_EXT						);
		VkStringifyCase(VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT					);
	}
#undef VkStringifyCase

	return "Unknown Vulkan error";
}


bool IsDepthFormat(Format format)
{
	return GetVkFormatMap(format).aspectFlags & VK_IMAGE_ASPECT_DEPTH_BIT;
}


bool IsStencilFormat(Format format)
{
	return GetVkFormatMap(format).aspectFlags & VK_IMAGE_ASPECT_STENCIL_BIT;
}


bool IsDepthStencilFormat(Format format)
{
	VkImageAspectFlags flags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	return (GetVkFormatMap(format).aspectFlags & flags) == flags;
}


bool IsDepthOrStencilFormat(Format format)
{
	VkImageAspectFlags flags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	return (GetVkFormatMap(format).aspectFlags & flags) != 0;
}


bool QueueSupportsPresent(VkPhysicalDevice physicalDevice, u32 index)
{
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	if (vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, index))
	{
		return true;
	}
#endif

	return false;
}