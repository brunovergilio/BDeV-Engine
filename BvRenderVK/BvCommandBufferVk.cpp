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
#include "BvTextureVk.h"
#include "BvSwapChainVk.h"
#include "BvSemaphoreVk.h"
#include "BvBufferVk.h"


constexpr auto kMaxCopyRegions = 14u; // I'm assuming a region per mip, so 2^14 = 16384 - more than I will ever use


BvCommandBufferVk::BvCommandBufferVk(const BvRenderDeviceVk & device, BvCommandPool * pCommandPool, const VkCommandBuffer commandBuffer)
	: BvCommandBuffer(pCommandPool), m_Device(device),
	m_pCommandPool(static_cast<BvCommandPoolVk *>(pCommandPool)), m_CommandBuffer(commandBuffer)
{
}


BvCommandBufferVk::~BvCommandBufferVk()
{
	m_CommandBuffer = VK_NULL_HANDLE;
}


void BvCommandBufferVk::Reset()
{
}


void BvCommandBufferVk::Begin()
{
	m_RenderTargetTransitions.Clear();
	m_SwapChainSignalSemaphores.Clear();

	VkCommandBufferBeginInfo cmdBI{};
	cmdBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBI.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	// TODO: Deal with secondary command buffers
	m_Device.GetDeviceFunctions().vkBeginCommandBuffer(m_CommandBuffer, &cmdBI);
}


void BvCommandBufferVk::End()
{
	if (m_RenderTargetTransitions.Size() > 0)
	{
		m_Device.GetDeviceFunctions().vkCmdPipelineBarrier(m_CommandBuffer, m_RenderTargetSrcStageFlags, m_RenderTargetDstStageFlags,
			VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, m_RenderTargetTransitions.Data());
	}

	m_Device.GetDeviceFunctions().vkEndCommandBuffer(m_CommandBuffer);
}


