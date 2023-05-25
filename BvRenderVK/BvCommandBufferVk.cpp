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
#include "BvBufferViewVk.h"


constexpr auto kMaxCopyRegions = 14u; // I'm assuming a region per mip, so 2^14 = 16384 - more than I will ever use


BvCommandBufferVk::BvCommandBufferVk(const BvRenderDeviceVk & device, BvCommandPool * pCommandPool, const VkCommandBuffer commandBuffer)
	: BvCommandBuffer(pCommandPool), m_Device(device), m_CommandBuffer(commandBuffer)
{
}


BvCommandBufferVk::~BvCommandBufferVk()
{
	m_CommandBuffer = VK_NULL_HANDLE;
}


void BvCommandBufferVk::Reset()
{
	m_WaitStageFlags = 0;
	m_SwapChains.Clear();
	m_CurrentState = CurrentState::kReset;
	m_RenderTargetTransitions.Clear();
	m_RenderTargetSrcStageFlags = 0;
	m_RenderTargetDstStageFlags = 0;

	ClearStateData();
}


void BvCommandBufferVk::Begin()
{
	VkCommandBufferBeginInfo cmdBI{};
	cmdBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBI.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	// TODO: Deal with secondary command buffers
	vkBeginCommandBuffer(m_CommandBuffer, &cmdBI);

	m_CurrentState = CurrentState::kRecording;
}


void BvCommandBufferVk::End()
{
	DecommitRenderTargets();

	if (m_RenderTargetTransitions.Size() > 0)
	{
		vkCmdPipelineBarrier(m_CommandBuffer, m_RenderTargetSrcStageFlags, m_RenderTargetDstStageFlags,
			VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, m_RenderTargetTransitions.Data());
		m_RenderTargetTransitions.Clear();
	}

	vkEndCommandBuffer(m_CommandBuffer);

	m_CurrentState = CurrentState::kRecorded;
}


