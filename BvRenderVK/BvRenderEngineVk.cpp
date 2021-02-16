#include "BvRenderEngineVk.h"
#include "BvUtilsVk.h"
#include "BvRenderDeviceVk.h"
#include "BvDebugReportVk.h"
#include "BvLoaderVk.h"


constexpr const char * const g_ValidationLayers[] =
{
	"VK_LAYER_LUNARG_standard_validation"
};

// For Android
//constexpr const char * const g_ValidationLayers[] =
//{
//	"VK_LAYER_GOOGLE_threading",
//	"VK_LAYER_LUNARG_parameter_validation",
//	"VK_LAYER_LUNARG_object_tracker",
//	"VK_LAYER_LUNARG_core_validation",
//	"VK_LAYER_LUNARG_swapchain",
//	"VK_LAYER_GOOGLE_unique_objects"
//};


constexpr const char * const g_EnabledExtensionProperties[] =
{
	VK_KHR_SURFACE_EXTENSION_NAME,
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#if defined(BV_DEBUG)
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	VK_EXT_DEBUG_REPORT_EXTENSION_NAME
#endif
};


BvRenderEngineVk::BvRenderEngineVk()
{
	Create();
}


BvRenderEngineVk::~BvRenderEngineVk()
{
	Destroy();
}


bool BvRenderEngineVk::Create()
{
	m_Loader.LoadExportedFunctions();
	m_Loader.LoadGlobalFunctions();

	if (!CreateInstance())
	{
		Destroy();
		return false;
	}

	m_Loader.LoadInstanceFunctions(m_Instance);

	if (!EnumerateGPUs())
	{
		Destroy();
		return false;
	}

	m_Devices.Resize(m_GPUs.Size());

#if defined(BV_DEBUG)
	m_pDebugReport = new BvDebugReportVk(m_Instance);
	m_pDebugReport->Create();
#endif

	return true;
}


void BvRenderEngineVk::Destroy()
{
#if defined(BV_DEBUG)
	if (m_pDebugReport)
	{
		delete m_pDebugReport;
		m_pDebugReport = nullptr;
	}
#endif

	if (m_Instance)
	{
		VulkanFunctions::vkDestroyInstance(m_Instance, nullptr);
		m_Instance = VK_NULL_HANDLE;
	}
}


void BvRenderEngineVk::GetGPUInfo(const u32 index, BvGPUInfo & gpuInfo) const
{
	BvAssert(index < m_GPUs.Size());

	gpuInfo.m_DeviceName = m_GPUs[index].m_DeviceProperties.properties.deviceName;
	gpuInfo.m_DeviceId = m_GPUs[index].m_DeviceProperties.properties.deviceID;
	gpuInfo.m_VendorId = m_GPUs[index].m_DeviceProperties.properties.vendorID;
	
	const auto & memoryProperties = m_GPUs[index].m_DeviceMemoryProperties;
	for (u32 i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if (memoryProperties.memoryTypes[i].propertyFlags & VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		{
			gpuInfo.m_DeviceMemory += memoryProperties.memoryHeaps[memoryProperties.memoryTypes[i].heapIndex].size;
		}
	}

	gpuInfo.m_GraphicsQueueCount = m_GPUs[index].m_QueueFamilyProperties[m_GPUs[index].m_GraphicsQueueIndex].queueCount;
	gpuInfo.m_ComputeQueueCount = m_GPUs[index].m_QueueFamilyProperties[m_GPUs[index].m_ComputeQueueIndex].queueCount;
	gpuInfo.m_TransferQueueCount = m_GPUs[index].m_QueueFamilyProperties[m_GPUs[index].m_TransferQueueIndex].queueCount;
}


BvRenderDevice * const BvRenderEngineVk::CreateRenderDevice(const u32 gpuIndex, const DeviceCreateDesc & deviceDesc)
{
	BvAssert(m_Devices[gpuIndex] == nullptr);
	if (m_Devices[gpuIndex] != nullptr)
	{
		return GetRenderDevice(gpuIndex);
	}

	auto pDevice = new BvRenderDeviceVk(m_Instance, m_Loader, m_GPUs[gpuIndex]);
	pDevice->Create(deviceDesc);
	m_Devices[gpuIndex] = pDevice;

	return pDevice;
}


bool BvRenderEngineVk::CreateInstance()
{
	constexpr const char * const pAppName = "BDeV";
	VkApplicationInfo applicationInfo{};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = pAppName;
	applicationInfo.pEngineName = pAppName;
	applicationInfo.apiVersion = VK_API_VERSION_1_2;

	{
		uint32_t extensionPropertyCount;
		auto result = VulkanFunctions::vkEnumerateInstanceExtensionProperties(nullptr, &extensionPropertyCount, nullptr);
		m_ExtensionProperties.Resize(extensionPropertyCount);
		result = VulkanFunctions::vkEnumerateInstanceExtensionProperties(nullptr, &extensionPropertyCount, m_ExtensionProperties.Data());
	}


	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;

	instanceCreateInfo.ppEnabledExtensionNames = g_EnabledExtensionProperties;
	instanceCreateInfo.enabledExtensionCount = BvArraySize(g_EnabledExtensionProperties);

#if defined(BV_DEBUG)
	instanceCreateInfo.ppEnabledLayerNames = g_ValidationLayers;
	instanceCreateInfo.enabledLayerCount = BvArraySize(g_ValidationLayers);
#endif // BV_DEBUG

	auto result = VulkanFunctions::vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		return false;
	}

	return true;
}


