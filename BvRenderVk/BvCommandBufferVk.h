#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/RenderAPI/BvRenderCommon.h"
#include "BDeV/Core/Container/BvFixedVector.h"


class BvRenderDeviceVk;
class BvSwapChainVk;
class BvGraphicsPipelineStateVk;
class BvComputePipelineStateVk;
class BvRayTracingPipelineStateVk;
class BvShaderResourceLayoutVk;
class IBvSampler;
class IBvBufferView;
class IBvTextureView;
class BvBufferVk;
class BvTextureVk;
class IBvQuery;
class IBvRenderPass;
class IBvGraphicsPipelineState;
class IBvComputePipelineState;
class IBvRayTracingPipelineState;
class IBvShaderResourceParams;
class BvFrameDataVk;
class BvQueryVk;


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

	BvCommandBufferVk(BvRenderDeviceVk* pDevice, VkCommandBuffer commandBuffer, BvFrameDataVk* pFrameData);
	BvCommandBufferVk(BvCommandBufferVk&& rhs) noexcept = default;
	BvCommandBufferVk& operator=(BvCommandBufferVk&& rhs) noexcept = default;
	~BvCommandBufferVk();

	void Reset();
	void Begin();
	void End();

	void BeginRenderPass(const IBvRenderPass* pRenderPass, u32 renderPassTargetCount, const RenderPassTargetDesc* pRenderPassTargets);
	void NextSubpass();
	void EndRenderPass();

	void SetRenderTargets(u32 renderTargetCount, const RenderTargetDesc* pRenderTargets, u32 multiviewCount = 0);

	void SetViewports(u32 viewportCount, const Viewport* pViewports);
	void SetScissors(u32 scissorCount, const Rect* pScissors);

	void SetGraphicsPipeline(const IBvGraphicsPipelineState* pPipeline);
	void SetComputePipeline(const IBvComputePipelineState* pPipeline);
	void SetRayTracingPipeline(const IBvRayTracingPipelineState* pPipeline);

	void SetShaderResourceParams(u32 resourceParamsCount, IBvShaderResourceParams* const* ppResourceParams, u32 startIndex);
	void SetConstantBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0);
	void SetStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0);
	void SetRWStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0);
	void SetDynamicConstantBuffers(u32 count, const IBvBufferView* const* ppResources, const u32* pOffsets, u32 set, u32 binding, u32 startIndex = 0);
	void SetDynamicStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, const u32* pOffsets, u32 set, u32 binding, u32 startIndex = 0);
	void SetDynamicRWStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, const u32* pOffsets, u32 set, u32 binding, u32 startIndex = 0);
	void SetFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0);
	void SetRWFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0);
	void SetTextures(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0);
	void SetRWTextures(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0);
	void SetSamplers(u32 count, const IBvSampler* const* ppResources, u32 set, u32 binding, u32 startIndex = 0);
	void SetInputAttachments(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex = 0);
	void SetAccelerationStructures(u32 count, const IBvAccelerationStructure* const* ppResources, u32 set, u32 binding, u32 startIndex = 0);
	void SetShaderConstants(u32 size, const void* pData, u32 binding, u32 set);

	void SetVertexBufferViews(u32 vertexBufferCount, const VertexBufferView* pVertexBufferViews, u32 firstBinding = 0);
	void SetIndexBufferView(const IndexBufferView& indexBufferView);

	void SetDepthBounds(float min, float max);
	void SetStencilRef(u32 stencilRef);
	void SetBlendConstants(const float (pColors[4]));
	void SetShadingRate(ShadingRateDimensions dimensions, ShadingRateCombinerOp (pCombinerOps[2]));

	void Draw(const DrawCommandArgs& args);
	void DrawIndexed(const DrawIndexedCommandArgs& args);
	void Dispatch(const DispatchCommandArgs& args);
	void DispatchMesh(const DispatchMeshCommandArgs& args);

	void DrawIndirect(const IBvBuffer* pBuffer, u32 drawCount = 1, u64 offset = 0);
	void DrawIndexedIndirect(const IBvBuffer* pBuffer, u32 drawCount = 1, u64 offset = 0);
	void DispatchIndirect(const IBvBuffer* pBuffer, u64 offset = 0);
	void DispatchMeshIndirect(const IBvBuffer* pBuffer, u64 offset = 0);
	void DispatchMeshIndirectCount(const IBvBuffer* pBuffer, u64 offset, const IBvBuffer* pCountBuffer, u64 countOffset, u32 maxCount);

	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const VkBufferCopy& copyRegion);
	void CopyBuffer(const IBvBuffer* pSrcBuffer, IBvBuffer* pDstBuffer);
	void CopyBuffer(const IBvBuffer* pSrcBuffer, IBvBuffer* pDstBuffer, const BufferCopyDesc& copyDesc);
	
	void CopyTexture(const IBvTexture* pSrcTexture, IBvTexture* pDstTexture);
	void CopyTexture(const IBvTexture* pSrcTexture, IBvTexture* pDstTexture, const TextureCopyDesc& copyDesc);

	void CopyBufferToTexture(VkBuffer srcBuffer, VkImage dstTexture, u32 copyCount, const VkBufferImageCopy* pCopyRegions);
	void CopyBufferToTexture(const IBvBuffer* pSrcBuffer, IBvTexture* pDstTexture, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs);

	void CopyTextureToBuffer(VkImage srcTexture, VkBuffer dstBuffer, u32 copyCount, const VkBufferImageCopy* pCopyRegions);
	void CopyTextureToBuffer(const IBvTexture* pSrcTexture, IBvBuffer* pDstBuffer, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs);

	void ResourceBarrier(u32 bufferBarrierCount, const VkBufferMemoryBarrier2* pBufferBarriers,
		u32 imageBarrierCount, const VkImageMemoryBarrier2* pImageBarriers, u32 memoryBarrierCount, const VkMemoryBarrier2* pMemoryBarriers,
		VkDependencyFlags dependencyFlags = 0);
	void ResourceBarrier(u32 barrierCount, const ResourceBarrierDesc* pBarriers);

	void SetPredication(const IBvBuffer* pBuffer, u64 offset, PredicationOp predicationOp);

	void BeginQuery(IBvQuery* pQuery);
	void EndQuery(IBvQuery* pQuery);

	void BeginEvent(const char* pName, const BvColor& color);
	void EndEvent();
	void SetMarker(const char* pName, const BvColor& color);

	void BuildBLAS(const BLASBuildDesc& desc, const ASPostBuildDesc* pPostBuildDesc = nullptr);
	void BuildTLAS(const TLASBuildDesc& desc, const ASPostBuildDesc* pPostBuildDesc = nullptr);
	void EmitASPostBuild(IBvAccelerationStructure* pAS, const ASPostBuildDesc& postBuildDesc);
	void CopyBLAS(const AccelerationStructureCopyDesc& copyDesc);
	void CopyTLAS(const AccelerationStructureCopyDesc& copyDesc);
	void DispatchRays(const DispatchRaysCommandArgs& args);
	void DispatchRaysIndirect(const IBvBuffer* pBuffer, u64 offset = 0);

	BV_INLINE const VkCommandBuffer GetHandle() const { return m_CommandBuffer; }
	BV_INLINE const BvVector<BvSwapChainVk*>& GetSwapChains() const { return m_SwapChains; }

private:
	void FlushDescriptorSets();
	void ResetRenderTargets();
	void AddSwapChain(BvSwapChainVk* pSwapChain);
	void BeginMeshQueries();
	void EndMeshQueries();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
	BvFrameDataVk* m_pFrameData = nullptr;

	BvVector<BvSwapChainVk*> m_SwapChains;
	
	BvVector<VkWriteDescriptorSetAccelerationStructureKHR> m_ASWriteSets;
	BvVector<VkWriteDescriptorSet> m_WriteSets;
	BvVector<VkDescriptorSet> m_DescriptorSets;
	BvVector<u32> m_DynamicOffsets;

	BvVector<VkBufferImageCopy> m_BufferImageCopyRegions;
	BvVector<VkImageCopy> m_ImageCopyRegions;

	BvVector<VkImageMemoryBarrier2> m_PreRenderBarriers;
	BvVector<VkImageMemoryBarrier2> m_PostRenderBarriers;
	BvVector<VkMemoryBarrier2> m_MemoryBarriers;
	BvVector<VkBufferMemoryBarrier2> m_BufferBarriers;
	BvVector<VkImageMemoryBarrier2> m_ImageBarriers;

	const BvGraphicsPipelineStateVk* m_pGraphicsPipeline = nullptr;
	const BvComputePipelineStateVk* m_pComputePipeline = nullptr;
	const BvRayTracingPipelineStateVk* m_pRayTracingPipeline = nullptr;
	const BvShaderResourceLayoutVk* m_pShaderResourceLayout = nullptr;

	BvVector<VkAccelerationStructureGeometryKHR> m_ASGeometries;
	BvVector<VkAccelerationStructureBuildRangeInfoKHR> m_ASRanges;
	BvVector<BvQueryVk*> m_MeshQueries;

	VkPipelineBindPoint m_PipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_MAX_ENUM;

	State m_CurrentState = State::kRecording;
	bool m_HasDebugUtils = false;
};