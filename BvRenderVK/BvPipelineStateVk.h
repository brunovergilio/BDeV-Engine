#pragma once


#include "BDeV/Core/RenderAPI/BvPipelineState.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


class BvGraphicsPipelineStateVk final : public BvGraphicsPipelineState
{
public:
	BvGraphicsPipelineStateVk(BvRenderDeviceVk* pDevice, const GraphicsPipelineStateDesc& pipelineStateDesc,
		const VkPipelineCache pipelineCache = VK_NULL_HANDLE);
	~BvGraphicsPipelineStateVk();

	BvRenderDevice* GetDevice() override;
	BV_INLINE VkPipeline GetHandle() const { return m_Pipeline; }

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkPipeline m_Pipeline = VK_NULL_HANDLE;
	VkPipelineCache m_PipelineCache = VK_NULL_HANDLE;
};


class BvComputePipelineStateVk final : public BvComputePipelineState
{
public:
	BvComputePipelineStateVk(BvRenderDeviceVk* pDevice, const ComputePipelineStateDesc& pipelineStateDesc,
		const VkPipelineCache pipelineCache);
	~BvComputePipelineStateVk();

	BvRenderDevice* GetDevice() override;
	BV_INLINE VkPipeline GetHandle() const { return m_Pipeline; }

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkPipeline m_Pipeline = VK_NULL_HANDLE;
	VkPipelineCache m_PipelineCache = VK_NULL_HANDLE;
};


BV_CREATE_CAST_TO_VK(BvGraphicsPipelineState)
BV_CREATE_CAST_TO_VK(BvComputePipelineState)