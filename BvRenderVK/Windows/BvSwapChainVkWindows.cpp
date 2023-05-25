#include "BvRenderVK/BvSwapchainVk.h"
#include "BDeV/System/Window/BvWindow.h"
#include "BvRenderVk/BvUtilsVk.h"


#if (BV_PLATFORM == BV_PLATFORM_WIN32)


void BvSwapChainVk::CreateSurface()
{
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
	surfaceCreateInfo.hwnd = m_pWindow->GetHandle();

	auto result = vkCreateWin32SurfaceKHR(m_Device.GetInstanceHandle(), &surfaceCreateInfo, nullptr, &m_Surface);
	BvCheckErrorReturnVk(result, );
}


#endif