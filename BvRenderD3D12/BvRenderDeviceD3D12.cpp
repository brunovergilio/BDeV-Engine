#include "BvRenderDeviceD3D12.h"
#include "BvTypeConversionsD3D12.h"


BvRenderDeviceD3D12::BvRenderDeviceD3D12(BvRenderEngineD3D12* pEngine, IDXGIAdapter1* pAdapter, u32 index, const BvGPUInfo& gpuInfo, const BvRenderDeviceCreateDescD3D12& deviceDesc)
	: m_pEngine(pEngine), m_GPUInfo(gpuInfo), m_Index(index), m_pAdapter(pAdapter)
{
	Create(deviceDesc);
}


BvRenderDeviceD3D12::~BvRenderDeviceD3D12()
{
	Destroy();
}


IBvSwapChain* BvRenderDeviceD3D12::CreateSwapChainImpl(BvWindow* pWindow, const SwapChainDesc& swapChainDesc, IBvCommandContext* pContext)
{
	return nullptr;
}


IBvBuffer* BvRenderDeviceD3D12::CreateBufferImpl(const BufferDesc& desc, const BufferInitData* pInitData)
{
	return nullptr;
}


IBvBufferView* BvRenderDeviceD3D12::CreateBufferViewImpl(const BufferViewDesc& desc)
{
	return nullptr;
}


IBvTexture* BvRenderDeviceD3D12::CreateTextureImpl(const TextureDesc& desc, const TextureInitData* pInitData)
{
	return nullptr;
}


IBvTextureView* BvRenderDeviceD3D12::CreateTextureViewImpl(const TextureViewDesc& desc)
{
	return nullptr;
}


IBvSampler* BvRenderDeviceD3D12::CreateSamplerImpl(const SamplerDesc& desc)
{
	return nullptr;
}


IBvRenderPass* BvRenderDeviceD3D12::CreateRenderPassImpl(const RenderPassDesc& renderPassDesc)
{
	return nullptr;
}


IBvShaderResourceLayout* BvRenderDeviceD3D12::CreateShaderResourceLayoutImpl(const ShaderResourceLayoutDesc& srlDesc)
{
	return nullptr;
}


IBvShader* BvRenderDeviceD3D12::CreateShaderImpl(const ShaderCreateDesc& shaderDesc)
{
	return nullptr;
}


IBvGraphicsPipelineState* BvRenderDeviceD3D12::CreateGraphicsPipelineImpl(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc)
{
	return nullptr;
}


IBvComputePipelineState* BvRenderDeviceD3D12::CreateComputePipelineImpl(const ComputePipelineStateDesc& computePipelineStateDesc)
{
	return nullptr;
}


IBvRayTracingPipelineState* BvRenderDeviceD3D12::CreateRayTracingPipelineImpl(const RayTracingPipelineStateDesc& rayTracingPipelineStateDesc)
{
	return nullptr;
}


IBvQuery* BvRenderDeviceD3D12::CreateQueryImpl(QueryType queryType)
{
	return nullptr;
}


IBvGPUFence* BvRenderDeviceD3D12::CreateFenceImpl(u64 value)
{
	return nullptr;
}


IBvAccelerationStructure* BvRenderDeviceD3D12::CreateAccelerationStructureImpl(const RayTracingAccelerationStructureDesc& asDesc)
{
	return nullptr;
}


IBvShaderBindingTable* BvRenderDeviceD3D12::CreateShaderBindingTableImpl(const ShaderBindingTableDesc& sbtDesc, IBvCommandContext* pContext)
{
	return nullptr;
}


IBvCommandContext* BvRenderDeviceD3D12::GetGraphicsContextImpl(u32 index /*= 0*/)
{
	return nullptr;
}


IBvCommandContext* BvRenderDeviceD3D12::GetComputeContextImpl(u32 index /*= 0*/)
{
	return nullptr;
}


IBvCommandContext* BvRenderDeviceD3D12::GetTransferContextImpl(u32 index /*= 0*/)
{
	return nullptr;
}


void BvRenderDeviceD3D12::WaitIdle() const
{

}


void BvRenderDeviceD3D12::GetCopyableFootprints(const TextureDesc& textureDesc, u32 subresourceCount, SubresourceFootprint* pSubresources, u64* pTotalSize) const
{
	u64 totalSize = 0;
	BvVector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> footprints(subresourceCount);
	auto resourceDesc = GetD3D12ResourceDesc(textureDesc);
	m_Device->GetCopyableFootprints(&resourceDesc, 0, subresourceCount, 0, footprints.Data(), nullptr, nullptr, &totalSize);
	if (pTotalSize)
	{
		*pTotalSize = totalSize;
	}
}


