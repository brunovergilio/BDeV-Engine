#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/RenderAPI/BvCommandContext.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/Container/BvRobinMap.h"
#include "BvCommandPoolVk.h"
#include "BvDescriptorSetVk.h"
#include "BvCommandQueueVk.h"
#include "BvFramebufferVk.h"


class BvRenderDeviceVk;
class BvBufferVk;
class BvTextureVk;
class BvQueryVk;
class BvSwapChainVk;
class BvGPUFenceVk;
class BvQueryHeapManagerVk;
class BvQueryASVk;
class BvFramebufferManagerVk;


struct ContextDataVk
{
	BvResourceBindingStateVk m_ResourceBindingState;
	BvRobinMap<u64, BvDescriptorPoolVk> m_DescriptorPools;
	BvRobinMap<u64, BvDescriptorSetVk> m_DescriptorSets;
	BvRobinMap<u64, BvDescriptorSetVk> m_BindlessDescriptorSets;
	BvQueryHeapManagerVk* m_pQueryHeapManager = nullptr;
	BvFramebufferManagerVk* m_pFramebufferManager = nullptr;
	BvQueryASVk* m_pASQueries = nullptr;
};


class BvFrameDataVk final
{
	BV_NOCOPY(BvFrameDataVk);

public:
	BvFrameDataVk();
	BvFrameDataVk(BvRenderDeviceVk* pDevice, u32 queueFamilyIndex, u32 frameIndex, ContextDataVk* pContextData);
	BvFrameDataVk(BvFrameDataVk&& rhs) noexcept;
	BvFrameDataVk& operator=(BvFrameDataVk&& rhs) noexcept;
	~BvFrameDataVk();

	void Reset(bool resetQueries = true);
	BvCommandBufferVk* RequestCommandBuffer();
	VkDescriptorSet RequestDescriptorSet(u32 set, const BvShaderResourceLayoutVk* pLayout, BvVector<VkWriteDescriptorSet>& writeSets, u32 hashSeed, bool bindless = false);
	void UpdateSignalIndex(u64 value);
	void UpdateSignalValue();
	void ClearActiveCommandBuffers();
	void AddQuery(BvQueryVk* pQuery);
	VkFramebuffer GetFramebuffer(const FramebufferDesc& fbDesc);
	void RemoveFramebuffers(VkImageView view);
	void UpdateQueryData();
	
	BV_INLINE const auto& GetCommandBuffers() const { return m_CommandBuffers; }
	BV_INLINE auto& GetResourceBindingState() { return m_pContextData->m_ResourceBindingState; }
	BV_INLINE BvGPUFenceVk* GetGPUFence() { return m_pFence; }
	BV_INLINE std::pair<u64, u64> GetSemaphoreValueIndex() const { return m_SignaValueIndex; }
	BV_INLINE u32 GetFrameIndex() const { return m_FrameIndex; }
	BV_INLINE BvQueryHeapManagerVk* GetQueryHeapManager() const { return m_pContextData->m_pQueryHeapManager; }
	BV_INLINE BvQueryASVk* GetQueryAS() const { return m_pContextData->m_pASQueries; }

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	BvCommandPoolVk m_CommandPool;
	BvVector<BvCommandBufferVk*> m_CommandBuffers;
	ContextDataVk* m_pContextData;
	BvVector<BvQueryVk*> m_Queries;
	u32 m_UpdatedQueries = 0;
	BvGPUFenceVk* m_pFence = nullptr;
	std::pair<u64, u64> m_SignaValueIndex;
	u32 m_FrameIndex;
};


//BV_OBJECT_DEFINE_ID(IBvCommandContextVk, "87f18018-a53f-43e7-b7f5-b39d662d0ce5");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvCommandContextVk);


class BvCommandContextVk final : public IBvCommandContext, public IBvResourceVk
{
	BV_NOCOPYMOVE(BvCommandContextVk);
	BV_VK_DEVICE_RES_DECL;

public:
	BvCommandContextVk(BvRenderDeviceVk* pDevice, u32 frameCount, u32 queueFamilyIndex, u32 queueIndex);
	~BvCommandContextVk();

