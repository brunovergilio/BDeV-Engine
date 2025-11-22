#include "BvRenderDeviceGl.h"
#include "BvRenderEngineGl.h"
#include "BvCommandQueueGl.h"
#include "BvSwapChainGl.h"
#include "BvRenderPassGl.h"
#include "BvCommandPoolGl.h"
#include "BvShaderResourceGl.h"
#include "BvShaderGl.h"
#include "BvPipelineStateGl.h"
#include "BvBufferGl.h"
#include "BvBufferViewGl.h"
#include "BvTextureGl.h"
#include "BvTextureViewGl.h"
#include "BvSamplerGl.h"
#include "BvQueryGl.h"
#include "BvFramebufferGl.h"
#include "BvTypeConversionsGl.h"
#include "BvCommandContextGl.h"
#include "BvGPUFenceGl.h"
#include "BvCommandContextGl.h"
#include "BDeV/Core/RenderAPI/BvRenderAPIUtils.h"


BvRenderDeviceGl::BvRenderDeviceGl(BvRenderEngineGl* pEngine, const BvGPUInfo& gpuInfo, const BvRenderDeviceCreateDesc& deviceDesc)
	: m_pEngine(pEngine), m_GPUInfo(gpuInfo), m_MasterContext(nullptr)
{
	Create(deviceDesc);
}


BvRenderDeviceGl::~BvRenderDeviceGl()
{
	Destroy();
}


bool BvRenderDeviceGl::CreateSwapChainImpl(BvWindow* pWindow, const SwapChainDesc& desc, IBvCommandContext* pContext, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_GL_IS_TYPE_VALID(objId, BvSwapChain)))
	{
		return false;
	}

	auto pObj = CreateResource<BvSwapChainGl>(this, pWindow, desc, TO_GL(pContext));
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceGl::CreateBufferImpl(const BufferDesc& desc, const BufferInitData* pInitData, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_GL_IS_TYPE_VALID(objId, BvBuffer)))
	{
		return false;
	}

	auto pObj = CreateResource<BvBufferGl>(this, desc, pInitData);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceGl::CreateBufferViewImpl(const BufferViewDesc& desc, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_GL_IS_TYPE_VALID(objId, BvBufferView)))
	{
		return false;
	}

	auto pObj = CreateResource<BvBufferViewGl>(this, desc);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceGl::CreateTextureImpl(const TextureDesc& desc, const TextureInitData* pInitData, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_GL_IS_TYPE_VALID(objId, BvTexture)))
	{
		return false;
	}

	auto pObj = CreateResource<BvTextureGl>(this, desc, pInitData);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceGl::CreateTextureViewImpl(const TextureViewDesc& desc, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_GL_IS_TYPE_VALID(objId, BvTextureView)))
	{
		return false;
	}

	auto pObj = CreateResource<BvTextureViewGl>(this, desc);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceGl::CreateSamplerImpl(const SamplerDesc& desc, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_GL_IS_TYPE_VALID(objId, BvSampler)))
	{
		return false;
	}

	auto pObj = CreateResource<BvSamplerGl>(this, desc);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceGl::CreateRenderPassImpl(const RenderPassDesc& renderPassDesc, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_GL_IS_TYPE_VALID(objId, BvRenderPass)))
	{
		return false;
	}

	auto pObj = CreateResource<BvRenderPassGl>(this, renderPassDesc);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceGl::CreateShaderResourceLayoutImpl(const ShaderResourceLayoutDesc& srlDesc, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_GL_IS_TYPE_VALID(objId, BvShaderResourceLayout)))
	{
		return false;
	}

	auto pObj = CreateResource<BvShaderResourceLayoutGl>(this, srlDesc);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceGl::CreateShaderImpl(const ShaderCreateDesc& shaderDesc, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_GL_IS_TYPE_VALID(objId, BvShader)))
	{
		return false;
	}

	auto pObj = CreateResource<BvShaderGl>(this, shaderDesc);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceGl::CreateGraphicsPipelineImpl(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_GL_IS_TYPE_VALID(objId, BvGraphicsPipelineState)))
	{
		return false;
	}

	auto pObj = CreateResource<BvGraphicsPipelineStateGl>(this, graphicsPipelineStateDesc, Gl_NULL_HANDLE);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceGl::CreateComputePipelineImpl(const ComputePipelineStateDesc& computePipelineStateDesc, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_GL_IS_TYPE_VALID(objId, BvComputePipelineState)))
	{
		return false;
	}

	auto pObj = CreateResource<BvComputePipelineStateGl>(this, computePipelineStateDesc, Gl_NULL_HANDLE);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceGl::CreateRayTracingPipelineImpl(const RayTracingPipelineStateDesc& rayTracingPipelineStateDesc, const BvUUID& objId, void** ppObj)
{
	return false;
}


