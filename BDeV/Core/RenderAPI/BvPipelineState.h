#pragma once


#include "BvShaderResource.h"
#include "BvRenderPass.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Container/BvFixedVector.h"
#include "BDeV/Core/Container/BvString.h"


struct BlendStateDesc
{
	BvFixedVector<BlendAttachmentStateDesc, kMaxRenderTargets> m_BlendAttachments;
	LogicOp m_LogicOp = LogicOp::kClear;
	bool m_LogicEnable = false;
	bool m_AlphaToCoverageEnable = false;
};


struct GraphicsPipelineStateDesc
{
	BvFixedVector<VertexInputDesc, kMaxVertexAttributes>		m_VertexInputDesc;
	BvFixedVector<ShaderByteCodeDesc, kMaxShaderStages>			m_ShaderStages;
	InputAssemblyStateDesc										m_InputAssemblyStateDesc;
	TessellationStateDesc										m_TessellationStateDesc;
	ViewportStateDesc											m_ViewportStateDesc;
	RasterizerStateDesc											m_RasterizerStateDesc;
	DepthStencilDesc											m_DepthStencilDesc;
	BlendStateDesc												m_BlendStateDesc;
	u32															m_SampleCount = 1;
	BvShaderResourceLayout*										m_pShaderResourceLayout = nullptr;
	BvRenderPass*												m_pRenderPass = nullptr;
	BvFixedVector<Format, kMaxRenderTargets>					m_RenderTargetFormats;
	Format														m_DepthStencilFormat = Format::kUnknown;
	u32															m_SampleMask = kMax<u32>;
	u32															m_SubpassIndex = 0;
};


struct ComputePipelineStateDesc
{
	ShaderByteCodeDesc			m_ShaderByteCodeDesc;
	BvShaderResourceLayout*		m_pShaderResourceLayout = nullptr;
};


class BvGraphicsPipelineState : public BvRenderDeviceChild
{
	BV_NOCOPYMOVE(BvGraphicsPipelineState);

public:
	BV_INLINE const GraphicsPipelineStateDesc & GetDesc() const { return m_PipelineStateDesc; }

protected:
	BvGraphicsPipelineState(const GraphicsPipelineStateDesc & pipelineStateDesc)
		: m_PipelineStateDesc(pipelineStateDesc) {}
	virtual ~BvGraphicsPipelineState() = 0 {}

protected:
	GraphicsPipelineStateDesc m_PipelineStateDesc;
};


class BvComputePipelineState : public BvRenderDeviceChild
{
	BV_NOCOPYMOVE(BvComputePipelineState);

public:
	BV_INLINE const ComputePipelineStateDesc & GetDesc() const { return m_PipelineStateDesc; }

protected:
	BvComputePipelineState(const ComputePipelineStateDesc & pipelineStateDesc)
		: m_PipelineStateDesc(pipelineStateDesc) {}
	virtual ~BvComputePipelineState() = 0 {}

protected:
	ComputePipelineStateDesc m_PipelineStateDesc;
};