u64 BvRenderDeviceD3D12::GetDynamicBufferElementSize(BufferUsage usageFlags, u64 elementStride) const
{
	u64 elementSize = elementStride;
	if (EHasFlag(usageFlags, BufferUsage::kConstantBuffer))
	{
		elementSize = RoundToNearestPowerOf2(elementSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	}

	return elementSize;
}


bool BvRenderDeviceD3D12::SupportsQueryType(QueryType queryType, CommandType commandType) const
{
	if (queryType == QueryType::kTimestamp)
	{
		if (commandType == CommandType::kGraphics || commandType == CommandType::kCompute)
		{
			return true;
		}
		else
		{
			return m_pDeviceInfo->m_Options3.CopyQueueTimestampQueriesSupported;
		}
	}
	else if (queryType == QueryType::kMeshPipelineStatistics)
	{
		return m_pDeviceInfo->m_Options9.MeshShaderPipelineStatsSupported;
	}

	return commandType == CommandType::kGraphics;
}


FormatFeatures BvRenderDeviceD3D12::GetFormatFeatures(Format format) const
{
	auto formatFeatures = FormatFeatures::kNone;
	D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport{ DXGI_FORMAT(format) };
	auto hr = m_Device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport));
	if (SUCCEEDED(hr))
	{
		if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE1D)
		{
			formatFeatures |= FormatFeatures::kTexture1D;
		}
		if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE2D)
		{
			formatFeatures |= FormatFeatures::kTexture2D;
		}
		if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE3D)
		{
			formatFeatures |= FormatFeatures::kTexture3D;
		}
		if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURECUBE)
		{
			formatFeatures |= FormatFeatures::kTextureCube;
		}
		if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_BUFFER)
		{
			formatFeatures |= FormatFeatures::kBuffer;
		}
		if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW)
		{
			formatFeatures |= FormatFeatures::kRWBuffer;
			formatFeatures |= FormatFeatures::kRWTexture;
		}
		if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_IA_VERTEX_BUFFER)
		{
			formatFeatures |= FormatFeatures::kVertexBuffer;
		}
		if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET)
		{
			formatFeatures |= FormatFeatures::kRenderTarget;
		}
		if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_BLENDABLE)
		{
			formatFeatures |= FormatFeatures::kBlendable;
		}
		if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL)
		{
			formatFeatures |= FormatFeatures::kDepthStencil;
		}
		if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE)
		{
			formatFeatures |= FormatFeatures::kSampling;
		}
		if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE_COMPARISON)
		{
			formatFeatures |= FormatFeatures::kComparisonSampling;
		}
		if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_MULTISAMPLE_RESOLVE)
		{
			formatFeatures |= FormatFeatures::kResolve;
		}
		if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_MULTISAMPLE_RENDERTARGET)
		{
			formatFeatures |= FormatFeatures::kMultisampleRenderTarget;
		}
	}

	return formatFeatures;
}


void BvRenderDeviceD3D12::Create(const BvRenderDeviceCreateDescD3D12& deviceCreateDesc)
{
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	for (auto i = 0; i < ArraySize(levels); ++i)
	{
		if (SUCCEEDED(D3D12CreateDevice(m_pAdapter, levels[i], IID_PPV_ARGS(&m_Device))))
		{
			break;
		}
	}

	if (!m_Device)
	{
		return;
	}

	constexpr u32 kMaxContextsPerType = 16;
	m_GraphicsContexts.Resize(std::min(deviceCreateDesc.m_GraphicsQueueCount, kMaxContextsPerType));
	m_ComputeContexts.Resize(std::min(deviceCreateDesc.m_ComputeQueueCount, kMaxContextsPerType));
	m_TransferContexts.Resize(std::min(deviceCreateDesc.m_TransferQueueCount, kMaxContextsPerType));

	SetupDeviceInfo();
	SetupSupportedDisplayFormats();
	CreateCommandSignatures();
}


void BvRenderDeviceD3D12::Destroy()
{
	if (m_Device)
	{
		BV_DELETE(m_pDeviceInfo);

		m_Device = nullptr;
	}
}


void BvRenderDeviceD3D12::SelfDestroy()
{

}


void BvRenderDeviceD3D12::CreateAllocator()
{
	D3D12MA::ALLOCATOR_DESC desc{};
	desc.pDevice = m_Device.Get();
	desc.pAdapter = m_pAdapter;
	D3D12MA::CreateAllocator(&desc, &m_Allocator);
}


void BvRenderDeviceD3D12::DestroyAllocator()
{
	m_Allocator = nullptr;
}


void BvRenderDeviceD3D12::SetupSupportedDisplayFormats()
{
	D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport{};
	HRESULT hr = S_OK;

#define TEST_DXGI_FORMAT(format)																				\
	formatSupport.Format = format;																				\
	hr = m_Device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport));	\
	if (SUCCEEDED(hr) && (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_DISPLAY))								\
	{																											\
		m_SupportedDisplayFormats.EmplaceBack(Format(format));													\
	}

	for (auto i = 1; i <= i32(DXGI_FORMAT_B4G4R4A4_UNORM); ++i)
	{
		TEST_DXGI_FORMAT(DXGI_FORMAT(i));
	}

	TEST_DXGI_FORMAT(DXGI_FORMAT_P208);
	TEST_DXGI_FORMAT(DXGI_FORMAT_V208);
	TEST_DXGI_FORMAT(DXGI_FORMAT_V408);

