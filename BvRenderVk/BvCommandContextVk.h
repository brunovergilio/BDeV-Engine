#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/RenderAPI/BvCommandContext.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/Container/BvRobinMap.h"
#include "BvCommandPoolVk.h"
#include "BvDescriptorSetVk.h"
#include "BvSemaphoreVk.h"
#include "BvCommandQueueVk.h"
#include "BvFramebufferVk.h"


class BvRenderDeviceVk;
class BvBufferVk;
class BvTextureVk;
class BvQueryHeapManagerVk;
class BvQueryVk;
class BvSwapChainVk;
class BvGPUFenceVk;
class BvFramebufferManagerVk;


class BvFrameDataVk final
{
	BV_NOCOPY(BvFrameDataVk);

public:
	BvFrameDataVk();
	BvFrameDataVk(const BvRenderDeviceVk* pDevice, u32 queueFamilyIndex, u32 frameIndex, BvQueryHeapManagerVk* pQueryHeapManager);
	BvFrameDataVk(BvFrameDataVk&& rhs) noexcept;
	BvFrameDataVk& operator=(BvFrameDataVk&& rhs) noexcept;
	~BvFrameDataVk();

	void Reset();
	BvCommandBufferVk* RequestCommandBuffer();
	VkDescriptorSet RequestDescriptorSet(u32 set, const BvShaderResourceLayoutVk* pLayout, BvVector<VkWriteDescriptorSet>& writeSets);
	void UpdateSignalIndex(u64 value);
	void UpdateSignalValue();
	void ClearActiveCommandBuffers();
	void AddQuery(BvQueryVk* pQuery);
	VkFramebuffer GetFramebuffer(const FramebufferDesc& fbDesc);
	void RemoveFramebuffers(VkImageView view);
	void UpdateQueryData();
	
	BV_INLINE const auto& GetCommandBuffers() const { return m_CommandBuffers; }
	BV_INLINE auto& GetResourceBindingState() { return m_ResourceBindingState; }
	BV_INLINE BvGPUFenceVk* GetGPUFence() { return m_pFence; }
	BV_INLINE std::pair<u64, u64> GetSemaphoreValueIndex() const { return m_SignaValueIndex; }
	BV_INLINE u32 GetFrameIndex() const { return m_FrameIndex; }
	BV_INLINE BvQueryHeapManagerVk* GetQueryHeapManager() const { return m_pQueryHeapManager; }

private:
	const BvRenderDeviceVk* m_pDevice = nullptr;
	BvCommandPoolVk m_CommandPool;
	BvVector<BvCommandBufferVk*> m_CommandBuffers;
	BvResourceBindingStateVk m_ResourceBindingState;
	BvRobinMap<u64, BvDescriptorPoolVk> m_DescriptorPools;
	BvRobinMap<u64, BvDescriptorSetVk> m_DescriptorSets;
	BvQueryHeapManagerVk* m_pQueryHeapManager = nullptr;
	//BvRobinMap<u64, BvDescriptorSetVk> m_BindlessDescriptorSets;
	BvFramebufferManagerVk* m_pFramebufferManager = nullptr;
	BvVector<BvQueryVk*> m_Queries;
	u32 m_UpdatedQueries = 0;
	BvGPUFenceVk* m_pFence = nullptr;
	std::pair<u64, u64> m_SignaValueIndex;
	u32 m_FrameIndex;
};


class BvCommandContextVk final : public BvCommandContext
{
	BV_NOCOPYMOVE(BvCommandContextVk);

public:
	BvCommandContextVk(BvRenderDeviceVk* pDevice, u32 frameCount, CommandType queueFamilyType, u32 queueFamilyIndex, u32 queueIndex);
	~BvCommandContextVk();

	BvGPUOp Execute() override;
	BvGPUOp Execute(u64 value) override;
	void Execute(BvGPUFence* pFence, u64 value) override;
	void Wait(BvCommandContext* pCommandContext, u64 value) override;
	void NewCommandList() override;
	void FlushFrame() override;
	void WaitForGPU() override;

	void BeginRenderPass(const BvRenderPass* pRenderPass, u32 renderPassTargetCount, const RenderPassTargetDesc* pRenderPassTargets) override;
	void EndRenderPass() override;

	void SetRenderTargets(u32 renderTargetCount, const RenderTargetDesc* pRenderTargets) override;

	void SetViewports(u32 viewportCount, const Viewport* pViewports) override;
	void SetScissors(u32 scissorCount, const Rect* pScissors) override;

	void SetGraphicsPipeline(const BvGraphicsPipelineState* pPipeline) override;
	void SetComputePipeline(const BvComputePipelineState* pPipeline) override;

	void SetShaderResourceParams(u32 resourceParamsCount, BvShaderResourceParams* const* ppResourceParams, u32 startIndex) override;
	void SetConstantBuffers(u32 count, const BvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) override;
	void SetStructuredBuffers(u32 count, const BvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) override;
	void SetRWStructuredBuffers(u32 count, const BvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) override;
	void SetFormattedBuffers(u32 count, const BvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) override;
	void SetRWFormattedBuffers(u32 count, const BvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) override;
	void SetTextures(u32 count, const BvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) override;
	void SetRWTextures(u32 count, const BvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) override;
	void SetSamplers(u32 count, const BvSampler* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) override;
	void SetShaderConstants(u32 size, const void* pData, u32 offset) override;

