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
#include "BvGPUFenceVk.h"
#include "BvAccelerationStructureVk.h"
#include "BvShaderBindingTableVk.h"
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


bool BvRenderDeviceVk::CreateSwapChain(BvWindow* pWindow, const SwapChainDesc& swapChainDesc, IBvCommandContext* pContext, IBvSwapChain** ppObj)
{
	IBvSwapChainVk* pObjVk;
	if (CreateSwapChainVk(pWindow, swapChainDesc, pContext, &pObjVk))
	{
		*ppObj = pObjVk;
		return true;
	}

	return false;
}


bool BvRenderDeviceVk::CreateBuffer(const BufferDesc& desc, const BufferInitData* pInitData, IBvBuffer** ppObj)
{
	IBvBufferVk* pObjVk;
	if (CreateBufferVk(desc, pInitData, &pObjVk))
	{
		*ppObj = pObjVk;
		return true;
	}

	return false;
}


bool BvRenderDeviceVk::CreateBufferView(const BufferViewDesc& desc, IBvBufferView** ppObj)
{
	IBvBufferViewVk* pObjVk;
	if (CreateBufferViewVk(desc, &pObjVk))
	{
		*ppObj = pObjVk;
		return true;
	}

	return false;
}


bool BvRenderDeviceVk::CreateTexture(const TextureDesc& desc, const TextureInitData* pInitData, IBvTexture** ppObj)
{
	IBvTextureVk* pObjVk;
	if (CreateTextureVk(desc, pInitData, &pObjVk))
	{
		*ppObj = pObjVk;
		return true;
	}

	return false;
}


bool BvRenderDeviceVk::CreateTextureView(const TextureViewDesc& desc, IBvTextureView** ppObj)
{
	IBvTextureViewVk* pObjVk;
	if (CreateTextureViewVk(desc, &pObjVk))
	{
		*ppObj = pObjVk;
		return true;
	}

	return false;
}


bool BvRenderDeviceVk::CreateSampler(const SamplerDesc& desc, IBvSampler** ppObj)
{
	IBvSamplerVk* pObjVk;
	if (CreateSamplerVk(desc, &pObjVk))
	{
		*ppObj = pObjVk;
		return true;
	}

	return false;
}


bool BvRenderDeviceVk::CreateRenderPass(const RenderPassDesc& renderPassDesc, IBvRenderPass** ppObj)
{
	IBvRenderPassVk* pObjVk;
	if (CreateRenderPassVk(renderPassDesc, &pObjVk))
	{
		*ppObj = pObjVk;
		return true;
	}

	return false;
}


bool BvRenderDeviceVk::CreateShaderResourceLayout(const ShaderResourceLayoutDesc& srlDesc, IBvShaderResourceLayout** ppObj)
{
	IBvShaderResourceLayoutVk* pObjVk;
	if (CreateShaderResourceLayoutVk(srlDesc, &pObjVk))
	{
		*ppObj = pObjVk;
		return true;
	}

	return false;
}


bool BvRenderDeviceVk::CreateShader(const ShaderCreateDesc& shaderDesc, IBvShader** ppObj)
{
	IBvShaderVk* pObjVk;
	if (CreateShaderVk(shaderDesc, &pObjVk))
	{
		*ppObj = pObjVk;
		return true;
	}

	return false;
}


bool BvRenderDeviceVk::CreateGraphicsPipeline(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc, IBvGraphicsPipelineState** ppObj)
{
	IBvGraphicsPipelineStateVk* pObjVk;
	if (CreateGraphicsPipelineVk(graphicsPipelineStateDesc, &pObjVk))
	{
		*ppObj = pObjVk;
		return true;
	}

	return false;
}


bool BvRenderDeviceVk::CreateComputePipeline(const ComputePipelineStateDesc& computePipelineStateDesc, IBvComputePipelineState** ppObj)
{
	IBvComputePipelineStateVk* pObjVk;
	if (CreateComputePipelineVk(computePipelineStateDesc, &pObjVk))
	{
		*ppObj = pObjVk;
		return true;
	}

	return false;
}


bool BvRenderDeviceVk::CreateRayTracingPipeline(const RayTracingPipelineStateDesc& rayTracingPipelineStateDesc, IBvRayTracingPipelineState** ppObj)
{
	IBvRayTracingPipelineStateVk* pObjVk;
	if (CreateRayTracingPipelineVk(rayTracingPipelineStateDesc, &pObjVk))
	{
		*ppObj = pObjVk;
		return true;
	}

	return false;
}


bool BvRenderDeviceVk::CreateQuery(QueryType queryType, IBvQuery** ppObj)
{
	IBvQueryVk* pObjVk;
	if (CreateQueryVk(queryType, &pObjVk))
	{
		*ppObj = pObjVk;
		return true;
	}

	return false;
}


