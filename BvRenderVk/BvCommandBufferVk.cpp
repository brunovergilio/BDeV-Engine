#include "BvCommandBufferVk.h"
#include "BvUtilsVk.h"
#include "BvRenderEngineVk.h"
#include "BvRenderDeviceVk.h"
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
#include "BvBufferVk.h"
#include "BvBufferViewVk.h"
#include "BvSamplerVk.h"
#include "BvQueryVk.h"
#include "BvCommandQueueVk.h"
#include "BvCommandContextVk.h"
#include "BvShaderResourceVk.h"
#include "BvAccelerationStructureVk.h"
#include "BvShaderBindingTableVk.h"
#include "BDeV/Core/RenderAPI/BvRenderAPIUtils.h"


BvCommandBufferVk::BvCommandBufferVk(BvRenderDeviceVk* pDevice, VkCommandBuffer commandBuffer, BvFrameDataVk* pFrameData)
	: m_pDevice(pDevice), m_CommandBuffer(commandBuffer), m_pFrameData(pFrameData), m_HasDebugUtils(pDevice->GetEngine()->HasDebugUtils())
{
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
	m_pShaderResourceLayout = nullptr;
	m_DescriptorSets.Clear();
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


void BvCommandBufferVk::BeginRenderPass(const IBvRenderPass* pRenderPass, u32 renderPassTargetCount, const RenderPassTargetDesc* pRenderPassTargets)
{
	BV_ASSERT(pRenderPass != nullptr, "Invalid render pass");
	BV_ASSERT(pRenderPassTargets != nullptr, "No render / depth targets");
	BV_ASSERT(renderPassTargetCount <= kMaxRenderTargetsWithDepth, "Too many render targets");

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
		auto viewVk = TO_VK(pRenderPassTargets[i].m_pView);
		if (auto pSwapChain = TO_VK(viewVk->GetDesc().m_pTexture)->GetSwapChain())
		{
			AddSwapChain(pSwapChain);
		}
		auto& viewDesc = viewVk->GetDesc();
		auto& desc = viewDesc.m_pTexture->GetDesc();
		fbDesc.m_Width = std::min(desc.m_Size.width, fbDesc.m_Width);
		fbDesc.m_Height = std::min(desc.m_Size.height, fbDesc.m_Height);
		fbDesc.m_LayerCount = std::min((viewDesc.m_SubresourceDesc.layerCount == kU32Max ? desc.m_ArraySize : viewDesc.m_SubresourceDesc.layerCount)
			- viewDesc.m_SubresourceDesc.firstLayer, fbDesc.m_LayerCount);

		fbDesc.m_Views.EmplaceBack(viewVk->GetHandle());
		if (!IsDepthOrStencilFormat(viewDesc.m_Format))
		{
			memcpy(clearValues.EmplaceBack().color.float32, pRenderPassTargets[i].m_ClearValues.colors, sizeof(float) * 4);
		}
		else
		{
			clearValues.EmplaceBack().depthStencil = { pRenderPassTargets[i].m_ClearValues.depth, pRenderPassTargets[i].m_ClearValues.stencil };
		}
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
	BV_ASSERT(m_CurrentState == State::kRenderPass, "Command buffer not in render pass");

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

	BV_ASSERT(renderTargetCount <= kMaxRenderTargetsWithDepth, "Too many render targets");

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
		auto pTexture = TO_VK(renderTarget.m_pView->GetDesc().m_pTexture);
		auto& desc = renderTarget.m_pView->GetDesc().m_pTexture->GetDesc();
		auto& viewDesc = renderTarget.m_pView->GetDesc();

		layerCount = std::min(layerCount, (viewDesc.m_SubresourceDesc.layerCount == kU32Max ? desc.m_ArraySize : viewDesc.m_SubresourceDesc.layerCount)
			 - viewDesc.m_SubresourceDesc.firstLayer);
		renderArea.width = std::min(renderArea.width, desc.m_Size.width);
		renderArea.height = std::min(renderArea.height, desc.m_Size.height);

		auto loadOp = GetVkAttachmentLoadOp(renderTarget.m_LoadOp);
		auto storeOp = GetVkAttachmentStoreOp(renderTarget.m_StoreOp);

		auto aspectFlags = GetVkFormatMap(viewDesc.m_Format).aspectFlags;
		bool isColorTarget = (aspectFlags & VK_IMAGE_ASPECT_COLOR_BIT) != 0;
		bool isDepthTarget = (aspectFlags & VK_IMAGE_ASPECT_DEPTH_BIT) != 0;
		bool isStencilTarget = (aspectFlags & VK_IMAGE_ASPECT_STENCIL_BIT) != 0;
		bool isShadingRate = renderTarget.m_State == ResourceState::kShadingRate
			|| renderTarget.m_ShadingRateTexelSizes[0] != 0 || renderTarget.m_ShadingRateTexelSizes[1] != 0;
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
					AddSwapChain(pSwapChain);
				}
				++colorAttachmentCount;
			}
			else
			{
				BV_ASSERT(renderTarget.m_ResolveMode == ResolveMode::kAverage, "Invalid resolve mode for render target");
				colorAttachments[resolveCount].resolveImageView = pView->GetHandle();
				colorAttachments[resolveCount].resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				colorAttachments[resolveCount].resolveMode = GetVkResolveMode(renderTarget.m_ResolveMode);
				++resolveCount;

				// If any of the view objects happen to be a swap chain texture, we need to request
				// a semaphore from the swap chain
				if (auto pSwapChain = pTexture->GetSwapChain())
				{
					if (!m_SwapChains.Contains(pSwapChain))
					{
						m_SwapChains.EmplaceBack(pSwapChain);
					}
				}
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
				BV_ASSERT(renderTarget.m_ResolveMode == ResolveMode::kMin || renderTarget.m_ResolveMode == ResolveMode::kMax,
					"Invalid resolve mode for depth stencil target");
				depthAttachment.resolveImageView = pView->GetHandle();
				depthAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
				depthAttachment.resolveMode = GetVkResolveMode(renderTarget.m_ResolveMode);

				if (isStencilTarget)
				{
					stencilAttachment.resolveImageView = depthAttachment.resolveImageView;
					stencilAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
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

			barrier.oldLayout = GetVkImageLayout(renderTarget.m_StateBefore, isDepthTarget || isStencilTarget);
			barrier.newLayout = GetVkImageLayout(renderTarget.m_State, isDepthTarget || isStencilTarget);

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

			barrier.oldLayout = GetVkImageLayout(renderTarget.m_State, isDepthTarget || isStencilTarget);
			barrier.newLayout = GetVkImageLayout(renderTarget.m_StateAfter, isDepthTarget || isStencilTarget);

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
	BV_ASSERT(viewportCount <= kMaxViewports, "Viewport count greater than limit");
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
	BV_ASSERT(scissorCount <= kMaxScissors, "Scissor count greater than limit");
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


void BvCommandBufferVk::SetGraphicsPipeline(const IBvGraphicsPipelineState* pPipeline)
{
	auto pPSO = TO_VK(pPipeline);
	if (!pPSO->HasMeshShaders())
	{
		EndMeshQueries();
	}

	m_pComputePipeline = nullptr;
	m_pRayTracingPipeline = nullptr;
	m_pGraphicsPipeline = pPSO;
	m_pShaderResourceLayout = TO_VK(m_pGraphicsPipeline->GetDesc().m_pShaderResourceLayout);
	m_PipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
	vkCmdBindPipeline(m_CommandBuffer, m_PipelineBindPoint, m_pGraphicsPipeline->GetHandle());
}


void BvCommandBufferVk::SetComputePipeline(const IBvComputePipelineState* pPipeline)
{
	ResetRenderTargets();

	m_pGraphicsPipeline = nullptr;
	m_pRayTracingPipeline = nullptr;
	m_pComputePipeline = TO_VK(pPipeline);
	m_pShaderResourceLayout = TO_VK(m_pComputePipeline->GetDesc().m_pShaderResourceLayout);
	m_PipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE;
	vkCmdBindPipeline(m_CommandBuffer, m_PipelineBindPoint, m_pComputePipeline->GetHandle());
}


void BvCommandBufferVk::SetRayTracingPipeline(const IBvRayTracingPipelineState* pPipeline)
{
	ResetRenderTargets();

	m_pGraphicsPipeline = nullptr;
	m_pComputePipeline = nullptr;
	m_pRayTracingPipeline = TO_VK(pPipeline);
	m_pShaderResourceLayout = TO_VK(m_pRayTracingPipeline->GetDesc().m_pShaderResourceLayout);
	m_PipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
	vkCmdBindPipeline(m_CommandBuffer, m_PipelineBindPoint, m_pRayTracingPipeline->GetHandle());
}


void BvCommandBufferVk::SetShaderResourceParams(u32 resourceParamsCount, IBvShaderResourceParams* const* ppResourceParams, u32 startIndex)
{
	if (startIndex + resourceParamsCount > m_DescriptorSets.Size())
	{
		m_DescriptorSets.Resize(startIndex + resourceParamsCount, VK_NULL_HANDLE);
	}

	for (auto i = 0; i < resourceParamsCount; ++i)
	{
		auto pSRP = TO_VK(ppResourceParams[i]);
		m_DescriptorSets[i] = pSRP ? pSRP->GetHandle() : VK_NULL_HANDLE;
	}

	vkCmdBindDescriptorSets(m_CommandBuffer, m_PipelineBindPoint, m_pShaderResourceLayout->GetPipelineLayoutHandle(), startIndex, resourceParamsCount, m_DescriptorSets.Data() + startIndex, 0, nullptr);
}


void BvCommandBufferVk::SetConstantBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, TO_VK(ppResources[i]), set, binding, startIndex + i);
	}
}


