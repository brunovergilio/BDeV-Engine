#include "BvRender/RenderAPIs/Vulkan/BvVulkanSwapchain.h"
#include "BvRender/RenderAPIs/Vulkan/BvVulkanDevice.h"
#include "BvCore/System/Window/BvWindow.h"

#if (BV_PLATFORM == BV_PLATFORM_WIN32)


void BvVulkanSwapchain::CreateSurface(const BvWindow & window)
{
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
	surfaceCreateInfo.hwnd = window.GetHandle();

	vkCreateWin32SurfaceKHR(m_pDevice->GetInstance(), &surfaceCreateInfo, nullptr, &m_Surface);
}


#endif