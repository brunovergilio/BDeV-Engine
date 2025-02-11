#pragma once


#include "BvRenderCommon.h"
#include "BvRenderDeviceObject.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Container/BvFixedVector.h"
#include "BDeV/Core/Container/BvString.h"


class BvShaderResourceLayout;
class BvShader;
class BvRenderPass;


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


BV_OBJECT_DEFINE_ID(BvGraphicsPipelineState, "863fd051-1ea0-4913-bd1c-2dd02f69589f");
class BvGraphicsPipelineState : public IBvRenderDeviceObject
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
BV_OBJECT_ENABLE_ID_OPERATOR(BvGraphicsPipelineState);


BV_OBJECT_DEFINE_ID(BvComputePipelineState, "3d4b6d6a-9cb4-42ff-bc14-ec6a328b2801");
class BvComputePipelineState : public IBvRenderDeviceObject
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
BV_OBJECT_ENABLE_ID_OPERATOR(BvComputePipelineState);