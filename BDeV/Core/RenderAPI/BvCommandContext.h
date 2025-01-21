#pragma once


#include "BDeV/Core/RenderAPI/BvRenderCommon.h"


class BvBuffer;
class BvBufferView;
class BvTexture;
class BvTextureView;
class BvSampler;
class BvRenderPass;
class BvGraphicsPipelineState;
class BvComputePipelineState;
class BvShaderResourceParams;
class BvQuery;


class BvCommandContext
{
public:
	virtual void AddDeferredContext(BvCommandContext* pDeferredContext) = 0;
	virtual void Signal() = 0;
	virtual void Signal(u64 value) = 0;
	virtual void Wait(BvCommandContext* pCommandContext, u64 value) = 0;
	virtual void Flush() = 0;
	virtual void WaitForGPU() = 0;

	virtual void BeginRenderPass(const BvRenderPass* pRenderPass, u32 renderPassTargetCount, const RenderPassTargetDesc* pRenderPassTargets) = 0;
	virtual void EndRenderPass() = 0;

	virtual void SetRenderTargets(u32 renderTargetCount, const RenderTargetDesc* pRenderTargets) = 0;
	BV_INLINE void SetRenderTarget(const RenderTargetDesc& renderTarget)
	{
		SetRenderTargets(1, &renderTarget);
	}

	virtual void SetViewports(u32 viewportCount, const Viewport* pViewports) = 0;
	BV_INLINE void SetViewport(const Viewport& viewport) { SetViewports(1, &viewport); }
	BV_INLINE void SetViewport(f32 width, f32 height, f32 minDepth = 0.0f, f32 maxDepth = 1.0f)
	{
		Viewport viewport{ 0.0f, 0.0f, width, height, minDepth, maxDepth };
		SetViewports(1, &viewport);
	}

	virtual void SetScissors(u32 scissorCount, const Rect* pScissors) = 0;
	BV_INLINE void SetScissor(const Rect& scissor) { SetScissors(1, &scissor); }
	BV_INLINE void SetScissor(u32 width, u32 height)
	{
		Rect scissor{ 0, 0, width, height };
		SetScissors(1, &scissor);
	}

	virtual void SetGraphicsPipeline(const BvGraphicsPipelineState* pPipeline) = 0;
	virtual void SetComputePipeline(const BvComputePipelineState* pPipeline) = 0;