#undef TEST_DXGI_FORMAT
}


void BvRenderDeviceD3D12::SetupDeviceInfo()
{
	m_pDeviceInfo = BV_NEW(BvDeviceInfoD3D12)();

#define GET_FEATURE_OPTIONS(option) m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS##option, &m_pDeviceInfo->m_Options##option, sizeof(m_pDeviceInfo->m_Options##option))
	GET_FEATURE_OPTIONS();
	GET_FEATURE_OPTIONS(1);
	GET_FEATURE_OPTIONS(2);
	GET_FEATURE_OPTIONS(3);
	GET_FEATURE_OPTIONS(4);
	GET_FEATURE_OPTIONS(5);
	GET_FEATURE_OPTIONS(6);
	GET_FEATURE_OPTIONS(7);
	GET_FEATURE_OPTIONS(8);
	GET_FEATURE_OPTIONS(9);
	GET_FEATURE_OPTIONS(10);
	GET_FEATURE_OPTIONS(11);
	GET_FEATURE_OPTIONS(12);
	GET_FEATURE_OPTIONS(13);
#undef GET_FEATURE_OPTIONS

	m_DeviceCaps |= RenderDeviceCapabilities::kWireframe | RenderDeviceCapabilities::kGeometryShader | RenderDeviceCapabilities::kTesselationShader
		| RenderDeviceCapabilities::kTimestampQueries | RenderDeviceCapabilities::kIndirectDrawCount
		| RenderDeviceCapabilities::kCustomBorderColor | RenderDeviceCapabilities::kPredication;
	if (m_pDeviceInfo->m_Options2.DepthBoundsTestSupported)
	{
		m_DeviceCaps |= RenderDeviceCapabilities::kDepthBoundsTest;
	}
	if (m_pDeviceInfo->m_Options.TiledResourcesTier >= D3D12_TILED_RESOURCES_TIER_2)
	{
		m_DeviceCaps |= RenderDeviceCapabilities::kSamplerMinMaxReduction;
	}
	if (m_pDeviceInfo->m_Options.ConservativeRasterizationTier != D3D12_CONSERVATIVE_RASTERIZATION_TIER_NOT_SUPPORTED)
	{
		m_DeviceCaps |= RenderDeviceCapabilities::kConservativeRasterization;
	}
	if (m_pDeviceInfo->m_Options6.VariableShadingRateTier != D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED)
	{
		m_DeviceCaps |= RenderDeviceCapabilities::kShadingRate;
	}
	if (m_pDeviceInfo->m_Options7.MeshShaderTier != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED)
	{
		m_DeviceCaps |= RenderDeviceCapabilities::kMeshShader;
	}
	if (m_pDeviceInfo->m_Options9.MeshShaderPipelineStatsSupported)
	{
		m_DeviceCaps |= RenderDeviceCapabilities::kMeshQuery;
	}
	if (m_pDeviceInfo->m_Options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
	{
		m_DeviceCaps |= RenderDeviceCapabilities::kRayTracing | RenderDeviceCapabilities::kRayQuery;
	}
}


void BvRenderDeviceD3D12::CreateCommandSignatures()
{
	D3D12_INDIRECT_ARGUMENT_DESC argDesc{};
	D3D12_COMMAND_SIGNATURE_DESC desc{};
	desc.NumArgumentDescs = 1;
	desc.pArgumentDescs = &argDesc;

	desc.ByteStride = sizeof(D3D12_DRAW_ARGUMENTS);
	argDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
	m_Device->CreateCommandSignature(&desc, nullptr, IID_PPV_ARGS(&m_DrawIndirectSig));

	desc.ByteStride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
	argDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
	m_Device->CreateCommandSignature(&desc, nullptr, IID_PPV_ARGS(&m_DrawIndexedIndirectSig));

	desc.ByteStride = sizeof(D3D12_DISPATCH_ARGUMENTS);
	argDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
	m_Device->CreateCommandSignature(&desc, nullptr, IID_PPV_ARGS(&m_DispatchIndirectSig));

	if (EHasFlag(m_DeviceCaps, RenderDeviceCapabilities::kMeshShader))
	{
		desc.ByteStride = sizeof(D3D12_DISPATCH_MESH_ARGUMENTS);
		argDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH;
		m_Device->CreateCommandSignature(&desc, nullptr, IID_PPV_ARGS(&m_DispatchMeshIndirectSig));
	}

	if (EHasFlag(m_DeviceCaps, RenderDeviceCapabilities::kRayTracing))
	{
		desc.ByteStride = sizeof(D3D12_DISPATCH_RAYS_DESC);
		argDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_RAYS;
		m_Device->CreateCommandSignature(&desc, nullptr, IID_PPV_ARGS(&m_DispatchRaysIndirectSig));
	}
}