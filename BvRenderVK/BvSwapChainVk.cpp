#include "BvSwapChainVk.h"
#include "BvCommandQueueVk.h"
#include "BvUtilsVk.h"
#include "BvTextureVk.h"
#include "BvTextureViewVk.h"
#include "BvTypeConversionsVk.h"
#include "BvFenceVk.h"
#include "BvSemaphoreVk.h"


BvSwapChainVk::BvSwapChainVk(const BvRenderDeviceVk & device, BvCommandQueueVk & commandQueue, const SwapChainDesc & swapChainParams)
	: BvSwapChain(swapChainParams), m_Device(device), m_CommandQueue(commandQueue),
	m_PresentationQueueIndex(device.GetGPUInfo().m_PresentationQueueIndex)
{
	CreateSurface();
}


BvSwapChainVk::~BvSwapChainVk()
{
	Destroy();

	DestroySurface();
}


bool BvSwapChainVk::Create()
{
	m_SwapChainDesc.m_WindowDesc = m_Window.GetWindowDesc();

	auto device = m_Device.GetHandle();
	auto physicalDevice = m_Device.GetGPUInfo().m_PhysicalDevice;

	VkBool32 presentationSupported = VK_FALSE;
	auto result = VulkanFunctions::vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, m_Device.GetGPUInfo().m_PresentationQueueIndex, m_Surface, &presentationSupported);
	BvCheckErrorReturnVk(result, false);
	if (!presentationSupported)
	{
		return false;
	}

	// Get list of supported surface formats
	u32 formatCount{};
	result = VulkanFunctions::vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, nullptr);
	BvCheckErrorReturnVk(result, false);

	BvVector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	result = VulkanFunctions::vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, surfaceFormats.Data());
	BvCheckErrorReturnVk(result, false);

	VkFormat chosenFormat = m_SwapChainDesc.m_Format != Format::kUndefined ?
		GetVkFormat(m_SwapChainDesc.m_Format) : VkFormat::VK_FORMAT_B8G8R8A8_UNORM;

	// If there's one surface format, with type VK_FORMAT_UNDEFINED, we use
	// whatever was specified in the SwapChainDesc
	VkFormat format = VkFormat::VK_FORMAT_UNDEFINED;
	VkColorSpaceKHR colorSpace = VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
	{
		format = chosenFormat;
		colorSpace = surfaceFormats[0].colorSpace;
	}
	// Otherwise, we look for the chosen format or whatever we find first
	else
	{
		// Default to the first one
		format = surfaceFormats[0].format;
		colorSpace = surfaceFormats[0].colorSpace;
		for (auto & surfaceFormat : surfaceFormats)
		{
			if (surfaceFormat.format == chosenFormat)
			{
				format = surfaceFormat.format;
				colorSpace = surfaceFormat.colorSpace;
				break;
			}
		}

		m_SwapChainDesc.m_Format = GetFormat(format);
	}

	// Get physical device surface properties and formats
	VkSurfaceCapabilitiesKHR surfCaps;
	result = VulkanFunctions::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface, &surfCaps);
	BvCheckErrorReturnVk(result, false);

	// Get available present modes
	u32 presentModeCount;
	result = VulkanFunctions::vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, nullptr);
	BvCheckErrorReturnVk(result, false);
	BvAssert(presentModeCount > 0, "No present modes");

	BvVector<VkPresentModeKHR> presentModes(presentModeCount);
	result = VulkanFunctions::vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, presentModes.Data());
	BvCheckErrorReturnVk(result, false);

	VkExtent2D swapchainExtent = {};
	// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
	if (surfCaps.currentExtent.width == u32(-1))
	{
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
		swapchainExtent.width = m_SwapChainDesc.m_WindowDesc.m_Width;
		swapchainExtent.height = m_SwapChainDesc.m_WindowDesc.m_Height;
	}
	else
	{
		// If the surface size is defined, the swap chain size must match
		swapchainExtent = surfCaps.currentExtent;

		// Sometimes when restoring from a minimized state, the swap chain won't pick up
		// the latest changes from the window, so force it here
		if (swapchainExtent.width == 0 || swapchainExtent.height == 0)
		{
			swapchainExtent.width = m_SwapChainDesc.m_WindowDesc.m_Width;
			swapchainExtent.height = m_SwapChainDesc.m_WindowDesc.m_Height;
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

	for (auto & compositeAlphaFlag : compositeAlphaFlags)
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
	swapchainCreateInfo.imageExtent = { swapchainExtent.width, swapchainExtent.height };
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
	textureDesc.m_Size = { m_SwapChainDesc.m_WindowDesc.m_Width, m_SwapChainDesc.m_WindowDesc.m_Height, 1 };
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

	result = m_Device.GetDeviceFunctions().vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &m_Swapchain);
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
		m_Device.GetDeviceFunctions().vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
	}

	u32 imageCount = 0;
	result = m_Device.GetDeviceFunctions().vkGetSwapchainImagesKHR(device, m_Swapchain, &imageCount, nullptr);
	BvCheckErrorReturnVk(result, false);

	// Get the swap chain images
	m_SwapChainTextures.Resize(imageCount);
	m_SwapChainTextureViews.Resize(imageCount);
	BvVector<VkImage> swapChainImages(imageCount);
	result = m_Device.GetDeviceFunctions().vkGetSwapchainImagesKHR(device, m_Swapchain, &imageCount, swapChainImages.Data());
	BvCheckErrorReturnVk(result, false);

	TextureViewDesc textureViewDesc;
	textureViewDesc.m_Format = textureDesc.m_Format;

	for (auto i = 0u; i < imageCount; i++)
	{
		m_SwapChainTextures[i] = new BvSwapChainTextureVk(this, textureDesc, swapChainImages[i]);

		textureViewDesc.m_pTexture = m_SwapChainTextures[i];
		m_SwapChainTextureViews[i] = new BvTextureViewVk(m_Device, textureViewDesc);
		m_SwapChainTextureViews[i]->Create();
	}

	CreateSynchronizationResources();

	AcquireImage();

	return true;
}


