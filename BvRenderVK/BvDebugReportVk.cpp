#include "BvDebugReportVk.h"
#include "BvLoaderVk.h"


BvDebugReportVk::BvDebugReportVk(const VkInstance instance)
	: m_Instance(instance)
{
}


BvDebugReportVk::~BvDebugReportVk()
{
	Destroy();
}


void BvDebugReportVk::Create()
{
	VkDebugReportFlagsEXT flags = VK_DEBUG_REPORT_WARNING_BIT_EXT
		| VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT
		| VK_DEBUG_REPORT_ERROR_BIT_EXT;
	VkDebugReportCallbackCreateInfoEXT debugInfo{};
	debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debugInfo.pUserData = nullptr;
	debugInfo.pfnCallback = DebugReportCallback;
	debugInfo.flags = flags;

	auto result = VulkanFunctions::vkCreateDebugReportCallbackEXT(m_Instance, &debugInfo, nullptr, &m_DebugReport);
}


void BvDebugReportVk::Destroy()
{
	if (m_DebugReport)
	{
		VulkanFunctions::vkDestroyDebugReportCallbackEXT(m_Instance, m_DebugReport, nullptr);
		m_DebugReport = VK_NULL_HANDLE;
	}
}


VkBool32 VKAPI_PTR BvDebugReportVk::DebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char * pLayerPrefix, const char * pMessage, void * pUserData)
{
	// Error that may result in undefined behaviour
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		DPrintF("Error on [%s] (Code %d):\n%s\n", pLayerPrefix, messageCode, pMessage);
		abort();
	};
	// Warnings may hint at unexpected / non-spec API usage
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		DPrintF("Warning on [%s] (Code %d):\n%s\n", pLayerPrefix, messageCode, pMessage);
		abort();
	};
	// May indicate sub-optimal usage of the API
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
	{
		DPrintF("Performance Warning on [%s] (Code %d):\n%s\n", pLayerPrefix, messageCode, pMessage);
		abort();
	};
	// Informal messages that may become handy during debugging
	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
	{
	}
	// Diagnostic info from the Vulkan loader and layers
	// Usually not helpful in terms of API usage, but may help to debug layer and loader problems 
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
	{
	}

	return VK_FALSE;
}