#include "BvVulkanSwapchain.h"
#include "BvRender/RenderAPIs/Vulkan/BvVulkanDevice.h"
#include "BvCore/System/Window/BvWindow.h"


BvVulkanSwapchain::BvVulkanSwapchain(const BvVulkanDevice * const pDevice, BvWindow * const pWindow)
	: m_pDevice(pDevice), m_pWindow(pWindow), m_PresentationQueue(pDevice->GetGraphicsQueue())
{
}


BvVulkanSwapchain::~BvVulkanSwapchain()
{
	Destroy();
}


void BvVulkanSwapchain::Create(const bool vSync, const bool createDepthBuffer, const VkFormat depthFormat)
{
	m_VSync = vSync;
	m_DepthFormat = createDepthBuffer ? depthFormat : VK_FORMAT_UNDEFINED;

	CreateSurface(*m_pWindow);

	m_Width = m_pWindow->GetWidth();
	m_Height = m_pWindow->GetHeight();

	auto logicalDevice =  m_pDevice->GetLogical();
	auto physicalDevice = m_pDevice->GetPhysical();

	// Get list of supported surface formats
	u32 formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, nullptr);

	BvVector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, surfaceFormats.Data());

	// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
	// there is no preferered format, so we assume VK_FORMAT_B8G8R8A8_UNORM
	VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
	{
		m_ColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
		colorSpace = surfaceFormats[0].colorSpace;
	}
	else
	{
		// iterate over the list of available surface format and
		// check for the presence of VK_FORMAT_B8G8R8A8_UNORM
		bool found_B8G8R8A8_UNORM = false;
		for (auto & surfaceFormat : surfaceFormats)
		{
			if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
			{
				m_ColorFormat = surfaceFormat.format;
				colorSpace = surfaceFormat.colorSpace;
				found_B8G8R8A8_UNORM = true;
				break;
			}
		}

		// in case VK_FORMAT_B8G8R8A8_UNORM is not available
		// select the first available color format
		if (!found_B8G8R8A8_UNORM)
		{
			m_ColorFormat = surfaceFormats[0].format;
			colorSpace = surfaceFormats[0].colorSpace;
		}
	}

	// Get physical device surface properties and formats
	VkSurfaceCapabilitiesKHR surfCaps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface, &surfCaps);

	// Get available present modes
	u32 presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, nullptr);
	assert(presentModeCount > 0);

	BvVector<VkPresentModeKHR> presentModes(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, presentModes.Data());

	VkExtent2D swapchainExtent = {};
	// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
	if (surfCaps.currentExtent.width == (u32)-1)
	{
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
		swapchainExtent.width = m_Width;
		swapchainExtent.height = m_Height;
	}
	else
	{
		// If the surface size is defined, the swap chain size must match
		swapchainExtent = surfCaps.currentExtent;
		m_Width = surfCaps.currentExtent.width;
		m_Height = surfCaps.currentExtent.height;
	}

	// Select a present mode for the swapchain

	// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
	// This mode waits for the vertical blank ("v-sync")
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

	// If v-sync is not requested, try to find a mailbox mode
	// It's the lowest latency non-tearing present mode available
	if (!vSync)
	{
		for (size_t i = 0; i < presentModeCount; i++)
		{
			if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}
			if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
			{
				swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}
	}

	// Determine the number of images
	u32 desiredNumberOfSwapchainImages = surfCaps.minImageCount + 1;
	if ((surfCaps.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfCaps.maxImageCount))
	{
		desiredNumberOfSwapchainImages = surfCaps.maxImageCount;
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
	VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
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
	swapchainCreateInfo.minImageCount = desiredNumberOfSwapchainImages;
	swapchainCreateInfo.imageFormat = m_ColorFormat;
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

	vkCreateSwapchainKHR(logicalDevice, &swapchainCreateInfo, nullptr, &m_Swapchain);

	// If an existing swap chain is re-created, destroy the old swap chain
	// This also cleans up all the presentable images
	if (oldSwapchain != VK_NULL_HANDLE)
	{
		for (size_t i = 0; i < m_SwapchainViews.Size(); i++)
		{
			vkDestroyImageView(logicalDevice, m_SwapchainViews[i], nullptr);
		}
		m_SwapchainViews.Clear();
		m_SwapchainImages.Clear();
		vkDestroySwapchainKHR(logicalDevice, oldSwapchain, nullptr);
	}
	u32 imageCount = 0;
	vkGetSwapchainImagesKHR(logicalDevice, m_Swapchain, &imageCount, nullptr);

	// Get the swap chain images
	m_SwapchainImages.Resize(imageCount);
	m_SwapchainViews.Resize(imageCount);
	vkGetSwapchainImagesKHR(logicalDevice, m_Swapchain, &imageCount, m_SwapchainImages.Data());

	// Get the swap chain buffers containing the image and imageview
	for (u32 i = 0; i < imageCount; i++)
	{
		VkImageViewCreateInfo colorAttachmentView = {};
		colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		colorAttachmentView.pNext = nullptr;
		colorAttachmentView.format = m_ColorFormat;
		colorAttachmentView.components =
		{
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A
		};
		colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		colorAttachmentView.subresourceRange.baseMipLevel = 0;
		colorAttachmentView.subresourceRange.levelCount = 1;
		colorAttachmentView.subresourceRange.baseArrayLayer = 0;
		colorAttachmentView.subresourceRange.layerCount = 1;
		colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		colorAttachmentView.flags = 0;

		colorAttachmentView.image = m_SwapchainImages[i];
		vkCreateImageView(logicalDevice, &colorAttachmentView, nullptr, &m_SwapchainViews[i]);
	}

	if (!m_RenderPass)
	{
		CreateRenderPass();
	}
	CreateFramebuffers();
	CreateDepthImage();

	m_RenderTarget.m_RenderPass = m_RenderPass;
	m_RenderTarget.m_Framebuffers = m_Framebuffers;
}


