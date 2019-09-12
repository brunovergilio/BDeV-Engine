#pragma once


#include "BvCore/Container/BvVector.h"
#include "BvRender/BvRenderDefines.h"
#include "BvRender/RenderAPIs/Vulkan/BvVulkanRenderTarget.h"


class BvVulkanDevice;
class BvWindow;


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

	BV_INLINE const BvVector<VkImageView> & GetViews() const { return m_SwapchainViews; }
	BV_INLINE const BvVector<VkImage> & GetImages() const { return m_SwapchainImages; }
	BV_INLINE const VkFormat GetColorFormat() const { return m_ColorFormat; }
	BV_INLINE const u32 GetCurrImage() const { return m_CurrImageIndex; }
	BV_INLINE const u32 GetWidth() const { return m_Width; }
	BV_INLINE const u32 GetHeight() const { return m_Height; }
	BV_INLINE const BvVulkanRenderTarget * const GetRenderTarget() const { return &m_RenderTarget; }

private:
	void CreateSurface(const BvWindow & window);
	void CreateDepthImage();
	void CreateRenderPass();
	void CreateFramebuffers();

	void DestroyDepthImage();
	void DestroyRenderPass();
	void DestroyFramebuffers();

private:
	const BvVulkanDevice * m_pDevice = nullptr;
	BvWindow * m_pWindow = nullptr;

	VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
	VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
	VkQueue m_PresentationQueue = VK_NULL_HANDLE;

	VkRenderPass m_RenderPass = VK_NULL_HANDLE;
	BvVector<VkFramebuffer> m_Framebuffers;

	BvVector<VkImage> m_SwapchainImages;
	BvVector<VkImageView> m_SwapchainViews;

	VkImage m_DepthImage = VK_NULL_HANDLE;
	VkDeviceMemory m_DepthImageMemory = VK_NULL_HANDLE;
	VkImageView m_DepthView = VK_NULL_HANDLE;

	VkFormat m_ColorFormat = VK_FORMAT_UNDEFINED;
	VkFormat m_DepthFormat = VK_FORMAT_UNDEFINED;

	BvVulkanRenderTarget m_RenderTarget;

	u32 m_Width = 0;
	u32 m_Height = 0;
	
	u32 m_CurrImageIndex;

	bool m_VSync = false;
};