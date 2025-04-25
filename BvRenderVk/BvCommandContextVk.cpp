#include "BvCommandContextVk.h"
#include "BvRenderDeviceVk.h"
#include "BvShaderResourceVk.h"
#include "BvCommandBufferVk.h"
#include "BvQueryVk.h"
#include "BvSwapChainVk.h"
#include "BvGPUFenceVk.h"
#include "BvFramebufferVk.h"
#include "BvShaderBindingTableVk.h"
#include "BvBufferVk.h"


BvFrameDataVk::BvFrameDataVk()
{
}


BvFrameDataVk::BvFrameDataVk(BvRenderDeviceVk *pDevice, u32 queueFamilyIndex, u32 frameIndex, ContextDataVk* pContextData)
	: m_pDevice(pDevice), m_CommandPool(pDevice, queueFamilyIndex), m_FrameIndex(frameIndex),
	m_pContextData(pContextData), m_pFence(pDevice->CreateFence<BvGPUFenceVk>(0))
{
	BV_ASSERT(m_pFence->IsValid(), "Fence has to be valid");
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
	std::swap(m_pContextData, rhs.m_pContextData);
	m_pFence = rhs.m_pFence;
	std::swap(m_Queries, rhs.m_Queries);
	m_SignaValueIndex = rhs.m_SignaValueIndex;
	m_FrameIndex = rhs.m_FrameIndex;

	return *this;
}


BvFrameDataVk::~BvFrameDataVk()
{
	if (m_pFence)
	{
		m_pFence->Release();
		m_pFence = nullptr;
	}
}


void BvFrameDataVk::Reset()
{
	m_pFence->Wait(m_SignaValueIndex.first);

	if (m_Queries.Size() > 0)
	{
		m_pContextData->m_pQueryHeapManager->Reset(m_FrameIndex);
		m_Queries.Clear();
		m_UpdatedQueries = 0;
	}

	m_CommandPool.Reset();
	m_CommandBuffers.Clear();
	m_pContextData->m_ResourceBindingState.Reset();
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


VkDescriptorSet BvFrameDataVk::RequestDescriptorSet(u32 set, const BvShaderResourceLayoutVk* pLayout, BvVector<VkWriteDescriptorSet>& writeSets, u32 hashSeed, bool bindless)
{
	auto srlHash = MurmurHash64A(&pLayout->GetSetLayoutHandles().At(set), sizeof(VkDescriptorSetLayout));
	auto& pool = m_pContextData->m_DescriptorPools[srlHash];
	if (!pool.IsValid())
	{
		pool = BvDescriptorPoolVk(m_pDevice, pLayout, set, bindless ? 1 : 16);
	}

	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

	if (bindless)
	{
		auto result = m_pContextData->m_BindlessDescriptorSets.Emplace(srlHash, BvDescriptorSetVk());
		if (result.second)
		{
			result.first->second = BvDescriptorSetVk(m_pDevice, pool.Allocate());
		}

		for (auto& writeSet : writeSets)
		{
			writeSet.dstSet = result.first->second.GetHandle();
		}
		result.first->second.Update(writeSets);

		descriptorSet = result.first->second.GetHandle();
	}
	else
	{
		u64 descriptorSetHash = hashSeed;
		for (auto& writeSet : writeSets)
		{
			HashCombine(descriptorSetHash, writeSet);
		}

		auto result = m_pContextData->m_DescriptorSets.Emplace(descriptorSetHash, BvDescriptorSetVk());
		if (result.second)
		{
			result.first->second = BvDescriptorSetVk(m_pDevice, pool.Allocate());
			for (auto& writeSet : writeSets)
			{
				writeSet.dstSet = result.first->second.GetHandle();
			}
			result.first->second.Update(writeSets);
		}

		descriptorSet = result.first->second.GetHandle();
	}

	return descriptorSet;
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


VkFramebuffer BvFrameDataVk::GetFramebuffer(const FramebufferDesc& fbDesc)
{
	return m_pContextData->m_pFramebufferManager->GetFramebuffer(fbDesc);
}


void BvFrameDataVk::RemoveFramebuffers(VkImageView view)
{
	m_pContextData->m_pFramebufferManager->RemoveFramebuffersWithView(view);
}


void BvFrameDataVk::UpdateQueryData()
{
	auto value = m_SignaValueIndex.first + m_SignaValueIndex.second;
	for (auto i = m_UpdatedQueries; i < m_Queries.Size(); ++i, ++m_UpdatedQueries)
	{
		m_Queries[i]->SetFenceData(m_pFence, value);
		m_Queries[i]->SetLatestFrameIndex(m_FrameIndex);
	}
}


BV_VK_DEVICE_RES_DEF(BvCommandContextVk)


BvCommandContextVk::BvCommandContextVk(BvRenderDeviceVk* pDevice, u32 frameCount, CommandType queueFamilyType, u32 queueFamilyIndex, u32 queueIndex)
	: m_pDevice(pDevice), m_Queue(pDevice->GetHandle(), queueFamilyType, queueFamilyIndex, queueIndex)
{
	u32 querySizes[kQueryTypeCount];
	for (auto& querySize : querySizes)
	{
		querySize = 2;
	}
	m_pContextData = BV_NEW(ContextDataVk)();
	m_pContextData->m_pQueryHeapManager = BV_NEW(BvQueryHeapManagerVk)(pDevice, querySizes, frameCount);
	m_pContextData->m_pFramebufferManager = BV_NEW(BvFramebufferManagerVk)(pDevice->GetHandle());
	m_Frames.Reserve(frameCount);
	for (auto i = 0; i < frameCount; ++i)
	{
		m_Frames.EmplaceBack(pDevice, queueFamilyIndex, i, m_pContextData);
	}
}


BvCommandContextVk::~BvCommandContextVk()
{
	BV_DELETE(m_pContextData->m_pFramebufferManager);
	BV_DELETE(m_pContextData->m_pQueryHeapManager);
	BV_DELETE(m_pContextData);
}


BvGPUOp BvCommandContextVk::Execute()
{
	return Execute(m_Frames[m_ActiveFrameIndex].GetSemaphoreValueIndex().second + 1);
}


BvGPUOp BvCommandContextVk::Execute(u64 value)
{
	// Submit active command buffers
	if (m_pCurrCommandBuffer)
	{
		m_pCurrCommandBuffer->End();
	}
	
	// Update semaphore value
	m_Frames[m_ActiveFrameIndex].UpdateSignalIndex(value);
	auto [signalValue, index] = m_Frames[m_ActiveFrameIndex].GetSemaphoreValueIndex();
	auto& commandBuffers = m_Frames[m_ActiveFrameIndex].GetCommandBuffers();
	auto pFence = m_Frames[m_ActiveFrameIndex].GetGPUFence();
	auto semaphoreValue = signalValue + index;
	m_Queue.Submit(commandBuffers, pFence->GetHandle(), semaphoreValue);

	m_Frames[m_ActiveFrameIndex].ClearActiveCommandBuffers();
	m_Frames[m_ActiveFrameIndex].UpdateQueryData();

	m_pCurrCommandBuffer = nullptr;

	return BvGPUOp(pFence, value);
}


void BvCommandContextVk::Execute(IBvGPUFence* pFence, u64 value)
{
	auto pFenceVk = TO_VK(pFence);
	m_Queue.AddSignalSemaphore(pFenceVk->GetHandle(), value);

	Execute();
}


void BvCommandContextVk::Wait(IBvCommandContext* pCommandContext, u64 value)
{
	// Add a wait semaphore and its value
	auto pContextVk = reinterpret_cast<BvCommandContextVk*>(pCommandContext);
	m_Queue.AddWaitSemaphore(pContextVk->GetCurrentGPUFence()->GetHandle(), value);
}


void BvCommandContextVk::NewCommandList()
{
	if (m_pCurrCommandBuffer)
	{
		m_pCurrCommandBuffer->End();
	}

	// Get command buffer
	m_pCurrCommandBuffer = m_Frames[m_ActiveFrameIndex].RequestCommandBuffer();
}


void BvCommandContextVk::FlushFrame()
{
	// Sanity check
	BV_ASSERT(m_pCurrCommandBuffer == nullptr, "All command buffers must have been submitted");

	// Update the signal value
	m_Frames[m_ActiveFrameIndex].UpdateSignalValue();

	// Update swap chains' fences
	for (auto& pSwapChain : m_SwapChains)
	{
		pSwapChain->SetCurrentFence(GetCurrentGPUFence(), GetCurrentValue());
	}

	// Get next frame
	m_ActiveFrameIndex = (m_ActiveFrameIndex + 1) % (u32)m_Frames.Size();

	// Wait for frame's signal value
	m_Frames[m_ActiveFrameIndex].Reset();

	// Get swap chains ready
	for (auto& pSwapChain : m_SwapChains)
	{
		pSwapChain->AcquireImage();
	}
}


void BvCommandContextVk::WaitForGPU()
{
	m_Queue.WaitIdle();

	m_Frames[m_ActiveFrameIndex].UpdateSignalValue();

	m_Frames[m_ActiveFrameIndex].Reset();
}


void BvCommandContextVk::BeginRenderPass(const IBvRenderPass* pRenderPass, u32 renderPassTargetCount, const RenderPassTargetDesc* pRenderPassTargets)
{
	m_pCurrCommandBuffer->BeginRenderPass(pRenderPass, renderPassTargetCount, pRenderPassTargets);
}


void BvCommandContextVk::NextSubpass()
{
	m_pCurrCommandBuffer->NextSubpass();
}


void BvCommandContextVk::EndRenderPass()
{
	m_pCurrCommandBuffer->EndRenderPass();
}


void BvCommandContextVk::SetRenderTargets(u32 renderTargetCount, const RenderTargetDesc* pRenderTargets)
{
	m_pCurrCommandBuffer->SetRenderTargets(renderTargetCount, pRenderTargets);
}


void BvCommandContextVk::SetViewports(u32 viewportCount, const Viewport* pViewports)
{
	m_pCurrCommandBuffer->SetViewports(viewportCount, pViewports);
}


void BvCommandContextVk::SetScissors(u32 scissorCount, const Rect* pScissors)
{
	m_pCurrCommandBuffer->SetScissors(scissorCount, pScissors);
}


void BvCommandContextVk::SetGraphicsPipeline(const IBvGraphicsPipelineState* pPipeline)
{
	m_pCurrCommandBuffer->SetGraphicsPipeline(pPipeline);
}


void BvCommandContextVk::SetComputePipeline(const IBvComputePipelineState* pPipeline)
{
	m_pCurrCommandBuffer->SetComputePipeline(pPipeline);
}


void BvCommandContextVk::SetRayTracingPipeline(const IBvRayTracingPipelineState* pPipeline)
{
	m_pCurrCommandBuffer->SetRayTracingPipeline(pPipeline);
}


void BvCommandContextVk::SetShaderResourceParams(u32 resourceParamsCount, IBvShaderResourceParams* const* ppResourceParams, u32 startIndex)
{
	m_pCurrCommandBuffer->SetShaderResourceParams(resourceParamsCount, ppResourceParams, startIndex);
}

void BvCommandContextVk::SetConstantBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	m_pCurrCommandBuffer->SetConstantBuffers(count, ppResources, set, binding, startIndex);
}


void BvCommandContextVk::SetStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	m_pCurrCommandBuffer->SetStructuredBuffers(count, ppResources, set, binding, startIndex);
}


