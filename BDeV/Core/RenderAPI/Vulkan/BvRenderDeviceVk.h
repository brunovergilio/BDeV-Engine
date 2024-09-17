#pragma once


#include "BvGPUInfoVk.h"
#include "BvRenderEngineVk.h"


class BvSwapChainVk;
class BvBufferVk;
class BvBufferViewVk;
class BvTextureVk;
class BvTextureViewVk;
class BvSamplerVk;
class BvRenderPassVk;
class BvShaderResourceLayoutVk;
class BvShaderVk;
class BvGraphicsPipelineStateVk;
class BvComputePipelineStateVk;
class BvQueryVk;
class BvFramebufferManagerVk;
class BvCommandContextVk;
class BvQueryHeapManagerVk;


class BvRenderDeviceVk final : public BvRenderDevice
{
public:
	BvRenderDeviceVk(BvRenderEngineVk* pEngine, BvGPUInfoVk& gpuInfo, const BvRenderDeviceCreateDescVk& deviceDesc);
	~BvRenderDeviceVk();

	BvSwapChain* CreateSwapChain(BvWindow* pWindow, const SwapChainDesc& swapChainDesc, BvCommandContext* pContext) override;
	BvBuffer* CreateBuffer(const BufferDesc& desc, const BufferInitData* pInitData = nullptr) override;
	BvBufferView* CreateBufferView(const BufferViewDesc& desc) override;
	BvTexture* CreateTexture(const TextureDesc& desc, const TextureInitData* pInitData = nullptr) override;
	BvTextureView* CreateTextureView(const TextureViewDesc& desc) override;
	BvSampler* CreateSampler(const SamplerDesc& desc) override;
	BvRenderPass* CreateRenderPass(const RenderPassDesc & renderPassDesc) override;
	BvShaderResourceLayout* CreateShaderResourceLayout(u32 shaderResourceCount,	const ShaderResourceDesc* pShaderResourceDescs,
		const ShaderResourceConstantDesc& shaderResourceConstantDesc) override;
	BvShader* CreateShader(const ShaderCreateDesc& shaderDesc) override;
	BvGraphicsPipelineState* CreateGraphicsPipeline(const GraphicsPipelineStateDesc & graphicsPipelineStateDesc) override;
	BvComputePipelineState* CreateComputePipeline(const ComputePipelineStateDesc& computePipelineStateDesc) override;
	BvQuery* CreateQuery(QueryType queryType) override;

	BvSwapChainVk* CreateSwapChainVk(BvWindow* pWindow, const SwapChainDesc& swapChainDesc, BvCommandContext* pContext);
	BvBufferVk* CreateBufferVk(const BufferDesc& desc, const BufferInitData* pInitData = nullptr);
	BvBufferViewVk* CreateBufferViewVk(const BufferViewDesc& desc);
	BvTextureVk* CreateTextureVk(const TextureDesc& desc, const TextureInitData* pInitData = nullptr);
	BvTextureViewVk* CreateTextureViewVk(const TextureViewDesc& desc);
	BvSamplerVk* CreateSamplerVk(const SamplerDesc& desc);
	BvRenderPassVk* CreateRenderPassVk(const RenderPassDesc& renderPassDesc);
	BvShaderResourceLayoutVk* CreateShaderResourceLayoutVk(u32 shaderResourceCount, const ShaderResourceDesc* pShaderResourceDescs,
		const ShaderResourceConstantDesc& shaderResourceConstantDesc);
	BvShaderVk* CreateShaderVk(const ShaderCreateDesc& shaderDesc);
	BvGraphicsPipelineStateVk* CreateGraphicsPipelineVk(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc);
	BvComputePipelineStateVk* CreateComputePipelineVk(const ComputePipelineStateDesc& computePipelineStateDesc);
	BvQueryVk* CreateQueryVk(QueryType queryType);

	void Release(IBvRenderDeviceChild* pDeviceObj) override;

	void WaitIdle() const override;

	const u32 GetMemoryTypeIndex(const u32 memoryTypeBits, const VkMemoryPropertyFlags properties) const;
	const VkFormat GetBestDepthFormat(const VkFormat format = VK_FORMAT_UNDEFINED) const;
	bool HasFormatSupport(Format format);

	BvCommandContext* GetGraphicsContext(u32 index = 0) const override;
	BvCommandContext* GetComputeContext(u32 index = 0) const override;
	BvCommandContext* GetTransferContext(u32 index = 0) const override;
	
	void GetCopyableFootprints(const TextureDesc& textureDesc, u32 subresourceCount, SubresourceFootprint* pSubresources, u64* pTotalSize) const override;
	bool SupportsQueryType(QueryType queryType, QueueFamilyType commandType) const override;
	bool IsFormatSupported(Format format) const override;

	BV_INLINE const VkDevice GetHandle() const { return m_Device; }
	BV_INLINE VkInstance GetInstanceHandle() const { return m_pEngine->GetHandle(); }
	BV_INLINE const BvGPUInfoVk& GetGPUInfo() const { return m_GPUInfo; }
	BV_INLINE VmaAllocator GetAllocator() const { return m_VMA; }
	BV_INLINE BvFramebufferManagerVk* GetFramebufferManager() const { return m_pFramebufferManager; }
	BV_INLINE BvQueryHeapManagerVk* GetQueryHeapManager() const { return m_pQueryHeapManager; }

private:
	void Create(const BvRenderDeviceCreateDescVk& deviceCreateDesc);
	void Destroy();

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
	BvVector<IBvRenderDeviceChild*> m_DeviceObjects;
	VmaAllocator m_VMA{};
};


BV_CREATE_CAST_TO_VK(BvRenderDevice)