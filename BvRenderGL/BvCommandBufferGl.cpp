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


void BvCommandBufferGl::BeginRenderPass(const IBvRenderPass* const pRenderPass, IBvTextureView* const* const pRenderTargets, const ClearColorValue* const pClearColors, IBvTextureView* const pDepthStencilTarget, const ClearColorValue& depthClear)
{
}


void BvCommandBufferGl::EndRenderPass()
{
}


void BvCommandBufferGl::SetRenderTargets(const u32 renderTargetCount, IBvTextureView* const* const pRenderTargets, const ClearColorValue* const pClearColors, IBvTextureView* const pDepthStencilTarget, const ClearColorValue& depthClear, const ClearFlags clearFlags)
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


void BvCommandBufferGl::SetPipeline(const IBvGraphicsPipelineState* const pPipeline)
{
}


void BvCommandBufferGl::SetPipeline(const IBvComputePipelineState* const pPipeline)
{
}


void BvCommandBufferGl::SetShaderResourceParams(const u32 setCount, BvShaderResourceParams* const* const ppSets, const u32 firstSet)
{
}


void BvCommandBufferGl::SetVertexBufferViews(const u32 vertexBufferCount, const IBvBufferView* const* const pVertexBufferViews, const u32 firstBinding /*= 0*/)
{
}


void BvCommandBufferGl::SetIndexBufferView(const IBvBufferView* const pIndexBufferView, const IndexFormat indexFormat)
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


void BvCommandBufferGl::DrawIndirect(const IBvBuffer* const pBuffer, const u32 drawCount /*= 1*/, const u64 offset /*= 0*/)
{
}


void BvCommandBufferGl::DrawIndexedIndirect(const IBvBuffer* const pBuffer, const u32 drawCount /*= 1*/, const u64 offset /*= 0*/)
{
}


void BvCommandBufferGl::DispatchIndirect(const IBvBuffer* const pBuffer, const u64 offset /*= 0*/)
{
}


void BvCommandBufferGl::CopyBuffer(const IBvBuffer* const pSrcBuffer, IBvBuffer* const pDstBuffer)
{
}


void BvCommandBufferGl::CopyBufferRegion(const IBvBuffer* const pSrcBuffer, IBvBuffer* const pDstBuffer, const CopyRegion& copyRegion)
{
}


void BvCommandBufferGl::CopyTexture(const IBvTexture* const pSrcTexture, IBvTexture* const pDstTexture)
{
}


void BvCommandBufferGl::CopyTextureRegion(const IBvTexture* const pSrcTexture, IBvTexture* const pDstTexture, const CopyRegion& copyRegion)
{
}


void BvCommandBufferGl::CopyTextureRegion(const IBvBuffer* const pSrcBuffer, IBvTexture* const pDstTexture, const CopyRegion& copyRegion)
{
}


void BvCommandBufferGl::CopyTextureRegion(const IBvTexture* const pSrcTexture, IBvBuffer* const pDstBuffer, const CopyRegion& copyRegion)
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