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


BvRenderDeviceVk::BvRenderDeviceVk(BvRenderEngineVk* pEngine, BvGPUInfoVk& gpuInfo, const BvRenderDeviceCreateDescVk& deviceDesc)
	: m_pEngine(pEngine), m_GPUInfo(gpuInfo)
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

	auto & memoryProperties = m_GPUInfo.m_DeviceMemoryProperties;
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
		vkGetPhysicalDeviceFormatProperties(m_GPUInfo.m_PhysicalDevice, depthFormat, &formatProps);
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
	vkGetPhysicalDeviceFormatProperties(m_GPUInfo.m_PhysicalDevice, GetVkFormat(format), &formatProperties);

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
		if (m_GPUInfo.m_DeviceProperties.properties.limits.timestampPeriod == 0)
		{
			return false;
		}

		if (m_GPUInfo.m_DeviceProperties.properties.limits.timestampComputeAndGraphics
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
				pProperties = &m_GPUInfo.m_QueueFamilyProperties[m_GPUInfo.m_GraphicsQueueInfo.m_QueueFamilyIndex].queueFamilyProperties;
				break;
			case CommandType::kCompute:
				pProperties = &m_GPUInfo.m_QueueFamilyProperties[m_GPUInfo.m_ComputeQueueInfo.m_QueueFamilyIndex].queueFamilyProperties;
				break;
			case CommandType::kTransfer:
				pProperties = &m_GPUInfo.m_QueueFamilyProperties[m_GPUInfo.m_TransferQueueInfo.m_QueueFamilyIndex].queueFamilyProperties;
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
	BvAssert(deviceCreateDesc.m_GraphicsQueueCount + deviceCreateDesc.m_ComputeQueueCount + deviceCreateDesc.m_TransferQueueCount > 0,
		"No device queues");

	constexpr u32 kMaxQueueCount = 16;
	BvAssert(deviceCreateDesc.m_GraphicsQueueCount <= kMaxQueueCount, "Graphics queue count greater than limit");
	BvAssert(deviceCreateDesc.m_ComputeQueueCount <= kMaxQueueCount, "Compute queue count greater than limit");
	BvAssert(deviceCreateDesc.m_TransferQueueCount <= kMaxQueueCount, "Transfer queue count greater than limit");

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

	// ===========================================================
	// Prepare Device Queues and create the logical device
	constexpr auto kMaxQueueTypes = 3;
	BvFixedVector<VkDeviceQueueCreateInfo, kMaxQueueTypes> queueInfos;

	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.pQueuePriorities = queuePriorities;
	if (deviceCreateDesc.m_GraphicsQueueCount > 0)
	{
		BvAssert(deviceCreateDesc.m_GraphicsQueueCount <= m_GPUInfo.m_GraphicsQueueInfo.m_QueueCount, "Not enough graphics queues available");
		queueCreateInfo.queueFamilyIndex = m_GPUInfo.m_GraphicsQueueInfo.m_QueueFamilyIndex;
		queueCreateInfo.queueCount = m_GPUInfo.m_GraphicsQueueInfo.m_QueueCount;
		queueInfos.PushBack(queueCreateInfo);

		m_GraphicsContexts.Resize(deviceCreateDesc.m_GraphicsQueueCount);
	}
	if (deviceCreateDesc.m_ComputeQueueCount > 0)
	{
		BvAssert(deviceCreateDesc.m_ComputeQueueCount <= m_GPUInfo.m_ComputeQueueInfo.m_QueueCount, "Not enough compute queues available");
		queueCreateInfo.queueFamilyIndex = m_GPUInfo.m_ComputeQueueInfo.m_QueueFamilyIndex;
		queueCreateInfo.queueCount = m_GPUInfo.m_ComputeQueueInfo.m_QueueCount;
		queueInfos.PushBack(queueCreateInfo);

		m_ComputeContexts.Resize(deviceCreateDesc.m_ComputeQueueCount);
	}
	if (deviceCreateDesc.m_TransferQueueCount > 0)
	{
		BvAssert(deviceCreateDesc.m_TransferQueueCount <= m_GPUInfo.m_TransferQueueInfo.m_QueueCount, "Not enough transfer queues available");
		queueCreateInfo.queueFamilyIndex = m_GPUInfo.m_TransferQueueInfo.m_QueueFamilyIndex;
		queueCreateInfo.queueCount = m_GPUInfo.m_TransferQueueInfo.m_QueueCount;
		queueInfos.PushBack(queueCreateInfo);

		m_TransferContexts.Resize(deviceCreateDesc.m_TransferQueueCount);
	}

	VkDeviceCreateInfo deviceCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	//deviceCreateInfo.pEnabledFeatures = nullptr;
	deviceCreateInfo.pNext = &m_GPUInfo.m_DeviceFeatures;
	deviceCreateInfo.pQueueCreateInfos = queueInfos.Data();
	deviceCreateInfo.queueCreateInfoCount = static_cast<u32>(queueInfos.Size());
	deviceCreateInfo.ppEnabledExtensionNames = m_GPUInfo.m_EnabledExtensions.Data();
	deviceCreateInfo.enabledExtensionCount = static_cast<u32>(m_GPUInfo.m_EnabledExtensions.Size());

	VkResult result = vkCreateDevice(m_GPUInfo.m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device);
	BvAssert(result == VK_SUCCESS, "Couldn't create a logical device");
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		return;
	}

	volkLoadDevice(m_Device);

	m_GPUInfo.m_QueueFamilyIndices.Reserve(3);
	for (auto i = 0; i < deviceCreateDesc.m_GraphicsQueueCount; i++)
	{
		m_GraphicsContexts[i] = new BvCommandContextVk(this, 3, CommandType::kGraphics, m_GPUInfo.m_GraphicsQueueInfo.m_QueueFamilyIndex, i);
		m_GPUInfo.m_QueueFamilyIndices.EmplaceBack(m_GPUInfo.m_GraphicsQueueInfo.m_QueueFamilyIndex);
	}
	for (auto i = 0; i < deviceCreateDesc.m_ComputeQueueCount; i++)
	{
		m_ComputeContexts[i] = new BvCommandContextVk(this, 3, CommandType::kCompute, m_GPUInfo.m_ComputeQueueInfo.m_QueueFamilyIndex, i);
		m_GPUInfo.m_QueueFamilyIndices.EmplaceBack(m_GPUInfo.m_ComputeQueueInfo.m_QueueFamilyIndex);
	}
	for (auto i = 0; i < deviceCreateDesc.m_TransferQueueCount; i++)
	{
		m_TransferContexts[i] = new BvCommandContextVk(this, 3, CommandType::kTransfer, m_GPUInfo.m_TransferQueueInfo.m_QueueFamilyIndex, i);
		m_GPUInfo.m_QueueFamilyIndices.EmplaceBack(m_GPUInfo.m_TransferQueueInfo.m_QueueFamilyIndex);
	}

	CreateVMA();

	m_pFramebufferManager = new BvFramebufferManagerVk();

	u32 querySizes[kQueryTypeCount]{ 16, 16, 16 };
	m_pQueryHeapManager = new BvQueryHeapManagerVk(this, querySizes, 3);
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
}


void BvRenderDeviceVk::CreateVMA()
{
	VmaAllocatorCreateInfo vmaACI{};
	vmaACI.instance = m_pEngine->GetHandle();
	vmaACI.device = m_Device;
	vmaACI.physicalDevice = m_GPUInfo.m_PhysicalDevice;
	vmaACI.vulkanApiVersion = m_GPUInfo.m_DeviceProperties.properties.apiVersion;

	if (m_GPUInfo.m_DeviceFeatures1_2.bufferDeviceAddress)
	{
		vmaACI.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	}
	if (m_GPUInfo.m_FeaturesSupported.memoryBudget)
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