	BV_INLINE u32 GetGroupIndex() const override { return m_ContextGroupIndex; }
	BV_INLINE u32 GetIndex() const override { return m_ContextIndex; }

	BvGPUOp Execute() override;
	BvGPUOp Execute(u64 value) override;
	void Execute(IBvGPUFence* pFence, u64 value) override;
	void ExecuteAndWait() override;
	void Wait(IBvCommandContext* pCommandContext, u64 value) override;
	void NewCommandList() override;
	void FlushFrame() override;

	void BeginRenderPass(const IBvRenderPass* pRenderPass, u32 renderPassTargetCount, const RenderPassTargetDesc* pRenderPassTargets) override;
	void NextSubpass() override;
	void EndRenderPass() override;

	void SetRenderTargets(u32 renderTargetCount, const RenderTargetDesc* pRenderTargets, u32 multiviewCount = 0) override;

	void SetViewports(u32 viewportCount, const Viewport* pViewports) override;
	void SetScissors(u32 scissorCount, const Rect* pScissors) override;

	void SetGraphicsPipeline(const IBvGraphicsPipelineState* pPipeline) override;
	void SetComputePipeline(const IBvComputePipelineState* pPipeline) override;
	void SetRayTracingPipeline(const IBvRayTracingPipelineState* pPipeline) override;

	void SetShaderResourceParams(u32 resourceParamsCount, IBvShaderResourceParams* const* ppResourceParams, u32 startIndex) override;
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
	void SetInputAttachments(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0) override;
	void SetAccelerationStructures(u32 count, const IBvAccelerationStructure* const* ppResources, u32 set, u32 binding, u32 startIndex) override;
	void SetShaderConstants(u32 size, const void* pData, u32 binding, u32 set) override;

	void SetVertexBufferViews(u32 vertexBufferCount, const VertexBufferView* pVertexBufferViews, u32 firstBinding = 0) override;
	void SetIndexBufferView(const IndexBufferView& indexBufferView) override;

	void SetDepthBounds(f32 min, f32 max) override;
	void SetStencilRef(u32 stencilRef) override;
	void SetBlendConstants(const float(pColors[4])) override;
	void SetShadingRate(ShadingRateDimensions dimensions, ShadingRateCombinerOp(pCombinerOps[2])) override;

	void Draw(const DrawCommandArgs& args) override;
	void DrawIndexed(const DrawIndexedCommandArgs& args) override;
	void Dispatch(const DispatchCommandArgs& args) override;
	void DispatchMesh(const DispatchMeshCommandArgs& args) override;

	void DrawIndirect(const IBvBuffer* pBuffer, u32 drawCount = 1, u64 offset = 0) override;
	void DrawIndexedIndirect(const IBvBuffer* pBuffer, u32 drawCount = 1, u64 offset = 0) override;
	void DispatchIndirect(const IBvBuffer* pBuffer, u64 offset = 0) override;
	void DispatchMeshIndirect(const IBvBuffer* pBuffer, u64 offset = 0) override;
	void DispatchMeshIndirectCount(const IBvBuffer* pBuffer, u64 offset, const IBvBuffer* pCountBuffer, u64 countOffset, u32 maxCount) override;

	void CopyBufferVk(const BvBufferVk* pSrcBuffer, BvBufferVk* pDstBuffer, const VkBufferCopy& copyRegion);
	void CopyBuffer(const IBvBuffer* pSrcBuffer, IBvBuffer* pDstBuffer) override;
	void CopyBuffer(const IBvBuffer* pSrcBuffer, IBvBuffer* pDstBuffer, const BufferCopyDesc& copyDesc) override;

	void CopyTexture(const IBvTexture* pSrcTexture, IBvTexture* pDstTexture) override;
	void CopyTexture(const IBvTexture* pSrcTexture, IBvTexture* pDstTexture, const TextureCopyDesc& copyDesc) override;