void BvCommandBufferVk::SetStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, TO_VK(ppResources[i]), set, binding, startIndex + i);
	}
}


void BvCommandBufferVk::SetRWStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, TO_VK(ppResources[i]), set, binding, startIndex + i);
	}
}

void BvCommandBufferVk::SetDynamicConstantBuffers(u32 count, const IBvBufferView* const* ppResources, const u32* pOffsets, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, TO_VK(ppResources[i]), set, binding, startIndex + i, pOffsets[i]);
	}
}


void BvCommandBufferVk::SetDynamicStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, const u32* pOffsets, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, TO_VK(ppResources[i]), set, binding, startIndex + i, pOffsets[i]);
	}
}


void BvCommandBufferVk::SetDynamicRWStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, const u32* pOffsets, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, TO_VK(ppResources[i]), set, binding, startIndex + i, pOffsets[i]);
	}
}


void BvCommandBufferVk::SetFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, TO_VK(ppResources[i]), set, binding, startIndex + i);
	}
}


void BvCommandBufferVk::SetRWFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, TO_VK(ppResources[i]), set, binding, startIndex + i);
	}
}


void BvCommandBufferVk::SetTextures(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, TO_VK(ppResources[i]), set, binding, startIndex + i);
	}
}


void BvCommandBufferVk::SetRWTextures(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, TO_VK(ppResources[i]), set, binding, startIndex + i);
	}
}


