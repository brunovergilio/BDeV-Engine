#include "BvRenderDeviceVk.h"
#include "BvUtilsVk.h"
#include "BvCommandQueueVk.h"
#include "BvSwapChainVk.h"
#include "BvRenderPassVk.h"
#include "BvCommandPoolVk.h"
#include "BvShaderResourceVk.h"
#include "BvShaderVk.h"
#include "BvPipelineStateVk.h"
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
#include "BvGPUFenceVk.h"
#include "BvAccelerationStructureVk.h"
#include "BvShaderBindingTableVk.h"
#include "BDeV/Core/RenderAPI/BvRenderAPIUtils.h"


extern void OnVkDeviceDestroyed(u32 index);


BvRenderDeviceVk::BvRenderDeviceVk(VkInstance instance, VkPhysicalDevice physicalDevice, BvDeviceInfoVk* pDeviceInfo, u32 index,const BvGPUInfo& gpuInfo, const RenderDeviceDesc& renderDeviceDesc)
	: m_Instance(instance), m_PhysicalDevice(physicalDevice), m_pDeviceInfo(pDeviceInfo), m_GPUInfo(gpuInfo), m_Index(index)
{
	Create(renderDeviceDesc);
}


BvRenderDeviceVk::~BvRenderDeviceVk()
{
	Destroy();
}


bool BvRenderDeviceVk::CreateSwapChainImpl(BvWindow* pWindow, const SwapChainDesc& desc, IBvCommandContext* pContext, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	SwapChainDesc scd(desc);
	auto result = VkHelpers::CreateSwapChain(this, m_Instance, TO_VK(pContext)->GetGroupIndex(), nullptr, VK_NULL_HANDLE, scd, pWindow);
	if (result.first != VK_SUCCESS)
	{
		return false;
	}

	auto& obj = result.second;
	*ppObj = BV_RC_CREATE(BvSwapChainVk, this, obj.m_SwapChain, obj.m_Surface, obj.m_Images, obj.m_Extents, pWindow, scd, TO_VK(pContext));

	return true;
}


bool BvRenderDeviceVk::CreateBufferImpl(const BufferDesc& desc, const BufferInitData* pInitData, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	auto result = VkHelpers::CreateBuffer(this, desc);
	if (result.first != VK_SUCCESS)
	{
		return false;
	}

	auto& obj = result.second;
	if (pInitData)
	{
		VkHelpers::UploadMemoryToGPU(this, obj.m_Buffer, *pInitData);
	}

	*ppObj = BV_RC_CREATE(BvBufferVk, this, desc, obj.m_Buffer, obj.m_DeviceAddress, obj.m_Memory, obj.m_pMappedMemory, obj.m_NeedsFlush);

	return true;
}


bool BvRenderDeviceVk::CreateBufferViewImpl(const BufferViewDesc& desc, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	auto result = VkHelpers::CreateBufferView(this, desc);
	if (result.first != VK_SUCCESS)
	{
		return false;
	}

	*ppObj = BV_RC_CREATE(BvBufferViewVk, this, desc, result.second);

	return true;
}


bool BvRenderDeviceVk::CreateTextureImpl(const TextureDesc& desc, const TextureInitData* pInitData, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	u32 mutableFormatCount = 0;
	const Format* pMutableFormats = nullptr;
	if (pInitData)
	{
		mutableFormatCount = pInitData->m_MutableFormatCount;
		pMutableFormats = pInitData->m_pMutableFormats;
	}

	auto result = VkHelpers::CreateTexture(this, desc, mutableFormatCount, pMutableFormats);
	if (result.first != VK_SUCCESS)
	{
		return false;
	}

	auto& obj = result.second;
	if (pInitData)
	{
		VkHelpers::UploadMemoryToGPU(this, obj.m_Texture, desc, *pInitData);
	}

	*ppObj = BV_RC_CREATE(BvTextureVk, this, desc, obj.m_Texture, obj.m_Memory);

	return true;
}


bool BvRenderDeviceVk::CreateTextureViewImpl(const TextureViewDesc& desc, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	auto result = VkHelpers::CreateTextureView(this, desc);
	if (result.first != VK_SUCCESS)
	{
		return false;
	}

	*ppObj = BV_RC_CREATE(BvTextureViewVk, this, desc, result.second);

	return true;
}


