#pragma once


#include "BvRenderVK/BvCommonVk.h"
#include "BDeV/Container/BvVector.h"
#include "BDeV/RenderAPI/BvCommandBuffer.h"
#include "BDeV/RenderAPI/BvCommon.h"
#include "BDeV/Container/BvFixedVector.h"


constexpr u32 kMaxSemaphores = 8;


class BvCommandPoolVk;
class BvRenderDeviceVk;
class BvTextureViewVk;
class BvSwapChainVk;
class BvGraphicsPipelineStateVk;
class BvComputePipelineStateVk;
class BvRenderPassVk;
class BvBufferViewVk;


class BvCommandBufferVk final : public BvCommandBuffer
{
public:
	BvCommandBufferVk(const BvRenderDeviceVk& device, VkCommandBuffer commandBuffer);
	~BvCommandBufferVk();

	void Reset() override;
	void Begin() override;
	void End() override;

	void BeginRenderPass(const BvRenderPass * const pRenderPass, BvTextureView * const * const pRenderTargets,
		const ClearColorValue * const pClearColors, BvTextureView * const pDepthStencilTarget,
		const ClearColorValue & depthClear) override;
	void EndRenderPass() override;

	void SetRenderTargets(const u32 renderTargetCount, BvTextureView* const* const pRenderTargets, const ClearColorValue* const pClearColors,
		BvTextureView* const pDepthStencilTarget, const ClearColorValue& depthClear, const ClearFlags clearFlags) override;
	void ClearRenderTargets(u32 renderTargetCount, const ClearColorValue* const pClearValues, u32 firstRenderTargetIndex = 0) override;

	void SetViewports(const u32 viewportCount, const Viewport * const pViewports) override;
	void SetScissors(const u32 scissorCount, const Rect * const pScissors) override;

	void SetPipeline(const BvGraphicsPipelineState * const pPipeline) override;
	void SetPipeline(const BvComputePipelineState * const pPipeline) override;

	void SetShaderResourceParams(const u32 setCount, BvShaderResourceParams * const * const ppSets, const u32 firstSet) override;

	void SetVertexBufferViews(const u32 vertexBufferCount, const BvBufferView * const * const pVertexBufferViews,
		const u32 firstBinding = 0) override;
	void SetIndexBufferView(const BvBufferView * const pIndexBufferView, const IndexFormat indexFormat) override;

	void Draw(const u32 vertexCount, const u32 instanceCount = 1,
		const u32 firstVertex = 0, const u32 firstInstance = 0) override;
	void DrawIndexed(const u32 indexCount, const u32 instanceCount = 1, const u32 firstIndex = 0,
		const i32 vertexOffset = 0, const u32 firstInstance = 0) override;
	void Dispatch(const u32 x, const u32 y = 1, const u32 z = 1) override;

	void DrawIndirect(const BvBuffer * const pBuffer, const u32 drawCount = 1, const u64 offset = 0) override;
	void DrawIndexedIndirect(const BvBuffer * const pBuffer, const u32 drawCount = 1, const u64 offset = 0) override;
	void DispatchIndirect(const BvBuffer * const pBuffer, const u64 offset = 0) override;

	void CopyBuffer(const BvBuffer * const pSrcBuffer, BvBuffer * const pDstBuffer) override;
	void CopyBufferRegion(const BvBuffer * const pSrcBuffer, BvBuffer * const pDstBuffer, const CopyRegion & copyRegion) override;

	void CopyTexture(const BvTexture * const pSrcTexture, BvTexture * const pDstTexture) override;
	void CopyTextureRegion(const BvTexture * const pSrcTexture, BvTexture * const pDstTexture, const CopyRegion & copyRegion) override;

	void CopyTextureRegion(const BvBuffer * const pSrcBuffer, BvTexture * const pDstTexture, const CopyRegion & copyRegion) override;
	void CopyTextureRegion(const BvTexture * const pSrcTexture, BvBuffer * const pDstBuffer, const CopyRegion & copyRegion) override;

	void ResourceBarrier(const u32 barrierCount, const ResourceBarrierDesc * const pBarriers) override;

	BV_INLINE const VkCommandBuffer GetHandle() const { return m_CommandBuffer; }
	BV_INLINE const BvVector<BvSwapChainVk*>& GetSwapChains() const { return m_SwapChains; }
	BV_INLINE VkPipelineStageFlags GetWaitStageFlags() const { return m_WaitStageFlags; }

private:
	void CommitGraphicsData();
	void CommitComputeData();
	void CommitRenderTargets();
	void DecommitRenderTargets();
	void ClearStateData();

private:
	const BvRenderDeviceVk & m_Device;
	VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;

	VkPipelineStageFlags m_WaitStageFlags = 0;

	BvVector<BvSwapChainVk*> m_SwapChains;
	BvVector<VkImageMemoryBarrier> m_RenderTargetTransitions;
	VkPipelineStageFlags m_RenderTargetSrcStageFlags = 0;
	VkPipelineStageFlags m_RenderTargetDstStageFlags = 0;
	BvVector<VkMemoryBarrier> m_MemoryBarriers;
	BvVector<VkBufferMemoryBarrier> m_BufferBarriers;
	BvVector<VkImageMemoryBarrier> m_ImageBarriers;
	
	BvRenderPassVk* m_pRenderPass = nullptr;
	BvVector<BvTextureViewVk*> m_RenderTargets;
	BvVector<VkClearValue> m_RenderTargetClearValues;
	BvVector<VkAttachmentLoadOp> m_RenderTargetLoadOps;
	BvTextureViewVk* m_pDepthStencilTarget = nullptr;
	VkClearValue m_DepthStencilTargetClearValue{};
	VkAttachmentLoadOp m_DepthTargetLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
	VkAttachmentLoadOp m_StencilTargetLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
	const BvGraphicsPipelineStateVk* m_pGraphicsPSO = nullptr;
	const BvComputePipelineStateVk* m_pComputePSO = nullptr;
	BvVector<VkViewport> m_Viewports;
	BvVector<VkRect2D> m_Scissors;
	BvVector<VkBuffer> m_VertexBuffers;
	BvVector<VkDeviceSize> m_VertexBufferOffsets;
	const BvBufferViewVk* m_pIndexBufferView = nullptr;
	BvVector<VkDescriptorSet> m_DescriptorSets;
	u32 m_FirstVertexBinding = 0;
	u32 m_FirstDescriptorSet = 0;
	VkIndexType m_IndexFormat = VK_INDEX_TYPE_UINT32;

	bool m_RenderTargetsBindNeeded = false;
	bool m_GraphicsPSOBindNeeded = false;
	bool m_ComputePSOBindNeeded = false;
	bool m_ViewportsBindNeeded = false;
	bool m_ScissorsBindNeeded = false;
	bool m_VertexBuffersBindNeeded = false;
	bool m_IndexBufferBindNeeded = false;
	bool m_DescriptorSetBindNeeded = false;
};