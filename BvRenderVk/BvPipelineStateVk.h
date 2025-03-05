#pragma once


#include "BDeV/Core/RenderAPI/BvPipelineState.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


BV_OBJECT_DEFINE_ID(IBvGraphicsPipelineStateVk, "55186b73-74c0-418f-918f-89a434b5ba0e");
class IBvGraphicsPipelineStateVk : public IBvGraphicsPipelineState
{
public:
	virtual VkPipeline GetHandle() const = 0;
	virtual bool IsValid() const = 0;

protected:
	IBvGraphicsPipelineStateVk() {}
	~IBvGraphicsPipelineStateVk() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvGraphicsPipelineStateVk);


class BvGraphicsPipelineStateVk final : public IBvGraphicsPipelineStateVk
{
public:
	BvGraphicsPipelineStateVk(BvRenderDeviceVk* pDevice, const GraphicsPipelineStateDesc& pipelineStateDesc,
		const VkPipelineCache pipelineCache = VK_NULL_HANDLE);
	~BvGraphicsPipelineStateVk();

	IBvRenderDevice* GetDevice() override;
	BV_INLINE const GraphicsPipelineStateDesc& GetDesc() const override { return m_PipelineStateDesc; }
	BV_INLINE VkPipeline GetHandle() const override { return m_Pipeline; }
	BV_INLINE bool IsValid() const override { return m_Pipeline != VK_NULL_HANDLE; }

	BV_OBJECT_IMPL_INTERFACE(IBvGraphicsPipelineStateVk, IBvGraphicsPipelineState, IBvRenderDeviceObject);

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkPipeline m_Pipeline = VK_NULL_HANDLE;
	VkPipelineCache m_PipelineCache = VK_NULL_HANDLE;
	GraphicsPipelineStateDesc m_PipelineStateDesc;
};


BV_OBJECT_DEFINE_ID(IBvComputePipelineStateVk, "7cd60015-65f1-4258-a8db-fd093dd085c6");
class IBvComputePipelineStateVk : public IBvComputePipelineState
{
public:
	virtual VkPipeline GetHandle() const = 0;
	virtual bool IsValid() const = 0;

protected:
	IBvComputePipelineStateVk() {}
	~IBvComputePipelineStateVk() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvComputePipelineStateVk);


class BvComputePipelineStateVk final : public IBvComputePipelineStateVk
{
public:
	BvComputePipelineStateVk(BvRenderDeviceVk* pDevice, const ComputePipelineStateDesc& pipelineStateDesc,
		const VkPipelineCache pipelineCache);
	~BvComputePipelineStateVk();

	IBvRenderDevice* GetDevice() override;
	BV_INLINE const ComputePipelineStateDesc& GetDesc() const override { return m_PipelineStateDesc; }
	BV_INLINE VkPipeline GetHandle() const override { return m_Pipeline; }
	BV_INLINE bool IsValid() const override { return m_Pipeline != VK_NULL_HANDLE; }

	BV_OBJECT_IMPL_INTERFACE(IBvComputePipelineStateVk, IBvComputePipelineState, IBvRenderDeviceObject);

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkPipeline m_Pipeline = VK_NULL_HANDLE;
	VkPipelineCache m_PipelineCache = VK_NULL_HANDLE;
	ComputePipelineStateDesc m_PipelineStateDesc;
};


BV_OBJECT_DEFINE_ID(IBvRayTracingPipelineStateVk, "5ab21c28-09a4-4a86-a001-776e28fa3324");
class IBvRayTracingPipelineStateVk : public IBvRayTracingPipelineState
{
public:
	virtual VkPipeline GetHandle() const = 0;
	virtual bool IsValid() const = 0;

protected:
	IBvRayTracingPipelineStateVk() {}
	~IBvRayTracingPipelineStateVk() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvRayTracingPipelineStateVk);


class BvRayTracingPipelineStateVk final : public IBvRayTracingPipelineStateVk
{
public:
	BvRayTracingPipelineStateVk(BvRenderDeviceVk* pDevice, const RayTracingPipelineStateDesc& pipelineStateDesc,
		const VkPipelineCache pipelineCache = VK_NULL_HANDLE);
	~BvRayTracingPipelineStateVk();

	IBvRenderDevice* GetDevice() override;
	BV_INLINE const RayTracingPipelineStateDesc& GetDesc() const override { return m_PipelineStateDesc; }
	BV_INLINE VkPipeline GetHandle() const override { return m_Pipeline; }
	BV_INLINE bool IsValid() const override { return m_Pipeline != VK_NULL_HANDLE; }

	BV_OBJECT_IMPL_INTERFACE(IBvRayTracingPipelineStateVk, IBvRayTracingPipelineState, IBvRenderDeviceObject);

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkPipeline m_Pipeline = VK_NULL_HANDLE;
	VkPipelineCache m_PipelineCache = VK_NULL_HANDLE;
	RayTracingPipelineStateDesc m_PipelineStateDesc;
};


BV_CREATE_CAST_TO_VK(IBvGraphicsPipelineState)
BV_CREATE_CAST_TO_VK(IBvComputePipelineState)
BV_CREATE_CAST_TO_VK(IBvRayTracingPipelineState)