#include "BvCommandListD3D12.h"
#include "BvRenderEngineD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BvTextureViewD3D12.h"
#include "BvRenderPassD3D12.h"
#include "BvPipelineStateD3D12.h"
#include "BvBufferD3D12.h"
#include "BvTypeConversionsD3D12.h"
#include "BvCommandAllocatorD3D12.h"
#include "BvShaderResourceD3D12.h"
#include "BvTextureD3D12.h"
#include "BvSwapChainD3D12.h"
#include "BvBufferViewD3D12.h"
#include "BvSamplerD3D12.h"
#include "BvQueryD3D12.h"
#include "BvCommandQueueD3D12.h"
#include "BvCommandContextD3D12.h"
#include "BvShaderResourceD3D12.h"
#include "BvAccelerationStructureD3D12.h"
#include "BvShaderBindingTableD3D12.h"
#include "BDeV/Core/RenderAPI/BvRenderAPIUtils.h"


BvCommandListD3D12::BvCommandListD3D12(BvRenderDeviceD3D12* pDevice, ID3D12CommandAllocator* pCommandAllocator,
	ID3D12GraphicsCommandList* pCommandList, BvFrameDataD3D12* pFrameData)
	: m_pDevice(pDevice), m_pCommandAllocator(pCommandAllocator), m_CommandList(pCommandList), m_pFrameData(pFrameData)
{
	m_CommandList.As(&m_CommandListEx);
}


BvCommandListD3D12::~BvCommandListD3D12()
{
}


void BvCommandListD3D12::Reset()
{
	m_SwapChains.Clear();
	m_pGraphicsPipeline = nullptr;
	m_pComputePipeline = nullptr;
	m_pShaderResourceLayout = nullptr;
	m_DescriptorSets.Clear();

	m_CommandList->Reset(m_pCommandAllocator, nullptr);

	m_CurrentState = State::kRecording;
}


void BvCommandListD3D12::Close()
{
	ResetRenderTargets();

	m_CommandList->Close();
}


