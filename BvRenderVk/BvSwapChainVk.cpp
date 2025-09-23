#include "BvSwapChainVk.h"
#include "BvCommandQueueVk.h"
#include "BvUtilsVk.h"
#include "BvTextureVk.h"
#include "BvTypeConversionsVk.h"
#include "BDeV/Core/System/Window/BvWindow.h"
#include "BvFramebufferVk.h"
#include "BvCommandContextVk.h"
#include "BvRenderDeviceVk.h"
#include "BvRenderEngineVk.h"
#include "BvGPUFenceVk.h"
#include "BDeV/Core/System/Window/BvMonitor.h"


BV_VK_DEVICE_RES_DEF(BvSwapChainVk)


BvSwapChainVk::BvSwapChainVk(BvRenderDeviceVk* pDevice, BvWindow* pWindow, const SwapChainDesc& swapChainParams, BvCommandContextVk* pContext)
	: m_pWindow(pWindow), m_SwapChainDesc(swapChainParams), m_pDevice(pDevice), m_pCommandContext(pContext),
	m_pCommandQueue(static_cast<BvCommandContextVk*>(pContext)->GetCommandQueue())
{
	CreateSurface();

	Create();

	m_pCommandContext->AddSwapChain(this);
}


BvSwapChainVk::~BvSwapChainVk()
{
	m_pCommandContext->RemoveSwapChain(this);

	Destroy();
}


void BvSwapChainVk::AcquireImage()
{
	if (!m_pWindow->IsVisible() || m_IsReady)
	{
		return;
	}

	auto& fenceData = m_Fences[m_CurrImageIndex];
	if (fenceData.m_pFence)
	{
		fenceData.m_pFence->Wait(fenceData.m_Value);
	}

	auto result = vkAcquireNextImageKHR(m_pDevice->GetHandle(), m_Swapchain, UINT64_MAX,
		m_ImageAcquiredSemaphores[m_CurrSemaphoreIndex], VK_NULL_HANDLE, &m_CurrImageIndex);
	if (result != VkResult::VK_SUCCESS)
	{
		if (result == VkResult::VK_SUBOPTIMAL_KHR || result == VkResult::VK_ERROR_OUT_OF_DATE_KHR)
		{
			Resize();
			return;
		}
		else
		{
			BV_ASSERT(result == VK_SUCCESS, "Failed to acquire a swap chain image");
		}
	}

	if (m_CurrImageIndex != m_CurrSemaphoreIndex)
	{
		std::swap(m_ImageAcquiredSemaphores[m_CurrImageIndex], m_ImageAcquiredSemaphores[m_CurrSemaphoreIndex]);
		std::swap(m_RenderCompleteSemaphores[m_CurrImageIndex], m_RenderCompleteSemaphores[m_CurrSemaphoreIndex]);
	}

	m_IsReady = true;
}


void BvSwapChainVk::Present(bool vSync)
{
	if (!m_pWindow->IsVisible() || !m_IsReady)
	{
		return;
	}
	m_IsReady = false;

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_Swapchain;
	presentInfo.pImageIndices = &m_CurrImageIndex;

	VkSemaphore semaphores[] = { GetCurrentRenderCompleteSemaphore() };
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = semaphores;

	auto result = VK_SUCCESS;
	if (vSync == m_SwapChainDesc.m_VSync)
	{
		result = vkQueuePresentKHR(m_pCommandQueue->GetHandle(), &presentInfo);
	}
	else
	{
		m_SwapChainDesc.m_VSync = vSync;
		result = VkResult::VK_ERROR_OUT_OF_DATE_KHR;
	}

	if (result != VkResult::VK_SUCCESS)
	{
		if (result == VkResult::VK_SUBOPTIMAL_KHR || result == VkResult::VK_ERROR_OUT_OF_DATE_KHR)
		{
			Resize();
			return;
		}
		else
		{
			BV_ASSERT(result == VK_SUCCESS, "Failed to present swap chain image");
		}
	}

	m_CurrSemaphoreIndex = (m_CurrSemaphoreIndex + 1) % (u32)m_SwapChainTextures.Size();
}


