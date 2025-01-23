#include "BvRenderDeviceVk.h"
#include "BvUtilsVk.h"
#include "BvCommandQueueVk.h"
#include "BvSwapChainVk.h"
#include "BvRenderPassVk.h"
#include "BvCommandPoolVk.h"
#include "BvShaderResourceVk.h"
#include "BvShaderVk.h"
#include "BvPipelineStateVk.h"
#include "BvSemaphoreVk.h"
#include "BvBufferVk.h"
#include "BvBufferViewVk.h"
#include "BvTextureVk.h"
#include "BvTextureViewVk.h"
#include "BvSamplerVk.h"
#include "BvDescriptorSetVk.h"
#include "BvQueryVk.h"
#include "BvFramebufferVk.h"
#include "BvTypeConversionsVk.h"
#include "BvCommandContextVk.h"
#include "BvQueryVk.h"
#include "BDeV/Core/RenderAPI/BvRenderAPIUtils.h"


bool IsPhysicalDeviceExtensionSupported(const BvVector<VkExtensionProperties>& supportedExtensions, const char* pExtensionName);
RenderDeviceCapabilities SetupDeviceInfo(VkPhysicalDevice physicalDevice, BvDeviceInfoVk& gpu);
u32 GetQueueFamilyIndex(const BvVector<VkQueueFamilyProperties2>& queueFamilyProperties, VkQueueFlags queueFlags, bool& isAsync);
bool QueueSupportsPresent(VkPhysicalDevice physicalDevice, u32 index);


BvRenderDeviceVk::BvRenderDeviceVk(BvRenderEngineVk* pEngine, VkPhysicalDevice physicalDevice, const BvRenderDeviceCreateDescVk& deviceDesc)
	: m_pEngine(pEngine), m_PhysicalDevice(physicalDevice), m_pDeviceInfo(BV_NEW(BvDeviceInfoVk)())
{
	Create(deviceDesc);
}


BvRenderDeviceVk::~BvRenderDeviceVk()
{
	Destroy();
}


BvSwapChain* BvRenderDeviceVk::CreateSwapChain(BvWindow* pWindow, const SwapChainDesc& swapChainDesc, BvCommandContext* pContext)
{
	return CreateSwapChainVk(pWindow, swapChainDesc, pContext);
}


BvBuffer* BvRenderDeviceVk::CreateBuffer(const BufferDesc& desc, const BufferInitData* pInitData)
{
	return CreateBufferVk(desc, pInitData);
}


BvBufferView* BvRenderDeviceVk::CreateBufferView(const BufferViewDesc& desc)
{
	return CreateBufferViewVk(desc);
}


BvTexture* BvRenderDeviceVk::CreateTexture(const TextureDesc& desc, const TextureInitData* pInitData)
{
	return CreateTextureVk(desc, pInitData);
}


BvTextureView* BvRenderDeviceVk::CreateTextureView(const TextureViewDesc& desc)
{
	return CreateTextureViewVk(desc);
}


BvSampler* BvRenderDeviceVk::CreateSampler(const SamplerDesc& desc)
{
	return CreateSamplerVk(desc);
}


BvRenderPass* BvRenderDeviceVk::CreateRenderPass(const RenderPassDesc& renderPassDesc)
{
	return CreateRenderPassVk(renderPassDesc);
}


BvShaderResourceLayout* BvRenderDeviceVk::CreateShaderResourceLayout(u32 shaderResourceCount,
	const ShaderResourceDesc* pShaderResourceDescs, const ShaderResourceConstantDesc& shaderResourceConstantDesc)
{
	return CreateShaderResourceLayoutVk(shaderResourceCount, pShaderResourceDescs, shaderResourceConstantDesc);
}


BvShader* BvRenderDeviceVk::CreateShader(const ShaderCreateDesc& shaderDesc)
{
	return CreateShaderVk(shaderDesc);
}


BvGraphicsPipelineState* BvRenderDeviceVk::CreateGraphicsPipeline(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc)
{
	return CreateGraphicsPipelineVk(graphicsPipelineStateDesc);
}


BvComputePipelineState* BvRenderDeviceVk::CreateComputePipeline(const ComputePipelineStateDesc& computePipelineStateDesc)
{
	return CreateComputePipelineVk(computePipelineStateDesc);
}


BvQuery* BvRenderDeviceVk::CreateQuery(QueryType queryType)
{
	return CreateQueryVk(queryType);
}


