#include "BvVulkanDevice.h"
#include "BvCore/Utils/BvUtils.h"


BvVulkanDevice::BvVulkanDevice()
{
}


BvVulkanDevice::~BvVulkanDevice()
{
	Destroy();
}


void BvVulkanDevice::Create()
{
	CreateInstance();
	SelectGPU();
	CreateDevice();
}


void BvVulkanDevice::Destroy()
{
	if (m_Device)
	{
		vkDestroyDevice(m_Device, nullptr);
		m_Device = VK_NULL_HANDLE;
	}

	if (m_Instance)
	{
		vkDestroyInstance(m_Instance, nullptr);
		m_Instance = VK_NULL_HANDLE;
	}
}


void BvVulkanDevice::WaitIdle()
{
	vkDeviceWaitIdle(m_Device);
}


const uint32_t BvVulkanDevice::GetQueueFamilyIndex(const VkQueueFlags queueFlags) const
{
	auto & queueFamilyProperties = m_GPUInfo.m_QueueFamilyProperties;

	uint32_t index = UINT32_MAX;

	// Dedicated queue for compute
	// Try to find a queue family index that supports compute but not graphics
	if (queueFlags & VK_QUEUE_COMPUTE_BIT)
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.Size()); i++)
		{
			if ((queueFamilyProperties[i].queueFlags & queueFlags)
				&& ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
			{
				index = i;
				break;
			}
		}
	}

	// Dedicated queue for transfer
	// Try to find a queue family index that supports transfer but not graphics and compute
	else if (queueFlags & VK_QUEUE_TRANSFER_BIT)
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.Size()); i++)
		{
			if ((queueFamilyProperties[i].queueFlags & queueFlags)
				&& ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
				&& ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
			{
				index = i;
				break;
			}
		}
	}


	if (UINT32_MAX == index)
	{
		// For other queue types or if no separate compute queue is present, return the first one to support the requested flags
		for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.Size()); i++)
		{
			if (queueFamilyProperties[i].queueFlags & queueFlags)
			{
				index = i;
				break;
			}
		}
	}

	return index;
}


const uint32_t BvVulkanDevice::GetMemoryTypeIndex(const uint32_t memoryTypeBits, const VkMemoryPropertyFlags properties) const
{
	uint32_t index = UINT32_MAX;
	uint32_t typeBits = memoryTypeBits;

	auto & memoryProperties = m_GPUInfo.m_DeviceMemoryProperties;
	// Iterate over all memory types available for the device used in this example
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				index = i;
				break;
			}
		}
		typeBits >>= 1;
	}

	return index;
}


const VkFormat BvVulkanDevice::GetBestDepthFormat(const VkFormat format) const
{
	VkFormat chosenFormat = VK_FORMAT_UNDEFINED;

	// Since all depth formats may be optional, we need to find a suitable depth format to use
	// Start with the highest precision packed format
	VkFormat depthFormats[] =
	{
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

	VkFormatProperties formatProps;
	for (auto & depthFormat : depthFormats)
	{
		vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, depthFormat, &formatProps);
		// Format must support depth stencil attachment for optimal tiling
		if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			chosenFormat = depthFormat;
			if (format == VK_FORMAT_UNDEFINED || format == chosenFormat)
			{
				break;
			}
		}
	}

	return chosenFormat;
}

void BvVulkanDevice::CreateInstance()
{
	const char * const pAppName = "BDeV";
	VkApplicationInfo applicationInfo{};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = pAppName;
	applicationInfo.pEngineName = pAppName;
	applicationInfo.apiVersion = VK_API_VERSION_1_1;

	const char * const enabledExtensions[] =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#if defined(BV_DEBUG)
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
	};

	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;

	instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions;
	instanceCreateInfo.enabledExtensionCount = BvFixedVectorSize(enabledExtensions);

#if defined(BV_DEBUG)
	const char * const validationLayers[] =
	{
		"VK_LAYER_LUNARG_standard_validation"
	};
	instanceCreateInfo.ppEnabledLayerNames = validationLayers;
	instanceCreateInfo.enabledLayerCount = BvFixedVectorSize(validationLayers);
#endif // BV_DEBUG

	vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance);

	BvAssertMsg(m_Instance != VK_NULL_HANDLE, "Couldn't create a Vulkan Instance");
}


