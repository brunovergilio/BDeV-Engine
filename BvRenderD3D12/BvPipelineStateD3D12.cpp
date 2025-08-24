#include "BvPipelineStateD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BvShaderResourceLayoutD3D12.h"
#include "BvTypeConversionsD3D12.h"
#include "BDeV/Core/RenderAPI/BvRenderAPIUtils.h"


BV_D3D12_DEVICE_RES_DEF(BvGraphicsPipelineStateD3D12);


BvGraphicsPipelineStateD3D12::BvGraphicsPipelineStateD3D12(BvRenderDeviceD3D12* pDevice, const GraphicsPipelineStateDesc& pipelineStateDesc)
	: m_pDevice(pDevice), m_PipelineStateDesc(pipelineStateDesc)
{
	VertexInputDesc* pVertexInputDescs = nullptr;
	if (m_PipelineStateDesc.m_VertexInputDescCount)
	{
		pVertexInputDescs = BV_NEW_ARRAY(VertexInputDesc, m_PipelineStateDesc.m_VertexInputDescCount);
		memcpy(pVertexInputDescs, m_PipelineStateDesc.m_pVertexInputDescs, sizeof(VertexInputDesc) * m_PipelineStateDesc.m_VertexInputDescCount);
		m_PipelineStateDesc.m_pVertexInputDescs = pVertexInputDescs;
	}

	for (auto i = 0u; i < m_PipelineStateDesc.m_VertexInputDescCount; ++i)
	{
		auto& vertex = m_PipelineStateDesc.m_pVertexInputDescs[i];
		if (vertex.m_pName)
		{
			auto count = std::char_traits<char>::length(vertex.m_pName) + 1;
			auto pName = BV_NEW_ARRAY(char, count);
			strcpy(pName, vertex.m_pName);
			vertex.m_pName = pName;
		}
	}

	Create();
}


BvGraphicsPipelineStateD3D12::~BvGraphicsPipelineStateD3D12()
{
	Destroy();

	for (auto i = 0u; i < m_PipelineStateDesc.m_VertexInputDescCount; ++i)
	{
		auto& vertex = m_PipelineStateDesc.m_pVertexInputDescs[i];
		if (vertex.m_pName)
		{
			BV_DELETE_ARRAY(vertex.m_pName);
		}
	}

	if (m_PipelineStateDesc.m_pVertexInputDescs)
	{
		BV_DELETE_ARRAY(m_PipelineStateDesc.m_pVertexInputDescs);
	}
}


