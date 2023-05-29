#include "BvPipelineStateGl.h"


BvGraphicsPipelineStateGl::BvGraphicsPipelineStateGl(const BvRenderDeviceGl& device, const GraphicsPipelineStateDesc& pipelineStateDesc)
	: BvGraphicsPipelineState(pipelineStateDesc), m_Device(device)
{
}


BvGraphicsPipelineStateGl::~BvGraphicsPipelineStateGl()
{
}


void BvGraphicsPipelineStateGl::SetStates()
{
}


BvComputePipelineStateGl::BvComputePipelineStateGl(const BvRenderDeviceGl& device, const ComputePipelineStateDesc& pipelineStateDesc)
	: BvComputePipelineState(pipelineStateDesc), m_Device(device)
{
}


BvComputePipelineStateGl::~BvComputePipelineStateGl()
{
}


void BvComputePipelineStateGl::SetStates()
{
}