void BvCommandListD3D12::BeginRenderPass(const IBvRenderPass* pRenderPass, u32 renderPassTargetCount, const RenderPassTargetDesc* pRenderPassTargets)
{
	BV_ASSERT(pRenderPass != nullptr, "Invalid render pass");
	BV_ASSERT(pRenderPassTargets != nullptr, "No render / depth targets");
	BV_ASSERT(renderPassTargetCount <= kMaxRenderTargetsWithDepth, "Too many render targets");

	ResetRenderTargets();

	BvFixedVector<D3D12ClearValue, kMaxRenderTargetsWithDepth> clearValues;
	D3D12Rect2D renderArea{};
	D3D12RenderPass renderPass = TO_D3D12(pRenderPass)->GetHandle();
	FramebufferDesc fbDesc;
	fbDesc.m_RenderPass = renderPass;
	fbDesc.m_Width = kU32Max;
	fbDesc.m_Height = kU32Max;
	fbDesc.m_LayerCount = kU32Max;
	for (auto i = 0u; i < renderPassTargetCount; ++i)
	{
		auto viewD3D12 = TO_D3D12(pRenderPassTargets[i].m_pView);
		if (auto pSwapChain = TO_D3D12(viewD3D12->GetDesc().m_pTexture)->GetSwapChain())
		{
			AddSwapChain(pSwapChain);
		}
		auto& viewDesc = viewD3D12->GetDesc();
		auto& desc = viewDesc.m_pTexture->GetDesc();
		fbDesc.m_Width = std::min(desc.m_Size.width, fbDesc.m_Width);
		fbDesc.m_Height = std::min(desc.m_Size.height, fbDesc.m_Height);
		fbDesc.m_LayerCount = std::min((viewDesc.m_SubresourceDesc.layerCount == kU32Max ? desc.m_ArraySize : viewDesc.m_SubresourceDesc.layerCount)
			- viewDesc.m_SubresourceDesc.firstLayer, fbDesc.m_LayerCount);

		fbDesc.m_Views.EmplaceBack(viewD3D12->GetHandle());
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

	D3D12SubpassBeginInfo spBeginInfo{ D3D12_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO, nullptr, D3D12_SUBPASS_CONTENTS_INLINE };
	D3D12RenderPassBeginInfo rpBeginInfo{ D3D12_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	//rpBeginInfo.pNext = nullptr;
	rpBeginInfo.renderPass = renderPass;
	rpBeginInfo.framebuffer = m_pFrameData->GetFramebuffer(fbDesc);
	rpBeginInfo.renderArea = renderArea;
	rpBeginInfo.clearValueCount = (u32)clearValues.Size();
	rpBeginInfo.pClearValues = clearValues.Data();

	D3D12CmdBeginRenderPass2(m_CommandBuffer, &rpBeginInfo, &spBeginInfo);

	m_CurrentState = State::kRenderPass;
}


void BvCommandListD3D12::NextSubpass()
{
}


void BvCommandListD3D12::EndRenderPass()
{
	BV_ASSERT(m_CurrentState == State::kRenderPass, "Command buffer not in render pass");

	m_CurrentState = State::kRecording;
}


void BvCommandListD3D12::SetRenderTargets(u32 renderTargetCount, const RenderTargetDesc* pRenderTargets, u32 multiviewCount)
{
	m_CommandList->OMSetRenderTargets();
	ResetRenderTargets();

	if (!renderTargetCount)
	{
		return;
	}

	BV_ASSERT(renderTargetCount <= kMaxRenderTargetsWithDepth, "Too many render targets");

	D3D12Extent2D renderArea{ kU32Max, kU32Max };

	D3D12RenderingAttachmentInfo colorAttachments[kMaxRenderTargets]{};
	D3D12RenderingAttachmentInfo depthAttachment{ D3D12_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
	D3D12RenderingAttachmentInfo stencilAttachment{ D3D12_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
	D3D12RenderingFragmentShadingRateAttachmentInfoKHR shadingRateAttachment{ D3D12_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR };
	D3D12RenderingInfo renderingInfo{ D3D12_STRUCTURE_TYPE_RENDERING_INFO };

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
		auto pView = TO_D3D12(renderTarget.m_pView);
		auto pTexture = TO_D3D12(renderTarget.m_pView->GetDesc().m_pTexture);
		auto& desc = renderTarget.m_pView->GetDesc().m_pTexture->GetDesc();
		auto& viewDesc = renderTarget.m_pView->GetDesc();

		layerCount = std::min(layerCount, (viewDesc.m_SubresourceDesc.layerCount == kU32Max ? desc.m_ArraySize : viewDesc.m_SubresourceDesc.layerCount)
			- viewDesc.m_SubresourceDesc.firstLayer);
		renderArea.width = std::min(renderArea.width, desc.m_Size.width);
		renderArea.height = std::min(renderArea.height, desc.m_Size.height);

		auto loadOp = GetD3D12AttachmentLoadOp(renderTarget.m_LoadOp);
		auto storeOp = GetD3D12AttachmentStoreOp(renderTarget.m_StoreOp);

		auto aspectFlags = GetD3D12FormatMap(viewDesc.m_Format).aspectFlags;
		bool isColorTarget = (aspectFlags & D3D12_IMAGE_ASPECT_COLOR_BIT) != 0;
		bool isDepthTarget = (aspectFlags & D3D12_IMAGE_ASPECT_DEPTH_BIT) != 0;
		bool isStencilTarget = (aspectFlags & D3D12_IMAGE_ASPECT_STENCIL_BIT) != 0;
		bool isShadingRate = renderTarget.m_State == ResourceState::kShadingRate
			|| renderTarget.m_ShadingRateTexelSizes[0] != 0 || renderTarget.m_ShadingRateTexelSizes[1] != 0;
		bool isResolveImage = renderTarget.m_ResolveMode != ResolveMode::kNone;

		if (isShadingRate)
		{
			shadingRateAttachment.imageView = pView->GetHandle();
			shadingRateAttachment.imageLayout = D3D12_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;
			shadingRateAttachment.shadingRateAttachmentTexelSize.width = renderTarget.m_ShadingRateTexelSizes[0];
			shadingRateAttachment.shadingRateAttachmentTexelSize.height = renderTarget.m_ShadingRateTexelSizes[1];
			renderingInfo.pNext = &shadingRateAttachment;
		}
		else if (isColorTarget)
		{
			if (!isResolveImage)
			{
				colorAttachments[colorAttachmentCount].sType = D3D12_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
				colorAttachments[colorAttachmentCount].imageView = pView->GetHandle();
				colorAttachments[colorAttachmentCount].imageLayout = D3D12_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				colorAttachments[colorAttachmentCount].loadOp = loadOp;
				colorAttachments[colorAttachmentCount].storeOp = storeOp;
				if (loadOp == D3D12AttachmentLoadOp::D3D12_ATTACHMENT_LOAD_OP_CLEAR)
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
				colorAttachments[resolveCount].resolveImageLayout = D3D12_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				colorAttachments[resolveCount].resolveMode = GetD3D12ResolveMode(renderTarget.m_ResolveMode);
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
				depthAttachment.sType = D3D12_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
				depthAttachment.imageView = pView->GetHandle();
				depthAttachment.imageLayout = renderTarget.m_State == ResourceState::kDepthStencilWrite ? D3D12_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
					: D3D12_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
				depthAttachment.loadOp = loadOp;
				depthAttachment.storeOp = storeOp;
				if (loadOp == D3D12AttachmentLoadOp::D3D12_ATTACHMENT_LOAD_OP_CLEAR)
				{
					depthAttachment.clearValue.depthStencil.depth = renderTarget.m_ClearValues.depth;
				}
				hasDepth = true;

				if (isStencilTarget)
				{
					stencilAttachment.sType = D3D12_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
					stencilAttachment.imageView = pView->GetHandle();
					stencilAttachment.imageLayout = renderTarget.m_State == ResourceState::kDepthStencilWrite ? D3D12_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL
						: D3D12_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
					stencilAttachment.loadOp = loadOp;
					stencilAttachment.storeOp = storeOp;
					if (loadOp == D3D12AttachmentLoadOp::D3D12_ATTACHMENT_LOAD_OP_CLEAR)
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
				depthAttachment.resolveImageLayout = D3D12_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
				depthAttachment.resolveMode = GetD3D12ResolveMode(renderTarget.m_ResolveMode);

				if (isStencilTarget)
				{
					stencilAttachment.resolveImageView = depthAttachment.resolveImageView;
					stencilAttachment.resolveImageLayout = D3D12_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
					stencilAttachment.resolveMode = depthAttachment.resolveMode;
				}
			}
		}

		auto textureHandle = pTexture->GetHandle();
		if (renderTarget.m_StateBefore != renderTarget.m_State)
		{
			auto& barrier = m_PreRenderBarriers.EmplaceBack(D3D12ImageMemoryBarrier2{ D3D12_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 });
			//barrier.pNext = nullptr;
			barrier.image = textureHandle;
			barrier.subresourceRange.aspectMask = aspectFlags;
			barrier.subresourceRange.baseMipLevel = viewDesc.m_SubresourceDesc.firstMip;
			barrier.subresourceRange.levelCount = viewDesc.m_SubresourceDesc.mipCount;
			barrier.subresourceRange.baseArrayLayer = viewDesc.m_SubresourceDesc.firstLayer;
			barrier.subresourceRange.layerCount = viewDesc.m_SubresourceDesc.layerCount;

			barrier.oldLayout = GetD3D12ImageLayout(renderTarget.m_StateBefore, isDepthTarget || isStencilTarget);
			barrier.newLayout = GetD3D12ImageLayout(renderTarget.m_State, isDepthTarget || isStencilTarget);

			barrier.srcAccessMask = GetD3D12AccessFlags(renderTarget.m_StateBefore);
			barrier.dstAccessMask = GetD3D12AccessFlags(renderTarget.m_State);

			barrier.srcStageMask = GetD3D12PipelineStageFlags(barrier.srcAccessMask);
			barrier.dstStageMask = GetD3D12PipelineStageFlags(barrier.dstAccessMask);

			barrier.srcQueueFamilyIndex = D3D12_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = D3D12_QUEUE_FAMILY_IGNORED;
		}
		if (renderTarget.m_State != renderTarget.m_StateAfter)
		{
			auto& barrier = m_PostRenderBarriers.EmplaceBack(D3D12ImageMemoryBarrier2{ D3D12_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 });
			//barrier.pNext = nullptr;
			barrier.image = textureHandle;
			barrier.subresourceRange.aspectMask = aspectFlags;
			barrier.subresourceRange.baseMipLevel = viewDesc.m_SubresourceDesc.firstMip;
			barrier.subresourceRange.levelCount = viewDesc.m_SubresourceDesc.mipCount;
			barrier.subresourceRange.baseArrayLayer = viewDesc.m_SubresourceDesc.firstLayer;
			barrier.subresourceRange.layerCount = viewDesc.m_SubresourceDesc.layerCount;

			barrier.oldLayout = GetD3D12ImageLayout(renderTarget.m_State, isDepthTarget || isStencilTarget);
			barrier.newLayout = GetD3D12ImageLayout(renderTarget.m_StateAfter, isDepthTarget || isStencilTarget);

			barrier.srcAccessMask = GetD3D12AccessFlags(renderTarget.m_State);
			barrier.dstAccessMask = GetD3D12AccessFlags(renderTarget.m_StateAfter);

			barrier.srcStageMask = GetD3D12PipelineStageFlags(barrier.srcAccessMask);
			barrier.dstStageMask = GetD3D12PipelineStageFlags(barrier.dstAccessMask);

			barrier.srcQueueFamilyIndex = D3D12_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = D3D12_QUEUE_FAMILY_IGNORED;
		}
	}

	if (m_PreRenderBarriers.Size() > 0)
	{
		D3D12DependencyInfo dependencyInfo{ D3D12_STRUCTURE_TYPE_DEPENDENCY_INFO };
		dependencyInfo.imageMemoryBarrierCount = static_cast<u32>(m_PreRenderBarriers.Size());
		dependencyInfo.pImageMemoryBarriers = m_PreRenderBarriers.Data();

		D3D12CmdPipelineBarrier2(m_CommandBuffer, &dependencyInfo);

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
	if (multiviewCount > 1)
	{
		renderingInfo.viewMask = (1 << multiviewCount) - 1;
	}

	D3D12CmdBeginRendering(m_CommandBuffer, &renderingInfo);

	m_CurrentState = State::kRenderTarget;
}


void BvCommandListD3D12::SetViewports(u32 viewportCount, const Viewport* pViewports)
{
	constexpr u32 kMaxViewports = 16;
	BV_ASSERT(viewportCount <= kMaxViewports, "Viewport count greater than limit");
	D3D12_VIEWPORT vps[kMaxViewports];
	for (auto i = 0u; i < viewportCount; ++i)
	{
		vps[i].TopLeftX = pViewports[i].x;
		vps[i].TopLeftY = pViewports[i].y;
		vps[i].Width = pViewports[i].width;
		vps[i].Height = pViewports[i].height;
		vps[i].MinDepth = pViewports[i].minDepth;
		vps[i].MaxDepth = pViewports[i].maxDepth;
	}

	m_CommandList->RSSetViewports(viewportCount, vps);
}


void BvCommandListD3D12::SetScissors(u32 scissorCount, const Rect* pScissors)
{
	constexpr u32 kMaxScissors = 16;
	BV_ASSERT(scissorCount <= kMaxScissors, "Scissor count greater than limit");
	D3D12_RECT scissors[kMaxScissors];
	for (auto i = 0u; i < scissorCount; ++i)
	{
		scissors[i].left = pScissors[i].x;
		scissors[i].top = pScissors[i].y;
		scissors[i].right = pScissors[i].width;
		scissors[i].bottom = pScissors[i].height;
	}

	m_CommandList->RSSetScissorRects(scissorCount, scissors);
}


void BvCommandListD3D12::SetGraphicsPipeline(const IBvGraphicsPipelineState* pPipeline)
{
	auto pPSO = TO_D3D12(pPipeline);

	m_pComputePipeline = nullptr;
	m_pRayTracingPipeline = nullptr;
	m_pGraphicsPipeline = pPSO;
	m_pShaderResourceLayout = TO_D3D12(m_pGraphicsPipeline->GetDesc().m_pShaderResourceLayout);
	
	m_CommandList->SetPipelineState(m_pGraphicsPipeline->GetHandle());
}


void BvCommandListD3D12::SetComputePipeline(const IBvComputePipelineState* pPipeline)
{
	m_pGraphicsPipeline = nullptr;
	m_pRayTracingPipeline = nullptr;
	m_pComputePipeline = TO_D3D12(pPipeline);
	m_pShaderResourceLayout = TO_D3D12(m_pComputePipeline->GetDesc().m_pShaderResourceLayout);

	m_CommandList->SetPipelineState(m_pComputePipeline->GetHandle());
}


void BvCommandListD3D12::SetRayTracingPipeline(const IBvRayTracingPipelineState* pPipeline)
{
	m_pGraphicsPipeline = nullptr;
	m_pComputePipeline = nullptr;
	m_pRayTracingPipeline = TO_D3D12(pPipeline);
	m_pShaderResourceLayout = TO_D3D12(m_pRayTracingPipeline->GetDesc().m_pShaderResourceLayout);
	
	m_CommandListEx->SetPipelineState1(m_pRayTracingPipeline->GetHandle());
}


void BvCommandListD3D12::SetShaderResourceParams(u32 resourceParamsCount, IBvShaderResourceParams* const* ppResourceParams, u32 startIndex)
{
	if (startIndex + resourceParamsCount > m_DescriptorSets.Size())
	{
		m_DescriptorSets.Resize(startIndex + resourceParamsCount, D3D12_NULL_HANDLE);
	}

	for (auto i = 0; i < resourceParamsCount; ++i)
	{
		auto pSRP = TO_D3D12(ppResourceParams[i]);
		m_DescriptorSets[i] = pSRP ? pSRP->GetHandle() : D3D12_NULL_HANDLE;
	}

	D3D12CmdBindDescriptorSets(m_CommandBuffer, m_PipelineBindPoint, m_pShaderResourceLayout->GetPipelineLayoutHandle(), startIndex, resourceParamsCount, m_DescriptorSets.Data() + startIndex, 0, nullptr);
}


void BvCommandListD3D12::SetConstantBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(D3D12_DESCRIPTOR_TYPE_UNIFORM_BUFFER, TO_D3D12(ppResources[i]), set, binding, startIndex + i);
	}
}


void BvCommandListD3D12::SetStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(D3D12_DESCRIPTOR_TYPE_STORAGE_BUFFER, TO_D3D12(ppResources[i]), set, binding, startIndex + i);
	}
}


void BvCommandListD3D12::SetRWStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(D3D12_DESCRIPTOR_TYPE_STORAGE_BUFFER, TO_D3D12(ppResources[i]), set, binding, startIndex + i);
	}
}

void BvCommandListD3D12::SetDynamicConstantBuffers(u32 count, const IBvBufferView* const* ppResources, const u32* pOffsets, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(D3D12_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, TO_D3D12(ppResources[i]), set, binding, startIndex + i, pOffsets[i]);
	}
}


void BvCommandListD3D12::SetDynamicStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, const u32* pOffsets, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(D3D12_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, TO_D3D12(ppResources[i]), set, binding, startIndex + i, pOffsets[i]);
	}
}


void BvCommandListD3D12::SetDynamicRWStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, const u32* pOffsets, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(D3D12_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, TO_D3D12(ppResources[i]), set, binding, startIndex + i, pOffsets[i]);
	}
}


void BvCommandListD3D12::SetFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(D3D12_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, TO_D3D12(ppResources[i]), set, binding, startIndex + i);
	}
}


void BvCommandListD3D12::SetRWFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(D3D12_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, TO_D3D12(ppResources[i]), set, binding, startIndex + i);
	}
}


void BvCommandListD3D12::SetTextures(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(D3D12_DESCRIPTOR_TYPE_SAMPLED_IMAGE, TO_D3D12(ppResources[i]), set, binding, startIndex + i);
	}
}


void BvCommandListD3D12::SetRWTextures(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(D3D12_DESCRIPTOR_TYPE_STORAGE_IMAGE, TO_D3D12(ppResources[i]), set, binding, startIndex + i);
	}
}


void BvCommandListD3D12::SetSamplers(u32 count, const IBvSampler* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(D3D12_DESCRIPTOR_TYPE_SAMPLER, TO_D3D12(ppResources[i]), set, binding, startIndex + i);
	}
}


void BvCommandListD3D12::SetInputAttachments(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(D3D12_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, TO_D3D12(ppResources[i]), set, binding, startIndex + i);
	}
}


void BvCommandListD3D12::SetAccelerationStructures(u32 count, const IBvAccelerationStructure* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(D3D12_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, TO_D3D12(ppResources[i]), set, binding, startIndex + i);
	}
}


void BvCommandListD3D12::SetShaderConstants(u32 size, const void* pData, u32 binding, u32 set)
{
	if (auto pConstant = m_pShaderResourceLayout->GetPushConstantData(size, binding, set))
	{
		D3D12CmdPushConstants(m_CommandBuffer, m_pShaderResourceLayout->GetPipelineLayoutHandle(), pConstant->m_ShaderStages, pConstant->m_Offset, size, pData);
	}
}


void BvCommandListD3D12::SetVertexBufferViews(u32 vertexBufferCount, const VertexBufferView* pVertexBufferViews, u32 firstBinding)
{
	BV_ASSERT(vertexBufferCount <= kMaxVertexBuffers, "Vertex buffer view count greater than limit");

	D3D12_VERTEX_BUFFER_VIEW vertexBuffers[kMaxVertexBuffers];
	for (auto i = 0u; i < vertexBufferCount; i++)
	{
		if (pVertexBufferViews[i].m_pBuffer)
		{
			vertexBuffers[i].BufferLocation = TO_D3D12(pVertexBufferViews[i].m_pBuffer)->GetDeviceAddress() + pVertexBufferViews[i].m_Offset;
			vertexBuffers[i].SizeInBytes = pVertexBufferViews[i].m_pBuffer->GetDesc().m_Size - pVertexBufferViews[i].m_Offset;
			vertexBuffers[i].StrideInBytes = pVertexBufferViews[i].m_Stride;
		}
	}
	
	m_CommandList->IASetVertexBuffers(firstBinding, vertexBufferCount, vertexBuffers);
}


void BvCommandListD3D12::SetIndexBufferView(const IndexBufferView& indexBufferView)
{
	D3D12_INDEX_BUFFER_VIEW ibv{ indexBufferView.m_pBuffer->GetDeviceAddress() + indexBufferView.m_Offset,
		indexBufferView.m_pBuffer->GetDesc().m_Size - indexBufferView.m_Offset, };
	m_CommandList->IASetIndexBuffer(&ibv);
}


void BvCommandListD3D12::SetDepthBounds(float min, float max)
{
	m_CommandListEx->OMSetDepthBounds(min, max);
}


void BvCommandListD3D12::SetStencilRef(u32 stencilRef)
{
	m_CommandList->OMSetStencilRef(stencilRef);
}


void BvCommandListD3D12::SetBlendConstants(const float(pColors[4]))
{
	m_CommandList->OMSetBlendFactor(pColors);
}


void BvCommandListD3D12::SetShadingRate(ShadingRateDimensions dimensions, ShadingRateCombinerOp(pCombinerOps[2]))
{
	BV_ASSERT(false, "Not done yet");
}


void BvCommandListD3D12::Draw(const DrawCommandArgs& args)
{
	FlushDescriptorSets();

	m_CommandList->DrawInstanced(args.m_VertexCount, args.m_InstanceCount, args.m_FirstVertex, args.m_FirstInstance);
}


void BvCommandListD3D12::DrawIndexed(const DrawIndexedCommandArgs& args)
{
	FlushDescriptorSets();

	m_CommandList->DrawIndexedInstanced(args.m_IndexCount, args.m_InstanceCount, args.m_FirstIndex, args.m_VertexOffset, args.m_FirstInstance);
}


void BvCommandListD3D12::Dispatch(const DispatchCommandArgs& args)
{
	FlushDescriptorSets();

	m_CommandList->Dispatch(args.m_ThreadGroupCountX, args.m_ThreadGroupCountY, args.m_ThreadGroupCountZ);
}


void BvCommandListD3D12::DispatchMesh(const DispatchMeshCommandArgs& args)
{
	FlushDescriptorSets();

	BeginMeshQueries();

	m_CommandListEx->DispatchMesh(args.m_ThreadGroupCountX, args.m_ThreadGroupCountY, args.m_ThreadGroupCountZ);
}


void BvCommandListD3D12::DrawIndirect(const IBvBuffer* pBuffer, u32 drawCount, u64 offset)
{
	FlushDescriptorSets();

	auto pBufferD3D12 = TO_D3D12(pBuffer)->GetHandle();
	D3D12CmdDrawIndirect(m_CommandBuffer, pBufferD3D12, offset, drawCount, (u32)sizeof(D3D12DrawIndirectCommand));
}


void BvCommandListD3D12::DrawIndexedIndirect(const IBvBuffer* pBuffer, u32 drawCount, u64 offset)
{
	FlushDescriptorSets();

	auto pBufferD3D12 = TO_D3D12(pBuffer)->GetHandle();
	D3D12CmdDrawIndexedIndirect(m_CommandBuffer, pBufferD3D12, offset, drawCount, (u32)sizeof(D3D12DrawIndexedIndirectCommand));
}


void BvCommandListD3D12::DispatchIndirect(const IBvBuffer* pBuffer, u64 offset)
{
	FlushDescriptorSets();

	auto pBufferD3D12 = TO_D3D12(pBuffer)->GetHandle();
	D3D12CmdDispatchIndirect(m_CommandBuffer, pBufferD3D12, offset);
}


void BvCommandListD3D12::DispatchMeshIndirect(const IBvBuffer* pBuffer, u64 offset)
{
	FlushDescriptorSets();

	BeginMeshQueries();

	D3D12CmdDrawMeshTasksIndirectEXT(m_CommandBuffer, TO_D3D12(pBuffer)->GetHandle(), offset, 1, sizeof(D3D12DispatchIndirectCommand));
}


void BvCommandListD3D12::DispatchMeshIndirectCount(const IBvBuffer* pBuffer, u64 offset, const IBvBuffer* pCountBuffer, u64 countOffset, u32 maxCount)
{
	FlushDescriptorSets();

	BeginMeshQueries();

	D3D12CmdDrawMeshTasksIndirectCountEXT(m_CommandBuffer, TO_D3D12(pBuffer)->GetHandle(), offset, TO_D3D12(pCountBuffer)->GetHandle(), countOffset,
		maxCount, sizeof(D3D12DispatchIndirectCommand));
}


void BvCommandListD3D12::CopyBuffer(const IBvBuffer* pSrcBuffer, IBvBuffer* pDstBuffer)
{
	D3D12BufferCopy region{};
	region.size = std::min(pSrcBuffer->GetDesc().m_Size, pDstBuffer->GetDesc().m_Size);

	auto pSrc = TO_D3D12(pSrcBuffer);
	auto pDst = TO_D3D12(pDstBuffer);

	CopyBuffer(pSrc->GetHandle(), pDst->GetHandle(), region);
}


void BvCommandListD3D12::CopyBuffer(const IBvBuffer* pSrcBuffer, IBvBuffer* pDstBuffer, const BufferCopyDesc& copyDesc)
{
	D3D12BufferCopy region{};
	region.srcOffset = copyDesc.m_SrcOffset;
	region.dstOffset = copyDesc.m_DstOffset;
	region.size = std::min(copyDesc.m_SrcSize, pSrcBuffer->GetDesc().m_Size);

	auto pSrc = TO_D3D12(pSrcBuffer);
	auto pDst = TO_D3D12(pDstBuffer);

	CopyBuffer(pSrc->GetHandle(), pDst->GetHandle(), region);
}


void BvCommandListD3D12::CopyTexture(const IBvTexture* pSrcTexture, IBvTexture* pDstTexture)
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
			D3D12ImageCopy& imageCopyRegion = m_ImageCopyRegions.EmplaceBack();
			imageCopyRegion.srcOffset = { 0, 0, 0 };
			imageCopyRegion.dstOffset = { 0, 0, 0 };

			imageCopyRegion.srcSubresource.aspectMask = GetD3D12FormatMap(srcDesc.m_Format).aspectFlags;
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

	auto pSrc = TO_D3D12(pSrcTexture);
	auto pDst = TO_D3D12(pDstTexture);
	if (auto pSwapChain = pDst->GetSwapChain())
	{
		AddSwapChain(pSwapChain);
	}

	D3D12CmdCopyImage(m_CommandBuffer, pSrc->GetHandle(), D3D12ImageLayout::D3D12_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		pDst->GetHandle(), D3D12ImageLayout::D3D12_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		(u32)m_ImageCopyRegions.Size(), m_ImageCopyRegions.Data());

	m_ImageCopyRegions.Clear();
}


void BvCommandListD3D12::CopyTexture(const IBvTexture* pSrcTexture, IBvTexture* pDstTexture, const TextureCopyDesc& copyDesc)
{
	ResetRenderTargets();

	D3D12ImageCopy imageCopyRegion{};
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

	imageCopyRegion.srcSubresource.aspectMask = GetD3D12FormatMap(srcDesc.m_Format).aspectFlags;
	imageCopyRegion.srcSubresource.mipLevel = copyDesc.m_SrcMip;
	imageCopyRegion.srcSubresource.baseArrayLayer = copyDesc.m_SrcLayer;
	imageCopyRegion.srcSubresource.layerCount = 1;

	imageCopyRegion.dstSubresource.aspectMask = GetD3D12FormatMap(dstDesc.m_Format).aspectFlags;
	imageCopyRegion.dstSubresource.mipLevel = copyDesc.m_DstMip;
	imageCopyRegion.dstSubresource.baseArrayLayer = copyDesc.m_DstLayer;
	imageCopyRegion.dstSubresource.layerCount = 1;

	imageCopyRegion.extent =
	{
		std::min(copyDesc.m_Size.width, std::min(srcDesc.m_Size.width, dstDesc.m_Size.width)),
		std::min(copyDesc.m_Size.height, std::min(srcDesc.m_Size.height, dstDesc.m_Size.height)),
		std::min(copyDesc.m_Size.depth, std::min(srcDesc.m_Size.depth, dstDesc.m_Size.depth))
	};

	auto pSrc = TO_D3D12(pSrcTexture);
	auto pDst = TO_D3D12(pDstTexture);
	if (auto pSwapChain = pDst->GetSwapChain())
	{
		AddSwapChain(pSwapChain);
	}

	D3D12CmdCopyImage(m_CommandBuffer, pSrc->GetHandle(), D3D12ImageLayout::D3D12_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		pDst->GetHandle(), D3D12ImageLayout::D3D12_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1u, &imageCopyRegion);
}


void BvCommandListD3D12::CopyBufferToTexture(const IBvBuffer* pSrcBuffer, IBvTexture* pDstTexture, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs)
{
	auto pSrc = TO_D3D12(pSrcBuffer);
	auto pDst = TO_D3D12(pDstTexture);

	m_BufferImageCopyRegions.Resize(copyCount, {});
	for (auto i = 0u; i < copyCount; ++i)
	{
		m_BufferImageCopyRegions[i].bufferOffset = pCopyDescs[i].m_BufferOffset;
		//m_BufferImageCopyRegions[i].bufferImageHeight = 0;
		//m_BufferImageCopyRegions[i].bufferRowLength = 0;
		m_BufferImageCopyRegions[i].imageExtent = { pCopyDescs[i].m_TextureSize.width, pCopyDescs[i].m_TextureSize.height, pCopyDescs[i].m_TextureSize.depth };
		m_BufferImageCopyRegions[i].imageOffset = { pCopyDescs[i].m_TextureOffset.x, pCopyDescs[i].m_TextureOffset.y, pCopyDescs[i].m_TextureOffset.z };
		m_BufferImageCopyRegions[i].imageSubresource.aspectMask = GetD3D12FormatMap(pDst->GetDesc().m_Format).aspectFlags;
		m_BufferImageCopyRegions[i].imageSubresource.mipLevel = pCopyDescs[i].m_Mip;
		m_BufferImageCopyRegions[i].imageSubresource.baseArrayLayer = pCopyDescs[i].m_Layer;
		m_BufferImageCopyRegions[i].imageSubresource.layerCount = 1;
	}

	CopyBufferToTexture(pSrc->GetHandle(), pDst->GetHandle(), copyCount, m_BufferImageCopyRegions.Data());

	m_BufferImageCopyRegions.Clear();
}


void BvCommandListD3D12::CopyTextureToBuffer(const IBvTexture* pSrcTexture, IBvBuffer* pDstBuffer, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs)
{
	auto pSrc = TO_D3D12(pSrcTexture);
	auto pDst = TO_D3D12(pDstBuffer);

	m_BufferImageCopyRegions.Resize(copyCount, {});
	for (auto i = 0u; i < copyCount; ++i)
	{
		m_BufferImageCopyRegions[i].bufferOffset = pCopyDescs[i].m_BufferOffset;
		//m_BufferImageCopyRegions[i].bufferImageHeight = 0;
		//m_BufferImageCopyRegions[i].bufferRowLength = 0;
		m_BufferImageCopyRegions[i].imageExtent = { pCopyDescs[i].m_TextureSize.width, pCopyDescs[i].m_TextureSize.height, pCopyDescs[i].m_TextureSize.depth };
		m_BufferImageCopyRegions[i].imageOffset = { pCopyDescs[i].m_TextureOffset.x, pCopyDescs[i].m_TextureOffset.y, pCopyDescs[i].m_TextureOffset.z };
		m_BufferImageCopyRegions[i].imageSubresource.aspectMask = GetD3D12FormatMap(pSrc->GetDesc().m_Format).aspectFlags;
		m_BufferImageCopyRegions[i].imageSubresource.mipLevel = pCopyDescs[i].m_Mip;
		m_BufferImageCopyRegions[i].imageSubresource.baseArrayLayer = pCopyDescs[i].m_Layer;
		m_BufferImageCopyRegions[i].imageSubresource.layerCount = 1;
	}

	CopyTextureToBuffer(pSrc->GetHandle(), pDst->GetHandle(), copyCount, m_BufferImageCopyRegions.Data());

	m_BufferImageCopyRegions.Clear();
}


void BvCommandListD3D12::ResourceBarrier(u32 bufferBarrierCount, const D3D12BufferMemoryBarrier2* pBufferBarriers,
	u32 imageBarrierCount, const D3D12ImageMemoryBarrier2* pImageBarriers, u32 memoryBarrierCount, const D3D12MemoryBarrier2* pMemoryBarriers,
	D3D12DependencyFlags dependencyFlags)
{
	ResetRenderTargets();

	D3D12DependencyInfo di{ D3D12_STRUCTURE_TYPE_DEPENDENCY_INFO };
	di.dependencyFlags = dependencyFlags;
	di.bufferMemoryBarrierCount = bufferBarrierCount;
	di.pBufferMemoryBarriers = pBufferBarriers;
	di.imageMemoryBarrierCount = imageBarrierCount;
	di.pImageMemoryBarriers = pImageBarriers;
	di.memoryBarrierCount = memoryBarrierCount;
	di.pMemoryBarriers = pMemoryBarriers;

	D3D12CmdPipelineBarrier2(m_CommandBuffer, &di);
}


void BvCommandListD3D12::ResourceBarrier(u32 barrierCount, const ResourceBarrierDesc* pBarriers)
{
	D3D12DependencyFlags dependencyFlags = 0;
	for (auto i = 0u; i < barrierCount; i++)
	{
		if (pBarriers[i].m_Type == ResourceBarrierDesc::Type::kMemory)
		{
			auto& barrier = m_MemoryBarriers.EmplaceBack(D3D12MemoryBarrier2{ D3D12_STRUCTURE_TYPE_MEMORY_BARRIER_2 });
			//barrier.pNext = nullptr;
			barrier.srcAccessMask = pBarriers[i].m_SrcAccess == ResourceAccess::kAuto ?
				GetD3D12AccessFlags(pBarriers[i].m_SrcState) : GetD3D12AccessFlags(pBarriers[i].m_SrcAccess);
			barrier.srcStageMask |= pBarriers[i].m_SrcPipelineStage == PipelineStage::kAuto ?
				GetD3D12PipelineStageFlags(barrier.srcAccessMask) : GetD3D12PipelineStageFlags(pBarriers[i].m_SrcPipelineStage);

			barrier.dstAccessMask = pBarriers[i].m_DstAccess == ResourceAccess::kAuto ?
				GetD3D12AccessFlags(pBarriers[i].m_DstState) : GetD3D12AccessFlags(pBarriers[i].m_DstAccess);
			barrier.dstStageMask |= pBarriers[i].m_DstPipelineStage == PipelineStage::kAuto ?
				GetD3D12PipelineStageFlags(barrier.dstAccessMask) : GetD3D12PipelineStageFlags(pBarriers[i].m_DstPipelineStage);
		}
		else if (pBarriers[i].m_pBuffer)
		{
			auto buffer = TO_D3D12(pBarriers[i].m_pBuffer)->GetHandle();
			auto& barrier = m_BufferBarriers.EmplaceBack(D3D12BufferMemoryBarrier2{ D3D12_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2 });
			//barrier.pNext = nullptr;
			barrier.buffer = buffer;
			barrier.size = D3D12_WHOLE_SIZE;
			//barrier.offset = 0;

			if (pBarriers[i].m_Type == ResourceBarrierDesc::Type::kStateTransition)
			{
				barrier.srcQueueFamilyIndex = D3D12_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = D3D12_QUEUE_FAMILY_IGNORED;
			}
			else
			{
				auto pSrcContext = TO_D3D12(pBarriers[i].m_pSrcContext);
				auto pDstContext = TO_D3D12(pBarriers[i].m_pDstContext);
				BV_ASSERT(pSrcContext != pDstContext, "Command Types must be different for transition acquire/release");
				barrier.srcQueueFamilyIndex = pSrcContext->GetGroupIndex();
				barrier.dstQueueFamilyIndex = pDstContext->GetGroupIndex();

				dependencyFlags |= D3D12_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR;
			}

			barrier.srcAccessMask = pBarriers[i].m_SrcAccess == ResourceAccess::kAuto ?
				GetD3D12AccessFlags(pBarriers[i].m_SrcState) : GetD3D12AccessFlags(pBarriers[i].m_SrcAccess);
			barrier.srcStageMask |= pBarriers[i].m_SrcPipelineStage == PipelineStage::kAuto ?
				GetD3D12PipelineStageFlags(barrier.srcAccessMask) : GetD3D12PipelineStageFlags(pBarriers[i].m_SrcPipelineStage);

			barrier.dstAccessMask = pBarriers[i].m_DstAccess == ResourceAccess::kAuto ?
				GetD3D12AccessFlags(pBarriers[i].m_DstState) : GetD3D12AccessFlags(pBarriers[i].m_DstAccess);
			barrier.dstStageMask |= pBarriers[i].m_DstPipelineStage == PipelineStage::kAuto ?
				GetD3D12PipelineStageFlags(barrier.dstAccessMask) : GetD3D12PipelineStageFlags(pBarriers[i].m_DstPipelineStage);
		}
		else if (pBarriers[i].m_pTexture)
		{
			auto& barrier = m_ImageBarriers.EmplaceBack(D3D12ImageMemoryBarrier2{ D3D12_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 });
			//barrier.pNext = nullptr;
			barrier.image = TO_D3D12(pBarriers[i].m_pTexture)->GetHandle();
			barrier.subresourceRange.aspectMask = GetD3D12FormatMap(pBarriers[i].m_pTexture->GetDesc().m_Format).aspectFlags;
			barrier.subresourceRange.baseMipLevel = pBarriers[i].m_Subresource.firstMip;
			barrier.subresourceRange.levelCount = pBarriers[i].m_Subresource.mipCount;
			barrier.subresourceRange.baseArrayLayer = pBarriers[i].m_Subresource.firstLayer;
			barrier.subresourceRange.layerCount = pBarriers[i].m_Subresource.layerCount;

			if (pBarriers[i].m_Type == ResourceBarrierDesc::Type::kStateTransition)
			{
				barrier.srcQueueFamilyIndex = D3D12_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = D3D12_QUEUE_FAMILY_IGNORED;
			}
			else
			{
				auto pSrcContext = TO_D3D12(pBarriers[i].m_pSrcContext);
				auto pDstContext = TO_D3D12(pBarriers[i].m_pDstContext);
				BV_ASSERT(pSrcContext != pDstContext, "Command Types must be different for transition acquire/release");
				barrier.srcQueueFamilyIndex = pSrcContext->GetGroupIndex();
				barrier.dstQueueFamilyIndex = pDstContext->GetGroupIndex();

				dependencyFlags |= D3D12_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR;
			}

			barrier.oldLayout = GetD3D12ImageLayout(pBarriers[i].m_SrcState);
			barrier.newLayout = GetD3D12ImageLayout(pBarriers[i].m_DstState);

			barrier.srcAccessMask = pBarriers[i].m_SrcAccess == ResourceAccess::kAuto ?
				GetD3D12AccessFlags(pBarriers[i].m_SrcState) : GetD3D12AccessFlags(pBarriers[i].m_SrcAccess);
			barrier.srcStageMask |= pBarriers[i].m_SrcPipelineStage == PipelineStage::kAuto ?
				GetD3D12PipelineStageFlags(barrier.srcAccessMask) : GetD3D12PipelineStageFlags(pBarriers[i].m_SrcPipelineStage);

			barrier.dstAccessMask = pBarriers[i].m_DstAccess == ResourceAccess::kAuto ?
				GetD3D12AccessFlags(pBarriers[i].m_DstState) : GetD3D12AccessFlags(pBarriers[i].m_DstAccess);
			barrier.dstStageMask |= pBarriers[i].m_DstPipelineStage == PipelineStage::kAuto ?
				GetD3D12PipelineStageFlags(barrier.dstAccessMask) : GetD3D12PipelineStageFlags(pBarriers[i].m_DstPipelineStage);
		}
	}

	if (m_BufferBarriers.Size() > 0 || m_ImageBarriers.Size() > 0 || m_MemoryBarriers.Size() > 0)
	{
		ResourceBarrier(static_cast<u32>(m_BufferBarriers.Size()), m_BufferBarriers.Data(),
			static_cast<u32>(m_ImageBarriers.Size()), m_ImageBarriers.Data(),
			static_cast<u32>(m_MemoryBarriers.Size()), m_MemoryBarriers.Data(), dependencyFlags);
	}

	m_MemoryBarriers.Clear();
	m_BufferBarriers.Clear();
	m_ImageBarriers.Clear();
}


void BvCommandListD3D12::SetPredication(const IBvBuffer* pBuffer, u64 offset, PredicationOp predicationOp)
{
	if (pBuffer)
	{
		auto pBufferD3D12 = TO_D3D12(pBuffer);
		m_CommandList->SetPredication(pBufferD3D12->GetHandle(), offset, );
	}
	else
	{
		m_CommandList->SetPredication(nullptr, 0, D3D12_PREDICATION_OP_EQUAL_ZERO);
	}
}


void BvCommandListD3D12::BeginQuery(IBvQuery* pQuery)
{
	auto pQueryD3D12 = TO_D3D12(pQuery);
	auto queryType = pQueryD3D12->GetQueryType();
	auto pData = pQueryD3D12->Allocate(m_pFrameData->GetQueryHeapManager(), m_pFrameData->GetFrameIndex());
	if (queryType != QueryType::kTimestamp)
	{
		D3D12QueryControlFlags flags = queryType == QueryType::kOcclusion ? D3D12_QUERY_CONTROL_PRECISE_BIT : 0;
		D3D12CmdBeginQuery(m_CommandBuffer, pData->m_pQueryHeap->GetHandle(pData->m_HeapIndex), pData->m_QueryIndex, flags);
		if (queryType == QueryType::kMeshPipelineStatistics)
		{
			auto meshPool = pData->m_pQueryHeap->GetHandle(pData->m_HeapIndex, 1);
			if (meshPool != D3D12_NULL_HANDLE)
			{
				m_MeshQueries.EmplaceBack(pQueryD3D12);
			}
		}
	}

	m_pFrameData->AddQuery(pQueryD3D12);
}


void BvCommandListD3D12::EndQuery(IBvQuery* pQuery)
{
	ResetRenderTargets();

	auto pQueryD3D12 = TO_D3D12(pQuery);
	auto queryType = pQueryD3D12->GetQueryType();
	auto frameIndex = m_pFrameData->GetFrameIndex();
	auto pData = pQueryD3D12->GetQueryData(frameIndex);
	auto pool = pData->m_pQueryHeap->GetHandle(pData->m_HeapIndex);
	auto meshPool = pData->m_pQueryHeap->GetHandle(pData->m_HeapIndex, 1);
	if (queryType == QueryType::kTimestamp)
	{
		D3D12CmdWriteTimestamp2(m_CommandBuffer, D3D12_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, pool, pData->m_QueryIndex);
	}
	else
	{
		D3D12CmdEndQuery(m_CommandBuffer, pool, pData->m_QueryIndex);
	}
	D3D12Buffer buffer;
	u64 stride = 0, offset = 0;
	pData->m_pQueryHeap->GetBufferInformation(pData->m_HeapIndex, frameIndex, pData->m_QueryIndex, buffer, offset, stride);
	D3D12QueryResultFlags flags = D3D12_QUERY_RESULT_64_BIT | D3D12_QUERY_RESULT_WAIT_BIT;
	if (queryType == QueryType::kOcclusionBinary)
	{
		flags |= D3D12_QUERY_RESULT_PARTIAL_BIT;
	}

	D3D12CmdCopyQueryPoolResults(m_CommandBuffer, pool, pData->m_QueryIndex, 1, buffer, offset, stride, flags);
	if (queryType == QueryType::kMeshPipelineStatistics && meshPool != D3D12_NULL_HANDLE)
	{
		pData->m_pQueryHeap->GetBufferInformation(pData->m_HeapIndex, frameIndex, pData->m_QueryIndex, buffer, offset, stride, 1);
		D3D12CmdCopyQueryPoolResults(m_CommandBuffer, meshPool, pData->m_QueryIndex, 1, buffer, offset, stride, flags);
	}
}


void BvCommandListD3D12::BeginEvent(const char* pName, const BvColor& color)
{
	D3D12DebugUtilsLabelEXT label{ D3D12_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
	label.pLabelName = pName;
	label.color[0] = color.m_Red;
	label.color[1] = color.m_Green;
	label.color[2] = color.m_Blue;
	label.color[3] = color.m_Alpha;
	D3D12CmdBeginDebugUtilsLabelEXT(m_CommandBuffer, &label);
}


void BvCommandListD3D12::EndEvent()
{
	if (!m_HasDebugUtils)
	{
		return;
	}

	D3D12CmdEndDebugUtilsLabelEXT(m_CommandBuffer);
}


void BvCommandListD3D12::SetMarker(const char* pName, const BvColor& color)
{
	if (!m_HasDebugUtils)
	{
		return;
	}

	D3D12DebugUtilsLabelEXT label{ D3D12_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
	label.pLabelName = pName;
	label.color[0] = color.m_Red;
	label.color[1] = color.m_Green;
	label.color[2] = color.m_Blue;
	label.color[3] = color.m_Alpha;
	D3D12CmdInsertDebugUtilsLabelEXT(m_CommandBuffer, &label);
}


void BvCommandListD3D12::BuildBLAS(const BLASBuildDesc& desc, const ASPostBuildDesc* pPostBuildDesc)
{
	if (desc.m_GeometryCount > desc.m_pBLAS->GetDesc().m_BLAS.m_GeometryCount)
	{
		return;
	}

	auto pAS = TO_D3D12(desc.m_pBLAS);
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
		if (geoms[index].geometryType != GetD3D12GeometryType(srcGeometry.m_Type))
		{
			continue;
		}

		D3D12AccelerationStructureGeometryKHR& dstGeometry = m_ASGeometries.EmplaceBack(geoms[index]);
		D3D12AccelerationStructureBuildRangeInfoKHR& range = m_ASRanges.EmplaceBack();
		if (srcGeometry.m_Type == RayTracingGeometryType::kTriangles)
		{
			D3D12AccelerationStructureGeometryTrianglesDataKHR& triangle = dstGeometry.geometry.triangles;
			triangle.vertexData.deviceAddress = TO_D3D12(srcGeometry.m_Triangle.m_pVertexBuffer)->GetDeviceAddress() + srcGeometry.m_Triangle.m_VertexOffset;
			if (srcGeometry.m_Triangle.m_pIndexBuffer)
			{
				triangle.indexData.deviceAddress = TO_D3D12(srcGeometry.m_Triangle.m_pIndexBuffer)->GetDeviceAddress() + srcGeometry.m_Triangle.m_IndexOffset;
			}
		}
		else if (srcGeometry.m_Type == RayTracingGeometryType::kAABB)
		{
			D3D12AccelerationStructureGeometryAabbsDataKHR& aabb = dstGeometry.geometry.aabbs;
			aabb.data.deviceAddress = TO_D3D12(srcGeometry.m_AABB.m_pBuffer)->GetDeviceAddress() + srcGeometry.m_AABB.m_Offset;
		}
		range.primitiveCount = pAS->GetPrimitiveCounts()[index];
	}

	D3D12AccelerationStructureBuildGeometryInfoKHR buildInfo{ D3D12_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
	buildInfo.type = D3D12_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	buildInfo.flags = GetD3D12BuildAccelerationStructureFlags(pAS->GetDesc().m_Flags);
	if (desc.m_Update && (buildInfo.flags & D3D12_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR))
	{
		buildInfo.mode = D3D12_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
		buildInfo.srcAccelerationStructure = pAS->GetHandle();
	}
	else
	{
		buildInfo.mode = D3D12_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	}
	buildInfo.dstAccelerationStructure = pAS->GetHandle();
	buildInfo.geometryCount = u32(m_ASGeometries.Size());
	buildInfo.pGeometries = m_ASGeometries.Data();
	//buildInfo.ppGeometries = nullptr;
	buildInfo.scratchData.deviceAddress = TO_D3D12(desc.m_pScratchBuffer)->GetDeviceAddress() + desc.m_ScratchBufferOffset;

	D3D12AccelerationStructureBuildRangeInfoKHR* pRanges = m_ASRanges.Data();
	D3D12CmdBuildAccelerationStructuresKHR(m_CommandBuffer, 1, &buildInfo, &pRanges);

	m_ASGeometries.Clear();
	m_ASRanges.Clear();

	u32 barrierCount = 1;
	D3D12BufferMemoryBarrier2 barriers[2] = { { D3D12_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2 }, { D3D12_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2 } };
	barriers[0].srcAccessMask = D3D12_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	barriers[0].dstAccessMask = D3D12_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	barriers[0].srcStageMask = D3D12_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
	barriers[0].dstStageMask = D3D12_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
	barriers[0].buffer = TO_D3D12(pAS->GetBuffer())->GetHandle();
	barriers[0].size = D3D12_WHOLE_SIZE;

	if (pPostBuildDesc && pPostBuildDesc->m_pDstBuffer)
	{
		barrierCount++;
		barriers[1].srcAccessMask = D3D12_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
		barriers[1].dstAccessMask = D3D12_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
		barriers[1].srcStageMask = D3D12_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		barriers[1].dstStageMask = D3D12_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		barriers[1].buffer = TO_D3D12(pPostBuildDesc->m_pDstBuffer)->GetHandle();
		barriers[1].size = D3D12_WHOLE_SIZE;
	}

	ResourceBarrier(barrierCount, barriers, 0, nullptr, 0, nullptr);

	if (pPostBuildDesc)
	{
		EmitASPostBuild(pAS, *pPostBuildDesc);
	}
}


void BvCommandListD3D12::BuildTLAS(const TLASBuildDesc& desc, const ASPostBuildDesc* pPostBuildDesc)
{
	auto pAS = TO_D3D12(desc.m_pTLAS);
	D3D12AccelerationStructureGeometryKHR& dstGeometry = m_ASGeometries.EmplaceBack(pAS->GetGeometries()[0]);
	dstGeometry.geometry.instances.data.deviceAddress = TO_D3D12(desc.m_pInstanceBuffer)->GetDeviceAddress() + desc.m_InstanceBufferOffset;

	D3D12AccelerationStructureBuildGeometryInfoKHR buildInfo{ D3D12_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
	buildInfo.type = D3D12_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	buildInfo.flags = GetD3D12BuildAccelerationStructureFlags(pAS->GetDesc().m_Flags);
	if (desc.m_Update && (buildInfo.flags & D3D12_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR))
	{
		buildInfo.mode = D3D12_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
		buildInfo.srcAccelerationStructure = pAS->GetHandle();
	}
	else
	{
		buildInfo.mode = D3D12_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	}
	buildInfo.dstAccelerationStructure = pAS->GetHandle();
	buildInfo.geometryCount = u32(m_ASGeometries.Size());
	buildInfo.pGeometries = m_ASGeometries.Data();
	//buildInfo.ppGeometries = nullptr;
	buildInfo.scratchData.deviceAddress = TO_D3D12(desc.m_pScratchBuffer)->GetDeviceAddress() + desc.m_ScratchBufferOffset;

	D3D12AccelerationStructureBuildRangeInfoKHR& range = m_ASRanges.EmplaceBack(D3D12AccelerationStructureBuildRangeInfoKHR{});
	range.primitiveCount = desc.m_InstanceCount;

	D3D12AccelerationStructureBuildRangeInfoKHR* pRanges = m_ASRanges.Data();
	D3D12CmdBuildAccelerationStructuresKHR(m_CommandBuffer, 1, &buildInfo, &pRanges);

	m_ASGeometries.Clear();
	m_ASRanges.Clear();

	if (pPostBuildDesc)
	{
		EmitASPostBuild(pAS, *pPostBuildDesc);
	}
}


void BvCommandListD3D12::EmitASPostBuild(IBvAccelerationStructure* pAS, const ASPostBuildDesc& postBuildDesc)
{
	auto pASD3D12 = TO_D3D12(pAS);
	if (postBuildDesc.m_Action == ASPostBuildAction::kWriteCompactedSize)
	{
		BV_ASSERT(pAS->GetDesc().m_CompactedSize == 0, "Acceleration Structure already compacted");

		auto pASQueries = m_pFrameData->GetQueryAS();
		auto query = pASQueries->Allocate(m_pFrameData->GetFrameIndex());
		auto as = pASD3D12->GetHandle();

		D3D12CmdWriteAccelerationStructuresPropertiesKHR(m_CommandBuffer, 1, &as, pASQueries->GetType(), query.m_Pool, query.m_Index);

		constexpr D3D12QueryResultFlags flags = D3D12_QUERY_RESULT_64_BIT | D3D12_QUERY_RESULT_WAIT_BIT;
		D3D12CmdCopyQueryPoolResults(m_CommandBuffer, query.m_Pool, query.m_Index, 1, TO_D3D12(postBuildDesc.m_pDstBuffer)->GetHandle(),
			postBuildDesc.m_DstBufferOffset, sizeof(u64), flags);
	}
}


void BvCommandListD3D12::CopyBLAS(const AccelerationStructureCopyDesc& copyDesc)
{
	BV_ASSERT(copyDesc.m_pSrc->GetDesc().m_Type == RayTracingAccelerationStructureType::kBottomLevel &&
		copyDesc.m_pDst->GetDesc().m_Type == RayTracingAccelerationStructureType::kBottomLevel, "Acceleration structure(s) not bottom level");

	D3D12CopyAccelerationStructureInfoKHR ci{ D3D12_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR };
	ci.mode = GetD3D12CopyAccelerationStructureMode(copyDesc.m_CopyMode);
	ci.src = TO_D3D12(copyDesc.m_pSrc)->GetHandle();
	ci.dst = TO_D3D12(copyDesc.m_pDst)->GetHandle();
	D3D12CmdCopyAccelerationStructureKHR(m_CommandBuffer, &ci);
}


void BvCommandListD3D12::CopyTLAS(const AccelerationStructureCopyDesc& copyDesc)
{
	BV_ASSERT(copyDesc.m_pSrc->GetDesc().m_Type == RayTracingAccelerationStructureType::kTopLevel &&
		copyDesc.m_pDst->GetDesc().m_Type == RayTracingAccelerationStructureType::kTopLevel, "Acceleration structure(s) not top level");

	D3D12CopyAccelerationStructureInfoKHR ci{ D3D12_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR };
	ci.mode = GetD3D12CopyAccelerationStructureMode(copyDesc.m_CopyMode);
	ci.src = TO_D3D12(copyDesc.m_pSrc)->GetHandle();
	ci.dst = TO_D3D12(copyDesc.m_pDst)->GetHandle();
	D3D12CmdCopyAccelerationStructureKHR(m_CommandBuffer, &ci);
}


void BvCommandListD3D12::DispatchRays(const DispatchRaysCommandArgs& args)
{
	FlushDescriptorSets();

	D3D12_DISPATCH_RAYS_DESC drd{ { args.m_RayGenShader.m_Address, args.m_RayGenShader.m_Size }, // For RayGen, stride == size
		{ args.m_MissShader.m_Address, args.m_MissShader.m_Stride, args.m_MissShader.m_Size },
		{ args.m_HitShader.m_Address, args.m_HitShader.m_Stride, args.m_HitShader.m_Size },
		{ args.m_CallableShader.m_Address, args.m_CallableShader.m_Stride, args.m_CallableShader.m_Size },
		args.m_Width, args.m_Height, args.m_Depth };
	m_CommandListEx->DispatchRays(&drd);
}


void BvCommandListD3D12::DispatchRaysIndirect(const IBvBuffer* pBuffer, u64 offset)
{
	FlushDescriptorSets();

	auto deviceAddress = pBuffer->GetDeviceAddress() + offset;
	D3D12CmdTraceRaysIndirect2KHR(m_CommandBuffer, deviceAddress);
}


void BvCommandListD3D12::FlushDescriptorSets()
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
				ResourceIdD3D12 resId{ set, resource.m_Binding, arrayIndex };
				if (auto pResourceData = rbs.GetResource(resId))
				{
					if (isSetDirty)
					{
						m_WriteSets.PushBack({ D3D12_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET });
						auto& writeSet = m_WriteSets.Back();
						//writeSet.dstSet = nullptr; // This is set in BvDescriptorSetD3D12::Update()
						writeSet.descriptorType = pResourceData->m_DescriptorType;
						writeSet.dstBinding = resource.m_Binding;
						writeSet.dstArrayElement = arrayIndex;
						writeSet.descriptorCount = 1;

						switch (pResourceData->m_DescriptorType)
						{
						case D3D12_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
						case D3D12_DESCRIPTOR_TYPE_STORAGE_BUFFER:
						case D3D12_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
							writeSet.pBufferInfo = &pResourceData->m_Data.m_BufferInfo;
							HashCombine(hashSeed, pResourceData->m_Data.m_BufferInfo);
							break;
						case D3D12_DESCRIPTOR_TYPE_SAMPLER:
						case D3D12_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
						case D3D12_DESCRIPTOR_TYPE_STORAGE_IMAGE:
						case D3D12_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
							writeSet.pImageInfo = &pResourceData->m_Data.m_ImageInfo;
							HashCombine(hashSeed, pResourceData->m_Data.m_ImageInfo);
							break;
						case D3D12_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
						case D3D12_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
							writeSet.pTexelBufferView = &pResourceData->m_Data.m_BufferView;
							HashCombine(hashSeed, pResourceData->m_Data.m_BufferView);
							break;
						case D3D12_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
							auto& asWrite = m_ASWriteSets.EmplaceBack(D3D12WriteDescriptorSetAccelerationStructureKHR{ D3D12_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR });
							asWrite.accelerationStructureCount = 1;
							asWrite.pAccelerationStructures = &pResourceData->m_Data.m_AccelerationStructure;
							writeSet.pNext = &asWrite;
							HashCombine(hashSeed, pResourceData->m_Data.m_AccelerationStructure);
							break;
						}
					}

					if (pResourceData->m_DescriptorType == D3D12_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
						|| pResourceData->m_DescriptorType == D3D12_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
					{
						m_DynamicOffsets.EmplaceBack(pResourceData->m_DynamicOffset);
					}
				}
			}
		}

		if (!m_WriteSets.Empty() || !m_DynamicOffsets.Empty())
		{
			D3D12DescriptorSet descriptorSet;
			if (isSetDirty)
			{
				descriptorSet = m_pFrameData->RequestDescriptorSet(set, m_pShaderResourceLayout, m_WriteSets, hashSeed, resourceSet.m_Bindless);
				if (m_DescriptorSets.Size() <= set)
				{
					m_DescriptorSets.Resize(set + 1, D3D12_NULL_HANDLE);
				}
				m_DescriptorSets[set] = descriptorSet;
			}
			else
			{
				descriptorSet = m_DescriptorSets[set];
			}

			D3D12CmdBindDescriptorSets(m_CommandBuffer, m_PipelineBindPoint, m_pShaderResourceLayout->GetPipelineLayoutHandle(), set,
				1, &descriptorSet, m_DynamicOffsets.Size(), m_DynamicOffsets.Size() > 0 ? m_DynamicOffsets.Data() : nullptr);
			m_WriteSets.Clear();
			m_ASWriteSets.Clear();
			m_DynamicOffsets.Clear();
		}

		rbs.MarkClean(set);
	}
}


