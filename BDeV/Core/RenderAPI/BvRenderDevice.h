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
#include "BvAccelerationStructure.h"
#include "BvShaderBindingTable.h"


BV_OBJECT_DEFINE_ID(IBvRenderDevice, "31126d8a-8b56-489b-8b0c-0008de31a1d2");
class IBvRenderDevice : public BvObjectBase
{
	BV_NOCOPYMOVE(IBvRenderDevice);

public:
	virtual bool CreateSwapChain(BvWindow* pWindow, const SwapChainDesc& desc, IBvCommandContext* pContext, IBvSwapChain** ppObj) = 0;
	virtual bool CreateBuffer(const BufferDesc& desc, const BufferInitData* pInitData, IBvBuffer** ppObj) = 0;
	virtual bool CreateBufferView(const BufferViewDesc& desc, IBvBufferView** ppObj) = 0;
	virtual bool CreateTexture(const TextureDesc& desc, const TextureInitData* pInitData, IBvTexture** ppObj) = 0;
	virtual bool CreateTextureView(const TextureViewDesc& desc, IBvTextureView** ppObj) = 0;
	virtual bool CreateSampler(const SamplerDesc& desc, IBvSampler** ppObj) = 0;
	virtual bool CreateRenderPass(const RenderPassDesc& renderPassDesc, IBvRenderPass** ppObj) = 0;
	virtual bool CreateShaderResourceLayout(const ShaderResourceLayoutDesc& srlDesc, IBvShaderResourceLayout** ppObj) = 0;
	virtual bool CreateShader(const ShaderCreateDesc& shaderDesc, IBvShader** ppObj) = 0;
	virtual bool CreateGraphicsPipeline(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc, IBvGraphicsPipelineState** ppObj) = 0;
	virtual bool CreateComputePipeline(const ComputePipelineStateDesc& computePipelineStateDesc, IBvComputePipelineState** ppObj) = 0;
	virtual bool CreateRayTracingPipeline(const RayTracingPipelineStateDesc& rayTracingPipelineStateDesc, IBvRayTracingPipelineState** ppObj) = 0;
	virtual bool CreateQuery(QueryType queryType, IBvQuery** ppObj) = 0;
	virtual bool CreateFence(u64 value, IBvGPUFence** ppObj) = 0;
	virtual bool CreateAccelerationStructure(const RayTracingAccelerationStructureDesc& asDesc, IBvAccelerationStructure** ppObj) = 0;
	virtual bool CreateShaderBindingTable(const ShaderBindingTableDesc& sbtDesc, IBvCommandContext* pContext, IBvShaderBindingTable** ppObj) = 0;

	virtual bool CreateGraphicsContext(u32 index, IBvCommandContext** ppObj) = 0;
	virtual bool CreateComputeContext(u32 index, IBvCommandContext** ppObj) = 0;
	virtual bool CreateTransferContext(u32 index, IBvCommandContext** ppObj) = 0;

	virtual IBvCommandContext* GetGraphicsContext(u32 index = 0) const = 0;
	virtual IBvCommandContext* GetComputeContext(u32 index = 0) const = 0;
	virtual IBvCommandContext* GetTransferContext(u32 index = 0) const = 0;
	
	virtual void WaitIdle() const = 0;

	virtual void GetCopyableFootprints(const TextureDesc& textureDesc, u32 subresourceCount, SubresourceFootprint* pSubresources, u64* pTotalBytes = nullptr) const = 0;
	virtual u64 GetDynamicBufferElementSize(BufferUsage usageFlags, u64 elementStride) const = 0;

	virtual bool SupportsQueryType(QueryType queryType, CommandType commandType) const = 0;
	virtual FormatFeatures GetFormatFeatures(Format format) const = 0;

	virtual RenderDeviceCapabilities GetDeviceCaps() const = 0;

protected:
	IBvRenderDevice() {}
	~IBvRenderDevice() {};
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvRenderDevice);