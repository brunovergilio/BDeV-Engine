#pragma once


#include "BvCore/Container/BvVector.h"
#include "BvRender/BvRenderDefines.h"
#include "BvRender/RenderAPIs/Vulkan/BvVulkanTexture.h"


class BvVulkanTexture;


constexpr uint32_t kMaxRenderTargets = 8;


struct RenderTargetDesc
{
	uint32_t m_Width = 0;
	uint32_t m_Height = 0;
	uint32_t m_Layers = 0;

	BvVector<BvVulkanTexture *> m_Textures;
};


class BvVulkanRenderTarget
{
	BvVulkanRenderTarget() = default;
public:
	friend class BvVulkanSwapchain;

	BvVulkanRenderTarget(const BvVulkanDevice * const pDevice, const RenderTargetDesc & renderTargetDesc);
	~BvVulkanRenderTarget();

	void Create();
	void Destroy();

	BV_INLINE const RenderTargetDesc & GetRenderTargetDesc() const { return m_RenderTargetDesc; }
	BV_INLINE const VkRenderPass GetRenderPass() const { return m_RenderPass; }
	BV_INLINE const VkFramebuffer GetFramebuffer(const uint32_t index = 0) const { return m_Framebuffers.Size() > 0 ? 
		m_Framebuffers[index] : VK_NULL_HANDLE; }

private:
	void CreateRenderPass();
	void CreateFramebuffer();

private:
	const BvVulkanDevice * m_pDevice = nullptr;
	BvVector<VkFramebuffer> m_Framebuffers;
	VkRenderPass m_RenderPass = VK_NULL_HANDLE;
	RenderTargetDesc m_RenderTargetDesc;
};