	void CopyBufferToTextureVk(const BvBufferVk* pSrcBuffer, BvTextureVk* pDstTexture, u32 copyCount, const VkBufferImageCopy* pCopyRegions);
	void CopyBufferToTexture(const IBvBuffer* pSrcBuffer, IBvTexture* pDstTexture, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs) override;

	void CopyTextureToBufferVk(const BvTextureVk* pSrcTexture, BvBufferVk* pDstBuffer, u32 copyCount, const VkBufferImageCopy* pCopyRegions);
	void CopyTextureToBuffer(const IBvTexture* pSrcTexture, IBvBuffer* pDstBuffer, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs) override;

	void ResourceBarrierVk(u32 bufferBarrierCount, const VkBufferMemoryBarrier2* pBufferBarriers,
		u32 imageBarrierCount, const VkImageMemoryBarrier2* pImageBarriers, u32 memoryBarrierCount, const VkMemoryBarrier2* pMemoryBarriers);
	void ResourceBarrier(u32 barrierCount, const ResourceBarrierDesc* pBarriers) override;

	void SetPredication(const IBvBuffer* pBuffer, u64 offset, PredicationOp predicationOp) override;

	bool SupportsQueryType(QueryType queryType) const override;
	void BeginQuery(IBvQuery* pQuery) override;
	void EndQuery(IBvQuery* pQuery) override;

	void BeginEvent(const char* pName, const BvColor& color = BvColor::Black) override;
	void EndEvent() override;
	void SetMarker(const char* pName, const BvColor& color = BvColor::Black) override;

	void BuildBLAS(const BLASBuildDesc& desc, const ASPostBuildDesc* pPostBuildDesc = nullptr) override;
	void BuildTLAS(const TLASBuildDesc& desc, const ASPostBuildDesc* pPostBuildDesc = nullptr) override;
	void CopyBLAS(const AccelerationStructureCopyDesc& copyDesc) override;
	void CopyTLAS(const AccelerationStructureCopyDesc& copyDesc) override;
	void DispatchRays(const DispatchRaysCommandArgs& args) override;
	void DispatchRays(IBvShaderBindingTable* pSBT, u32 rayGenIndex, u32 missIndex, u32 hitIndex, u32 callableIndex,
		u32 width, u32 height, u32 depth) override;

	void DispatchRaysIndirect(const IBvBuffer* pBuffer, u64 offset = 0) override;

	BV_INLINE BvCommandQueueVk* GetCommandQueue() { return &m_Queue; }
	BV_INLINE BvGPUFenceVk* GetCurrentGPUFence() { return m_Frames[m_ActiveFrameIndex].GetGPUFence(); }
	BV_INLINE u64 GetCurrentValue() { return m_Frames[m_ActiveFrameIndex].GetSemaphoreValueIndex().first; }
	BV_INLINE BvCommandBufferVk* GetCurrentCommandBuffer() const { return m_pCurrCommandBuffer; }

	void AddSwapChain(BvSwapChainVk* pSwapChain);
	void RemoveSwapChain(BvSwapChainVk* pSwapChain);
	void RemoveFramebuffers(VkImageView view);

	//BV_OBJECT_IMPL_INTERFACE(IBvCommandContextVk, IBvCommandContext, IBvRenderDeviceObject);

private:
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	BvCommandQueueVk m_Queue;
	BvVector<BvFrameDataVk> m_Frames;
	BvVector<BvSwapChainVk*> m_SwapChains;
	BvCommandBufferVk* m_pCurrCommandBuffer = nullptr;
	BvFrameDataVk* m_pCurrFrame = nullptr;
	ContextDataVk* m_pContextData;
	u32 m_ActiveFrameIndex = 0;
	u32 m_ContextGroupIndex = 0;
	u32 m_ContextIndex = 0;
};


BV_CREATE_CAST_TO_VK(BvCommandContext)