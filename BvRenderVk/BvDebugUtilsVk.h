#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/Utils/BvUtils.h"


class BvDebugUtilsVk
{
	BV_NOCOPYMOVE(BvDebugUtilsVk);

public:
	BvDebugUtilsVk(VkInstance instance);
	~BvDebugUtilsVk();

private:
	void Create();
	void Destroy();

	static VkBool32 VKAPI_PTR DebugUtilsMessengerCallbackEXT(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,	void* pUserData);

private:
	VkInstance m_Instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT m_DebugUtils = VK_NULL_HANDLE;
};