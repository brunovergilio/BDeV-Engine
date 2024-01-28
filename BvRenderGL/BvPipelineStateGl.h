#pragma once


#include "BDeV/RenderAPI/BvPipelineState.h"
#include "BvCommonGl.h"


class BvRenderDeviceGl;


class BvGraphicsPipelineStateGl final : public BvGraphicsPipelineState
{
public:
	BvGraphicsPipelineStateGl(const BvRenderDeviceGl& device, const GraphicsPipelineStateDesc& pipelineStateDesc);
	~BvGraphicsPipelineStateGl();

	void SetStates();

private:
	const BvRenderDeviceGl& m_Device;
};


class BvComputePipelineStateGl final : public BvComputePipelineState
{
public:
	BvComputePipelineStateGl(const BvRenderDeviceGl& device, const ComputePipelineStateDesc& pipelineStateDesc);
	~BvComputePipelineStateGl();

	void SetStates();

private:
	const BvRenderDeviceGl& m_Device;
};