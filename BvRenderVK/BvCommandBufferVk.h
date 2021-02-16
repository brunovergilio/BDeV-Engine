#pragma once


#include "BvRenderVK/BvCommonVk.h"
#include "BvCore/Container/BvVector.h"
#include "BvRender/BvCommandBuffer.h"
#include "BvRender/BvCommon.h"
#include "BvCore/Container/BvFixedVector.h"


constexpr u32 kMaxSemaphores = 8;


class BvCommandPoolVk;
class BvRenderDeviceVk;
class BvTextureViewVk;
class BvSwapChainVk;


class BvCommandBufferVk final : public BvCommandBuffer
{
public:
	BvCommandBufferVk(const BvRenderDeviceVk & device, BvCommandPool * pCommandPool, const VkCommandBuffer commandBuffer);
	~BvCommandBufferVk();

	void Reset() override final;
	void Begin() override final;
	void End() override final;
	void BeginRenderPass(const BvRenderPass * const pRenderPass, BvTextureView * const * const pRenderTargets,
		const ClearColorValue * const pClearColors, BvTextureView * const pDepthStencilTarget,
		const ClearColorValue & depthClear) override final;
	void EndRenderPass() override final;

	void SetViewports(const u32 viewportCount, const Viewport * const pViewports) override final;
	void SetScissors(const u32 scissorCount, const Rect * const pScissors) override final;

	void SetPipeline(const BvGraphicsPipelineState * const pPipeline) override final;
	void SetPipeline(const BvComputePipelineState * const pPipeline) override final;

	void SetShaderResourceSets(const u32 setCount, BvShaderResourceSet * const * const ppSets, const u32 firstSet) override final;

	void SetVertexBuffers(const u32 vertexBufferCount, const BvBuffer * const * const pVertexBuffers,
		const u32 firstBinding = 0) override final;
	void SetIndexBuffer(const BvBuffer * const pIndexBuffer, const IndexFormat indexFormat) override final;

	void Draw(const u32 vertexCount, const u32 instanceCount = 1,
		const u32 firstVertex = 0, const uint32_t firstInstance = 0) override final;
	void DrawIndexed(const u32 indexCount, const u32 instanceCount = 1, const u32 firstIndex = 0,
		const i32 vertexOffset = 0, const u32 firstInstance = 0) override final;
	void Dispatch(const u32 x, const u32 y = 1, const u32 z = 1) override final;

	void DrawIndirect(const BvBuffer * const pBuffer, const u32 drawCount = 1, const u64 offset = 0) override final;
	void DrawIndexedIndirect(const BvBuffer * const pBuffer, const u32 drawCount = 1, const u64 offset = 0) override final;
	void DispatchIndirect(const BvBuffer * const pBuffer, const u64 offset = 0) override final;

	void CopyBuffer(const BvBuffer * const pSrcBuffer, BvBuffer * const pDstBuffer) override final;
	void CopyBufferRegion(const BvBuffer * const pSrcBuffer, BvBuffer * const pDstBuffer, const CopyRegion & copyRegion) override final;

	void CopyTexture(const BvTexture * const pSrcTexture, BvTexture * const pDstTexture) override final;
	void CopyTextureRegion(const BvTexture * const pSrcTexture, BvTexture * const pDstTexture, const CopyRegion & copyRegion) override final;

	void CopyTextureRegion(const BvBuffer * const pSrcBuffer, BvTexture * const pDstTexture, const CopyRegion & copyRegion) override final;
	void CopyTextureRegion(const BvTexture * const pSrcTexture, BvBuffer * const pDstBuffer, const CopyRegion & copyRegion) override final;

	void ResourceBarrier(const u32 barrierCount, const ResourceBarrierDesc * const pBarriers) override final;

	BV_INLINE const VkCommandBuffer GetHandle() const { return m_CommandBuffer; }
	BV_INLINE const BvVector<VkSemaphore> & GetSwapChainSignalSemaphores() const { return m_SwapChainSignalSemaphores; }
	BV_INLINE VkPipelineStageFlags GetWaitStageFlags() const { return m_WaitStageFlags; }

private:
	const BvRenderDeviceVk & m_Device;
	BvCommandPoolVk * m_pCommandPool;
	VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;

	VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
	VkPipelineBindPoint m_PipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_MAX_ENUM;

	BvFixedVector<VkImageMemoryBarrier, kMaxRenderTargetsWithDepth> m_RenderTargetTransitions;
	VkPipelineStageFlags m_RenderTargetSrcStageFlags = 0;
	VkPipelineStageFlags m_RenderTargetDstStageFlags = 0;

	BvVector<VkSemaphore> m_SwapChainSignalSemaphores;
	VkPipelineStageFlags m_WaitStageFlags = 0;
	bool m_IsInRenderPass = false;
};