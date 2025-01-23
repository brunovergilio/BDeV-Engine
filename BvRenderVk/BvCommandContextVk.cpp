#include "BvCommandContextVk.h"
#include "BvRenderDeviceVk.h"
#include "BvShaderResourceVk.h"
#include "BvCommandBufferVk.h"
#include "BvQueryVk.h"
#include "BvSwapChainVk.h"


BvFrameDataVk::BvFrameDataVk()
{
}


BvFrameDataVk::BvFrameDataVk(const BvRenderDeviceVk *pDevice, u32 queueFamilyIndex, u32 frameIndex)
	: m_pDevice(pDevice), m_CommandPool(pDevice, queueFamilyIndex), m_SignalSemaphore(pDevice->GetHandle()), m_FrameIndex(frameIndex)
{
}


BvFrameDataVk::BvFrameDataVk(BvFrameDataVk&& rhs) noexcept
{
	*this = std::move(rhs);
}


BvFrameDataVk& BvFrameDataVk::operator=(BvFrameDataVk&& rhs) noexcept
{
	m_pDevice = rhs.m_pDevice;
	std::swap(m_CommandPool, rhs.m_CommandPool);
	std::swap(m_CommandBuffers, rhs.m_CommandBuffers);
	std::swap(m_ResourceBindingState, rhs.m_ResourceBindingState);
	std::swap(m_DescriptorPools, rhs.m_DescriptorPools);
	std::swap(m_DescriptorSets, rhs.m_DescriptorSets);
	std::swap(m_SignalSemaphore, rhs.m_SignalSemaphore);

	return *this;
}


BvFrameDataVk::~BvFrameDataVk()
{
}


void BvFrameDataVk::Reset()
{
	m_SignalSemaphore.Wait(m_SignaValueIndex.first);

	if (m_Queries.Size() > 0)
	{
		for (auto& pQuery : m_Queries)
		{
			pQuery->UpdateResults(m_FrameIndex);
		}
		GetQueryHeapManager()->Reset(m_FrameIndex);
		m_Queries.Clear();
	}

	m_CommandPool.Reset();
	m_CommandBuffers.Clear();
	m_ResourceBindingState.Reset();
}


BvCommandBufferVk* BvFrameDataVk::RequestCommandBuffer()
{
	if (auto pCommandBuffer = m_CommandPool.GetCommandBuffer(this))
	{
		m_CommandBuffers.EmplaceBack(pCommandBuffer);
		return pCommandBuffer;
	}

	return nullptr;
}


VkDescriptorSet BvFrameDataVk::RequestDescriptorSet(u32 set, const BvShaderResourceLayoutVk* pLayout, BvVector<VkWriteDescriptorSet>& writeSets)
{
	u64 descriptorSetHash = 0;
	for (auto& writeSet : writeSets)
	{
		HashCombine(descriptorSetHash, writeSet);
	}
	auto it = m_DescriptorSets.FindKey(descriptorSetHash);
	if (it != m_DescriptorSets.cend())
	{
		return it->second.GetHandle();
	}

	auto srlHash = MurmurHash64A(&pLayout->GetSetLayoutHandles().At(set), sizeof(VkDescriptorSetLayout));
	auto& pool = m_DescriptorPools[srlHash];
	if (!pool.IsValid())
	{
		pool = BvDescriptorPoolVk(m_pDevice, pLayout, set, 16);
	}
	auto descriptorSet = pool.Allocate();
	auto& descriptorSetVk = m_DescriptorSets[descriptorSetHash];
	descriptorSetVk = BvDescriptorSetVk(m_pDevice, descriptorSet);
	for (auto& writeSet : writeSets)
	{
		writeSet.dstSet = descriptorSetVk.GetHandle();
	}
	descriptorSetVk.Update(writeSets);

	return descriptorSetVk.GetHandle();
}


void BvFrameDataVk::UpdateSignalIndex(u64 value)
{
	BV_ASSERT(value > m_SignaValueIndex.second, "Signal value must be greater than the previous one");
	m_SignaValueIndex.second = value;
}


void BvFrameDataVk::UpdateSignalValue()
{
	m_SignaValueIndex.first += m_SignaValueIndex.second;
	m_SignaValueIndex.second = 0;
}


void BvFrameDataVk::ClearActiveCommandBuffers()
{
	m_CommandBuffers.Clear();
}


void BvFrameDataVk::AddQuery(BvQueryVk* pQuery)
{
	m_Queries.EmplaceBack(pQuery);
}


BvQueryHeapManagerVk* BvFrameDataVk::GetQueryHeapManager() const
{
	return m_pDevice->GetQueryHeapManager();
}


VkFramebuffer BvFrameDataVk::GetFramebuffer(const FramebufferDesc& fbDesc) const
{
	return m_pDevice->GetFramebufferManager()->GetFramebuffer(m_pDevice->GetHandle(), fbDesc);
}


BvCommandContextVk::BvCommandContextVk(const BvRenderDeviceVk* pDevice, u32 frameCount, CommandType queueFamilyType, u32 queueFamilyIndex, u32 queueIndex)
	: m_Queue(pDevice, queueFamilyType, queueFamilyIndex, queueIndex)
{
	m_Frames.Reserve(frameCount);
	for (auto i = 0; i < frameCount; ++i)
	{
		m_Frames.EmplaceBack(pDevice, queueFamilyIndex, i);
	}
}


BvCommandContextVk::~BvCommandContextVk()
{
}


void BvCommandContextVk::AddDeferredContext(BvCommandContext* pDeferredContext)
{
}


void BvCommandContextVk::Signal()
{
	Signal(m_Frames[m_ActiveFrameIndex].GetSemaphoreValueIndex().second + 1);
}


void BvCommandContextVk::Signal(u64 value)
{
	// Submit active command buffers
	m_pCurrCommandBuffer->End();
	
	// Update semaphore value
	m_Frames[m_ActiveFrameIndex].UpdateSignalIndex(value);
	auto [signalValue, index] = m_Frames[m_ActiveFrameIndex].GetSemaphoreValueIndex();
	auto& commandBuffers = m_Frames[m_ActiveFrameIndex].GetCommandBuffers();
	m_Queue.Submit(commandBuffers, GetSemaphore()->GetHandle(), signalValue + index);

	m_Frames[m_ActiveFrameIndex].ClearActiveCommandBuffers();

	m_pCurrCommandBuffer = nullptr;
}


void BvCommandContextVk::Wait(BvCommandContext* pCommandContext, u64 value)
{
	// Add a wait semaphore and its value
	auto pContextVk = reinterpret_cast<BvCommandContextVk*>(pCommandContext);
	m_Queue.AddWaitSemaphore(pContextVk->GetSemaphore()->GetHandle(), value);
}


void BvCommandContextVk::Flush()
{
	m_Frames[m_ActiveFrameIndex].UpdateSignalValue();

	// Get next frame
	m_ActiveFrameIndex = (m_ActiveFrameIndex + 1) % (u32)m_Frames.Size();

	// Wait for frame's signal value
	m_Frames[m_ActiveFrameIndex].Reset();

	for (auto& pSwapChain : m_SwapChains)
	{
		pSwapChain->AcquireImage();
	}
}


void BvCommandContextVk::WaitForGPU()
{
	m_Queue.WaitIdle();

	m_Frames[m_ActiveFrameIndex].UpdateSignalValue();
}


void BvCommandContextVk::BeginRenderPass(const BvRenderPass* pRenderPass, u32 renderPassTargetCount, const RenderPassTargetDesc* pRenderPassTargets)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->BeginRenderPass(pRenderPass, renderPassTargetCount, pRenderPassTargets);
}


void BvCommandContextVk::EndRenderPass()
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->EndRenderPass();
}


void BvCommandContextVk::SetRenderTargets(u32 renderTargetCount, const RenderTargetDesc* pRenderTargets)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->SetRenderTargets(renderTargetCount, pRenderTargets);
}


void BvCommandContextVk::SetViewports(u32 viewportCount, const Viewport* pViewports)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->SetViewports(viewportCount, pViewports);
}


void BvCommandContextVk::SetScissors(u32 scissorCount, const Rect* pScissors)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->SetScissors(scissorCount, pScissors);
}


void BvCommandContextVk::SetGraphicsPipeline(const BvGraphicsPipelineState* pPipeline)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->SetGraphicsPipeline(pPipeline);
}


void BvCommandContextVk::SetComputePipeline(const BvComputePipelineState* pPipeline)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->SetComputePipeline(pPipeline);
}


void BvCommandContextVk::SetShaderResourceParams(u32 resourceParamsCount, BvShaderResourceParams* const* ppResourceParams, u32 startIndex)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->SetShaderResourceParams(resourceParamsCount, ppResourceParams, startIndex);
}

void BvCommandContextVk::SetConstantBuffers(u32 count, const BvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->SetConstantBuffers(count, ppResources, set, binding, startIndex);
}


void BvCommandContextVk::SetStructuredBuffers(u32 count, const BvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->SetStructuredBuffers(count, ppResources, set, binding, startIndex);
}


void BvCommandContextVk::SetRWStructuredBuffers(u32 count, const BvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->SetRWStructuredBuffers(count, ppResources, set, binding, startIndex);
}


void BvCommandContextVk::SetFormattedBuffers(u32 count, const BvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->SetFormattedBuffers(count, ppResources, set, binding, startIndex);
}


void BvCommandContextVk::SetRWFormattedBuffers(u32 count, const BvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->SetRWFormattedBuffers(count, ppResources, set, binding, startIndex);
}


void BvCommandContextVk::SetTextures(u32 count, const BvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->SetTextures(count, ppResources, set, binding, startIndex);
}


void BvCommandContextVk::SetRWTextures(u32 count, const BvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->SetRWTextures(count, ppResources, set, binding, startIndex);
}


void BvCommandContextVk::SetSamplers(u32 count, const BvSampler* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->SetSamplers(count, ppResources, set, binding, startIndex);
}


void BvCommandContextVk::SetShaderConstants(u32 size, const void* pData, u32 offset)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->SetShaderConstants(size, pData, offset);
}


void BvCommandContextVk::SetVertexBufferViews(u32 vertexBufferCount, const BvBufferView* const* pVertexBufferViews, u32 firstBinding /*= 0*/)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->SetVertexBufferViews(vertexBufferCount, pVertexBufferViews, firstBinding);
}


void BvCommandContextVk::SetIndexBufferView(const BvBufferView* pIndexBufferView, IndexFormat indexFormat)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->SetIndexBufferView(pIndexBufferView, indexFormat);
}


void BvCommandContextVk::Draw(u32 vertexCount, u32 instanceCount /*= 1*/, u32 firstVertex /*= 0*/, u32 firstInstance /*= 0*/)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->Draw(vertexCount, instanceCount, firstVertex, firstInstance);
}


void BvCommandContextVk::DrawIndexed(u32 indexCount, u32 instanceCount /*= 1*/, u32 firstIndex /*= 0*/, i32 vertexOffset /*= 0*/, u32 firstInstance /*= 0*/)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->DrawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}


void BvCommandContextVk::Dispatch(u32 x, u32 y /*= 1*/, u32 z /*= 1*/)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->Dispatch(x, y, z);
}


void BvCommandContextVk::DrawIndirect(const BvBuffer* pBuffer, u32 drawCount /*= 1*/, u64 offset /*= 0*/)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->DrawIndirect(pBuffer, drawCount, offset);
}


void BvCommandContextVk::DrawIndexedIndirect(const BvBuffer* pBuffer, u32 drawCount /*= 1*/, u64 offset /*= 0*/)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->DrawIndexedIndirect(pBuffer, drawCount, offset);
}


void BvCommandContextVk::DispatchIndirect(const BvBuffer* pBuffer, u64 offset /*= 0*/)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->DispatchIndirect(pBuffer, offset);
}


void BvCommandContextVk::DispatchMesh(u32 x, u32 y, u32 z)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->DispatchMesh(x, y, z);
}


void BvCommandContextVk::DispatchMeshIndirect(const BvBuffer* pBuffer, u64 offset)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->DispatchMeshIndirect(pBuffer, offset);
}


void BvCommandContextVk::DispatchMeshIndirectCount(const BvBuffer* pBuffer, u64 offset, const BvBuffer* pCountBuffer, u64 countOffset, u32 maxCount)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->DispatchMeshIndirectCount(pBuffer, offset, pCountBuffer, countOffset, maxCount);
}


void BvCommandContextVk::CopyBuffer(const BvBufferVk* pSrcBuffer, BvBufferVk* pDstBuffer, const VkBufferCopy& copyRegion)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->CopyBuffer(pSrcBuffer, pDstBuffer, copyRegion);
}


