#pragma once


#include "BDeV/System/Debug/BvDebug.h"
#include "BvRenderVK/BvCommonVk.h"
#include "BDeV/Render/BvCommon.h"


#define BvDebugVkResult(result) DPrintF("==== Vulkan Error: %-56s | %s\n", VkResultToString(result), BV_FUNCTION)
#define BvCheckErrorReturnVk(result, ret) if (result != VK_SUCCESS) \
{																	\
	BvDebugVkResult(result);										\
	BvDebugBreak();													\
	return ret;														\
}

const char * const VkResultToString(const VkResult result);
bool IsDepthFormat(const VkFormat format);
bool IsStencilFormat(const VkFormat format);
bool IsDepthStencilFormat(const VkFormat format);
VkImageAspectFlags GetVkAspectMaskFlags(const VkFormat format);