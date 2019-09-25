#pragma once


#include "BvRender/BvRenderDefines.h"
#include "BvCore/Container/BvVector.h"


class BvVulkanDevice;
class BvWindow;
class BvVulkanTexture;
class BvVulkanFramebuffer;
class BvVulkanRenderPass;


class BvVulkanSwapchain
{
public:
	BvVulkanSwapchain(const BvVulkanDevice * const pDevice, BvWindow * const pWindow);
	~BvVulkanSwapchain();

	void Create(const bool vSync = false, const bool createDepthBuffer = true, const VkFormat depthFormat = VK_FORMAT_UNDEFINED);
	void Resize();
	void Destroy();

	void Prepare(u32 & currImageIndex, const VkSemaphore semaphore, const VkFence fence = VK_NULL_HANDLE);
	void Present(const VkSemaphore waitSemaphore);

	BV_INLINE const VkFormat GetColorFormat() const { return m_ColorFormat; }
	BV_INLINE const VkFormat GetDepthFormat() const { return m_DepthFormat; }
	BV_INLINE const u32 GetCurrImageIndex() const { return m_CurrImageIndex; }
	BV_INLINE const u32 GetWidth() const { return m_Width; }
	BV_INLINE const u32 GetHeight() const { return m_Height; }

	BV_INLINE const BvVector<BvVulkanFramebuffer *> & GetFramebuffers() const { return m_pFramebuffers; }

private:
	void CreateSurface(const BvWindow & window);
	void CreateSwapchainViews();
	void CreateDepthImage();
	void CreateFramebuffers();

private:
	const BvVulkanDevice * m_pDevice = nullptr;
	BvWindow * m_pWindow = nullptr;

	VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
	VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
	VkQueue m_PresentationQueue = VK_NULL_HANDLE;
	uint32_t m_PresentationQueueIndex = 0;

	BvVector<VkImage> m_SwapchainImages;
	//BvVector<VkImageView> m_SwapchainViews;
	BvVector<BvVulkanTexture *> m_pSwapchainTextures;

	BvVulkanRenderPass * m_pRenderPass = nullptr;
	BvVulkanTexture * m_pDepthTexture = nullptr;

	BvVector<BvVulkanFramebuffer *> m_pFramebuffers;

	VkFormat m_ColorFormat = VK_FORMAT_UNDEFINED;
	VkFormat m_DepthFormat = VK_FORMAT_UNDEFINED;

	u32 m_Width = 0;
	u32 m_Height = 0;
	
	u32 m_CurrImageIndex;

	bool m_VSync = false;
};