#include "BvRenderDeviceD3D12.h"
#include "BvTypeConversionsD3D12.h"
#include "BvDescriptorHeapD3D12.h"


BvRenderDeviceD3D12::BvRenderDeviceD3D12(BvRenderEngineD3D12* pEngine, IDXGIAdapter1* pAdapter, BvDeviceInfoD3D12* pDeviceInfo, u32 index, const BvGPUInfo& gpuInfo, const BvRenderDeviceCreateDescD3D12& deviceDesc)
	: m_pEngine(pEngine), m_GPUInfo(gpuInfo), m_Index(index), m_pAdapter(pAdapter), m_pDeviceInfo(pDeviceInfo)
{
	Create(deviceDesc);
}


BvRenderDeviceD3D12::~BvRenderDeviceD3D12()
{
	Destroy();
}


bool BvRenderDeviceD3D12::CreateSwapChainImpl(BvWindow* pWindow, const SwapChainDesc& swapChainDesc, IBvCommandContext* pContext, const BvUUID& objId, void** ppObj)
{
	return false;
}


bool BvRenderDeviceD3D12::CreateBufferImpl(const BufferDesc& desc, const BufferInitData* pInitData, const BvUUID& objId, void** ppObj)
{
	return false;
}


bool BvRenderDeviceD3D12::CreateBufferViewImpl(const BufferViewDesc& desc, const BvUUID& objId, void** ppObj)
{
	return false;
}


bool BvRenderDeviceD3D12::CreateTextureImpl(const TextureDesc& desc, const TextureInitData* pInitData, const BvUUID& objId, void** ppObj)
{
	return false;
}


bool BvRenderDeviceD3D12::CreateTextureViewImpl(const TextureViewDesc& desc, const BvUUID& objId, void** ppObj)
{
	return false;
}


bool BvRenderDeviceD3D12::CreateSamplerImpl(const SamplerDesc& desc, const BvUUID& objId, void** ppObj)
{
	return false;
}


bool BvRenderDeviceD3D12::CreateRenderPassImpl(const RenderPassDesc& renderPassDesc, const BvUUID& objId, void** ppObj)
{
	return false;
}


bool BvRenderDeviceD3D12::CreateShaderResourceLayoutImpl(const ShaderResourceLayoutDesc& srlDesc, const BvUUID& objId, void** ppObj)
{
	return false;
}


bool BvRenderDeviceD3D12::CreateShaderImpl(const ShaderCreateDesc& shaderDesc, const BvUUID& objId, void** ppObj)
{
	return false;
}


bool BvRenderDeviceD3D12::CreateGraphicsPipelineImpl(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc, const BvUUID& objId, void** ppObj)
{
	return false;
}


bool BvRenderDeviceD3D12::CreateComputePipelineImpl(const ComputePipelineStateDesc& computePipelineStateDesc, const BvUUID& objId, void** ppObj)
{
	return false;
}


bool BvRenderDeviceD3D12::CreateRayTracingPipelineImpl(const RayTracingPipelineStateDesc& rayTracingPipelineStateDesc, const BvUUID& objId, void** ppObj)
{
	return false;
}


bool BvRenderDeviceD3D12::CreateQueryImpl(QueryType queryType, const BvUUID& objId, void** ppObj)
{
	return false;
}


bool BvRenderDeviceD3D12::CreateFenceImpl(u64 value, const BvUUID& objId, void** ppObj)
{
	return false;
}


bool BvRenderDeviceD3D12::CreateAccelerationStructureImpl(const RayTracingAccelerationStructureDesc& asDesc, const BvUUID& objId, void** ppObj)
{
	return false;
}


bool BvRenderDeviceD3D12::CreateShaderBindingTableImpl(const ShaderBindingTableDesc& sbtDesc, IBvCommandContext* pContext, const BvUUID& objId, void** ppObj)
{
	return false;
}


bool BvRenderDeviceD3D12::CreateCommandContextImpl(const CommandContextDesc& commandContextDesc, const BvUUID& objId, void** ppObj)
{
	D3D12_COMMAND_QUEUE_DESC cqd{};
	if (commandContextDesc.m_CommandType != CommandType::kNone)
	{
		cqd.Type = GetD3D12CommandListType(commandContextDesc.m_CommandType);
	}
	else
	{
		BV_ASSERT(commandContextDesc.m_ContextGroupIndex < m_GPUInfo.m_ContextGroups.Size(), "Invalid context group index");
		cqd.Type = GetD3D12CommandListType(m_GPUInfo.m_ContextGroups[commandContextDesc.m_ContextGroupIndex].m_DedicatedCommandType);
	}

	ComPtr<ID3D12CommandQueue> queue;
	auto hr = m_Device->CreateCommandQueue(&cqd, IID_PPV_ARGS(&queue));
	if (FAILED(hr))
	{
		// TODO: Handle error
	}

	return false;
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

	auto hr = D3D12CreateDevice(m_pAdapter, m_pDeviceInfo->m_FeatureLevel, IID_PPV_ARGS(&m_Device));
	BV_ASSERT(SUCCEEDED(hr), "Device creation failed");

	if (!m_Device)
	{
		return;
	}

	CreateDescriptorHeaps();
	SetupSupportedDisplayFormats();
	CreateCommandSignatures();
}


void BvRenderDeviceD3D12::Destroy()
{
	if (m_Device)
	{
		BV_DELETE(m_pGPUShaderHeap);
		BV_DELETE(m_pGPUSamplerHeap);
		BV_DELETE(m_pCPUShaderHeap);
		BV_DELETE(m_pCPUSamplerHeap);
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


void BvRenderDeviceD3D12::CreateDescriptorHeaps()
{
	m_pGPUShaderHeap = BV_NEW(BvGPUDescriptorHeapD3D12)(this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1000000);
	m_pGPUSamplerHeap = BV_NEW(BvGPUDescriptorHeapD3D12)(this, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 2048);

	m_pCPUShaderHeap = BV_NEW(BvCPUDescriptorHeapD3D12)(this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1000);
	m_pCPUSamplerHeap = BV_NEW(BvCPUDescriptorHeapD3D12)(this, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 64);
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