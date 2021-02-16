#include "BvRenderVK/BvSwapchainVk.h"
#include "BvCore/System/Window/BvNativeWindow.h"
#include "BvRenderVk/BvUtilsVk.h"


#if (BV_PLATFORM == BV_PLATFORM_WIN32)


void BvSwapChainVk::CreateSurface()
{
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
	surfaceCreateInfo.hwnd = m_Window.GetHandle();

	auto result = VulkanFunctions::vkCreateWin32SurfaceKHR(m_Device.GetInstanceHandle(), &surfaceCreateInfo, nullptr, &m_Surface);
	BvCheckErrorReturnVk(result, );
}


#endif