bool BvRenderDeviceGl::CreateQueryImpl(QueryType queryType, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_GL_IS_TYPE_VALID(objId, BvQuery)))
	{
		return false;
	}

	auto pObj = CreateResource<BvQueryGl>(this, queryType, 3);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceGl::CreateFenceImpl(u64 value, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_GL_IS_TYPE_VALID(objId, BvGPUFence)))
	{
		return false;
	}

	auto pObj = CreateResource<BvGPUFenceGl>(this, value);
	*ppObj = pObj;

	return true;
}


bool BvRenderDeviceGl::CreateAccelerationStructureImpl(const RayTracingAccelerationStructureDesc& asDesc, const BvUUID& objId, void** ppObj)
{
	return false;
}


bool BvRenderDeviceGl::CreateShaderBindingTableImpl(const ShaderBindingTableDesc& sbtDesc, IBvCommandContext* pContext, const BvUUID& objId, void** ppObj)
{
	return false;
}


bool BvRenderDeviceGl::CreateCommandContextImpl(const CommandContextDesc& commandContextDesc, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_GL_IS_TYPE_VALID(objId, BvCommandContext)))
	{
		return false;
	}

	if (commandContextDesc.m_RequireDedicated && commandContextDesc.m_CommandType != CommandType::kGraphics)
	{
		return false;
	}

	auto pContext = CreateResource<BvCommandContextGl>(this, 3, contextGroupIndex, commandContextGroup.Size());

	*ppObj = pContext;

	return true;
}


void BvRenderDeviceGl::WaitIdle() const
{
}


void BvRenderDeviceGl::GetCopyableFootprints(const TextureDesc& textureDesc, u32 subresourceCount, SubresourceFootprint* pSubresources, u64* pTotalSize) const
{
	u64 totalSize = GetBufferSizeForTexture(textureDesc, m_pDeviceInfo->m_DeviceProperties.properties.limits.optimalBufferCopyOffsetAlignment,
		subresourceCount, pSubresources);

	if (pTotalSize)
	{
		*pTotalSize = totalSize;
	}
}


u64 BvRenderDeviceGl::GetDynamicBufferElementSize(BufferUsage usageFlags, u64 elementStride) const
{
	// TODO: Figure this out
	return 64;
}


FormatFeatures BvRenderDeviceGl::GetFormatFeatures(Format format) const
{
	FormatFeatures formatFeatures = FormatFeatures::kNone;
	// TODO: glGetInternalformat

	return formatFeatures;
}


void BvRenderDeviceGl::Create(const BvRenderDeviceCreateDesc& deviceCreateDesc)
{
	BV_ASSERT(deviceCreateDesc.m_ContextGroups.Size() == 1 && deviceCreateDesc.m_ContextGroups[0].m_GroupIndex == 0,
		"OpenGL implementation has only 1 context group");
	m_CommandContexts.Resize(deviceCreateDesc.m_ContextGroups[0].m_ContextCount);

	m_SupportedDisplayFormats.EmplaceBack(Format::kRGBA8_UNorm);
	m_SupportedDisplayFormats.EmplaceBack(Format::kBGRA8_UNorm);

	if (m_MasterContext.SupportsSRGB())
	{
		m_SupportedDisplayFormats.EmplaceBack(Format::kRGBA8_UNorm_SRGB);
		m_SupportedDisplayFormats.EmplaceBack(Format::kBGRA8_UNorm_SRGB);
	}
}


void BvRenderDeviceGl::Destroy()
{
	if (m_Device)
	{
		GlDeviceWaitIdle(m_Device);

		for (auto& pObj : m_DeviceObjects)
		{
			pObj->Destroy();
		}
		m_DeviceObjects.Clear();

		DestroyVMA();

		GlDestroyDevice(m_Device, nullptr);
		m_Device = Gl_NULL_HANDLE;

		m_pEngine->OnDeviceDestroyed(m_Index);
	}
}


void BvRenderDeviceGl::SelfDestroy()
{
	BV_DELETE_IN_PLACE(this);
}