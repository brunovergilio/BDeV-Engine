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
#include "BvRenderDeviceObject.h"
#include "BvGPUFence.h"


BV_OBJECT_DEFINE_ID(BvRenderDevice, "31126d8a-8b56-489b-8b0c-0008de31a1d2");
class BvRenderDevice : public BvObjectBase
{
	BV_NOCOPYMOVE(BvRenderDevice);

public:
	virtual bool CreateSwapChain(BvWindow* pWindow, const SwapChainDesc& desc, BvCommandContext* pContext, BvSwapChain** ppObj) = 0;
	virtual bool CreateBuffer(const BufferDesc& desc, const BufferInitData* pInitData, BvBuffer** ppObj) = 0;
	virtual bool CreateBufferView(const BufferViewDesc& desc, BvBufferView** ppObj) = 0;
	virtual bool CreateTexture(const TextureDesc& desc, const TextureInitData* pInitData, BvTexture** ppObj) = 0;
	virtual bool CreateTextureView(const TextureViewDesc& desc, BvTextureView** ppObj) = 0;
	virtual bool CreateSampler(const SamplerDesc& desc, BvSampler** ppObj) = 0;
	virtual bool CreateRenderPass(const RenderPassDesc& renderPassDesc, BvRenderPass** ppObj) = 0;
	virtual bool CreateShaderResourceLayout(u32 shaderResourceCount, const ShaderResourceDesc* pShaderResourceDescs,
		const ShaderResourceConstantDesc* pShaderResourceConstantDesc, BvShaderResourceLayout** ppObj) = 0;
	virtual bool CreateShader(const ShaderCreateDesc& shaderDesc, BvShader** ppObj) = 0;
	virtual bool CreateGraphicsPipeline(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc, BvGraphicsPipelineState** ppObj) = 0;
	virtual bool CreateComputePipeline(const ComputePipelineStateDesc& computePipelineStateDesc, BvComputePipelineState** ppObj) = 0;
	virtual bool CreateQuery(QueryType queryType, BvQuery** ppObj) = 0;
	virtual bool CreateFence(u64 value, BvGPUFence** ppObj) = 0;
	
	virtual void WaitIdle() const = 0;

	virtual BvCommandContext* GetGraphicsContext(u32 index = 0) const = 0;
	virtual BvCommandContext* GetComputeContext(u32 index = 0) const = 0;
	virtual BvCommandContext* GetTransferContext(u32 index = 0) const = 0;

	virtual void GetCopyableFootprints(const TextureDesc& textureDesc, u32 subresourceCount, SubresourceFootprint* pSubresources, u64* pTotalBytes = nullptr) const = 0;

	virtual bool SupportsQueryType(QueryType queryType, CommandType commandType) const = 0;
	virtual FormatFeatures GetFormatFeatures(Format format) const = 0;

	virtual RenderDeviceCapabilities GetDeviceCaps() const = 0;

protected:
	BvRenderDevice() {}
	virtual ~BvRenderDevice() = 0 {};
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvRenderDevice);