#include "BvRenderDeviceD3D12.h"
#include "BvTypeConversionsD3D12.h"
#include "BvDescriptorHeapD3D12.h"
#include "BvCommandContextD3D12.h"
#include "BvSwapChainD3D12.h"
#include "BvBufferD3D12.h"
#include "BvBufferViewD3D12.h"
#include "BvTextureD3D12.h"
#include "BvTextureViewD3D12.h"
#include "BvSamplerD3D12.h"
#include "BvRenderPassD3D12.h"
#include "BvShaderResourceD3D12.h"
#include "BvShaderD3D12.h"
#include "BvPipelineStateD3D12.h"
#include "BvGPUFenceD3D12.h"
#include "BvUtilsD3D12.h"
#include "BDeV/Core/RenderAPI/BvRenderAPIUtils.h"


extern void OnD3D12DeviceDestroyed(u32 index);


BvRenderDeviceD3D12::BvRenderDeviceD3D12(BvRenderEngineD3D12* pEngine, IDXGIAdapter1* pAdapter, BvDeviceInfoD3D12* pDeviceInfo, u32 index,
	const BvGPUInfo& gpuInfo, const RenderDeviceDesc& renderDeviceDesc)
	: m_pEngine(pEngine), m_GPUInfo(gpuInfo), m_Index(index), m_pAdapter(pAdapter), m_pDeviceInfo(pDeviceInfo)
{
	Create(renderDeviceDesc);
}


BvRenderDeviceD3D12::~BvRenderDeviceD3D12()
{
	Destroy();
}


bool BvRenderDeviceD3D12::CreateSwapChainImpl(BvWindow* pWindow, const SwapChainDesc& swapChainDesc, IBvCommandContext* pContext, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	auto result = D3D12Utils::CreateSwapChainD3D12(this, pWindow, const_cast<SwapChainDesc&>(swapChainDesc), TO_D3D12(pContext));
	if (FAILED(result.first))
	{
		return false;
	}

	*ppObj = BV_RC_CREATE_CUSTOM(*BV_DEFAULT_MEMORY_ARENA, BvSwapChainD3D12, this, pWindow, swapChainDesc, TO_D3D12(pContext),
		result.second.m_SwapChain, result.second.m_Textures, result.second.m_PresentFlags);

	return true;
}


bool BvRenderDeviceD3D12::CreateBufferImpl(const BufferDesc& desc, const BufferInitData* pInitData, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	auto result = D3D12Utils::CreateBuffer(this, desc);
	if (FAILED(result.first))
	{
		return false;
	}

	*ppObj = BV_RC_CREATE_CUSTOM(*BV_DEFAULT_MEMORY_ARENA, BvBufferD3D12, this, desc, result.second.m_Buffer, result.second.m_Allocation, result.second.m_pMappedMemory);

	return true;
}


bool BvRenderDeviceD3D12::CreateBufferViewImpl(const BufferViewDesc& desc, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	*ppObj = BV_RC_CREATE_CUSTOM(*BV_DEFAULT_MEMORY_ARENA, BvBufferViewD3D12, this, desc);

	return true;
}


bool BvRenderDeviceD3D12::CreateTextureImpl(const TextureDesc& desc, const TextureInitData* pInitData, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	D3D12_RESOURCE_STATES initialState = pInitData && pInitData->m_pSubresources && pInitData->m_SubresourceCount ? D3D12_RESOURCE_STATE_COPY_DEST
		: D3D12_RESOURCE_STATE_COMMON;
	auto result = D3D12Utils::CreateTexture(this, desc, initialState);
	if (FAILED(result.first))
	{
		return false;
	}

	if (pInitData)
	{
		D3D12Utils::UploadMemoryToGPU(this, result.second.m_Texture.Get(), desc, *pInitData);
	}

	*ppObj = BV_RC_CREATE_CUSTOM(*BV_DEFAULT_MEMORY_ARENA, BvTextureD3D12, this, desc, result.second.m_Texture, result.second.m_Allocation);

	return true;
}


bool BvRenderDeviceD3D12::CreateTextureViewImpl(const TextureViewDesc& desc, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	*ppObj = BV_RC_CREATE_CUSTOM(*BV_DEFAULT_MEMORY_ARENA, BvTextureViewD3D12, this, desc);

	return true;
}