bool BvRenderDeviceVk::CreateSamplerImpl(const SamplerDesc& desc, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	auto result = VkHelpers::CreateSampler(this, desc);
	if (result.first != VK_SUCCESS)
	{
		return false;
	}

	*ppObj = BV_RC_CREATE(BvSamplerVk, this, desc, result.second);

	return true;
}


bool BvRenderDeviceVk::CreateRenderPassImpl(const RenderPassDesc& renderPassDesc, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	auto result = VkHelpers::CreateRenderPass(this, renderPassDesc);
	if (result.first != VK_SUCCESS)
	{
		return false;
	}

	*ppObj = BV_RC_CREATE(BvRenderPassVk, this, renderPassDesc, result.second);

	return true;
}


bool BvRenderDeviceVk::CreateShaderResourceLayoutImpl(const ShaderResourceLayoutCreateDesc& srlDesc, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	auto result = VkHelpers::CreateShaderResourceLayout(this, srlDesc);
	if (result.first != VK_SUCCESS)
	{
		return false;
	}

	auto& obj = result.second;
	*ppObj = BV_RC_CREATE(BvShaderResourceLayoutVk, this, srlDesc, obj.m_DescriptorSetLayouts, obj.m_PipelineLayout);

	return true;
}


bool BvRenderDeviceVk::CreateShaderImpl(const ShaderDesc& shaderDesc, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	*ppObj = BV_RC_CREATE(BvShaderVk, this, shaderDesc);

	return true;
}


bool BvRenderDeviceVk::CreateGraphicsPipelineImpl(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc, IBvPipelineCache* pPipelineCache, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	auto cache = pPipelineCache ? TO_VK(pPipelineCache)->GetHandle() : VK_NULL_HANDLE;
	auto result = VkHelpers::CreateGraphicsPipeline(this, graphicsPipelineStateDesc, cache);
	if (result.first != VK_SUCCESS)
	{
		return false;
	}

	*ppObj = BV_RC_CREATE(BvGraphicsPipelineStateVk, this, graphicsPipelineStateDesc, result.second);

	return true;
}


bool BvRenderDeviceVk::CreateComputePipelineImpl(const ComputePipelineStateDesc& computePipelineStateDesc, IBvPipelineCache* pPipelineCache, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	auto cache = pPipelineCache ? TO_VK(pPipelineCache)->GetHandle() : VK_NULL_HANDLE;
	auto result = VkHelpers::CreateComputePipeline(this, computePipelineStateDesc, cache);
	if (result.first != VK_SUCCESS)
	{
		return false;
	}

	*ppObj = BV_RC_CREATE(BvComputePipelineStateVk, this, computePipelineStateDesc, result.second);

	return true;
}


bool BvRenderDeviceVk::CreateRayTracingPipelineImpl(const RayTracingPipelineStateDesc& rayTracingPipelineStateDesc, IBvPipelineCache* pPipelineCache, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	auto cache = pPipelineCache ? TO_VK(pPipelineCache)->GetHandle() : VK_NULL_HANDLE;
	auto result = VkHelpers::CreateRayTracingPipeline(this, rayTracingPipelineStateDesc, cache);
	if (result.first != VK_SUCCESS)
	{
		return false;
	}

	*ppObj = BV_RC_CREATE(BvRayTracingPipelineStateVk, this, rayTracingPipelineStateDesc, result.second);

	return true;
}


bool BvRenderDeviceVk::CreateQueryImpl(QueryType queryType, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	*ppObj = BV_RC_CREATE(BvQueryVk, this, queryType, 3);

	return true;
}


bool BvRenderDeviceVk::CreateFenceImpl(const GPUFenceDesc& fenceDesc, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	auto result = VkHelpers::CreateSemaphore(this, fenceDesc);
	if (result.first != VK_SUCCESS)
	{
		return false;
	}

	*ppObj = BV_RC_CREATE(BvGPUFenceVk, this, result.second);

	return true;
}


bool BvRenderDeviceVk::CreateAccelerationStructureImpl(const RayTracingAccelerationStructureDesc& asDesc, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	auto result = VkHelpers::CreateAccelerationStructure(this, asDesc);
	if (result.first != VK_SUCCESS)
	{
		return false;
	}

	auto& obj = result.second;
	*ppObj = BV_RC_CREATE(BvAccelerationStructureVk, this, asDesc, obj.m_AS, obj.m_DeviceAddress, obj.m_Geometries, obj.m_PrimitiveCounts,
		obj.m_BufferObj.m_Buffer, obj.m_BufferObj.m_Memory, obj.m_BufferObj.m_DeviceAddress, obj.m_ScratchSizes);

	return true;
}


