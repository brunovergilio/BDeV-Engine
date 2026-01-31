#include "BvPipelineStateD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BvShaderResourceD3D12.h"
#include "BvTypeConversionsD3D12.h"
#include "BDeV/Core/RenderAPI/BvRenderAPIUtils.h"
#include "BDeV/Core/RenderAPI/BvShader.h"
#include "BDeV/Core/RenderAPI/BvSampler.h"


BvGraphicsPipelineStateD3D12::BvGraphicsPipelineStateD3D12(BvRenderDeviceD3D12* pDevice, const GraphicsPipelineStateDesc& pipelineStateDesc,
	ComPtr<ID3D12PipelineState>& pipeline, ComPtr<ID3D12RootSignature>& rootSig, D3D_PRIMITIVE_TOPOLOGY primitiveTopology)
	: m_pDevice(pDevice), m_PipelineStateDesc(pipelineStateDesc), m_Pipeline(std::move(pipeline)), m_RootSig(std::move(rootSig)),
	m_PrimitiveTopology(primitiveTopology)
{
}


BvGraphicsPipelineStateD3D12::~BvGraphicsPipelineStateD3D12()
{
	Destroy();
}


void BvGraphicsPipelineStateD3D12::Destroy()
{
}


BvComputePipelineStateD3D12::BvComputePipelineStateD3D12(BvRenderDeviceD3D12* pDevice, const ComputePipelineStateDesc& pipelineStateDesc,
	ComPtr<ID3D12PipelineState>& pipeline, ComPtr<ID3D12RootSignature>& rootSig)
	: m_pDevice(pDevice), m_PipelineStateDesc(pipelineStateDesc), m_Pipeline(std::move(pipeline)), m_RootSig(std::move(rootSig))
{
}


BvComputePipelineStateD3D12::~BvComputePipelineStateD3D12()
{
	Destroy();
}


void BvComputePipelineStateD3D12::Destroy()
{
}


BvPipelineCacheD3D12::BvPipelineCacheD3D12(BvRenderDeviceD3D12* pDevice, ComPtr<ID3D12PipelineLibrary1>& library)
	: m_pDevice(pDevice), m_Library(std::move(library))
{
}


BvPipelineCacheD3D12::~BvPipelineCacheD3D12()
{
}


void BvPipelineCacheD3D12::GetCachedData(u64& size, void* pData)
{
	size = m_Library->GetSerializedSize();
	if (pData)
	{
		if (FAILED(m_Library->Serialize(pData, size)))
		{
			// TODO: Handle error
		}
	}
}


void BvPipelineCacheD3D12::Destroy()
{
}