void BvSwapChainVk::SetWindowMode(SwapChainMode mode, BvMonitor* pMonitor)
{
	auto pCurrMonitor = m_pWindow->GetWindowDesc().m_pMonitor;
	if (!pMonitor)
	{
		pMonitor = pCurrMonitor;
	}

	if (m_SwapChainDesc.m_WindowMode == mode)
	{
		if (mode == SwapChainMode::kWindowed || pMonitor == pCurrMonitor)
		{
			return;
		}
	}

	m_pCommandQueue->WaitIdle();
	m_SwapChainDesc.m_WindowMode = mode;
	SetTrueFullscreen(false);

	m_pWindow->SetFullscreen(mode != SwapChainMode::kWindowed, pMonitor);
	Create();
}


void BvSwapChainVk::SetCurrentFence(BvGPUFenceVk* pFence, u64 value)
{
	m_Fences[m_CurrImageIndex] = FenceData{ pFence, value };
}


bool BvSwapChainVk::Create()
{
	for (auto pView : m_SwapChainTextureViews)
	{
		m_pCommandContext->RemoveFramebuffers(pView->GetHandle());
	}

	auto device = m_pDevice->GetHandle();
	auto physicalDevice = m_pDevice->GetPhysicalDeviceHandle();

	VkBool32 presentationSupported = VK_FALSE;
	u32 queueFamilyIndex = m_pCommandContext->GetGroupIndex();
	auto result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, m_Surface, &presentationSupported);
	BV_ASSERT(result == VK_SUCCESS, "Failed to check for surface support");
	if (!presentationSupported)
	{
		return false;
	}

	// Get list of supported surface formats
	u32 formatCount{};
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, nullptr);
	BV_ASSERT(result == VK_SUCCESS, "Failed to retrieve surface formats");

	BvVector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, surfaceFormats.Data());
	BV_ASSERT(result == VK_SUCCESS, "Failed to retrieve surface formats");

	// 
	VkFormat requestedFormat = GetVkFormat(m_SwapChainDesc.m_Format);
	if (requestedFormat == VkFormat::VK_FORMAT_UNDEFINED)
	{
		// We default to VkFormat::VK_FORMAT_B8G8R8A8_UNORM since it's one of
		// the most commonly accepted formats for a swap chain
		requestedFormat = VK_FORMAT_B8G8R8A8_UNORM;
	}

	VkFormat format = VkFormat::VK_FORMAT_UNDEFINED;
	VkColorSpaceKHR colorSpace = VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
	{
		// If there's one surface format, with type VK_FORMAT_UNDEFINED, we use
		// whatever was specified in the SwapChainDesc
		format = requestedFormat;
		colorSpace = surfaceFormats[0].colorSpace;
	}
	else
	{
		// Otherwise, we look for the requested format
		for (const auto& surfaceFormat : surfaceFormats)
		{
			if (surfaceFormat.format == requestedFormat)
			{
				format = surfaceFormat.format;
				colorSpace = surfaceFormat.colorSpace;

				// We can stop looking if either is true:
				// 1) We want an HDR color space and found one
				// 2) We don't want to use HDR and found a default color space
				// Formats such as VK_FORMAT_A2B10G10R10_UNORM_PACK32 can be used with different color spaces,
				// so doing this can help us choose the proper option based on the swap chain's parameters
				if ((m_SwapChainDesc.m_PreferHDR && surfaceFormat.colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
					|| (!m_SwapChainDesc.m_PreferHDR && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR))
				{
					break;
				}
			}
		}

		// If the requested format is not available, see if we can get a "replacement"
		if (format == VkFormat::VK_FORMAT_UNDEFINED)
		{
			// We can try looking for RGBA formats if BGRA are not available and vice versa
			switch (requestedFormat)
			{
			case VkFormat::VK_FORMAT_R8G8B8A8_UNORM: requestedFormat = VkFormat::VK_FORMAT_B8G8R8A8_UNORM;	break;
			case VkFormat::VK_FORMAT_B8G8R8A8_UNORM: requestedFormat = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;	break;
			case VkFormat::VK_FORMAT_R8G8B8A8_SRGB: requestedFormat = VkFormat::VK_FORMAT_B8G8R8A8_SRGB;	break;
			case VkFormat::VK_FORMAT_B8G8R8A8_SRGB: requestedFormat = VkFormat::VK_FORMAT_R8G8B8A8_SRGB;	break;
			}

			for (const auto& surfaceFormat : surfaceFormats)
			{
				if (surfaceFormat.format == requestedFormat)
				{
					format = surfaceFormat.format;
					colorSpace = surfaceFormat.colorSpace;
					break;
				}
			}

			// If we still couldn't find anything, default to VK_FORMAT_B8G8R8A8_UNORM
			if (format == VkFormat::VK_FORMAT_UNDEFINED)
			{
				format = VK_FORMAT_B8G8R8A8_UNORM;
				colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
				m_SwapChainDesc.m_Format = Format::kBGRA8_UNorm;
			}
		}
	}

	// Update the SwapChain's format in case it changed - if it ends up being Format::Unknown,
	// then that means it's using a format not on the list, but nevertheless supported
	switch (format)
	{
	case VK_FORMAT_R8G8B8A8_UNORM: m_SwapChainDesc.m_Format = Format::kRGBA8_UNorm; break;
	case VK_FORMAT_B8G8R8A8_UNORM: m_SwapChainDesc.m_Format = Format::kBGRA8_UNorm; break;
	case VK_FORMAT_R8G8B8A8_SRGB: m_SwapChainDesc.m_Format = Format::kRGBA8_UNorm_SRGB; break;
	case VK_FORMAT_B8G8R8A8_SRGB: m_SwapChainDesc.m_Format = Format::kBGRA8_UNorm_SRGB; break;
	case VK_FORMAT_R16G16B16A16_SFLOAT: m_SwapChainDesc.m_Format = Format::kRGBA16_Float; break;
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32: m_SwapChainDesc.m_Format = Format::kRGB10A2_UNorm; break;
	default: m_SwapChainDesc.m_Format = Format::kUnknown; break;
	}

	bool supportsTrueFullscreen = m_pDevice->GetDeviceInfo()->m_ExtendedSurfaceCaps.fullScreenExclusiveCaps.fullScreenExclusiveSupported;
	if (!supportsTrueFullscreen && m_SwapChainDesc.m_WindowMode == SwapChainMode::kFullscreen)
	{
		m_SwapChainDesc.m_WindowMode = SwapChainMode::kBorderlessFullscreen;
	}

	VkSurfaceFullScreenExclusiveInfoEXT surfaceFS{ VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT };
	surfaceFS.fullScreenExclusive = GetVkFullScreenExclusiveEXTMode(m_SwapChainDesc.m_WindowMode);
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	VkSurfaceFullScreenExclusiveWin32InfoEXT surfaceFSWin32{ VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT };
	if (supportsTrueFullscreen)
	{
		surfaceFS.pNext = &surfaceFSWin32;
		surfaceFSWin32.hmonitor = BvMonitor::FromWindow(m_pWindow)->GetHandle();
	}
#endif

	// Get physical device surface properties and formats
	VkSurfaceCapabilitiesKHR surfCaps{};
	result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface, &surfCaps);
	BV_ASSERT(result == VK_SUCCESS, "Failed to check for surface caps");

	// Get available present modes
	u32 presentModeCount;
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, nullptr);
	BV_ASSERT(result == VK_SUCCESS, "Failed to retrieve surface present modes");
	BV_ASSERT(presentModeCount > 0, "No present modes");

	BvVector<VkPresentModeKHR> presentModes(presentModeCount);
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, presentModes.Data());
	BV_ASSERT(result == VK_SUCCESS, "Failed to retrieve surface present modes");

	u32 width = m_pWindow->GetWidth();
	u32 height = m_pWindow->GetHeight();
	VkExtent2D swapchainExtent = {};
	if (m_SwapChainDesc.m_WindowMode == SwapChainMode::kWindowed)
	{
		// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
		if (surfCaps.currentExtent.width == u32(-1))
		{
			// If the surface size is undefined, the size is set to
			// the size of the images requested.
			swapchainExtent.width = width;
			swapchainExtent.height = height;
		}
		else
		{
			// If the surface size is defined, the swap chain size must match
			swapchainExtent = surfCaps.currentExtent;

			// Sometimes when restoring from a minimized state, the swap chain won't pick up
			// the latest changes from the window, so force it here
			if (swapchainExtent.width == 0 || swapchainExtent.height == 0)
			{
				swapchainExtent.width = width;
				swapchainExtent.height = height;
			}
		}
	}
	else
	{
		swapchainExtent = surfCaps.maxImageExtent;
	}

	// Select a present mode for the swapchain

	// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
	// This mode waits for the vertical blank ("v-sync")
	VkPresentModeKHR swapchainPresentMode = VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;

	// If v-sync is not requested, try to find a mailbox mode
	// It's the lowest latency non-tearing present mode available
	if (!m_SwapChainDesc.m_VSync)
	{
		for (size_t i = 0; i < presentModeCount; i++)
		{
			if (presentModes[i] == VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR)
			{
				swapchainPresentMode = VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}
			else if (presentModes[i] == VkPresentModeKHR::VK_PRESENT_MODE_IMMEDIATE_KHR)
			{
				swapchainPresentMode = VkPresentModeKHR::VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}
	}

	// Determine the number of images
	// Ask for at least minImageCount + 1, so if less was specified, change it
	if (m_SwapChainDesc.m_SwapChainImageCount <= surfCaps.minImageCount)
	{
		m_SwapChainDesc.m_SwapChainImageCount = surfCaps.minImageCount + 1;
	}
	// If there's a maximum limit and we went over it, clamp it
	if ((surfCaps.maxImageCount > 0) && (m_SwapChainDesc.m_SwapChainImageCount > surfCaps.maxImageCount))
	{
		m_SwapChainDesc.m_SwapChainImageCount = surfCaps.maxImageCount;
	}

	// Find the transformation of the surface
	VkSurfaceTransformFlagsKHR preTransform;
	if (surfCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		// We prefer a non-rotated transform
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = surfCaps.currentTransform;
	}

	// Find a supported composite alpha format (not all devices support alpha opaque)
	VkCompositeAlphaFlagBitsKHR compositeAlpha = VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	// Simply select the first composite alpha format available
	VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[] =
	{
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};

	for (auto& compositeAlphaFlag : compositeAlphaFlags)
	{
		if (surfCaps.supportedCompositeAlpha & compositeAlphaFlag)
		{
			compositeAlpha = compositeAlphaFlag;
			break;
		}
	}

	VkSwapchainKHR oldSwapchain = m_Swapchain;

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = supportsTrueFullscreen ? &surfaceFS : nullptr;
	swapchainCreateInfo.surface = m_Surface;
	swapchainCreateInfo.minImageCount = m_SwapChainDesc.m_SwapChainImageCount;
	swapchainCreateInfo.imageFormat = format;
	swapchainCreateInfo.imageColorSpace = colorSpace;
	swapchainCreateInfo.imageExtent = swapchainExtent;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
	swapchainCreateInfo.imageArrayLayers = 1;

	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount = 0;
	swapchainCreateInfo.pQueueFamilyIndices = nullptr;

	swapchainCreateInfo.presentMode = swapchainPresentMode;
	swapchainCreateInfo.oldSwapchain = oldSwapchain;
	// Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.compositeAlpha = compositeAlpha;

	TextureDesc textureDesc;
	textureDesc.m_Size = { swapchainExtent.width, swapchainExtent.height, 1 };
	textureDesc.m_Format = m_SwapChainDesc.m_Format;
	textureDesc.m_UsageFlags = TextureUsage::kRenderTarget;

	// Enable transfer source on swap chain images if supported
	if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
	{
		swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	// Enable transfer destination on swap chain images if supported
	if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
	{
		swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	result = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &m_Swapchain);
	BV_ASSERT(result == VK_SUCCESS, "Failed to create a swap chain");

	// If an existing swap chain is re-created, destroy the old swap chain
	// This also cleans up all the presentable images
	if (oldSwapchain != VK_NULL_HANDLE)
	{
		for (auto i = 0u; i < m_SwapChainTextures.Size(); i++)
		{
			m_SwapChainTextureViews[i]->Release();
			m_SwapChainTextures[i]->Release();
		}
		vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
	}

	u32 imageCount = 0;
	result = vkGetSwapchainImagesKHR(device, m_Swapchain, &imageCount, nullptr);
	BV_ASSERT(result == VK_SUCCESS, "Failed to retrieve swap chain images");

	// Get the swap chain images
	m_SwapChainTextures.Resize(imageCount);
	m_SwapChainTextureViews.Resize(imageCount);
	BvVector<VkImage> swapChainImages(imageCount);
	result = vkGetSwapchainImagesKHR(device, m_Swapchain, &imageCount, swapChainImages.Data());
	if (result != VK_SUCCESS && m_SwapChainDesc.m_WindowMode == SwapChainMode::kFullscreen)
	{
		m_SwapChainDesc.m_WindowMode = SwapChainMode::kBorderlessFullscreen;
		swapchainCreateInfo.pNext = nullptr;

		result = vkGetSwapchainImagesKHR(device, m_Swapchain, &imageCount, swapChainImages.Data());
	}
	BV_ASSERT(result == VK_SUCCESS, "Failed to retrieve swap chain images");

	TextureViewDesc textureViewDesc;
	textureViewDesc.m_Format = textureDesc.m_Format;

	for (auto i = 0u; i < imageCount; i++)
	{
		m_SwapChainTextures[i] = m_pDevice->CreateResource<BvTextureVk>(m_pDevice, this, textureDesc, swapChainImages[i]);

		textureViewDesc.m_pTexture = m_SwapChainTextures[i];
		m_pDevice->CreateTextureView<BvTextureViewVk>(textureViewDesc, &m_SwapChainTextureViews[i]);
	}

	CreateSynchronizationResources();

	if (m_SwapChainDesc.m_WindowMode == SwapChainMode::kFullscreen)
	{
		SetTrueFullscreen(true);
	}

	m_IsReady = false;
	AcquireImage();

	return true;
}


void BvSwapChainVk::Destroy()
{
	if (m_Swapchain)
	{
		m_pCommandQueue->WaitIdle();

		SetTrueFullscreen(false);

		DestroySynchronizationResources();

		for (auto& pTextureView : m_SwapChainTextureViews)
		{
			m_pCommandContext->RemoveFramebuffers(pTextureView->GetHandle());
			pTextureView->Release();
			pTextureView = nullptr;
		}
		m_SwapChainTextureViews.Clear();

		for (auto& pTexture : m_SwapChainTextures)
		{
			pTexture->Release();
			pTexture = nullptr;
		}
		m_SwapChainTextures.Clear();

		vkDestroySwapchainKHR(m_pDevice->GetHandle(), m_Swapchain, nullptr);
		m_Swapchain = VK_NULL_HANDLE;
		
		DestroySurface();
	}
}


void BvSwapChainVk::DestroySurface()
{
	if (m_Surface)
	{
		vkDestroySurfaceKHR(m_pDevice->GetEngine()->GetHandle(), m_Surface, nullptr);
		m_Surface = VK_NULL_HANDLE;
	}
}


void BvSwapChainVk::Resize()
{
	m_pCommandQueue->WaitIdle();
	if (m_pWindow->IsClosed() || m_pWindow->IsMinimized())
	{
		return;
	}

	SetTrueFullscreen(false);
	Create();
}


void BvSwapChainVk::SetTrueFullscreen(bool value)
{
	auto device = m_pDevice->GetHandle();
	if (value)
	{
		if (!m_FullscreenAcquired)
		{
			vkAcquireFullScreenExclusiveModeEXT(device, m_Swapchain);
			m_FullscreenAcquired = true;
		}
	}
	else
	{
		if (m_FullscreenAcquired)
		{
			vkReleaseFullScreenExclusiveModeEXT(device, m_Swapchain);
			m_FullscreenAcquired = false;
		}
	}
}


void BvSwapChainVk::CreateSynchronizationResources()
{
	DestroySynchronizationResources();

	m_ImageAcquiredSemaphores.Reserve(m_SwapChainTextures.Size());
	m_RenderCompleteSemaphores.Reserve(m_SwapChainTextures.Size());
	VkSemaphoreCreateInfo semaphoreCI{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	auto device = m_pDevice->GetHandle();
	for (auto i = 0u; i < m_SwapChainTextures.Size(); ++i)
	{
		vkCreateSemaphore(device, &semaphoreCI, nullptr, &m_ImageAcquiredSemaphores.EmplaceBack());
		vkCreateSemaphore(device, &semaphoreCI, nullptr, &m_RenderCompleteSemaphores.EmplaceBack());
	}

	m_Fences.Resize(m_SwapChainTextures.Size());
}


void BvSwapChainVk::DestroySynchronizationResources()
{
	auto device = m_pDevice->GetHandle();
	for (auto i = 0u; i < m_ImageAcquiredSemaphores.Size(); ++i)
	{
		vkDestroySemaphore(device, m_ImageAcquiredSemaphores[i], nullptr);
		vkDestroySemaphore(device, m_RenderCompleteSemaphores[i], nullptr);
	}

	m_ImageAcquiredSemaphores.Clear();
	m_RenderCompleteSemaphores.Clear();
	m_Fences.Clear();
}