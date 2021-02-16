#pragma once


#include "BvCommonVk.h"
#include "BvCore/Utils/BvUtils.h"


class BvDebugReportVk
{
	BV_NOCOPYMOVE(BvDebugReportVk);

public:
	BvDebugReportVk(const VkInstance instance);
	~BvDebugReportVk();

	void Create();
	void Destroy();

private:
	static VkBool32 VKAPI_PTR DebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
		uint64_t object, size_t location,
		int32_t messageCode, const char * pLayerPrefix,
		const char * pMessage, void * pUserData);

private:
	VkInstance m_Instance = VK_NULL_HANDLE;
	VkDebugReportCallbackEXT m_DebugReport = VK_NULL_HANDLE;
};