bool BvRenderDeviceVk::CreateFence(u64 value, IBvGPUFence** ppObj)
{
	IBvGPUFenceVk* pObjVk;
	if (CreateFenceVk(value, &pObjVk))
	{
		*ppObj = pObjVk;
		return true;
	}

	return false;
}


bool BvRenderDeviceVk::CreateAccelerationStructure(const RayTracingAccelerationStructureDesc& asDesc, IBvAccelerationStructure** ppObj)
{
	IBvAccelerationStructureVk* pObjVk;
	if (CreateAccelerationStructureVk(asDesc, &pObjVk))
	{
		*ppObj = pObjVk;
		return true;
	}

	return false;
}


bool BvRenderDeviceVk::CreateShaderBindingTable(const ShaderBindingTableDesc& sbtDesc, IBvCommandContext* pContext, IBvShaderBindingTable** ppObj)
{
	IBvShaderBindingTableVk* pObjVk;
	if (CreateShaderBindingTableVk(sbtDesc, pContext, &pObjVk))
	{
		*ppObj = pObjVk;
		return true;
	}

	return false;
}


bool BvRenderDeviceVk::CreateSwapChainVk(BvWindow* pWindow, const SwapChainDesc& swapChainDesc, IBvCommandContext* pContext, IBvSwapChainVk** ppObj)
{
	*ppObj = BV_OBJECT_CREATE(BvSwapChainVk, this, pWindow, swapChainDesc, pContext);
	if (!(*ppObj)->IsValid())
	{
		(*ppObj)->Release();
		return false;
	}

	m_DeviceObjects.PushBack(*ppObj);
	return true;
}


bool BvRenderDeviceVk::CreateBufferVk(const BufferDesc& desc, const BufferInitData* pInitData, IBvBufferVk** ppObj)
{
	*ppObj = BV_OBJECT_CREATE(BvBufferVk, this, desc, pInitData);
	if (!(*ppObj)->IsValid())
	{
		(*ppObj)->Release();
		return false;
	}

	m_DeviceObjects.PushBack(*ppObj);
	return true;
}


bool BvRenderDeviceVk::CreateBufferViewVk(const BufferViewDesc& desc, IBvBufferViewVk** ppObj)
{
	*ppObj = BV_OBJECT_CREATE(BvBufferViewVk, this, desc);
	if (!(*ppObj)->IsValid())
	{
		(*ppObj)->Release();
		return false;
	}

	m_DeviceObjects.PushBack(*ppObj);
	return true;
}


bool BvRenderDeviceVk::CreateTextureVk(const TextureDesc& desc, const TextureInitData* pInitData, IBvTextureVk** ppObj)
{
	*ppObj = BV_OBJECT_CREATE(BvTextureVk, this, desc, pInitData);
	if (!(*ppObj)->IsValid())
	{
		(*ppObj)->Release();
		return false;
	}

	m_DeviceObjects.PushBack(*ppObj);
	return true;
}


bool BvRenderDeviceVk::CreateTextureViewVk(const TextureViewDesc& desc, IBvTextureViewVk** ppObj)
{
	if (!(*ppObj)->IsValid())
	{
		(*ppObj)->Release();
		return false;
	}

	m_DeviceObjects.PushBack(*ppObj);
	return true;
}


bool BvRenderDeviceVk::CreateSamplerVk(const SamplerDesc& desc, IBvSamplerVk** ppObj)
{
	*ppObj = BV_OBJECT_CREATE(BvSamplerVk, this, desc);
	if (!(*ppObj)->IsValid())
	{
		(*ppObj)->Release();
		return false;
	}

	m_DeviceObjects.PushBack(*ppObj);
	return true;
}


bool BvRenderDeviceVk::CreateRenderPassVk(const RenderPassDesc& renderPassDesc, IBvRenderPassVk** ppObj)
{
	*ppObj = BV_OBJECT_CREATE(BvRenderPassVk, this, renderPassDesc);
	if (!(*ppObj)->IsValid())
	{
		(*ppObj)->Release();
		return false;
	}

	m_DeviceObjects.PushBack(*ppObj);
	return true;
}


bool BvRenderDeviceVk::CreateShaderResourceLayoutVk(const ShaderResourceLayoutDesc& srlDesc, IBvShaderResourceLayoutVk** ppObj)
{
	*ppObj = BV_OBJECT_CREATE(BvShaderResourceLayoutVk, this, srlDesc);
	if (!(*ppObj)->IsValid())
	{
		(*ppObj)->Release();
		return false;
	}

	m_DeviceObjects.PushBack(*ppObj);
	return true;
}


