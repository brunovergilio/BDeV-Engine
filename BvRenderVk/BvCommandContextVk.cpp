#include "BvCommandContextVk.h"
#include "BvRenderDeviceVk.h"
#include "BvShaderResourceVk.h"
#include "BvCommandBufferVk.h"
#include "BvQueryHeapVk.h"
#include "BvSwapChainVk.h"
#include "BvGPUFenceVk.h"
#include "BvFramebufferVk.h"
#include "BvShaderBindingTableVk.h"
#include "BvBufferVk.h"
#include "BvTextureVk.h"
#include "BvUtilsVk.h"


BvFrameDataVk::BvFrameDataVk(BvRenderDeviceVk *pDevice, u32 queueFamilyIndex, u32 frameIndex, ContextDataVk* pContextData)
	: m_pDevice(pDevice), m_CommandPool(pDevice, queueFamilyIndex), m_FrameIndex(frameIndex),
	m_pContextData(pContextData)
{
	auto result = VkHelpers::CreateSemaphore(pDevice, {});
	BV_ASSERT(result.first == VK_SUCCESS, "Couldn't create fence for BvFrameDataVk");

	auto pFence = BV_RC_CREATE(BvGPUFenceVk, pDevice, result.second);
	m_pFence.Attach(pFence);
}


BvFrameDataVk::~BvFrameDataVk()
{
}


