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


class IBvRenderDeviceVk;
class IBvBufferVk;
class IBvTextureVk;
class IBvQueryVk;
class IBvSwapChainVk;
class IBvGPUFenceVk;
class BvQueryHeapManagerVk;
class BvFramebufferManagerVk;


struct ContextDataVk
{
	BvResourceBindingStateVk m_ResourceBindingState;
	BvRobinMap<u64, BvDescriptorPoolVk> m_DescriptorPools;
	BvRobinMap<u64, BvDescriptorSetVk> m_DescriptorSets;
	BvRobinMap<u64, BvDescriptorSetVk> m_BindlessDescriptorSets;
	BvQueryHeapManagerVk* m_pQueryHeapManager = nullptr;
	BvFramebufferManagerVk* m_pFramebufferManager = nullptr;
};


class BvFrameDataVk final
{
	BV_NOCOPY(BvFrameDataVk);

public:
	BvFrameDataVk();
	BvFrameDataVk(IBvRenderDeviceVk* pDevice, u32 queueFamilyIndex, u32 frameIndex, ContextDataVk* pContextData);
	BvFrameDataVk(BvFrameDataVk&& rhs) noexcept;
	BvFrameDataVk& operator=(BvFrameDataVk&& rhs) noexcept;
	~BvFrameDataVk();

	void Reset();
	BvCommandBufferVk* RequestCommandBuffer();
	VkDescriptorSet RequestDescriptorSet(u32 set, const IBvShaderResourceLayoutVk* pLayout, BvVector<VkWriteDescriptorSet>& writeSets, bool bindless = false);
	void UpdateSignalIndex(u64 value);
	void UpdateSignalValue();
	void ClearActiveCommandBuffers();
	void AddQuery(IBvQueryVk* pQuery);
	VkFramebuffer GetFramebuffer(const FramebufferDesc& fbDesc);
	void RemoveFramebuffers(VkImageView view);
	void UpdateQueryData();
	
	BV_INLINE const auto& GetCommandBuffers() const { return m_CommandBuffers; }
	BV_INLINE auto& GetResourceBindingState() { return m_pContextData->m_ResourceBindingState; }
	BV_INLINE IBvGPUFenceVk* GetGPUFence() { return m_pFence; }
	BV_INLINE std::pair<u64, u64> GetSemaphoreValueIndex() const { return m_SignaValueIndex; }
	BV_INLINE u32 GetFrameIndex() const { return m_FrameIndex; }
	BV_INLINE BvQueryHeapManagerVk* GetQueryHeapManager() const { return m_pContextData->m_pQueryHeapManager; }

private:
	IBvRenderDeviceVk* m_pDevice = nullptr;
	BvCommandPoolVk m_CommandPool;
	BvVector<BvCommandBufferVk*> m_CommandBuffers;
	ContextDataVk* m_pContextData;
	BvVector<IBvQueryVk*> m_Queries;
	u32 m_UpdatedQueries = 0;
	IBvGPUFenceVk* m_pFence = nullptr;
	std::pair<u64, u64> m_SignaValueIndex;
	u32 m_FrameIndex;
};


BV_OBJECT_DEFINE_ID(IBvCommandContextVk, "87f18018-a53f-43e7-b7f5-b39d662d0ce5");
class IBvCommandContextVk : public IBvCommandContext
{
	BV_NOCOPYMOVE(IBvCommandContextVk);

public:
	virtual void CopyBufferVk(const IBvBufferVk* pSrcBuffer, IBvBufferVk* pDstBuffer, const VkBufferCopy& copyRegion) = 0;
	virtual void CopyBufferToTextureVk(const IBvBufferVk* pSrcBuffer, IBvTextureVk* pDstTexture, u32 copyCount, const VkBufferImageCopy* pCopyRegions) = 0;
	virtual void CopyTextureToBufferVk(const IBvTextureVk* pSrcTexture, IBvBufferVk* pDstBuffer, u32 copyCount, const VkBufferImageCopy* pCopyRegions) = 0;
	virtual void ResourceBarrierVk(u32 bufferBarrierCount, const VkBufferMemoryBarrier2* pBufferBarriers,
		u32 imageBarrierCount, const VkImageMemoryBarrier2* pImageBarriers, u32 memoryBarrierCount, const VkMemoryBarrier2* pMemoryBarriers) = 0;
	virtual BvCommandQueueVk* GetCommandQueue() = 0;
	virtual IBvGPUFenceVk* GetCurrentGPUFence() = 0;
	virtual u64 GetCurrentValue() = 0;
	virtual BvCommandBufferVk* GetCurrentCommandBuffer() const = 0;