void BvVulkanSwapchain::Resize()
{
	Create(m_VSync, m_DepthFormat != VK_FORMAT_UNDEFINED ? true : false, m_DepthFormat);
}


void BvVulkanSwapchain::Destroy()
{
	VkDevice device = m_pDevice->GetLogical();

	DestroyFramebuffers();
	DestroyRenderPass();
	DestroyDepthImage();

	m_RenderTarget.m_RenderPass = VK_NULL_HANDLE;
	m_RenderTarget.m_Framebuffers.Clear();

	for (auto && swapchainView : m_SwapchainViews)
	{
		vkDestroyImageView(device, swapchainView, nullptr);
	}
	vkDestroySwapchainKHR(device, m_Swapchain, nullptr);

	vkDestroySurfaceKHR(m_pDevice->GetInstance(), m_Surface, nullptr);
}


void BvVulkanSwapchain::Prepare(u32 & currImageIndex, const VkSemaphore semaphore, const VkFence fence)
{
	auto result = vkAcquireNextImageKHR(m_pDevice->GetLogical(), m_Swapchain, UINT64_MAX, semaphore, fence, &m_CurrImageIndex);
	currImageIndex = m_CurrImageIndex;

	if (result != VkResult::VK_SUCCESS)
	{
		if (result == VkResult::VK_SUBOPTIMAL_KHR || result == VkResult::VK_ERROR_OUT_OF_DATE_KHR)
		{
			if (!(m_pWindow->IsMinimized() || m_pWindow->IsResizing()))
			{
				Resize();
			}
		}
	}
}


void BvVulkanSwapchain::Present(const VkSemaphore waitSemaphore)
{
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_Swapchain;
	presentInfo.pImageIndices = &m_CurrImageIndex;
	// Check if a wait semaphore has been specified to wait for before presenting the image
	if (waitSemaphore)
	{
		presentInfo.pWaitSemaphores = &waitSemaphore;
		presentInfo.waitSemaphoreCount = 1;
	}

	vkQueuePresentKHR(m_PresentationQueue, &presentInfo);
}


