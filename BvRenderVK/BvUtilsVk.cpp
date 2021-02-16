#include "BvRenderVK/BvUtilsVk.h"
#include "BvCore/Container/BvRobinMap.h"
#include "BvCore/Container/BvString.h"


const char * const VkResultToString(const VkResult result)
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
	}
#undef VkStringifyCase

	return "VK_RESULT_MAX_ENUM";
}


static const BvRobinMap<VkFormat, std::pair<bool, bool>> s_DepthStencilFormats =
{
	{ VK_FORMAT_D16_UNORM, std::make_pair(true, false) },
	{ VK_FORMAT_X8_D24_UNORM_PACK32, std::make_pair(true, false) },
	{ VK_FORMAT_D32_SFLOAT, std::make_pair(true, false) },
	{ VK_FORMAT_D16_UNORM_S8_UINT, std::make_pair(true, true) },
	{ VK_FORMAT_D24_UNORM_S8_UINT, std::make_pair(true, true) },
	{ VK_FORMAT_D32_SFLOAT_S8_UINT, std::make_pair(true, true) },
	{ VK_FORMAT_S8_UINT, std::make_pair(false, true) },
};


bool IsDepthFormat(const VkFormat format)
{
	auto it = s_DepthStencilFormats.FindKey(format);
	if (it != s_DepthStencilFormats.cend())
	{
		return it->second.first;
	}

	return false;
}


bool IsStencilFormat(const VkFormat format)
{
	auto it = s_DepthStencilFormats.FindKey(format);
	if (it != s_DepthStencilFormats.cend())
	{
		return it->second.second;
	}

	return false;
}


bool IsDepthStencilFormat(const VkFormat format)
{
	auto it = s_DepthStencilFormats.FindKey(format);
	if (it != s_DepthStencilFormats.cend())
	{
		return true;
	}

	return false;
}


VkImageAspectFlags GetVkAspectMaskFlags(const VkFormat format)
{
	VkImageAspectFlags aspectMask = 0;
	if (IsDepthFormat(format))
	{
		aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	if (IsStencilFormat(format))
	{
		aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	return aspectMask == 0 ? VK_IMAGE_ASPECT_COLOR_BIT : aspectMask;
}