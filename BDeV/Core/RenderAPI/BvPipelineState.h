#pragma once


#include "BvRenderCommon.h"
#include "BvRenderDeviceObject.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Container/BvFixedVector.h"
#include "BDeV/Core/Container/BvString.h"


class IBvShaderResourceLayout;
class IBvShader;
class IBvRenderPass;


struct GraphicsPipelineStateDesc
{
	u32															m_VertexInputDescCount = 0;
	VertexInputDesc*											m_pVertexInputDescs = nullptr;
	IBvShader*													m_Shaders[kMaxShaderStages]{};
	InputAssemblyStateDesc										m_InputAssemblyStateDesc;
	TessellationStateDesc										m_TessellationStateDesc;
	ViewportStateDesc											m_ViewportStateDesc;
	RasterizerStateDesc											m_RasterizerStateDesc;
	DepthStencilDesc											m_DepthStencilDesc;
	BlendStateDesc												m_BlendStateDesc;
	u32															m_SampleCount = 1;
	IBvShaderResourceLayout*										m_pShaderResourceLayout = nullptr;
	IBvRenderPass*												m_pRenderPass = nullptr;
	Format														m_RenderTargetFormats[kMaxRenderTargets]{};
	Format														m_DepthStencilFormat = Format::kUnknown;
	bool														m_ShadingRateEnabled = false;
	u32															m_SampleMask = kMax<u32>;
	u32															m_SubpassIndex = 0;
};


struct ComputePipelineStateDesc
{
	IBvShader*					m_pShader = nullptr;
	IBvShaderResourceLayout*		m_pShaderResourceLayout = nullptr;
};


BV_OBJECT_DEFINE_ID(IBvGraphicsPipelineState, "863fd051-1ea0-4913-bd1c-2dd02f69589f");
class IBvGraphicsPipelineState : public IBvRenderDeviceObject
{
	BV_NOCOPYMOVE(IBvGraphicsPipelineState);

public:
	virtual const GraphicsPipelineStateDesc& GetDesc() const = 0;

protected:
	IBvGraphicsPipelineState() {}
	~IBvGraphicsPipelineState() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvGraphicsPipelineState);


BV_OBJECT_DEFINE_ID(IBvComputePipelineState, "3d4b6d6a-9cb4-42ff-bc14-ec6a328b2801");
class IBvComputePipelineState : public IBvRenderDeviceObject
{
	BV_NOCOPYMOVE(IBvComputePipelineState);

public:
	virtual const ComputePipelineStateDesc& GetDesc() const = 0;

protected:
	IBvComputePipelineState() {}
	~IBvComputePipelineState() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvComputePipelineState);


BV_OBJECT_DEFINE_ID(IBvRayTracingPipelineState, "e04f4570-bb75-4ff2-b9e2-02ba4affb9de");
class IBvRayTracingPipelineState : public IBvRenderDeviceObject
{
	BV_NOCOPYMOVE(IBvRayTracingPipelineState);

public:
	virtual const RayTracingPipelineStateDesc& GetDesc() const = 0;

protected:
	IBvRayTracingPipelineState() {}
	~IBvRayTracingPipelineState() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvRayTracingPipelineState);