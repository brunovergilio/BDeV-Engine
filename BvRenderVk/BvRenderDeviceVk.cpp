#include "BvRenderDeviceVk.h"
#include "BvRenderEngineVk.h"
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
#include "BvQueryVk.h"
#include "BvGPUFenceVk.h"
#include "BvAccelerationStructureVk.h"
#include "BvShaderBindingTableVk.h"
#include "BDeV/Core/RenderAPI/BvRenderAPIUtils.h"


BvRenderDeviceVk::BvRenderDeviceVk(BvRenderEngineVk* pEngine, VkPhysicalDevice physicalDevice, BvDeviceInfoVk* pDeviceInfo, u32 index, const BvGPUInfo& gpuInfo, const BvRenderDeviceCreateDescVk& deviceDesc)
	: m_pEngine(pEngine), m_PhysicalDevice(physicalDevice), m_pDeviceInfo(pDeviceInfo), m_GPUInfo(gpuInfo), m_Index(index)
{
	Create(deviceDesc);
}


BvRenderDeviceVk::~BvRenderDeviceVk()
{
	Destroy();
}


bool BvRenderDeviceVk::CreateSwapChainImpl(BvWindow* pWindow, const SwapChainDesc& desc, IBvCommandContext* pContext, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_VK_IS_TYPE_VALID(objId, BvSwapChain)))
	{
		return false;
	}

	auto pObj = CreateResource<BvSwapChainVk>(this, pWindow, desc, TO_VK(pContext));
	m_DeviceObjects.Emplace(pObj);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceVk::CreateBufferImpl(const BufferDesc& desc, const BufferInitData* pInitData, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_VK_IS_TYPE_VALID(objId, BvBuffer)))
	{
		return false;
	}

	auto pObj = CreateResource<BvBufferVk>(this, desc, pInitData);
	m_DeviceObjects.Emplace(pObj);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceVk::CreateBufferViewImpl(const BufferViewDesc& desc, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_VK_IS_TYPE_VALID(objId, BvBufferView)))
	{
		return false;
	}

	auto pObj = CreateResource<BvBufferViewVk>(this, desc);
	m_DeviceObjects.Emplace(pObj);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceVk::CreateTextureImpl(const TextureDesc& desc, const TextureInitData* pInitData, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_VK_IS_TYPE_VALID(objId, BvTexture)))
	{
		return false;
	}

	auto pObj = CreateResource<BvTextureVk>(this, desc, pInitData);
	m_DeviceObjects.Emplace(pObj);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceVk::CreateTextureViewImpl(const TextureViewDesc& desc, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_VK_IS_TYPE_VALID(objId, BvTextureView)))
	{
		return false;
	}

	auto pObj = CreateResource<BvTextureViewVk>(this, desc);
	m_DeviceObjects.Emplace(pObj);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceVk::CreateSamplerImpl(const SamplerDesc& desc, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_VK_IS_TYPE_VALID(objId, BvSampler)))
	{
		return false;
	}

	auto pObj = CreateResource<BvSamplerVk>(this, desc);
	m_DeviceObjects.Emplace(pObj);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceVk::CreateRenderPassImpl(const RenderPassDesc& renderPassDesc, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_VK_IS_TYPE_VALID(objId, BvRenderPass)))
	{
		return false;
	}

	auto pObj = CreateResource<BvRenderPassVk>(this, renderPassDesc);
	m_DeviceObjects.Emplace(pObj);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceVk::CreateShaderResourceLayoutImpl(const ShaderResourceLayoutDesc& srlDesc, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_VK_IS_TYPE_VALID(objId, BvShaderResourceLayout)))
	{
		return false;
	}

	auto pObj = CreateResource<BvShaderResourceLayoutVk>(this, srlDesc);
	m_DeviceObjects.Emplace(pObj);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceVk::CreateShaderImpl(const ShaderCreateDesc& shaderDesc, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_VK_IS_TYPE_VALID(objId, BvShader)))
	{
		return false;
	}

	auto pObj = CreateResource<BvShaderVk>(this, shaderDesc);
	m_DeviceObjects.Emplace(pObj);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceVk::CreateGraphicsPipelineImpl(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_VK_IS_TYPE_VALID(objId, BvGraphicsPipelineState)))
	{
		return false;
	}

	auto pObj = CreateResource<BvGraphicsPipelineStateVk>(this, graphicsPipelineStateDesc, VK_NULL_HANDLE);
	m_DeviceObjects.Emplace(pObj);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceVk::CreateComputePipelineImpl(const ComputePipelineStateDesc& computePipelineStateDesc, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_VK_IS_TYPE_VALID(objId, BvComputePipelineState)))
	{
		return false;
	}

	auto pObj = CreateResource<BvComputePipelineStateVk>(this, computePipelineStateDesc, VK_NULL_HANDLE);
	m_DeviceObjects.Emplace(pObj);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceVk::CreateRayTracingPipelineImpl(const RayTracingPipelineStateDesc& rayTracingPipelineStateDesc, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_VK_IS_TYPE_VALID(objId, BvRayTracingPipelineState)))
	{
		return false;
	}

	auto pObj = CreateResource<BvRayTracingPipelineStateVk>(this, rayTracingPipelineStateDesc, VK_NULL_HANDLE);
	m_DeviceObjects.Emplace(pObj);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceVk::CreateQueryImpl(QueryType queryType, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_VK_IS_TYPE_VALID(objId, BvQuery)))
	{
		return false;
	}

	auto pObj = CreateResource<BvQueryVk>(this, queryType, 3);
	m_DeviceObjects.Emplace(pObj);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceVk::CreateFenceImpl(u64 value, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_VK_IS_TYPE_VALID(objId, BvGPUFence)))
	{
		return false;
	}

	auto pObj = CreateResource<BvGPUFenceVk>(this, value);
	m_DeviceObjects.Emplace(pObj);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceVk::CreateAccelerationStructureImpl(const RayTracingAccelerationStructureDesc& asDesc, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_VK_IS_TYPE_VALID(objId, BvAccelerationStructure)))
	{
		return false;
	}

	auto pObj = CreateResource<BvAccelerationStructureVk>(this, asDesc);
	m_DeviceObjects.Emplace(pObj);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceVk::CreateShaderBindingTableImpl(const ShaderBindingTableDesc& sbtDesc, IBvCommandContext* pContext, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_VK_IS_TYPE_VALID(objId, BvShaderBindingTable)))
	{
		return false;
	}

	auto pObj = CreateResource<BvShaderBindingTableVk>(this, sbtDesc, m_pDeviceInfo->m_ExtendedProperties.rayTracingPipelineProps, TO_VK(pContext));
	m_DeviceObjects.Emplace(pObj);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceVk::CreateCommandContextImpl(const CommandContextDesc& commandContextDesc, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_VK_IS_TYPE_VALID(objId, BvCommandContext)))
	{
		return false;
	}

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
		pContext = CreateResource<BvCommandContextVk>(this, 3, contextGroupIndex, commandContextGroup.Size());
		commandContextGroup.EmplaceBack(pContext);
	}

	*ppObj = pContext;

	return true;
}


