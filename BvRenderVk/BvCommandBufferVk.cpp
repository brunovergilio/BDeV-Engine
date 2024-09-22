#include "BvCommandBufferVk.h"
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
#include "BDeV/Core/RenderAPI/BvRenderAPIUtils.h"


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
	BvAssert(renderPassTargetCount <= kMaxRenderTargetsWithDepth, "Too many render targets");

	ResetRenderTargets();

	BvFixedVector<VkClearValue, kMaxRenderTargetsWithDepth> clearValues;
	VkRect2D renderArea{};
	VkRenderPass renderPass = TO_VK(pRenderPass)->GetHandle();
	FramebufferDesc fbDesc;
	fbDesc.m_RenderPass = renderPass;
	fbDesc.m_Width = kU32Max;
	fbDesc.m_Height = kU32Max;
	fbDesc.m_LayerCount = kU32Max;
	for (auto i = 0u; i < renderPassTargetCount; ++i)
	{
		auto viewVk = TO_VK(pRenderPassTargets->m_pView);
		auto& viewDesc = viewVk->GetDesc();
		auto& desc = viewDesc.m_pTexture->GetDesc();
		fbDesc.m_Width = std::min(desc.m_Size.width, fbDesc.m_Width);
		fbDesc.m_Height = std::min(desc.m_Size.height, fbDesc.m_Height);
		fbDesc.m_LayerCount = std::min((viewDesc.m_SubresourceDesc.layerCount == kU32Max ? desc.m_LayerCount : viewDesc.m_SubresourceDesc.layerCount)
			- viewDesc.m_SubresourceDesc.firstLayer, fbDesc.m_LayerCount);

		fbDesc.m_Views.EmplaceBack(viewVk->GetHandle());
		memcpy(clearValues.EmplaceBack().color.float32, pRenderPassTargets[i].m_ClearValues.colors, sizeof(float) * 4);
	}

	renderArea.extent.width = fbDesc.m_Width;
	renderArea.extent.height = fbDesc.m_Height;

	VkSubpassBeginInfo spBeginInfo{ VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO, nullptr, VK_SUBPASS_CONTENTS_INLINE };
	VkRenderPassBeginInfo rpBeginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	//rpBeginInfo.pNext = nullptr;
	rpBeginInfo.renderPass = renderPass;
	rpBeginInfo.framebuffer = m_pFrameData->GetFramebuffer(fbDesc);
	rpBeginInfo.renderArea = renderArea;
	rpBeginInfo.clearValueCount = (u32)clearValues.Size();
	rpBeginInfo.pClearValues = clearValues.Data();

	vkCmdBeginRenderPass2(m_CommandBuffer, &rpBeginInfo, &spBeginInfo);

	m_CurrentState = State::kRenderPass;
}


void BvCommandBufferVk::NextSubpass()
{
	VkSubpassBeginInfo beginInfo{ VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO, nullptr, VK_SUBPASS_CONTENTS_INLINE };
	VkSubpassEndInfo endInfo{ VK_STRUCTURE_TYPE_SUBPASS_END_INFO, nullptr };
	vkCmdNextSubpass2(m_CommandBuffer, &beginInfo, &endInfo);
}


void BvCommandBufferVk::EndRenderPass()
{
	BvAssert(m_CurrentState == State::kRenderPass, "Command buffer not in render pass");

	VkSubpassEndInfo endInfo{ VK_STRUCTURE_TYPE_SUBPASS_END_INFO, nullptr };
	vkCmdEndRenderPass2(m_CommandBuffer, &endInfo);

	m_CurrentState = State::kRecording;
}


