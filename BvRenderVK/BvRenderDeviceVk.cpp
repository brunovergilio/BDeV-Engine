#include "BvRenderDeviceVk.h"
#include "BvUtilsVk.h"
#include "BvCommandQueueVk.h"
#include "BvSwapChainVk.h"
#include "BvRenderPassVk.h"
#include "BvCommandPoolVk.h"
#include "BvShaderResourceVk.h"
#include "BvPipelineStateVk.h"
#include "BvSemaphoreVk.h"


BvRenderDeviceVk::BvRenderDeviceVk(VkInstance instance, BvLoaderVk & loader, const BvGPUInfoVk & gpuInfo)
	: m_Instance(instance), m_Loader(loader), m_GPUInfo(gpuInfo)
{
}


BvRenderDeviceVk::~BvRenderDeviceVk()
{
}


bool BvRenderDeviceVk::Create(const DeviceCreateDesc & deviceCreateDesc)
{
	BvAssert(deviceCreateDesc.m_GraphicsQueueCount + deviceCreateDesc.m_ComputeQueueCount + deviceCreateDesc.m_TransferQueueCount > 0);

	// ===========================================================
	// Prepare Device Queues and create the logical device
	BvVector<VkDeviceQueueCreateInfo> queueInfos;
	VkDeviceQueueCreateInfo queueCreateInfo{};

	constexpr u32 kMaxQueueCount = 64;
	BvAssert(deviceCreateDesc.m_GraphicsQueueCount <= kMaxQueueCount);
	BvAssert(deviceCreateDesc.m_ComputeQueueCount <= kMaxQueueCount);
	BvAssert(deviceCreateDesc.m_TransferQueueCount <= kMaxQueueCount);

	constexpr float queuePriorities[kMaxQueueCount] =
	{
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
	};

	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.pQueuePriorities = queuePriorities;
	queueCreateInfo.queueFamilyIndex = m_GPUInfo.m_GraphicsQueueIndex;
	if (deviceCreateDesc.m_GraphicsQueueCount > 0)
	{
		queueCreateInfo.queueCount = deviceCreateDesc.m_GraphicsQueueCount;
		queueInfos.PushBack(queueCreateInfo);
	}
	if (m_GPUInfo.m_GraphicsQueueIndex != m_GPUInfo.m_ComputeQueueIndex
		&& deviceCreateDesc.m_ComputeQueueCount > 0)
	{
		queueCreateInfo.queueFamilyIndex = m_GPUInfo.m_ComputeQueueIndex;
		queueCreateInfo.queueCount = deviceCreateDesc.m_ComputeQueueCount;
		queueInfos.PushBack(queueCreateInfo);
	}
	if (m_GPUInfo.m_GraphicsQueueIndex != m_GPUInfo.m_TransferQueueIndex
		&& m_GPUInfo.m_ComputeQueueIndex != m_GPUInfo.m_TransferQueueIndex
		&& deviceCreateDesc.m_TransferQueueCount > 0)
	{
		queueCreateInfo.queueFamilyIndex = m_GPUInfo.m_TransferQueueIndex;
		queueCreateInfo.queueCount = deviceCreateDesc.m_TransferQueueCount;
		queueInfos.PushBack(queueCreateInfo);
	}


	// Check if queues are the same and use the highest queueCount from the ones set in the DeviceDesc
	// Also check whether a presentation queue has been included
	bool hasPresentationQueue = false;
	for (auto && queueInfo : queueInfos)
	{
		if (queueInfo.queueFamilyIndex == m_GPUInfo.m_GraphicsQueueIndex
			&& deviceCreateDesc.m_GraphicsQueueCount > queueInfo.queueCount)
		{
			queueInfo.queueCount = deviceCreateDesc.m_GraphicsQueueCount;
		}
		if (queueInfo.queueFamilyIndex == m_GPUInfo.m_ComputeQueueIndex
			&& deviceCreateDesc.m_ComputeQueueCount > queueInfo.queueCount)
		{
			queueInfo.queueCount = deviceCreateDesc.m_ComputeQueueCount;
		}
		if (queueInfo.queueFamilyIndex == m_GPUInfo.m_TransferQueueIndex
			&& deviceCreateDesc.m_TransferQueueCount > queueInfo.queueCount)
		{
			queueInfo.queueCount = deviceCreateDesc.m_TransferQueueCount;
		}
		if (queueInfo.queueFamilyIndex == m_GPUInfo.m_PresentationQueueIndex)
		{
			hasPresentationQueue = true;
		}
	}

	BvVector<const char *> enabledExtensions;
	if (deviceCreateDesc.m_GraphicsQueueCount > 0)
	{
		enabledExtensions.PushBack(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		// If we have at least one graphics queue, and we still don't have a presentation queue, add one
		if (!hasPresentationQueue)
		{
			queueCreateInfo.queueFamilyIndex = m_GPUInfo.m_PresentationQueueIndex;
			queueCreateInfo.queueCount = 1;
			queueInfos.PushBack(queueCreateInfo);
		}
	}

	if (std::find(m_GPUInfo.m_SupportedExtensions.cbegin(), m_GPUInfo.m_SupportedExtensions.cend(), VK_EXT_DEBUG_MARKER_EXTENSION_NAME)
		!= m_GPUInfo.m_SupportedExtensions.cend())
	{
		enabledExtensions.PushBack(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
	}

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = queueInfos.Data();
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.Size());
	deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.Data();
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.Size());
	//deviceCreateInfo.pEnabledFeatures = &m_GPUInfo.m_DeviceFeatures;
	deviceCreateInfo.pNext = &m_GPUInfo.m_DeviceFeatures;

	VkResult result = VulkanFunctions::vkCreateDevice(m_GPUInfo.m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device);
	BvAssertMsg(result == VK_SUCCESS, "Couldn't create a logical device");
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		return false;
	}

	m_Loader.LoadDeviceFunctions(m_Device, m_Functions);

	m_GraphicsQueues.Resize(deviceCreateDesc.m_GraphicsQueueCount);
	m_ComputeQueues.Resize(deviceCreateDesc.m_ComputeQueueCount);
	m_TransferQueues.Resize(deviceCreateDesc.m_TransferQueueCount);
	for (auto i = 0; i < deviceCreateDesc.m_GraphicsQueueCount; i++)
	{
		m_GraphicsQueues[i] = new BvCommandQueueVk(*this, QueueFamilyType::kGraphics, i);
	}
	for (auto i = 0; i < deviceCreateDesc.m_ComputeQueueCount; i++)
	{
		m_ComputeQueues[i] = new BvCommandQueueVk(*this, QueueFamilyType::kCompute, i);
	}
	for (auto i = 0; i < deviceCreateDesc.m_TransferQueueCount; i++)
	{
		m_TransferQueues[i] = new BvCommandQueueVk(*this, QueueFamilyType::kTransfer, i);
	}

	return true;
}