bool BvRenderDeviceVk::CreateShaderVk(const ShaderCreateDesc& shaderDesc, IBvShaderVk** ppObj)
{
	*ppObj = BV_OBJECT_CREATE(BvShaderVk, this, shaderDesc);
	m_DeviceObjects.PushBack(*ppObj);

	return true;
}


bool BvRenderDeviceVk::CreateGraphicsPipelineVk(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc, IBvGraphicsPipelineStateVk** ppObj)
{
	*ppObj = BV_OBJECT_CREATE(BvGraphicsPipelineStateVk, this, graphicsPipelineStateDesc, VK_NULL_HANDLE);
	if (!(*ppObj)->IsValid())
	{
		(*ppObj)->Release();
		return false;
	}

	m_DeviceObjects.PushBack(*ppObj);
	return true;
}


bool BvRenderDeviceVk::CreateComputePipelineVk(const ComputePipelineStateDesc& computePipelineStateDesc, IBvComputePipelineStateVk** ppObj)
{
	*ppObj = BV_OBJECT_CREATE(BvComputePipelineStateVk, this, computePipelineStateDesc, VK_NULL_HANDLE);
	if (!(*ppObj)->IsValid())
	{
		(*ppObj)->Release();
		return false;
	}

	m_DeviceObjects.PushBack(*ppObj);
	return true;
}


bool BvRenderDeviceVk::CreateRayTracingPipelineVk(const RayTracingPipelineStateDesc& rayTracingPipelineStateDesc, IBvRayTracingPipelineStateVk** ppObj)
{
	*ppObj = BV_OBJECT_CREATE(BvRayTracingPipelineStateVk, this, rayTracingPipelineStateDesc, VK_NULL_HANDLE);
	if (!(*ppObj)->IsValid())
	{
		(*ppObj)->Release();
		return false;
	}

	m_DeviceObjects.PushBack(*ppObj);
	return true;
}


bool BvRenderDeviceVk::CreateQueryVk(QueryType queryType, IBvQueryVk** ppObj)
{
	*ppObj = BV_OBJECT_CREATE(BvQueryVk, this, queryType, 3);
	m_DeviceObjects.PushBack(*ppObj);

	return true;
}


bool BvRenderDeviceVk::CreateFenceVk(u64 value, IBvGPUFenceVk** ppObj)
{
	*ppObj = BV_OBJECT_CREATE(BvGPUFenceVk, this, value);
	if (!(*ppObj)->IsValid())
	{
		(*ppObj)->Release();
		return false;
	}

	m_DeviceObjects.PushBack(*ppObj);
	return true;
}


bool BvRenderDeviceVk::CreateAccelerationStructureVk(const RayTracingAccelerationStructureDesc& asDesc, IBvAccelerationStructureVk** ppObj)
{
	*ppObj = BV_OBJECT_CREATE(BvAccelerationStructureVk, this, asDesc);
	if (!(*ppObj)->IsValid())
	{
		(*ppObj)->Release();
		return false;
	}

	m_DeviceObjects.PushBack(*ppObj);
	return true;
}


bool BvRenderDeviceVk::CreateShaderBindingTableVk(const ShaderBindingTableDesc& sbtDesc, IBvCommandContext* pContext, IBvShaderBindingTableVk** ppObj)
{
	*ppObj = BV_OBJECT_CREATE(BvShaderBindingTableVk, this, sbtDesc, m_pDeviceInfo->m_ExtendedProperties.rayTracingPipelineProps, TO_VK(pContext));
	m_DeviceObjects.PushBack(*ppObj);

	return true;
}


void BvRenderDeviceVk::WaitIdle() const
{
	auto result = vkDeviceWaitIdle(m_Device);
}


bool BvRenderDeviceVk::CreateGraphicsContext(u32 index, IBvCommandContext** ppObj)
{
	IBvCommandContextVk* pObjVk;
	if (CreateGraphicsContextVk(index, &pObjVk))
	{
		*ppObj = pObjVk;
		return true;
	}

	return false;
}


bool BvRenderDeviceVk::CreateComputeContext(u32 index, IBvCommandContext** ppObj)
{
	IBvCommandContextVk* pObjVk;
	if (CreateComputeContextVk(index, &pObjVk))
	{
		*ppObj = pObjVk;
		return true;
	}

	return false;
}


bool BvRenderDeviceVk::CreateTransferContext(u32 index, IBvCommandContext** ppObj)
{
	IBvCommandContextVk* pObjVk;
	if (CreateTransferContextVk(index, &pObjVk))
	{
		*ppObj = pObjVk;
		return true;
	}

	return false;
}