bool BvRenderDeviceVk::CreateShaderBindingTableImpl(const ShaderBindingTableDesc& sbtDesc, IBvCommandContext* pContext, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	auto result = VkHelpers::CreateShaderBindingTable(this, sbtDesc, TO_VK(pContext));
	if (result.first != VK_SUCCESS)
	{
		return false;
	}

	auto& obj = result.second;
	*ppObj = BV_RC_CREATE(BvShaderBindingTableVk, this, sbtDesc, obj.m_BufferObj.m_Buffer, obj.m_BufferObj.m_DeviceAddress,
		obj.m_BufferObj.m_Memory, obj.m_Regions);

	return true;
}


bool BvRenderDeviceVk::CreateCommandContextImpl(const CommandContextDesc& commandContextDesc, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	BvCommandContextVk* pContext = nullptr;
	u32 contextGroupIndex = kU32Max;
	// If we have a specific, valid context group index, choose that
	if (commandContextDesc.m_ContextGroupIndex < m_Contexts.Size())
	{
		contextGroupIndex = commandContextDesc.m_ContextGroupIndex;
	}
	// Otherwise, we look for a command queue based on its type
	else if (commandContextDesc.m_CommandType != CommandType::kNone)
	{
		for (auto i = 0; i < m_GPUInfo.m_ContextGroups.Size(); ++i)
		{
			// Some contexts may not be used or already have used all its available slots
			if (m_Contexts[i].Size() == m_Contexts[i].Capacity())
			{
				continue;
			}

			// We need either a dedicated command context of a given type or a non-dedicated
			// (if not required) context that supports the requested command type
			auto& contextGroup = m_GPUInfo.m_ContextGroups[i];
			if (contextGroup.m_DedicatedCommandType == commandContextDesc.m_CommandType
				|| (!commandContextDesc.m_RequireDedicated && contextGroup.SupportsCommandType(commandContextDesc.m_CommandType)))
			{
				contextGroupIndex = i;
				break;
			}
		}
	}
	else
	{
		BV_ASSERT(false, "Invalid command context creation parameters");
		return false;
	}

	if (contextGroupIndex < m_Contexts.Size())
	{
		auto& commandContextGroup = m_Contexts[contextGroupIndex];
		pContext = BV_RC_CREATE(BvCommandContextVk, this, 3, contextGroupIndex, commandContextGroup.Size());
		commandContextGroup.EmplaceBack(pContext);
	}

	*ppObj = pContext;

	return true;
}


bool BvRenderDeviceVk::CreatePipelineCacheImpl(const PipelineCacheInitData* pInitData, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	auto result = VkHelpers::CreatePipelineCache(this, pInitData);
	if (result.first != VK_SUCCESS)
	{
		return false;
	}

	*ppObj = BV_RC_CREATE(BvPipelineCacheVk, this, result.second);

	return true;
}


void BvRenderDeviceVk::WaitIdle() const
{
	auto result = vkDeviceWaitIdle(m_Device);
}


void BvRenderDeviceVk::GetCopyableFootprints(const TextureDesc& textureDesc, u32 subresourceCount, SubresourceFootprint* pSubresources, u64* pTotalSize,
	u64 baseOffset, u64 firstSubresource) const
{
	auto& limits = m_pDeviceInfo->m_DeviceProperties.properties.limits;
	auto totalSize = BvRenderUtils::GetCopyableFootprints(textureDesc, limits.optimalBufferCopyOffsetAlignment, limits.optimalBufferCopyRowPitchAlignment,
		subresourceCount, pSubresources, baseOffset, firstSubresource);

	if (pTotalSize)
	{
		*pTotalSize = totalSize;
	}
}


u64 BvRenderDeviceVk::GetDynamicBufferElementSize(BufferUsage usageFlags, u64 elementStride) const
{
	u64 alignment = elementStride;
	auto& limits = m_pDeviceInfo->m_DeviceProperties.properties.limits;
	if (EHasFlag(usageFlags, BufferUsage::kConstantBuffer))
	{
		alignment = std::max(limits.minUniformBufferOffsetAlignment, alignment);
	}
	if (EHasFlag(usageFlags, BufferUsage::kStructuredBuffer) || EHasFlag(usageFlags, BufferUsage::kRWStructuredBuffer))
	{
		alignment = std::max(limits.minStorageBufferOffsetAlignment, alignment);
	}
	
	return RoundToNearestPowerOf2(elementStride, alignment);
}