BvSwapChainVk* BvRenderDeviceVk::CreateSwapChainVk(BvWindow* pWindow, const SwapChainDesc& swapChainDesc, BvCommandContext* pContext)
{
	auto pObj = BV_NEW(BvSwapChainVk)(this, pWindow, swapChainDesc, pContext);
	m_DeviceObjects.PushBack(pObj);
	return pObj;
}


BvBufferVk* BvRenderDeviceVk::CreateBufferVk(const BufferDesc& desc, const BufferInitData* pInitData /*= nullptr*/)
{
	auto pObj = BV_NEW(BvBufferVk)(this, desc, pInitData);
	m_DeviceObjects.PushBack(pObj);
	return pObj;
}


BvBufferViewVk* BvRenderDeviceVk::CreateBufferViewVk(const BufferViewDesc& desc)
{
	auto pObj = BV_NEW(BvBufferViewVk)(this, desc);
	m_DeviceObjects.PushBack(pObj);
	return pObj;
}


BvTextureVk* BvRenderDeviceVk::CreateTextureVk(const TextureDesc& desc, const TextureInitData* pInitData /*= nullptr*/)
{
	auto pObj = BV_NEW(BvTextureVk)(this, desc, pInitData);
	m_DeviceObjects.PushBack(pObj);
	return pObj;
}


BvTextureViewVk* BvRenderDeviceVk::CreateTextureViewVk(const TextureViewDesc& desc)
{
	auto pObj = BV_NEW(BvTextureViewVk)(this, desc);
	m_DeviceObjects.PushBack(pObj);
	return pObj;
}


BvSamplerVk* BvRenderDeviceVk::CreateSamplerVk(const SamplerDesc& desc)
{
	auto pObj = BV_NEW(BvSamplerVk)(this, desc);
	m_DeviceObjects.PushBack(pObj);
	return pObj;
}


BvRenderPassVk* BvRenderDeviceVk::CreateRenderPassVk(const RenderPassDesc& renderPassDesc)
{
	auto pObj = BV_NEW(BvRenderPassVk)(this, renderPassDesc);
	m_DeviceObjects.PushBack(pObj);
	return pObj;
}


BvShaderResourceLayoutVk* BvRenderDeviceVk::CreateShaderResourceLayoutVk(u32 shaderResourceCount, const ShaderResourceDesc* pShaderResourceDescs, const ShaderResourceConstantDesc& shaderResourceConstantDesc)
{
	auto pObj = BV_NEW(BvShaderResourceLayoutVk)(this, shaderResourceCount, pShaderResourceDescs, shaderResourceConstantDesc);
	m_DeviceObjects.PushBack(pObj);
	return pObj;
}


BvShaderVk* BvRenderDeviceVk::CreateShaderVk(const ShaderCreateDesc& shaderDesc)
{
	auto pObj = BV_NEW(BvShaderVk)(this, shaderDesc);
	m_DeviceObjects.PushBack(pObj);
	return pObj;
}


BvGraphicsPipelineStateVk* BvRenderDeviceVk::CreateGraphicsPipelineVk(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc)
{
	auto pObj = BV_NEW(BvGraphicsPipelineStateVk)(this, graphicsPipelineStateDesc, VK_NULL_HANDLE);
	m_DeviceObjects.PushBack(pObj);
	return pObj;
}


BvComputePipelineStateVk* BvRenderDeviceVk::CreateComputePipelineVk(const ComputePipelineStateDesc& computePipelineStateDesc)
{
	auto pObj = BV_NEW(BvComputePipelineStateVk)(this, computePipelineStateDesc, VK_NULL_HANDLE);
	m_DeviceObjects.PushBack(pObj);
	return pObj;
}


BvQueryVk* BvRenderDeviceVk::CreateQueryVk(QueryType queryType)
{
	auto pObj = BV_NEW(BvQueryVk)(this, queryType, 3);
	m_DeviceObjects.PushBack(pObj);
	return pObj;
}


void BvRenderDeviceVk::Release(IBvRenderDeviceChild* pDeviceObj)
{
	auto index = m_DeviceObjects.Find(pDeviceObj);
	if (index != kU64Max)
	{
		BV_DELETE(pDeviceObj);
		m_DeviceObjects.Erase(index);
	}
}


void BvRenderDeviceVk::WaitIdle() const
{
	auto result = vkDeviceWaitIdle(m_Device);
}


