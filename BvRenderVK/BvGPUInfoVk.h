#pragma once


#include "BvRenderVK/BvCommonVk.h"
#include "BvCore/Container/BvVector.h"
#include "BvCore/Container/BvString.h"


struct BvGPUInfoVk
{
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties2 m_DeviceProperties{};
	VkPhysicalDeviceVulkan11Properties m_DeviceProperties11{};
	VkPhysicalDeviceVulkan12Properties m_DeviceProperties12{};
	VkPhysicalDeviceFeatures2 m_DeviceFeatures{};
	VkPhysicalDeviceVulkan11Features m_DeviceFeatures11{};
	VkPhysicalDeviceVulkan12Features m_DeviceFeatures12{};
	VkPhysicalDeviceMemoryProperties m_DeviceMemoryProperties{};
	struct
	{
		VkPhysicalDeviceConservativeRasterizationPropertiesEXT convervativeRasterizationProps{};
	} m_Extensions;
	BvVector<VkQueueFamilyProperties> m_QueueFamilyProperties;
	BvVector<BvStringT> m_SupportedExtensions;
	u32 m_GraphicsQueueIndex = UINT32_MAX;
	u32 m_ComputeQueueIndex = UINT32_MAX;
	u32 m_TransferQueueIndex = UINT32_MAX;
	u32 m_PresentationQueueIndex = UINT32_MAX;
};