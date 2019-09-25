#pragma once


#include "BvRender/BvRenderDefines.h"
#include "BvCore/Container/BvFixedVector.h"


class BvVulkanDevice;


class BvVulkanRenderPass
{
public:
	BvVulkanRenderPass(const BvVulkanDevice * const pDevice, const VkRenderPassCreateInfo & createInfo);
	~BvVulkanRenderPass();

	const VkRenderPass Get() const { return m_RenderPass; }

private:
	void Create(const VkRenderPassCreateInfo & createInfo);
	void Destroy();

private:
	const BvVulkanDevice * const m_pDevice = nullptr;
	VkRenderPass m_RenderPass = VK_NULL_HANDLE;
};