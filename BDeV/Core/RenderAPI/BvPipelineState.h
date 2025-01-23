#pragma once


#include "BvShaderResource.h"
#include "BvRenderPass.h"
#include "BvShader.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Container/BvFixedVector.h"
#include "BDeV/Core/Container/BvString.h"


struct GraphicsPipelineStateDesc
{
	u32															m_VertexInputDescCount = 0;
	VertexInputDesc*											m_pVertexInputDescs = nullptr;
	BvShader*													m_Shaders[kMaxShaderStages]{};
	InputAssemblyStateDesc										m_InputAssemblyStateDesc;
	TessellationStateDesc										m_TessellationStateDesc;
	ViewportStateDesc											m_ViewportStateDesc;
	RasterizerStateDesc											m_RasterizerStateDesc;
	DepthStencilDesc											m_DepthStencilDesc;
	BlendStateDesc												m_BlendStateDesc;
	u32															m_SampleCount = 1;
	BvShaderResourceLayout*										m_pShaderResourceLayout = nullptr;
	BvRenderPass*												m_pRenderPass = nullptr;
	Format														m_RenderTargetFormats[kMaxRenderTargets]{};
	Format														m_DepthStencilFormat = Format::kUnknown;
	bool														m_ShadingRateEnabled = false;
	u32															m_SampleMask = kMax<u32>;
	u32															m_SubpassIndex = 0;
};


struct ComputePipelineStateDesc
{
	BvShader*					m_pShader = nullptr;
	BvShaderResourceLayout*		m_pShaderResourceLayout = nullptr;
};


class BvGraphicsPipelineState : public IBvRenderDeviceChild
{
	BV_NOCOPYMOVE(BvGraphicsPipelineState);

public:
	BV_INLINE const GraphicsPipelineStateDesc& GetDesc() const { return m_PipelineStateDesc; }

protected:
	BvGraphicsPipelineState(const GraphicsPipelineStateDesc & pipelineStateDesc)
		: m_PipelineStateDesc(pipelineStateDesc) {}
	virtual ~BvGraphicsPipelineState() = 0 {}

protected:
	GraphicsPipelineStateDesc m_PipelineStateDesc;
};


class BvComputePipelineState : public IBvRenderDeviceChild
{
	BV_NOCOPYMOVE(BvComputePipelineState);

public:
	BV_INLINE const ComputePipelineStateDesc& GetDesc() const { return m_PipelineStateDesc; }

protected:
	BvComputePipelineState(const ComputePipelineStateDesc & pipelineStateDesc)
		: m_PipelineStateDesc(pipelineStateDesc) {}
	virtual ~BvComputePipelineState() = 0 {}

protected:
	ComputePipelineStateDesc m_PipelineStateDesc;
};