const u32 BvRenderDeviceVk::GetMemoryTypeIndex(const u32 memoryTypeBits, const VkMemoryPropertyFlags properties) const
{
	u32 index = UINT32_MAX;
	u32 typeBits = memoryTypeBits;

	auto& memoryProperties = m_pDeviceInfo->m_DeviceMemoryProperties;
	for (u32 i = 0; i < memoryProperties.memoryProperties.memoryTypeCount; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((memoryProperties.memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
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
		vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, depthFormat, &formatProps);
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


bool BvRenderDeviceVk::HasFormatSupport(Format format)
{
	const auto& vkFormatMap = GetVkFormatMap(format);

	VkFormatProperties formatProperties{};
	vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, GetVkFormat(format), &formatProperties);

	VkFormatFeatureFlags featureFlags = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
	if (vkFormatMap.aspectFlags & VK_IMAGE_ASPECT_COLOR_BIT)
	{
		featureFlags |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
	}
	else if ((vkFormatMap.aspectFlags & VK_IMAGE_ASPECT_DEPTH_BIT) || (vkFormatMap.aspectFlags & VK_IMAGE_ASPECT_STENCIL_BIT))
	{
		featureFlags |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}

	return (formatProperties.optimalTilingFeatures & featureFlags) == featureFlags
		|| (formatProperties.linearTilingFeatures & featureFlags) == featureFlags;
}


BvCommandContext* BvRenderDeviceVk::GetGraphicsContext(u32 index) const
{
	return index < m_GraphicsContexts.Size() ? m_GraphicsContexts[index] : nullptr;
}


BvCommandContext* BvRenderDeviceVk::GetComputeContext(u32 index) const
{
	return index < m_ComputeContexts.Size() ? m_ComputeContexts[index] : nullptr;
}


BvCommandContext* BvRenderDeviceVk::GetTransferContext(u32 index) const
{
	return index < m_TransferContexts.Size() ? m_TransferContexts[index] : nullptr;
}


void BvRenderDeviceVk::GetCopyableFootprints(const TextureDesc& textureDesc, u32 subresourceCount, SubresourceFootprint* pSubresources, u64* pTotalSize) const
{
	u64 totalSize = 0;
	u64 offset = 0;
	u32 subresourceIndex = 0;
	for (auto layer = 0u; layer < textureDesc.m_LayerCount; ++layer)
	{
		for (auto mip = 0u; mip < textureDesc.m_MipLevels; ++mip)
		{
			offset = totalSize;
			
			TextureSubresource subresource;
			GetTextureSubresourceData(textureDesc, mip, subresource);
			if (pSubresources)
			{
				pSubresources[subresourceIndex].m_Subresource = subresource;
				pSubresources[subresourceIndex].m_Offset = offset;
			}
			totalSize += RoundToNearestPowerOf2(subresource.m_MipSize, 4);

			++subresourceIndex;
		}
	}

	if (pTotalSize)
	{
		*pTotalSize = totalSize;
	}
}


bool BvRenderDeviceVk::SupportsQueryType(QueryType queryType, CommandType commandType) const
{
	if (queryType == QueryType::kTimestamp)
	{
		if (m_pDeviceInfo->m_DeviceProperties.properties.limits.timestampPeriod == 0)
		{
			return false;
		}

		if (m_pDeviceInfo->m_DeviceProperties.properties.limits.timestampComputeAndGraphics
			&& (commandType == CommandType::kGraphics || commandType == CommandType::kCompute))
		{
			return true;
		}
		else
		{
			const VkQueueFamilyProperties* pProperties = nullptr;
			switch (commandType)
			{
			case CommandType::kGraphics:
				pProperties = &m_pDeviceInfo->m_QueueFamilyProperties[m_pDeviceInfo->m_GraphicsQueueInfo.m_QueueFamilyIndex].queueFamilyProperties;
				break;
			case CommandType::kCompute:
				pProperties = &m_pDeviceInfo->m_QueueFamilyProperties[m_pDeviceInfo->m_ComputeQueueInfo.m_QueueFamilyIndex].queueFamilyProperties;
				break;
			case CommandType::kTransfer:
				pProperties = &m_pDeviceInfo->m_QueueFamilyProperties[m_pDeviceInfo->m_TransferQueueInfo.m_QueueFamilyIndex].queueFamilyProperties;
				break;
			}

			return pProperties->timestampValidBits != 0;
		}
	}

	return commandType == CommandType::kGraphics;
}


bool BvRenderDeviceVk::IsFormatSupported(Format format) const
{
	return GetVkFormat(format) != VK_FORMAT_UNDEFINED;
}


void BvRenderDeviceVk::Create(const BvRenderDeviceCreateDescVk& deviceCreateDesc)
{
	BV_ASSERT(deviceCreateDesc.m_GraphicsQueueCount + deviceCreateDesc.m_ComputeQueueCount + deviceCreateDesc.m_TransferQueueCount > 0, "No device queues");

	constexpr u32 kMaxQueueCount = 16;
	BV_ASSERT(deviceCreateDesc.m_GraphicsQueueCount <= kMaxQueueCount, "Graphics queue count greater than limit");
	BV_ASSERT(deviceCreateDesc.m_ComputeQueueCount <= kMaxQueueCount, "Compute queue count greater than limit");
	BV_ASSERT(deviceCreateDesc.m_TransferQueueCount <= kMaxQueueCount, "Transfer queue count greater than limit");

	constexpr float queuePriorities[kMaxQueueCount] =
	{
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		//1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		//1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		//1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		//1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		//1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		//1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
	};

	auto deviceCaps = SetupDeviceInfo(m_PhysicalDevice, *m_pDeviceInfo);

	// ===========================================================
	// Prepare Device Queues and create the logical device
	constexpr auto kMaxQueueTypes = 3;
	BvFixedVector<VkDeviceQueueCreateInfo, kMaxQueueTypes> queueInfos;

	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.pQueuePriorities = queuePriorities;
	if (deviceCreateDesc.m_GraphicsQueueCount > 0 && m_pDeviceInfo->m_GraphicsQueueInfo.m_QueueCount > 0)
	{
		queueCreateInfo.queueFamilyIndex = m_pDeviceInfo->m_GraphicsQueueInfo.m_QueueFamilyIndex;
		queueCreateInfo.queueCount = std::min(deviceCreateDesc.m_GraphicsQueueCount, m_pDeviceInfo->m_GraphicsQueueInfo.m_QueueCount);
		queueInfos.PushBack(queueCreateInfo);

		m_GraphicsContexts.Resize(deviceCreateDesc.m_GraphicsQueueCount);
	}
	if (deviceCreateDesc.m_ComputeQueueCount > 0 && m_pDeviceInfo->m_ComputeQueueInfo.m_QueueCount > 0)
	{
		queueCreateInfo.queueFamilyIndex = m_pDeviceInfo->m_ComputeQueueInfo.m_QueueFamilyIndex;
		queueCreateInfo.queueCount = std::min(deviceCreateDesc.m_ComputeQueueCount, m_pDeviceInfo->m_ComputeQueueInfo.m_QueueCount);
		queueInfos.PushBack(queueCreateInfo);

		m_ComputeContexts.Resize(deviceCreateDesc.m_ComputeQueueCount);
	}
	if (deviceCreateDesc.m_TransferQueueCount > 0 && m_pDeviceInfo->m_TransferQueueInfo.m_QueueCount > 0)
	{
		queueCreateInfo.queueFamilyIndex = m_pDeviceInfo->m_TransferQueueInfo.m_QueueFamilyIndex;
		queueCreateInfo.queueCount = std::min(deviceCreateDesc.m_TransferQueueCount, m_pDeviceInfo->m_TransferQueueInfo.m_QueueCount);
		queueInfos.PushBack(queueCreateInfo);

		m_TransferContexts.Resize(deviceCreateDesc.m_TransferQueueCount);
	}

	VkDeviceCreateInfo deviceCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	//deviceCreateInfo.pEnabledFeatures = nullptr;
	deviceCreateInfo.pNext = &m_pDeviceInfo->m_DeviceFeatures;
	deviceCreateInfo.pQueueCreateInfos = queueInfos.Data();
	deviceCreateInfo.queueCreateInfoCount = static_cast<u32>(queueInfos.Size());
	deviceCreateInfo.ppEnabledExtensionNames = m_pDeviceInfo->m_EnabledExtensions.Data();
	deviceCreateInfo.enabledExtensionCount = static_cast<u32>(m_pDeviceInfo->m_EnabledExtensions.Size());

	VkResult result = vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device);
	BV_ASSERT(result == VK_SUCCESS, "Couldn't create a logical device");
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		return;
	}

	volkLoadDevice(m_Device);

	for (auto i = 0; i < m_GraphicsContexts.Size(); i++)
	{
		m_GraphicsContexts[i] = new BvCommandContextVk(this, 3, CommandType::kGraphics, m_pDeviceInfo->m_GraphicsQueueInfo.m_QueueFamilyIndex, i);
	}
	for (auto i = 0; i < m_ComputeContexts.Size(); i++)
	{
		m_ComputeContexts[i] = new BvCommandContextVk(this, 3, CommandType::kCompute, m_pDeviceInfo->m_ComputeQueueInfo.m_QueueFamilyIndex, i);
	}
	for (auto i = 0; i < m_TransferContexts.Size(); i++)
	{
		m_TransferContexts[i] = new BvCommandContextVk(this, 3, CommandType::kTransfer, m_pDeviceInfo->m_TransferQueueInfo.m_QueueFamilyIndex, i);
	}

	CreateVMA();

	m_pFramebufferManager = new BvFramebufferManagerVk();

	u32 querySizes[kQueryTypeCount]{ 16, 16, 16 };
	m_pQueryHeapManager = new BvQueryHeapManagerVk(this, querySizes, 3);

	m_DeviceCaps = deviceCaps;
}


