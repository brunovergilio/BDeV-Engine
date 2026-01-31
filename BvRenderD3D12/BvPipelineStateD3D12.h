#pragma once


#include "BDeV/Core/RenderAPI/BvPipelineState.h"
#include "BvCommonD3D12.h"


class BvRenderDeviceD3D12;


class BvGraphicsPipelineStateD3D12 : public IBvGraphicsPipelineState, public IBvResourceD3D12
{
public:
	BvGraphicsPipelineStateD3D12(BvRenderDeviceD3D12* pDevice, const GraphicsPipelineStateDesc& pipelineStateDesc, ComPtr<ID3D12PipelineState>& pipeline,
	ComPtr<ID3D12RootSignature>& rootSig, D3D_PRIMITIVE_TOPOLOGY primitiveTopology);
	~BvGraphicsPipelineStateD3D12();

	BV_INLINE const GraphicsPipelineStateDesc& GetDesc() const override { return m_PipelineStateDesc; }
	BV_INLINE ID3D12PipelineState* GetHandle() const { return m_Pipeline.Get(); }
	BV_INLINE ID3D12RootSignature* GetRootSig() const { return m_RootSig.Get(); };
	BV_INLINE bool IsValid() const { return m_Pipeline != nullptr; }
	BV_INLINE D3D_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const { return m_PrimitiveTopology; }

private:
	void Destroy();

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	ComPtr<ID3D12PipelineState> m_Pipeline;
	ComPtr<ID3D12RootSignature> m_RootSig;
	GraphicsPipelineStateDesc m_PipelineStateDesc;
	D3D_PRIMITIVE_TOPOLOGY m_PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
};
BV_CREATE_CAST_TO_D3D12(BvGraphicsPipelineState)


class BvComputePipelineStateD3D12 : public IBvComputePipelineState, public IBvResourceD3D12
{
public:
	BvComputePipelineStateD3D12(BvRenderDeviceD3D12* pDevice, const ComputePipelineStateDesc& pipelineStateDesc, ComPtr<ID3D12PipelineState>& pipeline,
		ComPtr<ID3D12RootSignature>& rootSig);
	~BvComputePipelineStateD3D12();

	BV_INLINE const ComputePipelineStateDesc& GetDesc() const override { return m_PipelineStateDesc; }
	BV_INLINE ID3D12PipelineState* GetHandle() const { return m_Pipeline.Get(); }
	BV_INLINE ID3D12RootSignature* GetRootSig() const { return m_RootSig.Get(); };
	BV_INLINE bool IsValid() const { return m_Pipeline != nullptr; }

private:
	void Destroy();

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	ComPtr<ID3D12PipelineState> m_Pipeline;
	ComPtr<ID3D12RootSignature> m_RootSig;
	ComputePipelineStateDesc m_PipelineStateDesc;
};
BV_CREATE_CAST_TO_D3D12(BvComputePipelineState)


class BvPipelineCacheD3D12 final : public IBvPipelineCache
{
public:
	BvPipelineCacheD3D12(BvRenderDeviceD3D12* pDevice, ComPtr<ID3D12PipelineLibrary1>& library);
	~BvPipelineCacheD3D12();

	void GetCachedData(u64& size, void* pData) override;

	BV_INLINE auto GetHandle() const { return m_Library.Get(); }
	BV_INLINE auto operator*() const { return m_Library.Get(); }

private:
	void Destroy();

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	ComPtr<ID3D12PipelineLibrary1> m_Library;
};
BV_CREATE_CAST_TO_D3D12(BvPipelineCache)