void BvCommandContextVk::CopyBuffer(const BvBuffer* pSrcBuffer, BvBuffer* pDstBuffer)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->CopyBuffer(pSrcBuffer, pDstBuffer);
}


void BvCommandContextVk::CopyBuffer(const BvBuffer* pSrcBuffer, BvBuffer* pDstBuffer, const BufferCopyDesc& copyDesc)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->CopyBuffer(pSrcBuffer, pDstBuffer, copyDesc);
}


void BvCommandContextVk::CopyTexture(const BvTexture* pSrcTexture, BvTexture* pDstTexture)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->CopyTexture(pSrcTexture, pDstTexture);
}


void BvCommandContextVk::CopyTexture(const BvTexture* pSrcTexture, BvTexture* pDstTexture, const TextureCopyDesc& copyDesc)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->CopyTexture(pSrcTexture, pDstTexture, copyDesc);
}


void BvCommandContextVk::CopyBufferToTexture(const BvBufferVk* pSrcBuffer, BvTextureVk* pDstTexture, u32 copyCount, const VkBufferImageCopy* pCopyRegions)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->CopyBufferToTexture(static_cast<const BvBufferVk*>(pSrcBuffer), static_cast<BvTextureVk*>(pDstTexture), copyCount, pCopyRegions);
}


void BvCommandContextVk::CopyBufferToTexture(const BvBuffer* pSrcBuffer, BvTexture* pDstTexture, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->CopyBufferToTexture(pSrcBuffer, pDstTexture, copyCount, pCopyDescs);
}


void BvCommandContextVk::CopyTextureToBuffer(const BvTextureVk* pSrcTexture, BvBufferVk* pDstBuffer, u32 copyCount, const VkBufferImageCopy* pCopyRegions)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->CopyTextureToBuffer(static_cast<const BvTextureVk*>(pSrcTexture), static_cast<BvBufferVk*>(pDstBuffer), copyCount, pCopyRegions);
}


void BvCommandContextVk::CopyTextureToBuffer(const BvTexture* pSrcTexture, BvBuffer* pDstBuffer, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->CopyTextureToBuffer(pSrcTexture, pDstBuffer, copyCount, pCopyDescs);
}


void BvCommandContextVk::ResourceBarrier(u32 bufferBarrierCount, const VkBufferMemoryBarrier2* pBufferBarriers, u32 imageBarrierCount, const VkImageMemoryBarrier2* pImageBarriers, u32 memoryBarrierCount, const VkMemoryBarrier2* pMemoryBarriers)
{
	m_pCurrCommandBuffer->ResourceBarrier(bufferBarrierCount, pBufferBarriers, imageBarrierCount, pImageBarriers, memoryBarrierCount, pMemoryBarriers);
}


void BvCommandContextVk::ResourceBarrier(u32 barrierCount, const ResourceBarrierDesc* pBarriers)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->ResourceBarrier(barrierCount, pBarriers);
}


void BvCommandContextVk::SetPredication(const BvBuffer* pBuffer, u64 offset, PredicationOp predicationOp)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->SetPredication(pBuffer, offset, predicationOp);
}


void BvCommandContextVk::BeginQuery(BvQuery* pQuery)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->BeginQuery(pQuery);
}


void BvCommandContextVk::EndQuery(BvQuery* pQuery)
{
	SetupCommandBufferIfNotReady();

	m_pCurrCommandBuffer->EndQuery(pQuery);
}


void BvCommandContextVk::AddSwapChain(BvSwapChainVk* pSwapChain)
{
	m_SwapChains.EmplaceBack(pSwapChain);
}


void BvCommandContextVk::RemoveSwapChain(BvSwapChainVk* pSwapChain)
{
	for (auto it = m_SwapChains.begin(); it != m_SwapChains.end(); ++it)
	{
		if (*it == pSwapChain)
		{
			m_SwapChains.Erase(it);
			break;
		}
	}
}


void BvCommandContextVk::SetupCommandBufferIfNotReady()
{
	if (!m_pCurrCommandBuffer) [[unlikely]]
	{
		m_pCurrCommandBuffer = m_Frames[m_ActiveFrameIndex].RequestCommandBuffer();
	}
}