void BvFrameDataVk::Reset(bool newFrame)
{
	// If any resource was destroyed, make sure its descriptors have also been removed
	for (u32 numDeletedResources = m_pContextData->m_DeletedResourceCounter.load(std::memory_order_relaxed); numDeletedResources > 0;
		numDeletedResources = m_pContextData->m_DeletedResourceCounter.load(std::memory_order_relaxed))
	{
		constexpr u32 kDeletedResourceCopyCount = 8;
		ContextDataVk::HandleData deletedResources[kDeletedResourceCopyCount];
		u32 copyCount = std::min(numDeletedResources, kDeletedResourceCopyCount);
		
		{
			BvScopedLock lock(m_pContextData->m_DeletedResourceLock);
			for (auto i = 0; i < copyCount; i++)
			{
				deletedResources[i] = m_pContextData->m_DeletedResourceHandles[m_pContextData->m_DeletedResourceHandles.Size() - 1 - i];
				m_pContextData->m_DeletedResourceHandles.PopBack();
			}
		}

		// Update counter
		m_pContextData->m_DeletedResourceCounter.fetch_sub(copyCount, std::memory_order_relaxed);

		// Remove all destroyed resources from the list of active resources
		for (auto resourceIndex = 0; resourceIndex < copyCount; resourceIndex++)
		{
			auto resourceIt = m_pContextData->m_ActiveResources.FindKey(deletedResources[resourceIndex].m_Handle);
			if (resourceIt != m_pContextData->m_ActiveResources.cend())
			{
				for (auto& descriptorData : resourceIt->second)
				{
					// Recycle the descriptors
					descriptorData.m_pDescriptorPool->RecycleDescriptor(descriptorData.m_DescriptorSet);

					// Also remove said descriptor from the list of active descriptors
					for (auto setIt = m_pContextData->m_DescriptorSets.cbegin(); setIt != m_pContextData->m_DescriptorSets.cend(); setIt++)
					{
						if (setIt->second == descriptorData.m_DescriptorSet)
						{
							m_pContextData->m_DescriptorSets.Erase(setIt);
							break;
						}
					}
				}

				// If it's a texture, it could be used by the framebuffer
				if (deletedResources[resourceIndex].m_IsTexture)
				{
					m_pContextData->m_pFramebufferManager->RemoveFramebuffersWithView(VkImageView(deletedResources[resourceIndex].m_Handle));
				}

				m_pContextData->m_ActiveResources.Erase(resourceIt);
			}
		}
	}

	if (newFrame)
	{
		m_pFence->Wait(m_FenceValue);
	}

	m_CommandPool.Reset();
	m_CommandBuffers.Clear();
	m_pContextData->m_ResourceBindingState.Reset();

	for (auto qp : m_RayTracingQueryPools)
	{
		VkHelpers::DestroyDeviceObject(*m_pDevice, qp);
	}
	m_RayTracingQueryPools.Clear();
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


VkDescriptorSet BvFrameDataVk::RequestDescriptorSet(u32 set, const BvShaderResourceLayoutVk* pLayout, BvVector<VkWriteDescriptorSet>& writeSets, u32 descriptorsDataHash, bool bindless)
{
	auto srlHash = (u64)pLayout->GetSetLayoutHandles()[set];

	auto pDescriptorSets = &m_pContextData->m_DescriptorSets;
	u32 maxAllocationsPerPool = 16;
	u64 descriptorSetHash = srlHash;
	if (bindless)
	{
		pDescriptorSets = &m_pContextData->m_BindlessDescriptorSets;
		maxAllocationsPerPool = 1;
	}
	else
	{
		// For regular descriptors, we combine the layout's hash with the descriptors' data hash
		HashCombine(descriptorSetHash, descriptorsDataHash);
	}

	auto result = pDescriptorSets->Emplace(descriptorSetHash, VK_NULL_HANDLE);
	auto& descriptorSet = result.first->second;
	if (result.second)
	{
		// If it's a new entry, we need to allocate a descriptor, so retrieve a pool for it.
		// If a pool doesn't exist for this layout, create one
		auto& pPool = m_pContextData->m_DescriptorPools[srlHash];
		if (!pPool)
		{
			pPool = BV_NEW(BvDescriptorPoolVk)(m_pDevice, pLayout, set, maxAllocationsPerPool);
		}

		descriptorSet = pPool->Allocate();

		if (writeSets.Size() > 0)
		{
			for (auto& writeSet : writeSets)
			{
				writeSet.dstSet = descriptorSet;

				// Bindless descriptors can be changed at any time, so no need to keep track of them
				if (!bindless)
				{
					u64 handle = 0;
					switch (writeSet.descriptorType)
					{
					case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
					case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
					case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
					case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
						handle = u64(writeSet.pBufferInfo->buffer);
						break;
					case VK_DESCRIPTOR_TYPE_SAMPLER:
					case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
					case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
					case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
						handle = u64(writeSet.pImageInfo->imageView);
						break;
					case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
					case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
						handle = u64(*writeSet.pTexelBufferView);
						break;
					case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
						handle = u64(*((VkWriteDescriptorSetAccelerationStructureKHR*)writeSet.pNext)->pAccelerationStructures);
						break;
					}

					if (handle)
					{
						// Add descriptor set to every active resource (as well as the resource if it doesn't exist yet)
						m_pContextData->m_ActiveResources[handle].PushBack({ descriptorSet, pPool });
					}
				}
			}

			vkUpdateDescriptorSets(m_pDevice->GetHandle(), (u32)writeSets.Size(), writeSets.Data(), 0, nullptr);
		}
	}

	return descriptorSet;
}


void BvFrameDataVk::ClearActiveCommandBuffers()
{
	m_CommandBuffers.Clear();
}


VkQueryPool BvFrameDataVk::AddASQueryPool(VkQueryType queryType, u32 queryCount)
{
	auto obj = VkHelpers::CreateQueryPool(m_pDevice, queryType, queryCount);
	BV_ASSERT(obj.first == VK_SUCCESS, "Failed to create Acceleration Structure Query Pool");
	return m_RayTracingQueryPools.PushBack(obj.second.m_QueryPools[0]);
}


VkFramebuffer BvFrameDataVk::GetFramebuffer(const FramebufferDesc& fbDesc)
{
	if (!m_pContextData->m_pFramebufferManager)
	{
		m_pContextData->m_pFramebufferManager = BV_NEW(BvFramebufferManagerVk)(m_pDevice->GetHandle());
	}

	return m_pContextData->m_pFramebufferManager->GetFramebuffer(fbDesc);
}


void BvFrameDataVk::RemoveFramebuffers(VkImageView view)
{
	m_pContextData->m_pFramebufferManager->RemoveFramebuffersWithView(view);
}


BvCommandContextVk::BvCommandContextVk(BvRenderDeviceVk* pDevice, u32 frameCount, u32 queueFamilyIndex, u32 queueIndex)
	: m_pDevice(pDevice), m_Queue(pDevice->GetHandle(), queueFamilyIndex, queueIndex), m_ContextGroupIndex(queueFamilyIndex), m_ContextIndex(queueIndex),
	m_FrameCount(frameCount)
{
	auto pQuerySizes = m_pDevice->GetQueryPoolSizes();
	m_pContextData = BV_NEW(ContextDataVk)();

	auto pFrameData = reinterpret_cast<u8*>(BV_ALLOC(sizeof(BvFrameDataVk) * m_FrameCount, alignof(BvFrameDataVk)));
	for (auto i = 0; i < m_FrameCount; ++i)
	{
		new(pFrameData + i * sizeof(BvFrameDataVk)) BvFrameDataVk(pDevice, queueFamilyIndex, i, m_pContextData);
	}
	m_pFrames = reinterpret_cast<BvFrameDataVk*>(pFrameData);
}


BvCommandContextVk::~BvCommandContextVk()
{
	for (auto i = 0; i < m_FrameCount; ++i)
	{
		m_pFrames[i].~BvFrameDataVk();
	}
	BV_FREE(m_pFrames);

	if (m_pContextData->m_pFramebufferManager)
	{
		BV_DELETE(m_pContextData->m_pFramebufferManager);
	}

	for (auto& poolData : m_pContextData->m_DescriptorPools)
	{
		BV_DELETE(poolData.second);
	}

	BV_DELETE(m_pContextData);
}


void BvCommandContextVk::NewCommandList()
{
	if (m_pCurrCommandBuffer)
	{
		m_pCurrCommandBuffer->End();
	}

	// Get command buffer
	m_pCurrCommandBuffer = m_pFrames[m_ActiveFrameIndex].RequestCommandBuffer();
}


void BvCommandContextVk::AddWaitFence(IBvGPUFence* pFence, u64 value)
{
	// Add a wait semaphore and its value
	m_Queue.AddWaitSemaphore(TO_VK(pFence)->GetHandle(), value);
}


void BvCommandContextVk::AddSignalFence(IBvGPUFence* pFence, u64 value)
{
	// Add a signal semaphore and its value
	m_Queue.AddSignalSemaphore(TO_VK(pFence)->GetHandle(), value);
}


void BvCommandContextVk::Execute()
{
	// Submit active command buffers
	if (m_pCurrCommandBuffer)
	{
		m_pCurrCommandBuffer->End();
	}
	
	auto& commandBuffers = m_pFrames[m_ActiveFrameIndex].GetCommandBuffers();
	auto pFence = m_pFrames[m_ActiveFrameIndex].GetGPUFence();
	auto semaphoreValue = m_pFrames[m_ActiveFrameIndex].UpdateFenceValue();
	m_Queue.Submit(commandBuffers, pFence->GetHandle(), semaphoreValue);

	m_pFrames[m_ActiveFrameIndex].ClearActiveCommandBuffers();

	m_pCurrCommandBuffer = nullptr;
}


void BvCommandContextVk::ExecuteAndWait()
{
	Execute();

	m_Queue.WaitIdle();

	m_pFrames[m_ActiveFrameIndex].Reset(false);
}


void BvCommandContextVk::FlushFrame()
{
	// Sanity check
	BV_ASSERT(m_pCurrCommandBuffer == nullptr, "All command buffers must have been submitted");

	// Get next frame
	m_ActiveFrameIndex = (m_ActiveFrameIndex + 1) % m_FrameCount;

	// Wait for frame's signal value
	m_pFrames[m_ActiveFrameIndex].Reset();

	// Get swap chains ready
	for (auto& pSwapChain : m_SwapChains)
	{
		pSwapChain->AcquireImage();
	}
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


void BvCommandContextVk::SetRenderTargets(u32 renderTargetCount, const RenderTargetDesc* pRenderTargets, u32 multiviewCount)
{
	m_pCurrCommandBuffer->SetRenderTargets(renderTargetCount, pRenderTargets, multiviewCount);
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


void BvCommandContextVk::SetDynamicConstantBuffer(IBvBufferView* pResource, u32 offset, u32 set, u32 binding)
{
	m_pCurrCommandBuffer->SetDynamicConstantBuffer(pResource, offset, set, binding);
}


void BvCommandContextVk::SetDynamicStructuredBuffer(IBvBufferView* pResource, u32 offset, u32 set, u32 binding)
{
	m_pCurrCommandBuffer->SetDynamicStructuredBuffer(pResource, offset, set, binding);
}


void BvCommandContextVk::SetDynamicRWStructuredBuffer(IBvBufferView* pResource, u32 offset, u32 set, u32 binding)
{
	m_pCurrCommandBuffer->SetDynamicRWStructuredBuffer(pResource, offset, set, binding);
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


void BvCommandContextVk::SetBlendConstants(const float(&colors)[4])
{
	m_pCurrCommandBuffer->SetBlendConstants(colors);
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


bool BvCommandContextVk::SupportsQueryType(QueryType queryType) const
{
	auto pDeviceInfo = m_pDevice->GetDeviceInfo();
	auto& contextGroup = m_pDevice->GetGPUInfo().m_ContextGroups[m_ContextGroupIndex];
	if (queryType == QueryType::kTimestamp)
	{
		if (pDeviceInfo->m_DeviceProperties.properties.limits.timestampPeriod == 0)
		{
			return false;
		}

		if (pDeviceInfo->m_DeviceProperties.properties.limits.timestampComputeAndGraphics
			&& (contextGroup.SupportsCommandType(CommandType::kGraphics) || contextGroup.SupportsCommandType(CommandType::kCompute)))
		{
			return true;
		}
		else
		{
			auto& props = pDeviceInfo->m_QueueFamilyProperties[m_ContextGroupIndex].queueFamilyProperties;
			return props.timestampValidBits != 0;
		}
	}
	else if (queryType == QueryType::kMeshPipelineStatistics)
	{
		return EHasFlag(m_pDevice->GetGPUInfo().m_DeviceCaps, RenderDeviceCapabilities::kMeshQuery) && contextGroup.SupportsCommandType(CommandType::kGraphics);
	}

	return contextGroup.SupportsCommandType(CommandType::kGraphics);
}


void BvCommandContextVk::ResetQueryHeap(IBvQueryHeap* pQueryHeap, u32 startIndex, u32 queryCount)
{
	m_pCurrCommandBuffer->ResetQueryHeap(pQueryHeap, startIndex, queryCount);
}


void BvCommandContextVk::BeginQuery(IBvQueryHeap* pQueryHeap, u32 index)
{
	m_pCurrCommandBuffer->BeginQuery(pQueryHeap, index);
}


void BvCommandContextVk::EndQuery(IBvQueryHeap* pQueryHeap, u32 index)
{
	m_pCurrCommandBuffer->EndQuery(pQueryHeap, index);
}


void BvCommandContextVk::ResolveQueryData(IBvQueryHeap* pQueryHeap, u32 startIndex, u32 queryCount, IBvBuffer* pDstBuffer, u64 offset)
{
	m_pCurrCommandBuffer->ResolveQueryData(pQueryHeap, startIndex, queryCount, pDstBuffer, offset);
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


void BvCommandContextVk::BuildRayTracingAccelerationStructures(u32 count, const RayTracingAccelerationStructureBuildDesc* pBuildDescs,
	const RayTracingAccelerationStructurePostBuildDesc* pPostBuildDesc)
{
	m_pCurrCommandBuffer->BuildRayTracingAccelerationStructures(count, pBuildDescs, pPostBuildDesc);
}


void BvCommandContextVk::EmitRayTracingAccelerationStructurePostBuild(u32 count, IBvAccelerationStructure* const* ppAccelerationStructures,
	const RayTracingAccelerationStructurePostBuildDesc& postBuildDesc)
{
	m_pCurrCommandBuffer->EmitASPostBuild(count, ppAccelerationStructures, postBuildDesc);
}


void BvCommandContextVk::CopyRayTracingAccelerationStructure(const RayTracingAccelerationStructureCopyDesc& copyDesc)
{
	m_pCurrCommandBuffer->CopyRayTracingAccelerationStructure(copyDesc);
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
	for (auto i = 0; i < m_FrameCount; ++i)
	{
		m_pFrames[i].RemoveFramebuffers(view);
	}
}


void BvCommandContextVk::OnResourceDeleted(u64 handle, bool isTexture)
{
	BvScopedLock lock(m_pContextData->m_DeletedResourceLock);
	m_pContextData->m_DeletedResourceHandles.PushBack({ handle, isTexture });
	m_pContextData->m_DeletedResourceCounter.fetch_add(1, std::memory_order_relaxed);
}


void BvCommandContextVk::Destroy()
{
}