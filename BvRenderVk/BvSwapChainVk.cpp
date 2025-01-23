#include "BvSwapChainVk.h"
#include "BvCommandQueueVk.h"
#include "BvUtilsVk.h"
#include "BvTextureVk.h"
#include "BvTypeConversionsVk.h"
#include "BvSemaphoreVk.h"
#include "BDeV/Core/System/Window/BvWindow.h"
#include "BvFramebufferVk.h"
#include "BvCommandContextVk.h"
#include "BvRenderDeviceVk.h"


BvSwapChainVk::BvSwapChainVk(BvRenderDeviceVk* pDevice, BvWindow* pWindow, const SwapChainDesc& swapChainParams, BvCommandContext* pContext)
	: BvSwapChain(pWindow, swapChainParams), m_pDevice(pDevice), m_pCommandContext(static_cast<BvCommandContextVk*>(pContext)),
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

	DestroySurface();
}


void BvSwapChainVk::AcquireImage()
{
	auto result = vkAcquireNextImageKHR(m_pDevice->GetHandle(), m_Swapchain, UINT64_MAX,
		m_ImageAcquiredSemaphores[m_CurrSemaphoreIndex].GetHandle(), VK_NULL_HANDLE, &m_CurrImageIndex);
	if (result != VkResult::VK_SUCCESS)
	{
		if (result == VkResult::VK_SUBOPTIMAL_KHR || result == VkResult::VK_ERROR_OUT_OF_DATE_KHR)
		{
			Resize();
			return;
		}
		else
		{
			BvDebugVkResult(result);
		}
	}

	if (m_CurrImageIndex != m_CurrSemaphoreIndex)
	{
		std::swap(m_ImageAcquiredSemaphores[m_CurrImageIndex], m_ImageAcquiredSemaphores[m_CurrSemaphoreIndex]);
		std::swap(m_RenderCompleteSemaphores[m_CurrImageIndex], m_RenderCompleteSemaphores[m_CurrSemaphoreIndex]);
	}
}


void BvSwapChainVk::Present(bool vSync)
{
	if (!m_pWindow->IsVisible())
	{
		return;
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_Swapchain;
	presentInfo.pImageIndices = &m_CurrImageIndex;

	VkSemaphore semaphores[] = { GetCurrentRenderCompleteSemaphore()->GetHandle() };
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
			BvDebugVkResult(result);
		}
	}

	m_CurrSemaphoreIndex = (m_CurrSemaphoreIndex + 1) % (u32)m_SwapChainTextures.Size();
}


BvRenderDevice* BvSwapChainVk::GetDevice()
{
	return m_pDevice;
}