	virtual void AddSwapChain(IBvSwapChainVk* pSwapChain) = 0;
	virtual void RemoveSwapChain(IBvSwapChainVk* pSwapChain) = 0;
	virtual void RemoveFramebuffers(VkImageView view) = 0;

protected:
	IBvCommandContextVk() {}
	~IBvCommandContextVk() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvCommandContextVk);


class BvCommandContextVk final : public IBvCommandContextVk
{
	BV_NOCOPYMOVE(BvCommandContextVk);

public:
	BvCommandContextVk(IBvRenderDeviceVk* pDevice, u32 frameCount, CommandType queueFamilyType, u32 queueFamilyIndex, u32 queueIndex);
	~BvCommandContextVk();

	BvGPUOp Execute() override;
	BvGPUOp Execute(u64 value) override;
	void Execute(IBvGPUFence* pFence, u64 value) override;
	void Wait(IBvCommandContext* pCommandContext, u64 value) override;
	void NewCommandList() override;
	void FlushFrame() override;
	void WaitForGPU() override;

	void BeginRenderPass(const IBvRenderPass* pRenderPass, u32 renderPassTargetCount, const RenderPassTargetDesc* pRenderPassTargets) override;
	void EndRenderPass() override;

	void SetRenderTargets(u32 renderTargetCount, const RenderTargetDesc* pRenderTargets) override;

	void SetViewports(u32 viewportCount, const Viewport* pViewports) override;
	void SetScissors(u32 scissorCount, const Rect* pScissors) override;

	void SetGraphicsPipeline(const IBvGraphicsPipelineState* pPipeline) override;
	void SetComputePipeline(const IBvComputePipelineState* pPipeline) override;
	void SetRayTracingPipeline(const IBvRayTracingPipelineState* pPipeline) override;

