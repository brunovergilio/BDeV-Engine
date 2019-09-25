#pragma once


#include "BvCore/Container/BvFixedVector.h"
#include "BvRender/BvRenderDefines.h"
#include "BvRender/RenderAPIs/Vulkan/BvVulkanRenderPass.h"


constexpr uint32_t kMaxFramebuffers = 8;


class BvVulkanTexture;


struct FramebufferDesc
{
	uint32_t m_Width = 0;
	uint32_t m_Height = 0;
	uint32_t m_Layers = 1;

	BvFixedVector<BvVulkanTexture *, kMaxFramebuffers> m_Textures;
};


class BvVulkanFramebuffer
{
public:
	friend class BvVulkanSwapchain;

	BvVulkanFramebuffer(const BvVulkanDevice * const pDevice, const FramebufferDesc & FramebufferDesc,
		BvVulkanRenderPass * const pRenderPass = nullptr);
	~BvVulkanFramebuffer();

	// Used for when a buffer resize is needed
	// Previous resources are destroyed (the render pass is still kept the same)
	void Recreate(const FramebufferDesc & FramebufferDesc, BvVulkanRenderPass * const pRenderPass = nullptr);

	BV_INLINE const VkRenderPass GetRenderPass() const { return m_pRenderPass ? m_pRenderPass->Get() : m_RenderPass; }
	BV_INLINE const VkFramebuffer Get() const { return m_Framebuffer; }
	BV_INLINE const FramebufferDesc & GetDesc() const { return m_FramebufferDesc; }

private:
	void Create();
	void Destroy();

private:
	void CreateRenderPass();
	void CreateFramebuffer();

private:
	const BvVulkanDevice * m_pDevice = nullptr;
	VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;
	VkRenderPass m_RenderPass = VK_NULL_HANDLE;
	BvVulkanRenderPass * m_pRenderPass = nullptr;
	FramebufferDesc m_FramebufferDesc{};
};