#pragma once


#include "BvCommonD3D12.h"
#include "BDeV/Core/RenderAPI/BvCommandContext.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/Container/BvRobinMap.h"
//#include "BvCommandAllocatorD3D12.h"
//#include "BvDescriptorHeapD3D12.h"
//#include "BvCommandQueueD3D12.h"


class BvRenderDeviceD3D12;
class BvBufferD3D12;
class BvTextureD3D12;
class BvQueryD3D12;
class BvSwapChainD3D12;
class BvGPUFenceD3D12;
class BvQueryHeapManagerD3D12;


struct ContextDataD3D12
{
	//BvResourceBindingStateD3D12 m_ResourceBindingState;
	//BvRobinMap<u64, BvDescriptorPoolD3D12> m_DescriptorPools;
	//BvRobinMap<u64, BvDescriptorSetD3D12> m_DescriptorSets;
	//BvRobinMap<u64, BvDescriptorSetD3D12> m_BindlessDescriptorSets;
	//BvQueryHeapManagerD3D12* m_pQueryHeapManager = nullptr;
};


class BvFrameDataD3D12 final
{
	BV_NOCOPY(BvFrameDataD3D12);

public:
	BvFrameDataD3D12();
	BvFrameDataD3D12(BvRenderDeviceD3D12* pDevice, u32 queueFamilyIndex, u32 frameIndex, ContextDataD3D12* pContextData);
	BvFrameDataD3D12(BvFrameDataD3D12&& rhs) noexcept;
	BvFrameDataD3D12& operator=(BvFrameDataD3D12&& rhs) noexcept;
	~BvFrameDataD3D12();

	void Reset(bool resetQueries = true);
	BvCommandListD3D12* RequestCommandList();
	D3D12DescriptorSet RequestDescriptorSet(u32 set, const BvShaderResourceLayoutD3D12* pLayout, BvVector<D3D12WriteDescriptorSet>& writeSets, u32 hashSeed, bool bindless = false);
	void UpdateSignalIndex(u64 value);
	void UpdateSignalValue();
	void ClearActiveCommandLists();
	void AddQuery(BvQueryD3D12* pQuery);
	void UpdateQueryData();

	BV_INLINE const auto& GetCommandLists() const { return m_CommandLists; }
	BV_INLINE auto& GetResourceBindingState() { return m_pContextData->m_ResourceBindingState; }
	BV_INLINE BvGPUFenceD3D12* GetGPUFence() { return m_pFence; }
	BV_INLINE std::pair<u64, u64> GetSemaphoreValueIndex() const { return m_SignaValueIndex; }
	BV_INLINE u32 GetFrameIndex() const { return m_FrameIndex; }
	BV_INLINE BvQueryHeapManagerD3D12* GetQueryHeapManager() const { return m_pContextData->m_pQueryHeapManager; }

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	BvCommandAllocatorD3D12 m_CommandPool;
	BvVector<BvCommandListD3D12*> m_CommandLists;
	ContextDataD3D12* m_pContextData;
	BvVector<BvQueryD3D12*> m_Queries;
	u32 m_UpdatedQueries = 0;
	BvGPUFenceD3D12* m_pFence = nullptr;
	std::pair<u64, u64> m_SignaValueIndex;
	u32 m_FrameIndex;
};


//BV_OBJECT_DEFINE_ID(IBvCommandContextD3D12, "87f18018-a53f-43e7-b7f5-b39d662d0ce5");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvCommandContextD3D12);


class BvCommandContextD3D12 final : public IBvCommandContext, public IBvResourceD3D12
{
	BV_NOCOPYMOVE(BvCommandContextD3D12);
	BV_D3D12_DEVICE_RES_DECL;

public:
	BvCommandContextD3D12(BvRenderDeviceD3D12* pDevice, u32 frameCount, u32 queueFamilyIndex, u32 queueIndex);
	~BvCommandContextD3D12();

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

	void CopyBuffer(const IBvBuffer* pSrcBuffer, IBvBuffer* pDstBuffer) override;
	void CopyBuffer(const IBvBuffer* pSrcBuffer, IBvBuffer* pDstBuffer, const BufferCopyDesc& copyDesc) override;

	void CopyTexture(const IBvTexture* pSrcTexture, IBvTexture* pDstTexture) override;
	void CopyTexture(const IBvTexture* pSrcTexture, IBvTexture* pDstTexture, const TextureCopyDesc& copyDesc) override;

	void CopyBufferToTexture(const IBvBuffer* pSrcBuffer, IBvTexture* pDstTexture, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs) override;

	void CopyTextureToBuffer(const IBvTexture* pSrcTexture, IBvBuffer* pDstBuffer, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs) override;

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

	BV_INLINE auto GetCommandQueue() { return m_Queue.Get(); }
	BV_INLINE BvGPUFenceD3D12* GetCurrentGPUFence() { return m_Frames[m_ActiveFrameIndex].GetGPUFence(); }
	BV_INLINE u64 GetCurrentValue() { return m_Frames[m_ActiveFrameIndex].GetSemaphoreValueIndex().first; }
	BV_INLINE BvCommandListD3D12* GetCurrentCommandList() const { return m_pCurrCommandList; }

	void AddSwapChain(BvSwapChainD3D12* pSwapChain);
	void RemoveSwapChain(BvSwapChainD3D12* pSwapChain);

	//BV_OBJECT_IMPL_INTERFACE(IBvCommandContextD3D12, IBvCommandContext, IBvRenderDeviceObject);

private:
	void Destroy();

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	ComPtr<ID3D12CommandQueue> m_Queue;
	BvVector<BvFrameDataD3D12> m_Frames;
	BvVector<BvSwapChainD3D12*> m_SwapChains;
	BvCommandListD3D12* m_pCurrCommandList = nullptr;
	BvFrameDataD3D12* m_pCurrFrame = nullptr;
	ContextDataD3D12* m_pContextData;
	u32 m_ActiveFrameIndex = 0;
	u32 m_ContextGroupIndex = 0;
	u32 m_ContextIndex = 0;
};


BV_CREATE_CAST_TO_D3D12(BvCommandContext)