void BvVulkanSwapchain::CreateDepthImage()
{
	DestroyDepthImage();

	m_DepthFormat = m_pDevice->GetBestDepthFormat();

	VkImageCreateInfo imageCreateInfo
	{
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr,
		0,
		VK_IMAGE_TYPE_2D,
		m_DepthFormat,
		{ m_Width, m_Height, 1 },
		1,
		1,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		nullptr,
		VK_IMAGE_LAYOUT_UNDEFINED
	};


	auto device = m_pDevice->GetLogical();
	vkCreateImage(device, &imageCreateInfo, nullptr, &m_DepthImage);

	VkMemoryRequirements reqs{};
	vkGetImageMemoryRequirements(device, m_DepthImage, &reqs);

	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = reqs.size;
	allocateInfo.memoryTypeIndex = m_pDevice->GetMemoryTypeIndex(reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vkAllocateMemory(device, &allocateInfo, nullptr, &m_DepthImageMemory);

	vkBindImageMemory(device, m_DepthImage, m_DepthImageMemory, 0);

	TextureDesc textureDesc;
	textureDesc.SetFormat(m_DepthFormat);

	VkImageViewCreateInfo imageViewCreateInfo
	{
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		nullptr,
		0,
		m_DepthImage,
		VK_IMAGE_VIEW_TYPE_2D,
		m_DepthFormat,
		{ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A },
		{ textureDesc.GetAspectMask(), 0, 1, 0, 1 }
	};

	vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_DepthView);
}


void BvVulkanSwapchain::CreateRenderPass()
{
	VkAttachmentDescription attachmentDescriptions[2]{};

	attachmentDescriptions[0].format = m_ColorFormat;
	attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	attachmentDescriptions[1].format = m_DepthFormat;
	attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Collect attachment references
	VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

	// Default render pass setup uses only one subpass
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.pColorAttachments = &colorReference;
	subpass.colorAttachmentCount = 1;
	subpass.pDepthStencilAttachment = &depthReference;

	VkSubpassDependency dependencies[2]{};

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachmentDescriptions;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies;
	vkCreateRenderPass(m_pDevice->GetLogical(), &renderPassInfo, nullptr, &m_RenderPass);
}


void BvVulkanSwapchain::CreateFramebuffers()
{
	DestroyFramebuffers();

	VkImageView views[2] = { VK_NULL_HANDLE, m_DepthView };
	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = m_RenderPass;
	framebufferInfo.width = m_Width;
	framebufferInfo.height = m_Height;
	framebufferInfo.layers = 1;
	framebufferInfo.attachmentCount = m_DepthFormat != VK_FORMAT_UNDEFINED ? 2 : 1;
	framebufferInfo.pAttachments = views;

	for (auto i = 0U; i < m_Framebuffers.Size(); i++)
	{
		views[0] = m_SwapchainViews[i];
		vkCreateFramebuffer(m_pDevice->GetLogical(), &framebufferInfo, nullptr, &m_Framebuffers[i]);
	}
}


void BvVulkanSwapchain::DestroyDepthImage()
{
	auto device = m_pDevice->GetLogical();
	if (m_DepthView)
	{
		vkDestroyImageView(device, m_DepthView, nullptr);
		m_DepthView = VK_NULL_HANDLE;
	}

	if (m_DepthImage)
	{
		vkDestroyImage(device, m_DepthImage, nullptr);
		m_DepthImage = VK_NULL_HANDLE;
	}

	if (m_DepthImageMemory)
	{
		vkFreeMemory(device, m_DepthImageMemory, nullptr);
		m_DepthImageMemory = VK_NULL_HANDLE;
	}
}


void BvVulkanSwapchain::DestroyRenderPass()
{
	if (m_RenderPass)
	{
		vkDestroyRenderPass(m_pDevice->GetLogical(), m_RenderPass, nullptr);
	}
}


void BvVulkanSwapchain::DestroyFramebuffers()
{
	auto device = m_pDevice->GetLogical();
	for (auto && framebuffer : m_Framebuffers)
	{
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}
	m_Framebuffers.Resize(m_SwapchainViews.Size());
}