void BvCommandBufferVk::SetSamplers(u32 count, const IBvSampler* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(VK_DESCRIPTOR_TYPE_SAMPLER, TO_VK(ppResources[i]), set, binding, startIndex + i);
	}
}


void BvCommandBufferVk::SetInputAttachments(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, TO_VK(ppResources[i]), set, binding, startIndex + i);
	}
}


void BvCommandBufferVk::SetAccelerationStructures(u32 count, const IBvAccelerationStructure* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, TO_VK(ppResources[i]), set, binding, startIndex + i);
	}
}


void BvCommandBufferVk::SetShaderConstants(u32 size, const void* pData, u32 binding, u32 set)
{
	if (auto pConstant = m_pShaderResourceLayout->GetPushConstantData(size, binding, set))
	{
		vkCmdPushConstants(m_CommandBuffer, m_pShaderResourceLayout->GetPipelineLayoutHandle(), pConstant->m_ShaderStages, pConstant->m_Offset, size, pData);
	}
}


void BvCommandBufferVk::SetVertexBufferViews(u32 vertexBufferCount, const VertexBufferView* pVertexBufferViews, u32 firstBinding)
{
	BV_ASSERT(vertexBufferCount <= kMaxVertexBuffers, "Vertex buffer view count greater than limit");

	VkBuffer vertexBuffers[kMaxVertexBuffers]{};
	VkDeviceSize vertexBufferOffsets[kMaxVertexBuffers]{};
	VkDeviceSize vertexBufferSizes[kMaxVertexBuffers]{};
	VkDeviceSize vertexBufferStrides[kMaxVertexBuffers]{};
	for (auto i = 0u; i < vertexBufferCount; i++)
	{
		if (pVertexBufferViews[i].m_pBuffer)
		{
			vertexBuffers[i] = TO_VK(pVertexBufferViews[i].m_pBuffer)->GetHandle();
			vertexBufferOffsets[i] = pVertexBufferViews[i].m_Offset;
			vertexBufferSizes[i] = pVertexBufferViews[i].m_pBuffer->GetDesc().m_Size;
			vertexBufferStrides[i] = pVertexBufferViews[i].m_Stride;
		}
	}
	vkCmdBindVertexBuffers2(m_CommandBuffer, firstBinding, vertexBufferCount, vertexBuffers, vertexBufferOffsets, vertexBufferSizes, vertexBufferStrides);
}


void BvCommandBufferVk::SetIndexBufferView(const IndexBufferView& indexBufferView)
{
	auto formatVk = GetVkIndexType(indexBufferView.m_IndexFormat);
	VkBuffer indexBuffer = VK_NULL_HANDLE;
	VkDeviceSize indexBufferOffset = 0;
	if (indexBufferView.m_pBuffer)
	{
		indexBuffer = TO_VK(indexBufferView.m_pBuffer)->GetHandle();
		indexBufferOffset = indexBufferView.m_Offset;
	}
	
	vkCmdBindIndexBuffer(m_CommandBuffer, indexBuffer, indexBufferView.m_Offset, formatVk);
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


void BvCommandBufferVk::Draw(const DrawCommandArgs& args)
{
	FlushDescriptorSets();

	vkCmdDraw(m_CommandBuffer, args.m_VertexCount, args.m_InstanceCount, args.m_FirstVertex, args.m_FirstInstance);
}


void BvCommandBufferVk::DrawIndexed(const DrawIndexedCommandArgs& args)
{
	FlushDescriptorSets();

	vkCmdDrawIndexed(m_CommandBuffer, args.m_IndexCount, args.m_InstanceCount, args.m_FirstIndex, args.m_VertexOffset, args.m_FirstInstance);
}


void BvCommandBufferVk::Dispatch(const DispatchCommandArgs& args)
{
	FlushDescriptorSets();

	vkCmdDispatch(m_CommandBuffer, args.m_ThreadGroupCountX, args.m_ThreadGroupCountY, args.m_ThreadGroupCountZ);
}


void BvCommandBufferVk::DispatchMesh(const DispatchMeshCommandArgs& args)
{
	FlushDescriptorSets();

	BeginMeshQueries();

	vkCmdDrawMeshTasksEXT(m_CommandBuffer, args.m_ThreadGroupCountX, args.m_ThreadGroupCountY, args.m_ThreadGroupCountZ);
}


void BvCommandBufferVk::DrawIndirect(const IBvBuffer* pBuffer, u32 drawCount, u64 offset)
{
	FlushDescriptorSets();

	auto pBufferVk = TO_VK(pBuffer)->GetHandle();
	vkCmdDrawIndirect(m_CommandBuffer, pBufferVk, offset, drawCount, (u32)sizeof(VkDrawIndirectCommand));
}


void BvCommandBufferVk::DrawIndexedIndirect(const IBvBuffer* pBuffer, u32 drawCount, u64 offset)
{
	FlushDescriptorSets();

	auto pBufferVk = TO_VK(pBuffer)->GetHandle();
	vkCmdDrawIndexedIndirect(m_CommandBuffer, pBufferVk, offset, drawCount, (u32)sizeof(VkDrawIndexedIndirectCommand));
}


void BvCommandBufferVk::DispatchIndirect(const IBvBuffer* pBuffer, u64 offset)
{
	FlushDescriptorSets();

	auto pBufferVk = TO_VK(pBuffer)->GetHandle();
	vkCmdDispatchIndirect(m_CommandBuffer, pBufferVk, offset);
}


void BvCommandBufferVk::DispatchMeshIndirect(const IBvBuffer* pBuffer, u64 offset)
{
	FlushDescriptorSets();

	BeginMeshQueries();

	vkCmdDrawMeshTasksIndirectEXT(m_CommandBuffer, TO_VK(pBuffer)->GetHandle(), offset, 1, sizeof(VkDispatchIndirectCommand));
}


void BvCommandBufferVk::DispatchMeshIndirectCount(const IBvBuffer* pBuffer, u64 offset, const IBvBuffer* pCountBuffer, u64 countOffset, u32 maxCount)
{
	FlushDescriptorSets();

	BeginMeshQueries();

	vkCmdDrawMeshTasksIndirectCountEXT(m_CommandBuffer, TO_VK(pBuffer)->GetHandle(), offset, TO_VK(pCountBuffer)->GetHandle(), countOffset,
		maxCount, sizeof(VkDispatchIndirectCommand));
}


void BvCommandBufferVk::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const VkBufferCopy& copyRegion)
{
	ResetRenderTargets();

	vkCmdCopyBuffer(m_CommandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
}


void BvCommandBufferVk::CopyBuffer(const IBvBuffer* pSrcBuffer, IBvBuffer* pDstBuffer)
{
	VkBufferCopy region{};
	region.size = std::min(pSrcBuffer->GetDesc().m_Size, pDstBuffer->GetDesc().m_Size);

	auto pSrc = TO_VK(pSrcBuffer);
	auto pDst = TO_VK(pDstBuffer);

	CopyBuffer(pSrc->GetHandle(), pDst->GetHandle(), region);
}


void BvCommandBufferVk::CopyBuffer(const IBvBuffer* pSrcBuffer, IBvBuffer* pDstBuffer, const BufferCopyDesc& copyDesc)
{
	VkBufferCopy region{};
	region.srcOffset = copyDesc.m_SrcOffset;
	region.dstOffset = copyDesc.m_DstOffset;
	region.size = std::min(copyDesc.m_SrcSize, pSrcBuffer->GetDesc().m_Size);

	auto pSrc = TO_VK(pSrcBuffer);
	auto pDst = TO_VK(pDstBuffer);

	CopyBuffer(pSrc->GetHandle(), pDst->GetHandle(), region);
}


void BvCommandBufferVk::CopyTexture(const IBvTexture* pSrcTexture, IBvTexture* pDstTexture)
{
	ResetRenderTargets();

	auto& srcDesc = pSrcTexture->GetDesc();
	auto& dstDesc = pDstTexture->GetDesc();

	if ((srcDesc.m_Format != dstDesc.m_Format && srcDesc.m_Format != GetFormatInfo(dstDesc.m_Format).m_SRGBOrLinearVariant) || srcDesc.m_Size != dstDesc.m_Size
		|| srcDesc.m_MipLevels != dstDesc.m_MipLevels || srcDesc.m_ArraySize != dstDesc.m_ArraySize)
	{
		return;
	}

	for (auto layer = 0u; layer < srcDesc.m_ArraySize; ++layer)
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

	auto pSrc = TO_VK(pSrcTexture);
	auto pDst = TO_VK(pDstTexture);
	if (auto pSwapChain = pDst->GetSwapChain())
	{
		AddSwapChain(pSwapChain);
	}

	vkCmdCopyImage(m_CommandBuffer, pSrc->GetHandle(), VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		pDst->GetHandle(), VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		(u32)m_ImageCopyRegions.Size(), m_ImageCopyRegions.Data());
	
	m_ImageCopyRegions.Clear();
}


void BvCommandBufferVk::CopyTexture(const IBvTexture* pSrcTexture, IBvTexture* pDstTexture, const TextureCopyDesc& copyDesc)
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

	auto pSrc = TO_VK(pSrcTexture);
	auto pDst = TO_VK(pDstTexture);
	if (auto pSwapChain = pDst->GetSwapChain())
	{
		AddSwapChain(pSwapChain);
	}

	vkCmdCopyImage(m_CommandBuffer, pSrc->GetHandle(), VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		pDst->GetHandle(), VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1u, &imageCopyRegion);
}


void BvCommandBufferVk::CopyBufferToTexture(VkBuffer srcBuffer, VkImage dstTexture, u32 copyCount, const VkBufferImageCopy* pCopyRegions)
{
	ResetRenderTargets();

	vkCmdCopyBufferToImage(m_CommandBuffer, srcBuffer, dstTexture,
		VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, copyCount, pCopyRegions);
}


