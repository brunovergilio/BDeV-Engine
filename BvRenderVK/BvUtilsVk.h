#pragma once


#include "BDeV/System/Debug/BvDebug.h"
#include "BvRenderVK/BvCommonVk.h"
#include "BDeV/RenderAPI/BvCommon.h"


#define BvDebugVkResult(result) DPrintF("==== Vulkan Error: %-56s | %s\n", VkResultToString(result), BV_FUNCTION)
#define BvCheckErrorReturnVk(result, ret) if (result != VK_SUCCESS) \
{																	\
	BvDebugVkResult(result);										\
	BvDebugBreak();													\
	return ret;														\
}

const char * const VkResultToString(const VkResult result);
bool IsDepthFormat(Format format);
bool IsStencilFormat(Format format);
bool IsDepthStencilFormat(Format format);