bool BvSwapChainVk::Create()
{
	for (auto pView : m_SwapChainTextureViews)
	{
		m_pDevice->GetFramebufferManager()->RemoveFramebuffersWithView(pView->GetHandle());
	}

	u32 width = m_pWindow->GetWidth();
	u32 height = m_pWindow->GetHeight();

	auto device = m_pDevice->GetHandle();
	auto physicalDevice = m_pDevice->GetPhysicalDeviceHandle();

	VkBool32 presentationSupported = VK_FALSE;
	u32 queueFamilyIndex = m_pCommandQueue->GetFamilyIndex();
	auto result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, m_Surface, &presentationSupported);
	BvCheckErrorReturnVk(result, false);
	if (!presentationSupported)
	{
		return false;
	}

	// Get list of supported surface formats
	u32 formatCount{};
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, nullptr);
	BvCheckErrorReturnVk(result, false);

	BvVector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, surfaceFormats.Data());
	BvCheckErrorReturnVk(result, false);

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
				break;
			}
		}

		// If the requested format is not available, we can try to look for a replacement
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

			// If we still couldn't find anything, then throw an error
			if (format == VkFormat::VK_FORMAT_UNDEFINED)
			{
				BV_ASSERT(false, "Couldn't find a format for Vulkan's SwapChain");
				return false;
			}
		}
	}

	// Update the SwapChain's format in case it changed - if it ends up being Format::Unknown,
	// then that means it's using a format not on the list, but nevertheless supported
	m_SwapChainDesc.m_Format = GetFormat(format);

	// Get physical device surface properties and formats
	VkSurfaceCapabilitiesKHR surfCaps;
	result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface, &surfCaps);
	BvCheckErrorReturnVk(result, false);

	// Get available present modes
	u32 presentModeCount;
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, nullptr);
	BvCheckErrorReturnVk(result, false);
	BV_ASSERT(presentModeCount > 0, "No present modes");

	BvVector<VkPresentModeKHR> presentModes(presentModeCount);
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, presentModes.Data());
	BvCheckErrorReturnVk(result, false);

	VkExtent2D swapchainExtent = {};
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
			if ((swapchainPresentMode != VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR)
				&& (presentModes[i] == VkPresentModeKHR::VK_PRESENT_MODE_IMMEDIATE_KHR))
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
	swapchainCreateInfo.pNext = nullptr;
	swapchainCreateInfo.surface = m_Surface;
	swapchainCreateInfo.minImageCount = m_SwapChainDesc.m_SwapChainImageCount;
	swapchainCreateInfo.imageFormat = format;
	swapchainCreateInfo.imageColorSpace = colorSpace;
	swapchainCreateInfo.imageExtent = swapchainExtent;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
	swapchainCreateInfo.imageArrayLayers = 1;

	swapchainCreateInfo.imageSharingMode = queueFamilyIndex == m_pDevice->GetDeviceInfo()->m_GraphicsQueueInfo.m_QueueFamilyIndex ?
		VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
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
	textureDesc.m_UsageFlags = TextureUsage::kColorTarget;

	// Enable transfer source on swap chain images if supported
	if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
	{
		swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		textureDesc.m_UsageFlags |= TextureUsage::kTransferSrc;
	}

	// Enable transfer destination on swap chain images if supported
	if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
	{
		swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		textureDesc.m_UsageFlags |= TextureUsage::kTransferDst;
	}

	result = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &m_Swapchain);
	BvCheckErrorReturnVk(result, false);

	// If an existing swap chain is re-created, destroy the old swap chain
	// This also cleans up all the presentable images
	if (oldSwapchain != VK_NULL_HANDLE)
	{
		for (auto i = 0u; i < m_SwapChainTextures.Size(); i++)
		{
			delete m_SwapChainTextureViews[i];
			delete m_SwapChainTextures[i];
		}
		vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
	}

	u32 imageCount = 0;
	result = vkGetSwapchainImagesKHR(device, m_Swapchain, &imageCount, nullptr);
	BvCheckErrorReturnVk(result, false);

	// Get the swap chain images
	m_SwapChainTextures.Resize(imageCount);
	m_SwapChainTextureViews.Resize(imageCount);
	BvVector<VkImage> swapChainImages(imageCount);
	result = vkGetSwapchainImagesKHR(device, m_Swapchain, &imageCount, swapChainImages.Data());
	BvCheckErrorReturnVk(result, false);

	TextureViewDesc textureViewDesc;
	textureViewDesc.m_Format = textureDesc.m_Format;

	for (auto i = 0u; i < imageCount; i++)
	{
		m_SwapChainTextures[i] = new BvTextureVk(m_pDevice, this, textureDesc, swapChainImages[i]);

		textureViewDesc.m_pTexture = m_SwapChainTextures[i];
		m_SwapChainTextureViews[i] = new BvTextureViewVk(m_pDevice, textureViewDesc);
	}

	CreateSynchronizationResources();

	AcquireImage();

	return true;
}


void BvSwapChainVk::Destroy()
{
	m_pCommandQueue->WaitIdle();

	DestroySynchronizationResources();

	for (auto&& pTextureView : m_SwapChainTextureViews)
	{
		auto pResource = static_cast<BvTextureViewVk*>(pTextureView);
		delete pResource;
		pTextureView = nullptr;
	}

	for (auto&& pTexture : m_SwapChainTextures)
	{
		delete pTexture;
	}

	vkDestroySwapchainKHR(m_pDevice->GetHandle(), m_Swapchain, nullptr);
	m_Swapchain = VK_NULL_HANDLE;
}


void BvSwapChainVk::DestroySurface()
{
	vkDestroySurfaceKHR(m_pDevice->GetInstanceHandle(), m_Surface, nullptr);
	m_Surface = VK_NULL_HANDLE;
}


void BvSwapChainVk::Resize()
{
	if (!m_pWindow->IsValid())
	{
		return;
	}
	m_pCommandQueue->WaitIdle();

	Create();
}


void BvSwapChainVk::CreateSynchronizationResources()
{
	DestroySynchronizationResources();

	m_ImageAcquiredSemaphores.Reserve(m_SwapChainTextures.Size());
	for (auto i = 0u; i < m_SwapChainTextures.Size(); ++i)
	{
		m_ImageAcquiredSemaphores.EmplaceBack(BvSemaphoreVk(m_pDevice->GetHandle(), 0, false));
	}

	m_RenderCompleteSemaphores.Reserve(m_SwapChainTextures.Size());
	for (auto i = 0u; i < m_SwapChainTextures.Size(); ++i)
	{
		m_RenderCompleteSemaphores.EmplaceBack(BvSemaphoreVk(m_pDevice->GetHandle(), 0, false));
	}
}


void BvSwapChainVk::DestroySynchronizationResources()
{
	m_ImageAcquiredSemaphores.Clear();
	m_RenderCompleteSemaphores.Clear();
}