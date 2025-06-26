#pragma once


#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BvCommonVk.h"
#include "BDeV/Core/RenderAPI/BvRenderCommon.h"


const char * const VkResultToString(const VkResult result);
bool IsDepthFormat(Format format);
bool IsStencilFormat(Format format);
bool IsDepthStencilFormat(Format format);
bool IsDepthOrStencilFormat(Format format);
bool QueueSupportsPresent(VkPhysicalDevice physicalDevice, u32 index);