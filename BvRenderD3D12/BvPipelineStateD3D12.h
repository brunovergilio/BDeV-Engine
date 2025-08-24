#pragma once


#include "BDeV/Core/RenderAPI/BvPipelineState.h"
#include "BvCommonD3D12.h"


class BvRenderDeviceD3D12;


class BvGraphicsPipelineStateD3D12 : public IBvGraphicsPipelineState, public IBvResourceD3D12
{
	BV_D3D12_DEVICE_RES_DECL;

public:
	BvGraphicsPipelineStateD3D12(BvRenderDeviceD3D12* pDevice, const GraphicsPipelineStateDesc& pipelineStateDesc);
	~BvGraphicsPipelineStateD3D12();

	BV_INLINE const GraphicsPipelineStateDesc& GetDesc() const override { return m_PipelineStateDesc; }
	BV_INLINE ID3D12PipelineState* GetHandle() const { return m_Pipeline.Get(); }
	BV_INLINE bool IsValid() const { return m_Pipeline != nullptr; }
	BV_INLINE D3D_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const { return m_PrimitiveTopology; }

	//BV_OBJECT_IMPL_INTERFACE(IBvGraphicsPipelineStateD3D12, IBvGraphicsPipelineState, IBvRenderDeviceObject);

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	ComPtr<ID3D12PipelineState> m_Pipeline;
	GraphicsPipelineStateDesc m_PipelineStateDesc;
	D3D_PRIMITIVE_TOPOLOGY m_PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
};


BV_CREATE_CAST_TO_D3D12(BvGraphicsPipelineState)