void BvCommandBufferVk::BeginRenderPass(const BvRenderPass * const pRenderPass, BvTextureView * const * const pRenderTargets,
	const ClearColorValue * const pClearColors, BvTextureView * const pDepthStencilTarget, const ClearColorValue & depthClear)
{
	BvAssert(pRenderPass != nullptr, "Invalid render pass");
	BvAssert(pRenderTargets != nullptr || pDepthStencilTarget != nullptr, "No render / depth targets");
	
	VkExtent2D renderArea{};

	auto & renderPassDesc = pRenderPass->GetRenderTargetDesc();
	BvFixedVector<VkClearValue, kMaxRenderTargetsWithDepth> clearValues(renderPassDesc.m_RenderTargets.Size() + (renderPassDesc.m_HasDepth ? 1 : 0));
	BvFixedVector<VkImageView, kMaxRenderTargetsWithDepth> views(clearValues.Size());
	FramebufferDesc frameBufferDesc;
	u32 numIgnoredBarrierTransitions = 0;
	for (u32 i = 0; i < renderPassDesc.m_RenderTargets.Size(); i++)
	{
		auto pViewVk = static_cast<BvTextureViewVk * const>(pRenderTargets[i]);
		VkClearValue clearValue;
		auto & desc = pViewVk->GetTexture()->GetDesc();
		frameBufferDesc.m_RenderTargetViews.PushBack(pViewVk);
		if (pClearColors)
		{
			clearValue.color = { pClearColors[i].r, pClearColors[i].g, pClearColors[i].b, pClearColors[i].a };
		}
		else
		{
			clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };
		}

		if (desc.m_Size.width > renderArea.width) { renderArea.width = desc.m_Size.width; }
		if (desc.m_Size.height> renderArea.height) { renderArea.height = desc.m_Size.height; }

		clearValues[i] = clearValue;
		views[i] = pViewVk->GetHandle();

		// If any of the view objects happen to be a swap chain texture, we need to request
		// a semaphore from the swap chain
		if (pViewVk->GetTexture()->GetClassType() == BvTexture::ClassType::kSwapChainTexture)
		{
			auto pSwapChain = static_cast<BvSwapChainTextureVk *>(pViewVk->GetTexture())->GetSwapChain();
			auto pSemaphore = pSwapChain->RegisterSignalSemaphore();
			BvAssert(pSemaphore != nullptr, "Invalid semaphore handle");
			m_SwapChainSignalSemaphores.EmplaceBack(pSemaphore->GetHandle());
		}

		// Vulkan guarantees us implicit pre-transition to UNDEFINED layout as long as the image's
		// load operation is either set to DONT_CARE or CLEAR, so we take advantage of that here
		for (auto j = 0; j < m_RenderTargetTransitions.Size(); j++)
		{
			VkImage image = VK_NULL_HANDLE;
			if (pViewVk->GetTexture()->GetClassType() == BvTexture::ClassType::kTexture)
			{
				image = static_cast<BvTextureVk *>(pViewVk->GetTexture())->GetHandle();
			}
			else
			{
				image = static_cast<BvSwapChainTextureVk *>(pViewVk->GetTexture())->GetHandle();
			}

			if (m_RenderTargetTransitions[j].image == image
				&& pRenderPass->GetRenderTargetDesc().m_RenderTargets[j].m_LoadOp != LoadOp::kLoad)
			{
				std::swap(m_RenderTargetTransitions[j], m_RenderTargetTransitions[m_RenderTargetTransitions.Size() - 1 - numIgnoredBarrierTransitions++]);
				break;
			}
		}
	}

	if (renderPassDesc.m_HasDepth)
	{
		VkClearValue clearValue;
		clearValue.depthStencil = { depthClear.depth, depthClear.stencil };
		clearValues[clearValues.Size() - 1] = clearValue;
		frameBufferDesc.m_pDepthStencilView = static_cast<BvTextureViewVk * const>(pDepthStencilTarget);

		// We do the same as above, but for depth
		for (auto i = 0; i < m_RenderTargetTransitions.Size(); i++)
		{
			VkImage image = static_cast<BvTextureVk *>(frameBufferDesc.m_pDepthStencilView->GetTexture())->GetHandle();

			if (m_RenderTargetTransitions[i].image == image
				&& pRenderPass->GetRenderTargetDesc().m_RenderTargets[i].m_LoadOp != LoadOp::kLoad)
			{
				std::swap(m_RenderTargetTransitions[i], m_RenderTargetTransitions[m_RenderTargetTransitions.Size() - 1 - numIgnoredBarrierTransitions++]);
				break;
			}
		}
	}

	m_RenderTargetTransitions.Erase(m_RenderTargetTransitions.begin() + m_RenderTargetTransitions.Size() - numIgnoredBarrierTransitions,
		m_RenderTargetTransitions.end());

	auto renderPass = static_cast<const BvRenderPassVk * const>(pRenderPass)->GetHandle();
	frameBufferDesc.m_RenderPass = renderPass;

	auto pFramebuffer = g_FramebufferManager.GetFramebuffer(m_Device, frameBufferDesc);

	VkRenderPassAttachmentBeginInfo attachmentBI{};
	attachmentBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO;
	attachmentBI.attachmentCount = (u32)views.Size();
	attachmentBI.pAttachments = views.Data();

	VkRenderPassBeginInfo renderPassBI{};
	renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBI.pNext = &attachmentBI;
	renderPassBI.renderPass = renderPass;
	renderPassBI.framebuffer = pFramebuffer->GetHandle();
	//renderPassBI.renderArea.offset = { 0, 0 };

	renderPassBI.renderArea.extent = renderArea;
	renderPassBI.clearValueCount = (u32)clearValues.Size();
	renderPassBI.pClearValues = clearValues.Data();

	if (m_RenderTargetTransitions.Size() > 0)
	{
		m_Device.GetDeviceFunctions().vkCmdPipelineBarrier(m_CommandBuffer, m_RenderTargetSrcStageFlags, m_RenderTargetDstStageFlags,
			VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, m_RenderTargetTransitions.Data());
	}

	m_Device.GetDeviceFunctions().vkCmdBeginRenderPass(m_CommandBuffer, &renderPassBI, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

	m_IsInRenderPass = true;
}


void BvCommandBufferVk::EndRenderPass()
{
	m_Device.GetDeviceFunctions().vkCmdEndRenderPass(m_CommandBuffer);

	m_IsInRenderPass = false;
}


void BvCommandBufferVk::SetViewports(const u32 viewportCount, const Viewport * const pViewports)
{
	constexpr u32 kMaxViewports = 8;
	BvAssert(viewportCount <= kMaxViewports, "Viewport count greater than limit");
	BvFixedVector<VkViewport, kMaxViewports> viewports(viewportCount, {});
	for (auto i = 0u; i < viewports.Size(); i++)
	{
		viewports[i] = { pViewports[i].x, pViewports[i].y, pViewports[i].width, pViewports[i].height,
			pViewports[i].minDepth, pViewports[i].maxDepth };

		// Reverse viewport
		viewports[i].y += viewports[i].height;
		viewports[i].height = -viewports[i].height;
	}
	m_Device.GetDeviceFunctions().vkCmdSetViewport(m_CommandBuffer, 0, (u32)viewports.Size(), viewports.Data());
}


