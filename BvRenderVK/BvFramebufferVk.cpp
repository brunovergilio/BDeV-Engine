#include "BvFramebufferVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTextureViewVk.h"
#include "BvTypeConversionsVk.h"


BvFramebufferVk::BvFramebufferVk(const BvRenderDeviceVk & device, const FramebufferDesc & framebufferDesc)
	: m_FramebufferDesc(framebufferDesc), m_Device(device)
{
	Create();
}


BvFramebufferVk::~BvFramebufferVk()
{
	Destroy();
}


void BvFramebufferVk::Create()
{
	BvFixedVector<VkImageView, kMaxRenderTargetsWithDepth> imageViews(m_FramebufferDesc.m_RenderTargetViews.Size()
		+ (m_FramebufferDesc.m_pDepthStencilView ? 1 : 0));
	auto i = 0u;
	for (; i < m_FramebufferDesc.m_RenderTargetViews.Size(); i++)
	{
		imageViews[i] = m_FramebufferDesc.m_RenderTargetViews[i]->GetHandle();
	}
	if (m_FramebufferDesc.m_pDepthStencilView)
	{
		imageViews[i] = m_FramebufferDesc.m_pDepthStencilView->GetHandle();
	}

	decltype(auto) viewDesc = m_FramebufferDesc.m_RenderTargetViews.Size() > 0 ?
		m_FramebufferDesc.m_RenderTargetViews[0]->GetDesc() : m_FramebufferDesc.m_pDepthStencilView->GetDesc();
	decltype(auto) textureDesc = viewDesc.m_pTexture->GetDesc();

	VkFramebufferCreateInfo framebufferCI{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	framebufferCI.width = textureDesc.m_Size.width;
	framebufferCI.height = textureDesc.m_Size.height;
	framebufferCI.layers = textureDesc.m_Size.depthOrLayerCount;
	framebufferCI.renderPass = m_FramebufferDesc.m_RenderPass;
	framebufferCI.attachmentCount = (u32)imageViews.Size();
	framebufferCI.pAttachments = imageViews.Data();

	auto result = vkCreateFramebuffer(m_Device.GetHandle(), &framebufferCI, nullptr, &m_Framebuffer);
}


void BvFramebufferVk::Destroy()
{
	if (m_Framebuffer != VK_NULL_HANDLE)
	{
		vkDestroyFramebuffer(m_Device.GetHandle(), m_Framebuffer, nullptr);
		m_Framebuffer = VK_NULL_HANDLE;
	}
}


BvFramebufferManagerVk::BvFramebufferManagerVk()
{
}


BvFramebufferManagerVk::~BvFramebufferManagerVk()
{
	Destroy();
}


BvFramebufferVk * BvFramebufferManagerVk::GetFramebuffer(const BvRenderDeviceVk & device, const FramebufferDesc & desc)
{
	BvScopedLock lock(m_Lock);
	decltype(auto) pFramebuffer = m_Framebuffers[desc];

	if (pFramebuffer == nullptr)
	{
		pFramebuffer = new BvFramebufferVk(device, desc);
	}

	return pFramebuffer;
}


void BvFramebufferManagerVk::RemoveFramebuffer(const BvTextureViewVk * const pTextureView)
{
	BvScopedLock lock(m_Lock);
	for (const auto& pFramebuffer : m_Framebuffers)
	{
		decltype(auto) desc = pFramebuffer.second->GetDesc();
		for (auto pView : desc.m_RenderTargetViews)
		{
			if (pView == pTextureView)
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


void BvFramebufferManagerVk::Destroy()
{
	for (auto&& pFramebuffer : m_Framebuffers)
	{
		delete pFramebuffer.second;
	}
}


BvFramebufferManagerVk* GetFramebufferManager()
{
	static BvFramebufferManagerVk instance;
	return &instance;
}