#include "BvUtilsVk.h"
#include "BvTypeConversionsVk.h"

#include "BDeV/Core/System/Window/BvWindow.h"
#include "BDeV/Core/System/Window/BvMonitor.h"

#include "BvRenderDeviceVk.h"
#include "BvBufferVk.h"
#include "BvTextureVk.h"
#include "BvSamplerVk.h"
#include "BvShaderVk.h"
#include "BvRenderPassVk.h"
#include "BvShaderResourceVk.h"
#include "BvPipelineStateVk.h"
#include "BvCommandContextVk.h"
#include "BvCommandBufferVk.h"
#include "BDeV/Core/RenderAPI/BvRenderAPIUtils.h"


const char* const VkResultToString(const VkResult result)
{
#define VkStringifyCase(result) case result: return #result
	switch (result)
	{
		VkStringifyCase(VK_SUCCESS												);
		VkStringifyCase(VK_NOT_READY											);
		VkStringifyCase(VK_TIMEOUT												);
		VkStringifyCase(VK_EVENT_SET											);
		VkStringifyCase(VK_EVENT_RESET											);
		VkStringifyCase(VK_INCOMPLETE											);
		VkStringifyCase(VK_ERROR_OUT_OF_HOST_MEMORY								);
		VkStringifyCase(VK_ERROR_OUT_OF_DEVICE_MEMORY							);
		VkStringifyCase(VK_ERROR_INITIALIZATION_FAILED							);
		VkStringifyCase(VK_ERROR_DEVICE_LOST									);
		VkStringifyCase(VK_ERROR_MEMORY_MAP_FAILED								);
		VkStringifyCase(VK_ERROR_LAYER_NOT_PRESENT								);
		VkStringifyCase(VK_ERROR_EXTENSION_NOT_PRESENT							);
		VkStringifyCase(VK_ERROR_FEATURE_NOT_PRESENT							);
		VkStringifyCase(VK_ERROR_INCOMPATIBLE_DRIVER							);
		VkStringifyCase(VK_ERROR_TOO_MANY_OBJECTS								);
		VkStringifyCase(VK_ERROR_FORMAT_NOT_SUPPORTED							);
		VkStringifyCase(VK_ERROR_FRAGMENTED_POOL								);
		VkStringifyCase(VK_ERROR_UNKNOWN										);
		VkStringifyCase(VK_ERROR_OUT_OF_POOL_MEMORY								);
		VkStringifyCase(VK_ERROR_INVALID_EXTERNAL_HANDLE						);
		VkStringifyCase(VK_ERROR_FRAGMENTATION									);
		VkStringifyCase(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS					);
		VkStringifyCase(VK_ERROR_SURFACE_LOST_KHR								);
		VkStringifyCase(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR						);
		VkStringifyCase(VK_SUBOPTIMAL_KHR										);
		VkStringifyCase(VK_ERROR_OUT_OF_DATE_KHR								);
		VkStringifyCase(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR						);
		VkStringifyCase(VK_ERROR_VALIDATION_FAILED_EXT							);
		VkStringifyCase(VK_ERROR_INVALID_SHADER_NV								);
		VkStringifyCase(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT	);
		VkStringifyCase(VK_ERROR_NOT_PERMITTED_EXT								);
		VkStringifyCase(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT			);
		VkStringifyCase(VK_THREAD_IDLE_KHR										);
		VkStringifyCase(VK_THREAD_DONE_KHR										);
		VkStringifyCase(VK_OPERATION_DEFERRED_KHR								);
		VkStringifyCase(VK_OPERATION_NOT_DEFERRED_KHR							);
		VkStringifyCase(VK_ERROR_COMPRESSION_EXHAUSTED_EXT						);
		VkStringifyCase(VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT					);
	}
#undef VkStringifyCase

	return "Unknown Vulkan error";
}


bool IsDepthFormat(Format format)
{
	return GetVkFormatMap(format).aspectFlags & VK_IMAGE_ASPECT_DEPTH_BIT;
}


bool IsStencilFormat(Format format)
{
	return GetVkFormatMap(format).aspectFlags & VK_IMAGE_ASPECT_STENCIL_BIT;
}


bool IsDepthStencilFormat(Format format)
{
	VkImageAspectFlags flags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	return (GetVkFormatMap(format).aspectFlags & flags) == flags;
}


bool IsDepthOrStencilFormat(Format format)
{
	VkImageAspectFlags flags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	return (GetVkFormatMap(format).aspectFlags & flags) != 0;
}


namespace VkHelpers
{
	VkObj<VkShaderModule> CreateShaderModule(VkDevice device, size_t size, const u8* pShaderCode)
	{
		VkObj<VkShaderModule> shaderObj;
		auto& result = shaderObj.first;
		auto& shaderModule = shaderObj.second;

		VkShaderModuleCreateInfo shaderCI{};
		shaderCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderCI.pCode = reinterpret_cast<const u32*>(pShaderCode);
		shaderCI.codeSize = size;

		result = vkCreateShaderModule(device, &shaderCI, nullptr, &shaderModule);

		return shaderObj;
	}

	VkObj<VkSurfaceKHR> CreateSurface(VkInstance instance, BvRenderDeviceVk* pDevice, BvWindow* pWindow)
	{
		VkSurfaceKHR surface = VK_NULL_HANDLE;

#if (BV_PLATFORM_WIN32)
		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
		surfaceCreateInfo.hwnd = (HWND)pWindow->GetHandle();

		auto result = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
#endif

		return std::make_pair(result, surface);
	}


	VkObj<VkSwapChainObj> CreateSwapChain(BvRenderDeviceVk* pDevice, VkInstance instance, u32 queueFamilyIndex, VkSwapchainKHR oldSwapChain,
		VkSurfaceKHR surface, SwapChainDesc& swapChainDesc, BvWindow* pWindow)
	{
		VkObj<VkSwapChainObj> scObj;
		auto& result = scObj.first;
		auto& swapChain = scObj.second.m_SwapChain;
		auto& images = scObj.second.m_Images;
		auto& extents = scObj.second.m_Extents;

		auto device = pDevice->GetHandle();
		auto physicalDevice = pDevice->GetPhysicalDeviceHandle();

		do 
		{
			if (surface == VK_NULL_HANDLE)
			{
				auto surfaceObj = CreateSurface(instance, pDevice, pWindow);
				if (surfaceObj.first != VK_SUCCESS)
				{
					break;
				}
				surface = surfaceObj.second;
			}

			scObj.second.m_Surface = surface;

			VkBool32 presentationSupported = VK_FALSE;
			result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, &presentationSupported);

			BV_ASSERT(result == VK_SUCCESS, "Failed to check for surface support");
			if (!presentationSupported)
			{
				break;
			}

			// Get list of supported surface formats
			u32 formatCount{};
			result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
			BV_ASSERT(result == VK_SUCCESS, "Failed to retrieve surface formats");

			BvVector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
			result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.Data());
			BV_ASSERT(result == VK_SUCCESS, "Failed to retrieve surface formats");