bool BvRenderDeviceD3D12::CreateSamplerImpl(const SamplerDesc& desc, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	*ppObj = BV_RC_CREATE_CUSTOM(*BV_DEFAULT_MEMORY_ARENA, BvSamplerD3D12, this, desc);

	return true;
}


bool BvRenderDeviceD3D12::CreateRenderPassImpl(const RenderPassDesc& renderPassDesc, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	*ppObj = BV_RC_CREATE_CUSTOM(*BV_DEFAULT_MEMORY_ARENA, BvRenderPassD3D12, this, renderPassDesc);

	return true;
}


bool BvRenderDeviceD3D12::CreateShaderResourceLayoutImpl(const ShaderResourceLayoutCreateDesc& srlDesc, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	*ppObj = BV_RC_CREATE_CUSTOM(*BV_DEFAULT_MEMORY_ARENA, BvShaderResourceLayoutD3D12, this, srlDesc);

	return true;
}


bool BvRenderDeviceD3D12::CreateShaderImpl(const ShaderDesc& shaderDesc, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	*ppObj = BV_RC_CREATE_CUSTOM(*BV_DEFAULT_MEMORY_ARENA, BvShaderD3D12, this, shaderDesc);

	return true;
}


bool BvRenderDeviceD3D12::CreateGraphicsPipelineImpl(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc, IBvPipelineCache* pPipelineCache, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	auto result = D3D12Utils::CreatePipelineState(this, graphicsPipelineStateDesc, pPipelineCache ? TO_D3D12(pPipelineCache)->GetHandle() : nullptr);
	if (FAILED(result.first))
	{
		return false;
	}

	*ppObj = BV_RC_CREATE_CUSTOM(*BV_DEFAULT_MEMORY_ARENA, BvGraphicsPipelineStateD3D12, this, graphicsPipelineStateDesc, result.second.m_PSO,
		result.second.m_RootSig, result.second.m_PrimitiveTopology);

	return true;
}


bool BvRenderDeviceD3D12::CreateComputePipelineImpl(const ComputePipelineStateDesc& computePipelineStateDesc, IBvPipelineCache* pPipelineCache, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	auto result = D3D12Utils::CreatePipelineState(this, computePipelineStateDesc, pPipelineCache ? TO_D3D12(pPipelineCache)->GetHandle() : nullptr);
	if (FAILED(result.first))
	{
		return false;
	}

	*ppObj = BV_RC_CREATE_CUSTOM(*BV_DEFAULT_MEMORY_ARENA, BvComputePipelineStateD3D12, this, computePipelineStateDesc, result.second.m_PSO,
		result.second.m_RootSig);

	return true;
}


bool BvRenderDeviceD3D12::CreateRayTracingPipelineImpl(const RayTracingPipelineStateDesc& rayTracingPipelineStateDesc, IBvPipelineCache* pPipelineCache, void** ppObj)
{
	return false;
}


bool BvRenderDeviceD3D12::CreateQueryImpl(QueryType queryType, void** ppObj)
{
	return false;
}


bool BvRenderDeviceD3D12::CreateFenceImpl(const GPUFenceDesc& fenceDesc, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	auto result = D3D12Utils::CreateGPUFence(this, fenceDesc);
	if (FAILED(result.first))
	{
		return false;
	}

	*ppObj = BV_RC_CREATE_CUSTOM(*BV_DEFAULT_MEMORY_ARENA, BvGPUFenceD3D12, this, result.second.m_Fence, result.second.m_Event);

	return true;
}


bool BvRenderDeviceD3D12::CreateAccelerationStructureImpl(const RayTracingAccelerationStructureDesc& asDesc, void** ppObj)
{
	return false;
}


bool BvRenderDeviceD3D12::CreateShaderBindingTableImpl(const ShaderBindingTableDesc& sbtDesc, IBvCommandContext* pContext, void** ppObj)
{
	return false;
}


bool BvRenderDeviceD3D12::CreateCommandContextImpl(const CommandContextDesc& commandContextDesc, void** ppObj)
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
		return false;
	}

	auto contextGroupIndex = u32(commandContextDesc.m_CommandType) - 1;
	auto pContext = BV_RC_CREATE_CUSTOM(*BV_DEFAULT_MEMORY_ARENA, BvCommandContextD3D12, this, 3, m_Contexts[contextGroupIndex].Size(), contextGroupIndex, queue.Get());
	m_Contexts[contextGroupIndex].EmplaceBack(pContext);
	*ppObj = pContext;

	return true;
}