void BvCommandContextVk::SetRWStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	m_pCurrCommandBuffer->SetRWStructuredBuffers(count, ppResources, set, binding, startIndex);
}


void BvCommandContextVk::SetDynamicConstantBuffers(u32 count, const IBvBufferView* const* ppResources, const u32* pOffsets, u32 set, u32 binding, u32 startIndex)
{
	m_pCurrCommandBuffer->SetDynamicConstantBuffers(count, ppResources, pOffsets, set, binding, startIndex);
}


void BvCommandContextVk::SetDynamicStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, const u32* pOffsets, u32 set, u32 binding, u32 startIndex)
{
	m_pCurrCommandBuffer->SetDynamicStructuredBuffers(count, ppResources, pOffsets, set, binding, startIndex);
}


void BvCommandContextVk::SetDynamicRWStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, const u32* pOffsets, u32 set, u32 binding, u32 startIndex)
{
	m_pCurrCommandBuffer->SetDynamicRWStructuredBuffers(count, ppResources, pOffsets, set, binding, startIndex);
}


void BvCommandContextVk::SetFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	m_pCurrCommandBuffer->SetFormattedBuffers(count, ppResources, set, binding, startIndex);
}


void BvCommandContextVk::SetRWFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	m_pCurrCommandBuffer->SetRWFormattedBuffers(count, ppResources, set, binding, startIndex);
}


