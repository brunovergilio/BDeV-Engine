#pragma once


#include "BDeV/RenderAPI/BvRenderDevice.h"
#include "BDeV/RenderAPI/BvRenderEngine.h"
#include "BvGPUInfoGl.h"


class BvRenderEngineGl;
class BvRenderContextStateGl;
class BvFramebufferManagerGl;


class BvRenderDeviceGl final : public BvRenderDevice
{
public:
	BvRenderDeviceGl(const DeviceCreateDesc& deviceDesc, const BvGPUInfoGl& gpuInfo);
	~BvRenderDeviceGl();

	void Create(const DeviceCreateDesc& deviceCreateDesc);
	void Destroy();

	BvSwapChain* CreateSwapChain(BvWindow* pWindow, const SwapChainDesc& swapChainDesc, BvCommandQueue& commandQueue) override final;
	BvBuffer* CreateBuffer(const BufferDesc& desc) override final;
	BvBufferView* CreateBufferView(const BufferViewDesc& desc)  override final;
	BvTexture* CreateTexture(const TextureDesc& desc)  override final;
	BvTextureView* CreateTextureView(const TextureViewDesc& desc)  override final;
	BvSemaphore* CreateSemaphore(const u64 initialValue) override final;
	BvRenderPass* CreateRenderPass(const RenderPassDesc& renderPassDesc) override final;
	BvCommandPool* CreateCommandPool(const CommandPoolDesc& commandPoolDesc) override final;
	BvShaderResourceLayout* CreateShaderResourceLayout(const ShaderResourceLayoutDesc& shaderResourceLayoutDesc) override final;
	BvShaderResourceSetPool* CreateShaderResourceSetPool(const ShaderResourceSetPoolDesc& shaderResourceSetPoolDesc =
		ShaderResourceSetPoolDesc()) override final;
	BvGraphicsPipelineState* CreateGraphicsPipeline(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc) override final;

	void WaitIdle() const override final;

	BV_INLINE BvCommandQueue* GetGraphicsQueue(const u32 index = 0) const override final { return nullptr; }
	BV_INLINE BvCommandQueue* GetComputeQueue(const u32 index = 0) const override final { return nullptr; }
	BV_INLINE BvCommandQueue* GetTransferQueue(const u32 index = 0) const override final { return nullptr; }
	bool QueueFamilySupportsPresent(const QueueFamilyType queueFamilyType) const override final { return true; }

	BV_INLINE const BvGPUInfoGl& GetGPUInfo() const { return m_GPUInfo; }
	BV_INLINE BvFramebufferManagerGl* GetFramebufferManager() const { return m_pFramebufferManager; }
	BV_INLINE BvRenderContextStateGl* GetRenderContextState() const { return m_pContextState; }

private:
	BvRenderEngineGl* m_pEngine = nullptr;
	BvFramebufferManagerGl* m_pFramebufferManager = nullptr;
	BvRenderContextStateGl* m_pContextState = nullptr;
	BvRenderDeviceFactory* m_pFactory = nullptr;
	const BvGPUInfoGl& m_GPUInfo;
};