void BvRenderDeviceVk::Destroy()
{
	for (auto && pQueue : m_GraphicsQueues)
	{
		auto pQueueVk = static_cast<BvCommandQueueVk *>(pQueue);
		BvDelete(pQueueVk);
		pQueue = nullptr;
	}
	for (auto && pQueue : m_ComputeQueues)
	{
		auto pQueueVk = static_cast<BvCommandQueueVk *>(pQueue);
		BvDelete(pQueueVk);
		pQueue = nullptr;
	}
	for (auto && pQueue : m_TransferQueues)
	{
		auto pQueueVk = static_cast<BvCommandQueueVk *>(pQueue);
		BvDelete(pQueueVk);
		pQueue = nullptr;
	}

	if (m_Device)
	{
		m_Functions.vkDestroyDevice(m_Device, nullptr);
		m_Device = VK_NULL_HANDLE;
	}
}


BvSwapChain * BvRenderDeviceVk::CreateSwapChain(BvNativeWindow & window, const SwapChainDesc & swapChainDesc, BvCommandQueue & commandQueue)
{
	auto pQueue = static_cast<BvCommandQueueVk *>(&commandQueue);

	BvSwapChainVk * pSwapChain = nullptr;
	pSwapChain = new BvSwapChainVk(*this, *pQueue, window, swapChainDesc);
	pSwapChain->Create();

	return pSwapChain;
}

