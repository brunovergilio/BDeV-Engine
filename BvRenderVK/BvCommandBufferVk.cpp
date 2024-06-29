#include "BvRenderVK/BvCommandBufferVk.h"
#include "BvUtilsVk.h"
#include "BvFramebufferVk.h"
#include "BvTextureViewVk.h"
#include "BvRenderPassVk.h"
#include "BvPipelineStateVk.h"
#include "BvBufferVk.h"
#include "BvTypeConversionsVk.h"
#include "BvCommandPoolVk.h"
#include "BvShaderResourceVk.h"
#include "BvDescriptorSetVk.h"
#include "BvTextureVk.h"
#include "BvSwapChainVk.h"
#include "BvSemaphoreVk.h"
#include "BvBufferVk.h"
#include "BvBufferViewVk.h"
#include "BvSamplerVk.h"
#include "BvQueryVk.h"
#include "BvCommandQueueVk.h"
#include "BvCommandContextVk.h"
#include "BDeV/RenderAPI/BvRenderAPIUtils.h"


BvCommandBufferVk::BvCommandBufferVk(const BvRenderDeviceVk* pDevice, VkCommandBuffer commandBuffer, BvFrameDataVk* pFrameData)
	: m_pDevice(pDevice), m_CommandBuffer(commandBuffer), m_pFrameData(pFrameData)
{
}


BvCommandBufferVk::BvCommandBufferVk(BvCommandBufferVk&& rhs) noexcept
{
	*this = std::move(rhs);
}


BvCommandBufferVk& BvCommandBufferVk::operator=(BvCommandBufferVk&& rhs) noexcept
{
	m_pDevice = rhs.m_pDevice;
	m_CommandBuffer = rhs.m_CommandBuffer;

	return *this;
}


BvCommandBufferVk::~BvCommandBufferVk()
{
	m_CommandBuffer = VK_NULL_HANDLE;
}


void BvCommandBufferVk::Reset()
{
	m_SwapChains.Clear();
	m_pGraphicsPipeline = nullptr;
	m_pComputePipeline = nullptr;
	m_CurrentState = State::kRecording;
}


void BvCommandBufferVk::Begin()
{
	VkCommandBufferBeginInfo cmdBI{};
	cmdBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBI.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(m_CommandBuffer, &cmdBI);

	m_CurrentState = State::kRecording;
}


void BvCommandBufferVk::End()
{
	ResetRenderTargets();

	vkEndCommandBuffer(m_CommandBuffer);
}


void BvCommandBufferVk::BeginRenderPass(const BvRenderPass* pRenderPass, u32 renderPassTargetCount, const RenderPassTargetDesc* pRenderPassTargets)
{
	BvAssert(pRenderPass != nullptr, "Invalid render pass");
	BvAssert(pRenderPassTargets != nullptr, "No render / depth targets");
	
	ResetRenderTargets();

	VkExtent2D renderArea{};

	auto & renderPassDesc = pRenderPass->GetDesc();
	BvFixedVector<VkClearValue, kMaxRenderTargetsWithDepth> clearValues(renderPassDesc.m_RenderTargets.Size() + (renderPassDesc.m_HasDepth ? 1 : 0));
	BvFixedVector<VkImageView, kMaxRenderTargetsWithDepth> views(clearValues.Size());
	FramebufferDesc frameBufferDesc;
	u32 numIgnoredBarrierTransitions = 0;
	for (u32 i = 0; i < renderPassDesc.m_RenderTargets.Size(); i++)
	{
		auto pViewVk = static_cast<BvTextureViewVk * const>(pRenderPassTargets[i].m_pView);
		auto & desc = pViewVk->GetTexture()->GetDesc();
		frameBufferDesc.m_RenderTargetViews.PushBack(pViewVk);
		std::copy(pRenderPassTargets[i].m_ClearValues.colors, pRenderPassTargets[i].m_ClearValues.colors + 4, clearValues[i].color.float32);

		renderArea.width = std::max(renderArea.width, desc.m_Size.width);
		renderArea.height = std::max(renderArea.height, desc.m_Size.height);

		views[i] = pViewVk->GetHandle();

		// If any of the view objects happen to be a swap chain texture, we need to request
		// a semaphore from the swap chain
		if (pViewVk->GetTexture()->GetClassType() == BvTexture::ClassType::kSwapChainTexture)
		{
			auto pSwapChain = static_cast<BvSwapChainTextureVk *>(pViewVk->GetTexture())->GetSwapChain();
			m_SwapChains.EmplaceBack(pSwapChain);
		}
	}

	//if (renderPassDesc.m_HasDepth)
	//{
	//	VkClearValue clearValue;
	//	clearValue.depthStencil = { pDepthStencilTarget->m_ClearValue.depth, pDepthStencilTarget->m_ClearValue.stencil };
	//	clearValues[clearValues.Size() - 1] = clearValue;
	//	frameBufferDesc.m_pDepthStencilView = static_cast<BvTextureViewVk * const>(pDepthStencilTarget->m_pView);

	//	// We do the same as above, but for depth
	//	for (auto i = 0; i < m_RenderTargetTransitions.Size(); i++)
	//	{
	//		VkImage image = static_cast<BvTextureVk *>(frameBufferDesc.m_pDepthStencilView->GetTexture())->GetHandle();

	//		if (m_RenderTargetTransitions[i].image == image
	//			&& pRenderPass->GetDesc().m_RenderTargets[i].m_LoadOp != LoadOp::kLoad)
	//		{
	//			std::swap(m_RenderTargetTransitions[i], m_RenderTargetTransitions[m_RenderTargetTransitions.Size() - 1 - numIgnoredBarrierTransitions++]);
	//			break;
	//		}
	//	}
	//}

	//auto renderPass = static_cast<const BvRenderPassVk * const>(pRenderPass)->GetHandle();
	//frameBufferDesc.m_RenderPass = renderPass;

	//auto pFramebuffer = m_Device.GetFramebufferManager()->GetFramebuffer(m_Device, frameBufferDesc);

	//VkRenderPassBeginInfo renderPassBI{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	//renderPassBI.renderPass = renderPass;
	//renderPassBI.framebuffer = pFramebuffer->GetHandle();
	//renderPassBI.renderArea.extent = renderArea;
	//renderPassBI.clearValueCount = (u32)clearValues.Size();
	//renderPassBI.pClearValues = clearValues.Data();

	//vkCmdBeginRenderPass(m_CommandBuffer, &renderPassBI, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

	//m_CurrentState = State::kRenderPass;
}