void BvRenderDeviceVk::Destroy()
{
	vkDeviceWaitIdle(m_Device);

	for (i32 i = i32(m_DeviceObjects.Size()) - 1; i >= 0; --i)
	{
		BV_DELETE(m_DeviceObjects[i]);
	}

	delete m_pFramebufferManager;

	DestroyVMA();

	if (m_Device)
	{
		vkDestroyDevice(m_Device, nullptr);
		m_Device = VK_NULL_HANDLE;
	}

	BV_DELETE(m_pDeviceInfo);
}


void BvRenderDeviceVk::CreateVMA()
{
	VmaAllocatorCreateInfo vmaACI{};
	vmaACI.instance = m_pEngine->GetHandle();
	vmaACI.device = m_Device;
	vmaACI.physicalDevice = m_PhysicalDevice;
	vmaACI.vulkanApiVersion = m_pDeviceInfo->m_DeviceProperties.properties.apiVersion;

	if (m_pDeviceInfo->m_DeviceFeatures1_2.bufferDeviceAddress)
	{
		vmaACI.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	}
	if (IsPhysicalDeviceExtensionSupported(m_pDeviceInfo->m_SupportedExtensions, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME))
	{
		vmaACI.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
	}

	VmaVulkanFunctions functions{};
	functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
	functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
	functions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
	functions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
	functions.vkAllocateMemory = vkAllocateMemory;
	functions.vkFreeMemory = vkFreeMemory;
	functions.vkMapMemory = vkMapMemory;
	functions.vkUnmapMemory = vkUnmapMemory;
	functions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
	functions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
	functions.vkBindBufferMemory = vkBindBufferMemory;
	functions.vkBindImageMemory = vkBindImageMemory;
	functions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
	functions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
	functions.vkCreateBuffer = vkCreateBuffer;
	functions.vkDestroyBuffer = vkDestroyBuffer;
	functions.vkCreateImage = vkCreateImage;
	functions.vkDestroyImage = vkDestroyImage;
	functions.vkCmdCopyBuffer = vkCmdCopyBuffer;
#if VMA_DEDICATED_ALLOCATION || VMA_VULKAN_VERSION >= 1001000
	functions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2;
	functions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2;
#endif
#if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
	functions.vkBindBufferMemory2KHR = vkBindBufferMemory2;
	functions.vkBindImageMemory2KHR = vkBindImageMemory2;
#endif
#if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
	functions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2;
#endif
#if VMA_VULKAN_VERSION >= 1003000
	functions.vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements;
	functions.vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements;
#endif
	vmaACI.pVulkanFunctions = &functions;

	vmaCreateAllocator(&vmaACI, &m_VMA);
}