void BvCommandBufferVk::SetRenderTargets(u32 renderTargetCount, const RenderTargetDesc* pRenderTargets)
{
	ResetRenderTargets();

	if (!renderTargetCount)
	{
		return;
	}

	BvAssert(renderTargetCount <= kMaxRenderTargetsWithDepth, "Too many render targets");

	VkExtent2D renderArea{ kU32Max, kU32Max };

	VkRenderingAttachmentInfo colorAttachments[kMaxRenderTargets]{};
	VkRenderingAttachmentInfo depthAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
	VkRenderingAttachmentInfo stencilAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
	VkRenderingFragmentShadingRateAttachmentInfoKHR shadingRateAttachment{ VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR };
	VkRenderingInfo renderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };

	u32 colorAttachmentCount = 0;
	u32 resolveCount = 0;
	bool hasDepth = false;
	bool hasStencil = true;
	bool hasShadingRate = false;

	u32 layerCount = kU32Max;
	for (u32 i = 0; i < renderTargetCount; i++)
	{
		if (colorAttachmentCount >= 8)
		{
			continue;
		}

		const auto& renderTarget = pRenderTargets[i];
		auto pView = TO_VK(renderTarget.m_pView);
		auto pTexture = TO_VK(renderTarget.m_pView->GetTexture());
		auto& desc = renderTarget.m_pView->GetTexture()->GetDesc();
		auto& viewDesc = renderTarget.m_pView->GetDesc();

		layerCount = std::min(layerCount, (viewDesc.m_SubresourceDesc.layerCount == kU32Max ? desc.m_LayerCount : viewDesc.m_SubresourceDesc.layerCount)
			 - viewDesc.m_SubresourceDesc.firstLayer);
		renderArea.width = std::min(renderArea.width, desc.m_Size.width);
		renderArea.height = std::min(renderArea.height, desc.m_Size.height);

		auto loadOp = GetVkAttachmentLoadOp(renderTarget.m_LoadOp);
		auto storeOp = GetVkAttachmentStoreOp(renderTarget.m_StoreOp);

		auto aspectFlags = GetVkFormatMap(viewDesc.m_Format).aspectFlags;
		bool isColorTarget = (aspectFlags & VK_IMAGE_ASPECT_COLOR_BIT) != 0;
		bool isDepthTarget = (aspectFlags & VK_IMAGE_ASPECT_DEPTH_BIT) != 0;
		bool isStencilTarget = (aspectFlags & VK_IMAGE_ASPECT_STENCIL_BIT) != 0;
		bool isShadingRate = renderTarget.m_State == ResourceState::kShadingRate;
		bool isResolveImage = renderTarget.m_ResolveMode != ResolveMode::kNone;

		if (isShadingRate)
		{
			shadingRateAttachment.imageView = pView->GetHandle();
			shadingRateAttachment.imageLayout = VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;
			shadingRateAttachment.shadingRateAttachmentTexelSize.width = renderTarget.m_ShadingRateTexelSizes[0];
			shadingRateAttachment.shadingRateAttachmentTexelSize.height = renderTarget.m_ShadingRateTexelSizes[1];
			renderingInfo.pNext = &shadingRateAttachment;
		}
		else if (isColorTarget)
		{
			if (!isResolveImage)
			{
				colorAttachments[colorAttachmentCount].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
				colorAttachments[colorAttachmentCount].imageView = pView->GetHandle();
				colorAttachments[colorAttachmentCount].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				colorAttachments[colorAttachmentCount].loadOp = loadOp;
				colorAttachments[colorAttachmentCount].storeOp = storeOp;
				if (loadOp == VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR)
				{
					memcpy(colorAttachments[colorAttachmentCount].clearValue.color.float32, renderTarget.m_ClearValues.colors, sizeof(float) * 4);
				}

				// If any of the view objects happen to be a swap chain texture, we need to request
				// a semaphore from the swap chain
				if (auto pSwapChain = pTexture->GetSwapChain())
				{
					m_SwapChains.EmplaceBack(pSwapChain);
				}
				++colorAttachmentCount;
			}
			else
			{
				colorAttachments[resolveCount].resolveImageView = pView->GetHandle();
				colorAttachments[resolveCount].resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				colorAttachments[resolveCount].resolveMode = GetVkResolveMode(renderTarget.m_ResolveMode);
				++resolveCount;
			}
		}
		else if (isDepthTarget)
		{
			if (!isResolveImage)
			{
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

				if (isStencilTarget)
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
			}
			else
			{
				depthAttachment.resolveImageView = pView->GetHandle();
				depthAttachment.resolveImageLayout = renderTarget.m_State == ResourceState::kDepthStencilWrite ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
					: VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
				depthAttachment.resolveMode = GetVkResolveMode(renderTarget.m_ResolveMode);

				if (isStencilTarget)
				{
					stencilAttachment.resolveImageView = depthAttachment.resolveImageView;
					stencilAttachment.resolveImageLayout = renderTarget.m_State == ResourceState::kDepthStencilWrite ? VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL
						: VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
					stencilAttachment.resolveMode = depthAttachment.resolveMode;
				}
			}
		}

		auto textureHandle = pTexture->GetHandle();
		if (renderTarget.m_StateBefore != renderTarget.m_State)
		{
			auto& barrier = m_PreRenderBarriers.EmplaceBack(VkImageMemoryBarrier2{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 });
			//barrier.pNext = nullptr;
			barrier.image = textureHandle;
			barrier.subresourceRange.aspectMask = aspectFlags;
			barrier.subresourceRange.baseMipLevel = viewDesc.m_SubresourceDesc.firstMip;
			barrier.subresourceRange.levelCount = viewDesc.m_SubresourceDesc.mipCount;
			barrier.subresourceRange.baseArrayLayer = viewDesc.m_SubresourceDesc.firstLayer;
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
			barrier.image = textureHandle;
			barrier.subresourceRange.aspectMask = aspectFlags;
			barrier.subresourceRange.baseMipLevel = viewDesc.m_SubresourceDesc.firstMip;
			barrier.subresourceRange.levelCount = viewDesc.m_SubresourceDesc.mipCount;
			barrier.subresourceRange.baseArrayLayer = viewDesc.m_SubresourceDesc.firstLayer;
			barrier.subresourceRange.layerCount = viewDesc.m_SubresourceDesc.layerCount;

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
	constexpr u32 kMaxViewports = 16;
	BvAssert(viewportCount <= kMaxViewports, "Viewport count greater than limit");
	VkViewport vps[kMaxViewports];
	for (auto i = 0u; i < viewportCount; ++i)
	{
		vps[i].x = pViewports[i].x;
		vps[i].y = pViewports[i].y + pViewports[i].height; // Flip viewport
		vps[i].width = pViewports[i].width;
		vps[i].height = -pViewports[i].height; // Flip viewport
		vps[i].minDepth = pViewports[i].minDepth;
		vps[i].maxDepth = pViewports[i].maxDepth;
	}
	
	vkCmdSetViewportWithCount(m_CommandBuffer, viewportCount, vps);
}


void BvCommandBufferVk::SetScissors(u32 scissorCount, const Rect* pScissors)
{
	constexpr u32 kMaxScissors = 16;
	BvAssert(scissorCount <= kMaxScissors, "Scissor count greater than limit");
	VkRect2D scissors[kMaxScissors];
	for (auto i = 0u; i < scissorCount; ++i)
	{
		scissors[i].offset.x = pScissors[i].x;
		scissors[i].offset.y = pScissors[i].y;
		scissors[i].extent.width = pScissors[i].width;
		scissors[i].extent.height = pScissors[i].height;
	}

	vkCmdSetScissorWithCount(m_CommandBuffer, scissorCount, scissors);
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


void BvCommandBufferVk::SetShaderConstants(u32 size, const void* pData, u32 offset)
{
	auto pSRL = TO_VK(m_pGraphicsPipeline ? m_pGraphicsPipeline->GetDesc().m_pShaderResourceLayout :
		m_pComputePipeline->GetDesc().m_pShaderResourceLayout);

	auto stageFlags = GetVkShaderStageFlags(pSRL->GetDesc().m_ShaderResourceConstant.m_ShaderStages);

	vkCmdPushConstants(m_CommandBuffer, pSRL->GetPipelineLayoutHandle(), stageFlags, offset, size, pData);
}


void BvCommandBufferVk::SetVertexBufferViews(u32 vertexBufferCount, const BvBufferView* const* pVertexBufferViews, u32 firstBinding)
{
	constexpr u32 kMaxVertexBufferViews = 16;
	BvAssert(vertexBufferCount <= kMaxVertexBufferViews, "Vertex buffer view count greater than limit");

	VkBuffer vertexBuffers[kMaxVertexBufferViews];
	VkDeviceSize vertexBufferOffsets[kMaxVertexBufferViews];
	for (auto i = 0u; i < vertexBufferCount; i++)
	{
		vertexBuffers[i] = TO_VK(pVertexBufferViews[i]->GetBuffer())->GetHandle();
		vertexBufferOffsets[i] = pVertexBufferViews[i]->GetDesc().m_Offset;
	}

	vkCmdBindVertexBuffers(m_CommandBuffer, firstBinding, vertexBufferCount, vertexBuffers, vertexBufferOffsets);
}


void BvCommandBufferVk::SetIndexBufferView(const BvBufferView* pIndexBufferView, IndexFormat indexFormat)
{
	auto formatVk = GetVkIndexType(indexFormat);
	
	vkCmdBindIndexBuffer(m_CommandBuffer, static_cast<BvBufferVk*>(pIndexBufferView->GetBuffer())->GetHandle(), pIndexBufferView->GetDesc().m_Offset, formatVk);
}


void BvCommandBufferVk::SetDepthBounds(float min, float max)
{
	vkCmdSetDepthBounds(m_CommandBuffer, min, max);
}


void BvCommandBufferVk::SetStencilRef(u32 stencilRef)
{
	vkCmdSetStencilReference(m_CommandBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, stencilRef);
}


void BvCommandBufferVk::SetBlendConstants(const float(pColors[4]))
{
	vkCmdSetBlendConstants(m_CommandBuffer, pColors);
}


void BvCommandBufferVk::SetShadingRate(ShadingRateDimensions dimensions, ShadingRateCombinerOp (pCombinerOps[2]))
{
	VkExtent2D fragmentSize{ 1, 1 };
	fragmentSize.width = ((((u32)dimensions) >> 2) & 0x3) << 1u;
	if (!fragmentSize.width)
	{
		fragmentSize.width = 1;
	}
	
	fragmentSize.height = (((u32)dimensions) & 0x3) << 1u;
	if (!fragmentSize.height)
	{
		fragmentSize.height = 1;
	}

	VkFragmentShadingRateCombinerOpKHR combinerOps[] =
	{
		GetVkShadingRateCombinerOp(pCombinerOps[0]),
		GetVkShadingRateCombinerOp(pCombinerOps[1]),
	};

	vkCmdSetFragmentShadingRateKHR(m_CommandBuffer, &fragmentSize, combinerOps);
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
		if (pBarriers[i].m_Type == ResourceBarrierDesc::Type::kMemory)
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
		else if (pBarriers[i].m_pBuffer)
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
	}

	if (m_BufferBarriers.Size() > 0 || m_ImageBarriers.Size() > 0 || m_MemoryBarriers.Size() > 0)
	{
		ResourceBarrier(static_cast<u32>(m_BufferBarriers.Size()), m_BufferBarriers.Data(),
			static_cast<u32>(m_ImageBarriers.Size()), m_ImageBarriers.Data(),
			static_cast<u32>(m_MemoryBarriers.Size()), m_MemoryBarriers.Data());
	}

	m_MemoryBarriers.Clear();
	m_BufferBarriers.Clear();
	m_ImageBarriers.Clear();
}


void BvCommandBufferVk::SetPredication(const BvBuffer* pBuffer, u64 offset, PredicationOp predicationOp)
{
	if (pBuffer)
	{
		auto pBufferVk = TO_VK(pBuffer);

		VkConditionalRenderingBeginInfoEXT info{ VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_EXT };
		if (predicationOp == PredicationOp::kNotEqualZero)
		{
			info.flags = VK_CONDITIONAL_RENDERING_INVERTED_BIT_EXT;
		}
		info.offset = offset;
		info.buffer = pBufferVk->GetHandle();
		vkCmdBeginConditionalRenderingEXT(m_CommandBuffer, &info);
	}
	else
	{
		vkCmdEndConditionalRenderingEXT(m_CommandBuffer);
	}
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

	auto pSRL = TO_VK(m_pGraphicsPipeline->GetDesc().m_pShaderResourceLayout);
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

		if (m_PostRenderBarriers.Size() > 0)
		{
			VkDependencyInfo dependencyInfo{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
			dependencyInfo.imageMemoryBarrierCount = static_cast<u32>(m_PostRenderBarriers.Size());
			dependencyInfo.pImageMemoryBarriers = m_PostRenderBarriers.Data();

			vkCmdPipelineBarrier2(m_CommandBuffer, &dependencyInfo);
		}

		m_PostRenderBarriers.Clear();

		m_CurrentState = State::kRecording;
	}
}