void BvSwapChainVk::Destroy()
{
	m_CommandQueue.WaitIdle();

	DestroySynchronizationResources();

	for (auto && pTextureView : m_SwapChainTextureViews)
	{
		auto pResource = static_cast<BvTextureViewVk *>(pTextureView);
		delete pResource;
		pTextureView = nullptr;
	}

	for (auto && pTexture : m_SwapChainTextures)
	{
		delete pTexture;
	}

	m_Device.GetDeviceFunctions().vkDestroySwapchainKHR(m_Device.GetHandle(), m_Swapchain, nullptr);
	m_Swapchain = VK_NULL_HANDLE;
}


void BvSwapChainVk::Present(bool vSync)
{
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
		result = m_Device.GetDeviceFunctions().vkQueuePresentKHR(m_CommandQueue.GetHandle(), &presentInfo);
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

	AcquireImage();
}


void BvSwapChainVk::DestroySurface()
{
	VulkanFunctions::vkDestroySurfaceKHR(m_Device.GetInstanceHandle(), m_Surface, nullptr);
	m_Surface = VK_NULL_HANDLE;
}


void BvSwapChainVk::Resize()
{
	m_CommandQueue.WaitIdle();

	DestroySynchronizationResources();
	Create();
}


void BvSwapChainVk::AcquireImage()
{
	auto result = m_Device.GetDeviceFunctions().vkAcquireNextImageKHR(m_Device.GetHandle(), m_Swapchain, UINT64_MAX,
		m_ImageAcquiredSemaphores[m_CurrSemaphoreIndex]->GetHandle(), VK_NULL_HANDLE, &m_CurrImageIndex);
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
	m_CurrSemaphoreIndex = (m_CurrSemaphoreIndex + 1) % m_SwapChainTextures.Size();
}


void BvSwapChainVk::CreateSynchronizationResources()
{
	//DestroySynchronizationResources();

	//m_ImageAcquiredFences.Resize(m_SwapChainTextures.Size());
	m_ImageAcquiredSemaphores.Resize(m_SwapChainTextures.Size());
	for (auto&& pSemaphore : m_ImageAcquiredSemaphores)
	{
		pSemaphore = new BvSemaphoreVk(m_Device);
		pSemaphore->Create(false);
	}

	m_RenderCompleteSemaphores.Resize(m_SwapChainTextures.Size());
	for (auto&& pSemaphore : m_RenderCompleteSemaphores)
	{
		pSemaphore = new BvSemaphoreVk(m_Device);
		pSemaphore->Create(false);
	}
}


void BvSwapChainVk::DestroySynchronizationResources()
{
	for (auto&& pSemaphore : m_ImageAcquiredSemaphores)
	{
		delete pSemaphore;
	}
	m_ImageAcquiredSemaphores.Clear();

	for (auto&& pSemaphore : m_RenderCompleteSemaphores)
	{
		delete pSemaphore;
	}
	m_RenderCompleteSemaphores.Clear();
}