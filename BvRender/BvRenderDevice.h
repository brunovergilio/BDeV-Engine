#pragma once


#include "BvCore/Utils/BvUtils.h"
#include "BvRender/BvCommandQueue.h"
#include "BvRender/BvSwapChain.h"
#include "BvRenderPass.h"
#include "BvCommandPool.h"
#include "BvShaderResource.h"
#include "BvPipelineState.h"


#if defined(CreateSemaphore)
#undef CreateSemaphore
#endif


class BvSemaphore;


class BvRenderDevice
{
	BV_NOCOPYMOVE(BvRenderDevice);

public:
	virtual BvSwapChain * CreateSwapChain(BvNativeWindow & window, const SwapChainDesc & desc, BvCommandQueue & commandQueue) = 0;
	// CreateBuffer
	// CreateBufferView
	// CreateTexture
	// CreateTextureView
	// CreateFence
	virtual BvSemaphore * CreateSemaphore(const u64 initialValue = 0) = 0;
	// CreateQuery
	virtual BvRenderPass * CreateRenderPass(const RenderPassDesc & renderPassDesc) = 0;
	virtual BvCommandPool * CreateCommandPool(const CommandPoolDesc & commandPoolDesc = CommandPoolDesc()) = 0;
	virtual BvShaderResourceLayout * CreateShaderResourceLayout(const ShaderResourceLayoutDesc & shaderResourceLayoutDesc =
		ShaderResourceLayoutDesc()) = 0;
	virtual BvGraphicsPipelineState * CreateGraphicsPipeline(const GraphicsPipelineStateDesc & graphicsPipelineStateDesc) = 0;

	virtual void WaitIdle() const = 0;

	virtual const BvVector<BvCommandQueue *> & GetGraphicsQueue(const u32 index = 0) const = 0;
	virtual const BvVector<BvCommandQueue *> & GetComputeQueue(const u32 index = 0) const = 0;
	virtual const BvVector<BvCommandQueue *> & GetTransferQueue(const u32 index = 0) const = 0;
	virtual bool QueueFamilySupportsPresent(const QueueFamilyType queueFamilyType) const = 0;

protected:
	BvRenderDevice() {}
	virtual ~BvRenderDevice() = 0 {};
};