#pragma once


#include "BvRender/BvPipelineState.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


class BvGraphicsPipelineStateVk final : public BvGraphicsPipelineState
{
public:
	BvGraphicsPipelineStateVk(const BvRenderDeviceVk & device, const GraphicsPipelineStateDesc & pipelineStateDesc,
		const VkPipelineCache pipelineCache = VK_NULL_HANDLE);
	~BvGraphicsPipelineStateVk();

	void Create();
	void Destroy();

	BV_INLINE VkPipeline GetHandle() const { return m_Pipeline; }

private:
	const BvRenderDeviceVk & m_Device;
	VkPipeline m_Pipeline = VK_NULL_HANDLE;
	VkPipelineCache m_PipelineCache = VK_NULL_HANDLE;
};


class BvComputePipelineStateVk final : public BvComputePipelineState
{
public:
	BvComputePipelineStateVk(const BvRenderDeviceVk & device, const ComputePipelineStateDesc & pipelineStateDesc,
		const VkPipelineCache pipelineCache);
	~BvComputePipelineStateVk();

	void Create();
	void Destroy();

	BV_INLINE VkPipeline GetHandle() const { return m_Pipeline; }

private:
	const BvRenderDeviceVk & m_Device;
	VkPipeline m_Pipeline = VK_NULL_HANDLE;
	VkPipelineCache m_PipelineCache = VK_NULL_HANDLE;
};