void BvRenderDeviceVk::DestroyVMA()
{
	vmaDestroyAllocator(m_VMA);
}


bool IsPhysicalDeviceExtensionSupported(const BvVector<VkExtensionProperties>& supportedExtensions, const char* pExtensionName)
{
	for (auto& physicalDeviceExtension : supportedExtensions)
	{
		if (!strcmp(pExtensionName, physicalDeviceExtension.extensionName))
		{
			return true;
		}
	}

	return false;
}


RenderDeviceCapabilities SetupDeviceInfo(VkPhysicalDevice physicalDevice, BvDeviceInfoVk& deviceInfo)
{
	RenderDeviceCapabilities caps = RenderDeviceCapabilities::kNone;

	// =================================
	// Get / Store supported extensions
	u32 extensionCount = 0;
	auto result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
	if (result != VK_SUCCESS)
	{
		return caps;
	}

	deviceInfo.m_SupportedExtensions.Resize(extensionCount);
	result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, deviceInfo.m_SupportedExtensions.Data());
	if (result != VK_SUCCESS)
	{
		return caps;
	}

	bool swapChain = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME); // Also needs VK_KHR_SURFACE_EXTENSION_NAME
	if (!swapChain)
	{
		return caps;
	}

	deviceInfo.m_DeviceProperties.pNext = &deviceInfo.m_DeviceProperties1_1;
	deviceInfo.m_DeviceProperties1_1.pNext = &deviceInfo.m_DeviceProperties1_2;
	deviceInfo.m_DeviceProperties1_2.pNext = &deviceInfo.m_DeviceProperties1_3;

	deviceInfo.m_DeviceFeatures.pNext = &deviceInfo.m_DeviceFeatures1_1;
	deviceInfo.m_DeviceFeatures1_1.pNext = &deviceInfo.m_DeviceFeatures1_2;
	deviceInfo.m_DeviceFeatures1_2.pNext = &deviceInfo.m_DeviceFeatures1_3;

	void** pNextProperty = &deviceInfo.m_DeviceProperties1_3.pNext;
	void** pNextFeature = &deviceInfo.m_DeviceFeatures1_3.pNext;
	void** pNextMemoryProperty = &deviceInfo.m_DeviceMemoryProperties.pNext;

	bool vertexAttributeDivisor = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
	bool fragmentShading = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
	bool meshShader = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_EXT_MESH_SHADER_EXTENSION_NAME);
	bool accelerationStructure = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
	bool rayTracingPipeline = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
	bool rayQuery = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_KHR_RAY_QUERY_EXTENSION_NAME);
	bool conservativeRasterization = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
	bool customBorderColor = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
	bool memoryBudget = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
	bool deferredHostOperations = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
	bool predication = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
	bool depthClipEnable = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME);

	deviceInfo.m_EnabledExtensions.PushBack(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	if (vertexAttributeDivisor)
	{
		*pNextFeature = &deviceInfo.m_ExtendedFeatures.vertexAttributeDivisorFeatures;
		pNextFeature = &deviceInfo.m_ExtendedFeatures.vertexAttributeDivisorFeatures.pNext;

		*pNextProperty = &deviceInfo.m_ExtendedProperties.vertexAttributeDivisorProperties;
		pNextProperty = &deviceInfo.m_ExtendedProperties.vertexAttributeDivisorProperties.pNext;

		deviceInfo.m_EnabledExtensions.PushBack(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
	}

	if (fragmentShading)
	{
		*pNextFeature = &deviceInfo.m_ExtendedFeatures.fragmentShadingRateFeatures;
		pNextFeature = &deviceInfo.m_ExtendedFeatures.fragmentShadingRateFeatures.pNext;
		*pNextProperty = &deviceInfo.m_ExtendedProperties.fragmentShadingRateProps;
		pNextProperty = &deviceInfo.m_ExtendedProperties.fragmentShadingRateProps.pNext;

		deviceInfo.m_EnabledExtensions.PushBack(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
	}

	if (meshShader)
	{
		*pNextFeature = &deviceInfo.m_ExtendedFeatures.meshShaderFeatures;
		pNextFeature = &deviceInfo.m_ExtendedFeatures.meshShaderFeatures.pNext;

		*pNextProperty = &deviceInfo.m_ExtendedProperties.meshShaderProps;
		pNextProperty = &deviceInfo.m_ExtendedProperties.meshShaderProps.pNext;

		deviceInfo.m_EnabledExtensions.PushBack(VK_EXT_MESH_SHADER_EXTENSION_NAME);
	}

	if (accelerationStructure && deferredHostOperations && deviceInfo.m_DeviceFeatures1_2.bufferDeviceAddress)
	{
		*pNextFeature = &deviceInfo.m_ExtendedFeatures.accelerationStructureFeatures;
		pNextFeature = &deviceInfo.m_ExtendedFeatures.accelerationStructureFeatures.pNext;

		*pNextProperty = &deviceInfo.m_ExtendedProperties.accelerationStructureProps;
		pNextProperty = &deviceInfo.m_ExtendedProperties.accelerationStructureProps.pNext;
		
		deviceInfo.m_EnabledExtensions.PushBack(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
		
		if (rayTracingPipeline)
		{

			*pNextFeature = &deviceInfo.m_ExtendedFeatures.rayTracingPipelineFeatures;
			pNextFeature = &deviceInfo.m_ExtendedFeatures.rayTracingPipelineFeatures.pNext;

			*pNextProperty = &deviceInfo.m_ExtendedProperties.rayTracingPipelineProps;
			pNextProperty = &deviceInfo.m_ExtendedProperties.rayTracingPipelineProps.pNext;

			deviceInfo.m_EnabledExtensions.PushBack(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
		}

		if (rayQuery)
		{
			*pNextFeature = &deviceInfo.m_ExtendedFeatures.rayQueryFeatures;
			pNextFeature = &deviceInfo.m_ExtendedFeatures.rayQueryFeatures.pNext;

			deviceInfo.m_EnabledExtensions.PushBack(VK_KHR_RAY_QUERY_EXTENSION_NAME);
		}
	}

	if (conservativeRasterization)
	{
		*pNextProperty = &deviceInfo.m_ExtendedProperties.convervativeRasterizationProps;
		pNextProperty = &deviceInfo.m_ExtendedProperties.convervativeRasterizationProps.pNext;

		deviceInfo.m_EnabledExtensions.PushBack(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
	}

	if (customBorderColor)
	{
		*pNextFeature = &deviceInfo.m_ExtendedFeatures.customBorderColorFeatures;
		pNextFeature = &deviceInfo.m_ExtendedFeatures.customBorderColorFeatures.pNext;

		*pNextProperty = &deviceInfo.m_ExtendedProperties.customBorderColorProps;
		pNextProperty = &deviceInfo.m_ExtendedProperties.customBorderColorProps.pNext;

		deviceInfo.m_EnabledExtensions.PushBack(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
	}

	if (memoryBudget)
	{
		*pNextMemoryProperty = &deviceInfo.m_ExtendedProperties.memoryBudgetProperties;
		pNextMemoryProperty = &deviceInfo.m_ExtendedProperties.memoryBudgetProperties.pNext;

		deviceInfo.m_EnabledExtensions.PushBack(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
	}

	if (deferredHostOperations)
	{
		deviceInfo.m_EnabledExtensions.PushBack(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
	}

	if (predication)
	{
		*pNextFeature = &deviceInfo.m_ExtendedFeatures.conditionalRenderingFeatures;
		pNextFeature = &deviceInfo.m_ExtendedFeatures.conditionalRenderingFeatures.pNext;

		deviceInfo.m_EnabledExtensions.PushBack(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
	}

	if (depthClipEnable)
	{
		*pNextFeature = &deviceInfo.m_ExtendedFeatures.depthClibEnableFeature;
		pNextFeature = &deviceInfo.m_ExtendedFeatures.depthClibEnableFeature.pNext;

		deviceInfo.m_EnabledExtensions.PushBack(VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME);
	}

	// =================================
	// Store properties / features / memory properties
	vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceInfo.m_DeviceFeatures);
	vkGetPhysicalDeviceProperties2(physicalDevice, &deviceInfo.m_DeviceProperties);
	vkGetPhysicalDeviceMemoryProperties2(physicalDevice, &deviceInfo.m_DeviceMemoryProperties);

	if (deviceInfo.m_DeviceFeatures.features.fillModeNonSolid)
	{
		caps |= RenderDeviceCapabilities::kWireframe;
	}
	if (deviceInfo.m_DeviceFeatures.features.geometryShader)
	{
		caps |= RenderDeviceCapabilities::kGeometryShader;
	}
	if (deviceInfo.m_DeviceFeatures.features.tessellationShader)
	{
		caps |= RenderDeviceCapabilities::kTesselationShader;
	}
	if (deviceInfo.m_DeviceFeatures.features.depthBounds)
	{
		caps |= RenderDeviceCapabilities::kDepthBoundsTest;
	}
	if (deviceInfo.m_DeviceProperties.properties.limits.timestampComputeAndGraphics)
	{
		caps |= RenderDeviceCapabilities::kTimestampQueries;
	}
	if (deviceInfo.m_DeviceFeatures.features.multiDrawIndirect)
	{
		caps |= RenderDeviceCapabilities::kIndirectDrawCount;
	}
	if (customBorderColor)
	{
		caps |= RenderDeviceCapabilities::kCustomBorderColor;
	}
	if (predication)
	{
		caps |= RenderDeviceCapabilities::kPredication;
	}
	if (conservativeRasterization)
	{
		caps |= RenderDeviceCapabilities::kConservativeRasterization;
	}
	if (fragmentShading)
	{
		caps |= RenderDeviceCapabilities::kShadingRate;
	}
	if (meshShader && deviceInfo.m_ExtendedFeatures.meshShaderFeatures.meshShader && deviceInfo.m_ExtendedFeatures.meshShaderFeatures.taskShader)
	{
		caps |= RenderDeviceCapabilities::kMeshShader;
	}
	if (rayTracingPipeline)
	{
		caps |= RenderDeviceCapabilities::kRayTracing;
	}
	if (rayQuery)
	{
		caps |= RenderDeviceCapabilities::kRayQuery;
	}

	// =================================
	// Get the queue families
	u32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyCount, nullptr);

	deviceInfo.m_QueueFamilyProperties.Resize(queueFamilyCount, VkQueueFamilyProperties2{ VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2 });
	vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyCount, deviceInfo.m_QueueFamilyProperties.Data());

	bool isAsync;
	u32 queueIndex;
	if ((queueIndex = GetQueueFamilyIndex(deviceInfo.m_QueueFamilyProperties, VK_QUEUE_GRAPHICS_BIT, isAsync)) != kU32Max)
	{
		deviceInfo.m_GraphicsQueueInfo.m_QueueFamilyIndex = queueIndex;
		deviceInfo.m_GraphicsQueueInfo.m_QueueCount = deviceInfo.m_QueueFamilyProperties[queueIndex].queueFamilyProperties.queueCount;
		deviceInfo.m_GraphicsQueueInfo.m_SupportsPresent = QueueSupportsPresent(physicalDevice, queueIndex);
	}
	if ((queueIndex = GetQueueFamilyIndex(deviceInfo.m_QueueFamilyProperties, VK_QUEUE_COMPUTE_BIT, isAsync)) != kU32Max && isAsync)
	{
		deviceInfo.m_ComputeQueueInfo.m_QueueFamilyIndex = queueIndex;
		deviceInfo.m_ComputeQueueInfo.m_QueueCount = deviceInfo.m_QueueFamilyProperties[queueIndex].queueFamilyProperties.queueCount;
		deviceInfo.m_ComputeQueueInfo.m_SupportsPresent = QueueSupportsPresent(physicalDevice, queueIndex);
	}
	if ((queueIndex = GetQueueFamilyIndex(deviceInfo.m_QueueFamilyProperties, VK_QUEUE_TRANSFER_BIT, isAsync)) != kU32Max && isAsync)
	{
		deviceInfo.m_TransferQueueInfo.m_QueueFamilyIndex = queueIndex;
		deviceInfo.m_TransferQueueInfo.m_QueueCount = deviceInfo.m_QueueFamilyProperties[queueIndex].queueFamilyProperties.queueCount;
		deviceInfo.m_TransferQueueInfo.m_SupportsPresent = QueueSupportsPresent(physicalDevice, queueIndex);
	}

	return caps;
}


u32 GetQueueFamilyIndex(const BvVector<VkQueueFamilyProperties2>& queueFamilyProperties, VkQueueFlags queueFlags, bool& isAsync)
{
	u32 index = kU32Max;

	isAsync = false;

	// Dedicated queue for compute
	// Try to find a queue family index that supports compute but not graphics
	if (queueFlags == VK_QUEUE_COMPUTE_BIT)
	{
		for (u32 i = 0; i < static_cast<u32>(queueFamilyProperties.Size()); i++)
		{
			if ((queueFamilyProperties[i].queueFamilyProperties.queueFlags & queueFlags)
				&& ((queueFamilyProperties[i].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
			{
				index = i;
				isAsync = true;
				break;
			}
		}
	}

	// Dedicated queue for transfer
	// Try to find a queue family index that supports transfer but not graphics and compute
	else if (queueFlags == VK_QUEUE_TRANSFER_BIT)
	{
		for (u32 i = 0; i < static_cast<u32>(queueFamilyProperties.Size()); i++)
		{
			if ((queueFamilyProperties[i].queueFamilyProperties.queueFlags & queueFlags)
				&& ((queueFamilyProperties[i].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
				&& ((queueFamilyProperties[i].queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
			{
				index = i;
				isAsync = true;
				break;
			}
		}
	}

	if (index == kU32Max)
	{
		// For other queue types or if no separate compute or transfer queue is present, return the first one to support the requested flags
		for (u32 i = 0; i < static_cast<u32>(queueFamilyProperties.Size()); i++)
		{
			if (queueFamilyProperties[i].queueFamilyProperties.queueFlags & queueFlags)
			{
				index = i;
				break;
			}
		}
	}

	return index;
}


bool QueueSupportsPresent(VkPhysicalDevice physicalDevice, u32 index)
{
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	if (vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, index))
	{
		return true;
	}
#endif

	return false;
}