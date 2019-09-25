#pragma once


#include "BvRender/BvRenderDefines.h"
#include "BvCore/Container/BvVector.h"
#include "BvCore/Container/BvString.h"


class BvVulkanDevice
{
public:
	struct VulkanGPUInfo
	{
		VkPhysicalDeviceProperties m_DeviceProperties{};
		VkPhysicalDeviceFeatures m_DeviceFeatures{};
		VkPhysicalDeviceMemoryProperties m_DeviceMemoryProperties{};
		BvVector<VkQueueFamilyProperties> m_QueueFamilyProperties;
		BvVector<BvString> m_SupportedExtensions;
	};

	BvVulkanDevice();
	~BvVulkanDevice();

	void Create();
	void Destroy();

	void WaitIdle();

	const uint32_t GetQueueFamilyIndex(const VkQueueFlags queueFlags) const;
	const uint32_t GetMemoryTypeIndex(const uint32_t memoryTypeBits, const VkMemoryPropertyFlags properties) const;
	const VkFormat GetBestDepthFormat(const VkFormat format = VK_FORMAT_UNDEFINED) const;

	BV_INLINE const VulkanGPUInfo & GetGPUInfo() const { return m_GPUInfo; }
	
	BV_INLINE const VkQueue GetGraphicsQueue() const { return m_Queues[m_GraphicsQueueIndex]; }
	BV_INLINE const VkQueue GetComputeQueue() const { return m_Queues[m_ComputeQueueIndex]; }
	BV_INLINE const uint32_t GetGraphicsQueueIndex() const { return m_GraphicsQueueIndex; }
	BV_INLINE const uint32_t GetComputeQueueIndex() const { return m_ComputeQueueIndex; }

	BV_INLINE const VkInstance GetInstance() const { return m_Instance; }
	BV_INLINE const VkDevice GetLogical() const { return m_Device; }
	BV_INLINE const VkPhysicalDevice GetPhysical() const { return m_PhysicalDevice; }

private:
	void CreateInstance();
	void SelectGPU();
	void CreateDevice();

private:
	VulkanGPUInfo m_GPUInfo;

	VkInstance m_Instance = VK_NULL_HANDLE;
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	VkDevice m_Device = VK_NULL_HANDLE;

	// Device queues
	BvVector<VkQueue> m_Queues;
	uint32_t m_GraphicsQueueIndex = UINT32_MAX;
	uint32_t m_ComputeQueueIndex = UINT32_MAX;
};