	virtual void SetShaderResourceParams(u32 resourceParamsCount, BvShaderResourceParams* const* ppResourceParams, u32 startIndex = 0) = 0;
	virtual void SetConstantBuffers(u32 count, const BvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetStructuredBuffers(u32 count, const BvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetRWStructuredBuffers(u32 count, const BvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetFormattedBuffers(u32 count, const BvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetRWFormattedBuffers(u32 count, const BvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetTextures(u32 count, const BvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetRWTextures(u32 count, const BvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetSamplers(u32 count, const BvSampler* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetShaderConstants(u32 size, const void* pData, u32 offset = 0) = 0;

	BV_INLINE void SetShaderResourceParams(BvShaderResourceParams* pResourceParams, u32 startIndex = 0) { SetShaderResourceParams(1, &pResourceParams, startIndex); }
	BV_INLINE void SetConstantBuffer(const BvBufferView* pResource, u32 set, u32 binding, u32 startIndex = 0) { SetConstantBuffers(1, &pResource, set, binding, startIndex); }
	BV_INLINE void SetStructuredBuffer(const BvBufferView* pResource, u32 set, u32 binding, u32 startIndex = 0) { SetStructuredBuffers(1, &pResource, set, binding, startIndex); }
	BV_INLINE void SetRWStructuredBuffer(const BvBufferView* pResource, u32 set, u32 binding, u32 startIndex = 0) { SetRWStructuredBuffers(1, &pResource, set, binding, startIndex); }
	BV_INLINE void SetFormattedBuffer(const BvBufferView* pResource, u32 set, u32 binding, u32 startIndex = 0) { SetFormattedBuffers(1, &pResource, set, binding, startIndex); }
	BV_INLINE void SetRWFormattedBuffer(const BvBufferView* pResource, u32 set, u32 binding, u32 startIndex = 0) { SetRWFormattedBuffers(1, &pResource, set, binding, startIndex); }
	BV_INLINE void SetTexture(const BvTextureView* pResource, u32 set, u32 binding, u32 startIndex = 0) { SetTextures(1, &pResource, set, binding, startIndex); }
	BV_INLINE void SetRWTexture(const BvTextureView* pResource, u32 set, u32 binding, u32 startIndex = 0) { SetRWTextures(1, &pResource, set, binding, startIndex); }
	BV_INLINE void SetSampler(const BvSampler* pResource, u32 set, u32 binding, u32 startIndex = 0) { SetSamplers(1, &pResource, set, binding, startIndex); }
	template<typename T> BV_INLINE void SetShaderConstantsT(const T& value, u32 offset = 0) { SetShaderConstants(sizeof(T), &value, offset); }

	virtual void SetVertexBufferViews(u32 vertexBufferCount, const BvBufferView* const* pVertexBufferViews, u32 firstBinding = 0) = 0;
	BV_INLINE void SetVertexBufferView(const BvBufferView* pVertexBufferView, const u32 firstBinding = 0) { SetVertexBufferViews(1, &pVertexBufferView, firstBinding); }
	virtual void SetIndexBufferView(const BvBufferView* pIndexBufferView, IndexFormat indexFormat) = 0;

	virtual void Draw(u32 vertexCount, u32 instanceCount = 1, u32 firstVertex = 0, u32 firstInstance = 0) = 0;
	virtual void DrawIndexed(u32 indexCount, u32 instanceCount = 1, u32 firstIndex = 0, i32 vertexOffset = 0, u32 firstInstance = 0) = 0;
	virtual void Dispatch(u32 x, u32 y = 1, u32 z = 1) = 0;

	virtual void DrawIndirect(const BvBuffer* pBuffer, u32 drawCount = 1, u64 offset = 0) = 0;
	virtual void DrawIndexedIndirect(const BvBuffer* pBuffer, u32 drawCount = 1, u64 offset = 0) = 0;
	virtual void DispatchIndirect(const BvBuffer* pBuffer, u64 offset = 0) = 0;

	virtual void DispatchMesh(u32 x, u32 y = 1, u32 z = 1) = 0;
	virtual void DispatchMeshIndirect(const BvBuffer* pBuffer, u64 offset = 0) = 0;
	virtual void DispatchMeshIndirectCount(const BvBuffer* pBuffer, u64 offset, const BvBuffer* pCountBuffer, u64 countOffset, u32 maxCount) = 0;

	virtual void CopyBuffer(const BvBuffer* pSrcBuffer, BvBuffer* pDstBuffer) = 0;
	virtual void CopyBuffer(const BvBuffer* pSrcBuffer, BvBuffer* pDstBuffer, const BufferCopyDesc& copyDesc) = 0;

	virtual void CopyTexture(const BvTexture* pSrcTexture, BvTexture* pDstTexture) = 0;
	virtual void CopyTexture(const BvTexture* pSrcTexture, BvTexture* pDstTexture, const TextureCopyDesc& copyDesc = TextureCopyDesc()) = 0;

	virtual void CopyBufferToTexture(const BvBuffer* pSrcBuffer, BvTexture* pDstTexture, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs) = 0;
	BV_INLINE void CopyBufferToTexture(const BvBuffer* pSrcBuffer, BvTexture* pDstTexture, const BufferTextureCopyDesc& copyDesc)
	{
		CopyBufferToTexture(pSrcBuffer, pDstTexture, 1, &copyDesc);
	}

	virtual void CopyTextureToBuffer(const BvTexture* pSrcTexture, BvBuffer* pDstBuffer, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs) = 0;
	BV_INLINE void CopyTextureToBuffer(const BvTexture* pSrcTexture, BvBuffer* pDstBuffer, const BufferTextureCopyDesc& copyDesc)
	{
		CopyTextureToBuffer(pSrcTexture, pDstBuffer, 1, &copyDesc);
	}

	virtual void ResourceBarrier(u32 barrierCount, const ResourceBarrierDesc* pBarriers) = 0;

	virtual void SetPredication(const BvBuffer* pBuffer, u64 offset, PredicationOp predicationOp) = 0;

	virtual void BeginQuery(BvQuery* pQuery) = 0;
	virtual void EndQuery(BvQuery* pQuery) = 0;

protected:
	BvCommandContext() {}
	virtual ~BvCommandContext() = 0 {}
};