void BvCommandContextVk::SetTextures(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	m_pCurrCommandBuffer->SetTextures(count, ppResources, set, binding, startIndex);
}


void BvCommandContextVk::SetRWTextures(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	m_pCurrCommandBuffer->SetRWTextures(count, ppResources, set, binding, startIndex);
}


void BvCommandContextVk::SetSamplers(u32 count, const IBvSampler* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	m_pCurrCommandBuffer->SetSamplers(count, ppResources, set, binding, startIndex);
}


void BvCommandContextVk::SetInputAttachments(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	m_pCurrCommandBuffer->SetInputAttachments(count, ppResources, set, binding, startIndex);
}


void BvCommandContextVk::SetAccelerationStructures(u32 count, const IBvAccelerationStructure* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	m_pCurrCommandBuffer->SetAccelerationStructures(count, ppResources, set, binding, startIndex);
}


void BvCommandContextVk::SetShaderConstants(u32 size, const void* pData, u32 binding, u32 set)
{
	m_pCurrCommandBuffer->SetShaderConstants(size, pData, binding, set);
}


void BvCommandContextVk::SetVertexBufferViews(u32 vertexBufferCount, const VertexBufferView* pVertexBufferViews, u32 firstBinding /*= 0*/)
{
	m_pCurrCommandBuffer->SetVertexBufferViews(vertexBufferCount, pVertexBufferViews, firstBinding);
}


void BvCommandContextVk::SetIndexBufferView(const IndexBufferView& indexBufferView)
{
	m_pCurrCommandBuffer->SetIndexBufferView(indexBufferView);
}


void BvCommandContextVk::SetDepthBounds(f32 min, f32 max)
{
	m_pCurrCommandBuffer->SetDepthBounds(min, max);
}


void BvCommandContextVk::SetStencilRef(u32 stencilRef)
{
	m_pCurrCommandBuffer->SetStencilRef(stencilRef);
}


void BvCommandContextVk::SetBlendConstants(const float(pColors[4]))
{
	m_pCurrCommandBuffer->SetBlendConstants(pColors);
}


void BvCommandContextVk::SetShadingRate(ShadingRateDimensions dimensions, ShadingRateCombinerOp(pCombinerOps[2]))
{
	m_pCurrCommandBuffer->SetShadingRate(dimensions, pCombinerOps);
}


void BvCommandContextVk::Draw(const DrawCommandArgs& args)
{
	m_pCurrCommandBuffer->Draw(args);
}


void BvCommandContextVk::DrawIndexed(const DrawIndexedCommandArgs& args)
{
	m_pCurrCommandBuffer->DrawIndexed(args);
}


void BvCommandContextVk::Dispatch(const DispatchCommandArgs& args)
{
	m_pCurrCommandBuffer->Dispatch(args);
}