void BvCommandBufferVk::CopyBufferToTexture(const IBvBuffer* pSrcBuffer, IBvTexture* pDstTexture, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs)
{
	auto pSrc = TO_VK(pSrcBuffer);
	auto pDst = TO_VK(pDstTexture);

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

	CopyBufferToTexture(pSrc->GetHandle(), pDst->GetHandle(), copyCount, m_BufferImageCopyRegions.Data());
	
	m_BufferImageCopyRegions.Clear();
}


void BvCommandBufferVk::CopyTextureToBuffer(VkImage srcTexture, VkBuffer dstBuffer, u32 copyCount, const VkBufferImageCopy* pCopyRegions)
{
	ResetRenderTargets();

	vkCmdCopyImageToBuffer(m_CommandBuffer, srcTexture, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		dstBuffer, copyCount, pCopyRegions);
}


void BvCommandBufferVk::CopyTextureToBuffer(const IBvTexture* pSrcTexture, IBvBuffer* pDstBuffer, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs)
{
	auto pSrc = TO_VK(pSrcTexture);
	auto pDst = TO_VK(pDstBuffer);

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

	CopyTextureToBuffer(pSrc->GetHandle(), pDst->GetHandle(), copyCount, m_BufferImageCopyRegions.Data());

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
				GetVkAccessFlags(pBarriers[i].m_SrcState) : GetVkAccessFlags(pBarriers[i].m_SrcAccess);
			barrier.dstAccessMask = pBarriers[i].m_DstAccess == ResourceAccess::kAuto ?
				GetVkAccessFlags(pBarriers[i].m_DstState) : GetVkAccessFlags(pBarriers[i].m_DstAccess);

			barrier.srcStageMask |= pBarriers[i].m_SrcPipelineStage == PipelineStage::kAuto ?
				GetVkPipelineStageFlags(barrier.srcAccessMask) : GetVkPipelineStageFlags(pBarriers[i].m_SrcPipelineStage);
			barrier.dstStageMask |= pBarriers[i].m_DstPipelineStage == PipelineStage::kAuto ?
				GetVkPipelineStageFlags(barrier.dstAccessMask) : GetVkPipelineStageFlags(pBarriers[i].m_DstPipelineStage);
		}
		else if (pBarriers[i].m_pBuffer || pBarriers[i].m_pAS)
		{
			auto buffer = pBarriers[i].m_pBuffer ? TO_VK(pBarriers[i].m_pBuffer)->GetHandle() : TO_VK(pBarriers[i].m_pAS)->GetBuffer()->GetHandle();
			auto& barrier = m_BufferBarriers.EmplaceBack(VkBufferMemoryBarrier2{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2 });
			//barrier.pNext = nullptr;
			barrier.buffer = buffer;
			barrier.size = VK_WHOLE_SIZE;
			//barrier.offset = 0;
		
			barrier.srcAccessMask = pBarriers[i].m_SrcAccess == ResourceAccess::kAuto ?
				GetVkAccessFlags(pBarriers[i].m_SrcState) : GetVkAccessFlags(pBarriers[i].m_SrcAccess);
			barrier.dstAccessMask = pBarriers[i].m_DstAccess == ResourceAccess::kAuto ?
				GetVkAccessFlags(pBarriers[i].m_DstState) : GetVkAccessFlags(pBarriers[i].m_DstAccess);
		
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
			barrier.image = TO_VK(pBarriers[i].m_pTexture)->GetHandle();
			barrier.subresourceRange.aspectMask = GetVkFormatMap(pBarriers[i].m_pTexture->GetDesc().m_Format).aspectFlags;
			barrier.subresourceRange.baseMipLevel = pBarriers[i].m_Subresource.firstMip;
			barrier.subresourceRange.levelCount = pBarriers[i].m_Subresource.mipCount;
			barrier.subresourceRange.baseArrayLayer = pBarriers[i].m_Subresource.firstLayer;
			barrier.subresourceRange.layerCount = pBarriers[i].m_Subresource.layerCount;

			barrier.oldLayout = GetVkImageLayout(pBarriers[i].m_SrcState);
			barrier.newLayout = GetVkImageLayout(pBarriers[i].m_DstState);

			barrier.srcAccessMask = pBarriers[i].m_SrcAccess == ResourceAccess::kAuto ?
				GetVkAccessFlags(pBarriers[i].m_SrcState) : GetVkAccessFlags(pBarriers[i].m_SrcAccess);
			barrier.dstAccessMask = pBarriers[i].m_DstAccess == ResourceAccess::kAuto ?
				GetVkAccessFlags(pBarriers[i].m_DstState) : GetVkAccessFlags(pBarriers[i].m_DstAccess);

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


void BvCommandBufferVk::SetPredication(const IBvBuffer* pBuffer, u64 offset, PredicationOp predicationOp)
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


void BvCommandBufferVk::BeginQuery(IBvQuery* pQuery)
{
	auto pQueryVk = TO_VK(pQuery);
	auto queryType = pQueryVk->GetQueryType();
	auto pData = pQueryVk->Allocate(m_pFrameData->GetQueryHeapManager(), m_pFrameData->GetFrameIndex());
	if (queryType != QueryType::kTimestamp)
	{
		VkQueryControlFlags flags = queryType == QueryType::kOcclusion ? VK_QUERY_CONTROL_PRECISE_BIT : 0;
		vkCmdBeginQuery(m_CommandBuffer, pData->m_pQueryHeap->GetHandle(pData->m_HeapIndex), pData->m_QueryIndex, flags);
		if (queryType == QueryType::kMeshPipelineStatistics)
		{
			auto meshPool = pData->m_pQueryHeap->GetHandle(pData->m_HeapIndex, 1);
			if (meshPool != VK_NULL_HANDLE)
			{
				m_MeshQueries.EmplaceBack(pQueryVk);
			}
		}
	}

	m_pFrameData->AddQuery(pQueryVk);
}


void BvCommandBufferVk::EndQuery(IBvQuery* pQuery)
{
	ResetRenderTargets();

	auto pQueryVk = TO_VK(pQuery);
	auto queryType = pQueryVk->GetQueryType();
	auto frameIndex = m_pFrameData->GetFrameIndex();
	auto pData = pQueryVk->GetQueryData(frameIndex);
	auto pool = pData->m_pQueryHeap->GetHandle(pData->m_HeapIndex);
	auto meshPool = pData->m_pQueryHeap->GetHandle(pData->m_HeapIndex, 1);
	if (queryType == QueryType::kTimestamp)
	{
		vkCmdWriteTimestamp2(m_CommandBuffer, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, pool, pData->m_QueryIndex);
	}
	else
	{
		vkCmdEndQuery(m_CommandBuffer, pool, pData->m_QueryIndex);
	}
	VkBuffer buffer;
	u64 stride = 0, offset = 0;
	pData->m_pQueryHeap->GetBufferInformation(pData->m_HeapIndex, frameIndex, pData->m_QueryIndex, buffer, offset, stride);
	VkQueryResultFlags flags = VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT;
	if (queryType == QueryType::kOcclusionBinary)
	{
		flags |= VK_QUERY_RESULT_PARTIAL_BIT;
	}

	vkCmdCopyQueryPoolResults(m_CommandBuffer, pool, pData->m_QueryIndex, 1, buffer, offset, stride, flags);
	if (queryType == QueryType::kMeshPipelineStatistics && meshPool != VK_NULL_HANDLE)
	{
		pData->m_pQueryHeap->GetBufferInformation(pData->m_HeapIndex, frameIndex, pData->m_QueryIndex, buffer, offset, stride, 1);
		vkCmdCopyQueryPoolResults(m_CommandBuffer, meshPool, pData->m_QueryIndex, 1, buffer, offset, stride, flags);
	}
}


void BvCommandBufferVk::BeginEvent(const char* pName, const BvColor& color)
{
	if (!m_HasDebugUtils)
	{
		return;
	}

	VkDebugUtilsLabelEXT label{ VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
	label.pLabelName = pName;
	label.color[0] = color.m_Red;
	label.color[1] = color.m_Green;
	label.color[2] = color.m_Blue;
	label.color[3] = color.m_Alpha;
	vkCmdBeginDebugUtilsLabelEXT(m_CommandBuffer, &label);
}


void BvCommandBufferVk::EndEvent()
{
	if (!m_HasDebugUtils)
	{
		return;
	}

	vkCmdEndDebugUtilsLabelEXT(m_CommandBuffer);
}


void BvCommandBufferVk::SetMarker(const char* pName, const BvColor& color)
{
	if (!m_HasDebugUtils)
	{
		return;
	}

	VkDebugUtilsLabelEXT label{ VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
	label.pLabelName = pName;
	label.color[0] = color.m_Red;
	label.color[1] = color.m_Green;
	label.color[2] = color.m_Blue;
	label.color[3] = color.m_Alpha;
	vkCmdInsertDebugUtilsLabelEXT(m_CommandBuffer, &label);
}


void BvCommandBufferVk::BuildBLAS(const BLASBuildDesc& desc)
{
	if (desc.m_GeometryCount > desc.m_pBLAS->GetDesc().m_BLAS.m_GeometryCount)
	{
		return;
	}

	auto pAS = TO_VK(desc.m_pBLAS);
	auto& geoms = pAS->GetGeometries();
	for (auto i = 0u; i < desc.m_GeometryCount; ++i)
	{
		auto& srcGeometry = desc.m_pGeometries[i];

		// Try to find an index through the id; if not found,
		// revert back to the current index in the loop
		auto index = pAS->GetGeometryIndex(srcGeometry.m_Id);
		if (index == kU32Max)
		{
			index = i;
		}

		// If the data doesn't match, don't include it
		if (geoms[index].geometryType != GetVkGeometryType(srcGeometry.m_Type))
		{
			continue;
		}

		VkAccelerationStructureGeometryKHR& dstGeometry = m_ASGeometries.EmplaceBack(geoms[index]);
		VkAccelerationStructureBuildRangeInfoKHR& range = m_ASRanges.EmplaceBack();
		if (srcGeometry.m_Type == RayTracingGeometryType::kTriangles)
		{
			VkAccelerationStructureGeometryTrianglesDataKHR& triangle = dstGeometry.geometry.triangles;
			triangle.vertexData.deviceAddress = TO_VK(srcGeometry.m_Triangle.m_pVertexBuffer)->GetDeviceAddress() + srcGeometry.m_Triangle.m_VertexOffset;
			if (srcGeometry.m_Triangle.m_pIndexBuffer)
			{
				triangle.indexData.deviceAddress = TO_VK(srcGeometry.m_Triangle.m_pIndexBuffer)->GetDeviceAddress() + srcGeometry.m_Triangle.m_IndexOffset;
			}
		}
		else if (srcGeometry.m_Type == RayTracingGeometryType::kAABB)
		{
			VkAccelerationStructureGeometryAabbsDataKHR& aabb = dstGeometry.geometry.aabbs;
			aabb.data.deviceAddress = TO_VK(srcGeometry.m_AABB.m_pBuffer)->GetDeviceAddress() + srcGeometry.m_AABB.m_Offset;
		}
		range.primitiveCount = pAS->GetPrimitiveCounts()[index];
	}

	VkAccelerationStructureBuildGeometryInfoKHR buildInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	buildInfo.flags = GetVkBuildAccelerationStructureFlags(pAS->GetDesc().m_Flags);
	if (desc.m_Update)
	{
		buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
		buildInfo.srcAccelerationStructure = pAS->GetHandle();
	}
	else
	{
		buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	}
	buildInfo.dstAccelerationStructure = pAS->GetHandle();
	buildInfo.geometryCount = u32(m_ASGeometries.Size());
	buildInfo.pGeometries = m_ASGeometries.Data();
	//buildInfo.ppGeometries = nullptr;
	buildInfo.scratchData.deviceAddress = TO_VK(desc.m_pScratchBuffer)->GetDeviceAddress() + desc.m_ScratchBufferOffset;

	VkAccelerationStructureBuildRangeInfoKHR* pRanges = m_ASRanges.Data();
	vkCmdBuildAccelerationStructuresKHR(m_CommandBuffer, 1, &buildInfo, &pRanges);

	m_ASGeometries.Clear();
	m_ASRanges.Clear();
}


void BvCommandBufferVk::BuildTLAS(const TLASBuildDesc& desc)
{
	auto pAS = TO_VK(desc.m_pTLAS);
	VkAccelerationStructureGeometryKHR& dstGeometry = m_ASGeometries.EmplaceBack(pAS->GetGeometries()[0]);
	dstGeometry.geometry.instances.data.deviceAddress = TO_VK(desc.m_pInstanceBuffer)->GetDeviceAddress() + desc.m_InstanceBufferOffset;

	VkAccelerationStructureBuildGeometryInfoKHR buildInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	buildInfo.flags = GetVkBuildAccelerationStructureFlags(pAS->GetDesc().m_Flags);
	if (desc.m_Update)
	{
		buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
		buildInfo.srcAccelerationStructure = pAS->GetHandle();
	}
	else
	{
		buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	}
	buildInfo.dstAccelerationStructure = pAS->GetHandle();
	buildInfo.geometryCount = u32(m_ASGeometries.Size());
	buildInfo.pGeometries = m_ASGeometries.Data();
	//buildInfo.ppGeometries = nullptr;
	buildInfo.scratchData.deviceAddress = TO_VK(desc.m_pScratchBuffer)->GetDeviceAddress() + desc.m_ScratchBufferOffset;

	VkAccelerationStructureBuildRangeInfoKHR& range = m_ASRanges.EmplaceBack(VkAccelerationStructureBuildRangeInfoKHR{});
	range.primitiveCount = desc.m_InstanceCount;

	VkAccelerationStructureBuildRangeInfoKHR* pRanges = m_ASRanges.Data();
	vkCmdBuildAccelerationStructuresKHR(m_CommandBuffer, 1, &buildInfo, &pRanges);

	m_ASGeometries.Clear();
	m_ASRanges.Clear();
}


void BvCommandBufferVk::DispatchRays(const DispatchRaysCommandArgs& args)
{
	FlushDescriptorSets();

	VkStridedDeviceAddressRegionKHR addressRegions[4] =
	{
		{ args.m_RayGenShader.m_Address, args.m_RayGenShader.m_Size, args.m_RayGenShader.m_Size }, // For RayGen, stride == size
		{ args.m_MissShader.m_Address, args.m_MissShader.m_Stride, args.m_MissShader.m_Size },
		{ args.m_HitShader.m_Address, args.m_HitShader.m_Stride, args.m_HitShader.m_Size },
		{ args.m_CallableShader.m_Address, args.m_CallableShader.m_Stride, args.m_CallableShader.m_Size }
	};

	vkCmdTraceRaysKHR(m_CommandBuffer, &addressRegions[0], &addressRegions[1], &addressRegions[2], &addressRegions[3], args.m_Width, args.m_Height, args.m_Depth);
}


void BvCommandBufferVk::DispatchRaysIndirect(const IBvBuffer* pBuffer, u64 offset)
{
	FlushDescriptorSets();

	auto deviceAddress = pBuffer->GetDeviceAddress() + offset;
	vkCmdTraceRaysIndirect2KHR(m_CommandBuffer, deviceAddress);
}


void BvCommandBufferVk::FlushDescriptorSets()
{
	auto& rbs = m_pFrameData->GetResourceBindingState();
	if (rbs.IsEmpty())
	{
		return;
	}

	u64 hashSeed = 0;
	auto& srlDesc = m_pShaderResourceLayout->GetDesc();
	for (auto i = 0u; i < srlDesc.m_ShaderResourceSetCount; ++i)
	{
		auto& resourceSet = srlDesc.m_pShaderResourceSets[i];
		u32 set = resourceSet.m_Index;
		bool isSetDirty = rbs.IsDirty(set);

		for (auto j = 0u; j < resourceSet.m_ResourceCount; ++j)
		{
			auto& resource = resourceSet.m_pResources[j];
			for (auto arrayIndex = 0u; arrayIndex < resource.m_Count; arrayIndex++)
			{
				ResourceIdVk resId{ set, resource.m_Binding, arrayIndex };
				if (auto pResourceData = rbs.GetResource(resId))
				{
					if (isSetDirty)
					{
						m_WriteSets.PushBack({ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET });
						auto& writeSet = m_WriteSets.Back();
						//writeSet.dstSet = nullptr; // This is set in BvDescriptorSetVk::Update()
						writeSet.descriptorType = pResourceData->m_DescriptorType;
						writeSet.dstBinding = resource.m_Binding;
						writeSet.dstArrayElement = arrayIndex;
						writeSet.descriptorCount = 1;

						switch (pResourceData->m_DescriptorType)
						{
						case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
						case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
						case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
							writeSet.pBufferInfo = &pResourceData->m_Data.m_BufferInfo;
							HashCombine(hashSeed, pResourceData->m_Data.m_BufferInfo);
							break;
						case VK_DESCRIPTOR_TYPE_SAMPLER:
						case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
						case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
						case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
							writeSet.pImageInfo = &pResourceData->m_Data.m_ImageInfo;
							HashCombine(hashSeed, pResourceData->m_Data.m_ImageInfo);
							break;
						case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
						case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
							writeSet.pTexelBufferView = &pResourceData->m_Data.m_BufferView;
							HashCombine(hashSeed, pResourceData->m_Data.m_BufferView);
							break;
						case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
							auto& asWrite = m_ASWriteSets.EmplaceBack(VkWriteDescriptorSetAccelerationStructureKHR{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR });
							asWrite.accelerationStructureCount = 1;
							asWrite.pAccelerationStructures = &pResourceData->m_Data.m_AccelerationStructure;
							writeSet.pNext = &asWrite;
							HashCombine(hashSeed, pResourceData->m_Data.m_AccelerationStructure);
							break;
						}
					}

					if (pResourceData->m_DescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
						|| pResourceData->m_DescriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
					{
						m_DynamicOffsets.EmplaceBack(pResourceData->m_DynamicOffset);
					}
				}
			}
		}

		if (!m_WriteSets.Empty() || !m_DynamicOffsets.Empty())
		{
			VkDescriptorSet descriptorSet;
			if (isSetDirty)
			{
				descriptorSet = m_pFrameData->RequestDescriptorSet(set, m_pShaderResourceLayout, m_WriteSets, hashSeed, resourceSet.m_Bindless);
				if (m_DescriptorSets.Size() <= set)
				{
					m_DescriptorSets.Resize(set + 1, VK_NULL_HANDLE);
				}
				m_DescriptorSets[set] = descriptorSet;
			}
			else
			{
				descriptorSet = m_DescriptorSets[set];
			}

			vkCmdBindDescriptorSets(m_CommandBuffer, m_PipelineBindPoint, m_pShaderResourceLayout->GetPipelineLayoutHandle(), set,
				1, &descriptorSet, m_DynamicOffsets.Size(), m_DynamicOffsets.Size() > 0 ? m_DynamicOffsets.Data() : nullptr);
			m_WriteSets.Clear();
			m_ASWriteSets.Clear();
			m_DynamicOffsets.Clear();
		}

		rbs.MarkClean(set);
	}
}


void BvCommandBufferVk::ResetRenderTargets()
{
	if (m_CurrentState == State::kRenderPass)
	{
		EndMeshQueries();

		EndRenderPass();
	}
	else if (m_CurrentState == State::kRenderTarget)
	{
		EndMeshQueries();

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


void BvCommandBufferVk::AddSwapChain(BvSwapChainVk* pSwapChain)
{
	if (!m_SwapChains.Contains(pSwapChain))
	{
		m_SwapChains.EmplaceBack(pSwapChain);
	}
}


void BvCommandBufferVk::BeginMeshQueries()
{
	for (auto pQueryVk : m_MeshQueries)
	{
		auto frameIndex = m_pFrameData->GetFrameIndex();
		auto pData = pQueryVk->GetQueryData(frameIndex);
		vkCmdBeginQuery(m_CommandBuffer, pData->m_pQueryHeap->GetHandle(pData->m_HeapIndex, 1), pData->m_QueryIndex, 0);
	}
}


void BvCommandBufferVk::EndMeshQueries()
{
	for (auto pQueryVk : m_MeshQueries)
	{
		auto frameIndex = m_pFrameData->GetFrameIndex();
		auto pData = pQueryVk->GetQueryData(frameIndex);
		auto pool = pData->m_pQueryHeap->GetHandle(pData->m_HeapIndex, 1);
		auto index = pData->m_QueryIndex;
		vkCmdEndQuery(m_CommandBuffer, pool, index);
	}

	m_MeshQueries.Clear();
}