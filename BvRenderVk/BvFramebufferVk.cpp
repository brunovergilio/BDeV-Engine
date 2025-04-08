#include "BvFramebufferVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTextureViewVk.h"
#include "BvTypeConversionsVk.h"



BvFramebufferManagerVk::BvFramebufferManagerVk(VkDevice device)
	: m_Device(device)
{
}


BvFramebufferManagerVk::~BvFramebufferManagerVk()
{
	for (auto& fbPair : m_Framebuffers)
	{
		auto fb = fbPair.second;
		vkDestroyFramebuffer(m_Device, fb, nullptr);
	}
	m_Framebuffers.Clear();
}


VkFramebuffer BvFramebufferManagerVk::GetFramebuffer(const FramebufferDesc& framebufferDesc)
{
	auto& framebuffer = m_Framebuffers[framebufferDesc];
	if (framebuffer == VK_NULL_HANDLE)
	{
		VkFramebufferCreateInfo fbCI{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		fbCI.attachmentCount = (u32)framebufferDesc.m_Views.Size();
		fbCI.pAttachments = framebufferDesc.m_Views.Data();
		fbCI.renderPass = framebufferDesc.m_RenderPass;
		fbCI.width = framebufferDesc.m_Width;
		fbCI.height = framebufferDesc.m_Height;
		fbCI.layers = framebufferDesc.m_LayerCount;
		vkCreateFramebuffer(m_Device, &fbCI, nullptr, &framebuffer);
	}

	return framebuffer;
}


void BvFramebufferManagerVk::RemoveFramebuffersWithView(VkImageView view)
{
	bool found = false;
	do
	{
		for (auto& pair : m_Framebuffers)
		{
			for (auto i = 0u; i < pair.first.m_Views.Size(); ++i)
			{
				if (pair.first.m_Views[i] == view)
				{
					auto fb = pair.second;
					vkDestroyFramebuffer(m_Device, fb, nullptr);
					m_Framebuffers.Erase(pair.first);
					found = true;
					break;
				}
			}

			if (found)
			{
				break;
			}
		}
	} while (found);
}