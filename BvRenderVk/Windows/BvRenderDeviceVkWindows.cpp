#include "BvRenderVk/BvRenderDeviceVk.h"
#include <BDeV/Core/System/Window/BvMonitor.h>


#if (BV_PLATFORM == BV_PLATFORM_WIN32)


void BvRenderDeviceVk::SetupSupportedDisplayFormats()
{
	auto hInstance = GetModuleHandleW(nullptr);
	WNDCLASSW wc = {};
	wc.lpfnWndProc = DefWindowProcW;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"Dummy Vulkan Window Class";

	RegisterClassW(&wc);

	auto instance = GetInstanceHandle();
	HWND hWnd = nullptr;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	do
	{
		hWnd = CreateWindowExW(0, wc.lpszClassName, L"", WS_OVERLAPPEDWINDOW, 0, 0, 1, 1, nullptr, nullptr, hInstance, nullptr);
		if (!hWnd)
		{
			break;
		}

		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
		surfaceCreateInfo.hwnd = hWnd;
		auto result = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
		if (result != VK_SUCCESS)
		{
			break;
		}

		u32 formatCount{};
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, surface, &formatCount, nullptr);
		if (result != VK_SUCCESS)
		{
			break;
		}

		BvVector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, surface, &formatCount, surfaceFormats.Data());
		if (result != VK_SUCCESS)
		{
			break;
		}

		m_SupportedDisplayFormats.Reserve(formatCount);
		for (const auto& surfaceFormat : surfaceFormats)
		{
			// Add support for the most common types - can always add more later
			auto format = Format::kUnknown;
			switch (surfaceFormat.format)
			{
			case VK_FORMAT_R8G8B8A8_UNORM: format = Format::kRGBA8_UNorm; break;
			case VK_FORMAT_B8G8R8A8_UNORM: format = Format::kBGRA8_UNorm; break;
			case VK_FORMAT_R8G8B8A8_SRGB: format = Format::kRGBA8_UNorm_SRGB; break;
			case VK_FORMAT_B8G8R8A8_SRGB: format = Format::kBGRA8_UNorm_SRGB; break;
			case VK_FORMAT_R16G16B16A16_SFLOAT: format = Format::kRGBA16_Float; break;
			case VK_FORMAT_A2B10G10R10_UNORM_PACK32: format = Format::kRGB10A2_UNorm; break;
			}

			if (format != Format::kUnknown && !m_SupportedDisplayFormats.Contains(format))
			{
				m_SupportedDisplayFormats.EmplaceBack(format);
			}
		}

		if (m_pDeviceInfo->m_ExtendedSurfaceCaps.hasSurface2Caps)
		{
			auto& fullscreenCaps = m_pDeviceInfo->m_ExtendedSurfaceCaps.fullScreenExclusiveCaps;
			auto& fullscreenInfoWin32 = m_pDeviceInfo->m_ExtendedSurfaceCaps.fullScreenExclusiveInfoWin32;
			VkSurfaceCapabilities2KHR surfaceCaps2{ VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR };
			surfaceCaps2.pNext = &fullscreenCaps;
			VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR };
			surfaceInfo2.surface = surface;
			surfaceInfo2.pNext = &fullscreenInfoWin32;

			bool anyMonitorSupportsFullscreen = false;
			auto& monitors = BvMonitor::GetMonitors();
			
			for (auto i = 0; i < monitors.Size(); ++i)
			{
				fullscreenInfoWin32.hmonitor = monitors[i]->GetHandle();
				if (vkGetPhysicalDeviceSurfaceCapabilities2KHR(m_PhysicalDevice, &surfaceInfo2, &surfaceCaps2) == VK_SUCCESS
					&& m_pDeviceInfo->m_ExtendedSurfaceCaps.fullScreenExclusiveCaps.fullScreenExclusiveSupported)
				{
					anyMonitorSupportsFullscreen = true;
					break;
				}
			}

			if (anyMonitorSupportsFullscreen)
			{
				m_DeviceCaps |= ~RenderDeviceCapabilities::kTrueFullScreen;
			}
		}
	} while (0);

	if (surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(instance, surface, nullptr);
	}

	if (hWnd)
	{
		DestroyWindow(hWnd);
	}

	UnregisterClassW(wc.lpszClassName, hInstance);
}


#endif