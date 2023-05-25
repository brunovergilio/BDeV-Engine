#pragma once


#include "BvRenderVK/BvGPUInfoVk.h"
#include "BvRenderVk/BvCommandQueueVk.h"
#include "BvRenderEngineVk.h"
#include "BvLoaderVk.h"
#include <vma/vk_mem_alloc.h>


class BvFramebufferManagerVk;
class BvRenderPassManagerVk;


class BvRenderDeviceVk final : public BvRenderDevice
{
public:
	BvRenderDeviceVk(BvRenderEngineVk* pEngine, const BvGPUInfoVk & gpuInfo, const DeviceCreateDesc& deviceDesc);
	~BvRenderDeviceVk();

	void SetupDeviceFeatures(VkDeviceCreateInfo& deviceCreateInfo, VkPhysicalDeviceFeatures& enabledFeatures, BvVector<const char*>& enabledExtensions);

	bool Create(const DeviceCreateDesc & deviceCreateDesc);
	void Destroy();

	BvSwapChain* CreateSwapChain(BvWindow* pWindow, const SwapChainDesc & swapChainDesc, BvCommandQueue & commandQueue) override final;
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
	BV_INLINE const VkInstance GetInstanceHandle() const { return m_pEngine->GetInstance(); }
	BV_INLINE const BvGPUInfoVk & GetGPUInfo() const { return m_GPUInfo; }
	BV_INLINE VmaAllocator GetAllocator() const { return m_VMA; }
	BV_INLINE BvRenderPassManagerVk* GetRenderPassManager() const { return m_pRenderPassManager; }
	BV_INLINE BvFramebufferManagerVk* GetFramebufferManager() const { return m_pFramebufferManager; }

private:
	void CreateVMA();
	void DestroyVMA();

private:
	BvRenderEngineVk* m_pEngine = nullptr;
	VkDevice m_Device = VK_NULL_HANDLE;
	const BvGPUInfoVk & m_GPUInfo;
	BvVector<BvCommandQueueVk*> m_GraphicsQueues;
	BvVector<BvCommandQueueVk*> m_ComputeQueues;
	BvVector<BvCommandQueueVk*> m_TransferQueues;
	BvRenderPassManagerVk* m_pRenderPassManager;
	BvFramebufferManagerVk* m_pFramebufferManager;
	BvRenderDeviceFactory* m_pFactory = nullptr;
	VmaAllocator m_VMA{};
};