#include "BvVulkanFramebuffer.h"
#include "BvRender/RenderAPIs/Vulkan/BvVulkanDevice.h"
#include "BvRender/RenderAPIs/Vulkan/BvVulkanTexture.h"
#include "BvCore/BvDebug.h"



BvVulkanFramebuffer::BvVulkanFramebuffer(const BvVulkanDevice * const pDevice, const FramebufferDesc & FramebufferDesc,
	BvVulkanRenderPass * const pRenderPass)
	: m_pDevice(pDevice), m_FramebufferDesc(FramebufferDesc), m_pRenderPass(pRenderPass)
{
	Create();
}


BvVulkanFramebuffer::~BvVulkanFramebuffer()
{
	Destroy();
}


void BvVulkanFramebuffer::Recreate(const FramebufferDesc & FramebufferDesc, BvVulkanRenderPass * const pRenderPass)
{
	if (pRenderPass)
	{
		m_pRenderPass = pRenderPass;
	}

	if (m_Framebuffer)
	{
		vkDestroyFramebuffer(m_pDevice->GetLogical(), m_Framebuffer, nullptr);
		m_Framebuffer = VK_NULL_HANDLE;
	}

	m_FramebufferDesc = FramebufferDesc;

	Create();
}


void BvVulkanFramebuffer::Create()
{
	// Create a default render pass based on the framebuffer data
	if (!(m_pRenderPass || m_RenderPass))
	{
		CreateRenderPass();
	}

	CreateFramebuffer();
}


void BvVulkanFramebuffer::Destroy()
{
	auto device = m_pDevice->GetLogical();

	if (m_Framebuffer)
	{
		vkDestroyFramebuffer(device, m_Framebuffer, nullptr);
		m_Framebuffer = VK_NULL_HANDLE;
	}

	if (m_RenderPass)
	{
		vkDestroyRenderPass(device, m_RenderPass, nullptr);
		m_RenderPass = VK_NULL_HANDLE;
	}
}


void BvVulkanFramebuffer::CreateRenderPass()
{
	BvVector<VkAttachmentDescription> attachmentDescriptions(m_FramebufferDesc.m_Textures.Size());
	for (auto i = 0U; i < m_FramebufferDesc.m_Textures.Size() && i < kMaxFramebuffers; i++)
	{
		decltype(auto) textureDesc = m_FramebufferDesc.m_Textures[i]->GetTextureDesc();

		attachmentDescriptions[i].format = textureDesc.m_Format;
		attachmentDescriptions[i].samples = static_cast<VkSampleCountFlagBits>(textureDesc.m_SampleCount);
		attachmentDescriptions[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescriptions[i].storeOp = textureDesc.m_UsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT ?
			VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
		if (textureDesc.IsStencilTexture() && textureDesc.m_UseForStenciling)
		{
			attachmentDescriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentDescriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		}
		else
		{
			attachmentDescriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDescriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		}
		attachmentDescriptions[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		if (textureDesc.IsDepthTexture() || textureDesc.IsStencilTexture())
		{
			attachmentDescriptions[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		}
		else
		{
			attachmentDescriptions[i].finalLayout = textureDesc.m_Offscreen ?
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}
	};

	// Collect attachment references
	BvVector<VkAttachmentReference> colorReferences;
	VkAttachmentReference depthReference = {};
	bool hasDepth = false;
	bool hasColor = false;

	uint32_t attachmentIndex = 0;
	for (decltype(auto) texture : m_FramebufferDesc.m_Textures)
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


void BvVulkanFramebuffer::CreateFramebuffer()
{
	BvFixedVector<VkImageView, kMaxFramebuffers> views;
	for (auto i = 0U; i < m_FramebufferDesc.m_Textures.Size(); i++)
	{
		views.EmplaceBack(m_FramebufferDesc.m_Textures[i]->GetView());
	}

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = GetRenderPass();
	framebufferInfo.pAttachments = views.Data();
	framebufferInfo.attachmentCount = static_cast<uint32_t>(views.Size());
	framebufferInfo.width = m_FramebufferDesc.m_Width;
	framebufferInfo.height = m_FramebufferDesc.m_Height;
	framebufferInfo.layers = m_FramebufferDesc.m_Layers;

	vkCreateFramebuffer(m_pDevice->GetLogical(), &framebufferInfo, nullptr, &m_Framebuffer);
}