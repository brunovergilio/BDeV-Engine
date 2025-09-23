#pragma once


#include "BvRenderCommon.h"
#include "BDeV/Core/Container/BvVector.h"


class BvWindow;


BV_OBJECT_DEFINE_ID(IBvRenderDevice, "31126d8a-8b56-489b-8b0c-0008de31a1d2");
class IBvRenderDevice : public BvRCObj
{
	BV_NOCOPYMOVE(IBvRenderDevice);

public:
	template<BvRCType T> BV_INLINE bool CreateSwapChain(BvWindow* pWindow, const SwapChainDesc& desc, IBvCommandContext* pContext, T** ppObj) { return CreateSwapChainImpl(pWindow, desc, pContext, BV_OBJ_ARGS(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateBuffer(const BufferDesc& desc, const BufferInitData* pInitData, T** ppObj) { return CreateBufferImpl(desc, pInitData, BV_OBJ_ARGS(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateBufferView(const BufferViewDesc& desc, T** ppObj) { return CreateBufferViewImpl(desc, BV_OBJ_ARGS(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateTexture(const TextureDesc& desc, const TextureInitData* pInitData, T** ppObj) { return CreateTextureImpl(desc, pInitData, BV_OBJ_ARGS(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateTextureView(const TextureViewDesc& desc, T** ppObj) { return CreateTextureViewImpl(desc, BV_OBJ_ARGS(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateSampler(const SamplerDesc& desc, T** ppObj) { return CreateSamplerImpl(desc, BV_OBJ_ARGS(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateRenderPass(const RenderPassDesc& renderPassDesc, T** ppObj) { return CreateRenderPassImpl(renderPassDesc, BV_OBJ_ARGS(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateShaderResourceLayout(const ShaderResourceLayoutDesc& srlDesc, T** ppObj) { return CreateShaderResourceLayoutImpl(srlDesc, BV_OBJ_ARGS(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateShader(const ShaderCreateDesc& shaderDesc, T** ppObj) { return CreateShaderImpl(shaderDesc, BV_OBJ_ARGS(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateGraphicsPipeline(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc, T** ppObj) { return CreateGraphicsPipelineImpl(graphicsPipelineStateDesc, BV_OBJ_ARGS(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateComputePipeline(const ComputePipelineStateDesc& computePipelineStateDesc, T** ppObj) { return CreateComputePipelineImpl(computePipelineStateDesc, BV_OBJ_ARGS(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateRayTracingPipeline(const RayTracingPipelineStateDesc& rayTracingPipelineStateDesc, T** ppObj) { return CreateRayTracingPipelineImpl(rayTracingPipelineStateDesc, BV_OBJ_ARGS(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateQuery(QueryType queryType, T** ppObj) { return CreateQueryImpl(queryType, BV_OBJ_ARGS(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateFence(u64 value, T** ppObj) { return CreateFenceImpl(value, BV_OBJ_ARGS(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateAccelerationStructure(const RayTracingAccelerationStructureDesc& asDesc, T** ppObj) { return CreateAccelerationStructureImpl(asDesc, BV_OBJ_ARGS(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateShaderBindingTable(const ShaderBindingTableDesc& sbtDesc, IBvCommandContext* pContext, T** ppObj) { return CreateShaderBindingTableImpl(sbtDesc, pContext, BV_OBJ_ARGS(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateCommandContext(const CommandContextDesc& commandContextDesc, T** ppObj) { return CreateCommandContextImpl(commandContextDesc, BV_OBJ_ARGS(ppObj)); }

	virtual void WaitIdle() const = 0;

	virtual void GetCopyableFootprints(const TextureDesc& textureDesc, u32 subresourceCount, SubresourceFootprint* pSubresources, u64* pTotalBytes = nullptr) const = 0;
	virtual u64 GetDynamicBufferElementSize(BufferUsage usageFlags, u64 elementStride) const = 0;

	virtual FormatFeatures GetFormatFeatures(Format format) const = 0;

	virtual const BvGPUInfo& GetGPUInfo() const = 0;
	virtual const BvVector<Format>& GetSupportedDisplayFormats() const = 0;

protected:
	IBvRenderDevice() {}
	~IBvRenderDevice() {}

	virtual bool CreateSwapChainImpl(BvWindow* pWindow, const SwapChainDesc& desc, IBvCommandContext* pContext, const BvUUID& objId, void** ppObj) = 0;
	virtual bool CreateBufferImpl(const BufferDesc& desc, const BufferInitData* pInitData, const BvUUID& objId, void** ppObj) = 0;
	virtual bool CreateBufferViewImpl(const BufferViewDesc& desc, const BvUUID& objId, void** ppObj) = 0;
	virtual bool CreateTextureImpl(const TextureDesc& desc, const TextureInitData* pInitData, const BvUUID& objId, void** ppObj) = 0;
	virtual bool CreateTextureViewImpl(const TextureViewDesc& desc, const BvUUID& objId, void** ppObj) = 0;
	virtual bool CreateSamplerImpl(const SamplerDesc& desc, const BvUUID& objId, void** ppObj) = 0;
	virtual bool CreateRenderPassImpl(const RenderPassDesc& renderPassDesc, const BvUUID& objId, void** ppObj) = 0;
	virtual bool CreateShaderResourceLayoutImpl(const ShaderResourceLayoutDesc& srlDesc, const BvUUID& objId, void** ppObj) = 0;
	virtual bool CreateShaderImpl(const ShaderCreateDesc& shaderDesc, const BvUUID& objId, void** ppObj) = 0;
	virtual bool CreateGraphicsPipelineImpl(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc, const BvUUID& objId, void** ppObj) = 0;
	virtual bool CreateComputePipelineImpl(const ComputePipelineStateDesc& computePipelineStateDesc, const BvUUID& objId, void** ppObj) = 0;
	virtual bool CreateRayTracingPipelineImpl(const RayTracingPipelineStateDesc& rayTracingPipelineStateDesc, const BvUUID& objId, void** ppObj) = 0;
	virtual bool CreateQueryImpl(QueryType queryType, const BvUUID& objId, void** ppObj) = 0;
	virtual bool CreateFenceImpl(u64 value, const BvUUID& objId, void** ppObj) = 0;
	virtual bool CreateAccelerationStructureImpl(const RayTracingAccelerationStructureDesc& asDesc, const BvUUID& objId, void** ppObj) = 0;
	virtual bool CreateShaderBindingTableImpl(const ShaderBindingTableDesc& sbtDesc, IBvCommandContext* pContext, const BvUUID& objId, void** ppObj) = 0;
	virtual bool CreateCommandContextImpl(const CommandContextDesc& commandContextDesc, const BvUUID& objId, void** ppObj) = 0;
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvRenderDevice);