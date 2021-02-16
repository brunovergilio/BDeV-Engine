#pragma once


#include "BvRenderVK/BvGPUInfoVk.h"
#include "BvRenderVk/BvCommandQueueVk.h"
#include "BvRender/BvRenderEngine.h"
#include "BvLoaderVk.h"


class BvRenderDeviceVk final : public BvRenderDevice
{
public:
	BvRenderDeviceVk(VkInstance instance, BvLoaderVk & loader, const BvGPUInfoVk & gpuInfo);
	~BvRenderDeviceVk();

	bool Create(const DeviceCreateDesc & deviceCreateDesc);
	void Destroy();

	BvSwapChain * CreateSwapChain(BvNativeWindow & window, const SwapChainDesc & swapChainDesc, BvCommandQueue & commandQueue) override final;
	BvSemaphore * CreateSemaphore(const u64 initialValue) override final;
	BvRenderPass * CreateRenderPass(const RenderPassDesc & renderPassDesc) override final;
	BvCommandPool * CreateCommandPool(const CommandPoolDesc & commandPoolDesc) override final;
	BvShaderResourceLayout * CreateShaderResourceLayout(const ShaderResourceLayoutDesc & shaderResourceLayoutDesc) override final;
	BvGraphicsPipelineState * CreateGraphicsPipeline(const GraphicsPipelineStateDesc & graphicsPipelineStateDesc) override final;
	void WaitIdle() const override final;

	const uint32_t GetMemoryTypeIndex(const uint32_t memoryTypeBits, const VkMemoryPropertyFlags properties) const;
	const VkFormat GetBestDepthFormat(const VkFormat format = VK_FORMAT_UNDEFINED) const;
	bool QueueFamilySupportsPresent(const QueueFamilyType queueFamilyType) const override final;

	BV_INLINE const BvVector<BvCommandQueue *> & GetGraphicsQueue(const u32 index = 0) const override final { return m_GraphicsQueues; }
	BV_INLINE const BvVector<BvCommandQueue *> & GetComputeQueue(const u32 index = 0) const override final { return m_ComputeQueues; }
	BV_INLINE const BvVector<BvCommandQueue *> & GetTransferQueue(const u32 index = 0) const override final { return m_TransferQueues; }

	BV_INLINE const VkDevice GetHandle() const { return m_Device; }
	BV_INLINE const VkInstance GetInstanceHandle() const { return m_Instance; }
	BV_INLINE const BvGPUInfoVk & GetGPUInfo() const { return m_GPUInfo; }

	BV_INLINE const VulkanFunctions::DeviceFunctions & GetDeviceFunctions() const { return m_Functions; }

private:
	VkInstance m_Instance = VK_NULL_HANDLE;
	VkDevice m_Device = VK_NULL_HANDLE;
	const BvGPUInfoVk & m_GPUInfo;
	BvVector<BvCommandQueue *> m_GraphicsQueues;
	BvVector<BvCommandQueue *> m_ComputeQueues;
	BvVector<BvCommandQueue *> m_TransferQueues;
	BvLoaderVk & m_Loader;
	VulkanFunctions::DeviceFunctions m_Functions;
};