void BvCommandContextVk::DispatchMesh(const DispatchMeshCommandArgs& args)
{
	m_pCurrCommandBuffer->DispatchMesh(args);
}


void BvCommandContextVk::DrawIndirect(const IBvBuffer* pBuffer, u32 drawCount /*= 1*/, u64 offset /*= 0*/)
{
	m_pCurrCommandBuffer->DrawIndirect(pBuffer, drawCount, offset);
}


void BvCommandContextVk::DrawIndexedIndirect(const IBvBuffer* pBuffer, u32 drawCount /*= 1*/, u64 offset /*= 0*/)
{
	m_pCurrCommandBuffer->DrawIndexedIndirect(pBuffer, drawCount, offset);
}


void BvCommandContextVk::DispatchIndirect(const IBvBuffer* pBuffer, u64 offset /*= 0*/)
{
	m_pCurrCommandBuffer->DispatchIndirect(pBuffer, offset);
}


void BvCommandContextVk::DispatchMeshIndirect(const IBvBuffer* pBuffer, u64 offset)
{
	m_pCurrCommandBuffer->DispatchMeshIndirect(pBuffer, offset);
}


void BvCommandContextVk::DispatchMeshIndirectCount(const IBvBuffer* pBuffer, u64 offset, const IBvBuffer* pCountBuffer, u64 countOffset, u32 maxCount)
{
	m_pCurrCommandBuffer->DispatchMeshIndirectCount(pBuffer, offset, pCountBuffer, countOffset, maxCount);
}


void BvCommandContextVk::CopyBufferVk(const BvBufferVk* pSrcBuffer, BvBufferVk* pDstBuffer, const VkBufferCopy& copyRegion)
{
	m_pCurrCommandBuffer->CopyBuffer(pSrcBuffer->GetHandle(), pDstBuffer->GetHandle(), copyRegion);
}


void BvCommandContextVk::CopyBuffer(const IBvBuffer* pSrcBuffer, IBvBuffer* pDstBuffer)
{
	m_pCurrCommandBuffer->CopyBuffer(pSrcBuffer, pDstBuffer);
}


void BvCommandContextVk::CopyBuffer(const IBvBuffer* pSrcBuffer, IBvBuffer* pDstBuffer, const BufferCopyDesc& copyDesc)
{
	m_pCurrCommandBuffer->CopyBuffer(pSrcBuffer, pDstBuffer, copyDesc);
}


void BvCommandContextVk::CopyTexture(const IBvTexture* pSrcTexture, IBvTexture* pDstTexture)
{
	m_pCurrCommandBuffer->CopyTexture(pSrcTexture, pDstTexture);
}


void BvCommandContextVk::CopyTexture(const IBvTexture* pSrcTexture, IBvTexture* pDstTexture, const TextureCopyDesc& copyDesc)
{
	m_pCurrCommandBuffer->CopyTexture(pSrcTexture, pDstTexture, copyDesc);
}


void BvCommandContextVk::CopyBufferToTextureVk(const BvBufferVk* pSrcBuffer, BvTextureVk* pDstTexture, u32 copyCount, const VkBufferImageCopy* pCopyRegions)
{
	m_pCurrCommandBuffer->CopyBufferToTexture(pSrcBuffer->GetHandle(), pDstTexture->GetHandle(), copyCount, pCopyRegions);
}


void BvCommandContextVk::CopyBufferToTexture(const IBvBuffer* pSrcBuffer, IBvTexture* pDstTexture, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs)
{
	m_pCurrCommandBuffer->CopyBufferToTexture(pSrcBuffer, pDstTexture, copyCount, pCopyDescs);
}


void BvCommandContextVk::CopyTextureToBufferVk(const BvTextureVk* pSrcTexture, BvBufferVk* pDstBuffer, u32 copyCount, const VkBufferImageCopy* pCopyRegions)
{
	m_pCurrCommandBuffer->CopyTextureToBuffer(pSrcTexture->GetHandle(), pDstBuffer->GetHandle(), copyCount, pCopyRegions);
}


