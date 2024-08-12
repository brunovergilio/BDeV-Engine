#pragma once


#include "BvRenderGl/BvCommonGl.h"
#include "BDeV/Container/BvVector.h"
#include "BDeV/RenderAPI/BvCommandBuffer.h"
#include "BDeV/RenderAPI/BvRenderCommon.h"
#include "BDeV/Container/BvFixedVector.h"


constexpr u32 kMaxSemaphores = 8;


class BvCommandPoolGl;
class BvRenderDeviceGl;
class BvTextureViewGl;
class BvSwapChainGl;
class BvGraphicsPipelineStateGl;
class BvComputePipelineStateGl;
class BvRenderPassGl;
class BvBufferViewGl;


class BvCommandBufferGl final : public BvCommandBuffer
{
public:
	BvCommandBufferGl(const BvRenderDeviceGl& device, BvCommandPool* pCommandPool);
	~BvCommandBufferGl();

	void Reset() override;
	void Begin() override;
	void End() override;

	void BeginRenderPass(const BvRenderPass* const pRenderPass, BvTextureView* const* const pRenderTargets,
		const ClearColorValue* const pClearColors, BvTextureView* const pDepthStencilTarget,
		const ClearColorValue& depthClear) override;
	void EndRenderPass() override;

	void SetRenderTargets(const u32 renderTargetCount, BvTextureView* const* const pRenderTargets, const ClearColorValue* const pClearColors,
		BvTextureView* const pDepthStencilTarget, const ClearColorValue& depthClear, const ClearFlags clearFlags) override;
	void ClearRenderTargets(u32 renderTargetCount, const ClearColorValue* const pClearValues, u32 firstRenderTargetIndex = 0) override;

	void SetViewports(const u32 viewportCount, const Viewport* const pViewports) override;
	void SetScissors(const u32 scissorCount, const Rect* const pScissors) override;

	void SetPipeline(const BvGraphicsPipelineState* const pPipeline) override;
	void SetPipeline(const BvComputePipelineState* const pPipeline) override;

	void SetShaderResourceParams(const u32 setCount, BvShaderResourceParams* const* const ppSets, const u32 firstSet) override;

	void SetVertexBufferViews(const u32 vertexBufferCount, const BvBufferView* const* const pVertexBufferViews,
		const u32 firstBinding = 0) override;
	void SetIndexBufferView(const BvBufferView* const pIndexBufferView, const IndexFormat indexFormat) override;

	void Draw(const u32 vertexCount, const u32 instanceCount = 1,
		const u32 firstVertex = 0, const u32 firstInstance = 0) override;
	void DrawIndexed(const u32 indexCount, const u32 instanceCount = 1, const u32 firstIndex = 0,
		const i32 vertexOffset = 0, const u32 firstInstance = 0) override;
	void Dispatch(const u32 x, const u32 y = 1, const u32 z = 1) override;

	void DrawIndirect(const BvBuffer* const pBuffer, const u32 drawCount = 1, const u64 offset = 0) override;
	void DrawIndexedIndirect(const BvBuffer* const pBuffer, const u32 drawCount = 1, const u64 offset = 0) override;
	void DispatchIndirect(const BvBuffer* const pBuffer, const u64 offset = 0) override;

	void CopyBuffer(const BvBuffer* const pSrcBuffer, BvBuffer* const pDstBuffer) override;
	void CopyBufferRegion(const BvBuffer* const pSrcBuffer, BvBuffer* const pDstBuffer, const CopyRegion& copyRegion) override;

	void CopyTexture(const BvTexture* const pSrcTexture, BvTexture* const pDstTexture) override;
	void CopyTextureRegion(const BvTexture* const pSrcTexture, BvTexture* const pDstTexture, const CopyRegion& copyRegion) override;

	void CopyTextureRegion(const BvBuffer* const pSrcBuffer, BvTexture* const pDstTexture, const CopyRegion& copyRegion) override;
	void CopyTextureRegion(const BvTexture* const pSrcTexture, BvBuffer* const pDstBuffer, const CopyRegion& copyRegion) override;

	void ResourceBarrier(const u32 barrierCount, const ResourceBarrierDesc* const pBarriers) override;

	BV_INLINE const BvVector<BvSwapChainGl*>& GetSwapChains() const { return m_SwapChains; }

private:
	void CommitGraphicsData();
	void CommitComputeData();
	void CommitRenderTargets();
	void DecommitRenderTargets();
	void ClearStateData();

private:
	const BvRenderDeviceGl& m_Device;

	BvVector<BvSwapChainGl*> m_SwapChains;
	BvRenderPassGl* m_pRenderPass = nullptr;
	BvVector<BvTextureViewGl*> m_RenderTargets;
	//BvVector<GlClearValue> m_RenderTargetClearValues;
	//BvVector<GlAttachmentLoadOp> m_RenderTargetLoadOps;
	BvTextureViewGl* m_pDepthStencilTarget = nullptr;
	//GlClearValue m_DepthStencilTargetClearValue{};
	const BvGraphicsPipelineStateGl* m_pGraphicsPSO = nullptr;
	const BvComputePipelineStateGl* m_pComputePSO = nullptr;
	//BvVector<GlViewport> m_Viewports;
	//BvVector<GlRect2D> m_Scissors;
	//BvVector<GlBuffer> m_VertexBuffers;
	BvVector<u64> m_VertexBufferOffsets;
	const BvBufferViewGl* m_pIndexBufferView = nullptr;
	//BvVector<GlDescriptorSet> m_DescriptorSets;
	u32 m_FirstVertexBinding = 0;
	//GlIndexType m_IndexFormat = Gl_INDEX_TYPE_UINT32;

	bool m_RenderTargetsBindNeeded = false;
	bool m_GraphicsPSOBindNeeded = false;
	bool m_ComputePSOBindNeeded = false;
	bool m_ViewportsBindNeeded = false;
	bool m_ScissorsBindNeeded = false;
	bool m_VertexBuffersBindNeeded = false;
	bool m_IndexBufferBindNeeded = false;
	bool m_DescriptorSetBindNeeded = false;
};

constexpr auto s = sizeof BvCommandBufferGl;