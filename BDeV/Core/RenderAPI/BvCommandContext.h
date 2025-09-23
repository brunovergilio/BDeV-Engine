#pragma once


#include "BDeV/Core/RenderAPI/BvRenderCommon.h"
#include "BDeV/Core/RenderAPI/BvGPUFence.h"


class IBvGraphicsPipelineState;
class IBvComputePipelineState;
class IBvShaderResourceParams;
class IBvQuery;


class BvGPUOp
{
public:
	BvGPUOp() = default;
	BV_INLINE BvGPUOp(IBvGPUFence* pFence, u64 signalValue)
		: m_pFence(pFence), m_SignalValue(signalValue) {}
	BV_DEFAULTCOPYMOVE(BvGPUOp);

	BV_INLINE bool IsDone() { return m_pFence->IsDone(m_SignalValue); }
	BV_INLINE bool Wait(u64 timeout = kU64Max) { return m_pFence->Wait(m_SignalValue, timeout); }

private:
	IBvGPUFence* m_pFence = nullptr;
	u64 m_SignalValue = 0;
};


BV_OBJECT_DEFINE_ID(IBvCommandContext, "8740fae9-74bb-4a0f-bf07-b4ff7179e6e4");
class IBvCommandContext : public BvRCObj
{
	BV_NOCOPYMOVE(IBvCommandContext);

public:
	virtual u32 GetGroupIndex() const = 0;
	virtual u32 GetIndex() const = 0;

	virtual BvGPUOp Execute() = 0;
	virtual BvGPUOp Execute(u64 value) = 0;
	virtual void Execute(IBvGPUFence* pFence, u64 value) = 0;
	virtual void ExecuteAndWait() = 0;
	virtual void Wait(IBvCommandContext* pCommandContext, u64 value) = 0;
	virtual void NewCommandList() = 0;
	virtual void FlushFrame() = 0;

	virtual void BeginRenderPass(const IBvRenderPass* pRenderPass, u32 renderPassTargetCount, const RenderPassTargetDesc* pRenderPassTargets) = 0;
	virtual void NextSubpass() = 0;
	virtual void EndRenderPass() = 0;