void BvGraphicsPipelineStateD3D12::Create()
{
	// CD3DX12_PIPELINE_STATE_STREAM2 has support for mesh shaders and view instancing
	CD3DX12_PIPELINE_STATE_STREAM2 stream{};
	bool isMeshPipeline = false;
	bool isDefaultPipeline = false;
	for (auto i = 0; i < 5; ++i)
	{
		auto pShader = m_PipelineStateDesc.m_Shaders[i];
		auto shaderStage = pShader->GetShaderStage();
		auto pBlob = pShader->GetShaderBlob().Data();
		auto blobSize = pShader->GetShaderBlob().Size();
		D3D12_SHADER_BYTECODE byteCode{ pBlob, blobSize };

		switch (shaderStage)
		{
		case ShaderStage::kVertex: stream.VS = byteCode; isDefaultPipeline = true; break;
		case ShaderStage::kHullOrControl: stream.VS = byteCode; isDefaultPipeline = true; break;
		case ShaderStage::kDomainOrEvaluation: stream.VS = byteCode; isDefaultPipeline = true; break;
		case ShaderStage::kGeometry: stream.VS = byteCode; isDefaultPipeline = true; break;
		case ShaderStage::kPixelOrFragment: stream.VS = byteCode; break;
		case ShaderStage::kMesh: stream.VS = byteCode; isMeshPipeline = true; break;
		case ShaderStage::kAmplificationOrTask: stream.VS = byteCode; isMeshPipeline = true; break;
		}
	}

	BV_ASSERT((isDefaultPipeline && !isMeshPipeline) || (!isDefaultPipeline && isMeshPipeline), "Can't use Mesh and Graphics shaders together");

	stream.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	stream.NodeMask = 0;
	stream.pRootSignature = TO_D3D12(m_PipelineStateDesc.m_pShaderResourceLayout)->GetRootSignature();

	BvFixedVector<D3D12_INPUT_ELEMENT_DESC, kMaxVertexBindings> inputElems(m_PipelineStateDesc.m_VertexInputDescCount);
	if (isDefaultPipeline)
	{
		for (auto i = 0; i < m_PipelineStateDesc.m_VertexInputDescCount; ++i)
		{
			auto& input = m_PipelineStateDesc.m_pVertexInputDescs[i];
			inputElems.PushBack({ input.m_pName, input.m_Index, DXGI_FORMAT(input.m_Format), input.m_Binding,
				input.m_Offset, GetD3D12InputClassification(input.m_InputRate), input.m_InstanceRate });
		}
		D3D12_INPUT_LAYOUT_DESC layoutDesc{ inputElems.Data(), u32(inputElems.Size()) };
		stream.InputLayout = layoutDesc;
		stream.PrimitiveTopologyType = GetD3D12PrimitiveTopologyType(m_PipelineStateDesc.m_InputAssemblyStateDesc.m_Topology);
		m_PrimitiveTopology = GetD3D12PrimitiveTopology(m_PipelineStateDesc.m_InputAssemblyStateDesc.m_Topology, m_PipelineStateDesc.m_PatchControlPoints);
		if (m_PipelineStateDesc.m_InputAssemblyStateDesc.m_PrimitiveRestart)
		{
			stream.IBStripCutValue = m_PipelineStateDesc.m_InputAssemblyStateDesc.m_IndexFormatForPrimitiveRestart == IndexFormat::kU32 ?
				D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF : D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;
		}
		else
		{
			stream.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
		}
	}

	D3D12_RT_FORMAT_ARRAY rtvFormats{};
	CD3DX12_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = m_PipelineStateDesc.m_BlendStateDesc.m_AlphaToCoverageEnable;
	for (auto i = 0; i < kMaxRenderTargets && m_PipelineStateDesc.m_RenderTargetFormats[i] != Format::kUnknown; i++)
	{
		rtvFormats.NumRenderTargets++;
		rtvFormats.RTFormats[i] = DXGI_FORMAT(m_PipelineStateDesc.m_RenderTargetFormats[i]);

		auto& dst = blendDesc.RenderTarget[i];
		auto& src = m_PipelineStateDesc.m_BlendStateDesc.m_BlendAttachments[i];
		dst.BlendEnable = src.m_BlendEnable;
		dst.LogicOpEnable = m_PipelineStateDesc.m_BlendStateDesc.m_LogicEnable;
		dst.SrcBlend = GetD3D12Blend(src.m_SrcBlend);
		dst.DestBlend = GetD3D12Blend(src.m_DstBlend);
		dst.BlendOp = GetD3D12BlendOp(src.m_BlendOp);
		dst.SrcBlendAlpha = GetD3D12Blend(src.m_SrcBlendAlpha);
		dst.DestBlendAlpha = GetD3D12Blend(src.m_DstBlendAlpha);
		dst.BlendOpAlpha = GetD3D12BlendOp(src.m_AlphaBlendOp);
		dst.LogicOp = GetD3D12LogicOp(m_PipelineStateDesc.m_BlendStateDesc.m_LogicOp);
		dst.RenderTargetWriteMask = src.m_RenderTargetWriteMask;
	}
	stream.BlendState = blendDesc;
	stream.RTVFormats = rtvFormats;
	stream.DSVFormat = DXGI_FORMAT(m_PipelineStateDesc.m_DepthStencilFormat);
	auto& depth = m_PipelineStateDesc.m_DepthStencilDesc;
	stream.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC1(depth.m_DepthTestEnable, depth.m_DepthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO,
		GetD3D12ComparisonFunc(depth.m_DepthOp), depth.m_StencilTestEnable, depth.m_StencilReadMask, depth.m_StencilWriteMask,
		GetD3D12StencilOp(depth.m_StencilFront.m_StencilFailOp), GetD3D12StencilOp(depth.m_StencilFront.m_StencilDepthFailOp),
		GetD3D12StencilOp(depth.m_StencilFront.m_StencilPassOp), GetD3D12ComparisonFunc(depth.m_StencilFront.m_StencilFunc),
		GetD3D12StencilOp(depth.m_StencilBack.m_StencilFailOp), GetD3D12StencilOp(depth.m_StencilBack.m_StencilDepthFailOp),
		GetD3D12StencilOp(depth.m_StencilBack.m_StencilPassOp), GetD3D12ComparisonFunc(depth.m_StencilBack.m_StencilFunc),
		m_PipelineStateDesc.m_DepthStencilDesc.m_DepthBoundsTestEnable);

	auto& raster = m_PipelineStateDesc.m_RasterizerStateDesc;
	stream.RasterizerState = CD3DX12_RASTERIZER_DESC(GetD3D12FillMode(raster.m_FillMode), GetD3D12CullMode(raster.m_CullMode), raster.m_FrontFace == FrontFace::kCounterClockwise,
		raster.m_DepthBias, raster.m_DepthBiasClamp, raster.m_DepthBiasSlope, raster.m_EnableDepthClip, m_PipelineStateDesc.m_SampleCount > 1, FALSE, 0,
		raster.m_EnableConservativeRasterization ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF);

	stream.SampleDesc = { m_PipelineStateDesc.m_SampleCount, 0 };
	stream.SampleMask = m_PipelineStateDesc.m_SampleMask;

	// TODO: Handle this
	//stream.CachedPSO;

	BvFixedVector<D3D12_VIEW_INSTANCE_LOCATION, D3D12_MAX_VIEW_INSTANCE_COUNT> viewLocations;
	if (m_PipelineStateDesc.m_MultiviewCount > 1)
	{
		BV_ASSERT(m_PipelineStateDesc.m_MultiviewCount <= D3D12_MAX_VIEW_INSTANCE_COUNT, "Multiview count must not be greater than D3D12_MAX_VIEW_INSTANCE_COUNT");
		for (auto i = 0u; i < m_PipelineStateDesc.m_MultiviewCount; ++i)
		{
			viewLocations.PushBack({ i, i });
		}
		stream.ViewInstancingDesc = CD3DX12_VIEW_INSTANCING_DESC(m_PipelineStateDesc.m_MultiviewCount, viewLocations.Data(),
			D3D12_VIEW_INSTANCING_FLAG_ENABLE_VIEW_INSTANCE_MASKING);
	}

	ComPtr<ID3D12Device2> device2;
	ComPtr<ID3D12Device> device(m_pDevice->GetHandle());
	auto hr = device.As(&device2);
	if (FAILED(hr))
	{
		// TODO: Handle Error
		return;
	}

	D3D12_PIPELINE_STATE_STREAM_DESC psd{ sizeof(stream), &stream };
	hr = device2->CreatePipelineState(&psd, IID_PPV_ARGS(&m_Pipeline));
	if (FAILED(hr))
	{
		// TODO: Handle Error
	}
}


void BvGraphicsPipelineStateD3D12::Destroy()
{
}