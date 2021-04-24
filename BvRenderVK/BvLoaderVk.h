#pragma once


#include "BvCommonVk.h"
#include "BvCore/System/Library/BvSharedLib.h"


namespace VulkanFunctions
{
#define BV_VK_EXPORTED_FUNCTION(pfn)		extern PFN_##pfn pfn;
#define BV_VK_GLOBAL_LEVEL_FUNCTION(pfn)	extern PFN_##pfn pfn;
#define BV_VK_INSTANCE_LEVEL_FUNCTION(pfn)	extern PFN_##pfn pfn;
#define BV_VK_DEVICE_LEVEL_FUNCTION(pfn)	PFN_##pfn pfn;


	BV_VK_EXPORTED_FUNCTION(vkGetInstanceProcAddr);

	BV_VK_GLOBAL_LEVEL_FUNCTION(vkCreateInstance);
	BV_VK_GLOBAL_LEVEL_FUNCTION(vkEnumerateInstanceExtensionProperties);

	BV_VK_INSTANCE_LEVEL_FUNCTION(vkEnumeratePhysicalDevices);
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkGetPhysicalDeviceProperties);
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkGetPhysicalDeviceFeatures);
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties);
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkCreateDevice);
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkGetDeviceProcAddr);
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkDestroyInstance);
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkEnumerateDeviceExtensionProperties);
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkGetPhysicalDeviceSurfaceSupportKHR);
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkGetPhysicalDeviceSurfaceFormatsKHR);
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkGetPhysicalDeviceSurfacePresentModesKHR);
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkDestroySurfaceKHR);
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkGetPhysicalDeviceMemoryProperties);
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkGetPhysicalDeviceProperties2);
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkGetPhysicalDeviceFeatures2);
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkGetPhysicalDeviceFormatProperties);
	
	// =============================================================
	// Debug functions
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkCreateDebugUtilsMessengerEXT);
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkDestroyDebugUtilsMessengerEXT);
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkQueueBeginDebugUtilsLabelEXT);
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkQueueEndDebugUtilsLabelEXT);
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkCmdBeginDebugUtilsLabelEXT);
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkCmdEndDebugUtilsLabelEXT);
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkCmdInsertDebugUtilsLabelEXT);
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkSetDebugUtilsObjectNameEXT);
	// =============================================================

#if defined(VK_USE_PLATFORM_WIN32_KHR)
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkCreateWin32SurfaceKHR);
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkGetPhysicalDeviceWin32PresentationSupportKHR);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkCreateXcbSurfaceKHR);
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
	BV_VK_INSTANCE_LEVEL_FUNCTION(vkCreateXlibSurfaceKHR);
#endif


	struct DeviceFunctions
	{
		// Resource functions
		BV_VK_DEVICE_LEVEL_FUNCTION(vkGetDeviceQueue);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkQueueWaitIdle);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkDeviceWaitIdle);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkDestroyDevice);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCreateSemaphore);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCreateCommandPool);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkResetCommandPool);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkAllocateCommandBuffers);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkQueueSubmit);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkFreeCommandBuffers);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkDestroyCommandPool);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkDestroySemaphore);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCreateSwapchainKHR);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkGetSwapchainImagesKHR);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkAcquireNextImageKHR);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkQueuePresentKHR);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkDestroySwapchainKHR);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCreateImageView);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCreateRenderPass);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCreateFramebuffer);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCreateShaderModule);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCreatePipelineLayout);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCreateGraphicsPipelines);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkDestroyShaderModule);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkDestroyPipelineLayout);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkDestroyPipeline);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkDestroyRenderPass);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkDestroyFramebuffer);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkDestroyImageView);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCreateFence);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCreateBuffer);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkGetBufferMemoryRequirements);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkAllocateMemory);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkBindBufferMemory);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkMapMemory);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkFlushMappedMemoryRanges);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkUnmapMemory);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkWaitForFences);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkResetFences);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkFreeMemory);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkDestroyBuffer);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkDestroyFence);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCreateImage);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkGetImageMemoryRequirements);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkBindImageMemory);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCreateSampler);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCreateDescriptorSetLayout);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCreateDescriptorPool);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkAllocateDescriptorSets);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkUpdateDescriptorSets);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkDestroyDescriptorPool);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkDestroyDescriptorSetLayout);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkDestroySampler);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkDestroyImage);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCreateBufferView);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkDestroyBufferView);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkInvalidateMappedMemoryRanges);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCreateComputePipelines);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkSignalSemaphore);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkWaitSemaphores);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkGetSemaphoreCounterValue);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkFreeDescriptorSets);

		// Command Buffer functions
		BV_VK_DEVICE_LEVEL_FUNCTION(vkBeginCommandBuffer);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkEndCommandBuffer);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCmdPipelineBarrier);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCmdClearColorImage);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCmdBindDescriptorSets);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCmdCopyBufferToImage);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCmdSetViewport);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCmdSetScissor);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCmdBindVertexBuffers);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCmdCopyBuffer);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCmdBeginRenderPass);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCmdBindPipeline);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCmdDraw);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCmdEndRenderPass);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCmdBindIndexBuffer);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCmdDrawIndexed);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCmdDispatch);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCmdDrawIndirect);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCmdDrawIndexedIndirect);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCmdDispatchIndirect);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCmdCopyImage);
		BV_VK_DEVICE_LEVEL_FUNCTION(vkCmdCopyImageToBuffer);
	};


#undef BV_VK_EXPORTED_FUNCTION
#undef BV_VK_GLOBAL_LEVEL_FUNCTION
#undef BV_VK_INSTANCE_LEVEL_FUNCTION
#undef BV_VK_DEVICE_LEVEL_FUNCTION
}


class BvLoaderVk
{
	BV_NOCOPYMOVE(BvLoaderVk);

public:
	BvLoaderVk();
	~BvLoaderVk();

	void LoadExportedFunctions();
	void LoadGlobalFunctions();
	void LoadInstanceFunctions(VkInstance instance);
	void LoadDeviceFunctions(VkDevice device, VulkanFunctions::DeviceFunctions & deviceFunctions);

private:
	BvSharedLib m_DLL;
};