bool BvRenderDeviceD3D12::CreatePipelineCacheImpl(const PipelineCacheInitData* pInitData, void** ppObj)
{
	BV_ASSERT(ppObj != nullptr, "Invalid pointer");

	PipelineCacheInitData initData = pInitData ? *pInitData : PipelineCacheInitData{};
	auto result = D3D12Utils::CreatePipelineLibrary(this, initData);
	if (FAILED(result.first))
	{
		return false;
	}

	*ppObj = BV_RC_CREATE_CUSTOM(*BV_DEFAULT_MEMORY_ARENA, BvPipelineCacheD3D12, this, result.second);

	return true;
}


void BvRenderDeviceD3D12::WaitIdle() const
{
	for (auto i = 0; i < ArraySize(m_Contexts); ++i)
	{
		for (auto pContext : m_Contexts[i])
		{
			pContext->GetCommandQueue()->WaitIdle();
		}
	}
}


void BvRenderDeviceD3D12::GetCopyableFootprints(const TextureDesc& textureDesc, u32 subresourceCount, SubresourceFootprint* pSubresources, u64* pTotalBytes,
	u64 baseOffset, u64 firstSubresource) const
{
	u64 totalSize = 0;
	BvVector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> footprints(subresourceCount);

	auto totalBytes = BvRenderUtils::GetCopyableFootprints(textureDesc, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT,
		subresourceCount, pSubresources, baseOffset, firstSubresource);

	if (pTotalBytes)
	{
		*pTotalBytes = totalBytes;
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


void BvRenderDeviceD3D12::Create(const RenderDeviceDesc& renderDeviceDesc)
{
	auto hr = D3D12CreateDevice(m_pAdapter, m_pDeviceInfo->m_FeatureLevel, IID_PPV_ARGS(&m_Device));
	BV_ASSERT(SUCCEEDED(hr), "Device creation failed");

	if (!m_Device)
	{
		return;
	}

	CreateDescriptorHeaps();
	SetupSupportedDisplayFormats();
	CreateCommandSignatures();
	CreateAllocator();

	if (m_pEngine->IsDebugEnabled())
	{
		if (SUCCEEDED(m_Device.As(&m_InfoQueue)))
		{
			// TODO: Work on debug features
			//m_InfoQueue->RegisterMessageCallback(, , , &m_DebugCallbackCookie);
		}
	}
}


void BvRenderDeviceD3D12::Destroy()
{
	if (m_Device)
	{
		if (m_InfoQueue)
		{
			// TODO: Work on debug features
			//m_InfoQueue->UnregisterMessageCallback(m_DebugCallbackCookie);
		}

		BV_DELETE(m_pGPUShaderHeap);
		BV_DELETE(m_pGPUSamplerHeap);
		BV_DELETE(m_pCPUShaderHeap);
		BV_DELETE(m_pCPUSamplerHeap);
		m_Device = nullptr;
		
		OnD3D12DeviceDestroyed(m_Index);
	}
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
	m_pCPURTVHeap = BV_NEW(BvCPUDescriptorHeapD3D12)(this, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 64);
	m_pCPUDSVHeap = BV_NEW(BvCPUDescriptorHeapD3D12)(this, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 16);
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

	auto deviceCaps = m_GPUInfo.m_DeviceCaps;
	if (EHasFlag(deviceCaps, RenderDeviceCapabilities::kMeshShader))
	{
		desc.ByteStride = sizeof(D3D12_DISPATCH_MESH_ARGUMENTS);
		argDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH;
		m_Device->CreateCommandSignature(&desc, nullptr, IID_PPV_ARGS(&m_DispatchMeshIndirectSig));
	}

	if (EHasFlag(deviceCaps, RenderDeviceCapabilities::kRayTracing))
	{
		desc.ByteStride = sizeof(D3D12_DISPATCH_RAYS_DESC);
		argDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_RAYS;
		m_Device->CreateCommandSignature(&desc, nullptr, IID_PPV_ARGS(&m_DispatchRaysIndirectSig));
	}
}