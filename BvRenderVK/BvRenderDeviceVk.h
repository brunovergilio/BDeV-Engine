#pragma once


#include "BvRenderVK/BvGPUInfoVk.h"
#include "BvRenderVk/BvCommandQueueVk.h"
#include "BvRender/BvRenderEngine.h"
#include "BvLoaderVk.h"


class BvFramebufferManager;


class BvRenderDeviceVk final : public BvRenderDevice
{
public:
	BvRenderDeviceVk(VkInstance instance, BvLoaderVk & loader, const BvGPUInfoVk & gpuInfo);
	~BvRenderDeviceVk();

	bool Create(const DeviceCreateDesc & deviceCreateDesc);
	void Destroy();

	BvSwapChain* CreateSwapChain(const SwapChainDesc & swapChainDesc, BvCommandQueue & commandQueue) override final;
	BvBuffer* CreateBuffer(const BufferDesc& desc) override final;
	BvBufferView* CreateBufferView(const BufferViewDesc& desc)  override final;
	BvTexture* CreateTexture(const TextureDesc& desc)  override final;
	BvTextureView* CreateTextureView(const TextureViewDesc& desc)  override final;
	BvSemaphore* CreateSemaphore(const u64 initialValue) override final;
	BvRenderPass* CreateRenderPass(const RenderPassDesc & renderPassDesc) override final;
	BvCommandPool* CreateCommandPool(const CommandPoolDesc & commandPoolDesc) override final;
	BvShaderResourceLayout* CreateShaderResourceLayout(const ShaderResourceLayoutDesc & shaderResourceLayoutDesc) override final;
	BvShaderResourceSetPool* CreateShaderResourceSetPool(const ShaderResourceSetPoolDesc& shaderResourceSetPoolDesc =
		ShaderResourceSetPoolDesc()) override final;
	BvGraphicsPipelineState* CreateGraphicsPipeline(const GraphicsPipelineStateDesc & graphicsPipelineStateDesc) override final;
	
	void WaitIdle() const override final;

	const u32 GetMemoryTypeIndex(const u32 memoryTypeBits, const VkMemoryPropertyFlags properties) const;
	const VkFormat GetBestDepthFormat(const VkFormat format = VK_FORMAT_UNDEFINED) const;
	bool QueueFamilySupportsPresent(const QueueFamilyType queueFamilyType) const override final;

	BV_INLINE BvCommandQueue* GetGraphicsQueue(const u32 index = 0) const override final { return m_GraphicsQueues[index]; }
	BV_INLINE BvCommandQueue* GetComputeQueue(const u32 index = 0) const override final { return m_ComputeQueues[index]; }
	BV_INLINE BvCommandQueue* GetTransferQueue(const u32 index = 0) const override final { return m_TransferQueues[index]; }

	BV_INLINE const VkDevice GetHandle() const { return m_Device; }
	BV_INLINE const VkInstance GetInstanceHandle() const { return m_Instance; }
	BV_INLINE const BvGPUInfoVk & GetGPUInfo() const { return m_GPUInfo; }

	BV_INLINE BvFramebufferManager* GetFramebufferManager() const { return m_pFramebufferManager; }

	BV_INLINE const VulkanFunctions::DeviceFunctions & GetDeviceFunctions() const { return m_Functions; }

private:
	VkInstance m_Instance = VK_NULL_HANDLE;
	VkDevice m_Device = VK_NULL_HANDLE;
	const BvGPUInfoVk & m_GPUInfo;
	BvVector<BvCommandQueueVk*> m_GraphicsQueues;
	BvVector<BvCommandQueueVk*> m_ComputeQueues;
	BvVector<BvCommandQueueVk*> m_TransferQueues;
	BvRenderDeviceFactory* m_pFactory = nullptr;
	BvFramebufferManager* m_pFramebufferManager = nullptr;
	BvLoaderVk & m_Loader;
	VulkanFunctions::DeviceFunctions m_Functions{};
};