BvSemaphore * BvRenderDeviceVk::CreateSemaphore(const u64 initialValue)
{
	auto pSemaphore = new BvSemaphoreVk(*this);
	pSemaphore->Create(true, initialValue);

	return pSemaphore;
}

BvRenderPass * BvRenderDeviceVk::CreateRenderPass(const RenderPassDesc & renderPassDesc)
{
	auto pRenderPass = new BvRenderPassVk(*this, renderPassDesc);
	pRenderPass->Create();
	return pRenderPass;
}


BvCommandPool * BvRenderDeviceVk::CreateCommandPool(const CommandPoolDesc & commandPoolDesc)
{
	auto pCommandPool = new BvCommandPoolVk(*this, commandPoolDesc);
	pCommandPool->Create();
	return pCommandPool;
}


BvShaderResourceLayout * BvRenderDeviceVk::CreateShaderResourceLayout(const ShaderResourceLayoutDesc & shaderResourceLayoutDesc)
{
	auto pShaderResourceLayout = new BvShaderResourceLayoutVk(*this, shaderResourceLayoutDesc);
	pShaderResourceLayout->Create();
	return pShaderResourceLayout;
}


BvGraphicsPipelineState * BvRenderDeviceVk::CreateGraphicsPipeline(const GraphicsPipelineStateDesc & graphicsPipelineStateDesc)
{
	auto pGraphicsPSO = new BvGraphicsPipelineStateVk(*this, graphicsPipelineStateDesc, VK_NULL_HANDLE);
	pGraphicsPSO->Create();
	return pGraphicsPSO;
}


void BvRenderDeviceVk::WaitIdle() const
{
	auto result = m_Functions.vkDeviceWaitIdle(m_Device);
}


const uint32_t BvRenderDeviceVk::GetMemoryTypeIndex(const uint32_t memoryTypeBits, const VkMemoryPropertyFlags properties) const
{
	uint32_t index = UINT32_MAX;
	uint32_t typeBits = memoryTypeBits;

	auto & memoryProperties = m_GPUInfo.m_DeviceMemoryProperties;
	// Iterate over all memory types available for the device used in this example
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				index = i;
				break;
			}
		}
		typeBits >>= 1;
	}

	return index;
}


const VkFormat BvRenderDeviceVk::GetBestDepthFormat(const VkFormat format /*= VK_FORMAT_UNDEFINED*/) const
{
	VkFormat chosenFormat = VK_FORMAT_UNDEFINED;

	// Since all depth formats may be optional, we need to find a suitable depth format to use
	// Start with the highest precision packed format
	VkFormat depthFormats[] =
	{
		VK_FORMAT_D24_UNORM_S8_UINT, // 24-bit depths are better for optimal performance
		VK_FORMAT_D32_SFLOAT_S8_UINT, // 32-bit depth is costlier, use if you need more precision
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

	VkFormatProperties formatProps;
	for (auto & depthFormat : depthFormats)
	{
		VulkanFunctions::vkGetPhysicalDeviceFormatProperties(m_GPUInfo.m_PhysicalDevice, depthFormat, &formatProps);
		// Format must support depth stencil attachment for optimal tiling
		if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			chosenFormat = depthFormat;
			if (format == VK_FORMAT_UNDEFINED || format == chosenFormat)
			{
				break;
			}
		}
	}

	return chosenFormat;
}


bool BvRenderDeviceVk::QueueFamilySupportsPresent(const QueueFamilyType queueFamilyType) const
{
	u32 queueFamilyIndex = UINT32_MAX;

	switch (queueFamilyType)
	{
	case QueueFamilyType::kGraphics:	queueFamilyIndex = m_GPUInfo.m_GraphicsQueueIndex;
	case QueueFamilyType::kCompute:		queueFamilyIndex = m_GPUInfo.m_ComputeQueueIndex;
	case QueueFamilyType::kTransfer:	queueFamilyIndex = m_GPUInfo.m_TransferQueueIndex;
	}

#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	return VulkanFunctions::vkGetPhysicalDeviceWin32PresentationSupportKHR(m_GPUInfo.m_PhysicalDevice, queueFamilyIndex) == VK_TRUE;
#else
#endif
}