FormatFeatures BvRenderDeviceVk::GetFormatFeatures(Format format) const
{
	FormatFeatures formatFeatures = FormatFeatures::kNone;

	const auto& vkFormatMap = GetVkFormatMap(format);
	if (vkFormatMap.format == VK_FORMAT_UNDEFINED)
	{
		return formatFeatures;
	}

	VkImageFormatProperties2 imageFormatProperties{ VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2 };
	VkPhysicalDeviceImageFormatInfo2 imageFormatInfo{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2 };
	imageFormatInfo.format = vkFormatMap.format;
	imageFormatInfo.tiling = VK_IMAGE_TILING_OPTIMAL; // We only care about optimal tiling

	VkFormatProperties2 formatProperties{ VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2 };
	vkGetPhysicalDeviceFormatProperties2(m_PhysicalDevice, vkFormatMap.format, &formatProperties);
	auto textureFeatures = formatProperties.formatProperties.optimalTilingFeatures;
	if ((textureFeatures & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT)
		&& (textureFeatures & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
	{
		formatFeatures |= FormatFeatures::kSampling;
		formatFeatures |= FormatFeatures::kComparisonSampling;

		imageFormatInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

		imageFormatInfo.type = VK_IMAGE_TYPE_1D;
		if (vkGetPhysicalDeviceImageFormatProperties2(m_PhysicalDevice, &imageFormatInfo, &imageFormatProperties) == VK_SUCCESS)
		{
			formatFeatures |= FormatFeatures::kTexture1D;
		}

		imageFormatInfo.type = VK_IMAGE_TYPE_2D;
		if (vkGetPhysicalDeviceImageFormatProperties2(m_PhysicalDevice, &imageFormatInfo, &imageFormatProperties) == VK_SUCCESS)
		{
			formatFeatures |= FormatFeatures::kTexture2D;
		}

		imageFormatInfo.type = VK_IMAGE_TYPE_3D;
		if (vkGetPhysicalDeviceImageFormatProperties2(m_PhysicalDevice, &imageFormatInfo, &imageFormatProperties) == VK_SUCCESS)
		{
			formatFeatures |= FormatFeatures::kTexture3D;
		}

		imageFormatInfo.type = VK_IMAGE_TYPE_2D;
		imageFormatInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		if (vkGetPhysicalDeviceImageFormatProperties2(m_PhysicalDevice, &imageFormatInfo, &imageFormatProperties) == VK_SUCCESS)
		{
			formatFeatures |= FormatFeatures::kTextureCube;
		}
		imageFormatInfo.flags = 0;
	}

	if (textureFeatures & VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT)
	{
		formatFeatures |= FormatFeatures::kRWTexture;
	}

	if (textureFeatures & VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT)
	{
		imageFormatInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		imageFormatInfo.type = VK_IMAGE_TYPE_2D;
		if (vkGetPhysicalDeviceImageFormatProperties2(m_PhysicalDevice, &imageFormatInfo, &imageFormatProperties) == VK_SUCCESS)
		{
			formatFeatures |= FormatFeatures::kRenderTarget;
			if (imageFormatProperties.imageFormatProperties.sampleCounts > VK_SAMPLE_COUNT_1_BIT)
			{
				formatFeatures |= FormatFeatures::kMultisampleRenderTarget;
			}
		}
	}

	if (textureFeatures & VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BLEND_BIT)
	{
		imageFormatInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		imageFormatInfo.type = VK_IMAGE_TYPE_2D;
		if (vkGetPhysicalDeviceImageFormatProperties2(m_PhysicalDevice, &imageFormatInfo, &imageFormatProperties) == VK_SUCCESS)
		{
			formatFeatures |= FormatFeatures::kBlendable;
		}
	}

	if (textureFeatures & VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		imageFormatInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		imageFormatInfo.type = VK_IMAGE_TYPE_2D;
		if (vkGetPhysicalDeviceImageFormatProperties2(m_PhysicalDevice, &imageFormatInfo, &imageFormatProperties) == VK_SUCCESS)
		{
			formatFeatures |= FormatFeatures::kDepthStencil;
		}
	}

	if ((textureFeatures & VK_FORMAT_FEATURE_2_BLIT_SRC_BIT) && (textureFeatures & VK_FORMAT_FEATURE_2_BLIT_DST_BIT))
	{
		formatFeatures |= FormatFeatures::kResolve;
	}

	auto bufferFeatures = formatProperties.formatProperties.bufferFeatures;
	if (bufferFeatures & VK_FORMAT_FEATURE_2_VERTEX_BUFFER_BIT)
	{
		formatFeatures |= FormatFeatures::kVertexBuffer;
	}

	if (bufferFeatures & VK_FORMAT_FEATURE_2_UNIFORM_TEXEL_BUFFER_BIT)
	{
		formatFeatures |= FormatFeatures::kBuffer;
	}

	if (bufferFeatures & VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_BIT)
	{
		formatFeatures |= FormatFeatures::kRWBuffer;
	}
	
	return formatFeatures;
}


void BvRenderDeviceVk::Create(const RenderDeviceDesc& renderDeviceDesc)
{
	BV_ASSERT(renderDeviceDesc.m_ContextGroups.Size() > 0, "No device queues");

	constexpr u32 kMaxQueueCount = 16;

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
	BvFixedVector<VkDeviceQueueCreateInfo, kMaxContextGroupCount> queueInfos(renderDeviceDesc.m_ContextGroups.Size(), { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO });
	for (auto i = 0; i < queueInfos.Size(); ++i)
	{
		auto& queueCreateInfo = queueInfos[i];
		queueCreateInfo.pQueuePriorities = queuePriorities;

		auto familyIndex = renderDeviceDesc.m_ContextGroups[i].m_GroupIndex;
		auto& familyProps = m_pDeviceInfo->m_QueueFamilyProperties[familyIndex].queueFamilyProperties;
		BV_ASSERT_ONCE(renderDeviceDesc.m_ContextGroups[i].m_ContextCount <= familyProps.queueCount,
			"Using more context than what is available - truncating to maximum allowed");
		queueCreateInfo.queueFamilyIndex = familyIndex;
		queueCreateInfo.queueCount = std::min(renderDeviceDesc.m_ContextGroups[i].m_ContextCount, familyProps.queueCount);

		m_Contexts[familyIndex].Reserve(queueCreateInfo.queueCount);
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

	volkLoadDevice(m_Device);

	SetupSupportedDisplayFormats();

	CreateVMA();

	if (renderDeviceDesc.m_ExtendedBackendOptions == RenderDeviceDesc::kVulkan)
	{
		m_MaxSetsPerDescriptorPool = renderDeviceDesc.m_Vulkan.m_MaxSetsPerDescriptorPool;
		memcpy(m_QueryPoolSizes, renderDeviceDesc.m_Vulkan.m_QueryPoolSizes, sizeof(m_QueryPoolSizes));
		m_AccelerationStructureQueryPoolSize = renderDeviceDesc.m_Vulkan.m_AccelerationStructureQueryPoolSize;
	}
	else
	{
		constexpr u32 kDefaultMaxSetsPerDescriptorPool = 16;
		constexpr u32 kDefaultQueryPoolSizes[kQueryTypeCount] = { 8, 4, 4, 2, 2 };
		constexpr u32 kAccelerationStructureQueryPoolSize = 2;

		m_MaxSetsPerDescriptorPool = kDefaultMaxSetsPerDescriptorPool;
		memcpy(m_QueryPoolSizes, kDefaultQueryPoolSizes, sizeof(m_QueryPoolSizes));
		m_AccelerationStructureQueryPoolSize = kAccelerationStructureQueryPoolSize;
	}
}


void BvRenderDeviceVk::Destroy()
{
	if (m_Device)
	{
		vkDeviceWaitIdle(m_Device);

		DestroyVMA();
		
		vkDestroyDevice(m_Device, nullptr);
		m_Device = VK_NULL_HANDLE;
		
		OnVkDeviceDestroyed(m_Index);
	}
}


void BvRenderDeviceVk::CreateVMA()
{
	VmaAllocatorCreateInfo vmaACI{};
	vmaACI.instance = m_Instance;
	vmaACI.device = m_Device;
	vmaACI.physicalDevice = m_PhysicalDevice;
	vmaACI.vulkanApiVersion = VK_API_VERSION_1_3;

	if (m_pDeviceInfo->m_DeviceFeatures1_2.bufferDeviceAddress)
	{
		vmaACI.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	}
	if (m_pDeviceInfo->m_FeatureFlags.memoryBudget)
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