			// 
			VkFormat requestedFormat = GetVkFormat(swapChainDesc.m_Format);
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
						if ((swapChainDesc.m_PreferHDR && surfaceFormat.colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
							|| (!swapChainDesc.m_PreferHDR && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR))
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
						swapChainDesc.m_Format = Format::kBGRA8_UNorm;
					}
				}
			}

			// Update the SwapChain's format in case it changed - if it ends up being Format::Unknown,
			// then that means it's using a format not on the list, but nevertheless supported
			switch (format)
			{
			case VK_FORMAT_R8G8B8A8_UNORM: swapChainDesc.m_Format = Format::kRGBA8_UNorm; break;
			case VK_FORMAT_B8G8R8A8_UNORM: swapChainDesc.m_Format = Format::kBGRA8_UNorm; break;
			case VK_FORMAT_R8G8B8A8_SRGB: swapChainDesc.m_Format = Format::kRGBA8_UNorm_SRGB; break;
			case VK_FORMAT_B8G8R8A8_SRGB: swapChainDesc.m_Format = Format::kBGRA8_UNorm_SRGB; break;
			case VK_FORMAT_R16G16B16A16_SFLOAT: swapChainDesc.m_Format = Format::kRGBA16_Float; break;
			case VK_FORMAT_A2B10G10R10_UNORM_PACK32: swapChainDesc.m_Format = Format::kRGB10A2_UNorm; break;
			default: swapChainDesc.m_Format = Format::kUnknown; break;
			}

			if (swapChainDesc.m_Format == Format::kUnknown)
			{
				result = VK_ERROR_FORMAT_NOT_SUPPORTED;
				break;
			}

			bool supportsTrueFullscreen = pDevice->GetDeviceInfo()->m_ExtendedSurfaceCaps.fullScreenExclusiveCaps.fullScreenExclusiveSupported;
			if (!supportsTrueFullscreen && swapChainDesc.m_WindowMode == SwapChainMode::kFullscreen)
			{
				swapChainDesc.m_WindowMode = SwapChainMode::kBorderlessFullscreen;
			}

			VkSurfaceFullScreenExclusiveInfoEXT surfaceFS{ VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT };
			surfaceFS.fullScreenExclusive = GetVkFullScreenExclusiveEXTMode(swapChainDesc.m_WindowMode);
	#if BV_PLATFORM_WIN32
			VkSurfaceFullScreenExclusiveWin32InfoEXT surfaceFSWin32{ VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT };
			if (supportsTrueFullscreen)
			{
				surfaceFS.pNext = &surfaceFSWin32;
				surfaceFSWin32.hmonitor = BvMonitor::FromWindow(pWindow)->GetHandle();
			}
	#endif

			// Get physical device surface properties and formats
			VkSurfaceCapabilitiesKHR surfCaps{};
			result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfCaps);
			BV_ASSERT(result == VK_SUCCESS, "Failed to check for surface caps");

			// Get available present modes
			u32 presentModeCount;
			result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
			BV_ASSERT(result == VK_SUCCESS, "Failed to retrieve surface present modes");
			BV_ASSERT(presentModeCount > 0, "No present modes");

			BvVector<VkPresentModeKHR> presentModes(presentModeCount);
			result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.Data());
			BV_ASSERT(result == VK_SUCCESS, "Failed to retrieve surface present modes");

			u32 width = pWindow->GetWidth();
			u32 height = pWindow->GetHeight();
			VkExtent2D swapchainExtent = {};
			if (swapChainDesc.m_WindowMode == SwapChainMode::kWindowed)
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
			extents = swapchainExtent;

			// Select a present mode for the swapchain

			// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
			// This mode waits for the vertical blank ("v-sync")
			VkPresentModeKHR swapchainPresentMode = VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;

			// If v-sync is not requested, try to find a mailbox mode
			// It's the lowest latency non-tearing present mode available
			if (!swapChainDesc.m_VSync)
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

			// Determine the number of images - ask for at least minImageCount, so if less was specified, change it
			swapChainDesc.m_SwapChainImageCount = std::max(swapChainDesc.m_SwapChainImageCount, surfCaps.minImageCount);
			
			// If there's a maximum limit and we went over it, clamp it
			if ((surfCaps.maxImageCount > 0) && (swapChainDesc.m_SwapChainImageCount > surfCaps.maxImageCount))
			{
				swapChainDesc.m_SwapChainImageCount = surfCaps.maxImageCount;
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

			VkSwapchainCreateInfoKHR swapchainCreateInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
			swapchainCreateInfo.pNext = supportsTrueFullscreen ? &surfaceFS : nullptr;
			swapchainCreateInfo.surface = surface;
			swapchainCreateInfo.minImageCount = swapChainDesc.m_SwapChainImageCount;
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
			swapchainCreateInfo.oldSwapchain = oldSwapChain;
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

			result = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapChain);
			if (result != VK_SUCCESS && swapChainDesc.m_WindowMode == SwapChainMode::kFullscreen)
			{
				swapChainDesc.m_WindowMode = SwapChainMode::kBorderlessFullscreen;
				swapchainCreateInfo.pNext = nullptr;

				// If creating a full screen swap chain fails, try a regular one
				result = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapChain);
			}

			if (result != VK_SUCCESS)
			{
				break;
			}

			DestroyDeviceObject(device, oldSwapChain);

			u32 imageCount = 0;
			result = vkGetSwapchainImagesKHR(*pDevice, swapChain, &imageCount, nullptr);
			if (result != VK_SUCCESS)
			{
				DestroyDeviceObject(device, swapChain);
				break;
			}

			// Get the swap chain images
			images.Resize(imageCount);
			result = vkGetSwapchainImagesKHR(*pDevice, swapChain, &imageCount, images.Data());
			if (result != VK_SUCCESS)
			{
				DestroyDeviceObject(device, swapChain);
			}
		} while (false);

		return scObj;
	}


	VkObj<VkBufferObj> CreateBuffer(BvRenderDeviceVk* pDevice, const BufferDesc& bufferDesc)
	{
		VkObj<VkBufferObj> bufferObj;
		auto& result = bufferObj.first;
		auto& buffer = bufferObj.second.m_Buffer;
		auto& vmaA = bufferObj.second.m_Memory;
		auto& deviceAddress = bufferObj.second.m_DeviceAddress;
		auto& pMappedMemory = bufferObj.second.m_pMappedMemory;
		auto& needsFlush = bufferObj.second.m_NeedsFlush;

		VkBufferCreateInfo bufferCreateInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		//bufferCreateInfo.pNext = nullptr;
		//bufferCreateInfo.flags = 0; // No Sparse Binding for now
		bufferCreateInfo.size = bufferDesc.m_Size;
		bufferCreateInfo.usage = GetVkBufferUsageFlags(bufferDesc.m_UsageFlags, bufferDesc.m_Formatted);
		if (pDevice->GetDeviceInfo()->m_DeviceFeatures1_2.bufferDeviceAddress)
		{
			bufferCreateInfo.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		}
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		//bufferCreateInfo.queueFamilyIndexCount = 0;
		//bufferCreateInfo.pQueueFamilyIndices = nullptr;

		auto device = pDevice->GetHandle();

		do
		{
			auto vma = pDevice->GetAllocator();
			VmaAllocationCreateInfo vmaACI = {};
			vmaACI.requiredFlags = GetVkMemoryPropertyFlags(bufferDesc.m_MemoryType);
			vmaACI.preferredFlags = vmaACI.requiredFlags | GetPreferredVkMemoryPropertyFlags(bufferDesc.m_MemoryType);
			if (EHasFlag(bufferDesc.m_CreateFlags, BufferCreateFlags::kCreateMapped))
			{
				vmaACI.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
			}
			VmaAllocationInfo vmaAI{};

			result = vmaCreateBuffer(vma, &bufferCreateInfo, &vmaACI, &buffer, &vmaA, &vmaAI);
			if (result != VK_SUCCESS)
			{
				break;
			}

			//result = vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer);
			//BV_ASSERT(result == VK_SUCCESS, "Failed to create buffer");
			//if (result != VK_SUCCESS)
			//{
			//	break;
			//}

			//result = vmaAllocateMemoryForBuffer(vma, buffer, &vmaACI, &vmaA, &vmaAI);
			//BV_ASSERT(result == VK_SUCCESS, "Failed to allocate memory for buffer");
			//if (result != VK_SUCCESS)
			//{
			//	DestroyDeviceObject(device, buffer);
			//	break;
			//}

			//result = vkBindBufferMemory(device, buffer, vmaAI.deviceMemory, vmaAI.offset);
			//BV_ASSERT(result == VK_SUCCESS, "Failed to bind memory for buffer");
			//if (result != VK_SUCCESS)
			//{
			//	DestroyDeviceObject(device, buffer, vma, vmaA);
			//	break;
			//}

			pMappedMemory = vmaAI.pMappedData;

			if (bufferCreateInfo.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
			{
				VkBufferDeviceAddressInfo addressInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, buffer };
				deviceAddress = vkGetBufferDeviceAddress(device, &addressInfo);
			}

			const auto& memoryType = pDevice->GetDeviceInfo()->m_DeviceMemoryProperties.memoryProperties.memoryTypes[vmaAI.memoryType];
			needsFlush = ((memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0);
			if (needsFlush)
			{
				auto& bd = const_cast<BufferDesc&>(bufferDesc);
				if (bd.m_MemoryType == MemoryType::kReadBack)
				{
					bd.m_MemoryType = MemoryType::kReadBackNC;
				}
				else if (bd.m_MemoryType == MemoryType::kUpload)
				{
					bd.m_MemoryType = MemoryType::kUploadNC;
				}
			}
		} while (false);

		return bufferObj;
	}


	VkObj<VkBufferView> CreateBufferView(BvRenderDeviceVk* pDevice, const BufferViewDesc& bufferViewDesc)
	{
		VkResult result = VK_SUCCESS;
		VkBufferView bufferView = VK_NULL_HANDLE;

		if (bufferViewDesc.m_Format != Format::kUnknown)
		{
			VkBufferViewCreateInfo bufferViewCreateInfo{};
			bufferViewCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
			//bufferViewCreateInfo.pNext = nullptr;
			//bufferViewCreateInfo.flags = 0;
			bufferViewCreateInfo.buffer = TO_VK(bufferViewDesc.m_pBuffer)->GetHandle();
			bufferViewCreateInfo.format = GetVkFormat(bufferViewDesc.m_Format);
			bufferViewCreateInfo.offset = bufferViewDesc.m_Offset;
			bufferViewCreateInfo.range = bufferViewDesc.m_ElementCount * bufferViewDesc.m_Stride;

			result = vkCreateBufferView(pDevice->GetHandle(), &bufferViewCreateInfo, nullptr, &bufferView);
		}

		return std::make_pair(result, bufferView);
	}


	VkObj<VkTextureObj> CreateTexture(BvRenderDeviceVk* pDevice, const TextureDesc& textureDesc, u32 mutableFormatCount, const Format* pMutableFormats)
	{
		VkResult result;
		VkImage texture = VK_NULL_HANDLE;
		VmaAllocation vmaA = VK_NULL_HANDLE;

		VkImageCreateFlags imageCreateFlags = 0;
		if (EHasFlag(textureDesc.m_CreateFlags, TextureCreateFlags::kCreateCubemap))
		{
			imageCreateFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		}

		VkImageFormatListCreateInfo iflCI{ VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO };
		bool useMultiFormat = false;
		constexpr u32 kMaxMutableFormatCount = 8; // Should be enough
		VkFormat mutableVkFormats[kMaxMutableFormatCount];

		// Thanks to turanszkij for his article - https://wickedengine.net/2022/11/graphics-api-secrets-format-casting/
		auto fi = BvRenderUtils::GetFormatInfo(textureDesc.m_Format);
		if ((fi.m_IsPlanar && !fi.m_IsDepth) || fi.m_IsTypeless)
		{
			imageCreateFlags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
			if (fi.m_IsTypeless)
			{
				if (!mutableFormatCount || !pMutableFormats)
				{
					mutableFormatCount = kMaxMutableFormatCount;
					pMutableFormats = fi.m_pCastableFormats;
				}

				u32 numMutableFormats = 0;
				for (; numMutableFormats < mutableFormatCount && pMutableFormats[numMutableFormats] != Format::kUnknown; ++numMutableFormats)
				{
					mutableVkFormats[numMutableFormats] = GetVkFormat(pMutableFormats[numMutableFormats]);
				}

				iflCI.viewFormatCount = numMutableFormats;
				iflCI.pViewFormats = mutableVkFormats;
				useMultiFormat = true;
			}
		}

		if (EHasFlag(textureDesc.m_UsageFlags, TextureUsage::kUnorderedAccess) && fi.m_IsSRGBFormat)
		{
			imageCreateFlags |= VK_IMAGE_CREATE_EXTENDED_USAGE_BIT;
		}

		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext = nullptr;
		imageCreateInfo.flags = imageCreateFlags;
		imageCreateInfo.imageType = GetVkImageType(textureDesc.m_ImageType);
		imageCreateInfo.format = GetVkFormat(textureDesc.m_Format);
		imageCreateInfo.extent = { textureDesc.m_Size.m_Width, textureDesc.m_Size.m_Height, textureDesc.m_Size.m_Depth };
		imageCreateInfo.mipLevels = textureDesc.m_MipLevels;
		imageCreateInfo.arrayLayers = textureDesc.m_ArraySize;
		imageCreateInfo.samples = GetVkSampleCountFlagBits(textureDesc.m_SampleCount);
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = GetVkImageUsageFlags(textureDesc.m_UsageFlags);
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.queueFamilyIndexCount = 0;
		imageCreateInfo.pQueueFamilyIndices = nullptr;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		u32 extraMemoryFlags = 0;
		if (imageCreateInfo.usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)
		{
			extraMemoryFlags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
			imageCreateInfo.usage &= (~(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT));
		}

		auto device = pDevice->GetHandle();

		do 
		{
			auto vma = pDevice->GetAllocator();
			VmaAllocationCreateInfo vmaACI = {};
			vmaACI.requiredFlags = GetVkMemoryPropertyFlags(textureDesc.m_MemoryType);
			vmaACI.preferredFlags = GetPreferredVkMemoryPropertyFlags(textureDesc.m_MemoryType) | extraMemoryFlags;
			VmaAllocationInfo vmaAI{};

			result = vmaCreateImage(vma, &imageCreateInfo, &vmaACI, &texture, &vmaA, &vmaAI);

			//result = vkCreateImage(device, &imageCreateInfo, nullptr, &texture);
			//if (result != VK_SUCCESS)
			//{
			//	break;
			//}

			//result = vmaAllocateMemoryForImage(vma, texture, &vmaACI, &vmaA, &vmaAI);
			//BV_ASSERT(result == VK_SUCCESS, "Failed to allocate memory for image");
			//if (result != VK_SUCCESS)
			//{
			//	DestroyDeviceObject(device, texture);
			//	break;
			//}

			//result = vkBindImageMemory(device, texture, vmaAI.deviceMemory, vmaAI.offset);
			//if (result != VK_SUCCESS)
			//{
			//	DestroyDeviceObject(device, texture, vma, vmaA);
			//}
		} while (false);

		return std::make_pair(result, VkTextureObj{ texture, vmaA });
	}


	VkObj<VkImageView> CreateTextureView(BvRenderDeviceVk* pDevice, const TextureViewDesc& textureViewDesc)
	{
		VkResult result;
		VkImageView textureView = VK_NULL_HANDLE;

		BV_ASSERT(textureViewDesc.m_pTexture != nullptr, "Invalid texture handle");

		auto& vkFormatMap = GetVkFormatMap(textureViewDesc.m_Format);
		do
		{
			if (vkFormatMap.format == VK_FORMAT_UNDEFINED)
			{
				BV_ASSERT(vkFormatMap.format != VK_FORMAT_UNDEFINED, "Format not supported in Vulkan");

				result = VK_ERROR_FORMAT_NOT_SUPPORTED;
				break;
			}

			VkImageAspectFlags aspectFlags = vkFormatMap.aspectFlags;
			if ((vkFormatMap.aspectFlags & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))
				&& EHasAnyFlags(textureViewDesc.m_pTexture->GetDesc().m_UsageFlags, TextureUsage::kShaderResource | TextureUsage::kInputAttachment))
			{
				aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
			}
			else if (vkFormatMap.aspectFlags & (VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT))
			{
				BV_ASSERT(textureViewDesc.m_SubresourceDesc.planeSlice <= 2, "Invalid plane slice");
				aspectFlags = (VK_IMAGE_ASPECT_PLANE_0_BIT << textureViewDesc.m_SubresourceDesc.planeSlice);
			}

			// Thanks to turanszkij for his article - https://wickedengine.net/2022/11/graphics-api-secrets-format-casting/
			VkImageViewUsageCreateInfo viewUCI{ VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO };
			auto fi = BvRenderUtils::GetFormatInfo(textureViewDesc.m_Format);
			auto& textureDesc = textureViewDesc.m_pTexture->GetDesc();
			bool removeStorageBit = fi.m_IsSRGBFormat && EHasFlag(textureDesc.m_UsageFlags, TextureUsage::kUnorderedAccess);
			if (removeStorageBit)
			{
				viewUCI.usage = GetVkImageUsageFlags(textureDesc.m_UsageFlags) & (~VK_IMAGE_USAGE_STORAGE_BIT);
			}

			VkImageViewCreateInfo imageViewCreateInfo{};
			imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCreateInfo.pNext = nullptr;
			imageViewCreateInfo.flags = 0;
			imageViewCreateInfo.image = TO_VK(textureViewDesc.m_pTexture)->GetHandle();
			imageViewCreateInfo.viewType = GetVkImageViewType(textureViewDesc.m_ViewType);
			imageViewCreateInfo.format = vkFormatMap.format;
			imageViewCreateInfo.components = vkFormatMap.componentMapping;
			imageViewCreateInfo.subresourceRange =
			{
				aspectFlags,
				textureViewDesc.m_SubresourceDesc.firstMip,
				textureViewDesc.m_SubresourceDesc.mipCount,
				textureViewDesc.m_SubresourceDesc.firstLayer,
				textureViewDesc.m_SubresourceDesc.layerCount
			};

			result = vkCreateImageView(pDevice->GetHandle(), &imageViewCreateInfo, nullptr, &textureView);
		} while (false);

		return std::make_pair(result, textureView);
	}


	VkObj<VkSampler> CreateSampler(BvRenderDeviceVk* pDevice, const SamplerDesc& samplerDesc)
	{
		VkSamplerCreateInfo samplerCreateInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		//samplerCreateInfo.pNext = nullptr;
		//samplerCreateInfo.flags = 0;
		samplerCreateInfo.magFilter = GetVkFilter(samplerDesc.m_MagFilter);
		samplerCreateInfo.minFilter = GetVkFilter(samplerDesc.m_MinFilter);
		samplerCreateInfo.mipmapMode = GetVkSamplerMipmapMode(samplerDesc.m_MipmapMode);
		samplerCreateInfo.addressModeU = GetVkSamplerAddressMode(samplerDesc.m_AddressModeU);
		samplerCreateInfo.addressModeV = GetVkSamplerAddressMode(samplerDesc.m_AddressModeV);
		samplerCreateInfo.addressModeW = GetVkSamplerAddressMode(samplerDesc.m_AddressModeW);
		samplerCreateInfo.mipLodBias = samplerDesc.m_MipLodBias;
		samplerCreateInfo.anisotropyEnable = samplerDesc.m_AnisotropyEnable;
		samplerCreateInfo.maxAnisotropy = samplerDesc.m_MaxAnisotropy;
		if (samplerDesc.m_CompareOp != CompareOp::kNone)
		{
			samplerCreateInfo.compareEnable = VK_TRUE;
			samplerCreateInfo.compareOp = GetVkCompareOp(samplerDesc.m_CompareOp);
		}
		samplerCreateInfo.minLod = samplerDesc.m_MinLod;
		samplerCreateInfo.maxLod = samplerDesc.m_MaxLod;
		//samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		//samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

		bool usesCustomBorderColor = false;
		if (samplerDesc.m_BorderColor[0] == 0.0f
			|| samplerDesc.m_BorderColor[1] == 0.0f
			|| samplerDesc.m_BorderColor[2] == 0.0f
			|| samplerDesc.m_BorderColor[3] == 0.0f)
		{
			samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		}
		else if (samplerDesc.m_BorderColor[0] == 0.0f
			|| samplerDesc.m_BorderColor[1] == 0.0f
			|| samplerDesc.m_BorderColor[2] == 0.0f
			|| samplerDesc.m_BorderColor[3] == 1.0f)
		{
			samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		}
		else if (samplerDesc.m_BorderColor[0] == 1.0f
			|| samplerDesc.m_BorderColor[1] == 1.0f
			|| samplerDesc.m_BorderColor[2] == 1.0f
			|| samplerDesc.m_BorderColor[3] == 1.0f)
		{
			samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		}
		else
		{
			usesCustomBorderColor = true;
		}

		auto pDeviceInfo = pDevice->GetDeviceInfo();
		auto supportsBorderColor = pDeviceInfo->m_ExtendedFeatures.customBorderColorFeatures.customBorderColorWithoutFormat;
		bool supportsReduction = pDeviceInfo->m_DeviceFeatures1_2.samplerFilterMinmax;

		const void** pNext = &samplerCreateInfo.pNext;
		VkSamplerCustomBorderColorCreateInfoEXT customBorderColorCI{ VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT };
		if (usesCustomBorderColor && supportsBorderColor)
		{
			memcpy(customBorderColorCI.customBorderColor.float32, samplerDesc.m_BorderColor, sizeof(samplerDesc.m_BorderColor));
			//customBorderColorCI.format = VK_FORMAT_UNDEFINED;

			samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_CUSTOM_EXT;
			*pNext = &customBorderColorCI;
			pNext = &customBorderColorCI.pNext;
		}

		VkSamplerReductionModeCreateInfo reductionCI{ VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO };
		if (samplerDesc.m_ReductionMode != ReductionMode::kStandard && supportsReduction)
		{
			reductionCI.reductionMode = GetVkSamplerReductionMode(samplerDesc.m_ReductionMode);
			*pNext = &reductionCI;
		}

		VkSampler sampler = VK_NULL_HANDLE;
		auto result = vkCreateSampler(pDevice->GetHandle(), &samplerCreateInfo, nullptr, &sampler);
		return std::make_pair(result, sampler);
	}


	VkObj<VkRenderPass> CreateRenderPass(BvRenderDeviceVk* pDevice, const RenderPassDesc& renderPassDesc)
	{
		BvVector<VkAttachmentDescription2> attachments(renderPassDesc.m_Attachments.Size(), VkAttachmentDescription2{ VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2 });
		for (auto i = 0u; i < renderPassDesc.m_Attachments.Size(); ++i)
		{
			auto& attachment = renderPassDesc.m_Attachments[i];

			auto& attachmentVk = attachments[i];
			//attachmentVk.pNext = nullptr;
			//attachmentVk.flags;
			attachmentVk.format = GetVkFormat(attachment.m_Format);
			attachmentVk.samples = GetVkSampleCountFlagBits(attachment.m_SampleCount);
			attachmentVk.loadOp = GetVkAttachmentLoadOp(attachment.m_LoadOp);
			attachmentVk.storeOp = GetVkAttachmentStoreOp(attachment.m_StoreOp);
			if (IsDepthStencilFormat(attachment.m_Format))
			{
				attachmentVk.stencilLoadOp = attachmentVk.loadOp;
				attachmentVk.stencilStoreOp = attachmentVk.storeOp;
			}
			else
			{
				attachmentVk.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentVk.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			}
			attachmentVk.initialLayout = GetVkImageLayout(attachment.m_StateBefore, IsDepthOrStencilFormat(attachment.m_Format));
			attachmentVk.finalLayout = GetVkImageLayout(attachment.m_StateAfter, IsDepthOrStencilFormat(attachment.m_Format));
		}

		BvVector<VkSubpassDescription2> subpasses(renderPassDesc.m_Subpasses.Size(), VkSubpassDescription2{ VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2 });
		BvVector<u32> correlationMasks;
		BvVector<VkAttachmentReference2> attachmentReferences;
		BvVector<VkFragmentShadingRateAttachmentInfoKHR> shadingRateRefs;
		BvVector<VkSubpassDescriptionDepthStencilResolve> depthStencilResolves;

		attachmentReferences.Reserve(renderPassDesc.m_Subpasses.Size() * renderPassDesc.m_Attachments.Size());

		u32 srRefCount = 0;
		bool usesMultiview = false;
		for (auto i = 0; i < renderPassDesc.m_Subpasses.Size(); ++i)
		{
			auto& subpass = renderPassDesc.m_Subpasses[i];
			if (subpass.m_ShadingRateAttachment.IsValid())
			{
				++srRefCount;
			}

			if (subpass.m_MultiviewCount > 1)
			{
				usesMultiview = true;
			}
		}
		shadingRateRefs.Reserve(srRefCount);
		if (usesMultiview)
		{
			correlationMasks.Resize(renderPassDesc.m_Subpasses.Size());
		}

		for (auto i = 0u; i < renderPassDesc.m_Subpasses.Size(); ++i)
		{
			auto& subpass = renderPassDesc.m_Subpasses[i];

			auto& subpassVk = subpasses[i];
			const void** pNext = &subpassVk.pNext;

			subpassVk.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			if (subpass.m_ColorAttachments.Size() > 0)
			{
				subpassVk.colorAttachmentCount = subpass.m_ColorAttachments.Size();
				auto currIndex = (u32)attachmentReferences.Size();
				for (auto j = 0u; j < subpass.m_ColorAttachments.Size(); ++j)
				{
					auto& ref = subpass.m_ColorAttachments[j];

					auto& refVk = attachmentReferences.EmplaceBack(VkAttachmentReference2{ VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2 });
					refVk.aspectMask = GetVkFormatMap(renderPassDesc.m_Attachments[ref.m_Index].m_Format).aspectFlags;
					refVk.attachment = ref.m_Index;
					refVk.layout = GetVkImageLayout(ref.m_ResourceState);
				}

				subpassVk.pColorAttachments = &attachmentReferences[currIndex];
			}

			if (subpass.m_DepthStencilAttachment.IsValid())
			{
				auto& ref = subpass.m_DepthStencilAttachment;

				auto& refVk = attachmentReferences.EmplaceBack(VkAttachmentReference2{ VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2 });
				refVk.aspectMask = GetVkFormatMap(renderPassDesc.m_Attachments[ref.m_Index].m_Format).aspectFlags;
				refVk.attachment = ref.m_Index;
				refVk.layout = GetVkImageLayout(ref.m_ResourceState);

				subpassVk.pDepthStencilAttachment = &refVk;
			}

			if (subpass.m_InputAttachments.Size() > 0)
			{
				subpassVk.inputAttachmentCount = subpass.m_InputAttachments.Size();
				auto currIndex = (u32)attachmentReferences.Size();
				for (auto j = 0u; j < subpass.m_InputAttachments.Size(); ++j)
				{
					auto& ref = subpass.m_InputAttachments[j];

					auto& refVk = attachmentReferences.EmplaceBack(VkAttachmentReference2{ VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2 });
					refVk.aspectMask = GetVkFormatMap(renderPassDesc.m_Attachments[ref.m_Index].m_Format).aspectFlags;
					refVk.attachment = ref.m_Index;
					refVk.layout = GetVkImageLayout(ref.m_ResourceState);
					if ((refVk.aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))
						&& refVk.layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
					{
						refVk.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
					}
				}
				subpassVk.pInputAttachments = &attachmentReferences[currIndex];
			}

			if (subpass.m_ResolveAttachments.Size() > 0)
			{
				auto currIndex = (u32)attachmentReferences.Size();
				for (auto j = 0u; j < subpass.m_ColorAttachments.Size(); ++j)
				{
					auto& ref = subpass.m_ResolveAttachments[j];

					auto& refVk = attachmentReferences.EmplaceBack(VkAttachmentReference2{ VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2 });
					refVk.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					refVk.attachment = ref.m_Index;
					refVk.layout = GetVkImageLayout(ref.m_ResourceState);
				}

				subpassVk.pResolveAttachments = &attachmentReferences[currIndex];
			}

			if (subpass.m_DepthStencilResolveAttachment.IsValid())
			{
				auto& ref = subpass.m_DepthStencilResolveAttachment;

				auto& refVk = attachmentReferences.EmplaceBack(VkAttachmentReference2{ VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2 });
				refVk.aspectMask = GetVkFormatMap(renderPassDesc.m_Attachments[ref.m_Index].m_Format).aspectFlags;
				refVk.attachment = ref.m_Index;
				refVk.layout = GetVkImageLayout(ref.m_ResourceState);

				auto& depthStencilResolve = depthStencilResolves.EmplaceBack(VkSubpassDescriptionDepthStencilResolve{ VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE });
				depthStencilResolve.depthResolveMode = depthStencilResolve.stencilResolveMode = GetVkResolveMode(ref.m_ResolveMode);
				depthStencilResolve.pDepthStencilResolveAttachment = &refVk;

				*pNext = &depthStencilResolve;
				pNext = &depthStencilResolve.pNext;
			}

			if (subpass.m_ShadingRateAttachment.IsValid())
			{
				auto& ref = subpass.m_ShadingRateAttachment;

				auto& refVk = attachmentReferences.EmplaceBack(VkAttachmentReference2{ VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2 });
				refVk.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				refVk.attachment = ref.m_Index;
				refVk.layout = GetVkImageLayout(ref.m_ResourceState);

				auto& shadingRateRef = shadingRateRefs.EmplaceBack(VkFragmentShadingRateAttachmentInfoKHR{ VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR });
				shadingRateRef.shadingRateAttachmentTexelSize = VkExtent2D{ subpass.m_ShadingRateAttachment.m_TexelSizes[0],
					subpass.m_ShadingRateAttachment.m_TexelSizes[1] };
				shadingRateRef.pFragmentShadingRateAttachment = &refVk;

				*pNext = &shadingRateRef;
				pNext = &shadingRateRef.pNext;
			}

			if (subpass.m_MultiviewCount > 1)
			{
				subpassVk.viewMask = (1 << subpass.m_MultiviewCount) - 1;
				correlationMasks[i] = subpass.m_MultiviewCorrelationMask;
			}
		}

		auto getVkFlagsFn = [&renderPassDesc](u32 subpassIndex, VkAccessFlags2& accessFlags, VkPipelineStageFlags2& stageFlags)
			{
				auto& subpass = renderPassDesc.m_Subpasses[subpassIndex];
				if (subpass.m_ColorAttachments.Size() > 0)
				{
					accessFlags |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
				}
				if (subpass.m_InputAttachments.Size() > 0)
				{
					accessFlags |= VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT;
				}
				if (subpass.m_DepthStencilAttachment.IsValid())
				{
					accessFlags |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				}
				if (subpass.m_ShadingRateAttachment.IsValid())
				{
					accessFlags |= VK_ACCESS_2_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
				}
				stageFlags |= GetVkPipelineStageFlags(accessFlags);
				if (accessFlags & VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT)
				{
					stageFlags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
				}
			};

		BvVector<VkSubpassDependency2> dependencies;
		BvVector<VkMemoryBarrier2> barriers;
		if (renderPassDesc.m_SubpassDependencies.Size() > 0)
		{
			dependencies.Resize(renderPassDesc.m_SubpassDependencies.Size(), VkSubpassDependency2{ VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2 });
			barriers.Resize(dependencies.Size(), VkMemoryBarrier2{ VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 });
			for (auto i = 0u; i < renderPassDesc.m_SubpassDependencies.Size(); ++i)
			{
				auto& dep = renderPassDesc.m_SubpassDependencies[i];

				auto& depVk = dependencies[i];
				auto& barrierVk = barriers[i];
				depVk.srcSubpass = dep.m_SrcSubpass;
				depVk.dstSubpass = dep.m_DstSubpass;
				barrierVk.srcAccessMask = GetVkAccessFlags(dep.m_SrcAccess);
				barrierVk.dstAccessMask = GetVkAccessFlags(dep.m_DstAccess);
				barrierVk.srcStageMask = GetVkPipelineStageFlags(dep.m_SrcStage);
				barrierVk.dstStageMask = GetVkPipelineStageFlags(dep.m_DstStage);
				depVk.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			}
		}
		else
		{
			dependencies.Resize(renderPassDesc.m_Subpasses.Size() + 1, VkSubpassDependency2{ VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2 });
			barriers.Resize(dependencies.Size(), VkMemoryBarrier2{ VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 });

			auto lastPassIndex = renderPassDesc.m_Subpasses.Size() - 1;
			for (auto i = 0u; i < lastPassIndex; ++i)
			{
				auto& srcPass = renderPassDesc.m_Subpasses[i];
				auto& dstPass = renderPassDesc.m_Subpasses[i + 1];

				auto& dep = dependencies[i + 1];
				auto& barrier = barriers[i + 1];
				dep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
				dep.srcSubpass = i;
				dep.dstSubpass = i + 1;
				dep.pNext = &barrier;
				getVkFlagsFn(i, barrier.srcAccessMask, barrier.srcStageMask);
				getVkFlagsFn(i + 1, barrier.dstAccessMask, barrier.dstStageMask);
			}

			auto lastDependencyIndex = (u32)dependencies.Size() - 1;

			auto& extSrc = dependencies[0];
			extSrc.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			extSrc.srcSubpass = VK_SUBPASS_EXTERNAL;
			extSrc.dstSubpass = 0;
			extSrc.pNext = &barriers[0];
			getVkFlagsFn(0, barriers[0].dstAccessMask, barriers[0].dstStageMask);

			auto& extDst = dependencies[lastDependencyIndex];
			extDst.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			extDst.srcSubpass = lastPassIndex;
			extDst.dstSubpass = VK_SUBPASS_EXTERNAL;
			extDst.pNext = &barriers[lastDependencyIndex];
			getVkFlagsFn(lastPassIndex,
				barriers[lastDependencyIndex].srcAccessMask, barriers[lastDependencyIndex].srcStageMask);

			auto& lastSubpass = renderPassDesc.m_Subpasses[lastPassIndex];
			VkAccessFlags2 accessFlags = 0;
			VkPipelineStageFlags2 stageFlags = 0;
			for (auto i = 0u; i < lastSubpass.m_ColorAttachments.Size(); ++i)
			{
				auto& attachment = lastSubpass.m_ColorAttachments[i];
				if (renderPassDesc.m_Attachments[attachment.m_Index].m_StateAfter == ResourceState::kPresent)
				{
					accessFlags = VK_ACCESS_2_MEMORY_READ_BIT;
					stageFlags = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;

					break;
				}
			}

			if (accessFlags != 0 && stageFlags != 0)
			{
				barriers[0].srcAccessMask = accessFlags;
				barriers[0].srcStageMask = stageFlags;

				barriers[lastDependencyIndex].dstAccessMask = accessFlags;
				barriers[lastDependencyIndex].dstStageMask = stageFlags;
			}
			else
			{
				barriers[0].srcAccessMask = barriers[0].dstAccessMask;
				barriers[0].srcStageMask = barriers[0].dstStageMask;

				barriers[lastDependencyIndex].dstAccessMask = barriers[lastDependencyIndex].srcAccessMask;
				barriers[lastDependencyIndex].dstStageMask = barriers[lastDependencyIndex].srcStageMask;
			}
		}

		// Create render pass
		VkRenderPassCreateInfo2 createInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2 };
		createInfo.attachmentCount = (u32)attachments.Size();
		createInfo.pAttachments = attachments.Data();
		createInfo.subpassCount = (u32)subpasses.Size();
		createInfo.pSubpasses = subpasses.Data();
		createInfo.dependencyCount = (u32)dependencies.Size();
		createInfo.pDependencies = dependencies.Data();
		if (correlationMasks.Size() > 0)
		{
			createInfo.correlatedViewMaskCount = correlationMasks.Size();
			createInfo.pCorrelatedViewMasks = correlationMasks.Data();
		}

		VkRenderPass renderPass = VK_NULL_HANDLE;
		auto result = vkCreateRenderPass2(pDevice->GetHandle(), &createInfo, nullptr, &renderPass);

		return std::make_pair(result, renderPass);
	}


	VkObj<VkSRLObj> CreateShaderResourceLayout(BvRenderDeviceVk* pDevice, const ShaderResourceLayoutCreateDesc& shaderResourceLayoutDesc)
	{
		VkObj<VkSRLObj> srl{};
		auto& result = srl.first;
		auto& layouts = srl.second.m_DescriptorSetLayouts;
		auto& pipelineLayout = srl.second.m_PipelineLayout;

		layouts.Resize(shaderResourceLayoutDesc.m_ShaderResourceSets.Size());

		bool pushDescriptor = pDevice->GetDeviceInfo()->m_FeatureFlags.pushDescriptor;

		BvVector<VkDescriptorSetLayoutBinding> bindings;
		BvVector<VkDescriptorBindingFlags> flags;
		BvVector<VkSampler> samplers;
		BvVector<VkPushConstantRange> pushConstants;
		u32 currPushConstantOffset = 0;
		auto device = pDevice->GetHandle();
		for (auto i = 0u; i < shaderResourceLayoutDesc.m_ShaderResourceSets.Size(); ++i)
		{
			auto& set = shaderResourceLayoutDesc.m_ShaderResourceSets[i];
			u32 currSamplerIndex = 0;
			for (auto j = 0u; j < set.m_Resources.Size(); ++j)
			{
				auto& currResource = set.m_Resources[j];
				if (currResource.m_StaticSamplers.Size() == 0)
				{
					bindings.PushBack({ currResource.m_Binding, GetVkDescriptorType(currResource.m_ShaderResourceType, pushDescriptor),
						currResource.m_Count, GetVkShaderStageFlags(currResource.m_ShaderStages), nullptr });
				}
				else
				{
					for (auto k = 0; k < currResource.m_Count; ++k)
					{
						samplers.PushBack(TO_VK(currResource.m_StaticSamplers[k])->GetHandle());
					}
					bindings.PushBack({ currResource.m_Binding, VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER,
						currResource.m_Count, GetVkShaderStageFlags(currResource.m_ShaderStages), samplers.Data() + currSamplerIndex });
					currSamplerIndex += currResource.m_Count;
				}

				//if (currResource.m_Bindless)
				//{
				//	flags.PushBack(VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
				//}
				//else
				//{
				//	flags.PushBack(0);
				//}

				if (set.m_Type == ShaderResourceSetDesc::Type::kDynamic)
				{
					BV_ASSERT(currResource.IsDynamic(), "Dynamic set can only have dynamic resource types");
					result = VK_ERROR_NOT_PERMITTED;
					break;
				}
			}

			VkDescriptorSetLayoutCreateInfo layoutCI{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
			layoutCI.bindingCount = bindings.Size();
			layoutCI.pBindings = bindings.Data();

			VkDescriptorSetLayoutBindingFlagsCreateInfo flagsCI{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO };
			if (set.m_Type == ShaderResourceSetDesc::Type::kBindless)
			{
				flags.Resize(set.m_Resources.Size(), VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
				flagsCI.bindingCount = flags.Size();
				flagsCI.pBindingFlags = flags.Data();
				layoutCI.pNext = &flagsCI;
				layoutCI.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
			}

			if (layouts.Size() <= set.m_Index)
			{
				layouts.Resize(set.m_Index + 1, VK_NULL_HANDLE);
			}

			result = vkCreateDescriptorSetLayout(device, &layoutCI, nullptr, &layouts[set.m_Index]);
			if (result != VK_SUCCESS)
			{
				break;
			}

			bindings.Clear();
			samplers.Clear();

			for (auto j = 0u; j < set.m_Constants.Size(); ++j)
			{
				auto& constant = set.m_Constants[j];
				pushConstants.PushBack({ GetVkShaderStageFlags(constant.m_ShaderStages), currPushConstantOffset, constant.m_Size });
				currPushConstantOffset += constant.m_Size;
			}
		}

		if (result == VK_SUCCESS)
		{
			VkPipelineLayoutCreateInfo pipelineCI{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
			if (layouts.Size() > 0)
			{
				pipelineCI.setLayoutCount = u32(layouts.Size());
				pipelineCI.pSetLayouts = layouts.Data();
			}

			if (pushConstants.Size() > 0)
			{
				pipelineCI.pushConstantRangeCount = u32(pushConstants.Size());
				pipelineCI.pPushConstantRanges = pushConstants.Data();
			}
			result = vkCreatePipelineLayout(device, &pipelineCI, nullptr, &pipelineLayout);
		}

		if (result != VK_SUCCESS)
		{
			for (auto dsl : layouts)
			{
				if (dsl != VK_NULL_HANDLE)
				{
					DestroyDeviceObject(device, dsl);
				}
			}

			if (pipelineLayout)
			{
				DestroyDeviceObject(device, pipelineLayout);
			}
		}

		return srl;
	}


	VkObj<VkPipelineCache> CreatePipelineCache(BvRenderDeviceVk* pDevice, const PipelineCacheInitData* pInitData)
	{
		VkObj<VkPipelineCache> cacheObj;
		auto& result = cacheObj.first;
		auto& pipelineCache = cacheObj.second;

		VkPipelineCacheCreateInfo ci{ VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
		//ci.pNext = nullptr;
		//ci.flags = 0;
		if (pInitData)
		{
			ci.initialDataSize = pInitData->m_Size;
			ci.pInitialData = pInitData->m_pInitData;
		}

		result = vkCreatePipelineCache(*pDevice, &ci, nullptr, &pipelineCache);

		return cacheObj;
	}


	VkObj<VkPipeline> CreateGraphicsPipeline(BvRenderDeviceVk* pDevice, const GraphicsPipelineStateDesc& pipelineStateDesc, VkPipelineCache pipelineCache)
	{
		BvFixedVector<VkVertexInputAttributeDescription, kMaxVertexBindings> attributeDescs;
		BvFixedVector<VkVertexInputBindingDescription, kMaxVertexBindings> bindingDescs;
		BvFixedVector<VkVertexInputBindingDivisorDescriptionEXT, kMaxVertexBindings> divisorDescs;

		const auto& featureFlags = pDevice->GetDeviceInfo()->m_FeatureFlags;

		{
			BvFixedVector<u32, kMaxVertexBindings> bindingIndices(kMaxVertexBindings, kU32Max);
			BvFixedVector<u32, kMaxVertexBindings> bindingElemLocations(kMaxVertexBindings, 0);
			for (auto i = 0u; i < pipelineStateDesc.m_VertexInputDescs.Size(); i++)
			{
				auto& viDesc = pipelineStateDesc.m_VertexInputDescs[i];
				u32 stride = BvRenderUtils::GetFormatInfo(viDesc.m_Format).m_BitsPerPixel >> 3;

				// We keep track of each binding that hasn't already been set, and add the elements
				// when a new one is found (which means bindingIndex == kU32Max)
				auto& bindingIndex = bindingIndices[viDesc.m_Binding];
				if (bindingIndex == kU32Max)
				{
					bindingDescs.PushBack({ viDesc.m_Binding, 0,
						GetVkVertexInputRate(viDesc.m_InputRate) });
					bindingIndex = bindingDescs.Size() - 1;
				}

				auto& attribute = attributeDescs.EmplaceBack();
				attribute.format = GetVkFormat(viDesc.m_Format);
				attribute.location = bindingElemLocations[viDesc.m_Binding]++;
				attribute.binding = viDesc.m_Binding;
				attribute.offset = viDesc.m_Offset;
				if (attribute.offset == VertexInputDesc::kAutoOffset)
				{
					attribute.offset = bindingDescs[bindingIndex].stride;
				}

				bindingDescs[bindingIndex].stride += stride;

				if (viDesc.m_InputRate == InputRate::kPerInstance)
				{
					divisorDescs.PushBack({ viDesc.m_Binding, viDesc.m_InstanceRate });
				}
			}
		}

		VkPipelineVertexInputDivisorStateCreateInfoEXT vertexDivisorStateCI{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT };
		vertexDivisorStateCI.vertexBindingDivisorCount = divisorDescs.Size();
		vertexDivisorStateCI.pVertexBindingDivisors = divisorDescs.Data();

		VkPipelineVertexInputStateCreateInfo vertexCI{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		vertexCI.pNext = featureFlags.vertexAttributeDivisor && divisorDescs.Size() > 0 ? &vertexDivisorStateCI : nullptr;
		if (bindingDescs.Size() > 0)
		{
			vertexCI.vertexBindingDescriptionCount = (u32)bindingDescs.Size();
			vertexCI.pVertexBindingDescriptions = bindingDescs.Data();
		}
		if (attributeDescs.Size() > 0)
		{
			vertexCI.vertexAttributeDescriptionCount = (u32)attributeDescs.Size();
			vertexCI.pVertexAttributeDescriptions = attributeDescs.Data();
		}

		VkPipelineInputAssemblyStateCreateInfo iaCI{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		iaCI.primitiveRestartEnable = pipelineStateDesc.m_InputAssemblyStateDesc.m_PrimitiveRestart;
		iaCI.topology = GetVkPrimitiveTopology(pipelineStateDesc.m_InputAssemblyStateDesc.m_Topology);

		VkPipelineTessellationStateCreateInfo tessCI{ VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, nullptr, 0, pipelineStateDesc.m_PatchControlPoints };

		VkPipelineViewportStateCreateInfo viewportCI{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };

		VkPipelineRasterizationStateCreateInfo rasterizerCI{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		rasterizerCI.polygonMode = GetVkPolygonMode(pipelineStateDesc.m_RasterizerStateDesc.m_FillMode);
		rasterizerCI.cullMode = GetVkCullModeFlags(pipelineStateDesc.m_RasterizerStateDesc.m_CullMode);
		rasterizerCI.frontFace = GetVkFrontFace(pipelineStateDesc.m_RasterizerStateDesc.m_FrontFace);
		rasterizerCI.depthBiasEnable = (pipelineStateDesc.m_RasterizerStateDesc.m_DepthBias != 0 ||
			pipelineStateDesc.m_RasterizerStateDesc.m_DepthBiasSlope != 0.0f) ? VK_TRUE : VK_FALSE;
		rasterizerCI.depthBiasConstantFactor = f32(pipelineStateDesc.m_RasterizerStateDesc.m_DepthBias);
		rasterizerCI.depthBiasClamp = pipelineStateDesc.m_RasterizerStateDesc.m_DepthBiasClamp;
		rasterizerCI.depthBiasSlopeFactor = pipelineStateDesc.m_RasterizerStateDesc.m_DepthBiasSlope;
		rasterizerCI.depthClampEnable = pipelineStateDesc.m_RasterizerStateDesc.m_EnableDepthClip;
		rasterizerCI.lineWidth = 1.0f;
		const void** ppRasterNext = &rasterizerCI.pNext;

		VkDepthBiasRepresentationInfoEXT dbrI{ VK_STRUCTURE_TYPE_DEPTH_BIAS_REPRESENTATION_INFO_EXT };
		if (rasterizerCI.depthBiasEnable && featureFlags.depthBiasControl)
		{
			dbrI.depthBiasExact = VK_TRUE;
			*ppRasterNext = &dbrI;
			ppRasterNext = &dbrI.pNext;
		}
		else
		{
			auto dsFormat = pipelineStateDesc.m_DepthStencilFormat;
			if (dsFormat == Format::kUnknown && pipelineStateDesc.m_pRenderPass)
			{
				auto& rpDesc = pipelineStateDesc.m_pRenderPass->GetDesc();
				for (auto i = 0; i < rpDesc.m_Attachments.Size(); ++i)
				{
					auto fi = BvRenderUtils::GetFormatInfo(rpDesc.m_Attachments[i].m_Format);
					if (fi.m_IsDepthStencil)
					{
						dsFormat = rpDesc.m_Attachments[i].m_Format;
						break;
					}
				}
			}
			if (dsFormat != Format::kUnknown)
			{
				rasterizerCI.depthBiasConstantFactor = Internal::GetDepthBiasD3DToVk(dsFormat, rasterizerCI.depthBiasConstantFactor);
			}
		}

		VkPipelineRasterizationDepthClipStateCreateInfoEXT depthClip{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT };
		if (pDevice->GetDeviceInfo()->m_ExtendedFeatures.depthClibEnableFeature.depthClipEnable)
		{
			depthClip.depthClipEnable = VK_TRUE;
			*ppRasterNext = &depthClip;
			ppRasterNext = &depthClip.pNext;
		}

		VkPipelineRasterizationConservativeStateCreateInfoEXT conservativeRaster{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT };
		if (pipelineStateDesc.m_RasterizerStateDesc.m_EnableConservativeRasterization && featureFlags.conservativeRasterization)
		{
			const auto& conservativeRasterProps = pDevice->GetDeviceInfo()->m_ExtendedProperties.convervativeRasterizationProps;

			if (conservativeRasterProps.maxExtraPrimitiveOverestimationSize > 0.0f)
			{
				conservativeRaster.conservativeRasterizationMode =
					VkConservativeRasterizationModeEXT::VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT;
				conservativeRaster.extraPrimitiveOverestimationSize = conservativeRasterProps.maxExtraPrimitiveOverestimationSize;

				*ppRasterNext = &conservativeRaster;
			}
		}

		VkPipelineDepthStencilStateCreateInfo depthStencilCI{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		depthStencilCI.depthTestEnable = pipelineStateDesc.m_DepthStencilDesc.m_DepthTestEnable;
		depthStencilCI.depthWriteEnable = pipelineStateDesc.m_DepthStencilDesc.m_DepthWriteEnable;
		depthStencilCI.depthCompareOp = GetVkCompareOp(pipelineStateDesc.m_DepthStencilDesc.m_DepthOp);
		depthStencilCI.stencilTestEnable = pipelineStateDesc.m_DepthStencilDesc.m_StencilTestEnable;

		depthStencilCI.front.failOp = GetVkStencilOp(pipelineStateDesc.m_DepthStencilDesc.m_StencilFront.m_StencilFailOp);
		depthStencilCI.front.depthFailOp = GetVkStencilOp(pipelineStateDesc.m_DepthStencilDesc.m_StencilFront.m_StencilDepthFailOp);
		depthStencilCI.front.passOp = GetVkStencilOp(pipelineStateDesc.m_DepthStencilDesc.m_StencilFront.m_StencilPassOp);
		depthStencilCI.front.compareOp = GetVkCompareOp(pipelineStateDesc.m_DepthStencilDesc.m_StencilFront.m_StencilFunc);
		depthStencilCI.front.writeMask = pipelineStateDesc.m_DepthStencilDesc.m_StencilWriteMask;
		depthStencilCI.front.compareMask = pipelineStateDesc.m_DepthStencilDesc.m_StencilReadMask;
		//depthStencilCI.front.reference = 0; // Set dynamically

		depthStencilCI.back.failOp = GetVkStencilOp(pipelineStateDesc.m_DepthStencilDesc.m_StencilBack.m_StencilFailOp);
		depthStencilCI.back.depthFailOp = GetVkStencilOp(pipelineStateDesc.m_DepthStencilDesc.m_StencilBack.m_StencilDepthFailOp);
		depthStencilCI.back.passOp = GetVkStencilOp(pipelineStateDesc.m_DepthStencilDesc.m_StencilBack.m_StencilPassOp);
		depthStencilCI.back.compareOp = GetVkCompareOp(pipelineStateDesc.m_DepthStencilDesc.m_StencilBack.m_StencilFunc);
		depthStencilCI.back.writeMask = pipelineStateDesc.m_DepthStencilDesc.m_StencilWriteMask;
		depthStencilCI.back.compareMask = pipelineStateDesc.m_DepthStencilDesc.m_StencilReadMask;
		//depthStencilCI.back.reference = 0; // Set dynamically

		depthStencilCI.depthBoundsTestEnable = pipelineStateDesc.m_DepthStencilDesc.m_DepthBoundsTestEnable;
		//depthStencilCI.minDepthBounds = 0.0f; // Set dynamically
		//depthStencilCI.maxDepthBounds = 1.0f; // Set dynamically

		u32 sampleMask[] = { pipelineStateDesc.m_SampleMask, 0 };
		VkPipelineMultisampleStateCreateInfo msCI{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		msCI.rasterizationSamples = GetVkSampleCountFlagBits(pipelineStateDesc.m_SampleCount);
		msCI.pSampleMask = sampleMask;

		BvFixedVector<VkFormat, kMaxRenderTargets> rtvFormats;
		for (auto i = 0; i < pipelineStateDesc.m_RenderTargetFormats.Size(); i++)
		{
			rtvFormats.EmplaceBack(GetVkFormat(pipelineStateDesc.m_RenderTargetFormats[i]));
		}

		VkPipelineRenderingCreateInfo pipelineRenderingCI{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
		VkRenderPass renderPass = pipelineStateDesc.m_pRenderPass ? static_cast<BvRenderPassVk*>(pipelineStateDesc.m_pRenderPass)->GetHandle() : VK_NULL_HANDLE;
		if (renderPass == VK_NULL_HANDLE)
		{
			pipelineRenderingCI.colorAttachmentCount = rtvFormats.Size();
			pipelineRenderingCI.pColorAttachmentFormats = rtvFormats.Data();
			pipelineRenderingCI.depthAttachmentFormat = GetVkFormat(pipelineStateDesc.m_DepthStencilFormat);
			pipelineRenderingCI.stencilAttachmentFormat = pipelineRenderingCI.depthAttachmentFormat;
			if (pipelineStateDesc.m_MultiviewCount > 1)
			{
				pipelineRenderingCI.viewMask = (1 << pipelineStateDesc.m_MultiviewCount) - 1;
			}
		}

		auto attachmentCount = rtvFormats.Size();
		if (!attachmentCount && pipelineStateDesc.m_pRenderPass)
		{
			auto& subpass = pipelineStateDesc.m_pRenderPass->GetDesc().m_Subpasses[pipelineStateDesc.m_SubpassIndex];
			attachmentCount = subpass.m_ColorAttachments.Size();
		}

		BvFixedVector<VkPipelineColorBlendAttachmentState, kMaxRenderTargets> blendAttachments(attachmentCount, {});
		for (auto i = 0u; i < blendAttachments.Size(); i++)
		{
			blendAttachments[i].blendEnable = pipelineStateDesc.m_BlendStateDesc.m_BlendAttachments[i].m_BlendEnable;
			blendAttachments[i].srcColorBlendFactor = GetVkBlendFactor(pipelineStateDesc.m_BlendStateDesc.m_BlendAttachments[i].m_SrcBlend);
			blendAttachments[i].srcAlphaBlendFactor = GetVkBlendFactor(pipelineStateDesc.m_BlendStateDesc.m_BlendAttachments[i].m_SrcBlendAlpha);
			blendAttachments[i].colorBlendOp = GetVkBlendOp(pipelineStateDesc.m_BlendStateDesc.m_BlendAttachments[i].m_BlendOp);
			blendAttachments[i].dstColorBlendFactor = GetVkBlendFactor(pipelineStateDesc.m_BlendStateDesc.m_BlendAttachments[i].m_DstBlend);
			blendAttachments[i].dstAlphaBlendFactor = GetVkBlendFactor(pipelineStateDesc.m_BlendStateDesc.m_BlendAttachments[i].m_DstBlendAlpha);
			blendAttachments[i].alphaBlendOp = GetVkBlendOp(pipelineStateDesc.m_BlendStateDesc.m_BlendAttachments[i].m_AlphaBlendOp);
			blendAttachments[i].colorWriteMask = VkColorComponentFlags(pipelineStateDesc.m_BlendStateDesc.m_BlendAttachments[i].m_RenderTargetWriteMask);
		}

		VkPipelineColorBlendStateCreateInfo blendCI{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		if (blendAttachments.Size() > 0)
		{
			blendCI.attachmentCount = (u32)blendAttachments.Size();
			blendCI.pAttachments = blendAttachments.Data();
		}
		blendCI.logicOpEnable = pipelineStateDesc.m_BlendStateDesc.m_LogicEnable;
		blendCI.logicOp = GetVkLogicOp(pipelineStateDesc.m_BlendStateDesc.m_LogicOp);

		bool hasTessellationShaders = false;
		bool hasMeshShaders = false;
		BvFixedVector<VkPipelineShaderStageCreateInfo, kMaxShaderStages> shaderStages;
		for (auto i = 0u; i < pipelineStateDesc.m_Shaders.Size(); i++)
		{
			auto& shaderStage = shaderStages.EmplaceBack(VkPipelineShaderStageCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO });
			const auto& byteCode = pipelineStateDesc.m_Shaders[i]->GetShaderBlob();
			shaderStage.pName = pipelineStateDesc.m_Shaders[i]->GetEntryPoint();
			shaderStage.stage = GetVkShaderStageFlagBits(pipelineStateDesc.m_Shaders[i]->GetShaderStage());
			if (shaderStage.stage & (VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT))
			{
				hasTessellationShaders = true;
			}
			else if (shaderStage.stage & (VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT))
			{
				hasMeshShaders = true;
			}
			auto shaderObj = CreateShaderModule(pDevice->GetHandle(), byteCode.Size(), byteCode.Data());
			shaderStage.module = shaderObj.second;
		}

		BV_ASSERT(hasMeshShaders != hasTessellationShaders
			|| (hasMeshShaders == false && hasTessellationShaders == false), "Pipeline can't have both mesh and tess shaders");

		constexpr u32 kMaxDynamicStates = 8; // Change as needed
		BvFixedVector<VkDynamicState, kMaxDynamicStates> dynamicStates{};
		dynamicStates.PushBack(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
		dynamicStates.PushBack(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
		dynamicStates.PushBack(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
		dynamicStates.PushBack(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
		if (!hasMeshShaders && vertexCI.vertexBindingDescriptionCount > 0)
		{
			dynamicStates.PushBack(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
		}
		if (pipelineStateDesc.m_DepthStencilDesc.m_DepthBoundsTestEnable && pDevice->GetDeviceInfo()->m_DeviceFeatures.features.depthBounds)
		{
			dynamicStates.PushBack(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
		}
		if (pipelineStateDesc.m_ShadingRateEnabled && featureFlags.fragmentShading)
		{
			dynamicStates.PushBack(VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR);
		}

		VkPipelineDynamicStateCreateInfo dynamicStateCI{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		dynamicStateCI.dynamicStateCount = (u32)dynamicStates.Size();
		dynamicStateCI.pDynamicStates = dynamicStates.Data();

		VkGraphicsPipelineCreateInfo pipelineCI{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		pipelineCI.pNext = !pipelineStateDesc.m_pRenderPass ? &pipelineRenderingCI : nullptr;
		//pipelineCI.flags = 0;
		pipelineCI.stageCount = shaderStages.Size();
		pipelineCI.pStages = shaderStages.Data();
		if (!hasMeshShaders)
		{
			pipelineCI.pVertexInputState = &vertexCI;
			pipelineCI.pInputAssemblyState = &iaCI;
		}
		pipelineCI.pTessellationState = hasTessellationShaders ? &tessCI : nullptr;
		pipelineCI.pViewportState = &viewportCI;
		pipelineCI.pRasterizationState = &rasterizerCI;
		pipelineCI.pMultisampleState = &msCI;
		pipelineCI.pDepthStencilState = &depthStencilCI;
		pipelineCI.pColorBlendState = &blendCI;
		pipelineCI.pDynamicState = &dynamicStateCI;
		pipelineCI.renderPass = renderPass;
		pipelineCI.layout = TO_VK(pipelineStateDesc.m_pShaderResourceLayout)->GetPipelineLayoutHandle();
		pipelineCI.subpass = pipelineStateDesc.m_SubpassIndex;
		pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCI.basePipelineIndex = -1;

		VkPipeline pipeline = VK_NULL_HANDLE;
		auto result = vkCreateGraphicsPipelines(pDevice->GetHandle(), pipelineCache, 1, &pipelineCI, nullptr, &pipeline);

		for (auto i = 0u; i < shaderStages.Size(); i++)
		{
			DestroyDeviceObject(pDevice->GetHandle(), shaderStages[i].module);
		}

		return std::make_pair(result, pipeline);
	}


	VkObj<VkPipeline> CreateComputePipeline(BvRenderDeviceVk* pDevice, const ComputePipelineStateDesc& pipelineStateDesc, VkPipelineCache pipelineCache)
	{
		auto device = pDevice->GetHandle();

		VkComputePipelineCreateInfo pipelineCI{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
		pipelineCI.layout = TO_VK(pipelineStateDesc.m_pShaderResourceLayout)->GetPipelineLayoutHandle();
		pipelineCI.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pipelineCI.stage.pName = pipelineStateDesc.m_pShader->GetEntryPoint();
		pipelineCI.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		auto& blob = pipelineStateDesc.m_pShader->GetShaderBlob();
		auto shaderObj = CreateShaderModule(device, blob.Size(), blob.Data());
		pipelineCI.stage.module = shaderObj.second;
		pipelineCI.basePipelineIndex = -1;

		VkPipeline pipeline = VK_NULL_HANDLE;
		auto result = vkCreateComputePipelines(device, pipelineCache, 1, &pipelineCI, nullptr, &pipeline);

		DestroyDeviceObject(device, pipelineCI.stage.module);

		return std::make_pair(result, pipeline);
	}


	VkObj<VkPipeline> CreateRayTracingPipeline(BvRenderDeviceVk* pDevice, const RayTracingPipelineStateDesc& pipelineStateDesc, VkPipelineCache pipelineCache)
	{
		BvVector<VkPipelineShaderStageCreateInfo> shaderStages(pipelineStateDesc.m_Shaders.Size(), { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO });
		BvVector<VkRayTracingShaderGroupCreateInfoKHR> groups(pipelineStateDesc.m_ShaderGroupDescs.Size(), { VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR });

		auto device = pDevice->GetHandle();
		for (auto i = 0u; i < pipelineStateDesc.m_Shaders.Size(); i++)
		{
			auto& shaderStage = shaderStages[i];
			const auto& byteCode = pipelineStateDesc.m_Shaders[i]->GetShaderBlob();
			shaderStage.pName = pipelineStateDesc.m_Shaders[i]->GetEntryPoint();
			shaderStage.stage = GetVkShaderStageFlagBits(pipelineStateDesc.m_Shaders[i]->GetShaderStage());
			auto shaderObj = CreateShaderModule(device, byteCode.Size(), byteCode.Data());
			shaderStage.module = shaderObj.second;
		}

		for (auto i = 0u; i < pipelineStateDesc.m_ShaderGroupDescs.Size(); ++i)
		{
			auto& groupDesc = pipelineStateDesc.m_ShaderGroupDescs[i];
			BV_ASSERT(groupDesc.m_Type != ShaderGroupType::kNone, "Shader hit group type can't be kNone");

			auto& group = groups[i];
			group.type = GetVkRayTracingShaderGroupType(groupDesc.m_Type);
			group.generalShader = groupDesc.m_General;
			group.closestHitShader = groupDesc.m_ClosestHit;
			group.anyHitShader = groupDesc.m_AnyHit;
			group.intersectionShader = groupDesc.m_Intersection;
		}

		VkRayTracingPipelineInterfaceCreateInfoKHR pici{ VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_INTERFACE_CREATE_INFO_KHR, nullptr,
			pipelineStateDesc.m_MaxPayloadSize, pipelineStateDesc.m_MaxAttributeSize };

		VkRayTracingPipelineCreateInfoKHR ci{ VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR };
		ci.groupCount = u32(groups.Size());
		ci.pGroups = groups.Data();
		ci.stageCount = u32(shaderStages.Size());
		ci.pStages = shaderStages.Data();
		ci.maxPipelineRayRecursionDepth = pipelineStateDesc.m_MaxPipelineRayRecursionDepth;
		ci.layout = TO_VK(pipelineStateDesc.m_pShaderResourceLayout)->GetPipelineLayoutHandle();
		if (pipelineStateDesc.m_ForcePayloadAndAttributeSizes)
		{
			ci.pLibraryInterface = &pici;
		}

		VkPipeline pipeline = VK_NULL_HANDLE;
		auto result = vkCreateRayTracingPipelinesKHR(device, VK_NULL_HANDLE, pipelineCache, 1, &ci, nullptr, &pipeline);

		for (auto i = 0u; i < shaderStages.Size(); i++)
		{
			DestroyDeviceObject(device, shaderStages[i].module);
		}

		return std::make_pair(result, pipeline);
	}


	VkObj<VkSemaphore> CreateSemaphore(BvRenderDeviceVk* pDevice, const GPUFenceDesc& fenceDesc, bool isTimelineSemaphore)
	{
		VkSemaphoreTypeCreateInfo timelineCreateInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO };
		if (isTimelineSemaphore) [[likely]]
		{
			//timelineCreateInfo.pNext = nullptr;
			timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
			timelineCreateInfo.initialValue = fenceDesc.m_Value;
		}

		VkSemaphoreCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		createInfo.pNext = isTimelineSemaphore ? &timelineCreateInfo : nullptr;
		
		VkSemaphore semaphore = VK_NULL_HANDLE;
		auto result = vkCreateSemaphore(pDevice->GetHandle(), &createInfo, nullptr, &semaphore);
		
		return std::make_pair(result, semaphore);
	}


	VkObj<VkQueryPoolObj> CreateQueryPool(BvRenderDeviceVk* pDevice, const QueryHeapDesc& queryHeapDesc)
	{
		return CreateQueryPool(pDevice, GetVkQueryType(queryHeapDesc.m_Type), queryHeapDesc.m_Count, queryHeapDesc.m_Type == QueryType::kMeshPipelineStatistics);
	}


	VkObj<VkQueryPoolObj> CreateQueryPool(BvRenderDeviceVk* pDevice, VkQueryType queryType, u32 queryCount, bool meshPrimitivesPool)
	{
		VkObj<VkQueryPoolObj> obj{};
		auto& result = obj.first;
		auto& psoFlags = obj.second.m_PSOFlags;
		auto& queryPools = obj.second.m_QueryPools;

		VkQueryPoolCreateInfo qpCI{ VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
		qpCI.queryType = queryType;
		qpCI.queryCount = queryCount;
		if (qpCI.queryType == VK_QUERY_TYPE_PIPELINE_STATISTICS)
		{
			auto pDeviceInfo = pDevice->GetDeviceInfo();
			bool geometryShader = pDeviceInfo->m_DeviceFeatures.features.geometryShader;
			bool tessellationShader = pDeviceInfo->m_DeviceFeatures.features.tessellationShader;
			bool meshShader = pDeviceInfo->m_ExtendedFeatures.meshShaderFeatures.meshShaderQueries;
			qpCI.pipelineStatistics = GetVkQueryPipelineFlags(geometryShader, tessellationShader, meshPrimitivesPool && meshShader);
		}

		auto device = pDevice->GetHandle();
		result = vkCreateQueryPool(device, &qpCI, nullptr, &queryPools[0]);
		if (result != VK_SUCCESS)
		{
			return obj;
		}

		if (meshPrimitivesPool)
		{
			qpCI.queryType = VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT;
			result = vkCreateQueryPool(device, &qpCI, nullptr, &queryPools[1]);
			if (result != VK_SUCCESS)
			{
				DestroyDeviceObject(device, queryPools[0]);
			}
		}


		return obj;
	}


	VkObj<VkASObj> CreateRayTracingAccelerationStructure(BvRenderDeviceVk* pDevice, const RayTracingAccelerationStructureDesc& asDesc)
	{
		VkObj<VkASObj> asObj{};
		auto& result = asObj.first;
		auto& as = asObj.second;

		auto& asHandle = as.m_AS;
		auto& asDeviceAddress = as.m_DeviceAddress;
		auto& geometries = as.m_Geometries;
		auto& ranges = as.m_Ranges;
		auto& buffer = as.m_BufferObj;
		auto& scratchSizes = as.m_ScratchSizes;

		BvVector<u32> primitiveCounts;

		VkAccelerationStructureBuildSizesInfoKHR sizeInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR, nullptr, asDesc.m_CompactedSize, 0, 0 };
		VkAccelerationStructureTypeKHR asType{};

		auto device = pDevice->GetHandle();

		if (asDesc.m_CompactedSize == 0)
		{
			if (asDesc.m_Type == RayTracingAccelerationStructureType::kBottomLevel)
			{
				asType = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

				geometries.Resize(asDesc.m_Geometries.Size(), { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR });
				ranges.Resize(geometries.Size(), {});
				primitiveCounts.Resize(geometries.Size(), 0);

				for (auto i = 0u; i < asDesc.m_Geometries.Size(); ++i)
				{
					if (asDesc.m_Geometries[i].m_Type == RayTracingGeometryType::kTriangles)
					{
						auto& srcGeometry = asDesc.m_Geometries[i].m_Triangle;

						VkAccelerationStructureGeometryKHR& dstGeometry = geometries[i];
						//dstGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
						//dstGeometry.pNext = nullptr;
						dstGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
						dstGeometry.flags = GetVkGeometryFlags(asDesc.m_Geometries[i].m_Flags);

						VkAccelerationStructureGeometryTrianglesDataKHR& triangle = dstGeometry.geometry.triangles;
						triangle.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
						//triangle.pNext = nullptr;
						triangle.vertexFormat = GetVkFormat(srcGeometry.m_VertexFormat);
						triangle.vertexStride = srcGeometry.m_VertexStride;
						triangle.maxVertex = srcGeometry.m_VertexCount - 1;
						triangle.indexType = GetVkIndexType(srcGeometry.m_IndexFormat);

						u32 primitiveCount = srcGeometry.m_IndexCount > 0 ? srcGeometry.m_IndexCount / 3 : srcGeometry.m_VertexCount / 3;
						primitiveCounts[i] = primitiveCount;
						ranges[i].primitiveCount = primitiveCount;
					}
					else if (asDesc.m_Geometries[i].m_Type == RayTracingGeometryType::kAABB)
					{
						auto& srcGeometry = asDesc.m_Geometries[i].m_AABB;

						VkAccelerationStructureGeometryKHR& dstGeometry = geometries[i];
						//dstGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
						//dstGeometry.pNext = nullptr;
						dstGeometry.geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
						dstGeometry.flags = GetVkGeometryFlags(asDesc.m_Geometries[i].m_Flags);

						VkAccelerationStructureGeometryAabbsDataKHR& aabb = dstGeometry.geometry.aabbs;
						aabb.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
						//aabb.pNext = nullptr;
						aabb.stride = srcGeometry.m_Stride;

						primitiveCounts[i] = srcGeometry.m_Count;
						ranges[i].primitiveCount = srcGeometry.m_Count;
					}
				}
			}
			else if (asDesc.m_Type == RayTracingAccelerationStructureType::kTopLevel)
			{
				asType = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

				VkAccelerationStructureGeometryKHR& dstGeometry = geometries.PushBack({ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR });
				//dstGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
				//dstGeometry.pNext = nullptr;
				dstGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
				dstGeometry.flags = GetVkGeometryFlags(asDesc.m_Geometries[0].m_Flags);

				VkAccelerationStructureGeometryInstancesDataKHR& instances = dstGeometry.geometry.instances;
				instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
				//instances.arrayOfPointers = VK_FALSE;

				primitiveCounts.PushBack(asDesc.m_Geometries[0].m_Instance.m_InstanceCount);
				ranges.EmplaceBack().primitiveCount = primitiveCounts[0];
			}

			VkAccelerationStructureBuildGeometryInfoKHR buildInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
			//buildInfo.pNext = nullptr;
			buildInfo.type = asType;
			buildInfo.flags = GetVkBuildAccelerationStructureFlags(asDesc.m_Flags);
			buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
			buildInfo.geometryCount = u32(geometries.Size());
			buildInfo.pGeometries = geometries.Data();

			vkGetAccelerationStructureBuildSizesKHR(device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, primitiveCounts.Data(), &sizeInfo);

			scratchSizes.m_Build = sizeInfo.buildScratchSize;
			scratchSizes.m_Update = sizeInfo.updateScratchSize;
		}

		BufferDesc bufferDesc;
		bufferDesc.m_UsageFlags = BufferUsage::kRayTracing;
		bufferDesc.m_MemoryType = MemoryType::kDevice;
		bufferDesc.m_Size = sizeInfo.accelerationStructureSize;
		auto bufferObj = CreateBuffer(pDevice, bufferDesc);

		result = bufferObj.first;
		if (result == VK_SUCCESS)
		{
			buffer = bufferObj.second;

			VkAccelerationStructureCreateInfoKHR accelerationStructureCI{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
			//accelerationStructureCI.createFlags = 0;
			accelerationStructureCI.buffer = bufferObj.second.m_Buffer;
			//accelerationStructureCI.offset = 0;
			accelerationStructureCI.size = sizeInfo.accelerationStructureSize;
			accelerationStructureCI.type = asType;
			result = vkCreateAccelerationStructureKHR(device, &accelerationStructureCI, nullptr, &asHandle);
			if (result == VK_SUCCESS)
			{
				VkAccelerationStructureDeviceAddressInfoKHR addressInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR, nullptr, asHandle };
				asDeviceAddress = vkGetAccelerationStructureDeviceAddressKHR(device, &addressInfo);
			}
			else
			{
				DestroyDeviceObject(*pDevice, buffer.m_Buffer, pDevice->GetAllocator(), buffer.m_Memory);
				buffer.m_Buffer = VK_NULL_HANDLE;
				buffer.m_Memory = VK_NULL_HANDLE;
			}
		}

		return asObj;
	}

	VkObj<VkSBTObj> CreateShaderBindingTable(BvRenderDeviceVk* pDevice, const ShaderBindingTableDesc& sbtDesc, BvCommandContextVk* pContext)
	{
		VkObj<VkSBTObj> sbtObj;
		auto& result = sbtObj.first;
		auto& sbt = sbtObj.second;
		auto& props = pDevice->GetDeviceInfo()->m_ExtendedProperties.rayTracingPipelineProps;
		auto handleSize = props.shaderGroupHandleSize;
		auto groupHandleAlignment = props.shaderGroupHandleAlignment;
		auto baseGroupAlignment = props.shaderGroupBaseAlignment;

		auto& stages = TO_VK(sbtDesc.m_pPSO)->GetShaderStages();
		BvVector<u32> groupIndices[4];
		auto& psoDesc = sbtDesc.m_pPSO->GetDesc();
		for (auto g = 0; g < psoDesc.m_ShaderGroupDescs.Size(); ++g)
		{
			auto index = 0;
			auto& currGroup = psoDesc.m_ShaderGroupDescs[g];
			if (currGroup.m_Type == ShaderGroupType::kGeneral)
			{
				auto stage = stages[currGroup.m_General];
				switch (stage)
				{
				case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
					index = 0;
					break;
				case VK_SHADER_STAGE_MISS_BIT_KHR:
					index = 1;
					break;
				case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
					index = 3;
					break;
				}
			}
			else
			{
				index = 2;
			}

			groupIndices[index].PushBack(g);
		}

		u32 sbtSize = psoDesc.m_ShaderGroupDescs.Size() * handleSize;
		BvVector<u8> shaderHandleData(sbtSize);

		result = vkGetRayTracingShaderGroupHandlesKHR(pDevice->GetHandle(), TO_VK(sbtDesc.m_pPSO)->GetHandle(), 0,
			psoDesc.m_ShaderGroupDescs.Size(), sbtSize, shaderHandleData.Data());
		if (result != VK_SUCCESS)
		{
			return sbtObj;
		}

		u64 handleSizeAligned = RoundToNearestPowerOf2(handleSize, groupHandleAlignment);
		sbt.m_Regions[0].stride = RoundToNearestPowerOf2(handleSizeAligned, baseGroupAlignment);
		sbt.m_Regions[0].size = sbt.m_Regions[0].stride;
		sbt.m_Regions[1].stride = handleSizeAligned;
		sbt.m_Regions[1].size = RoundToNearestPowerOf2(handleSizeAligned * groupIndices[1].Size(), baseGroupAlignment);
		sbt.m_Regions[2].stride = handleSizeAligned;
		sbt.m_Regions[2].size = RoundToNearestPowerOf2(handleSizeAligned * groupIndices[2].Size(), baseGroupAlignment);
		sbt.m_Regions[3].stride = handleSizeAligned;
		sbt.m_Regions[3].size = RoundToNearestPowerOf2(handleSizeAligned * groupIndices[3].Size(), baseGroupAlignment);

		BufferDesc bufferDesc;
		bufferDesc.m_UsageFlags = BufferUsage::kRayTracing;
		bufferDesc.m_Size = sbt.m_Regions[0].stride * groupIndices[0].Size() + sbt.m_Regions[1].size + sbt.m_Regions[2].size + sbt.m_Regions[3].size;

		BvVector<u8> bufferData(bufferDesc.m_Size);
		auto pStart = bufferData.Data();
		u64 currOffset = 0;
		for (auto g = 0u; g < 4; ++g)
		{
			auto pBuffer = pStart + currOffset;
			auto& group = groupIndices[g];
			auto stride = sbt.m_Regions[g].stride;
			for (auto i = 0u; i < group.Size(); ++i)
			{
				auto pSrc = shaderHandleData.Data() + (handleSize * group[i]);
				memcpy(pBuffer, pSrc, handleSize);
				pBuffer += stride;
			}
			currOffset += g == 0 ? sbt.m_Regions[g].stride * groupIndices[g].Size() : sbt.m_Regions[g].size;
		}

		{
			auto bufferObj = CreateBuffer(pDevice, bufferDesc);
			result = bufferObj.first;
			if (result != VK_SUCCESS)
			{
				return sbtObj;
			}

			sbt.m_BufferObj = bufferObj.second;

			UploadMemoryToGPU(pDevice, sbt.m_BufferObj.m_Buffer, { pContext, bufferData.Data(), bufferData.Size() });
		}

		currOffset = 0;
		for (auto g = 0; g < 4; ++g)
		{
			if (groupIndices[g].Size() == 0)
			{
				sbt.m_Regions[g] = {};
				continue;
			}

			sbt.m_Regions[g].deviceAddress = sbt.m_BufferObj.m_DeviceAddress + currOffset;
			currOffset += g == 0 ? sbt.m_Regions[g].stride * groupIndices[g].Size() : sbt.m_Regions[g].size;
		}

		return sbtObj;
	}

	void UploadMemoryToGPU(BvRenderDeviceVk* pDevice, VkBuffer buffer, const BufferInitData& initData)
	{
		if (!initData.m_pContext || !initData.m_Size || !initData.m_pData)
		{
			return;
		}

		BufferDesc bufferDesc;
		bufferDesc.m_MemoryType = MemoryType::kUpload;
		bufferDesc.m_Size = initData.m_Size;
		bufferDesc.m_CreateFlags = BufferCreateFlags::kCreateMapped;

		auto stagingBuffer = CreateBuffer(pDevice, bufferDesc);
		if (stagingBuffer.first != VK_SUCCESS)
		{
			return;
		}
		memcpy(stagingBuffer.second.m_pMappedMemory, initData.m_pData, initData.m_Size);

		auto pContext = static_cast<BvCommandContextVk*>(initData.m_pContext);
		pContext->NewCommandList();
		pContext->GetCurrentCommandBuffer()->CopyBuffer(stagingBuffer.second.m_Buffer, buffer, { 0, 0, initData.m_Size });
		pContext->ExecuteAndWait();

		DestroyDeviceObject(*pDevice, stagingBuffer.second.m_Buffer, pDevice->GetAllocator(), stagingBuffer.second.m_Memory);
	}

	void UploadMemoryToGPU(BvRenderDeviceVk* pDevice, VkImage texture, const TextureDesc& textureDesc, const TextureInitData& initData)
	{
		constexpr ResourceState initialState = ResourceState::kCommon;

		// No way to copy and can't stay in VK_IMAGE_LAYOUT_UNDEFINED
		if (!initData.m_pContext || initData.m_ResourceState == initialState)
		{
			return;
		}
		
		ResourceState currState = initialState;

		VkImageMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
		//barrier.pNext = nullptr;
		barrier.image = texture;
		barrier.subresourceRange.aspectMask = GetVkFormatMap(textureDesc.m_Format).aspectFlags;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

		auto pContext = TO_VK(initData.m_pContext);
		pContext->NewCommandList();

		VkObj<VkBufferObj> stagingBuffer{};
		if (initData.m_SubresourceCount && initData.m_pSubresources)
		{
			u64 totalBytes = 0;
			BvVector<VkBufferImageCopy> copyRegions(initData.m_SubresourceCount);
			BvVector<SubresourceFootprint> footprints(initData.m_SubresourceCount);
			pDevice->GetCopyableFootprints(textureDesc, initData.m_SubresourceCount, footprints.Data(), &totalBytes, 0, initData.m_FirstSubresource);

			BufferDesc bufferDesc;
			bufferDesc.m_MemoryType = MemoryType::kUpload;
			bufferDesc.m_Size = totalBytes;
			bufferDesc.m_CreateFlags = BufferCreateFlags::kCreateMapped;
		
			stagingBuffer = CreateBuffer(pDevice, bufferDesc);
			if (stagingBuffer.first != VK_SUCCESS)
			{
				return;
			}

			BvRenderUtils::UpdateSubresources(initData.m_SubresourceCount, initData.m_pSubresources, footprints.Data(), stagingBuffer.second.m_pMappedMemory);

			for (auto i = 0; i < initData.m_SubresourceCount; ++i)
			{
				auto subresourceIndex = initData.m_FirstSubresource + i;
				u32 m, a, p;
				BvRenderUtils::DecomposeSubresourceIndex(subresourceIndex, textureDesc.m_MipLevels, textureDesc.m_ArraySize, m, a, p);

				auto& copyRegion = copyRegions[i];
				auto& footprint = footprints[i];

				copyRegion.bufferOffset = footprint.m_Offset;

				// Planar formats may require this to be revised
				copyRegion.bufferRowLength = 0;
				copyRegion.bufferImageHeight = 0;

				copyRegion.imageOffset = { 0, 0, 0 };
				copyRegion.imageExtent = { footprint.m_Subresource.m_Width, footprint.m_Subresource.m_Height, footprint.m_Subresource.m_Detph };
				copyRegion.imageSubresource.aspectMask = GetVkImageAspectFlags(textureDesc.m_Format, p);
				copyRegion.imageSubresource.mipLevel = m;
				copyRegion.imageSubresource.baseArrayLayer = a;
				copyRegion.imageSubresource.layerCount = 1;
			}

			currState = ResourceState::kTransferDst;

			barrier.oldLayout = GetVkImageLayout(initialState);
			barrier.srcAccessMask = GetVkAccessFlags(initialState);
			barrier.srcStageMask = GetVkPipelineStageFlags(barrier.srcAccessMask);

			barrier.newLayout = GetVkImageLayout(currState);
			barrier.dstAccessMask = GetVkAccessFlags(currState);
			barrier.dstStageMask = GetVkPipelineStageFlags(barrier.dstAccessMask);

			auto pCB = pContext->GetCurrentCommandBuffer();
			pCB->ResourceBarrier(0, nullptr, 1, &barrier, 0, nullptr);
			pCB->CopyBufferToTexture(stagingBuffer.second.m_Buffer, texture, (u32)copyRegions.Size(), copyRegions.Data());
		}

		if (currState != initData.m_ResourceState)
		{
			barrier.oldLayout = GetVkImageLayout(currState);
			barrier.srcAccessMask = GetVkAccessFlags(currState);
			barrier.srcStageMask = GetVkPipelineStageFlags(barrier.srcAccessMask);

			barrier.newLayout = GetVkImageLayout(initData.m_ResourceState);
			barrier.dstAccessMask = GetVkAccessFlags(initData.m_ResourceState);
			barrier.dstStageMask = GetVkPipelineStageFlags(barrier.dstAccessMask);

			pContext->GetCurrentCommandBuffer()->ResourceBarrier(0, nullptr, 1, &barrier, 0, nullptr);
		}

		pContext->ExecuteAndWait();

		if (stagingBuffer.second.m_Buffer)
		{
			DestroyDeviceObject(*pDevice, stagingBuffer.second.m_Buffer, pDevice->GetAllocator(), stagingBuffer.second.m_Memory);
		}
	}
}