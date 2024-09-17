#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/RenderAPI/BvRenderCommon.h"
#include "BDeV/Core/Container/BvFixedVector.h"


constexpr u32 kMaxSemaphores = 8;


class BvRenderDeviceVk;
class BvSwapChainVk;
class BvGraphicsPipelineStateVk;
class BvComputePipelineStateVk;
class BvFrameDataVk;
class BvSampler;
class BvBufferView;
class BvTextureView;
class BvBufferVk;
class BvTextureVk;
class BvQuery;
class BvRenderPass;
class BvGraphicsPipelineState;
class BvComputePipelineState;
class BvShaderResourceParams;


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

	BvCommandBufferVk(const BvRenderDeviceVk* pDevice, VkCommandBuffer commandBuffer, BvFrameDataVk* pFrameData);
	BvCommandBufferVk(BvCommandBufferVk&& rhs) noexcept;
	BvCommandBufferVk& operator=(BvCommandBufferVk&& rhs) noexcept;
	~BvCommandBufferVk();

	void Reset();
	void Begin();
	void End();

	void BeginRenderPass(const BvRenderPass* pRenderPass, u32 renderPassTargetCount, const RenderPassTargetDesc* pRenderPassTargets);
	void NextSubpass();
	void EndRenderPass();

	void SetRenderTargets(u32 renderTargetCount, const RenderTargetDesc* pRenderTargets);

	void SetViewports(u32 viewportCount, const Viewport* pViewports);
	void SetScissors(u32 scissorCount, const Rect* pScissors);

	void SetGraphicsPipeline(const BvGraphicsPipelineState* pPipeline);
	void SetComputePipeline(const BvComputePipelineState* pPipeline);

	void SetShaderResourceParams(u32 setCount, BvShaderResourceParams* const* ppSets, u32 firstSet);
	void SetShaderResource(const BvBufferView* pResource, u32 set, u32 binding, u32 arrayIndex);
	void SetShaderResource(const BvTextureView* pResource, u32 set, u32 binding, u32 arrayIndex);
	void SetShaderResource(const BvSampler* pResource, u32 set, u32 binding, u32 arrayIndex);
	void SetShaderConstants(u32 size, const void* pData, u32 offset);

	void SetVertexBufferViews(u32 vertexBufferCount, const BvBufferView* const* pVertexBufferViews, u32 firstBinding = 0);
	void SetIndexBufferView(const BvBufferView* pIndexBufferView, IndexFormat indexFormat);

	void SetDepthBounds(float min, float max);
	void SetStencilRef(u32 stencilRef);
	void SetBlendConstants(const float (pColors[4]));
	void SetShadingRate(ShadingRateDimensions dimensions, ShadingRateCombinerOp (pCombinerOps[2]));

	void Draw(u32 vertexCount, u32 instanceCount = 1, u32 firstVertex = 0, u32 firstInstance = 0);
	void DrawIndexed(u32 indexCount, u32 instanceCount = 1, u32 firstIndex = 0, i32 vertexOffset = 0, u32 firstInstance = 0);
	void Dispatch(u32 x, u32 y = 1, u32 z = 1);

	void DrawIndirect(const BvBuffer* pBuffer, u32 drawCount = 1, u64 offset = 0);
	void DrawIndexedIndirect(const BvBuffer* pBuffer, u32 drawCount = 1, u64 offset = 0);
	void DispatchIndirect(const BvBuffer* pBuffer, u64 offset = 0);

	void DispatchMesh(u32 x, u32 y = 1, u32 z = 1);
	void DispatchMeshIndirect(const BvBuffer* pBuffer, u64 offset = 0);
	void DispatchMeshIndirectCount(const BvBuffer* pBuffer, u64 offset, const BvBuffer* pCountBuffer, u64 countOffset, u32 maxCount);

	void CopyBuffer(const BvBufferVk* pSrcBuffer, BvBufferVk* pDstBuffer, const VkBufferCopy& copyRegion);
	void CopyBuffer(const BvBuffer* pSrcBuffer, BvBuffer* pDstBuffer);
	void CopyBuffer(const BvBuffer* pSrcBuffer, BvBuffer* pDstBuffer, const BufferCopyDesc& copyDesc);
	
	void CopyTexture(const BvTexture* pSrcTexture, BvTexture* pDstTexture);
	void CopyTexture(const BvTexture* pSrcTexture, BvTexture* pDstTexture, const TextureCopyDesc& copyDesc);

	void CopyBufferToTexture(const BvBufferVk* pSrcBuffer, BvTextureVk* pDstTexture, u32 copyCount, const VkBufferImageCopy* pCopyRegions);
	void CopyBufferToTexture(const BvBuffer* pSrcBuffer, BvTexture* pDstTexture, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs);

	void CopyTextureToBuffer(const BvTextureVk* pSrcTexture, BvBufferVk* pDstBuffer, u32 copyCount, const VkBufferImageCopy* pCopyRegions);
	void CopyTextureToBuffer(const BvTexture* pSrcTexture, BvBuffer* pDstBuffer, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs);

	void ResourceBarrier(u32 bufferBarrierCount, const VkBufferMemoryBarrier2* pBufferBarriers,
		u32 imageBarrierCount, const VkImageMemoryBarrier2* pImageBarriers, u32 memoryBarrierCount, const VkMemoryBarrier2* pMemoryBarriers);
	void ResourceBarrier(u32 barrierCount, const ResourceBarrierDesc* pBarriers);

	void SetPredication(const BvBuffer* pBuffer, u64 offset, PredicationOp predicationOp);

	void BeginQuery(BvQuery* pQuery);
	void EndQuery(BvQuery* pQuery);

	BV_INLINE const VkCommandBuffer GetHandle() const { return m_CommandBuffer; }
	BV_INLINE const BvVector<BvSwapChainVk*>& GetSwapChains() const { return m_SwapChains; }

private:
	void FlushDescriptorSets();
	void ResetRenderTargets();

private:
	const BvRenderDeviceVk* m_pDevice = nullptr;
	VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
	BvFrameDataVk* m_pFrameData = nullptr;

	BvVector<BvSwapChainVk*> m_SwapChains;
	
	BvVector<VkWriteDescriptorSet> m_WriteSets;

	BvVector<VkBufferImageCopy> m_BufferImageCopyRegions{};
	BvVector<VkImageCopy> m_ImageCopyRegions{};

	BvVector<VkImageMemoryBarrier2> m_PreRenderBarriers;
	BvVector<VkImageMemoryBarrier2> m_PostRenderBarriers;
	BvVector<VkMemoryBarrier2> m_MemoryBarriers;
	BvVector<VkBufferMemoryBarrier2> m_BufferBarriers;
	BvVector<VkImageMemoryBarrier2> m_ImageBarriers;

	const BvGraphicsPipelineStateVk* m_pGraphicsPipeline = nullptr;
	const BvComputePipelineStateVk* m_pComputePipeline = nullptr;

	State m_CurrentState = State::kRecording;
};