void BvCommandContextVk::CopyTextureToBuffer(const IBvTexture* pSrcTexture, IBvBuffer* pDstBuffer, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs)
{
	m_pCurrCommandBuffer->CopyTextureToBuffer(pSrcTexture, pDstBuffer, copyCount, pCopyDescs);
}


void BvCommandContextVk::ResourceBarrierVk(u32 bufferBarrierCount, const VkBufferMemoryBarrier2* pBufferBarriers, u32 imageBarrierCount, const VkImageMemoryBarrier2* pImageBarriers, u32 memoryBarrierCount, const VkMemoryBarrier2* pMemoryBarriers)
{
	m_pCurrCommandBuffer->ResourceBarrier(bufferBarrierCount, pBufferBarriers, imageBarrierCount, pImageBarriers, memoryBarrierCount, pMemoryBarriers);
}


void BvCommandContextVk::ResourceBarrier(u32 barrierCount, const ResourceBarrierDesc* pBarriers)
{
	m_pCurrCommandBuffer->ResourceBarrier(barrierCount, pBarriers);
}


void BvCommandContextVk::SetPredication(const IBvBuffer* pBuffer, u64 offset, PredicationOp predicationOp)
{
	m_pCurrCommandBuffer->SetPredication(pBuffer, offset, predicationOp);
}


void BvCommandContextVk::BeginQuery(IBvQuery* pQuery)
{
	m_pCurrCommandBuffer->BeginQuery(pQuery);
}


void BvCommandContextVk::EndQuery(IBvQuery* pQuery)
{
	m_pCurrCommandBuffer->EndQuery(pQuery);
}


void BvCommandContextVk::BeginEvent(const char* pName, const BvColor& color)
{
	m_pCurrCommandBuffer->BeginEvent(pName, color);
}


void BvCommandContextVk::EndEvent()
{
	m_pCurrCommandBuffer->EndEvent();
}


void BvCommandContextVk::SetMarker(const char* pName, const BvColor& color)
{
	m_pCurrCommandBuffer->SetMarker(pName, color);
}


void BvCommandContextVk::BuildBLAS(const BLASBuildDesc& desc)
{
	m_pCurrCommandBuffer->BuildBLAS(desc);
}


void BvCommandContextVk::BuildTLAS(const TLASBuildDesc& desc)
{
	m_pCurrCommandBuffer->BuildTLAS(desc);
}


void BvCommandContextVk::DispatchRays(const DispatchRaysCommandArgs& args)
{
	m_pCurrCommandBuffer->DispatchRays(args);
}


void BvCommandContextVk::DispatchRays(IBvShaderBindingTable* pSBT, u32 rayGenIndex, u32 missIndex, u32 hitIndex, u32 callableIndex,
	u32 width, u32 height, u32 depth)
{
	DispatchRaysCommandArgs args;
	pSBT->GetDeviceAddressRange(ShaderBindingTableGroupType::kRayGen, rayGenIndex, args.m_RayGenShader);
	pSBT->GetDeviceAddressRangeAndStride(ShaderBindingTableGroupType::kMiss, missIndex, args.m_MissShader);
	pSBT->GetDeviceAddressRangeAndStride(ShaderBindingTableGroupType::kHit, hitIndex, args.m_HitShader);
	pSBT->GetDeviceAddressRangeAndStride(ShaderBindingTableGroupType::kCallable, callableIndex, args.m_CallableShader);
	args.m_Width = width;
	args.m_Height = height;
	args.m_Depth = depth;

	DispatchRays(args);
}


void BvCommandContextVk::DispatchRaysIndirect(const IBvBuffer* pBuffer, u64 offset)
{
	m_pCurrCommandBuffer->DispatchRaysIndirect(pBuffer, offset);
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


void BvCommandContextVk::RemoveFramebuffers(VkImageView view)
{
	for (auto& frame : m_Frames)
	{
		frame.RemoveFramebuffers(view);
	}
}


void BvCommandContextVk::Destroy()
{
}