void BvRenderDeviceVk::WaitIdle() const
{
	auto result = vkDeviceWaitIdle(m_Device);
}


void BvRenderDeviceVk::GetCopyableFootprints(const TextureDesc& textureDesc, u32 subresourceCount, SubresourceFootprint* pSubresources, u64* pTotalSize) const
{
	u64 totalSize = GetBufferSizeForTexture(textureDesc, m_pDeviceInfo->m_DeviceProperties.properties.limits.optimalBufferCopyOffsetAlignment,
		subresourceCount, pSubresources);

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


void BvRenderDeviceVk::Create(const BvRenderDeviceCreateDescVk& deviceCreateDesc)
{
	BV_ASSERT(deviceCreateDesc.m_ContextGroups.Size() > 0, "No device queues");

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
	BvFixedVector<VkDeviceQueueCreateInfo, kMaxContextGroupCount> queueInfos(deviceCreateDesc.m_ContextGroups.Size(), { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO });
	for (auto i = 0; i < queueInfos.Size(); ++i)
	{
		auto& queueCreateInfo = queueInfos[i];
		queueCreateInfo.pQueuePriorities = queuePriorities;

		auto familyIndex = deviceCreateDesc.m_ContextGroups[i].m_GroupIndex;
		auto& familyProps = m_pDeviceInfo->m_QueueFamilyProperties[familyIndex].queueFamilyProperties;
		BV_ASSERT_ONCE(deviceCreateDesc.m_ContextGroups[i].m_ContextCount <= familyProps.queueCount,
			"Using more context than what is available - truncating to maximum allowed");
		queueCreateInfo.queueFamilyIndex = familyIndex;
		queueCreateInfo.queueCount = std::min(deviceCreateDesc.m_ContextGroups[i].m_ContextCount, familyProps.queueCount);

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
}


void BvRenderDeviceVk::Destroy()
{
	if (m_Device)
	{
		vkDeviceWaitIdle(m_Device);

		for (auto& pObj : m_DeviceObjects)
		{
			pObj->Destroy();
		}
		m_DeviceObjects.Clear();

		DestroyVMA();
		
		vkDestroyDevice(m_Device, nullptr);
		m_Device = VK_NULL_HANDLE;
		
		m_pEngine->OnDeviceDestroyed(m_Index);
	}
}


void BvRenderDeviceVk::SelfDestroy()
{
	BV_DELETE_IN_PLACE(this);
}


void BvRenderDeviceVk::CreateVMA()
{
	VmaAllocatorCreateInfo vmaACI{};
	vmaACI.instance = m_pEngine->GetHandle();
	vmaACI.device = m_Device;
	vmaACI.physicalDevice = m_PhysicalDevice;
	vmaACI.vulkanApiVersion = VK_API_VERSION_1_3;

	if (m_pDeviceInfo->m_DeviceFeatures1_2.bufferDeviceAddress)
	{
		vmaACI.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	}
	if (m_pDeviceInfo->m_EnabledExtensions.Contains(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME))
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