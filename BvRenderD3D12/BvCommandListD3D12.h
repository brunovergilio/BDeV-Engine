#pragma once


#include "BvCommonD3D12.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/RenderAPI/BvRenderCommon.h"
#include "BDeV/Core/Container/BvFixedVector.h"


class BvRenderDeviceD3D12;
class BvSwapChainD3D12;
class BvGraphicsPipelineStateD3D12;
class BvComputePipelineStateD3D12;
class BvRayTracingPipelineStateD3D12;
class BvShaderResourceLayoutD3D12;
class IBvSampler;
class IBvBufferView;
class IBvTextureView;
class BvBufferD3D12;
class BvTextureD3D12;
class IBvQuery;
class IBvRenderPass;
class IBvGraphicsPipelineState;
class IBvComputePipelineState;
class IBvRayTracingPipelineState;
class IBvShaderResourceParams;
class BvFrameDataD3D12;
class BvQueryD3D12;


class BvCommandListD3D12 final
{
	BV_NOCOPY(BvCommandListD3D12);

public:
	enum class State : u8
	{
		kRecording,
		kRenderPass,
	};

	BvCommandListD3D12(BvRenderDeviceD3D12* pDevice, ID3D12CommandAllocator* pCommandAllocator,
		ID3D12GraphicsCommandList* pCommandList, BvFrameDataD3D12* pFrameData);
	BvCommandListD3D12(BvCommandListD3D12&& rhs) noexcept = default;
	BvCommandListD3D12& operator=(BvCommandListD3D12&& rhs) noexcept = default;
	~BvCommandListD3D12();

	void Reset();
	void Close();

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
	void SetBlendConstants(const float(pColors[4]));
	void SetShadingRate(ShadingRateDimensions dimensions, ShadingRateCombinerOp(pCombinerOps[2]));

	void Draw(const DrawCommandArgs& args);
	void DrawIndexed(const DrawIndexedCommandArgs& args);
	void Dispatch(const DispatchCommandArgs& args);
	void DispatchMesh(const DispatchMeshCommandArgs& args);

	void DrawIndirect(const IBvBuffer* pBuffer, u32 drawCount = 1, u64 offset = 0);
	void DrawIndexedIndirect(const IBvBuffer* pBuffer, u32 drawCount = 1, u64 offset = 0);
	void DispatchIndirect(const IBvBuffer* pBuffer, u64 offset = 0);
	void DispatchMeshIndirect(const IBvBuffer* pBuffer, u64 offset = 0);
	void DispatchMeshIndirectCount(const IBvBuffer* pBuffer, u64 offset, const IBvBuffer* pCountBuffer, u64 countOffset, u32 maxCount);

	void CopyBuffer(const IBvBuffer* pSrcBuffer, IBvBuffer* pDstBuffer);
	void CopyBuffer(const IBvBuffer* pSrcBuffer, IBvBuffer* pDstBuffer, const BufferCopyDesc& copyDesc);

	void CopyTexture(const IBvTexture* pSrcTexture, IBvTexture* pDstTexture);
	void CopyTexture(const IBvTexture* pSrcTexture, IBvTexture* pDstTexture, const TextureCopyDesc& copyDesc);

	void CopyBufferToTexture(const IBvBuffer* pSrcBuffer, IBvTexture* pDstTexture, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs);
	void CopyTextureToBuffer(const IBvTexture* pSrcTexture, IBvBuffer* pDstBuffer, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs);

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

	BV_INLINE const ID3D12CommandList* GetHandle() const { return m_CommandList.Get(); }
	BV_INLINE const BvVector<BvSwapChainD3D12*>& GetSwapChains() const { return m_SwapChains; }

private:
	void FlushDescriptorSets();
	void ResetRenderTargets();
	void AddSwapChain(BvSwapChainD3D12* pSwapChain);
	void BeginMeshQueries();
	void EndMeshQueries();

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	ID3D12CommandAllocator* m_pCommandAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;
	ComPtr<ID3D12GraphicsCommandList6> m_CommandListEx;
	BvFrameDataD3D12* m_pFrameData = nullptr;

	BvVector<BvSwapChainD3D12*> m_SwapChains;

	BvVector<D3D12_RESOURCE_BARRIER> m_PreRenderBarriers;
	BvVector<D3D12_RESOURCE_BARRIER> m_PostRenderBarriers;

	const BvGraphicsPipelineStateD3D12* m_pGraphicsPipeline = nullptr;
	const BvComputePipelineStateD3D12* m_pComputePipeline = nullptr;
	const BvRayTracingPipelineStateD3D12* m_pRayTracingPipeline = nullptr;
	const BvShaderResourceLayoutD3D12* m_pShaderResourceLayout = nullptr;

	BvVector<BvQueryD3D12*> m_MeshQueries;

	State m_CurrentState = State::kRecording;
	bool m_HasDebugUtils = false;
};