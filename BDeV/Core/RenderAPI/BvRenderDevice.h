#pragma once


#include "BvRenderCommon.h"
#include "BDeV/Core/Container/BvVector.h"


class BvWindow;


class IBvRenderDevice : public BvRCObj
{
	BV_NOCOPYMOVE(IBvRenderDevice);

public:
	template<BvRCType T> BV_INLINE bool CreateSwapChain(BvWindow* pWindow, const SwapChainDesc& desc, IBvCommandContext* pContext, T** ppObj) { return CreateSwapChainImpl(pWindow, desc, pContext, reinterpret_cast<void**>(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateBuffer(const BufferDesc& desc, T** ppObj) { return CreateBufferImpl(desc, nullptr, reinterpret_cast<void**>(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateBuffer(const BufferDesc& desc, const BufferInitData& initData, T** ppObj) { return CreateBufferImpl(desc, &initData, reinterpret_cast<void**>(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateBufferView(const BufferViewDesc& desc, T** ppObj) { return CreateBufferViewImpl(desc, reinterpret_cast<void**>(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateTexture(const TextureDesc& desc, T** ppObj) { return CreateTextureImpl(desc, nullptr, reinterpret_cast<void**>(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateTexture(const TextureDesc& desc, const TextureInitData& initData, T** ppObj) { return CreateTextureImpl(desc, &initData, reinterpret_cast<void**>(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateTextureView(const TextureViewDesc& desc, T** ppObj) { return CreateTextureViewImpl(desc, reinterpret_cast<void**>(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateSampler(const SamplerDesc& desc, T** ppObj) { return CreateSamplerImpl(desc, reinterpret_cast<void**>(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateRenderPass(const RenderPassDesc& renderPassDesc, T** ppObj) { return CreateRenderPassImpl(renderPassDesc, reinterpret_cast<void**>(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateShaderResourceLayout(const ShaderResourceLayoutCreateDesc& srlDesc, T** ppObj) { return CreateShaderResourceLayoutImpl(srlDesc, reinterpret_cast<void**>(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateShader(const ShaderDesc& shaderDesc, T** ppObj) { return CreateShaderImpl(shaderDesc, reinterpret_cast<void**>(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateGraphicsPipeline(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc, T** ppObj) { return CreateGraphicsPipelineImpl(graphicsPipelineStateDesc, nullptr, reinterpret_cast<void**>(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateComputePipeline(const ComputePipelineStateDesc& computePipelineStateDesc, T** ppObj) { return CreateComputePipelineImpl(computePipelineStateDesc, nullptr, reinterpret_cast<void**>(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateRayTracingPipeline(const RayTracingPipelineStateDesc& rayTracingPipelineStateDesc, T** ppObj) { return CreateRayTracingPipelineImpl(rayTracingPipelineStateDesc, nullptr, reinterpret_cast<void**>(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateQuery(QueryType queryType, T** ppObj) { return CreateQueryImpl(queryType, reinterpret_cast<void**>(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateFence(const GPUFenceDesc& fenceDesc, T** ppObj) { return CreateFenceImpl(fenceDesc, reinterpret_cast<void**>(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateAccelerationStructure(const RayTracingAccelerationStructureDesc& asDesc, T** ppObj) { return CreateAccelerationStructureImpl(asDesc, reinterpret_cast<void**>(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateShaderBindingTable(const ShaderBindingTableDesc& sbtDesc, IBvCommandContext* pContext, T** ppObj) { return CreateShaderBindingTableImpl(sbtDesc, pContext, reinterpret_cast<void**>(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreateCommandContext(const CommandContextDesc& commandContextDesc, T** ppObj) { return CreateCommandContextImpl(commandContextDesc, reinterpret_cast<void**>(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreatePipelineCache(const PipelineCacheInitData& initData, T** ppObj) { return CreatePipelineCacheImpl(&initData, reinterpret_cast<void**>(ppObj)); }
	template<BvRCType T> BV_INLINE bool CreatePipelineCache(T** ppObj) { return CreatePipelineCacheImpl(nullptr, reinterpret_cast<void**>(ppObj)); }

	virtual void WaitIdle() const = 0;

	virtual void GetCopyableFootprints(const TextureDesc& textureDesc, u32 subresourceCount, SubresourceFootprint* pSubresources, u64* pTotalBytes = nullptr,
		u64 baseOffset = 0, u64 firstSubresource = 0) const = 0;
	virtual u64 GetDynamicBufferElementSize(BufferUsage usageFlags, u64 elementStride) const = 0;

	virtual FormatFeatures GetFormatFeatures(Format format) const = 0;

	virtual const BvGPUInfo& GetGPUInfo() const = 0;
	virtual const BvVector<Format>& GetSupportedDisplayFormats() const = 0;

protected:
	IBvRenderDevice() {}
	~IBvRenderDevice() {}

	virtual bool CreateSwapChainImpl(BvWindow* pWindow, const SwapChainDesc& desc, IBvCommandContext* pContext, void** ppObj) = 0;
	virtual bool CreateBufferImpl(const BufferDesc& desc, const BufferInitData* pInitData, void** ppObj) = 0;
	virtual bool CreateBufferViewImpl(const BufferViewDesc& desc, void** ppObj) = 0;
	virtual bool CreateTextureImpl(const TextureDesc& desc, const TextureInitData* pInitData, void** ppObj) = 0;
	virtual bool CreateTextureViewImpl(const TextureViewDesc& desc, void** ppObj) = 0;
	virtual bool CreateSamplerImpl(const SamplerDesc& desc, void** ppObj) = 0;
	virtual bool CreateRenderPassImpl(const RenderPassDesc& renderPassDesc, void** ppObj) = 0;
	virtual bool CreateShaderResourceLayoutImpl(const ShaderResourceLayoutCreateDesc& srlDesc, void** ppObj) = 0;
	virtual bool CreateShaderImpl(const ShaderDesc& shaderDesc, void** ppObj) = 0;
	virtual bool CreateGraphicsPipelineImpl(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc, IBvPipelineCache* pPipelineCache, void** ppObj) = 0;
	virtual bool CreateComputePipelineImpl(const ComputePipelineStateDesc& computePipelineStateDesc, IBvPipelineCache* pPipelineCache, void** ppObj) = 0;
	virtual bool CreateRayTracingPipelineImpl(const RayTracingPipelineStateDesc& rayTracingPipelineStateDesc, IBvPipelineCache* pPipelineCache, void** ppObj) = 0;
	virtual bool CreateQueryImpl(QueryType queryType, void** ppObj) = 0;
	virtual bool CreateFenceImpl(const GPUFenceDesc& fenceDesc, void** ppObj) = 0;
	virtual bool CreateAccelerationStructureImpl(const RayTracingAccelerationStructureDesc& asDesc, void** ppObj) = 0;
	virtual bool CreateShaderBindingTableImpl(const ShaderBindingTableDesc& sbtDesc, IBvCommandContext* pContext, void** ppObj) = 0;
	virtual bool CreateCommandContextImpl(const CommandContextDesc& commandContextDesc, void** ppObj) = 0;
	virtual bool CreatePipelineCacheImpl(const PipelineCacheInitData* pInitData, void** ppObj) = 0;
};
BV_OBJECT_DEFINE_ID(IBvRenderDevice, "31126d8a-8b56-489b-8b0c-0008de31a1d2");