bool BvRenderEngineVk::EnumerateGPUs()
{
	// ===========================================================
	// Get a list of the GPUs with support for Vulkan
	u32 physicalDeviceCount = 0;
	VkResult result = VulkanFunctions::vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, nullptr);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		return false;
	}

	m_GPUs.Resize(physicalDeviceCount);

	BvVector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	result = VulkanFunctions::vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, physicalDevices.Data());
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		return false;
	}

	constexpr size_t kInvalidDeviceIndex = static_cast<size_t>(-1);

	for (size_t i = 0; i < physicalDevices.Size(); ++i)
	{
		// =================================
		// Store the physical device
		m_GPUs[i].m_PhysicalDevice = physicalDevices[i];

		// =================================
		// Store properties / features / memory properties
		m_GPUs[i].m_DeviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		m_GPUs[i].m_DeviceProperties.pNext = &m_GPUs[i].m_DeviceProperties11;
		m_GPUs[i].m_DeviceProperties11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
		m_GPUs[i].m_DeviceProperties11.pNext = &m_GPUs[i].m_DeviceProperties12;
		m_GPUs[i].m_DeviceProperties12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
		m_GPUs[i].m_DeviceProperties12.pNext = &m_GPUs[i].m_Extensions.convervativeRasterizationProps;

		// Extensions
		m_GPUs[i].m_Extensions.convervativeRasterizationProps.sType =
			VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT;
		VulkanFunctions::vkGetPhysicalDeviceProperties2(m_GPUs[i].m_PhysicalDevice, &m_GPUs[i].m_DeviceProperties);

		m_GPUs[i].m_DeviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		m_GPUs[i].m_DeviceFeatures.pNext = &m_GPUs[i].m_DeviceFeatures11;
		m_GPUs[i].m_DeviceFeatures11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
		m_GPUs[i].m_DeviceFeatures11.pNext = &m_GPUs[i].m_DeviceFeatures12;
		m_GPUs[i].m_DeviceFeatures12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		m_GPUs[i].m_DeviceFeatures12.pNext = nullptr;
		VulkanFunctions::vkGetPhysicalDeviceFeatures2(m_GPUs[i].m_PhysicalDevice, &m_GPUs[i].m_DeviceFeatures);

		VulkanFunctions::vkGetPhysicalDeviceMemoryProperties(m_GPUs[i].m_PhysicalDevice, &m_GPUs[i].m_DeviceMemoryProperties);

		// =================================
		// Get / Store supported extensions
		u32 extensionCount = 0;
		result = VulkanFunctions::vkEnumerateDeviceExtensionProperties(m_GPUs[i].m_PhysicalDevice, nullptr, &extensionCount, nullptr);
		if (result != VK_SUCCESS)
		{
			BvDebugVkResult(result);
			return false;
		}

		BvVector<VkExtensionProperties> extensionProperties(extensionCount);
		result = VulkanFunctions::vkEnumerateDeviceExtensionProperties(m_GPUs[i].m_PhysicalDevice, nullptr, &extensionCount, extensionProperties.Data());
		if (result != VK_SUCCESS)
		{
			BvDebugVkResult(result);
			return false;
		}

		for (auto extension : extensionProperties)
		{
			m_GPUs[i].m_SupportedExtensions.EmplaceBack(extension.extensionName);
		}

		// =================================
		// Get the queue families
		u32 queueFamilyCount = 0;
		VulkanFunctions::vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, nullptr);

		m_GPUs[i].m_QueueFamilyProperties.Resize(queueFamilyCount);
		VulkanFunctions::vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, m_GPUs[i].m_QueueFamilyProperties.Data());

		m_GPUs[i].m_GraphicsQueueIndex = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT, i);
		m_GPUs[i].m_ComputeQueueIndex = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT, i);
		m_GPUs[i].m_TransferQueueIndex = GetQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT, i);

		for (auto j = 0U; j < m_GPUs[i].m_QueueFamilyProperties.Size(); j++)
		{
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
			if (VulkanFunctions::vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevices[i], j))
			{
				m_GPUs[i].m_PresentationQueueIndex = j;
				if (j == m_GPUs[i].m_GraphicsQueueIndex)
				{
					break;
				}
			}
#endif
		}
	}

	return true;
}


u32 BvRenderEngineVk::GetQueueFamilyIndex(const VkQueueFlags queueFlags, const u32 gpuIndex) const
{
	auto & queueFamilyProperties = m_GPUs[gpuIndex].m_QueueFamilyProperties;

	u32 index = UINT32_MAX;

	// Dedicated queue for compute
	// Try to find a queue family index that supports compute but not graphics
	if (queueFlags & VK_QUEUE_COMPUTE_BIT)
	{
		for (u32 i = 0; i < static_cast<u32>(queueFamilyProperties.Size()); i++)
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
		for (u32 i = 0; i < static_cast<u32>(queueFamilyProperties.Size()); i++)
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
		for (u32 i = 0; i < static_cast<u32>(queueFamilyProperties.Size()); i++)
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


BvRenderEngine* CreateRenderEngineVk()
{
	return new BvRenderEngineVk();
}

void DestroyRenderEngineVk(BvRenderEngine* pRenderEngine)
{
	auto pPtr = static_cast<BvRenderEngineVk*>(pRenderEngine);
	delete pPtr;
}