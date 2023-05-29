#include "BvRenderVK/BvRenderPassVk.h"
#include "BvRenderDeviceVk.h"
#include "BvUtilsVk.h"
#include "BvTypeConversionsVk.h"


BvRenderPassVk::BvRenderPassVk(const BvRenderDeviceVk & device, const RenderPassDesc & renderPassDesc)
	: BvRenderPass(renderPassDesc), m_Device(device)
{
	Create();
}


BvRenderPassVk::~BvRenderPassVk()
{
	Destroy();
}


void BvRenderPassVk::Create()
{
	BvFixedVector<VkAttachmentDescription, kMaxRenderTargetsWithDepth>
		attachmentDescs(m_RenderPassDesc.m_RenderTargets.Size() + (m_RenderPassDesc.m_HasDepth ? 1 : 0), VkAttachmentDescription());
	BvFixedVector<VkAttachmentReference, kMaxRenderTargetsWithDepth>
		attachmentReferences(m_RenderPassDesc.m_RenderTargets.Size() + (m_RenderPassDesc.m_HasDepth ? 1 : 0), VkAttachmentReference());
	SetupAttachments(attachmentDescs, attachmentReferences);

	BvFixedVector<VkSubpassDependency, 2> dependencies(2, VkSubpassDependency());
	SetupDependencies(dependencies);

	auto renderTargetCount = m_RenderPassDesc.m_RenderTargets.Size();
	VkSubpassDescription subpassDesc = {};
	subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	if (renderTargetCount > 0)
	{
		subpassDesc.pColorAttachments = attachmentReferences.Data();
		subpassDesc.colorAttachmentCount = static_cast<uint32_t>(renderTargetCount);
	}
	if (m_RenderPassDesc.m_HasDepth)
	{
		subpassDesc.pDepthStencilAttachment = &attachmentReferences[renderTargetCount];
	}

	// Create render pass
	VkRenderPassCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.pAttachments = attachmentDescs.Data();
	createInfo.attachmentCount = static_cast<u32>(attachmentDescs.Size());
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpassDesc;
	createInfo.dependencyCount = 2;
	createInfo.pDependencies = dependencies.Data();

	auto result = vkCreateRenderPass(m_Device.GetHandle(), &createInfo, nullptr, &m_RenderPass);
}


void BvRenderPassVk::Destroy()
{
	if (m_RenderPass)
	{
		vkDestroyRenderPass(m_Device.GetHandle(), m_RenderPass, nullptr);
		m_RenderPass = VK_NULL_HANDLE;
	}
}


void BvRenderPassVk::SetupAttachments(BvFixedVector<VkAttachmentDescription, kMaxRenderTargetsWithDepth> & attachmentDescs,
	BvFixedVector<VkAttachmentReference, kMaxRenderTargetsWithDepth> & attachmentReferences)
{
	u32 i = 0;
	for (; i < m_RenderPassDesc.m_RenderTargets.Size(); i++)
	{
		auto & rt = m_RenderPassDesc.m_RenderTargets[i];
		attachmentDescs[i].format = GetVkFormat(rt.m_Format);
		attachmentDescs[i].samples = static_cast<VkSampleCountFlagBits>(rt.m_SampleCount);
		attachmentDescs[i].loadOp = GetVkAttachmentLoadOp(rt.m_LoadOp);
		attachmentDescs[i].storeOp = GetVkAttachmentStoreOp(rt.m_StoreOp);
		attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		//attachmentDescs[i].flags = 0;

		if (attachmentDescs[i].loadOp == VK_ATTACHMENT_LOAD_OP_LOAD)
		{
			attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		else
		{
			attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		}

		if (attachmentDescs[i].storeOp == VK_ATTACHMENT_STORE_OP_STORE)
		{
			if (rt.m_StateAfter == ResourceState::kUnknown)
			{
				attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}
			else
			{
				attachmentDescs[i].finalLayout = GetVkImageLayout(rt.m_StateAfter);
			}
		}
		else
		{
			attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		attachmentReferences[i].attachment = i;
		attachmentReferences[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	if (m_RenderPassDesc.m_HasDepth)
	{
		auto & rt = m_RenderPassDesc.m_DepthStencilTarget;
		attachmentDescs[i].format = GetVkFormat(rt.m_Format);
		attachmentDescs[i].samples = static_cast<VkSampleCountFlagBits>(rt.m_SampleCount);
		attachmentDescs[i].loadOp = GetVkAttachmentLoadOp(rt.m_LoadOp);
		attachmentDescs[i].storeOp = GetVkAttachmentStoreOp(rt.m_StoreOp);
		attachmentDescs[i].stencilLoadOp = GetVkAttachmentLoadOp(rt.m_StencilLoadOp);
		attachmentDescs[i].stencilStoreOp = GetVkAttachmentStoreOp(rt.m_StencilStoreOp);
		//attachmentDescs[i].flags = 0;
		if (attachmentDescs[i].loadOp == VK_ATTACHMENT_LOAD_OP_LOAD)
		{
			attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else
		{
			attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		}

		if (attachmentDescs[i].storeOp == VK_ATTACHMENT_STORE_OP_STORE)
		{
			if (rt.m_StateAfter == ResourceState::kUnknown)
			{
				attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}
			else
			{
				attachmentDescs[i].finalLayout = GetVkImageLayout(rt.m_StateAfter);
				if (attachmentDescs[i].finalLayout == VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
				{
					attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				}
			}
		}
		else
		{
			attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}


		if (m_RenderPassDesc.m_DepthStencilTarget.m_IsReadOnlyDepth)
		{
			if (m_RenderPassDesc.m_DepthStencilTarget.m_IsReadOnlyStencil)
			{
				attachmentReferences[i].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			}
			else
			{
				attachmentReferences[i].layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
			}
		}
		else
		{
			if (m_RenderPassDesc.m_DepthStencilTarget.m_IsReadOnlyStencil)
			{
				attachmentReferences[i].layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
			}
			else
			{
				attachmentReferences[i].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}
		}

		attachmentReferences[i].attachment = i;
	}
}


void BvRenderPassVk::SetupDependencies(BvFixedVector<VkSubpassDependency, 2> & dependencies)
{
	bool hasSwapChain = false;
	bool hasColor = false;
	for (auto && rt : m_RenderPassDesc.m_RenderTargets)
	{
		if (rt.m_StateAfter == ResourceState::kPresent)
		{
			hasSwapChain = true;
		}
	}

	VkPipelineStageFlags srcStageMask = 0;
	VkPipelineStageFlags dstStageMask = 0;
	VkAccessFlags srcAccessMask = 0;
	VkAccessFlags dstAccessMask = 0;
	if (m_RenderPassDesc.m_RenderTargets.Size() > 0)
	{
		dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}
	if (m_RenderPassDesc.m_HasDepth)
	{
		dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}

	if (hasSwapChain)
	{
		srcStageMask |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		srcAccessMask |= VK_ACCESS_MEMORY_READ_BIT;
	}
	else
	{
		srcStageMask |= dstStageMask;
		srcAccessMask |= dstAccessMask;
	}

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = srcStageMask;
	dependencies[0].dstStageMask = dstStageMask;
	dependencies[0].srcAccessMask = srcAccessMask;
	dependencies[0].dstAccessMask = dstAccessMask;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = dstStageMask;
	dependencies[1].dstStageMask = srcStageMask;
	dependencies[1].srcAccessMask = dstAccessMask;
	dependencies[1].dstAccessMask = srcAccessMask;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
}


BvRenderPassManagerVk::BvRenderPassManagerVk()
{
}


BvRenderPassManagerVk::~BvRenderPassManagerVk()
{
	Destroy();
}


BvRenderPassVk* BvRenderPassManagerVk::GetRenderPass(const BvRenderDeviceVk& device, const RenderPassDesc& desc)
{
	BvScopedLock lock(m_Lock);
	decltype(auto) pRenderPass = m_RenderPasses[desc];

	if (pRenderPass == nullptr)
	{
		pRenderPass = new BvRenderPassVk(device, desc);
	}

	return pRenderPass;
}


void BvRenderPassManagerVk::Destroy()
{
	for (auto&& pRenderPass : m_RenderPasses)
	{
		delete pRenderPass.second;
	}
	m_RenderPasses.Clear();
}


BvRenderPassManagerVk* GetRenderPassManager()
{
	static BvRenderPassManagerVk instance;
	return &instance;
}