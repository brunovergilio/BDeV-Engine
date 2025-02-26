#pragma once


#include "BDeV/Core/RenderAPI/BvPipelineState.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


BV_OBJECT_DEFINE_ID(BvGraphicsPipelineStateVk, "55186b73-74c0-418f-918f-89a434b5ba0e");
class BvGraphicsPipelineStateVk final : public BvGraphicsPipelineState
{
public:
	BvGraphicsPipelineStateVk(BvRenderDeviceVk* pDevice, const GraphicsPipelineStateDesc& pipelineStateDesc,
		const VkPipelineCache pipelineCache = VK_NULL_HANDLE);
	~BvGraphicsPipelineStateVk();

	BvRenderDevice* GetDevice() override;
	BV_INLINE VkPipeline GetHandle() const { return m_Pipeline; }
	BV_INLINE bool IsValid() const { return m_Pipeline != VK_NULL_HANDLE; }

	BV_OBJECT_IMPL_INTERFACE(BvGraphicsPipelineStateVk, BvGraphicsPipelineState, IBvRenderDeviceObject);

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkPipeline m_Pipeline = VK_NULL_HANDLE;
	VkPipelineCache m_PipelineCache = VK_NULL_HANDLE;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvGraphicsPipelineStateVk);


BV_OBJECT_DEFINE_ID(BvComputePipelineStateVk, "7cd60015-65f1-4258-a8db-fd093dd085c6");
class BvComputePipelineStateVk final : public BvComputePipelineState
{
public:
	BvComputePipelineStateVk(BvRenderDeviceVk* pDevice, const ComputePipelineStateDesc& pipelineStateDesc,
		const VkPipelineCache pipelineCache);
	~BvComputePipelineStateVk();

	BvRenderDevice* GetDevice() override;
	BV_INLINE VkPipeline GetHandle() const { return m_Pipeline; }
	BV_INLINE bool IsValid() const { return m_Pipeline != VK_NULL_HANDLE; }

	BV_OBJECT_IMPL_INTERFACE(BvComputePipelineStateVk, BvComputePipelineState, IBvRenderDeviceObject);

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkPipeline m_Pipeline = VK_NULL_HANDLE;
	VkPipelineCache m_PipelineCache = VK_NULL_HANDLE;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvComputePipelineStateVk);


BV_OBJECT_DEFINE_ID(BvRayTracingPipelineStateVk, "5ab21c28-09a4-4a86-a001-776e28fa3324");
class BvRayTracingPipelineStateVk final : public BvRayTracingPipelineState
{
public:
	BvRayTracingPipelineStateVk(BvRenderDeviceVk* pDevice, const RayTracingPipelineStateDesc& pipelineStateDesc,
		const VkPipelineCache pipelineCache = VK_NULL_HANDLE);
	~BvRayTracingPipelineStateVk();

	BvRenderDevice* GetDevice() override;
	BV_INLINE VkPipeline GetHandle() const { return m_Pipeline; }
	BV_INLINE bool IsValid() const { return m_Pipeline != VK_NULL_HANDLE; }

	BV_OBJECT_IMPL_INTERFACE(BvRayTracingPipelineStateVk, BvRayTracingPipelineState, IBvRenderDeviceObject);

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkPipeline m_Pipeline = VK_NULL_HANDLE;
	VkPipelineCache m_PipelineCache = VK_NULL_HANDLE;
	VkPipeline m_PipelineLibrary = VK_NULL_HANDLE;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvRayTracingPipelineStateVk);


BV_CREATE_CAST_TO_VK(BvGraphicsPipelineState)
BV_CREATE_CAST_TO_VK(BvComputePipelineState)
BV_CREATE_CAST_TO_VK(BvRayTracingPipelineState)