	void SetVertexBufferViews(u32 vertexBufferCount, const BvBufferView* const* pVertexBufferViews, u32 firstBinding = 0) override;
	void SetIndexBufferView(const BvBufferView* pIndexBufferView, IndexFormat indexFormat) override;

	void Draw(u32 vertexCount, u32 instanceCount = 1, u32 firstVertex = 0, u32 firstInstance = 0) override;
	void DrawIndexed(u32 indexCount, u32 instanceCount = 1, u32 firstIndex = 0, i32 vertexOffset = 0, u32 firstInstance = 0) override;
	void Dispatch(u32 x, u32 y = 1, u32 z = 1) override;

	void DrawIndirect(const BvBuffer* pBuffer, u32 drawCount = 1, u64 offset = 0) override;
	void DrawIndexedIndirect(const BvBuffer* pBuffer, u32 drawCount = 1, u64 offset = 0) override;
	void DispatchIndirect(const BvBuffer* pBuffer, u64 offset = 0) override;

	void DispatchMesh(u32 x, u32 y = 1, u32 z = 1) override;
	void DispatchMeshIndirect(const BvBuffer* pBuffer, u64 offset = 0) override;
	void DispatchMeshIndirectCount(const BvBuffer* pBuffer, u64 offset, const BvBuffer* pCountBuffer, u64 countOffset, u32 maxCount) override;

	void CopyBuffer(const BvBufferVk* pSrcBuffer, BvBufferVk* pDstBuffer, const VkBufferCopy& copyRegion);
	void CopyBuffer(const BvBuffer* pSrcBuffer, BvBuffer* pDstBuffer) override;
	void CopyBuffer(const BvBuffer* pSrcBuffer, BvBuffer* pDstBuffer, const BufferCopyDesc& copyDesc) override;

	void CopyTexture(const BvTexture* pSrcTexture, BvTexture* pDstTexture) override;
	void CopyTexture(const BvTexture* pSrcTexture, BvTexture* pDstTexture, const TextureCopyDesc& copyDesc) override;

	void CopyBufferToTexture(const BvBufferVk* pSrcBuffer, BvTextureVk* pDstTexture, u32 copyCount, const VkBufferImageCopy* pCopyRegions);
	void CopyBufferToTexture(const BvBuffer* pSrcBuffer, BvTexture* pDstTexture, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs) override;

	void CopyTextureToBuffer(const BvTextureVk* pSrcTexture, BvBufferVk* pDstBuffer, u32 copyCount, const VkBufferImageCopy* pCopyRegions);
	void CopyTextureToBuffer(const BvTexture* pSrcTexture, BvBuffer* pDstBuffer, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs) override;

	void ResourceBarrier(u32 bufferBarrierCount, const VkBufferMemoryBarrier2* pBufferBarriers,
		u32 imageBarrierCount, const VkImageMemoryBarrier2* pImageBarriers, u32 memoryBarrierCount, const VkMemoryBarrier2* pMemoryBarriers);
	void ResourceBarrier(u32 barrierCount, const ResourceBarrierDesc* pBarriers) override;

	void SetPredication(const BvBuffer* pBuffer, u64 offset, PredicationOp predicationOp) override;

	void BeginQuery(BvQuery* pQuery) override;
	void EndQuery(BvQuery* pQuery) override;

	virtual void BeginEvent(const char* pName, const BvColor& color = BvColor::Black) override;
	virtual void EndEvent() override;
	virtual void SetMarker(const char* pName, const BvColor& color = BvColor::Black) override;

	BV_INLINE BvCommandQueueVk* GetCommandQueue() { return &m_Queue; }
	BV_INLINE BvGPUFenceVk* GetCurrentGPUFence() { return m_Frames[m_ActiveFrameIndex].GetGPUFence(); }
	BV_INLINE u64 GetCurrentValue() { return m_Frames[m_ActiveFrameIndex].GetSemaphoreValueIndex().first; }
	BV_INLINE BvCommandBufferVk* GetCurrentCommandBuffer() const { return m_pCurrCommandBuffer; }

	void AddSwapChain(BvSwapChainVk* pSwapChain);
	void RemoveSwapChain(BvSwapChainVk* pSwapChain);
	void RemoveFramebuffers(VkImageView view);

private:
	BvCommandQueueVk m_Queue;
	BvVector<BvFrameDataVk> m_Frames;
	BvVector<BvSwapChainVk*> m_SwapChains;
	BvCommandBufferVk* m_pCurrCommandBuffer = nullptr;
	BvFrameDataVk* m_pCurrFrame = nullptr;
	BvQueryHeapManagerVk* m_pQueryHeapManager = nullptr; // Query Manager can be placed here as it's used for all frames
	u32 m_ActiveFrameIndex = 0;
};


BV_CREATE_CAST_TO_VK(BvCommandContext)