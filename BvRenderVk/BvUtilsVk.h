#pragma once


#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BvCommonVk.h"
#include "BDeV/Core/RenderAPI/BvRenderCommon.h"


#define BvDebugVkResult(result) BvDebug::PrintF("==== Vulkan Error: %-56s | %s\n", VkResultToString(result), BV_FUNCTION)
#define BvCheckErrorReturnVk(result, ret) if (result != VK_SUCCESS) \
{																	\
	BvDebugVkResult(result);										\
	return ret;														\
}

const char * const VkResultToString(const VkResult result);
bool IsDepthFormat(Format format);
bool IsStencilFormat(Format format);
bool IsDepthStencilFormat(Format format);
bool IsDepthOrStencilFormat(Format format);