	void SetShaderResourceParams(u32 resourceParamsCount, BvShaderResourceParams* const* ppResourceParams, u32 startIndex) override;
	void SetConstantBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) override;
	void SetStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) override;
	void SetRWStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) override;
	void SetDynamicConstantBuffers(u32 count, const IBvBufferView* const* ppResources, const u32* pOffsets, u32 set, u32 binding, u32 startIndex = 0) override;
	void SetDynamicStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, const u32* pOffsets, u32 set, u32 binding, u32 startIndex = 0) override;
	void SetDynamicRWStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, const u32* pOffsets, u32 set, u32 binding, u32 startIndex = 0) override;
	void SetFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) override;
	void SetRWFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) override;
	void SetTextures(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) override;
	void SetRWTextures(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) override;
	void SetSamplers(u32 count, const IBvSampler* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) override;
	void SetAccelerationStructures(u32 count, const IBvAccelerationStructure* const* ppResources, u32 set, u32 binding, u32 startIndex) override;
	void SetShaderConstants(u32 size, const void* pData, u32 binding, u32 set) override;

	void SetVertexBufferViews(u32 vertexBufferCount, const IBvBufferView* const* pVertexBufferViews, u32 firstBinding = 0) override;
	void SetIndexBufferView(const IBvBufferView* pIndexBufferView, IndexFormat indexFormat) override;

	void Draw(const DrawCommandArgs& args) override;
	void DrawIndexed(const DrawIndexedCommandArgs& args) override;
	void Dispatch(const DispatchCommandArgs& args) override;
	void DispatchMesh(const DispatchMeshCommandArgs& args) override;

	void DrawIndirect(const IBvBuffer* pBuffer, u32 drawCount = 1, u64 offset = 0) override;
	void DrawIndexedIndirect(const IBvBuffer* pBuffer, u32 drawCount = 1, u64 offset = 0) override;
	void DispatchIndirect(const IBvBuffer* pBuffer, u64 offset = 0) override;
	void DispatchMeshIndirect(const IBvBuffer* pBuffer, u64 offset = 0) override;
	void DispatchMeshIndirectCount(const IBvBuffer* pBuffer, u64 offset, const IBvBuffer* pCountBuffer, u64 countOffset, u32 maxCount) override;

	void CopyBufferVk(const IBvBufferVk* pSrcBuffer, IBvBufferVk* pDstBuffer, const VkBufferCopy& copyRegion) override;
	void CopyBuffer(const IBvBuffer* pSrcBuffer, IBvBuffer* pDstBuffer) override;
	void CopyBuffer(const IBvBuffer* pSrcBuffer, IBvBuffer* pDstBuffer, const BufferCopyDesc& copyDesc) override;

	void CopyTexture(const IBvTexture* pSrcTexture, IBvTexture* pDstTexture) override;
	void CopyTexture(const IBvTexture* pSrcTexture, IBvTexture* pDstTexture, const TextureCopyDesc& copyDesc) override;

	void CopyBufferToTextureVk(const IBvBufferVk* pSrcBuffer, IBvTextureVk* pDstTexture, u32 copyCount, const VkBufferImageCopy* pCopyRegions) override;
	void CopyBufferToTexture(const IBvBuffer* pSrcBuffer, IBvTexture* pDstTexture, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs) override;

	void CopyTextureToBufferVk(const IBvTextureVk* pSrcTexture, IBvBufferVk* pDstBuffer, u32 copyCount, const VkBufferImageCopy* pCopyRegions) override;
	void CopyTextureToBuffer(const IBvTexture* pSrcTexture, IBvBuffer* pDstBuffer, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs) override;

	void ResourceBarrierVk(u32 bufferBarrierCount, const VkBufferMemoryBarrier2* pBufferBarriers,
		u32 imageBarrierCount, const VkImageMemoryBarrier2* pImageBarriers, u32 memoryBarrierCount, const VkMemoryBarrier2* pMemoryBarriers) override;
	void ResourceBarrier(u32 barrierCount, const ResourceBarrierDesc* pBarriers) override;

	void SetPredication(const IBvBuffer* pBuffer, u64 offset, PredicationOp predicationOp) override;

	void BeginQuery(IBvQuery* pQuery) override;
	void EndQuery(IBvQuery* pQuery) override;

	void BeginEvent(const char* pName, const BvColor& color = BvColor::Black) override;
	void EndEvent() override;
	void SetMarker(const char* pName, const BvColor& color = BvColor::Black) override;

	void BuildBLAS(const BLASBuildDesc& desc) override;
	void BuildTLAS(const TLASBuildDesc& desc) override;
	void DispatchRays(const DispatchRaysCommandArgs& args) override;
	void DispatchRays(IBvShaderBindingTable* pSBT, u32 rayGenIndex, u32 missIndex, u32 hitIndex, u32 callableIndex,
		u32 width, u32 height, u32 depth) override;

	void DispatchRaysIndirect(const IBvBuffer* pBuffer, u64 offset = 0) override;

	BV_INLINE BvCommandQueueVk* GetCommandQueue() override { return &m_Queue; }
	BV_INLINE IBvGPUFenceVk* GetCurrentGPUFence() override { return m_Frames[m_ActiveFrameIndex].GetGPUFence(); }
	BV_INLINE u64 GetCurrentValue() override { return m_Frames[m_ActiveFrameIndex].GetSemaphoreValueIndex().first; }
	BV_INLINE BvCommandBufferVk* GetCurrentCommandBuffer() const override { return m_pCurrCommandBuffer; }

	void AddSwapChain(IBvSwapChainVk* pSwapChain) override;
	void RemoveSwapChain(IBvSwapChainVk* pSwapChain) override;
	void RemoveFramebuffers(VkImageView view) override;
	IBvRenderDevice* GetDevice() override;

	BV_OBJECT_IMPL_INTERFACE(IBvCommandContextVk, IBvCommandContext, IBvRenderDeviceObject);

private:
	IBvRenderDeviceVk* m_pDevice = nullptr;
	BvCommandQueueVk m_Queue;
	BvVector<BvFrameDataVk> m_Frames;
	BvVector<IBvSwapChainVk*> m_SwapChains;
	BvCommandBufferVk* m_pCurrCommandBuffer = nullptr;
	BvFrameDataVk* m_pCurrFrame = nullptr;
	ContextDataVk* m_pContextData;
	u32 m_ActiveFrameIndex = 0;
};


BV_CREATE_CAST_TO_VK(IBvCommandContext)