void BvVulkanDevice::SelectGPU()
{
	// ===========================================================
	// Get a list of the GPUs with support for Vulkan
	uint32_t physicalDeviceCount = 0;
	VkResult result = vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, nullptr);
	BvAssertMsg(result == VK_SUCCESS, "Couldn't enumerate physical devices");

	BvVector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	result = vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, physicalDevices.Data());
	BvAssertMsg(result == VK_SUCCESS, "Couldn't enumerate physical devices");

	constexpr size_t kInvalidDeviceIndex = static_cast<size_t>(-1);

	size_t deviceIndex = kInvalidDeviceIndex;
	for (size_t i = 0; i < physicalDevices.Size(); ++i)
	{
		// =================================
		// Get the queue families
		// Do this first to check for presentation support
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, nullptr);

		BvVector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, queueFamilyProperties.Data());

		uint32_t presentationQueueIndex = UINT32_MAX;
		for (auto j = 0U; j < queueFamilyProperties.Size(); j++)
		{
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
			if (vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevices[i], j))
			{
				presentationQueueIndex = j;
				deviceIndex = i;
				break;
			}
#endif
		}

		if (deviceIndex != kInvalidDeviceIndex)
		{
			m_GPUInfo.m_QueueFamilyProperties = queueFamilyProperties;
			m_GraphicsQueueIndex = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
			m_ComputeQueueIndex = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);

			if (m_GraphicsQueueIndex == m_ComputeQueueIndex)
			{
				m_Queues.Resize(1, VK_NULL_HANDLE);
			}
			else
			{
				m_Queues.Resize(2, VK_NULL_HANDLE);
			}

			BvAssertMsg(presentationQueueIndex == m_GraphicsQueueIndex, "Graphics queue differs from the Presentation Queue");

			break;
		}
	}

	BvAssertMsg(deviceIndex != kInvalidDeviceIndex, "Couldn't find a GPU with presentation support");

	// =================================
	// Store the physical device
	m_PhysicalDevice = physicalDevices[deviceIndex];

	// =================================
	// Store properties / features / memory properties
	vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_GPUInfo.m_DeviceProperties);
	vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_GPUInfo.m_DeviceFeatures);
	vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &m_GPUInfo.m_DeviceMemoryProperties);

	// =================================
	// Get / Store supported extensions
	uint32_t extensionCount = 0;
	result = vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, nullptr);
	BvAssertMsg(result == VK_SUCCESS, "Couldn't enumerate physical device properties");

	BvVector<VkExtensionProperties> extensionProperties(extensionCount);
	result = vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, extensionProperties.Data());
	BvAssertMsg(result == VK_SUCCESS, "Couldn't enumerate physical device properties");

	for (auto extension : extensionProperties)
	{
		m_GPUInfo.m_SupportedExtensions.EmplaceBack(extension.extensionName);
	}
}


void BvVulkanDevice::CreateDevice()
{
	// ===========================================================
	// Prepare Device Queues and create the logical device
	BvVector<VkDeviceQueueCreateInfo> queueInfos;
	const float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo queueCreateInfo{};

	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.pQueuePriorities = &queuePriority;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.queueFamilyIndex = m_GraphicsQueueIndex;
	queueInfos.PushBack(queueCreateInfo);

	if (m_GraphicsQueueIndex != m_ComputeQueueIndex)
	{
		queueCreateInfo.queueFamilyIndex = m_ComputeQueueIndex;
		queueInfos.PushBack(queueCreateInfo);
	}

	BvVector<const char *> enabledExtensions;
	enabledExtensions.PushBack(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

#if defined(BV_DEBUG)
	if (std::find(m_GPUInfo.m_SupportedExtensions.cbegin(), m_GPUInfo.m_SupportedExtensions.cend(), VK_EXT_DEBUG_MARKER_EXTENSION_NAME)
		!= m_GPUInfo.m_SupportedExtensions.cend())
	{
		enabledExtensions.PushBack(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
	}
#endif

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = queueInfos.Data();
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.Size());
	deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.Data();
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.Size());
	deviceCreateInfo.pEnabledFeatures = &m_GPUInfo.m_DeviceFeatures;

	VkResult result = vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device);
	BvAssertMsg(result == VK_SUCCESS, "Couldn't create a logical device");

	for (auto i = 0U; i < queueInfos.Size(); i++)
	{
		vkGetDeviceQueue(m_Device, queueInfos[i].queueFamilyIndex, 0, &m_Queues[i]);
	}
}