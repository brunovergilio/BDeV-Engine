#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/RenderAPI/BvRenderCommon.h"
#include "BDeV/Core/Container/BvFixedVector.h"


class IBvRenderDeviceVk;
class IBvSwapChainVk;
class IBvGraphicsPipelineStateVk;
class IBvComputePipelineStateVk;
class IBvRayTracingPipelineStateVk;
class IBvShaderResourceLayoutVk;
class IBvSampler;
class IBvBufferView;
class IBvTextureView;
class IBvBufferVk;
class IBvTextureVk;
class IBvQuery;
class IBvRenderPass;
class IBvGraphicsPipelineState;
class IBvComputePipelineState;
class IBvRayTracingPipelineState;
class BvShaderResourceParams;
class BvFrameDataVk;


class BvCommandBufferVk final
{
	BV_NOCOPY(BvCommandBufferVk);

public:
	enum class State : u8
	{
		kRecording,
		kRenderPass,
		kRenderTarget,
	};

	BvCommandBufferVk(const IBvRenderDeviceVk* pDevice, VkCommandBuffer commandBuffer, BvFrameDataVk* pFrameData);
	BvCommandBufferVk(BvCommandBufferVk&& rhs) noexcept = default;
	BvCommandBufferVk& operator=(BvCommandBufferVk&& rhs) noexcept = default;
	~BvCommandBufferVk();

	void Reset();
	void Begin();
	void End();

	void BeginRenderPass(const IBvRenderPass* pRenderPass, u32 renderPassTargetCount, const RenderPassTargetDesc* pRenderPassTargets);
	void NextSubpass();
	void EndRenderPass();

	void SetRenderTargets(u32 renderTargetCount, const RenderTargetDesc* pRenderTargets);

	void SetViewports(u32 viewportCount, const Viewport* pViewports);
	void SetScissors(u32 scissorCount, const Rect* pScissors);

	void SetGraphicsPipeline(const IBvGraphicsPipelineState* pPipeline);
	void SetComputePipeline(const IBvComputePipelineState* pPipeline);
	void SetRayTracingPipeline(const IBvRayTracingPipelineState* pPipeline);

