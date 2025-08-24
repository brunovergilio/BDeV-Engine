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
#include "BvGPUFence.h"
#include "BvAccelerationStructure.h"
#include "BvShaderBindingTable.h"


struct BvBaseRenderDeviceTypes
{
	using IBvSwapChainPtr = BvRCRaw<IBvSwapChain>;
	using IBvBufferPtr = BvRCRaw<IBvBuffer>;
	using IBvBufferViewPtr = BvRCRaw<IBvBufferView>;
	using IBvTexturePtr = BvRCRaw<IBvTexture>;
	using IBvTextureViewPtr = BvRCRaw<IBvTextureView>;
	using IBvSamplerPtr = BvRCRaw<IBvSampler>;
	using IBvRenderPassPtr = BvRCRaw<IBvRenderPass>;
	using IBvShaderResourceLayoutPtr = BvRCRaw<IBvShaderResourceLayout>;
	using IBvShaderPtr = BvRCRaw<IBvShader>;
	using IBvGraphicsPipelineStatePtr = BvRCRaw<IBvGraphicsPipelineState>;
	using IBvComputePipelineStatePtr = BvRCRaw<IBvComputePipelineState>;
	using IBvRayTracingPipelineStatePtr = BvRCRaw<IBvRayTracingPipelineState>;
	using IBvQueryPtr = BvRCRaw<IBvQuery>;
	using IBvGPUFencePtr = BvRCRaw<IBvGPUFence>;
	using IBvAccelerationStructurePtr = BvRCRaw<IBvAccelerationStructure>;
	using IBvShaderBindingTablePtr = BvRCRaw<IBvShaderBindingTable>;
	using IBvCommandContextPtr = BvRCRaw<IBvCommandContext>;
};


//BV_OBJECT_DEFINE_ID(IBvRenderDevice, "31126d8a-8b56-489b-8b0c-0008de31a1d2");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvRenderDevice);
class IBvRenderDevice : public BvRCObj
{
	BV_NOCOPYMOVE(IBvRenderDevice);

public:
	template<typename T = IBvSwapChain> BV_INLINE BvRCRaw<T> CreateSwapChain(BvWindow* pWindow, const SwapChainDesc& desc, IBvCommandContext* pContext) { return static_cast<T*>(CreateSwapChainImpl(pWindow, desc, pContext)); }
	template<typename T = IBvBuffer> BV_INLINE BvRCRaw<T> CreateBuffer(const BufferDesc& desc, const BufferInitData* pInitData = nullptr) { return static_cast<T*>(CreateBufferImpl(desc, pInitData)); }
	template<typename T = IBvBufferView> BV_INLINE BvRCRaw<T> CreateBufferView(const BufferViewDesc& desc) { return static_cast<T*>(CreateBufferViewImpl(desc)); }
	template<typename T = IBvTexture> BV_INLINE BvRCRaw<T> CreateTexture(const TextureDesc& desc, const TextureInitData* pInitData = nullptr) { return static_cast<T*>(CreateTextureImpl(desc, pInitData)); }
	template<typename T = IBvTextureView> BV_INLINE BvRCRaw<T> CreateTextureView(const TextureViewDesc& desc) { return static_cast<T*>(CreateTextureViewImpl(desc)); }
	template<typename T = IBvSampler> BV_INLINE BvRCRaw<T> CreateSampler(const SamplerDesc& desc) { return static_cast<T*>(CreateSamplerImpl(desc)); }
	template<typename T = IBvRenderPass> BV_INLINE BvRCRaw<T> CreateRenderPass(const RenderPassDesc& renderPassDesc) { return static_cast<T*>(CreateRenderPassImpl(renderPassDesc)); }
	template<typename T = IBvShaderResourceLayout> BV_INLINE BvRCRaw<T> CreateShaderResourceLayout(const ShaderResourceLayoutDesc& srlDesc) { return static_cast<T*>(CreateShaderResourceLayoutImpl(srlDesc)); }
	template<typename T = IBvShader> BV_INLINE BvRCRaw<T> CreateShader(const ShaderCreateDesc& shaderDesc) { return static_cast<T*>(CreateShaderImpl(shaderDesc)); }
	template<typename T = IBvGraphicsPipelineState> BV_INLINE BvRCRaw<T> CreateGraphicsPipeline(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc) { return static_cast<T*>(CreateGraphicsPipelineImpl(graphicsPipelineStateDesc)); }
	template<typename T = IBvComputePipelineState> BV_INLINE BvRCRaw<T> CreateComputePipeline(const ComputePipelineStateDesc& computePipelineStateDesc) { return static_cast<T*>(CreateComputePipelineImpl(computePipelineStateDesc)); }
	template<typename T = IBvRayTracingPipelineState> BV_INLINE BvRCRaw<T> CreateRayTracingPipeline(const RayTracingPipelineStateDesc& rayTracingPipelineStateDesc) { return static_cast<T*>(CreateRayTracingPipelineImpl(rayTracingPipelineStateDesc)); }
	template<typename T = IBvQuery> BV_INLINE BvRCRaw<T> CreateQuery(QueryType queryType) { return static_cast<T*>(CreateQueryImpl(queryType)); }
	template<typename T = IBvGPUFence> BV_INLINE BvRCRaw<T> CreateFence(u64 value) { return static_cast<T*>(CreateFenceImpl(value)); }
	template<typename T = IBvAccelerationStructure> BV_INLINE BvRCRaw<T> CreateAccelerationStructure(const RayTracingAccelerationStructureDesc& asDesc) { return static_cast<T*>(CreateAccelerationStructureImpl(asDesc)); }
	template<typename T = IBvShaderBindingTable> BV_INLINE BvRCRaw<T> CreateShaderBindingTable(const ShaderBindingTableDesc& sbtDesc, IBvCommandContext* pContext) { return static_cast<T*>(CreateShaderBindingTableImpl(sbtDesc, pContext)); }
	template<typename T = IBvCommandContext> BV_INLINE BvRCRaw<T> CreateCommandContext(const CommandContextDesc& commandContextDesc) { return static_cast<T*>(CreateCommandContextImpl(commandContextDesc)); }

	virtual void WaitIdle() const = 0;

	virtual void GetCopyableFootprints(const TextureDesc& textureDesc, u32 subresourceCount, SubresourceFootprint* pSubresources, u64* pTotalBytes = nullptr) const = 0;
	virtual u64 GetDynamicBufferElementSize(BufferUsage usageFlags, u64 elementStride) const = 0;

	virtual FormatFeatures GetFormatFeatures(Format format) const = 0;

	virtual const BvGPUInfo& GetGPUInfo() const = 0;
	virtual const BvVector<Format>& GetSupportedDisplayFormats() const = 0;

protected:
	IBvRenderDevice() {}
	~IBvRenderDevice() {}

	virtual IBvSwapChain* CreateSwapChainImpl(BvWindow* pWindow, const SwapChainDesc& desc, IBvCommandContext* pContext) = 0;
	virtual IBvBuffer* CreateBufferImpl(const BufferDesc& desc, const BufferInitData* pInitData) = 0;
	virtual IBvBufferView* CreateBufferViewImpl(const BufferViewDesc& desc) = 0;
	virtual IBvTexture* CreateTextureImpl(const TextureDesc& desc, const TextureInitData* pInitData) = 0;
	virtual IBvTextureView* CreateTextureViewImpl(const TextureViewDesc& desc) = 0;
	virtual IBvSampler* CreateSamplerImpl(const SamplerDesc& desc) = 0;
	virtual IBvRenderPass* CreateRenderPassImpl(const RenderPassDesc& renderPassDesc) = 0;
	virtual IBvShaderResourceLayout* CreateShaderResourceLayoutImpl(const ShaderResourceLayoutDesc& srlDesc) = 0;
	virtual IBvShader* CreateShaderImpl(const ShaderCreateDesc& shaderDesc) = 0;
	virtual IBvGraphicsPipelineState* CreateGraphicsPipelineImpl(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc) = 0;
	virtual IBvComputePipelineState* CreateComputePipelineImpl(const ComputePipelineStateDesc& computePipelineStateDesc) = 0;
	virtual IBvRayTracingPipelineState* CreateRayTracingPipelineImpl(const RayTracingPipelineStateDesc& rayTracingPipelineStateDesc) = 0;
	virtual IBvQuery* CreateQueryImpl(QueryType queryType) = 0;
	virtual IBvGPUFence* CreateFenceImpl(u64 value) = 0;
	virtual IBvAccelerationStructure* CreateAccelerationStructureImpl(const RayTracingAccelerationStructureDesc& asDesc) = 0;
	virtual IBvShaderBindingTable* CreateShaderBindingTableImpl(const ShaderBindingTableDesc& sbtDesc, IBvCommandContext* pContext) = 0;
	virtual IBvCommandContext* CreateCommandContextImpl(const CommandContextDesc& commandContextDesc) = 0;
};