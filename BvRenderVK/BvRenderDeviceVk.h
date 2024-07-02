#pragma once


#include "BvRenderVK/BvGPUInfoVk.h"
#include "BvRenderEngineVk.h"


class BvFramebufferManagerVk;
class BvCommandContextVk;
class BvQueryHeapManagerVk;


class BvRenderDeviceVk final : public BvRenderDevice
{
public:
	BvRenderDeviceVk(BvRenderEngineVk* pEngine, BvGPUInfoVk& gpuInfo, const BvRenderDeviceCreateDescVk& deviceDesc);
	~BvRenderDeviceVk();

	void Create(const BvRenderDeviceCreateDescVk& deviceCreateDesc);
	void Destroy();

	BvSwapChain* CreateSwapChain(BvWindow* pWindow, const SwapChainDesc& swapChainDesc, BvCommandContext* pContext) override;
	BvBuffer* CreateBuffer(const BufferDesc& desc, const BufferInitData* pInitData = nullptr) override;
	BvBufferView* CreateBufferView(const BufferViewDesc& desc) override;
	BvTexture* CreateTexture(const TextureDesc& desc, const TextureInitData* pInitData = nullptr) override;
	BvTextureView* CreateTextureView(const TextureViewDesc& desc) override;
	BvSampler* CreateSampler(const SamplerDesc& desc) override;
	BvRenderPass* CreateRenderPass(const RenderPassDesc & renderPassDesc) override;
	BvShaderResourceLayout* CreateShaderResourceLayout(u32 shaderResourceCount,	const ShaderResourceDesc* pShaderResourceDescs,
		const ShaderResourceConstantDesc& shaderResourceConstantDesc) override;
	BvGraphicsPipelineState* CreateGraphicsPipeline(const GraphicsPipelineStateDesc & graphicsPipelineStateDesc) override;
	BvQuery* CreateQuery(QueryType queryType) override;

	void WaitIdle() const override;

	const u32 GetMemoryTypeIndex(const u32 memoryTypeBits, const VkMemoryPropertyFlags properties) const;
	const VkFormat GetBestDepthFormat(const VkFormat format = VK_FORMAT_UNDEFINED) const;
	bool HasFormatSupport(Format format);

	BvCommandContext* GetGraphicsContext(u32 index = 0) const override;
	BvCommandContext* GetComputeContext(u32 index = 0) const override;
	BvCommandContext* GetTransferContext(u32 index = 0) const override;
	void GetCopyableFootprints(const TextureDesc& textureDesc, u32 subresourceCount, SubresourceFootprint* pSubresources, u64* pTotalSize) const override;
	bool SupportsQueryType(QueryType queryType, QueueFamilyType commandType) const override;

	BV_INLINE const VkDevice GetHandle() const { return m_Device; }
	BV_INLINE const VkInstance GetInstanceHandle() const { return m_pEngine->GetInstance(); }
	BV_INLINE const BvGPUInfoVk& GetGPUInfo() const { return m_GPUInfo; }
	BV_INLINE VmaAllocator GetAllocator() const { return m_VMA; }
	BV_INLINE BvFramebufferManagerVk* GetFramebufferManager() const { return m_pFramebufferManager; }
	BV_INLINE BvQueryHeapManagerVk* GetQueryHeapManager() const { return m_pQueryHeapManager; }

private:
	void CreateVMA();
	void DestroyVMA();

private:
	BvRenderEngineVk* m_pEngine = nullptr;
	VkDevice m_Device = VK_NULL_HANDLE;
	BvGPUInfoVk& m_GPUInfo;
	BvVector<BvCommandContextVk*> m_GraphicsContexts;
	BvVector<BvCommandContextVk*> m_ComputeContexts;
	BvVector<BvCommandContextVk*> m_TransferContexts;
	BvFramebufferManagerVk* m_pFramebufferManager;
	BvQueryHeapManagerVk* m_pQueryHeapManager;
	BvRenderDeviceFactory* m_pFactory = nullptr;
	VmaAllocator m_VMA{};
};


BV_CREATE_CAST_TO_VK(BvRenderDevice)