	virtual void SetRenderTargets(u32 renderTargetCount, const RenderTargetDesc* pRenderTargets, u32 multiviewCount = 0) = 0;
	BV_INLINE void SetRenderTarget(const RenderTargetDesc& renderTarget, u32 multiviewCount = 0)
	{
		SetRenderTargets(1, &renderTarget, multiviewCount);
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

	virtual void SetGraphicsPipeline(const IBvGraphicsPipelineState* pPipeline) = 0;
	virtual void SetComputePipeline(const IBvComputePipelineState* pPipeline) = 0;
	virtual void SetRayTracingPipeline(const IBvRayTracingPipelineState* pPipeline) = 0;

	virtual void SetShaderResourceParams(u32 resourceParamsCount, IBvShaderResourceParams* const* ppResourceParams, u32 startIndex = 0) = 0;
	virtual void SetConstantBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetRWStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetDynamicConstantBuffers(u32 count, const IBvBufferView* const* ppResources, const u32* pOffsets, u32 set, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetDynamicStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, const u32* pOffsets, u32 set, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetDynamicRWStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, const u32* pOffsets, u32 set, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetRWFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetTextures(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetRWTextures(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetSamplers(u32 count, const IBvSampler* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetInputAttachments(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetAccelerationStructures(u32 count, const IBvAccelerationStructure* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetShaderConstants(u32 size, const void* pData, u32 binding, u32 set) = 0;

	BV_INLINE void SetShaderResourceParams(IBvShaderResourceParams* pResourceParams, u32 startIndex = 0) { SetShaderResourceParams(1, &pResourceParams, startIndex); }
	BV_INLINE void SetConstantBuffer(const IBvBufferView* pResource, u32 set, u32 binding, u32 startIndex = 0) { SetConstantBuffers(1, &pResource, set, binding, startIndex); }
	BV_INLINE void SetStructuredBuffer(const IBvBufferView* pResource, u32 set, u32 binding, u32 startIndex = 0) { SetStructuredBuffers(1, &pResource, set, binding, startIndex); }
	BV_INLINE void SetRWStructuredBuffer(const IBvBufferView* pResource, u32 set, u32 binding, u32 startIndex = 0) { SetRWStructuredBuffers(1, &pResource, set, binding, startIndex); }
	BV_INLINE void SetDynamicConstantBuffer(const IBvBufferView* pResource, u32 offset, u32 set, u32 binding, u32 startIndex = 0) { SetDynamicConstantBuffers(1, &pResource, &offset, set, binding, startIndex); }
	BV_INLINE void SetDynamicStructuredBuffer(const IBvBufferView* pResource, u32 offset, u32 set, u32 binding, u32 startIndex = 0) { SetDynamicStructuredBuffers(1, &pResource, &offset, set, binding, startIndex); }
	BV_INLINE void SetDynamicRWStructuredBuffer(const IBvBufferView* pResource, u32 offset, u32 set, u32 binding, u32 startIndex = 0) { SetDynamicRWStructuredBuffers(1, &pResource, &offset, set, binding, startIndex); }
	BV_INLINE void SetFormattedBuffer(const IBvBufferView* pResource, u32 set, u32 binding, u32 startIndex = 0) { SetFormattedBuffers(1, &pResource, set, binding, startIndex); }
	BV_INLINE void SetRWFormattedBuffer(const IBvBufferView* pResource, u32 set, u32 binding, u32 startIndex = 0) { SetRWFormattedBuffers(1, &pResource, set, binding, startIndex); }
	BV_INLINE void SetTexture(const IBvTextureView* pResource, u32 set, u32 binding, u32 startIndex = 0) { SetTextures(1, &pResource, set, binding, startIndex); }
	BV_INLINE void SetRWTexture(const IBvTextureView* pResource, u32 set, u32 binding, u32 startIndex = 0) { SetRWTextures(1, &pResource, set, binding, startIndex); }
	BV_INLINE void SetSampler(const IBvSampler* pResource, u32 set, u32 binding, u32 startIndex = 0) { SetSamplers(1, &pResource, set, binding, startIndex); }
	BV_INLINE void SetInputAttachment(const IBvTextureView* pResource, u32 set, u32 binding, u32 startIndex = 0) { SetInputAttachments(1, &pResource, set, binding, startIndex); }
	BV_INLINE void SetAccelerationStructure(const IBvAccelerationStructure* pResource, u32 set, u32 binding, u32 startIndex = 0) { SetAccelerationStructures(1, &pResource, set, binding, startIndex); }
	template<typename T> BV_INLINE void SetShaderConstantsT(const T& value, u32 binding, u32 set) { SetShaderConstants(sizeof(T), &value, binding, set); }

	virtual void SetVertexBufferViews(u32 vertexBufferCount, const VertexBufferView* pVertexBufferViews, u32 firstBinding = 0) = 0;
	BV_INLINE void SetVertexBufferView(const VertexBufferView& vertexBufferView, u32 firstBinding = 0) { SetVertexBufferViews(1, &vertexBufferView, firstBinding); }
	BV_INLINE void SetVertexBufferView(IBvBuffer* pBuffer, u32 stride, u64 offset = 0, u32 firstBinding = 0)
	{
		VertexBufferView vb{ pBuffer, offset, stride };
		SetVertexBufferViews(1, &vb, firstBinding);
	}
	virtual void SetIndexBufferView(const IndexBufferView& indexBufferView) = 0;
	BV_INLINE void SetIndexBufferView(IBvBuffer* pBuffer, IndexFormat format, u64 offset = 0)
	{
		IndexBufferView ib{ pBuffer, offset, format };
		SetIndexBufferView(ib);
	}

	virtual void SetDepthBounds(f32 min, f32 max) = 0;
	virtual void SetStencilRef(u32 stencilRef) = 0;
	virtual void SetBlendConstants(const float(pColors[4])) = 0;
	BV_INLINE void SetBlendConstants(f32 r, f32 g, f32 b, f32 a = 1.0f)
	{
		f32 colors[]{ r, g, b, a };
		SetBlendConstants(colors);
	}
	virtual void SetShadingRate(ShadingRateDimensions dimensions, ShadingRateCombinerOp(pCombinerOps[2])) = 0;

	virtual void Draw(const DrawCommandArgs& args) = 0;
	virtual void DrawIndexed(const DrawIndexedCommandArgs& args) = 0;
	virtual void Dispatch(const DispatchCommandArgs& args) = 0;
	virtual void DispatchMesh(const DispatchMeshCommandArgs& args) = 0;

	BV_INLINE void Draw(u32 vertexCount, u32 instanceCount = 1, u32 firstVertex = 0, u32 firstInstance = 0)
	{
		Draw({ vertexCount, instanceCount, firstVertex, firstInstance });
	}

	BV_INLINE void DrawIndexed(u32 indexCount, u32 instanceCount = 1, u32 firstIndex = 0, i32 vertexOffset = 0, u32 firstInstance = 0)
	{
		DrawIndexed({ indexCount, instanceCount, firstIndex, vertexOffset, firstInstance });
	}

	BV_INLINE void Dispatch(u32 x, u32 y = 1, u32 z = 1)
	{
		Dispatch({ x, y, z });
	}

	BV_INLINE void DispatchMesh(u32 x, u32 y = 1, u32 z = 1)
	{
		DispatchMesh({ x, y, z });
	}

	virtual void DrawIndirect(const IBvBuffer* pBuffer, u32 drawCount = 1, u64 offset = 0) = 0;
	virtual void DrawIndexedIndirect(const IBvBuffer* pBuffer, u32 drawCount = 1, u64 offset = 0) = 0;
	virtual void DispatchIndirect(const IBvBuffer* pBuffer, u64 offset = 0) = 0;
	virtual void DispatchMeshIndirect(const IBvBuffer* pBuffer, u64 offset = 0) = 0;
	virtual void DispatchMeshIndirectCount(const IBvBuffer* pBuffer, u64 offset, const IBvBuffer* pCountBuffer, u64 countOffset, u32 maxCount) = 0;

	virtual void CopyBuffer(const IBvBuffer* pSrcBuffer, IBvBuffer* pDstBuffer) = 0;
	virtual void CopyBuffer(const IBvBuffer* pSrcBuffer, IBvBuffer* pDstBuffer, const BufferCopyDesc& copyDesc) = 0;

	virtual void CopyTexture(const IBvTexture* pSrcTexture, IBvTexture* pDstTexture) = 0;
	virtual void CopyTexture(const IBvTexture* pSrcTexture, IBvTexture* pDstTexture, const TextureCopyDesc& copyDesc) = 0;

	virtual void CopyBufferToTexture(const IBvBuffer* pSrcBuffer, IBvTexture* pDstTexture, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs) = 0;
	BV_INLINE void CopyBufferToTexture(const IBvBuffer* pSrcBuffer, IBvTexture* pDstTexture, const BufferTextureCopyDesc& copyDesc)
	{
		CopyBufferToTexture(pSrcBuffer, pDstTexture, 1, &copyDesc);
	}

	virtual void CopyTextureToBuffer(const IBvTexture* pSrcTexture, IBvBuffer* pDstBuffer, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs) = 0;
	BV_INLINE void CopyTextureToBuffer(const IBvTexture* pSrcTexture, IBvBuffer* pDstBuffer, const BufferTextureCopyDesc& copyDesc)
	{
		CopyTextureToBuffer(pSrcTexture, pDstBuffer, 1, &copyDesc);
	}

	virtual void ResourceBarrier(u32 barrierCount, const ResourceBarrierDesc* pBarriers) = 0;

	virtual void SetPredication(const IBvBuffer* pBuffer, u64 offset, PredicationOp predicationOp) = 0;

	virtual bool SupportsQueryType(QueryType queryType) const = 0;
	virtual void BeginQuery(IBvQuery* pQuery) = 0;
	virtual void EndQuery(IBvQuery* pQuery) = 0;

	virtual void BeginEvent(const char* pName, const BvColor& color = BvColor::Black) = 0;
	virtual void EndEvent() = 0;
	virtual void SetMarker(const char* pName, const BvColor& color = BvColor::Black) = 0;

	virtual void BuildBLAS(const BLASBuildDesc& blasDesc, const ASPostBuildDesc* pPostBuildDesc = nullptr) = 0;
	virtual void BuildTLAS(const TLASBuildDesc& tlasDesc, const ASPostBuildDesc* pPostBuildDesc = nullptr) = 0;
	virtual void CopyBLAS(const AccelerationStructureCopyDesc& copyDesc) = 0;
	virtual void CopyTLAS(const AccelerationStructureCopyDesc& copyDesc) = 0;
	virtual void DispatchRays(const DispatchRaysCommandArgs& args) = 0;
	virtual void DispatchRays(IBvShaderBindingTable* pSBT, u32 rayGenIndex, u32 missIndex, u32 hitIndex, u32 callableIndex,
		u32 width, u32 height, u32 depth) = 0;
	virtual void DispatchRaysIndirect(const IBvBuffer* pBuffer, u64 offset = 0) = 0;

protected:
	IBvCommandContext() {}
	~IBvCommandContext() = 0 {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvCommandContext);