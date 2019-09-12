#include "BvVulkanRenderTarget.h"
#include "BvRender/RenderAPIs/Vulkan/BvVulkanDevice.h"
#include "BvCore/BvDebug.h"


BvVulkanRenderTarget::BvVulkanRenderTarget(const BvVulkanDevice * const pDevice, const RenderTargetDesc & renderTargetDesc)
	: m_pDevice(pDevice), m_RenderTargetDesc(renderTargetDesc)
{
}


BvVulkanRenderTarget::~BvVulkanRenderTarget()
{
	Destroy();
}


void BvVulkanRenderTarget::Create()
{
	CreateRenderPass();
	CreateFramebuffer();
}


void BvVulkanRenderTarget::Destroy()
{
	auto device = m_pDevice->GetLogical();
	for (auto && framebuffer : m_Framebuffers)
	{
		vkDestroyFramebuffer(device, framebuffer, nullptr);
		framebuffer = VK_NULL_HANDLE;
	}

	if (m_RenderPass)
	{
		vkDestroyRenderPass(device, m_RenderPass, nullptr);
		m_RenderPass = VK_NULL_HANDLE;
	}
}


void BvVulkanRenderTarget::CreateRenderPass()
{
	BvVector<VkAttachmentDescription> attachmentDescriptions(m_RenderTargetDesc.m_Textures.Size());
	for (auto i = 0U; i < m_RenderTargetDesc.m_Textures.Size() && i < kMaxRenderTargets; i++)
	{
		decltype(auto) textureDesc = m_RenderTargetDesc.m_Textures[i]->GetTextureDesc();

		attachmentDescriptions[i].format = textureDesc.m_Format;
		attachmentDescriptions[i].samples = static_cast<VkSampleCountFlagBits>(textureDesc.m_SampleCount);
		attachmentDescriptions[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescriptions[i].storeOp = textureDesc.m_UsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT ?
			VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
		if (textureDesc.IsStencilTexture() && textureDesc.m_UseForStenciling)
		{
			attachmentDescriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDescriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		}
		else
		{
			attachmentDescriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentDescriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		}
		attachmentDescriptions[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		if (textureDesc.IsDepthTexture() || textureDesc.IsStencilTexture())
		{
			attachmentDescriptions[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		}
		else
		{
			attachmentDescriptions[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
	};

	// Collect attachment references
	BvVector<VkAttachmentReference> colorReferences;
	VkAttachmentReference depthReference = {};
	bool hasDepth = false;
	bool hasColor = false;

	uint32_t attachmentIndex = 0;
	for (decltype(auto) texture : m_RenderTargetDesc.m_Textures)
	{
		decltype(auto) textureDesc = texture->GetTextureDesc();
		if (textureDesc.IsDepthTexture() || textureDesc.IsStencilTexture())
		{
			BvAssert(!hasDepth);
			depthReference.attachment = attachmentIndex;
			depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			hasDepth = true;
		}
		else
		{
			colorReferences.PushBack({ attachmentIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
			hasColor = true;
		}
		attachmentIndex++;
	};

	// Default render pass setup uses only one subpass
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	if (hasColor)
	{
		subpass.pColorAttachments = colorReferences.Data();
		subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.Size());
	}
	if (hasDepth)
	{
		subpass.pDepthStencilAttachment = &depthReference;
	}

	VkSubpassDependency dependencies[2]{};

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.Size());
	renderPassInfo.pAttachments = attachmentDescriptions.Data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies;
	vkCreateRenderPass(m_pDevice->GetLogical(), &renderPassInfo, nullptr, &m_RenderPass);
}


void BvVulkanRenderTarget::CreateFramebuffer()
{
	BvVector<VkImageView> views;
	for (auto i = 0U; i < m_RenderTargetDesc.m_Textures.Size(); i++)
	{
		views[i] = m_RenderTargetDesc.m_Textures[i]->GetView();
	}

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = m_RenderPass;
	framebufferInfo.pAttachments = views.Data();
	framebufferInfo.attachmentCount = static_cast<uint32_t>(views.Size());
	framebufferInfo.width = m_RenderTargetDesc.m_Width;
	framebufferInfo.height = m_RenderTargetDesc.m_Height;
	framebufferInfo.layers = m_RenderTargetDesc.m_Layers;

	// Right now, only creates one frambuffer, as the swapchain does the creation and cleaning
	// and only uses this class in order to make every render target generic
	m_Framebuffers.Resize(1);
	vkCreateFramebuffer(m_pDevice->GetLogical(), &framebufferInfo, nullptr, &m_Framebuffers[0]);
}