	void SetShaderResourceParams(u32 resourceParamsCount, BvShaderResourceParams* const* ppResourceParams, u32 startIndex);
	void SetConstantBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0);
	void SetStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0);
	void SetRWStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0);
	void SetFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0);
	void SetRWFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0);
	void SetTextures(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0);
	void SetRWTextures(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0);
	void SetSamplers(u32 count, const IBvSampler* const* ppResources, u32 set, u32 binding, u32 startIndex = 0);
	void SetAccelerationStructures(u32 count, const IBvAccelerationStructure* const* ppResources, u32 set, u32 binding, u32 startIndex = 0);
	void SetShaderConstants(u32 size, const void* pData, u32 offset);

	void SetVertexBufferViews(u32 vertexBufferCount, const IBvBufferView* const* pVertexBufferViews, u32 firstBinding = 0);
	void SetIndexBufferView(const IBvBufferView* pIndexBufferView, IndexFormat indexFormat);

	void SetDepthBounds(float min, float max);
	void SetStencilRef(u32 stencilRef);
	void SetBlendConstants(const float (pColors[4]));
	void SetShadingRate(ShadingRateDimensions dimensions, ShadingRateCombinerOp (pCombinerOps[2]));

	void Draw(u32 vertexCount, u32 instanceCount = 1, u32 firstVertex = 0, u32 firstInstance = 0);
	void DrawIndexed(u32 indexCount, u32 instanceCount = 1, u32 firstIndex = 0, i32 vertexOffset = 0, u32 firstInstance = 0);
	void Dispatch(u32 x, u32 y = 1, u32 z = 1);

	void DrawIndirect(const IBvBuffer* pBuffer, u32 drawCount = 1, u64 offset = 0);
	void DrawIndexedIndirect(const IBvBuffer* pBuffer, u32 drawCount = 1, u64 offset = 0);
	void DispatchIndirect(const IBvBuffer* pBuffer, u64 offset = 0);

	void DispatchMesh(u32 x, u32 y = 1, u32 z = 1);
	void DispatchMeshIndirect(const IBvBuffer* pBuffer, u64 offset = 0);
	void DispatchMeshIndirectCount(const IBvBuffer* pBuffer, u64 offset, const IBvBuffer* pCountBuffer, u64 countOffset, u32 maxCount);

	void CopyBuffer(const IBvBufferVk* pSrcBuffer, IBvBufferVk* pDstBuffer, const VkBufferCopy& copyRegion);
	void CopyBuffer(const IBvBuffer* pSrcBuffer, IBvBuffer* pDstBuffer);
	void CopyBuffer(const IBvBuffer* pSrcBuffer, IBvBuffer* pDstBuffer, const BufferCopyDesc& copyDesc);
	
	void CopyTexture(const IBvTexture* pSrcTexture, IBvTexture* pDstTexture);
	void CopyTexture(const IBvTexture* pSrcTexture, IBvTexture* pDstTexture, const TextureCopyDesc& copyDesc);

	void CopyBufferToTexture(const IBvBufferVk* pSrcBuffer, IBvTextureVk* pDstTexture, u32 copyCount, const VkBufferImageCopy* pCopyRegions);
	void CopyBufferToTexture(const IBvBuffer* pSrcBuffer, IBvTexture* pDstTexture, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs);

	void CopyTextureToBuffer(const IBvTextureVk* pSrcTexture, IBvBufferVk* pDstBuffer, u32 copyCount, const VkBufferImageCopy* pCopyRegions);
	void CopyTextureToBuffer(const IBvTexture* pSrcTexture, IBvBuffer* pDstBuffer, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs);

	void ResourceBarrier(u32 bufferBarrierCount, const VkBufferMemoryBarrier2* pBufferBarriers,
		u32 imageBarrierCount, const VkImageMemoryBarrier2* pImageBarriers, u32 memoryBarrierCount, const VkMemoryBarrier2* pMemoryBarriers);
	void ResourceBarrier(u32 barrierCount, const ResourceBarrierDesc* pBarriers);

	void SetPredication(const IBvBuffer* pBuffer, u64 offset, PredicationOp predicationOp);

	void BeginQuery(IBvQuery* pQuery);
	void EndQuery(IBvQuery* pQuery);

	void BeginEvent(const char* pName, const BvColor& color);
	void EndEvent();
	void SetMarker(const char* pName, const BvColor& color);

	void BuildBLAS(const BLASBuildDesc& desc);
	void BuildTLAS(const TLASBuildDesc& desc);
	void DispatchRays(const DispatchRaysDesc& drDesc);
	void DispatchRaysIndirect(const IBvBuffer* pBuffer, u64 offset = 0);

	BV_INLINE const VkCommandBuffer GetHandle() const { return m_CommandBuffer; }
	BV_INLINE const BvVector<IBvSwapChainVk*>& GetSwapChains() const { return m_SwapChains; }

private:
	void FlushDescriptorSets();
	void ResetRenderTargets();

private:
	const IBvRenderDeviceVk* m_pDevice = nullptr;
	VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
	BvFrameDataVk* m_pFrameData = nullptr;

	BvVector<IBvSwapChainVk*> m_SwapChains;
	
	BvVector<VkWriteDescriptorSetAccelerationStructureKHR> m_ASWriteSets;
	BvVector<VkWriteDescriptorSet> m_WriteSets;
	BvVector<VkDescriptorSet> m_DescriptorSets;

	BvVector<VkBufferImageCopy> m_BufferImageCopyRegions;
	BvVector<VkImageCopy> m_ImageCopyRegions;

	BvVector<VkImageMemoryBarrier2> m_PreRenderBarriers;
	BvVector<VkImageMemoryBarrier2> m_PostRenderBarriers;
	BvVector<VkMemoryBarrier2> m_MemoryBarriers;
	BvVector<VkBufferMemoryBarrier2> m_BufferBarriers;
	BvVector<VkImageMemoryBarrier2> m_ImageBarriers;

	const IBvGraphicsPipelineStateVk* m_pGraphicsPipeline = nullptr;
	const IBvComputePipelineStateVk* m_pComputePipeline = nullptr;
	const IBvRayTracingPipelineStateVk* m_pRayTracingPipeline = nullptr;
	IBvShaderResourceLayoutVk* m_pShaderResourceLayout = nullptr;

	BvVector<VkAccelerationStructureBuildRangeInfoKHR> m_ASRanges;
	BvVector<VkAccelerationStructureGeometryKHR> m_ASGeometries;

	State m_CurrentState = State::kRecording;
	bool m_HasDebugUtils = false;
};