bool BvRenderDeviceVk::CreateGraphicsContextVk(u32 index, IBvCommandContextVk** ppObj)
{
	if (index >= m_GraphicsContexts.Size())
	{
		return false;
	}

	BV_ASSERT_ONCE(m_GraphicsContexts[index] == nullptr, "Context already created");

	auto& pContext = m_GraphicsContexts[index];
	if (!pContext)
	{
		pContext = BV_OBJECT_CREATE(BvCommandContextVk, this, 3, CommandType::kGraphics, m_pDeviceInfo->m_GraphicsQueueInfo.m_QueueFamilyIndex, index);
	}
	*ppObj = pContext;

	return true;
}


bool BvRenderDeviceVk::CreateComputeContextVk(u32 index, IBvCommandContextVk** ppObj)
{
	if (index >= m_ComputeContexts.Size())
	{
		return false;
	}

	BV_ASSERT_ONCE(m_ComputeContexts[index] == nullptr, "Context already created");

	auto& pContext = m_ComputeContexts[index];
	if (!pContext)
	{
		pContext = BV_OBJECT_CREATE(BvCommandContextVk, this, 3, CommandType::kCompute, m_pDeviceInfo->m_ComputeQueueInfo.m_QueueFamilyIndex, index);
	}
	*ppObj = pContext;

	return true;
}


bool BvRenderDeviceVk::CreateTransferContextVk(u32 index, IBvCommandContextVk** ppObj)
{
	if (index >= m_TransferContexts.Size())
	{
		return false;
	}

	BV_ASSERT_ONCE(m_TransferContexts[index] == nullptr, "Context already created");

	auto& pContext = m_TransferContexts[index];
	if (!pContext)
	{
		pContext = BV_OBJECT_CREATE(BvCommandContextVk, this, 3, CommandType::kTransfer, m_pDeviceInfo->m_TransferQueueInfo.m_QueueFamilyIndex, index);
	}
	*ppObj = pContext;

	return true;
}


IBvCommandContext* BvRenderDeviceVk::GetGraphicsContext(u32 index) const
{
	return GetGraphicsContextVk(index);
}


IBvCommandContext* BvRenderDeviceVk::GetComputeContext(u32 index) const
{
	return GetComputeContextVk(index);
}


IBvCommandContext* BvRenderDeviceVk::GetTransferContext(u32 index) const
{
	return GetTransferContextVk(index);
}


IBvCommandContextVk* BvRenderDeviceVk::GetGraphicsContextVk(u32 index) const
{
	return index < m_GraphicsContexts.Size() ? m_GraphicsContexts[index] : nullptr;
}


IBvCommandContextVk* BvRenderDeviceVk::GetComputeContextVk(u32 index) const
{
	return index < m_ComputeContexts.Size() ? m_ComputeContexts[index] : nullptr;
}


IBvCommandContextVk* BvRenderDeviceVk::GetTransferContextVk(u32 index) const
{
	return index < m_TransferContexts.Size() ? m_TransferContexts[index] : nullptr;
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
	if (bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT)
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

		m_GraphicsContexts.Resize(queueCreateInfo.queueCount);
	}
	if (deviceCreateDesc.m_ComputeQueueCount > 0 && m_pDeviceInfo->m_ComputeQueueInfo.m_QueueCount > 0)
	{
		queueCreateInfo.queueFamilyIndex = m_pDeviceInfo->m_ComputeQueueInfo.m_QueueFamilyIndex;
		queueCreateInfo.queueCount = std::min(deviceCreateDesc.m_ComputeQueueCount, m_pDeviceInfo->m_ComputeQueueInfo.m_QueueCount);
		queueInfos.PushBack(queueCreateInfo);

		m_ComputeContexts.Resize(queueCreateInfo.queueCount);
	}
	if (deviceCreateDesc.m_TransferQueueCount > 0 && m_pDeviceInfo->m_TransferQueueInfo.m_QueueCount > 0)
	{
		queueCreateInfo.queueFamilyIndex = m_pDeviceInfo->m_TransferQueueInfo.m_QueueFamilyIndex;
		queueCreateInfo.queueCount = std::min(deviceCreateDesc.m_TransferQueueCount, m_pDeviceInfo->m_TransferQueueInfo.m_QueueCount);
		queueInfos.PushBack(queueCreateInfo);

		m_TransferContexts.Resize(queueCreateInfo.queueCount);
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

	CreateVMA();

	u32 querySizes[kQueryTypeCount]{ 16, 16, 16 };

	m_DeviceCaps = deviceCaps;
}


void BvRenderDeviceVk::Destroy()
{
	vkDeviceWaitIdle(m_Device);

	for (i32 i = i32(m_DeviceObjects.Size()) - 1; i >= 0; --i)
	{
		m_DeviceObjects[i]->Release();
	}

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
	vmaACI.vulkanApiVersion = VK_API_VERSION_1_3;

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
	if (queueFlags & VK_QUEUE_COMPUTE_BIT)
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
	else if (queueFlags & VK_QUEUE_TRANSFER_BIT)
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