void BvCommandBufferVk::SetScissors(const u32 scissorCount, const Rect * const pScissors)
{
	constexpr u32 kMaxScissors = 8;
	BvAssert(scissorCount <= kMaxScissors, "Scissor count greater than limit");
	BvFixedVector<VkRect2D, kMaxScissors> scissors(scissorCount, {});
	for (auto i = 0u; i < scissors.Size(); i++)
	{
		scissors[i] = { pScissors[i].x, pScissors[i].y, pScissors[i].width, pScissors[i].height };
	}
	m_Device.GetDeviceFunctions().vkCmdSetScissor(m_CommandBuffer, 0, scissors.Size(), scissors.Data());
}


void BvCommandBufferVk::SetPipeline(const BvGraphicsPipelineState * const pPipeline)
{
	m_PipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
	m_PipelineLayout = static_cast<BvShaderResourceLayoutVk *>(pPipeline->GetDesc().m_pShaderResourceLayout)->GetPipelineLayoutHandle();

	auto pipeline = static_cast<const BvGraphicsPipelineStateVk * const>(pPipeline)->GetHandle();
	m_Device.GetDeviceFunctions().vkCmdBindPipeline(m_CommandBuffer, m_PipelineBindPoint, pipeline);
}


void BvCommandBufferVk::SetPipeline(const BvComputePipelineState * const pPipeline)
{
	m_PipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE;
	m_PipelineLayout = static_cast<BvShaderResourceLayoutVk *>(pPipeline->GetDesc().m_pShaderResourceLayout)->GetPipelineLayoutHandle();

	auto pipeline = static_cast<const BvComputePipelineStateVk * const>(pPipeline)->GetHandle();
	m_Device.GetDeviceFunctions().vkCmdBindPipeline(m_CommandBuffer, m_PipelineBindPoint, pipeline);
}


void BvCommandBufferVk::SetShaderResourceSets(const u32 setCount, BvShaderResourceSet * const * const ppSets, const u32 firstSet)
{
	constexpr u32 kMaxShaderResourceSets = 8;
	BvAssert(setCount <= kMaxShaderResourceSets, "Shader resource set count greater than limit");

	BvFixedVector<VkDescriptorSet, kMaxShaderResourceSets> sets(setCount);
	for (auto i = 0u; i < setCount; i++)
	{
		sets[i] = static_cast<BvShaderResourceSetVk *>(ppSets[i])->GetHandle();
	}

	m_Device.GetDeviceFunctions().vkCmdBindDescriptorSets(m_CommandBuffer, m_PipelineBindPoint, m_PipelineLayout, firstSet, setCount, sets.Data(), 0, nullptr);
}


void BvCommandBufferVk::SetVertexBuffers(const u32 vertexBufferCount, const BvBuffer * const * const pVertexBuffers,
	const u32 firstBinding)
{
	constexpr u32 kMaxVertexBuffers = 8;
	BvAssert(vertexBufferCount <= kMaxVertexBuffers, "Vertex buffer count greater than limit");

	BvFixedVector<VkBuffer, kMaxVertexBuffers> vertexBuffers(vertexBufferCount, {});
	BvFixedVector<VkDeviceSize, kMaxVertexBuffers> vertexBufferOffsets(vertexBufferCount, {});
	for (auto i = 0u; i < vertexBuffers.Size(); i++)
	{
		vertexBuffers[i] = static_cast<const BvBufferVk * const>(pVertexBuffers[i])->GetHandle();
	}
	m_Device.GetDeviceFunctions().vkCmdBindVertexBuffers(m_CommandBuffer, firstBinding, vertexBuffers.Size(), vertexBuffers.Data(), vertexBufferOffsets.Data());
}


void BvCommandBufferVk::SetIndexBuffer(const BvBuffer * const pIndexBuffer, const IndexFormat indexFormat)
{
	auto indexBuffer = static_cast<const BvBufferVk * const>(pIndexBuffer)->GetHandle();
	m_Device.GetDeviceFunctions().vkCmdBindIndexBuffer(m_CommandBuffer, indexBuffer, 0, GetVkIndexType(indexFormat));
}