void BvCommandBufferVk::NextSubpass()
{
	BvAssert(false, "Not implemented");
}


void BvCommandBufferVk::EndRenderPass()
{
	BvAssert(m_CurrentState == State::kRenderPass, "Command buffer not in render pass");

	vkCmdEndRenderPass(m_CommandBuffer);

	m_CurrentState = State::kRecording;
}


void BvCommandBufferVk::SetRenderTargets(u32 renderTargetCount, const RenderTargetDesc* pRenderTargets)
{
	ResetRenderTargets();

	if (!renderTargetCount)
	{
		return;
	}

	VkExtent2D renderArea{};

	VkRenderingAttachmentInfo colorAttachments[kMaxRenderTargets]{};
	VkRenderingAttachmentInfo depthAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
	VkRenderingAttachmentInfo stencilAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
	
	u32 colorAttachmentCount = 0;
	u32 resolveCount = 0;
	bool hasDepth = false;
	bool hasStencil = true;
	bool hasShadingRate = false;

	u32 layerCount = 1;
	for (u32 i = 0; i < renderTargetCount; i++)
	{
		if (colorAttachmentCount >= 8)
		{
			continue;
		}

		const auto& renderTarget = pRenderTargets[i];
		auto pView = static_cast<BvTextureViewVk*>(renderTarget.m_pView);
		auto pTexture = static_cast<BvTexture*>(renderTarget.m_pView->GetTexture());
		auto& desc = renderTarget.m_pView->GetTexture()->GetDesc();
		auto& viewDesc = renderTarget.m_pView->GetDesc();

		layerCount = std::min(viewDesc.m_SubresourceDesc.layerCount, layerCount);
		renderArea.width = std::max(renderArea.width, desc.m_Size.width);
		renderArea.height = std::max(renderArea.height, desc.m_Size.height);

		auto loadOp = GetVkAttachmentLoadOp(renderTarget.m_LoadOp);
		auto storeOp = GetVkAttachmentStoreOp(renderTarget.m_StoreOp);

		switch (renderTarget.m_Type)
		{
		case RenderTargetType::kColor:
			colorAttachments[colorAttachmentCount].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			colorAttachments[colorAttachmentCount].imageView = pView->GetHandle();
			colorAttachments[colorAttachmentCount].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachments[colorAttachmentCount].loadOp = loadOp;
			colorAttachments[colorAttachmentCount].storeOp = storeOp;
			if (loadOp == VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR)
			{
				std::copy(renderTarget.m_ClearValues.colors, renderTarget.m_ClearValues.colors + 4, colorAttachments[colorAttachmentCount].clearValue.color.float32);
			}

			// If any of the view objects happen to be a swap chain texture, we need to request
			// a semaphore from the swap chain
			if (pTexture->GetClassType() == BvTexture::ClassType::kSwapChainTexture)
			{
				auto pSwapChain = static_cast<BvSwapChainTextureVk*>(pTexture)->GetSwapChain();
				m_SwapChains.EmplaceBack(pSwapChain);
			}
			++colorAttachmentCount;
			break;
		case RenderTargetType::kDepthStencil:
			depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			depthAttachment.imageView = pView->GetHandle();
			depthAttachment.imageLayout = renderTarget.m_State == ResourceState::kDepthStencilWrite ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
				: VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
			depthAttachment.loadOp = loadOp;
			depthAttachment.storeOp = storeOp;
			if (loadOp == VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR)
			{
				depthAttachment.clearValue.depthStencil.depth = renderTarget.m_ClearValues.depth;
			}
			hasDepth = true;

			if (IsDepthStencilFormat(desc.m_Format))
			{
				stencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
				stencilAttachment.imageView = pView->GetHandle();
				stencilAttachment.imageLayout = renderTarget.m_State == ResourceState::kDepthStencilWrite ? VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL
					: VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
				stencilAttachment.loadOp = loadOp;
				stencilAttachment.storeOp = storeOp;
				if (loadOp == VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR)
				{
					stencilAttachment.clearValue.depthStencil.stencil = renderTarget.m_ClearValues.stencil;
				}
				hasStencil = true;
			}
			break;
		case RenderTargetType::kResolve:
			BvAssert(false, "Not implemented");
			break;
		case RenderTargetType::kShadingRate:
			BvAssert(false, "Not implemented");
			break;
		}

		auto pTextureVk = static_cast<BvTextureVk*>(pTexture);
		if (renderTarget.m_StateBefore != renderTarget.m_State)
		{
			auto& barrier = m_PreRenderBarriers.EmplaceBack(VkImageMemoryBarrier2{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 });
			//barrier.pNext = nullptr;
			barrier.image = pTextureVk->GetHandle();
			barrier.subresourceRange.aspectMask = GetVkFormatMap(pTextureVk->GetDesc().m_Format).aspectFlags;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = desc.m_MipLevels;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = viewDesc.m_SubresourceDesc.layerCount;

			barrier.oldLayout = GetVkImageLayout(renderTarget.m_StateBefore);
			barrier.newLayout = GetVkImageLayout(renderTarget.m_State);

			barrier.srcAccessMask = GetVkAccessFlags(renderTarget.m_StateBefore);
			barrier.dstAccessMask = GetVkAccessFlags(renderTarget.m_State);

			barrier.srcStageMask = GetVkPipelineStageFlags(barrier.srcAccessMask);
			barrier.dstStageMask = GetVkPipelineStageFlags(barrier.dstAccessMask);

			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		}
		if (renderTarget.m_State != renderTarget.m_StateAfter)
		{
			auto& barrier = m_PostRenderBarriers.EmplaceBack(VkImageMemoryBarrier2{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 });
			//barrier.pNext = nullptr;
			barrier.image = pTextureVk->GetHandle();
			barrier.subresourceRange.aspectMask = GetVkFormatMap(pTextureVk->GetDesc().m_Format).aspectFlags;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = desc.m_MipLevels;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = desc.m_LayerCount;

			barrier.oldLayout = GetVkImageLayout(renderTarget.m_State);
			barrier.newLayout = GetVkImageLayout(renderTarget.m_StateAfter);

			barrier.srcAccessMask = GetVkAccessFlags(renderTarget.m_State);
			barrier.dstAccessMask = GetVkAccessFlags(renderTarget.m_StateAfter);

			barrier.srcStageMask = GetVkPipelineStageFlags(barrier.srcAccessMask);
			barrier.dstStageMask = GetVkPipelineStageFlags(barrier.dstAccessMask);

			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		}
	}

	if (m_PreRenderBarriers.Size() > 0)
	{
		VkDependencyInfo dependencyInfo{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
		dependencyInfo.imageMemoryBarrierCount = static_cast<u32>(m_PreRenderBarriers.Size());
		dependencyInfo.pImageMemoryBarriers = m_PreRenderBarriers.Data();

		vkCmdPipelineBarrier2(m_CommandBuffer, &dependencyInfo);

		m_PreRenderBarriers.Clear();
	}

	VkRenderingInfoKHR renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.renderArea = { 0, 0, renderArea.width, renderArea.height };
	renderingInfo.layerCount = layerCount;
	if (colorAttachmentCount > 0)
	{
		renderingInfo.colorAttachmentCount = colorAttachmentCount;
		renderingInfo.pColorAttachments = colorAttachments;
	}
	if (hasDepth)
	{
		renderingInfo.pDepthAttachment = &depthAttachment;
		if (hasStencil)
		{
			renderingInfo.pStencilAttachment = &stencilAttachment;
		}
	}

	vkCmdBeginRendering(m_CommandBuffer, &renderingInfo);

	m_CurrentState = State::kRenderTarget;
}


void BvCommandBufferVk::SetViewports(u32 viewportCount, const Viewport* pViewports)
{
	constexpr u32 kMaxViewports = 8;
	BvAssert(viewportCount <= kMaxViewports, "Viewport count greater than limit");
	
	vkCmdSetViewportWithCount(m_CommandBuffer, viewportCount, reinterpret_cast<const VkViewport*>(pViewports));
}


void BvCommandBufferVk::SetScissors(u32 scissorCount, const Rect* pScissors)
{
	constexpr u32 kMaxScissors = 8;
	BvAssert(scissorCount <= kMaxScissors, "Scissor count greater than limit");

	vkCmdSetScissorWithCount(m_CommandBuffer, scissorCount, reinterpret_cast<const VkRect2D*>(pScissors));
}


void BvCommandBufferVk::SetGraphicsPipeline(const BvGraphicsPipelineState* pPipeline)
{
	m_pComputePipeline = nullptr;
	m_pGraphicsPipeline = static_cast<const BvGraphicsPipelineStateVk*>(pPipeline);
	vkCmdBindPipeline(m_CommandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_pGraphicsPipeline->GetHandle());
}


void BvCommandBufferVk::SetComputePipeline(const BvComputePipelineState* pPipeline)
{
	ResetRenderTargets();

	m_pGraphicsPipeline = nullptr;
	m_pComputePipeline = static_cast<const BvComputePipelineStateVk*>(pPipeline);
	vkCmdBindPipeline(m_CommandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE, m_pComputePipeline->GetHandle());
}


void BvCommandBufferVk::SetShaderResourceParams(u32 setCount, BvShaderResourceParams* const* ppSets, u32 firstSet)
{
}


void BvCommandBufferVk::SetShaderResource(const BvBufferView* pResource, u32 set, u32 binding, u32 arrayIndex)
{
	auto& shaderResourceLayout = m_pGraphicsPipeline->GetDesc().m_pShaderResourceLayout->GetDesc();
	auto resourceSet = shaderResourceLayout.m_ShaderResources.FindKey(set);
	if (resourceSet == shaderResourceLayout.m_ShaderResources.cend())
	{
		return;
	}
	auto resource = resourceSet->second.FindKey(binding);
	if (resource == resourceSet->second.cend())
	{
		return;
	}

	auto& bindingState = m_pFrameData->GetResourceBindingState();
	bindingState.SetResource(GetVkDescriptorType(resource->second.m_ShaderResourceType), static_cast<const BvBufferViewVk*>(pResource), set, binding, arrayIndex);
}


void BvCommandBufferVk::SetShaderResource(const BvTextureView* pResource, u32 set, u32 binding, u32 arrayIndex)
{
	auto& shaderResourceLayout = m_pGraphicsPipeline->GetDesc().m_pShaderResourceLayout->GetDesc();
	auto resourceSet = shaderResourceLayout.m_ShaderResources.FindKey(set);
	if (resourceSet == shaderResourceLayout.m_ShaderResources.cend())
	{
		return;
	}
	auto resource = resourceSet->second.FindKey(binding);
	if (resource == resourceSet->second.cend())
	{
		return;
	}

	auto& bindingState = m_pFrameData->GetResourceBindingState();
	bindingState.SetResource(GetVkDescriptorType(resource->second.m_ShaderResourceType), static_cast<const BvTextureViewVk*>(pResource), set, binding, arrayIndex);
}


void BvCommandBufferVk::SetShaderResource(const BvSampler* pResource, u32 set, u32 binding, u32 arrayIndex)
{
	auto& shaderResourceLayout = m_pGraphicsPipeline->GetDesc().m_pShaderResourceLayout->GetDesc();
	auto resourceSet = shaderResourceLayout.m_ShaderResources.FindKey(set);
	if (resourceSet == shaderResourceLayout.m_ShaderResources.cend())
	{
		return;
	}
	auto resource = resourceSet->second.FindKey(binding);
	if (resource == resourceSet->second.cend())
	{
		return;
	}

	auto& bindingState = m_pFrameData->GetResourceBindingState();
	bindingState.SetResource(GetVkDescriptorType(resource->second.m_ShaderResourceType), static_cast<const BvSamplerVk*>(pResource), set, binding, arrayIndex);
}


void BvCommandBufferVk::SetVertexBufferViews(u32 vertexBufferCount, const BvBufferView* const* pVertexBufferViews, u32 firstBinding)
{
	constexpr u32 kMaxVertexBufferViews = 16;

	BvFixedVector<VkBuffer, kMaxVertexBufferViews> vertexBuffers(vertexBufferCount);
	BvFixedVector<VkDeviceSize, kMaxVertexBufferViews> vertexBufferOffsets(vertexBufferCount);
	for (auto i = 0u; i < vertexBufferCount; i++)
	{
		vertexBuffers[i] = static_cast<BvBufferVk*>(pVertexBufferViews[i]->GetBuffer())->GetHandle();
		vertexBufferOffsets[i] = pVertexBufferViews[i]->GetDesc().m_Offset;
	}

	vkCmdBindVertexBuffers(m_CommandBuffer, firstBinding, vertexBuffers.Size(), vertexBuffers.Data(), vertexBufferOffsets.Data());
}


void BvCommandBufferVk::SetIndexBufferView(const BvBufferView* pIndexBufferView, IndexFormat indexFormat)
{
	auto formatVk = GetVkIndexType(indexFormat);
	
	vkCmdBindIndexBuffer(m_CommandBuffer, static_cast<BvBufferVk*>(pIndexBufferView->GetBuffer())->GetHandle(), pIndexBufferView->GetDesc().m_Offset, formatVk);
}


void BvCommandBufferVk::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
{
	FlushDescriptorSets();

	vkCmdDraw(m_CommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}


void BvCommandBufferVk::DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstInstance)
{
	FlushDescriptorSets();

	vkCmdDrawIndexed(m_CommandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}


void BvCommandBufferVk::Dispatch(u32 x, u32 y, u32 z)
{
	FlushDescriptorSets();

	vkCmdDispatch(m_CommandBuffer, x, y, z);
}


void BvCommandBufferVk::DrawIndirect(const BvBuffer* pBuffer, u32 drawCount, u64 offset)
{
	FlushDescriptorSets();

	auto pBufferVk = static_cast<const BvBufferVk*>(pBuffer)->GetHandle();
	vkCmdDrawIndirect(m_CommandBuffer, pBufferVk, offset, drawCount, (u32)sizeof(VkDrawIndirectCommand));
}


void BvCommandBufferVk::DrawIndexedIndirect(const BvBuffer* pBuffer, u32 drawCount, u64 offset)
{
	FlushDescriptorSets();

	auto pBufferVk = static_cast<const BvBufferVk*>(pBuffer)->GetHandle();
	vkCmdDrawIndexedIndirect(m_CommandBuffer, pBufferVk, offset, drawCount, (u32)sizeof(VkDrawIndexedIndirectCommand));
}


void BvCommandBufferVk::DispatchIndirect(const BvBuffer* pBuffer, u64 offset)
{
	FlushDescriptorSets();

	auto pBufferVk = static_cast<const BvBufferVk*>(pBuffer)->GetHandle();
	vkCmdDispatchIndirect(m_CommandBuffer, pBufferVk, offset);
}


void BvCommandBufferVk::DispatchMesh(u32 x, u32 y, u32 z)
{
	vkCmdDrawMeshTasksEXT(m_CommandBuffer, x, y, z);
}


void BvCommandBufferVk::DispatchMeshIndirect(const BvBuffer* pBuffer, u64 offset)
{
	vkCmdDrawMeshTasksIndirectEXT(m_CommandBuffer, TO_VK(pBuffer)->GetHandle(), offset, 1, sizeof(VkDispatchIndirectCommand));
}


void BvCommandBufferVk::DispatchMeshIndirectCount(const BvBuffer* pBuffer, u64 offset, const BvBuffer* pCountBuffer, u64 countOffset, u32 maxCount)
{
	vkCmdDrawMeshTasksIndirectCountEXT(m_CommandBuffer, TO_VK(pBuffer)->GetHandle(), offset, TO_VK(pCountBuffer)->GetHandle(), countOffset,
		maxCount, sizeof(VkDispatchIndirectCommand));
}


void BvCommandBufferVk::CopyBuffer(const BvBufferVk* pSrcBuffer, BvBufferVk* pDstBuffer, const VkBufferCopy& copyRegion)
{
	ResetRenderTargets();

	vkCmdCopyBuffer(m_CommandBuffer, pSrcBuffer->GetHandle(), pDstBuffer->GetHandle(), 1, &copyRegion);
}


void BvCommandBufferVk::CopyBuffer(const BvBuffer* pSrcBuffer, BvBuffer* pDstBuffer)
{
	VkBufferCopy region{};
	region.srcOffset = 0;
	region.dstOffset = 0;
	region.size = std::min(pSrcBuffer->GetDesc().m_Size, pDstBuffer->GetDesc().m_Size);

	auto pSrc = static_cast<const BvBufferVk*>(pSrcBuffer);
	auto pDst = static_cast<BvBufferVk*>(pDstBuffer);

	CopyBuffer(pSrc, pDst, region);
}


void BvCommandBufferVk::CopyBuffer(const BvBuffer* pSrcBuffer, BvBuffer* pDstBuffer, const BufferCopyDesc& copyDesc)
{
	VkBufferCopy region{};
	region.srcOffset = copyDesc.m_SrcOffset;
	region.dstOffset = copyDesc.m_DstOffset;
	region.size = std::min(copyDesc.m_SrcSize, pSrcBuffer->GetDesc().m_Size);

	auto pSrc = static_cast<const BvBufferVk*>(pSrcBuffer);
	auto pDst = static_cast<BvBufferVk*>(pDstBuffer);

	CopyBuffer(pSrc, pDst, region);
}


void BvCommandBufferVk::CopyTexture(const BvTexture* pSrcTexture, BvTexture* pDstTexture)
{
	ResetRenderTargets();

	auto& srcDesc = pSrcTexture->GetDesc();
	auto& dstDesc = pDstTexture->GetDesc();

	if (srcDesc.m_Format != dstDesc.m_Format || srcDesc.m_Size != dstDesc.m_Size
		|| srcDesc.m_MipLevels != dstDesc.m_MipLevels || srcDesc.m_LayerCount != dstDesc.m_LayerCount)
	{
		return;
	}

	for (auto layer = 0u; layer < srcDesc.m_LayerCount; ++layer)
	{
		for (auto mip = 0u; mip < srcDesc.m_MipLevels; ++mip)
		{
			VkImageCopy& imageCopyRegion = m_ImageCopyRegions.EmplaceBack();
			imageCopyRegion.srcOffset = { 0, 0, 0 };
			imageCopyRegion.dstOffset = { 0, 0, 0 };

			imageCopyRegion.srcSubresource.aspectMask = GetVkFormatMap(srcDesc.m_Format).aspectFlags;
			imageCopyRegion.srcSubresource.mipLevel = mip;
			imageCopyRegion.srcSubresource.baseArrayLayer = layer;
			imageCopyRegion.srcSubresource.layerCount = 1;

			imageCopyRegion.dstSubresource = imageCopyRegion.srcSubresource;

			imageCopyRegion.extent =
			{
				std::max(1u, srcDesc.m_Size.width >> mip),
				std::max(1u, srcDesc.m_Size.height >> mip),
				std::max(1u, srcDesc.m_Size.depth >> mip)
			};
		}
	}

	auto pSrc = static_cast<const BvTextureVk*>(pSrcTexture);
	auto pDst = static_cast<BvTextureVk*>(pDstTexture);

	vkCmdCopyImage(m_CommandBuffer, pSrc->GetHandle(), VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		pDst->GetHandle(), VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		(u32)m_ImageCopyRegions.Size(), m_ImageCopyRegions.Data());
	
	m_ImageCopyRegions.Clear();
}


void BvCommandBufferVk::CopyTexture(const BvTexture* pSrcTexture, BvTexture* pDstTexture, const TextureCopyDesc& copyDesc)
{
	ResetRenderTargets();

	VkImageCopy imageCopyRegion{};
	imageCopyRegion.srcOffset =
	{
		copyDesc.m_SrcTextureOffset.x,
		copyDesc.m_SrcTextureOffset.y,
		copyDesc.m_SrcTextureOffset.z
	};
	
	imageCopyRegion.dstOffset =
	{
		copyDesc.m_DstTextureOffset.x,
		copyDesc.m_DstTextureOffset.y,
		copyDesc.m_DstTextureOffset.z
	};

	auto& srcDesc = pSrcTexture->GetDesc();
	auto& dstDesc = pDstTexture->GetDesc();

	imageCopyRegion.srcSubresource.aspectMask = GetVkFormatMap(srcDesc.m_Format).aspectFlags;
	imageCopyRegion.srcSubresource.mipLevel = copyDesc.m_SrcMip;
	imageCopyRegion.srcSubresource.baseArrayLayer = copyDesc.m_SrcLayer;
	imageCopyRegion.srcSubresource.layerCount = 1;

	imageCopyRegion.dstSubresource.aspectMask = GetVkFormatMap(dstDesc.m_Format).aspectFlags;
	imageCopyRegion.dstSubresource.mipLevel = copyDesc.m_DstMip;
	imageCopyRegion.dstSubresource.baseArrayLayer = copyDesc.m_DstLayer;
	imageCopyRegion.dstSubresource.layerCount = 1;

	imageCopyRegion.extent =
	{
		std::min(copyDesc.m_Size.width, std::min(srcDesc.m_Size.width, dstDesc.m_Size.width)),
		std::min(copyDesc.m_Size.height, std::min(srcDesc.m_Size.height, dstDesc.m_Size.height)),
		std::min(copyDesc.m_Size.depth, std::min(srcDesc.m_Size.depth, dstDesc.m_Size.depth))
	};

	auto pSrc = static_cast<const BvTextureVk*>(pSrcTexture);
	auto pDst = static_cast<BvTextureVk*>(pDstTexture);

	vkCmdCopyImage(m_CommandBuffer, pSrc->GetHandle(), VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		pDst->GetHandle(), VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1u, &imageCopyRegion);
}


void BvCommandBufferVk::CopyBufferToTexture(const BvBufferVk* pSrcBuffer, BvTextureVk* pDstTexture, u32 copyCount, const VkBufferImageCopy* pCopyRegions)
{
	ResetRenderTargets();

	vkCmdCopyBufferToImage(m_CommandBuffer, pSrcBuffer->GetHandle(), pDstTexture->GetHandle(),
		VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, copyCount, pCopyRegions);
}


void BvCommandBufferVk::CopyBufferToTexture(const BvBuffer* pSrcBuffer, BvTexture* pDstTexture, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs)
{
	auto pSrc = static_cast<const BvBufferVk* const>(pSrcBuffer);
	auto pDst = static_cast<BvTextureVk*>(pDstTexture);

	m_BufferImageCopyRegions.Resize(copyCount, {});
	for (auto i = 0u; i < copyCount; ++i)
	{
		m_BufferImageCopyRegions[i].bufferOffset = pCopyDescs[i].m_BufferOffset;
		//m_BufferImageCopyRegions[i].bufferImageHeight = 0;
		//m_BufferImageCopyRegions[i].bufferRowLength = 0;
		m_BufferImageCopyRegions[i].imageExtent = { pCopyDescs[i].m_TextureSize.width, pCopyDescs[i].m_TextureSize.height, pCopyDescs[i].m_TextureSize.depth };
		m_BufferImageCopyRegions[i].imageOffset = { pCopyDescs[i].m_TextureOffset.x, pCopyDescs[i].m_TextureOffset.y, pCopyDescs[i].m_TextureOffset.z };
		m_BufferImageCopyRegions[i].imageSubresource.aspectMask = GetVkFormatMap(pDst->GetDesc().m_Format).aspectFlags;
		m_BufferImageCopyRegions[i].imageSubresource.mipLevel = pCopyDescs[i].m_Mip;
		m_BufferImageCopyRegions[i].imageSubresource.baseArrayLayer = pCopyDescs[i].m_Layer;
		m_BufferImageCopyRegions[i].imageSubresource.layerCount = 1;
	}

	CopyBufferToTexture(pSrc, pDst, copyCount, m_BufferImageCopyRegions.Data());
	
	m_BufferImageCopyRegions.Clear();
}


void BvCommandBufferVk::CopyTextureToBuffer(const BvTextureVk* pSrcTexture, BvBufferVk* pDstBuffer, u32 copyCount, const VkBufferImageCopy* pCopyRegions)
{
	ResetRenderTargets();

	vkCmdCopyImageToBuffer(m_CommandBuffer, pSrcTexture->GetHandle(), VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		pDstBuffer->GetHandle(), copyCount, pCopyRegions);
}


void BvCommandBufferVk::CopyTextureToBuffer(const BvTexture* pSrcTexture, BvBuffer* pDstBuffer, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs)
{
	auto pSrc = static_cast<const BvTextureVk*>(pSrcTexture);
	auto pDst = static_cast<BvBufferVk*>(pDstBuffer);

	m_BufferImageCopyRegions.Resize(copyCount, {});
	for (auto i = 0u; i < copyCount; ++i)
	{
		m_BufferImageCopyRegions[i].bufferOffset = pCopyDescs[i].m_BufferOffset;
		//m_BufferImageCopyRegions[i].bufferImageHeight = 0;
		//m_BufferImageCopyRegions[i].bufferRowLength = 0;
		m_BufferImageCopyRegions[i].imageExtent = { pCopyDescs[i].m_TextureSize.width, pCopyDescs[i].m_TextureSize.height, pCopyDescs[i].m_TextureSize.depth };
		m_BufferImageCopyRegions[i].imageOffset = { pCopyDescs[i].m_TextureOffset.x, pCopyDescs[i].m_TextureOffset.y, pCopyDescs[i].m_TextureOffset.z };
		m_BufferImageCopyRegions[i].imageSubresource.aspectMask = GetVkFormatMap(pSrc->GetDesc().m_Format).aspectFlags;
		m_BufferImageCopyRegions[i].imageSubresource.mipLevel = pCopyDescs[i].m_Mip;
		m_BufferImageCopyRegions[i].imageSubresource.baseArrayLayer = pCopyDescs[i].m_Layer;
		m_BufferImageCopyRegions[i].imageSubresource.layerCount = 1;
	}

	CopyTextureToBuffer(pSrc, pDst, copyCount, m_BufferImageCopyRegions.Data());

	m_BufferImageCopyRegions.Clear();
}


void BvCommandBufferVk::ResourceBarrier(u32 bufferBarrierCount, const VkBufferMemoryBarrier2* pBufferBarriers,
	u32 imageBarrierCount, const VkImageMemoryBarrier2* pImageBarriers, u32 memoryBarrierCount, const VkMemoryBarrier2* pMemoryBarriers)
{
	ResetRenderTargets();

	VkDependencyInfo di{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
	di.bufferMemoryBarrierCount = bufferBarrierCount;
	di.pBufferMemoryBarriers = pBufferBarriers;
	di.imageMemoryBarrierCount = imageBarrierCount;
	di.pImageMemoryBarriers = pImageBarriers;
	di.memoryBarrierCount = memoryBarrierCount;
	di.pMemoryBarriers = pMemoryBarriers;

	vkCmdPipelineBarrier2(m_CommandBuffer, &di);
}


void BvCommandBufferVk::ResourceBarrier(u32 barrierCount, const ResourceBarrierDesc* pBarriers)
{
	for (auto i = 0u; i < barrierCount; i++)
	{
		if (pBarriers[i].m_pBuffer)
		{
			auto& barrier = m_BufferBarriers.EmplaceBack(VkBufferMemoryBarrier2{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2 });
			//barrier.pNext = nullptr;
			barrier.buffer = static_cast<BvBufferVk *>(pBarriers[i].m_pBuffer)->GetHandle();
			barrier.size = VK_WHOLE_SIZE;
			//barrier.offset = 0;

			barrier.srcAccessMask = pBarriers[i].m_SrcAccess == ResourceAccess::kAuto ?
				GetVkAccessFlags(pBarriers[i].m_SrcLayout) : GetVkAccessFlags(pBarriers[i].m_SrcAccess);
			barrier.dstAccessMask = pBarriers[i].m_DstAccess == ResourceAccess::kAuto ?
				GetVkAccessFlags(pBarriers[i].m_DstLayout) : GetVkAccessFlags(pBarriers[i].m_DstAccess);

			barrier.srcStageMask |= pBarriers[i].m_SrcPipelineStage == PipelineStage::kAuto ?
				GetVkPipelineStageFlags(barrier.srcAccessMask) : GetVkPipelineStageFlags(pBarriers[i].m_SrcPipelineStage);
			barrier.dstStageMask |= pBarriers[i].m_DstPipelineStage == PipelineStage::kAuto ?
				GetVkPipelineStageFlags(barrier.dstAccessMask) : GetVkPipelineStageFlags(pBarriers[i].m_DstPipelineStage);

			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		}
		else if (pBarriers[i].m_pTexture)
		{
			auto& barrier = m_ImageBarriers.EmplaceBack(VkImageMemoryBarrier2{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 });
			//barrier.pNext = nullptr;
			barrier.image = static_cast<BvTextureVk *>(pBarriers[i].m_pTexture)->GetHandle();
			barrier.subresourceRange.aspectMask = GetVkFormatMap(pBarriers[i].m_pTexture->GetDesc().m_Format).aspectFlags;
			barrier.subresourceRange.baseMipLevel = pBarriers[i].m_Subresource.firstMip;
			barrier.subresourceRange.levelCount = pBarriers[i].m_Subresource.mipCount;
			barrier.subresourceRange.baseArrayLayer = pBarriers[i].m_Subresource.firstLayer;
			barrier.subresourceRange.layerCount = pBarriers[i].m_Subresource.layerCount;

			barrier.oldLayout = GetVkImageLayout(pBarriers[i].m_SrcLayout);
			barrier.newLayout = GetVkImageLayout(pBarriers[i].m_DstLayout);

			barrier.srcAccessMask = pBarriers[i].m_SrcAccess == ResourceAccess::kAuto ?
				GetVkAccessFlags(pBarriers[i].m_SrcLayout) : GetVkAccessFlags(pBarriers[i].m_SrcAccess);
			barrier.dstAccessMask = pBarriers[i].m_DstAccess == ResourceAccess::kAuto ?
				GetVkAccessFlags(pBarriers[i].m_DstLayout) : GetVkAccessFlags(pBarriers[i].m_DstAccess);

			barrier.srcStageMask |= pBarriers[i].m_SrcPipelineStage == PipelineStage::kAuto ?
				GetVkPipelineStageFlags(barrier.srcAccessMask) : GetVkPipelineStageFlags(pBarriers[i].m_SrcPipelineStage);
			barrier.dstStageMask |= pBarriers[i].m_DstPipelineStage == PipelineStage::kAuto ?
				GetVkPipelineStageFlags(barrier.dstAccessMask) : GetVkPipelineStageFlags(pBarriers[i].m_DstPipelineStage);

			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		}
		else if (pBarriers[i].m_Type == ResourceBarrierDesc::Type::kMemory)
		{
			auto& barrier = m_MemoryBarriers.EmplaceBack(VkMemoryBarrier2{ VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 });
			//barrier.pNext = nullptr;
			barrier.srcAccessMask = pBarriers[i].m_SrcAccess == ResourceAccess::kAuto ?
				GetVkAccessFlags(pBarriers[i].m_SrcLayout) : GetVkAccessFlags(pBarriers[i].m_SrcAccess);
			barrier.dstAccessMask = pBarriers[i].m_DstAccess == ResourceAccess::kAuto ?
				GetVkAccessFlags(pBarriers[i].m_DstLayout) : GetVkAccessFlags(pBarriers[i].m_DstAccess);

			barrier.srcStageMask |= pBarriers[i].m_SrcPipelineStage == PipelineStage::kAuto ?
				GetVkPipelineStageFlags(barrier.srcAccessMask) : GetVkPipelineStageFlags(pBarriers[i].m_SrcPipelineStage);
			barrier.dstStageMask |= pBarriers[i].m_DstPipelineStage == PipelineStage::kAuto ?
				GetVkPipelineStageFlags(barrier.dstAccessMask) : GetVkPipelineStageFlags(pBarriers[i].m_DstPipelineStage);
		}
	}

	ResourceBarrier(static_cast<u32>(m_BufferBarriers.Size()), m_BufferBarriers.Data(),
		static_cast<u32>(m_ImageBarriers.Size()), m_ImageBarriers.Data(),
		static_cast<u32>(m_MemoryBarriers.Size()), m_MemoryBarriers.Data());

	m_MemoryBarriers.Clear();
	m_BufferBarriers.Clear();
	m_ImageBarriers.Clear();
}


void BvCommandBufferVk::BeginQuery(BvQuery* pQuery)
{
	auto pQueryVk = TO_VK(pQuery);
	auto queryType = pQueryVk->GetQueryType();
	auto pData = pQueryVk->Allocate(m_pFrameData->GetQueryHeapManager(), 1, m_pFrameData->GetFrameIndex());
	if (queryType != QueryType::kTimestamp)
	{
		VkQueryControlFlags flags = queryType == QueryType::kOcclusion ? VK_QUERY_CONTROL_PRECISE_BIT : 0;
		vkCmdBeginQuery(m_CommandBuffer, pData->m_pQueryHeap->GetHandle(), pData->m_QueryIndex, flags);
	}

	m_pFrameData->AddQuery(pQueryVk);
}


void BvCommandBufferVk::EndQuery(BvQuery* pQuery)
{
	ResetRenderTargets();

	auto pQueryVk = TO_VK(pQuery);
	auto queryType = pQueryVk->GetQueryType();
	auto pData = pQueryVk->GetQueryData(m_pFrameData->GetFrameIndex());
	if (queryType == QueryType::kTimestamp)
	{
		vkCmdWriteTimestamp2(m_CommandBuffer, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, pData->m_pQueryHeap->GetHandle(), pData->m_QueryIndex);
	}
	else
	{
		vkCmdEndQuery(m_CommandBuffer, pData->m_pQueryHeap->GetHandle(), pData->m_QueryIndex);
	}
}


void BvCommandBufferVk::FlushDescriptorSets()
{
	auto& rbs = m_pFrameData->GetResourceBindingState();

	auto pSRL = static_cast<const BvShaderResourceLayoutVk*>(m_pGraphicsPipeline->GetDesc().m_pShaderResourceLayout);
	for (auto& resourceSet : pSRL->GetDesc().m_ShaderResources)
	{
		u32 set = resourceSet.first;
		for (auto& resource : resourceSet.second)
		{
			for (auto i = 0u; i < resource.second.m_Count; i++)
			{
				BvResourceBindingStateVk::ResourceId resId{ set, resource.first, i };
				if (auto pResourceData = rbs.GetResource(resId))
				{
					m_WriteSets.PushBack({ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET });
					auto& writeSet = m_WriteSets.Back();
					//writeSet.dstSet = nullptr; // This is set in BvDescriptorSetVk::Update()
					writeSet.descriptorType = pResourceData->m_DescriptorType;
					writeSet.dstBinding = resource.first;
					writeSet.dstArrayElement = i;
					writeSet.descriptorCount = 1;

					switch (pResourceData->m_DescriptorType)
					{
					case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
					case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
						writeSet.pBufferInfo = &pResourceData->m_Data.m_BufferInfo;
						break;
					case VK_DESCRIPTOR_TYPE_SAMPLER:
					case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
					case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
						writeSet.pImageInfo = &pResourceData->m_Data.m_ImageInfo;
						break;
					case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
					case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
						writeSet.pTexelBufferView = &pResourceData->m_Data.m_BufferView;
						break;
					}
				}
			}
		}

		auto descriptorSet = m_pFrameData->RequestDescriptorSet(set, pSRL, m_WriteSets);
		auto pipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
		if (m_pComputePipeline)
		{
			pipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE;
		}

		vkCmdBindDescriptorSets(m_CommandBuffer, pipelineBindPoint, pSRL->GetPipelineLayoutHandle(), set, 1, &descriptorSet, 0, nullptr);
		m_WriteSets.Clear();
	}
}


void BvCommandBufferVk::ResetRenderTargets()
{
	if (m_CurrentState == State::kRenderPass)
	{
		EndRenderPass();
	}
	else if (m_CurrentState == State::kRenderTarget)
	{
		vkCmdEndRendering(m_CommandBuffer);

		VkDependencyInfo dependencyInfo{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
		dependencyInfo.imageMemoryBarrierCount = static_cast<u32>(m_PostRenderBarriers.Size());
		dependencyInfo.pImageMemoryBarriers = m_PostRenderBarriers.Data();

		vkCmdPipelineBarrier2(m_CommandBuffer, &dependencyInfo);

		m_PostRenderBarriers.Clear();
	}

	m_CurrentState = State::kRecording;
}