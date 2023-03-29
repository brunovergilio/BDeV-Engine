#include "BvDebugReportVk.h"
#include "BvLoaderVk.h"


BvDebugReportVk::BvDebugReportVk(const VkInstance instance)
	: m_Instance(instance)
{
	Create();
}


BvDebugReportVk::~BvDebugReportVk()
{
	Destroy();
}


void BvDebugReportVk::Create()
{

	VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
	debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	//debugInfo.pNext = nullptr;
	//debugInfo.flags = 0;
	debugInfo.messageSeverity = VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;;
	debugInfo.messageType = VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
		| VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	debugInfo.pfnUserCallback = DebugUtilsMessengerCallbackEXT;
	//debugInfo.pUserData = nullptr;

	auto result = vkCreateDebugUtilsMessengerEXT(m_Instance, &debugInfo, nullptr, &m_DebugReport);
}


void BvDebugReportVk::Destroy()
{
	if (m_DebugReport)
	{
		vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugReport, nullptr);
		m_DebugReport = VK_NULL_HANDLE;
	}
}


VkBool32 VKAPI_PTR BvDebugReportVk::DebugUtilsMessengerCallbackEXT(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	char prefix[64]{};
	char message[2048]{};

	bool triggerAbort = false;
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
	{
		strcpy_s(prefix, "VERBOSE: ");
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
	{
		strcpy_s(prefix, "INFO: ");
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		triggerAbort = true;
		strcpy_s(prefix, "WARNING: ");
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		triggerAbort = true;
		strcpy_s(prefix, "ERROR: ");
	}

	if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
	{
		strcat_s(prefix, "GENERAL");
	}
	else
	{
		if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
		{
			strcat_s(prefix, "VALIDATION");
		}
		if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
		{
			if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
			{
				strcat_s(prefix, " | ");
			}
			strcat_s(prefix, "PERFORMANCE");
		}
	}
	sprintf_s(message, "%s - Message ID Number %d, Message ID[%s] String :\n%s", prefix, pCallbackData->messageIdNumber,
		pCallbackData->pMessageIdName, pCallbackData->pMessage);
	if (pCallbackData->objectCount > 0)
	{
		char tmp_message[500];
		sprintf_s(tmp_message, "\n Objects - %d\n", pCallbackData->objectCount);
		strcat_s(message, tmp_message);
		for (uint32_t object = 0; object < pCallbackData->objectCount; ++object)
		{
			sprintf_s(tmp_message, " Object[%d] - Type %d, Value %p, Name \"%s\"\n", object,
				pCallbackData->pObjects[object].objectType, (void*)(pCallbackData->pObjects[object].objectHandle),
				pCallbackData->pObjects[object].pObjectName);
			strcat_s(message, tmp_message);
		}
	}
	if (pCallbackData->cmdBufLabelCount > 0)
	{
		char tmp_message[500];
		sprintf_s(tmp_message, "\n Command Buffer Labels - %d\n",	pCallbackData->cmdBufLabelCount);
		strcat_s(message, tmp_message);
		for (uint32_t label = 0; label < pCallbackData->cmdBufLabelCount; ++label)
		{
			sprintf_s(tmp_message, " Label[%d] - %s { %f, %f, %f, %f}\n",	label, pCallbackData->pCmdBufLabels[label].pLabelName,
				pCallbackData->pCmdBufLabels[label].color[0], pCallbackData->pCmdBufLabels[label].color[1],
				pCallbackData->pCmdBufLabels[label].color[2], pCallbackData->pCmdBufLabels[label].color[3]);
			strcat_s(message, tmp_message);
		}
	}

	DPrintF("%s\n", message);
	if (triggerAbort)
	{
		BV_ERROR(message);
	}

	return VK_FALSE;
}