void BvCommandListD3D12::ResetRenderTargets()
{
	if (m_CurrentState == State::kRenderPass)
	{
		EndMeshQueries();

		EndRenderPass();
	}
	else if (m_CurrentState == State::kRenderTarget)
	{
		EndMeshQueries();

		m_CommandList->OMSetRenderTargets(0, nullptr, FALSE, nullptr);

		if (m_PostRenderBarriers.Size() > 0)
		{
			D3D12DependencyInfo dependencyInfo{ D3D12_STRUCTURE_TYPE_DEPENDENCY_INFO };
			dependencyInfo.imageMemoryBarrierCount = static_cast<u32>(m_PostRenderBarriers.Size());
			dependencyInfo.pImageMemoryBarriers = m_PostRenderBarriers.Data();

			D3D12CmdPipelineBarrier2(m_CommandBuffer, &dependencyInfo);
		}

		m_PostRenderBarriers.Clear();

		m_CurrentState = State::kRecording;
	}
}


void BvCommandListD3D12::AddSwapChain(BvSwapChainD3D12* pSwapChain)
{
	if (!m_SwapChains.Contains(pSwapChain))
	{
		m_SwapChains.EmplaceBack(pSwapChain);
	}
}


void BvCommandListD3D12::BeginMeshQueries()
{
	for (auto pQueryD3D12 : m_MeshQueries)
	{
		auto frameIndex = m_pFrameData->GetFrameIndex();
		auto pData = pQueryD3D12->GetQueryData(frameIndex);
		D3D12CmdBeginQuery(m_CommandBuffer, pData->m_pQueryHeap->GetHandle(pData->m_HeapIndex, 1), pData->m_QueryIndex, 0);
	}
}


void BvCommandListD3D12::EndMeshQueries()
{
	for (auto pQueryD3D12 : m_MeshQueries)
	{
		auto frameIndex = m_pFrameData->GetFrameIndex();
		auto pData = pQueryD3D12->GetQueryData(frameIndex);
		auto pool = pData->m_pQueryHeap->GetHandle(pData->m_HeapIndex, 1);
		auto index = pData->m_QueryIndex;
		D3D12CmdEndQuery(m_CommandBuffer, pool, index);
	}

	m_MeshQueries.Clear();
}