void BvCommandBufferVk::Draw(const u32 vertexCount, const u32 instanceCount, const u32 firstVertex, const u32 firstInstance)
{
	m_WaitStageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	m_Device.GetDeviceFunctions().vkCmdDraw(m_CommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}


void BvCommandBufferVk::DrawIndexed(const u32 indexCount, const u32 instanceCount, const u32 firstIndex,
	const i32 vertexOffset, const u32 firstInstance)
{
	m_WaitStageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	m_Device.GetDeviceFunctions().vkCmdDrawIndexed(m_CommandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}


void BvCommandBufferVk::Dispatch(const u32 x, const u32 y, const u32 z)
{
	m_WaitStageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	m_Device.GetDeviceFunctions().vkCmdDispatch(m_CommandBuffer, x, y, z);
}


void BvCommandBufferVk::DrawIndirect(const BvBuffer * const pBuffer, const u32 drawCount, const u64 offset)
{
	m_WaitStageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	auto pBufferVk = static_cast<const BvBufferVk * const>(pBuffer)->GetHandle();
	m_Device.GetDeviceFunctions().vkCmdDrawIndirect(m_CommandBuffer, pBufferVk, offset, drawCount, (u32)sizeof(VkDrawIndexedIndirectCommand));
}


void BvCommandBufferVk::DrawIndexedIndirect(const BvBuffer * const pBuffer, const u32 drawCount, const u64 offset)
{
	m_WaitStageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	auto pBufferVk = static_cast<const BvBufferVk * const>(pBuffer)->GetHandle();
	m_Device.GetDeviceFunctions().vkCmdDrawIndexedIndirect(m_CommandBuffer, pBufferVk, offset, drawCount, (u32)sizeof(VkDrawIndirectCommand));
}


void BvCommandBufferVk::DispatchIndirect(const BvBuffer * const pBuffer, const u64 offset)
{
	m_WaitStageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	auto pBufferVk = static_cast<const BvBufferVk * const>(pBuffer)->GetHandle();
	m_Device.GetDeviceFunctions().vkCmdDispatchIndirect(m_CommandBuffer, pBufferVk, offset);
}


void BvCommandBufferVk::CopyBuffer(const BvBuffer * const pSrcBuffer, BvBuffer * const pDstBuffer)
{
	BvAssert(pDstBuffer->GetDesc().m_Size >= pSrcBuffer->GetDesc().m_Size, "Dst buffer is too small");

	CopyRegion copyRegion;
	copyRegion.buffer = { pDstBuffer->GetDesc().m_Size, 0, 0 };
	CopyBufferRegion(pSrcBuffer, pDstBuffer, copyRegion);
}


void BvCommandBufferVk::CopyBufferRegion(const BvBuffer * const pSrcBuffer, BvBuffer * const pDstBuffer, const CopyRegion & copyRegion)
{
	BvAssert(pDstBuffer->GetDesc().m_Size >= pSrcBuffer->GetDesc().m_Size, "Dst buffer is too small");

	auto pSrc = static_cast<const BvBufferVk * const>(pSrcBuffer);
	auto pDst = static_cast<BvBufferVk * const>(pDstBuffer);

	VkBufferCopy region{};
	region.dstOffset = copyRegion.buffer.dstOffset;
	region.srcOffset = copyRegion.buffer.srcOffset;
	region.size = copyRegion.buffer.srcSize;
	m_Device.GetDeviceFunctions().vkCmdCopyBuffer(m_CommandBuffer, pSrc->GetHandle(), pDst->GetHandle(), 1, &region);
}


void BvCommandBufferVk::CopyTexture(const BvTexture * const pSrcTexture, BvTexture * const pDstTexture)
{
	const auto & srcDesc = pSrcTexture->GetDesc();
	const auto & dstDesc = pDstTexture->GetDesc();

	BvAssert(srcDesc.m_ImageType == dstDesc.m_ImageType, "Image types don't match");
	BvAssert(srcDesc.m_Size.width == dstDesc.m_Size.width, "Image widths don't match");
	BvAssert(srcDesc.m_Size.height == dstDesc.m_Size.height, "Image heights don't match");
	BvAssert(srcDesc.m_Size.depthOrLayerCount == dstDesc.m_Size.depthOrLayerCount, "Image depth / layer count don't match");
	BvAssert(srcDesc.m_MipLevels >= dstDesc.m_MipLevels, "Image mip count don't match");

	CopyRegion copyRegion{};
	for (auto i = 0u; i < dstDesc.m_MipLevels; i++)
	{
		copyRegion.texture.srcMipLevel = i;
		copyRegion.texture.dstMipLevel = i;
		copyRegion.texture.size =
		{
			srcDesc.m_Size.width > 1 ? (srcDesc.m_Size.width >> i) : 1,
			srcDesc.m_Size.height > 1 ? (srcDesc.m_Size.height >> i) : 1,
			srcDesc.m_Size.depthOrLayerCount > 1 ? (srcDesc.m_Size.depthOrLayerCount >> i) : 1
		};
		CopyTextureRegion(pSrcTexture, pDstTexture, copyRegion);
	}
}


void BvCommandBufferVk::CopyTextureRegion(const BvTexture * const pSrcTexture, BvTexture * const pDstTexture, const CopyRegion & copyRegion)
{
	auto pSrc = static_cast<const BvTextureVk * const>(pSrcTexture);
	auto pDst = static_cast<BvTextureVk * const>(pDstTexture);

	VkImageCopy imageCopyRegion{};
	imageCopyRegion.extent =
	{
		copyRegion.texture.size.width,
		copyRegion.texture.size.height,
		copyRegion.texture.size.depthOrLayerCount
	};

	imageCopyRegion.srcOffset =
	{
		(i32)copyRegion.texture.srcTextureOffset.x,
		(i32)copyRegion.texture.srcTextureOffset.y,
		(i32)copyRegion.texture.srcTextureOffset.z
	};
	
	imageCopyRegion.dstOffset =
	{
		(i32)copyRegion.texture.dstTextureOffset.x,
		(i32)copyRegion.texture.dstTextureOffset.y,
		(i32)copyRegion.texture.dstTextureOffset.z
	};

	switch (pSrc->GetDesc().m_ImageType)
	{
	case TextureType::kTexture1D:
		imageCopyRegion.srcSubresource.baseArrayLayer = copyRegion.texture.srcTextureOffset.height;
		imageCopyRegion.srcSubresource.layerCount = copyRegion.texture.size.height;
		
		imageCopyRegion.dstSubresource.baseArrayLayer = copyRegion.texture.dstTextureOffset.height;
		imageCopyRegion.dstSubresource.layerCount = copyRegion.texture.size.height;
		
		break;
	case TextureType::kTexture2D:
		imageCopyRegion.srcSubresource.baseArrayLayer = copyRegion.texture.srcTextureOffset.height;
		imageCopyRegion.srcSubresource.layerCount = copyRegion.texture.size.height;
		
		imageCopyRegion.dstSubresource.baseArrayLayer = copyRegion.texture.dstTextureOffset.depthOrLayerCount;
		imageCopyRegion.dstSubresource.layerCount = copyRegion.texture.size.depthOrLayerCount;
		
		break;
	case TextureType::kTexture3D:
		imageCopyRegion.srcSubresource.baseArrayLayer = 0;
		imageCopyRegion.srcSubresource.layerCount = 1;
		
		imageCopyRegion.dstSubresource.baseArrayLayer = 0;
		imageCopyRegion.dstSubresource.layerCount = 1;

		break;
	}

	imageCopyRegion.srcSubresource.mipLevel = copyRegion.texture.srcMipLevel;
	imageCopyRegion.srcSubresource.aspectMask = GetVkAspectMaskFlags(GetVkFormat(pSrc->GetDesc().m_Format));

	imageCopyRegion.dstSubresource.mipLevel = copyRegion.texture.dstMipLevel;
	imageCopyRegion.dstSubresource.aspectMask = GetVkAspectMaskFlags(GetVkFormat(pDst->GetDesc().m_Format));

	m_Device.GetDeviceFunctions().vkCmdCopyImage(m_CommandBuffer, pSrc->GetHandle(), VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		pDst->GetHandle(), VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1u, &imageCopyRegion);
}


void BvCommandBufferVk::CopyTextureRegion(const BvBuffer * const pSrcBuffer, BvTexture * const pDstTexture, const CopyRegion & copyRegion)
{
	auto pSrc = static_cast<const BvBufferVk * const>(pSrcBuffer);
	auto pDst = static_cast<BvTextureVk * const>(pDstTexture);

	VkBufferImageCopy bufferImageCopyRegion{};
	bufferImageCopyRegion.bufferOffset = copyRegion.bufferTexture.bufferOffset;
	//bufferImageCopyRegion.bufferRowLength = 0;
	//bufferImageCopyRegion.bufferImageHeight = 0;
	bufferImageCopyRegion.imageOffset =
	{
		(i32)copyRegion.bufferTexture.textureOffset.x,
		(i32)copyRegion.bufferTexture.textureOffset.y,
		(i32)copyRegion.bufferTexture.textureOffset.z
	};
	bufferImageCopyRegion.imageExtent =
	{
		copyRegion.bufferTexture.textureSize.width,
		copyRegion.bufferTexture.textureSize.height,
		copyRegion.bufferTexture.textureSize.depthOrLayerCount
	};
	bufferImageCopyRegion.imageSubresource.aspectMask = GetVkAspectMaskFlags(GetVkFormat(pDst->GetDesc().m_Format));;
	bufferImageCopyRegion.imageSubresource.mipLevel = copyRegion.bufferTexture.mipLevel;

	switch (pDst->GetDesc().m_ImageType)
	{
	case TextureType::kTexture1D:
		bufferImageCopyRegion.imageSubresource.baseArrayLayer = copyRegion.texture.srcTextureOffset.height;
		bufferImageCopyRegion.imageSubresource.layerCount = copyRegion.texture.size.height;
		
		bufferImageCopyRegion.imageSubresource.baseArrayLayer = copyRegion.texture.dstTextureOffset.height;
		bufferImageCopyRegion.imageSubresource.layerCount = copyRegion.texture.size.height;

		break;
	case TextureType::kTexture2D:
		bufferImageCopyRegion.imageSubresource.baseArrayLayer = copyRegion.texture.srcTextureOffset.height;
		bufferImageCopyRegion.imageSubresource.layerCount = copyRegion.texture.size.height;

		bufferImageCopyRegion.imageSubresource.baseArrayLayer = copyRegion.texture.dstTextureOffset.depthOrLayerCount;
		bufferImageCopyRegion.imageSubresource.layerCount = copyRegion.texture.size.depthOrLayerCount;

		break;
	case TextureType::kTexture3D:
		bufferImageCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferImageCopyRegion.imageSubresource.layerCount = 1;

		bufferImageCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferImageCopyRegion.imageSubresource.layerCount = 1;

		break;
	}

	m_Device.GetDeviceFunctions().vkCmdCopyBufferToImage(m_CommandBuffer, pSrc->GetHandle(), pDst->GetHandle(),
		VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopyRegion);
}


void BvCommandBufferVk::CopyTextureRegion(const BvTexture * const pSrcTexture, BvBuffer * const pDstBuffer, const CopyRegion & copyRegion)
{
	auto pSrc = static_cast<const BvTextureVk * const>(pSrcTexture);
	auto pDst = static_cast<BvBufferVk * const>(pDstBuffer);

	VkBufferImageCopy bufferImageCopyRegion{};
	bufferImageCopyRegion.bufferOffset = copyRegion.bufferTexture.bufferOffset;
	//bufferImageCopyRegion.bufferRowLength = 0;
	//bufferImageCopyRegion.bufferImageHeight = 0;
	bufferImageCopyRegion.imageOffset =
	{
		(i32)copyRegion.bufferTexture.textureOffset.x,
		(i32)copyRegion.bufferTexture.textureOffset.y,
		(i32)copyRegion.bufferTexture.textureOffset.z
	};
	bufferImageCopyRegion.imageExtent =
	{
		copyRegion.bufferTexture.textureSize.width,
		copyRegion.bufferTexture.textureSize.height,
		copyRegion.bufferTexture.textureSize.depthOrLayerCount
	};
	bufferImageCopyRegion.imageSubresource.aspectMask = GetVkAspectMaskFlags(GetVkFormat(pSrc->GetDesc().m_Format));;
	bufferImageCopyRegion.imageSubresource.mipLevel = copyRegion.bufferTexture.mipLevel;

	switch (pSrc->GetDesc().m_ImageType)
	{
	case TextureType::kTexture1D:
		bufferImageCopyRegion.imageSubresource.baseArrayLayer = copyRegion.texture.srcTextureOffset.height;
		bufferImageCopyRegion.imageSubresource.layerCount = copyRegion.texture.size.height;

		bufferImageCopyRegion.imageSubresource.baseArrayLayer = copyRegion.texture.dstTextureOffset.height;
		bufferImageCopyRegion.imageSubresource.layerCount = copyRegion.texture.size.height;

		break;
	case TextureType::kTexture2D:
		bufferImageCopyRegion.imageSubresource.baseArrayLayer = copyRegion.texture.srcTextureOffset.height;
		bufferImageCopyRegion.imageSubresource.layerCount = copyRegion.texture.size.height;

		bufferImageCopyRegion.imageSubresource.baseArrayLayer = copyRegion.texture.dstTextureOffset.depthOrLayerCount;
		bufferImageCopyRegion.imageSubresource.layerCount = copyRegion.texture.size.depthOrLayerCount;

		break;
	case TextureType::kTexture3D:
		bufferImageCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferImageCopyRegion.imageSubresource.layerCount = 1;

		bufferImageCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferImageCopyRegion.imageSubresource.layerCount = 1;

		break;
	}

	m_Device.GetDeviceFunctions().vkCmdCopyImageToBuffer(m_CommandBuffer, pSrc->GetHandle(), VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		pDst->GetHandle(), 1, &bufferImageCopyRegion);
}


void BvCommandBufferVk::ResourceBarrier(const u32 barrierCount, const ResourceBarrierDesc * const pBarriers)
{
	VkPipelineStageFlags srcStageFlags = 0, dstStageFlags = 0;
	for (auto i = 0u; i < barrierCount; i++)
	{
		if (pBarriers[i].pBuffer)
		{
			m_BufferBarriers.PushBack({});
			VkBufferMemoryBarrier& barrier = m_BufferBarriers.Back();
			barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			//barrier.pNext = nullptr;
			barrier.buffer = static_cast<BvBufferVk *>(pBarriers[i].pBuffer)->GetHandle();
			barrier.size = VK_WHOLE_SIZE;
			//barrier.offset = 0;

			barrier.srcAccessMask = pBarriers[i].srcAccess == ResourceAccess::kAuto ?
				GetVkAccessFlags(pBarriers[i].srcLayout) : GetVkAccessFlags(pBarriers[i].srcAccess);
			barrier.dstAccessMask = pBarriers[i].dstAccess == ResourceAccess::kAuto ?
				GetVkAccessFlags(pBarriers[i].dstLayout) : GetVkAccessFlags(pBarriers[i].dstAccess);

			auto pSrcQueue = static_cast<BvCommandQueueVk *>(pBarriers[i].pSrcQueue);
			auto pDstQueue = static_cast<BvCommandQueueVk *>(pBarriers[i].pSrcQueue);

			if (!pSrcQueue || !pDstQueue)
			{
				barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			}
			else
			{
				barrier.srcQueueFamilyIndex = pSrcQueue->GetFamilyIndex();
				barrier.dstQueueFamilyIndex = pDstQueue->GetFamilyIndex();
			}

			srcStageFlags |= pBarriers[i].srcPipelineStage == PipelineStage::kAuto ?
				GetVkPipelineStageFlags(barrier.srcAccessMask) : GetVkPipelineStageFlags(pBarriers[i].srcPipelineStage);
			dstStageFlags |= pBarriers[i].dstPipelineStage == PipelineStage::kAuto ?
				GetVkPipelineStageFlags(barrier.dstAccessMask) : GetVkPipelineStageFlags(pBarriers[i].dstPipelineStage);
		}
		else if (pBarriers[i].pTexture)
		{
			m_ImageBarriers.PushBack({});
			VkImageMemoryBarrier& barrier = m_ImageBarriers.Back();
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			//barrier.pNext = nullptr;
			barrier.image = static_cast<BvTextureVk *>(pBarriers[i].pTexture)->GetHandle();
			barrier.subresourceRange.aspectMask = GetVkAspectMaskFlags(GetVkFormat(pBarriers[i].pTexture->GetDesc().m_Format));
			barrier.subresourceRange.baseMipLevel = pBarriers[i].subresource.firstMip;
			barrier.subresourceRange.levelCount = pBarriers[i].subresource.mipCount;
			barrier.subresourceRange.baseArrayLayer = pBarriers[i].subresource.firstLayer;
			barrier.subresourceRange.layerCount = pBarriers[i].subresource.layerCount;

			barrier.oldLayout = GetVkImageLayout(pBarriers[i].srcLayout);
			barrier.newLayout = GetVkImageLayout(pBarriers[i].dstLayout);

			barrier.srcAccessMask = pBarriers[i].srcAccess == ResourceAccess::kAuto ?
				GetVkAccessFlags(pBarriers[i].srcLayout) : GetVkAccessFlags(pBarriers[i].srcAccess);
			barrier.dstAccessMask = pBarriers[i].dstAccess == ResourceAccess::kAuto ?
				GetVkAccessFlags(pBarriers[i].dstLayout) : GetVkAccessFlags(pBarriers[i].dstAccess);

			auto pSrcQueue = static_cast<BvCommandQueueVk *>(pBarriers[i].pSrcQueue);
			auto pDstQueue = static_cast<BvCommandQueueVk *>(pBarriers[i].pSrcQueue);

			if (!pSrcQueue || !pDstQueue)
			{
				barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			}
			else
			{
				barrier.srcQueueFamilyIndex = pSrcQueue->GetFamilyIndex();
				barrier.dstQueueFamilyIndex = pDstQueue->GetFamilyIndex();
			}

			// If we're outside of a render pass and we're trying to transition to render target or depth target,
			// then I'm assuming these are objects used in the render pass, and if so, they can be filtered out
			// before the render pass
			if (!m_IsInRenderPass &&
				(barrier.newLayout == VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				|| barrier.newLayout == VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				|| barrier.newLayout == VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL))
			{
				m_RenderTargetTransitions.PushBack(barrier);

				m_RenderTargetSrcStageFlags |= pBarriers[i].srcPipelineStage == PipelineStage::kAuto ?
					GetVkPipelineStageFlags(barrier.srcAccessMask) : GetVkPipelineStageFlags(pBarriers[i].srcPipelineStage);
				m_RenderTargetDstStageFlags |= pBarriers[i].dstPipelineStage == PipelineStage::kAuto ?
					GetVkPipelineStageFlags(barrier.dstAccessMask) : GetVkPipelineStageFlags(pBarriers[i].dstPipelineStage);
			}
			else
			{
				srcStageFlags |= pBarriers[i].srcPipelineStage == PipelineStage::kAuto ?
					GetVkPipelineStageFlags(barrier.srcAccessMask) : GetVkPipelineStageFlags(pBarriers[i].srcPipelineStage);
				dstStageFlags |= pBarriers[i].dstPipelineStage == PipelineStage::kAuto ?
					GetVkPipelineStageFlags(barrier.dstAccessMask) : GetVkPipelineStageFlags(pBarriers[i].dstPipelineStage);
			}
		}
		else
		{
			m_MemoryBarriers.PushBack({});
			VkMemoryBarrier& barrier = m_MemoryBarriers.Back();
			barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			//barrier.pNext = nullptr;
			barrier.srcAccessMask = pBarriers[i].srcAccess == ResourceAccess::kAuto ?
				GetVkAccessFlags(pBarriers[i].srcLayout) : GetVkAccessFlags(pBarriers[i].srcAccess);
			barrier.dstAccessMask = pBarriers[i].dstAccess == ResourceAccess::kAuto ?
				GetVkAccessFlags(pBarriers[i].dstLayout) : GetVkAccessFlags(pBarriers[i].dstAccess);

			srcStageFlags |= pBarriers[i].srcPipelineStage == PipelineStage::kAuto ?
				GetVkPipelineStageFlags(barrier.srcAccessMask) : GetVkPipelineStageFlags(pBarriers[i].srcPipelineStage);
			dstStageFlags |= pBarriers[i].dstPipelineStage == PipelineStage::kAuto ?
				GetVkPipelineStageFlags(barrier.dstAccessMask) : GetVkPipelineStageFlags(pBarriers[i].dstPipelineStage);
		}
	}

	auto total = m_MemoryBarriers.Size() + m_BufferBarriers.Size() + m_ImageBarriers.Size();
	if (total > 0)
	{
		m_Device.GetDeviceFunctions().vkCmdPipelineBarrier(m_CommandBuffer, srcStageFlags, dstStageFlags,
			VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT,
			(u32)m_MemoryBarriers.Size(), m_MemoryBarriers.Size() > 0 ? m_MemoryBarriers.Data() : nullptr,
			(u32)m_BufferBarriers.Size(), m_BufferBarriers.Size() > 0 ? m_BufferBarriers.Data() : nullptr,
			(u32)m_ImageBarriers.Size(), m_ImageBarriers.Size() > 0 ? m_ImageBarriers.Data() : nullptr);
	}

	m_MemoryBarriers.Clear();
	m_BufferBarriers.Clear();
	m_ImageBarriers.Clear();
}