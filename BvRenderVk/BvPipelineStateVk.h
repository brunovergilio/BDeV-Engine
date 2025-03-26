#pragma once


#include "BDeV/Core/RenderAPI/BvPipelineState.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


//BV_OBJECT_DEFINE_ID(IBvGraphicsPipelineStateVk, "55186b73-74c0-418f-918f-89a434b5ba0e");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvGraphicsPipelineStateVk);


class BvGraphicsPipelineStateVk final : public IBvGraphicsPipelineState, public IBvResourceVk
{
	BV_VK_DEVICE_RES_DECL;

public:
	BvGraphicsPipelineStateVk(BvRenderDeviceVk* pDevice, const GraphicsPipelineStateDesc& pipelineStateDesc,
		const VkPipelineCache pipelineCache = VK_NULL_HANDLE);
	~BvGraphicsPipelineStateVk();

	BV_INLINE const GraphicsPipelineStateDesc& GetDesc() const override { return m_PipelineStateDesc; }
	BV_INLINE VkPipeline GetHandle() const { return m_Pipeline; }
	BV_INLINE bool IsValid() const { return m_Pipeline != VK_NULL_HANDLE; }

	//BV_OBJECT_IMPL_INTERFACE(IBvGraphicsPipelineStateVk, IBvGraphicsPipelineState, IBvRenderDeviceObject);

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkPipeline m_Pipeline = VK_NULL_HANDLE;
	VkPipelineCache m_PipelineCache = VK_NULL_HANDLE;
	GraphicsPipelineStateDesc m_PipelineStateDesc;
};


//BV_OBJECT_DEFINE_ID(IBvComputePipelineStateVk, "7cd60015-65f1-4258-a8db-fd093dd085c6");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvComputePipelineStateVk);


class BvComputePipelineStateVk final : public IBvComputePipelineState, public IBvResourceVk
{
	BV_VK_DEVICE_RES_DECL;

public:
	BvComputePipelineStateVk(BvRenderDeviceVk* pDevice, const ComputePipelineStateDesc& pipelineStateDesc,
		const VkPipelineCache pipelineCache);
	~BvComputePipelineStateVk();

	BV_INLINE const ComputePipelineStateDesc& GetDesc() const override { return m_PipelineStateDesc; }
	BV_INLINE VkPipeline GetHandle() const { return m_Pipeline; }
	BV_INLINE bool IsValid() const { return m_Pipeline != VK_NULL_HANDLE; }

	//BV_OBJECT_IMPL_INTERFACE(IBvComputePipelineStateVk, IBvComputePipelineState, IBvRenderDeviceObject);

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkPipeline m_Pipeline = VK_NULL_HANDLE;
	VkPipelineCache m_PipelineCache = VK_NULL_HANDLE;
	ComputePipelineStateDesc m_PipelineStateDesc;
};


//BV_OBJECT_DEFINE_ID(IBvRayTracingPipelineStateVk, "5ab21c28-09a4-4a86-a001-776e28fa3324");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvRayTracingPipelineStateVk);


class BvRayTracingPipelineStateVk final : public IBvRayTracingPipelineState, public IBvResourceVk
{
	BV_VK_DEVICE_RES_DECL;

public:
	BvRayTracingPipelineStateVk(BvRenderDeviceVk* pDevice, const RayTracingPipelineStateDesc& pipelineStateDesc,
		const VkPipelineCache pipelineCache = VK_NULL_HANDLE);
	~BvRayTracingPipelineStateVk();

	BV_INLINE const RayTracingPipelineStateDesc& GetDesc() const override { return m_PipelineStateDesc; }
	BV_INLINE VkPipeline GetHandle() const { return m_Pipeline; }
	BV_INLINE bool IsValid() const { return m_Pipeline != VK_NULL_HANDLE; }

	//BV_OBJECT_IMPL_INTERFACE(IBvRayTracingPipelineStateVk, IBvRayTracingPipelineState, IBvRenderDeviceObject);

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkPipeline m_Pipeline = VK_NULL_HANDLE;
	VkPipelineCache m_PipelineCache = VK_NULL_HANDLE;
	RayTracingPipelineStateDesc m_PipelineStateDesc;
};


BV_CREATE_CAST_TO_VK(BvGraphicsPipelineState)
BV_CREATE_CAST_TO_VK(BvComputePipelineState)
BV_CREATE_CAST_TO_VK(BvRayTracingPipelineState)