void BvCommandBufferVk::BeginRenderPass(const BvRenderPass * const pRenderPass, BvTextureView * const * const pRenderTargets,
	const ClearColorValue * const pClearColors, BvTextureView * const pDepthStencilTarget, const ClearColorValue & depthClear)
{
	BvAssert(pRenderPass != nullptr, "Invalid render pass");
	BvAssert(pRenderTargets != nullptr || pDepthStencilTarget != nullptr, "No render / depth targets");
	
	DecommitRenderTargets();

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
			m_SwapChains.EmplaceBack(pSwapChain);
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

	auto pFramebuffer = m_Device.GetFramebufferManager()->GetFramebuffer(m_Device, frameBufferDesc);

	VkRenderPassBeginInfo renderPassBI{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	renderPassBI.renderPass = renderPass;
	renderPassBI.framebuffer = pFramebuffer->GetHandle();
	renderPassBI.renderArea.extent = renderArea;
	renderPassBI.clearValueCount = (u32)clearValues.Size();
	renderPassBI.pClearValues = clearValues.Data();

	if (m_RenderTargetTransitions.Size() > 0)
	{
		vkCmdPipelineBarrier(m_CommandBuffer, m_RenderTargetSrcStageFlags, m_RenderTargetDstStageFlags,
			VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, m_RenderTargetTransitions.Data());
		m_RenderTargetTransitions.Clear();
	}

	vkCmdBeginRenderPass(m_CommandBuffer, &renderPassBI, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

	m_CurrentState = CurrentState::kInRenderPass;
}


void BvCommandBufferVk::EndRenderPass()
{
	vkCmdEndRenderPass(m_CommandBuffer);

	m_CurrentState = CurrentState::kRecording;
}


void BvCommandBufferVk::SetRenderTargets(const u32 renderTargetCount, BvTextureView* const* const pRenderTargets,
	const ClearColorValue* const pClearColors, BvTextureView* const pDepthStencilTarget,
	const ClearColorValue& depthClear, const ClearFlags clearFlags)
{
	DecommitRenderTargets();

	if (!renderTargetCount && !pDepthStencilTarget)
	{
		return;
	}

	m_RenderTargets.Resize(renderTargetCount);
	m_RenderTargetClearValues.Resize(renderTargetCount);
	m_RenderTargetLoadOps.Resize(renderTargetCount);
	for (auto i = 0; i < m_RenderTargets.Size(); i++)
	{
		m_RenderTargets[i] = reinterpret_cast<BvTextureViewVk*>(pRenderTargets[i]);
		if (pClearColors)
		{
			m_RenderTargetClearValues[i] = *((VkClearValue*)(&pClearColors[i]));
			m_RenderTargetLoadOps[i] = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		}
	}

	m_pDepthStencilTarget = nullptr;
	if (pDepthStencilTarget)
	{
		m_pDepthStencilTarget = reinterpret_cast<BvTextureViewVk*>(pDepthStencilTarget);
		m_DepthStencilTargetClearValue = *((VkClearValue*)(&depthClear));
		m_DepthTargetLoadOp = (clearFlags & ClearFlags::kClearDepth) == ClearFlags::kClearDepth ?
			VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR : VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
		m_StencilTargetLoadOp = (clearFlags & ClearFlags::kClearStencil) == ClearFlags::kClearStencil ?
			VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR : VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
	}

	m_RenderTargetsBindNeeded = true;
}


void BvCommandBufferVk::ClearRenderTargets(u32 renderTargetCount, const ClearColorValue* const pClearValues, u32 firstRenderTargetIndex)
{
	BvAssert(firstRenderTargetIndex < m_RenderTargets.Size(), "Render Target index out of bounds!");

	auto endIndex = std::min(firstRenderTargetIndex + renderTargetCount, renderTargetCount);
	for (auto i = 0; i < endIndex; i++)
	{
		m_RenderTargetClearValues[i + firstRenderTargetIndex] = *((VkClearValue*)(&pClearValues[i]));
		m_RenderTargetLoadOps[i + firstRenderTargetIndex] = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
	}
}


void BvCommandBufferVk::SetViewports(const u32 viewportCount, const Viewport * const pViewports)
{
	constexpr u32 kMaxViewports = 8;
	BvAssert(viewportCount <= kMaxViewports, "Viewport count greater than limit");
	
	m_Viewports.Resize(viewportCount);
	for (auto i = 0; i < viewportCount; i++)
	{
		m_Viewports[i] = *((VkViewport*)(&pViewports[i]));
	}

	if (m_Device.GetGPUInfo().m_FeaturesSupported.maintenance1)
	{
		for (auto i = 0; i < viewportCount; i++)
		{
			m_Viewports[i].y += m_Viewports[i].height;
			m_Viewports[i].height = -m_Viewports[i].height;
		}
	}

	m_ViewportsBindNeeded = true;
}


void BvCommandBufferVk::SetScissors(const u32 scissorCount, const Rect * const pScissors)
{
	constexpr u32 kMaxScissors = 8;
	BvAssert(scissorCount <= kMaxScissors, "Scissor count greater than limit");

	m_Scissors.Resize(scissorCount);
	for (auto i = 0; i < scissorCount; i++)
	{
		m_Scissors[i] = *((VkRect2D*)(&pScissors[i]));
	}

	m_ScissorsBindNeeded = true;
}


void BvCommandBufferVk::SetPipeline(const BvGraphicsPipelineState * const pPipeline)
{
	auto pPipelineVk = static_cast<const BvGraphicsPipelineStateVk * const>(pPipeline);
	m_pGraphicsPSO = pPipelineVk;
	m_GraphicsPSOBindNeeded = true;
}


void BvCommandBufferVk::SetPipeline(const BvComputePipelineState * const pPipeline)
{
	auto pPipelineVk = static_cast<const BvComputePipelineStateVk * const>(pPipeline);
	m_pComputePSO = pPipelineVk;
	m_ComputePSOBindNeeded = true;
}


void BvCommandBufferVk::SetShaderResourceParams(const u32 setCount, BvShaderResourceParams * const * const ppSets, const u32 firstSet)
{
	m_FirstDescriptorSet = firstSet;
	m_DescriptorSets.Resize(setCount);
	for (auto i = 0u; i < setCount; i++)
	{
		auto pSetVk = static_cast<BvShaderResourceParamsVk*>(ppSets[i]);
		m_DescriptorSets[i] = pSetVk->GetHandle();
	}

	m_DescriptorSetBindNeeded = true;
}


void BvCommandBufferVk::SetVertexBufferViews(const u32 vertexBufferCount, const BvBufferView * const * const pVertexBufferViews,
	const u32 firstBinding)
{
	m_FirstVertexBinding = firstBinding;
	m_VertexBuffers.Resize(vertexBufferCount);
	m_VertexBufferOffsets.Resize(vertexBufferCount);
	for (auto i = 0u; i < vertexBufferCount; i++)
	{
		m_VertexBuffers[i] = static_cast<const BvBufferVk * const>(pVertexBufferViews[i]->GetBuffer())->GetHandle();
		m_VertexBufferOffsets[i] = pVertexBufferViews[i]->GetDesc().m_Offset;
	}

	m_VertexBuffersBindNeeded = true;
}


void BvCommandBufferVk::SetIndexBufferView(const BvBufferView * const pIndexBufferView, const IndexFormat indexFormat)
{
	m_pIndexBufferView = static_cast<const BvBufferViewVk* const>(pIndexBufferView);
	m_IndexFormat = GetVkIndexType(indexFormat);
	
	m_IndexBufferBindNeeded = true;
}


void BvCommandBufferVk::Draw(const u32 vertexCount, const u32 instanceCount, const u32 firstVertex, const u32 firstInstance)
{
	CommitGraphicsData();

	m_WaitStageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	vkCmdDraw(m_CommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}


void BvCommandBufferVk::DrawIndexed(const u32 indexCount, const u32 instanceCount, const u32 firstIndex,
	const i32 vertexOffset, const u32 firstInstance)
{
	CommitGraphicsData();

	m_WaitStageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	vkCmdDrawIndexed(m_CommandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}


void BvCommandBufferVk::Dispatch(const u32 x, const u32 y, const u32 z)
{
	DecommitRenderTargets();
	CommitComputeData();

	m_WaitStageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	vkCmdDispatch(m_CommandBuffer, x, y, z);
}


void BvCommandBufferVk::DrawIndirect(const BvBuffer * const pBuffer, const u32 drawCount, const u64 offset)
{
	CommitGraphicsData();

	m_WaitStageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	auto pBufferVk = static_cast<const BvBufferVk * const>(pBuffer)->GetHandle();
	vkCmdDrawIndirect(m_CommandBuffer, pBufferVk, offset, drawCount, (u32)sizeof(VkDrawIndirectCommand));
}


void BvCommandBufferVk::DrawIndexedIndirect(const BvBuffer * const pBuffer, const u32 drawCount, const u64 offset)
{
	CommitGraphicsData();

	m_WaitStageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	auto pBufferVk = static_cast<const BvBufferVk * const>(pBuffer)->GetHandle();
	vkCmdDrawIndexedIndirect(m_CommandBuffer, pBufferVk, offset, drawCount, (u32)sizeof(VkDrawIndexedIndirectCommand));
}


void BvCommandBufferVk::DispatchIndirect(const BvBuffer * const pBuffer, const u64 offset)
{
	DecommitRenderTargets();
	CommitComputeData();

	m_WaitStageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	auto pBufferVk = static_cast<const BvBufferVk * const>(pBuffer)->GetHandle();
	vkCmdDispatchIndirect(m_CommandBuffer, pBufferVk, offset);
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
	vkCmdCopyBuffer(m_CommandBuffer, pSrc->GetHandle(), pDst->GetHandle(), 1, &region);
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

	vkCmdCopyImage(m_CommandBuffer, pSrc->GetHandle(), VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
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

	vkCmdCopyBufferToImage(m_CommandBuffer, pSrc->GetHandle(), pDst->GetHandle(),
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

	vkCmdCopyImageToBuffer(m_CommandBuffer, pSrc->GetHandle(), VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		pDst->GetHandle(), 1, &bufferImageCopyRegion);
}


void BvCommandBufferVk::ResourceBarrier(const u32 barrierCount, const ResourceBarrierDesc * const pBarriers)
{
	DecommitRenderTargets();

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
			// before the render pass, as long as the load operation is not set to LOAD
			if (m_CurrentState == CurrentState::kRecording &&
				(barrier.newLayout == VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				|| barrier.newLayout == VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				|| barrier.newLayout == VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL))
			{
				m_RenderTargetTransitions.PushBack(barrier);
				m_ImageBarriers.PopBack();

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
		vkCmdPipelineBarrier(m_CommandBuffer, srcStageFlags, dstStageFlags,
			VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT,
			(u32)m_MemoryBarriers.Size(), m_MemoryBarriers.Size() > 0 ? m_MemoryBarriers.Data() : nullptr,
			(u32)m_BufferBarriers.Size(), m_BufferBarriers.Size() > 0 ? m_BufferBarriers.Data() : nullptr,
			(u32)m_ImageBarriers.Size(), m_ImageBarriers.Size() > 0 ? m_ImageBarriers.Data() : nullptr);
	}

	m_MemoryBarriers.Clear();
	m_BufferBarriers.Clear();
	m_ImageBarriers.Clear();
}


void BvCommandBufferVk::CommitGraphicsData()
{
	if (m_RenderTargetsBindNeeded)
	{
		CommitRenderTargets();
		m_RenderTargetsBindNeeded = false;
	}

	BvShaderResourceLayoutVk* pLayout = nullptr;
	if (m_GraphicsPSOBindNeeded)
	{
		pLayout = static_cast<BvShaderResourceLayoutVk*>(m_pGraphicsPSO->GetDesc().m_pShaderResourceLayout);
		vkCmdBindPipeline(m_CommandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_pGraphicsPSO->GetHandle());
		m_GraphicsPSOBindNeeded = false;
	}

	if (m_ViewportsBindNeeded)
	{
		vkCmdSetViewport(m_CommandBuffer, 0, m_Viewports.Size(), m_Viewports.Data());
		m_ViewportsBindNeeded = false;
	}

	if (m_ScissorsBindNeeded)
	{
		vkCmdSetScissor(m_CommandBuffer, 0, m_Scissors.Size(), m_Scissors.Data());
		m_ScissorsBindNeeded = false;
	}

	if (m_VertexBuffersBindNeeded)
	{
		vkCmdBindVertexBuffers(m_CommandBuffer, m_FirstVertexBinding, m_VertexBuffers.Size(), m_VertexBuffers.Data(), m_VertexBufferOffsets.Data());

		m_VertexBuffersBindNeeded = false;
	}

	if (m_IndexBufferBindNeeded)
	{
		auto offset = m_pIndexBufferView->GetDesc().m_Offset;
		vkCmdBindIndexBuffer(m_CommandBuffer, static_cast<BvBufferVk*>(m_pIndexBufferView->GetBuffer())->GetHandle(), offset, m_IndexFormat);

		m_IndexBufferBindNeeded = false;
	}

	if (m_DescriptorSetBindNeeded)
	{
		vkCmdBindDescriptorSets(m_CommandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pLayout->GetPipelineLayoutHandle(), m_FirstDescriptorSet,
			m_DescriptorSets.Size(), m_DescriptorSets.Data(), 0, nullptr);
		m_DescriptorSetBindNeeded = false;
	}
}


void BvCommandBufferVk::CommitComputeData()
{
	BvShaderResourceLayoutVk* pLayout = nullptr;
	if (m_ComputePSOBindNeeded)
	{
		pLayout = static_cast<BvShaderResourceLayoutVk*>(m_pComputePSO->GetDesc().m_pShaderResourceLayout);
		vkCmdBindPipeline(m_CommandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE, m_pComputePSO->GetHandle());
		m_ComputePSOBindNeeded = false;
	}

	if (m_DescriptorSetBindNeeded)
	{
		vkCmdBindDescriptorSets(m_CommandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE, pLayout->GetPipelineLayoutHandle(), m_FirstDescriptorSet,
			m_DescriptorSets.Size(), m_DescriptorSets.Data(), 0, nullptr);
		m_DescriptorSetBindNeeded = false;
	}
}


void BvCommandBufferVk::CommitRenderTargets()
{
	if (m_Device.GetGPUInfo().m_FeaturesSupported.dynamicRendering)
	{
		VkExtent2D renderArea{};

		VkRenderingAttachmentInfoKHR colorAttachments[kMaxRenderTargets]{};
		VkRenderingAttachmentInfoKHR depthAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR };
		VkRenderingAttachmentInfoKHR stencilAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR };
		u32 numIgnoredBarrierTransitions = 0;
		u32 layerCount = 0;
		for (u32 i = 0; i < m_RenderTargets.Size(); i++)
		{
			layerCount = std::max(m_RenderTargets[i]->GetDesc().m_SubresourceDesc.layerCount, layerCount);
			decltype(auto) desc = m_RenderTargets[i]->GetTexture()->GetDesc();

			if (desc.m_Size.width > renderArea.width) { renderArea.width = desc.m_Size.width; }
			if (desc.m_Size.height > renderArea.height) { renderArea.height = desc.m_Size.height; }

			colorAttachments[i].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
			colorAttachments[i].imageView = m_RenderTargets[i]->GetHandle();
			colorAttachments[i].imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
			colorAttachments[i].loadOp = m_RenderTargetLoadOps[i];
			colorAttachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			if (m_RenderTargetLoadOps[i] == VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR)
			{
				colorAttachments[i].clearValue.color = m_RenderTargetClearValues[i].color;
			}

			// If any of the view objects happen to be a swap chain texture, we need to request
			// a semaphore from the swap chain
			if (m_RenderTargets[i]->GetTexture()->GetClassType() == BvTexture::ClassType::kSwapChainTexture)
			{
				auto pSwapChain = static_cast<BvSwapChainTextureVk*>(m_RenderTargets[i]->GetTexture())->GetSwapChain();
				m_SwapChains.EmplaceBack(pSwapChain);
			}

			// Vulkan doesn't perform an implicit pre-transition to UNDEFINED layout if we use dynamic rendering.
			// What we do instead is check if the target's old layout is set to present mode, and if it is, then
			// we change it to Undefined, along with the src access mask and the src stage flags
			for (auto j = 0; j < m_RenderTargetTransitions.Size(); j++)
			{
				// For Dynamic Rendering, if our previous layout happens to be a swap chain presentation,
				// we use undefined instead
				if (m_RenderTargetTransitions[i].oldLayout == VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
				{
					// Presentation layout will instead become undefined
					m_RenderTargetTransitions[i].oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
					// There will be no access flags
					m_RenderTargetTransitions[i].srcAccessMask = 0;
					// And the stage is top of pipe, not bottom
					m_RenderTargetSrcStageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
					m_RenderTargetSrcStageFlags &= ~VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
				}
			}
		}

		bool useSameDepthStencilAttachment = m_DepthTargetLoadOp == m_StencilTargetLoadOp;

		if (m_pDepthStencilTarget)
		{
			layerCount = std::max(m_pDepthStencilTarget->GetDesc().m_SubresourceDesc.layerCount, layerCount);

			if (useSameDepthStencilAttachment)
			{
				depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
				depthAttachment.imageView = m_pDepthStencilTarget->GetHandle();
				depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				depthAttachment.loadOp = m_DepthTargetLoadOp;
				depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				if (m_DepthTargetLoadOp == VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR)
				{
					depthAttachment.clearValue.depthStencil = m_DepthStencilTargetClearValue.depthStencil;
				}
			}
			else
			{
				depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
				depthAttachment.imageView = m_pDepthStencilTarget->GetHandle();
				depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				depthAttachment.loadOp = m_DepthTargetLoadOp;
				depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				if (m_DepthTargetLoadOp == VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR)
				{
					depthAttachment.clearValue.depthStencil = m_DepthStencilTargetClearValue.depthStencil;
				}

				stencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
				stencilAttachment.imageView = m_pDepthStencilTarget->GetHandle();
				stencilAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				stencilAttachment.loadOp = m_StencilTargetLoadOp;
				stencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				if (m_StencilTargetLoadOp == VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR)
				{
					stencilAttachment.clearValue.depthStencil = m_DepthStencilTargetClearValue.depthStencil;
				}
			}

			// We do the same as above, but for depth
			for (auto i = 0; i < m_RenderTargetTransitions.Size(); i++)
			{
				VkImage image = static_cast<BvTextureVk*>(m_pDepthStencilTarget->GetTexture())->GetHandle();

				if (m_RenderTargetTransitions[i].image == image)
				{
					std::swap(m_RenderTargetTransitions[i], m_RenderTargetTransitions[m_RenderTargetTransitions.Size() - 1 - numIgnoredBarrierTransitions++]);
					break;
				}
			}
		}

		m_RenderTargetTransitions.Erase(m_RenderTargetTransitions.begin() + m_RenderTargetTransitions.Size() - numIgnoredBarrierTransitions,
			m_RenderTargetTransitions.end());

		VkRenderingInfoKHR renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		renderingInfo.renderArea = { 0, 0, renderArea.width, renderArea.height };
		renderingInfo.layerCount = layerCount;
		renderingInfo.colorAttachmentCount = m_RenderTargets.Size();
		renderingInfo.pColorAttachments = colorAttachments;
		if (m_pDepthStencilTarget)
		{
			renderingInfo.pDepthAttachment = &depthAttachment;
			renderingInfo.pStencilAttachment = useSameDepthStencilAttachment ? &depthAttachment : &stencilAttachment;
		}

		if (m_RenderTargetTransitions.Size() > 0)
		{
			vkCmdPipelineBarrier(m_CommandBuffer, m_RenderTargetSrcStageFlags, m_RenderTargetDstStageFlags,
				VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, m_RenderTargetTransitions.Data());
			m_RenderTargetTransitions.Clear();
		}

		vkCmdBeginRenderingKHR(m_CommandBuffer, &renderingInfo);

		m_CurrentState = CurrentState::kInRender;
	}
	else
	{
		RenderPassDesc rpd;
		rpd.m_RenderTargets.Resize(m_RenderTargets.Size());
		for (auto i = 0; i < rpd.m_RenderTargets.Size(); i++)
		{
			rpd.m_RenderTargets[i].m_Format = m_RenderTargets[i]->GetDesc().m_Format;
			rpd.m_RenderTargets[i].m_SampleCount = m_pGraphicsPSO->GetDesc().m_MultisampleStateDesc.m_SampleCount;
			rpd.m_RenderTargets[i].m_LoadOp = m_RenderTargetLoadOps[i] == VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR ? LoadOp::kClear : LoadOp::kLoad;
		}
		if (m_pDepthStencilTarget)
		{
			rpd.m_HasDepth = true;
			rpd.m_DepthStencilTarget.m_Format = m_pDepthStencilTarget->GetDesc().m_Format;
			rpd.m_DepthStencilTarget.m_SampleCount = m_pGraphicsPSO->GetDesc().m_MultisampleStateDesc.m_SampleCount;
			rpd.m_DepthStencilTarget.m_LoadOp = m_DepthTargetLoadOp == VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR ? LoadOp::kClear : LoadOp::kLoad;
		}

		auto pRenderPass = m_Device.GetRenderPassManager()->GetRenderPass(m_Device, rpd);
		BvTextureView* rtvs[kMaxRenderTargets];
		for (auto i = 0u; i < rpd.m_RenderTargets.Size(); i++)
		{
			rtvs[i] = m_RenderTargets[i];
		}
		BeginRenderPass(pRenderPass, rtvs, *(ClearColorValue**)(&m_RenderTargetClearValues), m_pDepthStencilTarget, *(ClearColorValue*)(&m_DepthStencilTargetClearValue));
	}
}


void BvCommandBufferVk::DecommitRenderTargets()
{
	if (m_CurrentState == CurrentState::kInRenderPass)
	{
		EndRenderPass();
	}
	else if (m_CurrentState == CurrentState::kInRender)
	{
		vkCmdEndRenderingKHR(m_CommandBuffer);
	}

	m_CurrentState = BvCommandBuffer::CurrentState::kRecording;
}


void BvCommandBufferVk::ClearStateData()
{
	m_RenderTargets.Clear();
	m_RenderTargetClearValues.Clear();
	m_RenderTargetLoadOps.Clear();
	m_pDepthStencilTarget = nullptr;
	m_pGraphicsPSO = nullptr;
	m_pComputePSO = nullptr;
	m_Viewports.Clear();
	m_Scissors.Clear();
	m_VertexBuffers.Clear();
	m_VertexBufferOffsets.Clear();
	m_pIndexBufferView = nullptr;
	m_DescriptorSets.Clear();

	m_RenderTargetsBindNeeded = false;
	m_GraphicsPSOBindNeeded = false;
	m_ComputePSOBindNeeded = false;
	m_ViewportsBindNeeded = false;
	m_ScissorsBindNeeded = false;
	m_VertexBuffersBindNeeded = false;
	m_IndexBufferBindNeeded = false;
	m_DescriptorSetBindNeeded = false;
}