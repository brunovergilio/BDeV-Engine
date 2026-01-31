#include "BvUtilsD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BvCommandContextD3D12.h"
#include "BvCommandListD3D12.h"
#include "BvShaderResourceD3D12.h"
#include "BvSamplerD3D12.h"
#include "BvShaderD3D12.h"
#include "BvTypeConversionsD3D12.h"
#include "BDeV/Core/RenderAPI/BvRenderAPIUtils.h"
#include "BDeV/Core/System/Window/BvWindow.h"
#include <dxgi1_5.h>


namespace D3D12Utils
{
	Obj<SwapChainObj> CreateSwapChainD3D12(BvRenderDeviceD3D12* pDevice, BvWindow* pWindow, SwapChainDesc& swapChainDesc, BvCommandContextD3D12* pContext)
	{
		Obj<SwapChainObj> result;
		auto& hr = result.first;
		auto& swapChain = result.second.m_SwapChain;
		auto& textures = result.second.m_Textures;
		auto& presentFlags = result.second.m_PresentFlags;

		auto [width, height] = pWindow->GetSize();
		auto fi = BvRenderUtils::GetFormatInfo(swapChainDesc.m_Format);

		if (swapChainDesc.m_WindowMode == SwapChainMode::kFullscreen)
		{
			swapChainDesc.m_SwapChainImageCount += 1;
		}

		DXGI_SWAP_CHAIN_DESC1 scd{};
		scd.Width = width;
		scd.Height = height;
		scd.Format = DXGI_FORMAT(fi.m_IsSRGBFormat ? fi.m_SRGBOrLinearVariant : swapChainDesc.m_Format);
		//scd.Stereo = FALSE;
		//scd.SampleDesc.Quality = 0;
		scd.SampleDesc.Count = 1;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
		scd.BufferCount = swapChainDesc.m_SwapChainImageCount;
		scd.Scaling = DXGI_SCALING_NONE;
		scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		scd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		ComPtr<IDXGIFactory2> factory2;
		hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory2));
		if (FAILED(hr))
		{
			// TODO: Handle error
			return result;
		}

		auto hWnd = pWindow->GetHandle();

		{
			ComPtr<IDXGIFactory5> factory5;
			hr = factory2.As(&factory5);
			if (SUCCEEDED(hr))
			{
				BOOL allowTearing = FALSE;
				hr = factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
				if (SUCCEEDED(hr) && allowTearing)
				{
					scd.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
				}
			}
		}

		//auto [num, den] = GetRefreshRate(factory2.Get(), scd.Format);
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC scfd{};
		scfd.Windowed = swapChainDesc.m_WindowMode != SwapChainMode::kFullscreen ? TRUE : FALSE;
		//scfd.RefreshRate = { num, den };
		scfd.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		scfd.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

		if (swapChainDesc.m_WindowMode != SwapChainMode::kFullscreen)
		{
			presentFlags = DXGI_PRESENT_ALLOW_TEARING;
		}

		ComPtr<IDXGISwapChain1> swapChain1;
		hr = factory2->CreateSwapChainForHwnd(pContext->GetCommandQueue()->GetHandle(), hWnd, &scd, &scfd, nullptr, &swapChain1);
		if (FAILED(hr))
		{
			// TODO: Handle error
			return result;
		}

		hr = swapChain1.As(&swapChain);
		if (FAILED(hr))
		{
			// TODO: Handle error
			return result;
		}

		ComPtr<IDXGIFactory1> factory1;
		hr = factory2->GetParent(IID_PPV_ARGS(&factory1));
		if (SUCCEEDED(hr))
		{
			factory1->MakeWindowAssociation(hWnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
		}

		textures.Resize(scd.BufferCount);
		for (auto i = 0u; i < scd.BufferCount; i++)
		{
			hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&textures[i]));
		}

		return result;
	}

	Obj<BufferObj> CreateBuffer(BvRenderDeviceD3D12* pDevice, const BufferDesc& bufferDesc)
	{
		Obj<BufferObj> result;
		auto& hr = result.first;
		auto& buffer = result.second.m_Buffer;
		auto& allocation = result.second.m_Allocation;
		auto& pMappedMemory = result.second.m_pMappedMemory;

		u64 alignedSize = bufferDesc.m_Size;
		if (EHasFlag(bufferDesc.m_UsageFlags, BufferUsage::kConstantBuffer))
		{
			alignedSize = RoundToNearestPowerOf2(alignedSize, u64(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT));
		}

		CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(alignedSize, GetD3D12ResourceFlags(bufferDesc.m_UsageFlags));

		D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_COMMON;

		D3D12MA::ALLOCATION_DESC allocationDesc{};
		allocationDesc.HeapType = GetD3D12HeapType(bufferDesc.m_MemoryType);
		if (allocationDesc.HeapType == D3D12_HEAP_TYPE_UPLOAD)
		{
			resourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
		}
		else if (allocationDesc.HeapType == D3D12_HEAP_TYPE_READBACK)
		{
			resourceState = D3D12_RESOURCE_STATE_COPY_DEST;
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
		}

		auto pAllocator = pDevice->GetAllocator();
		auto pD3DDevice = pDevice->GetHandle();
		D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = pD3DDevice->GetResourceAllocationInfo(0, 1, &resourceDesc);

		do 
		{
			hr = pAllocator->AllocateMemory(&allocationDesc, &allocationInfo, &allocation);
			if (FAILED(hr))
			{
				break;
			}

			hr = pD3DDevice->CreatePlacedResource(allocation->GetHeap(), allocation->GetOffset(), &resourceDesc, resourceState, nullptr, IID_PPV_ARGS(&buffer));
			if (FAILED(hr))
			{
				allocation = nullptr;
				break;
			}

			if (bufferDesc.m_MemoryType != MemoryType::kDevice)
			{
				if (EHasFlag(bufferDesc.m_CreateFlags, BufferCreateFlags::kCreateMapped))
				{
					hr = buffer->Map(0, nullptr, &pMappedMemory);
					if (FAILED(hr))
					{
						buffer = nullptr;
						allocation = nullptr;
					}
				}
			}
		} while (false);

		return result;
	}


	Obj<TextureObj> CreateTexture(BvRenderDeviceD3D12* pDevice, const TextureDesc& textureDesc, D3D12_RESOURCE_STATES initialState)
	{
		Obj<TextureObj> result;
		auto& hr = result.first;
		auto& texture = result.second.m_Texture;
		auto& allocation = result.second.m_Allocation;

		CD3DX12_RESOURCE_DESC resourceDesc = GetD3D12ResourceDesc(textureDesc);

		auto pAllocator = pDevice->GetAllocator();
		auto pD3DDevice = pDevice->GetHandle();
		D3D12MA::ALLOCATION_DESC allocationDesc{};
		allocationDesc.HeapType = GetD3D12HeapType(textureDesc.m_MemoryType);
		D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = pD3DDevice->GetResourceAllocationInfo(0, 1, &resourceDesc);

		do 
		{
			hr = pAllocator->AllocateMemory(&allocationDesc, &allocationInfo, &allocation);
			if (FAILED(hr))
			{
				break;
			}

			hr = pD3DDevice->CreatePlacedResource(allocation->GetHeap(), allocation->GetOffset(), &resourceDesc,
				initialState, nullptr, IID_PPV_ARGS(&texture));
			if (FAILED(hr))
			{
				allocation = nullptr;
			}
		} while (false);

		return result;
	}


	Obj<FenceObj> CreateGPUFence(BvRenderDeviceD3D12* pDevice, const GPUFenceDesc& fenceDesc)
	{
		Obj<FenceObj> result;
		auto& hr = result.first;
		auto& fence = result.second.m_Fence;
		auto& event = result.second.m_Event;

		do 
		{
			hr = pDevice->GetHandle()->CreateFence(fenceDesc.m_Value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
			if (FAILED(hr))
			{
				break;
			}

			event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (event == nullptr)
			{
				fence = nullptr;
			}
		} while (false);

		return result;
	}


	HRESULT CreateRootSignature(BvRenderDeviceD3D12* pDevice, BvShaderResourceLayoutD3D12* pLayout,
		D3D12_ROOT_SIGNATURE_FLAGS flags, ComPtr<ID3D12RootSignature>& rootSig)
	{
		auto& srlDesc = pLayout->GetDesc();
		ShaderStage samplerStages = ShaderStage::kUnknown;
		BvVector<CD3DX12_STATIC_SAMPLER_DESC> staticSamplers;
		for (auto i = 0u; i < srlDesc.m_ShaderResourceSets.Size(); ++i)
		{
			auto& set = srlDesc.m_ShaderResourceSets[i];

			for (auto j = 0u; j < set.m_Resources.Size(); ++j)
			{
				auto& resource = set.m_Resources[j];
				if (resource.m_StaticSamplers.Size() > 0)
				{
					samplerStages |= resource.m_ShaderStages;
					auto visibility = GetD3D12ShaderVisibility(resource.m_ShaderStages);
					for (auto k = 0; k < resource.m_Count; ++k)
					{
						staticSamplers.EmplaceBack(GetD3D12StaticSamplerDesc(resource.m_StaticSamplers[k]->GetDesc(), resource.m_Binding,
							set.m_Index, visibility));
					}
				}
			}
		}

		auto& params = pLayout->GetRootParams();
		//flags |= GetD3D12RootSignatureFlags(samplerStages);
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc(params.Size(), params.Data(), staticSamplers.Size(), staticSamplers.Data(), flags);
		ComPtr<ID3DBlob> blob, errorBlob;
		auto hr = D3D12SerializeVersionedRootSignature(&rootSigDesc, &blob, &errorBlob);
		if (FAILED(hr))
		{
			return hr;
		}

		return pDevice->GetHandle()->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&rootSig));
	}


	Obj<GraphicsPSOObj> CreatePipelineState(BvRenderDeviceD3D12* pDevice, const GraphicsPipelineStateDesc& graphicsPipelineStateDesc, ID3D12PipelineLibrary1* pLibrary)
	{
		Obj<GraphicsPSOObj> result;
		auto& hr = result.first;
		auto& pso = result.second.m_PSO;
		auto& rootSig = result.second.m_RootSig;
		auto& primitiveTopology = result.second.m_PrimitiveTopology;

		D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;

		VertexInputDesc* pVertexInputDescs = nullptr;

		// CD3DX12_PIPELINE_STATE_STREAM2 has support for mesh shaders and view instancing
		CD3DX12_PIPELINE_STATE_STREAM2 stream{};
		bool isMeshPipeline = false;
		bool isDefaultPipeline = false;
		for (auto i = 0; i < graphicsPipelineStateDesc.m_Shaders.Size(); ++i)
		{
			auto pShader = graphicsPipelineStateDesc.m_Shaders[i];
			auto shaderStage = pShader->GetShaderStage();
			auto pBlob = pShader->GetShaderBlob().Data();
			auto blobSize = pShader->GetShaderBlob().Size();
			D3D12_SHADER_BYTECODE byteCode{ pBlob, blobSize };

			switch (shaderStage)
			{
			case ShaderStage::kVertex: stream.VS = byteCode; isDefaultPipeline = true; flags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS; break;
			case ShaderStage::kHullOrControl: stream.HS = byteCode; isDefaultPipeline = true; flags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS; break;
			case ShaderStage::kDomainOrEvaluation: stream.DS = byteCode; isDefaultPipeline = true; flags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS; break;
			case ShaderStage::kGeometry: stream.GS = byteCode; isDefaultPipeline = true; flags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS; break;
			case ShaderStage::kPixelOrFragment: stream.PS = byteCode; flags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS; break;
			case ShaderStage::kMesh: stream.MS = byteCode; isMeshPipeline = true; flags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS; break;
			case ShaderStage::kAmplificationOrTask: stream.AS = byteCode; isMeshPipeline = true; flags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS; break;
			}
		}

		BV_ASSERT((isDefaultPipeline && !isMeshPipeline) || (!isDefaultPipeline && isMeshPipeline), "Can't use Mesh and Graphics shaders together");

		stream.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		stream.NodeMask = 0;

		BvFixedVector<D3D12_INPUT_ELEMENT_DESC, kMaxVertexBindings> inputElems(graphicsPipelineStateDesc.m_VertexInputDescs.Size());
		if (isDefaultPipeline)
		{
			if (graphicsPipelineStateDesc.m_VertexInputDescs.Size())
			{
				flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
			}

			for (auto i = 0; i < graphicsPipelineStateDesc.m_VertexInputDescs.Size(); ++i)
			{
				auto& input = graphicsPipelineStateDesc.m_VertexInputDescs[i];
				inputElems[i] = { input.m_pName, input.m_Index, DXGI_FORMAT(input.m_Format), input.m_Binding,
					input.m_Offset, GetD3D12InputClassification(input.m_InputRate), input.m_InstanceRate };
			}
			D3D12_INPUT_LAYOUT_DESC layoutDesc{ inputElems.Data(), u32(inputElems.Size()) };
			if (inputElems.Size() > 0)
			{
				stream.InputLayout = layoutDesc;
			}
			stream.PrimitiveTopologyType = GetD3D12PrimitiveTopologyType(graphicsPipelineStateDesc.m_InputAssemblyStateDesc.m_Topology);
			primitiveTopology = GetD3D12PrimitiveTopology(graphicsPipelineStateDesc.m_InputAssemblyStateDesc.m_Topology, graphicsPipelineStateDesc.m_PatchControlPoints);
			if (graphicsPipelineStateDesc.m_InputAssemblyStateDesc.m_PrimitiveRestart)
			{
				stream.IBStripCutValue = graphicsPipelineStateDesc.m_InputAssemblyStateDesc.m_IndexFormatForPrimitiveRestart == IndexFormat::kU32 ?
					D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF : D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;
			}
			else
			{
				stream.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
			}
		}

		hr = CreateRootSignature(pDevice, TO_D3D12(graphicsPipelineStateDesc.m_pShaderResourceLayout), flags, rootSig);
		if (FAILED(hr))
		{
			return result;
		}
		stream.pRootSignature = rootSig.Get();

		D3D12_RT_FORMAT_ARRAY rtvFormats{};
		CD3DX12_BLEND_DESC blendDesc{};
		blendDesc.AlphaToCoverageEnable = graphicsPipelineStateDesc.m_BlendStateDesc.m_AlphaToCoverageEnable;
		for (auto i = 0; i < graphicsPipelineStateDesc.m_RenderTargetFormats.Size(); i++)
		{
			rtvFormats.NumRenderTargets++;
			rtvFormats.RTFormats[i] = DXGI_FORMAT(graphicsPipelineStateDesc.m_RenderTargetFormats[i]);

			auto& dst = blendDesc.RenderTarget[i];
			auto& src = graphicsPipelineStateDesc.m_BlendStateDesc.m_BlendAttachments[i];
			dst.BlendEnable = src.m_BlendEnable;
			dst.LogicOpEnable = graphicsPipelineStateDesc.m_BlendStateDesc.m_LogicEnable;
			dst.SrcBlend = GetD3D12Blend(src.m_SrcBlend);
			dst.DestBlend = GetD3D12Blend(src.m_DstBlend);
			dst.BlendOp = GetD3D12BlendOp(src.m_BlendOp);
			dst.SrcBlendAlpha = GetD3D12Blend(src.m_SrcBlendAlpha);
			dst.DestBlendAlpha = GetD3D12Blend(src.m_DstBlendAlpha);
			dst.BlendOpAlpha = GetD3D12BlendOp(src.m_AlphaBlendOp);
			dst.LogicOp = GetD3D12LogicOp(graphicsPipelineStateDesc.m_BlendStateDesc.m_LogicOp);
			dst.RenderTargetWriteMask = u8(src.m_RenderTargetWriteMask);
		}
		stream.BlendState = blendDesc;
		stream.RTVFormats = rtvFormats;
		stream.DSVFormat = DXGI_FORMAT(graphicsPipelineStateDesc.m_DepthStencilFormat);
		auto& depth = graphicsPipelineStateDesc.m_DepthStencilDesc;
		stream.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC1(depth.m_DepthTestEnable, depth.m_DepthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO,
			GetD3D12ComparisonFunc(depth.m_DepthOp), depth.m_StencilTestEnable, depth.m_StencilReadMask, depth.m_StencilWriteMask,
			GetD3D12StencilOp(depth.m_StencilFront.m_StencilFailOp), GetD3D12StencilOp(depth.m_StencilFront.m_StencilDepthFailOp),
			GetD3D12StencilOp(depth.m_StencilFront.m_StencilPassOp), GetD3D12ComparisonFunc(depth.m_StencilFront.m_StencilFunc),
			GetD3D12StencilOp(depth.m_StencilBack.m_StencilFailOp), GetD3D12StencilOp(depth.m_StencilBack.m_StencilDepthFailOp),
			GetD3D12StencilOp(depth.m_StencilBack.m_StencilPassOp), GetD3D12ComparisonFunc(depth.m_StencilBack.m_StencilFunc),
			graphicsPipelineStateDesc.m_DepthStencilDesc.m_DepthBoundsTestEnable);

		auto& raster = graphicsPipelineStateDesc.m_RasterizerStateDesc;
		stream.RasterizerState = CD3DX12_RASTERIZER_DESC(GetD3D12FillMode(raster.m_FillMode), GetD3D12CullMode(raster.m_CullMode), raster.m_FrontFace == FrontFace::kCounterClockwise,
			raster.m_DepthBias, raster.m_DepthBiasClamp, raster.m_DepthBiasSlope, raster.m_EnableDepthClip, graphicsPipelineStateDesc.m_SampleCount > 1, FALSE, 0,
			raster.m_EnableConservativeRasterization ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF);

		stream.SampleDesc = { graphicsPipelineStateDesc.m_SampleCount, 0 };
		stream.SampleMask = graphicsPipelineStateDesc.m_SampleMask;

		BvFixedVector<D3D12_VIEW_INSTANCE_LOCATION, D3D12_MAX_VIEW_INSTANCE_COUNT> viewLocations;
		if (graphicsPipelineStateDesc.m_MultiviewCount > 1)
		{
			BV_ASSERT(graphicsPipelineStateDesc.m_MultiviewCount <= D3D12_MAX_VIEW_INSTANCE_COUNT, "Multiview count must not be greater than D3D12_MAX_VIEW_INSTANCE_COUNT");
			for (auto i = 0u; i < graphicsPipelineStateDesc.m_MultiviewCount; ++i)
			{
				viewLocations.PushBack({ i, i });
			}
			stream.ViewInstancingDesc = CD3DX12_VIEW_INSTANCING_DESC(graphicsPipelineStateDesc.m_MultiviewCount, viewLocations.Data(),
				D3D12_VIEW_INSTANCING_FLAG_ENABLE_VIEW_INSTANCE_MASKING);
		}

		ComPtr<ID3D12Device2> device2;
		ComPtr<ID3D12Device> device(pDevice->GetHandle());
		hr = device.As(&device2);
		if (FAILED(hr))
		{
			return result;
		}

		D3D12_PIPELINE_STATE_STREAM_DESC psd{ sizeof(stream), &stream };
		//if (pLibrary)
		//{
		//	hr = pLibrary->LoadPipeline(L"", &psd, IID_PPV_ARGS(&pso));
		//	if (SUCCEEDED(hr))
		//	{
		//		return result;
		//	}
		//}

		hr = device2->CreatePipelineState(&psd, IID_PPV_ARGS(&pso));
		if (FAILED(hr))
		{
			rootSig = nullptr;
		}

		//if (pLibrary)
		//{
		//	hr = pLibrary->StorePipeline(L"", pso.Get());
		//}

		return result;
	}


	Obj<ComputePSOObj> CreatePipelineState(BvRenderDeviceD3D12* pDevice, const ComputePipelineStateDesc& computePipelineStateDesc, ID3D12PipelineLibrary1* pLibrary)
	{
		Obj<ComputePSOObj> result;
		auto& hr = result.first;
		auto& pso = result.second.m_PSO;
		auto& rootSig = result.second.m_RootSig;

		D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;

		hr = CreateRootSignature(pDevice, TO_D3D12(computePipelineStateDesc.m_pShaderResourceLayout), flags, rootSig);
		if (FAILED(hr))
		{
			return result;
		}

		auto pShader = computePipelineStateDesc.m_pShader;
		auto shaderStage = pShader->GetShaderStage();
		auto pBlob = pShader->GetShaderBlob().Data();
		auto blobSize = pShader->GetShaderBlob().Size();
		D3D12_SHADER_BYTECODE byteCode{ pBlob, blobSize };

		CD3DX12_PIPELINE_STATE_STREAM2 stream{};
		stream.CS = byteCode;
		stream.pRootSignature = rootSig.Get();

		ComPtr<ID3D12Device2> device2;
		ComPtr<ID3D12Device> device(pDevice->GetHandle());
		hr = device.As(&device2);
		if (FAILED(hr))
		{
			return result;
		}

		D3D12_PIPELINE_STATE_STREAM_DESC psd{ sizeof(stream), &stream };
		//if (pLibrary)
		//{
		//	hr = pLibrary->LoadPipeline(L"", &psd, IID_PPV_ARGS(&pso));
		//	if (SUCCEEDED(hr))
		//	{
		//		return result;
		//	}
		//}

		hr = device2->CreatePipelineState(&psd, IID_PPV_ARGS(&pso));
		if (FAILED(hr))
		{
			rootSig = nullptr;
		}

		//if (pLibrary)
		//{
		//	hr = pLibrary->StorePipeline(L"", pso.Get());
		//}

		return result;
	}


	Obj<ComPtr<ID3D12PipelineLibrary1>> CreatePipelineLibrary(BvRenderDeviceD3D12* pDevice, const PipelineCacheInitData& initData)
	{
		Obj<ComPtr<ID3D12PipelineLibrary1>> result;
		auto& hr = result.first;
		auto& library = result.second;

		ComPtr<ID3D12Device> device(pDevice->GetHandle());
		ComPtr<ID3D12Device1> device1;
		hr = device.As(&device1);
		if (FAILED(hr))
		{
			return result;
		}

		hr = device1->CreatePipelineLibrary(initData.m_pInitData, initData.m_Size, IID_PPV_ARGS(&library));

		return result;
	}


	void UploadMemoryToGPU(BvRenderDeviceD3D12* pDevice, ID3D12Resource* pBuffer, const BufferInitData& initData)
	{
		if (!initData.m_pContext || !initData.m_Size || !initData.m_pData)
		{
			return;
		}

		BufferDesc bufferDesc;
		bufferDesc.m_MemoryType = MemoryType::kUpload;
		bufferDesc.m_Size = initData.m_Size;
		bufferDesc.m_CreateFlags = BufferCreateFlags::kCreateMapped;

		auto stagingBuffer = CreateBuffer(pDevice, bufferDesc);
		if (FAILED(stagingBuffer.first))
		{
			return;
		}

		memcpy(stagingBuffer.second.m_pMappedMemory, initData.m_pData, initData.m_Size);

		auto pContext = TO_D3D12(initData.m_pContext);
		pContext->NewCommandList();
		pContext->GetCurrentCommandList()->CopyBuffer(stagingBuffer.second.m_Buffer.Get(), pBuffer, { initData.m_Size, 0, 0 });
		pContext->ExecuteAndWait();
	}


	void UploadMemoryToGPU(BvRenderDeviceD3D12* pDevice, ID3D12Resource* pTexture, const TextureDesc& textureDesc, const TextureInitData& initData)
	{
		constexpr ResourceState initialState = ResourceState::kTransferDst;

		if (!initData.m_pContext || !initData.m_pSubresources || !initData.m_SubresourceCount)
		{
			return;
		}

		auto pContext = TO_D3D12(initData.m_pContext);
		pContext->NewCommandList();

		Obj<BufferObj> stagingBuffer{};

		u64 totalBytes = 0;
		BvVector<SubresourceFootprint> footprints(initData.m_SubresourceCount);
		pDevice->GetCopyableFootprints(textureDesc, initData.m_SubresourceCount, footprints.Data(), &totalBytes, 0, initData.m_FirstSubresource);

		BufferDesc bufferDesc;
		bufferDesc.m_MemoryType = MemoryType::kUpload;
		bufferDesc.m_Size = totalBytes;
		bufferDesc.m_CreateFlags = BufferCreateFlags::kCreateMapped;

		stagingBuffer = CreateBuffer(pDevice, bufferDesc);
		if (FAILED(stagingBuffer.first))
		{
			return;
		}

		BvRenderUtils::UpdateSubresources(initData.m_SubresourceCount, initData.m_pSubresources, footprints.Data(), stagingBuffer.second.m_pMappedMemory);

		auto pCL = pContext->GetCurrentCommandList();
		pCL->CopyBufferToTexture(stagingBuffer.second.m_Buffer.Get(), pTexture, footprints.Size(), footprints.Data());

		if (initialState != initData.m_ResourceState)
		{
			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(pTexture, D3D12_RESOURCE_STATE_COPY_DEST,
				GetD3D12ResourceState(initData.m_ResourceState));
			pCL->GetHandle()->ResourceBarrier(1, &barrier);
		}

		pContext->ExecuteAndWait();
	}
}