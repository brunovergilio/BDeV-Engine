#include "BvVulkanRenderPass.h"
#include "BvRender/RenderAPIs/Vulkan/BvVulkanDevice.h"


BvVulkanRenderPass::BvVulkanRenderPass(const BvVulkanDevice * const pDevice, const VkRenderPassCreateInfo & createInfo)
	: m_pDevice(pDevice)
{
	Create(createInfo);
}


BvVulkanRenderPass::~BvVulkanRenderPass()
{
	Destroy();
}


void BvVulkanRenderPass::Create(const VkRenderPassCreateInfo & createInfo)
{
	vkCreateRenderPass(m_pDevice->GetLogical(), &createInfo, nullptr, &m_RenderPass);
}


void BvVulkanRenderPass::Destroy()
{
	if (m_RenderPass)
	{
		vkDestroyRenderPass(m_pDevice->GetLogical(), m_RenderPass, nullptr);
		m_RenderPass = VK_NULL_HANDLE;
	}
}