#include "BvRenderVK/BvSwapchainVk.h"
#include "BDeV/Core/System/Window/BvWindow.h"
#include "BvRenderVk/BvUtilsVk.h"
#include "BvRenderVk/BvRenderDeviceVk.h"


#if (BV_PLATFORM == BV_PLATFORM_WIN32)


void BvSwapChainVk::CreateSurface()
{
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
	surfaceCreateInfo.hwnd = (HWND)m_pWindow->GetHandle();

	auto result = vkCreateWin32SurfaceKHR(m_pDevice->GetInstanceHandle(), &surfaceCreateInfo, nullptr, &m_Surface);
}


#endif