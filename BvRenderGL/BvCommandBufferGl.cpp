#include "BvCommandBufferGl.h"


BvCommandBufferGl::BvCommandBufferGl(const BvRenderDeviceGl& device, BvCommandPool* pCommandPool)
	: BvCommandBuffer(pCommandPool), m_Device(device)
{
}


BvCommandBufferGl::~BvCommandBufferGl()
{
}


void BvCommandBufferGl::Reset()
{
}


void BvCommandBufferGl::Begin()
{
}


void BvCommandBufferGl::End()
{
}


void BvCommandBufferGl::BeginRenderPass(const BvRenderPass* const pRenderPass, BvTextureView* const* const pRenderTargets, const ClearColorValue* const pClearColors, BvTextureView* const pDepthStencilTarget, const ClearColorValue& depthClear)
{
}


void BvCommandBufferGl::EndRenderPass()
{
}


void BvCommandBufferGl::SetRenderTargets(const u32 renderTargetCount, BvTextureView* const* const pRenderTargets, const ClearColorValue* const pClearColors, BvTextureView* const pDepthStencilTarget, const ClearColorValue& depthClear, const ClearFlags clearFlags)
{
}


void BvCommandBufferGl::ClearRenderTargets(u32 renderTargetCount, const ClearColorValue* const pClearValues, u32 firstRenderTargetIndex /*= 0*/)
{
}


void BvCommandBufferGl::SetViewports(const u32 viewportCount, const Viewport* const pViewports)
{
}


void BvCommandBufferGl::SetScissors(const u32 scissorCount, const Rect* const pScissors)
{
}


void BvCommandBufferGl::SetPipeline(const BvGraphicsPipelineState* const pPipeline)
{
}


void BvCommandBufferGl::SetPipeline(const BvComputePipelineState* const pPipeline)
{
}


void BvCommandBufferGl::SetShaderResourceParams(const u32 setCount, BvShaderResourceParams* const* const ppSets, const u32 firstSet)
{
}


void BvCommandBufferGl::SetVertexBufferViews(const u32 vertexBufferCount, const BvBufferView* const* const pVertexBufferViews, const u32 firstBinding /*= 0*/)
{
}


void BvCommandBufferGl::SetIndexBufferView(const BvBufferView* const pIndexBufferView, const IndexFormat indexFormat)
{
}


void BvCommandBufferGl::Draw(const u32 vertexCount, const u32 instanceCount /*= 1*/, const u32 firstVertex /*= 0*/, const u32 firstInstance /*= 0*/)
{
}


void BvCommandBufferGl::DrawIndexed(const u32 indexCount, const u32 instanceCount /*= 1*/, const u32 firstIndex /*= 0*/, const i32 vertexOffset /*= 0*/, const u32 firstInstance /*= 0*/)
{
}


void BvCommandBufferGl::Dispatch(const u32 x, const u32 y /*= 1*/, const u32 z /*= 1*/)
{
}


void BvCommandBufferGl::DrawIndirect(const BvBuffer* const pBuffer, const u32 drawCount /*= 1*/, const u64 offset /*= 0*/)
{
}


void BvCommandBufferGl::DrawIndexedIndirect(const BvBuffer* const pBuffer, const u32 drawCount /*= 1*/, const u64 offset /*= 0*/)
{
}


void BvCommandBufferGl::DispatchIndirect(const BvBuffer* const pBuffer, const u64 offset /*= 0*/)
{
}


void BvCommandBufferGl::CopyBuffer(const BvBuffer* const pSrcBuffer, BvBuffer* const pDstBuffer)
{
}


void BvCommandBufferGl::CopyBufferRegion(const BvBuffer* const pSrcBuffer, BvBuffer* const pDstBuffer, const CopyRegion& copyRegion)
{
}


void BvCommandBufferGl::CopyTexture(const BvTexture* const pSrcTexture, BvTexture* const pDstTexture)
{
}


void BvCommandBufferGl::CopyTextureRegion(const BvTexture* const pSrcTexture, BvTexture* const pDstTexture, const CopyRegion& copyRegion)
{
}


void BvCommandBufferGl::CopyTextureRegion(const BvBuffer* const pSrcBuffer, BvTexture* const pDstTexture, const CopyRegion& copyRegion)
{
}


void BvCommandBufferGl::CopyTextureRegion(const BvTexture* const pSrcTexture, BvBuffer* const pDstBuffer, const CopyRegion& copyRegion)
{
}


void BvCommandBufferGl::ResourceBarrier(const u32 barrierCount, const ResourceBarrierDesc* const pBarriers)
{
}


void BvCommandBufferGl::CommitGraphicsData()
{
}


void BvCommandBufferGl::CommitComputeData()
{
}


void BvCommandBufferGl::CommitRenderTargets()
{
}


void BvCommandBufferGl::DecommitRenderTargets()
{
}


void BvCommandBufferGl::ClearStateData()
{
}