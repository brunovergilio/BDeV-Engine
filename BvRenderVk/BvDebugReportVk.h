#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/Utils/BvUtils.h"


class BvDebugReportVk
{
	BV_NOCOPYMOVE(BvDebugReportVk);

public:
	BvDebugReportVk(const VkInstance instance);
	~BvDebugReportVk();

private:
	void Create();
	void Destroy();

	static VkBool32 VKAPI_PTR DebugUtilsMessengerCallbackEXT(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,	void* pUserData);

private:
	VkInstance m_Instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT m_DebugReport = VK_NULL_HANDLE;
};