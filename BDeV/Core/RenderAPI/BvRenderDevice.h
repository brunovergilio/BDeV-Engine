#pragma once


#include "BDeV/Core/Utils/BvUtils.h"
#include "BvSwapChain.h"
#include "BvRenderPass.h"
#include "BvShaderResource.h"
#include "BvShader.h"
#include "BvPipelineState.h"
#include "BvBuffer.h"
#include "BvBufferView.h"
#include "BvTexture.h"
#include "BvTextureView.h"
#include "BvSampler.h"
#include "BvQuery.h"
#include "BvCommandContext.h"


class BvRenderDevice
{
	BV_NOCOPYMOVE(BvRenderDevice);

public:
	virtual BvSwapChain* CreateSwapChain(BvWindow* pWindow, const SwapChainDesc& desc, BvCommandContext* pContext) = 0;
	virtual BvBuffer* CreateBuffer(const BufferDesc& desc, const BufferInitData* pInitData = nullptr) = 0;
	virtual BvBufferView* CreateBufferView(const BufferViewDesc& desc) = 0;
	virtual BvTexture* CreateTexture(const TextureDesc& desc, const TextureInitData* pInitData = nullptr) = 0;
	virtual BvTextureView* CreateTextureView(const TextureViewDesc& desc) = 0;
	virtual BvSampler* CreateSampler(const SamplerDesc& desc) = 0;
	virtual BvRenderPass* CreateRenderPass(const RenderPassDesc& renderPassDesc) = 0;
	virtual BvShaderResourceLayout* CreateShaderResourceLayout(u32 shaderResourceCount, const ShaderResourceDesc* pShaderResourceDescs,
		const ShaderResourceConstantDesc& shaderResourceConstantDesc) = 0;
	virtual BvShader* CreateShader(const ShaderCreateDesc& shaderDesc) = 0;
	virtual BvGraphicsPipelineState* CreateGraphicsPipeline(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc) = 0;
	virtual BvComputePipelineState* CreateComputePipeline(const ComputePipelineStateDesc& computePipelineStateDesc) = 0;
	virtual BvQuery* CreateQuery(QueryType queryType) = 0;
	
	virtual void Release(IBvRenderDeviceChild* pDeviceObj) = 0;

	virtual void WaitIdle() const = 0;

	virtual BvCommandContext* GetGraphicsContext(u32 index = 0) const = 0;
	virtual BvCommandContext* GetComputeContext(u32 index = 0) const = 0;
	virtual BvCommandContext* GetTransferContext(u32 index = 0) const = 0;

	virtual void GetCopyableFootprints(const TextureDesc& textureDesc, u32 subresourceCount, SubresourceFootprint* pSubresources, u64* pTotalBytes = nullptr) const = 0;

	virtual bool SupportsQueryType(QueryType queryType, CommandType commandType) const = 0;
	virtual bool IsFormatSupported(Format format) const = 0;

	virtual RenderDeviceCapabilities GetDeviceCaps() const = 0;

protected:
	BvRenderDevice() {}
	virtual ~BvRenderDevice() = 0 {};
};