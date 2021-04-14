#include "BvFramebufferVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTextureViewVk.h"
#include "BvTypeConversionsVk.h"


BvFramebufferManager g_FramebufferManager;


BvFramebufferVk::BvFramebufferVk(const BvRenderDeviceVk & device, const FramebufferDesc & framebufferDesc)
	: m_FramebufferDesc(framebufferDesc), m_Device(device)
{
}


BvFramebufferVk::~BvFramebufferVk()
{
	Destroy();
}


void BvFramebufferVk::Create()
{
	BvFixedVector<VkFramebufferAttachmentImageInfo, kMaxRenderTargetsWithDepth> framebufferAIIs(m_FramebufferDesc.m_RenderTargetViews.Size()
		+ (m_FramebufferDesc.m_pDepthStencilView ? 1 : 0), {});
	BvAssert(framebufferAIIs.Size() > 0, "No framebuffer attachments");
	BvFixedVector<VkFormat, kMaxRenderTargetsWithDepth> formats(framebufferAIIs.Size(), {});

	u32 i = 0;
	for (; i < framebufferAIIs.Size(); i++)
	{
		decltype(auto) viewDesc = i < m_FramebufferDesc.m_RenderTargetViews.Size() ?
			m_FramebufferDesc.m_RenderTargetViews[i]->GetDesc() : m_FramebufferDesc.m_pDepthStencilView->GetDesc();
		decltype(auto) textureDesc = viewDesc.m_pTexture->GetDesc();
		VkImageCreateFlags flags = 0;
		if (textureDesc.m_UseAsCubeMap)
		{
			flags |= VkImageCreateFlagBits::VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		}
		formats[i] = GetVkFormat(textureDesc.m_Format);

		framebufferAIIs[i].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO;
		framebufferAIIs[i].flags = flags;
		framebufferAIIs[i].usage = GetVkImageUsageFlags(textureDesc.m_UsageFlags);
		framebufferAIIs[i].width = textureDesc.m_Size.width;
		framebufferAIIs[i].height = textureDesc.m_Size.height;
		framebufferAIIs[i].layerCount = viewDesc.m_SubresourceDesc.layerCount;
		framebufferAIIs[i].viewFormatCount = 1;
		framebufferAIIs[i].pViewFormats = formats.Data();
	}

	VkFramebufferAttachmentsCreateInfo framebufferACI{};
	framebufferACI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO;
	framebufferACI.attachmentImageInfoCount = (u32)framebufferAIIs.Size();
	framebufferACI.pAttachmentImageInfos = framebufferAIIs.Data();

	VkFramebufferCreateInfo framebufferCI{};
	framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCI.pNext = &framebufferACI;
	framebufferCI.flags = VkFramebufferCreateFlagBits::VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
	framebufferCI.width = framebufferAIIs[0].width;
	framebufferCI.height = framebufferAIIs[0].height;
	framebufferCI.layers = framebufferAIIs[0].layerCount;
	framebufferCI.renderPass = m_FramebufferDesc.m_RenderPass;
	framebufferCI.attachmentCount = (u32)framebufferAIIs.Size();

	auto result = m_Device.GetDeviceFunctions().vkCreateFramebuffer(m_Device.GetHandle(), &framebufferCI, nullptr, &m_Framebuffer);
}


void BvFramebufferVk::Destroy()
{
	if (m_Framebuffer != VK_NULL_HANDLE)
	{
		m_Device.GetDeviceFunctions().vkDestroyFramebuffer(m_Device.GetHandle(), m_Framebuffer, nullptr);
		m_Framebuffer = VK_NULL_HANDLE;
	}
}


BvFramebufferManager::BvFramebufferManager()
{
}


BvFramebufferManager::~BvFramebufferManager()
{
}


BvFramebufferVk * BvFramebufferManager::GetFramebuffer(const BvRenderDeviceVk & device, const FramebufferDesc & desc)
{
	auto hash = std::hash<FramebufferDesc>()(desc);

	BvScopedLock lock(m_Lock);
	BvFramebufferVk * pFramebuffer = m_Framebuffers[hash];

	if (pFramebuffer == nullptr)
	{
		pFramebuffer = new BvFramebufferVk(device, desc);
		pFramebuffer->Create();

		m_Framebuffers[hash] = pFramebuffer;
	}

	return pFramebuffer;
}


void BvFramebufferManager::RemoveFramebuffer(const BvTextureViewVk * const pTextureView)
{
	BvScopedLock lock(m_Lock);
	for (auto && pFramebuffer : m_Framebuffers)
	{
		decltype(auto) desc = pFramebuffer.second->GetDesc();
		for (auto && pRTV : desc.m_RenderTargetViews)
		{
			if (pRTV == pTextureView)
			{
				m_Framebuffers.Erase(pFramebuffer.first);
				return;
			}
		}

		if (desc.m_pDepthStencilView != nullptr && desc.m_pDepthStencilView == pTextureView)
		{
			m_Framebuffers.Erase(pFramebuffer.first);
			return;
		}
	}
}