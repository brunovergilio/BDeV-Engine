#include "BvRenderEngineVk.h"
#include "BvUtilsVk.h"
#include "BvRenderDeviceVk.h"
#include "BvDebugReportVk.h"


bool IsInstanceExtensionSupported(const BvVector<VkExtensionProperties>& extensions, const char* pExtension);
bool IsInstanceLayerSupported(const BvVector<VkLayerProperties>& layers, const char* pLayer);
bool IsDeviceSuitable(VkPhysicalDevice gpu);
void SetupGPUInfo(VkPhysicalDevice gpu, BvGPUInfo& gpuInfo);


bool BvRenderEngineVk::CreateRenderDevice(const BvRenderDeviceCreateDesc& deviceCreateDesc, BvRenderDevice** ppObj)
{
	BvRenderDeviceCreateDescVk descVk;
	memcpy(&descVk, &deviceCreateDesc, sizeof(BvRenderDeviceCreateDesc));
	BvRenderDeviceVk* pObjVk;
	if (CreateRenderDeviceVk(descVk, &pObjVk))
	{
		*ppObj = pObjVk;
		return true;
	}

	return false;
}


bool BvRenderEngineVk::CreateRenderDeviceVk(const BvRenderDeviceCreateDescVk& deviceDesc, BvRenderDeviceVk** ppObj)
{
	u32 gpuIndex = deviceDesc.m_GPUIndex;
	if (gpuIndex >= m_GPUs.Size())
	{
		gpuIndex = 0;
		for (auto i = 0; i < m_GPUs.Size(); ++i)
		{
			if (m_GPUs[i].m_Type == GPUType::kDiscrete)
			{
				gpuIndex = i;
				break;
			}
		}
	}

	auto& pDevice = m_Devices[gpuIndex];
	BV_ASSERT_ONCE(pDevice == nullptr, "Render Device has already been created");
	if (!pDevice)
	{
		pDevice = BV_OBJECT_CREATE(BvRenderDeviceVk, this, (VkPhysicalDevice)m_GPUs[gpuIndex].m_pPhysicalDevice, deviceDesc);
		if (!pDevice->IsValid())
		{
			pDevice->Release();
			pDevice = nullptr;
			return false;
		}
	}

	*ppObj = pDevice;

	return true;
}


BvRenderEngineVk* BvRenderEngineVk::GetInstance()
{
	static BvRenderEngineVk engine;
	if (engine.GetGPUs().Size() == 0)
	{
		return nullptr;
	}

	return &engine;
}


BvRenderEngineVk::BvRenderEngineVk()
	: m_VulkanLib(BV_VULKAN_DLL_NAME)
{
	Create();
}


BvRenderEngineVk::~BvRenderEngineVk()
{
	Destroy();
}


void BvRenderEngineVk::Create()
{
	if (!m_VulkanLib)
	{
		return;
	}

	PFN_vkGetInstanceProcAddr pProcAddressFn = nullptr;
	pProcAddressFn = m_VulkanLib.GetProcAddressT<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
	if (!pProcAddressFn)
	{
		return;
	}

	volkInitializeCustom(pProcAddressFn);
	constexpr u32 apiVersion = VK_API_VERSION_1_3;
	if (volkGetInstanceVersion() < apiVersion)
	{
		return;
	}

	constexpr const char* const pAppName = "BDeV Application";
	constexpr const char* const pEngineName = "BDeV Engine";
	VkApplicationInfo applicationInfo{};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = pAppName;
	applicationInfo.pEngineName = pEngineName;
	applicationInfo.apiVersion = apiVersion;

	VkResult result = VK_SUCCESS;

	uint32_t extensionPropertyCount{};
	result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionPropertyCount, nullptr);
	BvVector<VkExtensionProperties> supportedExtensions(extensionPropertyCount);
	result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionPropertyCount, supportedExtensions.Data());

	uint32_t layerPropertyCount{};
	result = vkEnumerateInstanceLayerProperties(&layerPropertyCount, nullptr);
	BvVector<VkLayerProperties> supportedLayers(layerPropertyCount);
	result = vkEnumerateInstanceLayerProperties(&layerPropertyCount, supportedLayers.Data());

	struct BvExtensionLayerVk
	{
		const char* pName;
		bool required;
	};

	constexpr const BvExtensionLayerVk kExtensions[] =
	{
		{ VK_KHR_SURFACE_EXTENSION_NAME, true },
	#if (BV_PLATFORM == BV_PLATFORM_WIN32)
		{ VK_KHR_WIN32_SURFACE_EXTENSION_NAME, true },
	#endif
		{ VK_EXT_DEBUG_UTILS_EXTENSION_NAME, false },
	};

	constexpr const BvExtensionLayerVk kLayers[] =
	{
		{ "VK_LAYER_KHRONOS_validation", false },
	};

	BvVector<const char*> enabledExtensions;
	BvVector<const char*> enabledLayers;
	for (auto extension : kExtensions)
	{
		if (IsInstanceExtensionSupported(supportedExtensions, extension.pName))
		{
			enabledExtensions.PushBack(extension.pName);
		}
		else if (extension.required)
		{
			return;
		}
	}

	for (auto layer : kLayers)
	{
		if (IsInstanceLayerSupported(supportedLayers, layer.pName))
		{
			enabledLayers.PushBack(layer.pName);
		}
		else if (layer.required)
		{
			return;
		}
	}

	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	if (enabledExtensions.Size() > 0)
	{
		instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.Data();
		instanceCreateInfo.enabledExtensionCount = (u32)enabledExtensions.Size();
	}
	if (enabledLayers.Size() > 0)
	{
		instanceCreateInfo.ppEnabledLayerNames = enabledLayers.Data();
		instanceCreateInfo.enabledLayerCount = (u32)enabledLayers.Size();
	}

	result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance);
	if (result != VK_SUCCESS)
	{
		return;
	}

	volkLoadInstance(m_Instance);

	// ===========================================================
	// Get a list of the GPUs with support for Vulkan
	u32 physicalDeviceCount = 0;
	result = vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, nullptr);
	if (result != VK_SUCCESS || physicalDeviceCount == 0)
	{
		Destroy();
		return;
	}

	BvVector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	result = vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, physicalDevices.Data());
	if (result != VK_SUCCESS)
	{
		Destroy();
		return;
	}

	for (auto it = physicalDevices.begin(); it != physicalDevices.end(); ++it)
	{
		if (!IsDeviceSuitable(*it))
		{
			it = physicalDevices.Erase(it);
		}
	}

	if (physicalDevices.Size() == 0)
	{
		Destroy();
		return;
	}

	m_GPUs.Resize(physicalDevices.Size(), {});
	m_Devices.Resize(physicalDevices.Size());
	for (auto i = 0; i < m_GPUs.Size(); ++i)
	{
		SetupGPUInfo(physicalDevices[i], m_GPUs[i]);
	}

	if (IsInstanceExtensionSupported(supportedExtensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
	{
		m_HasDebugUtils = true;

#if BV_DEBUG
		m_pDebugReport = BV_NEW(BvDebugReportVk)(m_Instance);
#endif
	}
}


void BvRenderEngineVk::Destroy()
{
	if (m_pDebugReport)
	{
		BV_DELETE(m_pDebugReport);
	}

	if (m_Instance)
	{
		vkDestroyInstance(m_Instance, nullptr);
		m_Instance = VK_NULL_HANDLE;
	}

	m_VulkanLib.Close();
}


bool IsInstanceExtensionSupported(const BvVector<VkExtensionProperties>& extensions, const char* pExtension)
{
	for (auto& extension : extensions)
	{
		if (!strcmp(pExtension, extension.extensionName))
		{
			return true;
		}
	}

	return false;
}


bool IsInstanceLayerSupported(const BvVector<VkLayerProperties>& layers, const char* pLayer)
{
	for (auto& layer : layers)
	{
		if (!strcmp(pLayer, layer.layerName))
		{
			return true;
		}
	}

	return false;
}


bool IsDeviceSuitable(VkPhysicalDevice gpu)
{
	VkPhysicalDeviceVulkan13Features deviceFeatures1_3{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
	VkPhysicalDeviceVulkan12Features deviceFeatures1_2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, &deviceFeatures1_3 };
	VkPhysicalDeviceVulkan11Features deviceFeatures1_1{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES, &deviceFeatures1_2 };
	VkPhysicalDeviceFeatures2 deviceFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &deviceFeatures1_1 };

	vkGetPhysicalDeviceFeatures2(gpu, &deviceFeatures);

	// We need support for these features or the code won't work
	return deviceFeatures1_2.timelineSemaphore && deviceFeatures1_3.dynamicRendering;
}


void SetupGPUInfo(VkPhysicalDevice gpu, BvGPUInfo& gpuInfo)
{
	VkPhysicalDeviceProperties2 deviceProperties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
	VkPhysicalDeviceMemoryProperties2 deviceMemoryProperties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2 };
	vkGetPhysicalDeviceProperties2(gpu, &deviceProperties);
	vkGetPhysicalDeviceMemoryProperties2(gpu, &deviceMemoryProperties);

	strcpy(gpuInfo.m_DeviceName, deviceProperties.properties.deviceName);
	gpuInfo.m_DeviceId = deviceProperties.properties.deviceID;
	gpuInfo.m_VendorId = deviceProperties.properties.vendorID;

	for (u32 i = 0; i < deviceMemoryProperties.memoryProperties.memoryTypeCount; i++)
	{
		if (deviceMemoryProperties.memoryProperties.memoryTypes[i].propertyFlags & VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		{
			gpuInfo.m_DeviceMemory += deviceMemoryProperties.memoryProperties.memoryHeaps[deviceMemoryProperties.memoryProperties.memoryTypes[i].heapIndex].size;
		}
	}

	u32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties2(gpu, &queueFamilyCount, nullptr);

	BvVector<VkQueueFamilyProperties2> queueFamilyProperties(queueFamilyCount, VkQueueFamilyProperties2{ VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2 });
	vkGetPhysicalDeviceQueueFamilyProperties2(gpu, &queueFamilyCount, queueFamilyProperties.Data());

	for (auto i = 0; i < queueFamilyProperties.Size(); ++i)
	{
		if (queueFamilyProperties[i].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			gpuInfo.m_GraphicsContextCount += queueFamilyProperties[i].queueFamilyProperties.queueCount;
		}
		else if (queueFamilyProperties[i].queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			gpuInfo.m_ComputeContextCount += queueFamilyProperties[i].queueFamilyProperties.queueCount;
		}
		else if (queueFamilyProperties[i].queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			// TODO: Some transfer queues may support video encoding and decoding, so will need
			// to add support for those in the future
			gpuInfo.m_TransferContextCount += queueFamilyProperties[i].queueFamilyProperties.queueCount;
		}
	}

	switch (gpuInfo.m_VendorId)
	{
	case 0x1002:
	case 0x1022:
		gpuInfo.m_Vendor = GPUVendor::kAMD; break;
	case 0x1010:
		gpuInfo.m_Vendor = GPUVendor::kImgTec; break;
	case 0x10DE:
		gpuInfo.m_Vendor = GPUVendor::kNvidia; break;
	case 0x13B5:
		gpuInfo.m_Vendor = GPUVendor::kARM; break;
	case 0x5143:
		gpuInfo.m_Vendor = GPUVendor::kQualcomm; break;
	case 0x163C:
	case 0x8086:
	case 0x8087:
		gpuInfo.m_Vendor = GPUVendor::kIntel; break;
	}

	switch (deviceProperties.properties.deviceType)
	{
	case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		gpuInfo.m_Type = GPUType::kDiscrete;
		break;
	case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		gpuInfo.m_Type = GPUType::kIntegrated;
		break;
	default:
		gpuInfo.m_Type = GPUType::kUnknown;
		break;
	}

	gpuInfo.m_pPhysicalDevice = gpu;
};


namespace BvRenderVk
{
	extern "C"
	{
		BV_API bool CreateRenderEngine(BvRenderEngine** ppObj)
		{
			static bool initialized = false;
			static BvRenderEngineVk* pEngine = nullptr;
			if (initialized)
			{
				*ppObj = pEngine;
			}
			else
			{
				initialized = true;
				pEngine = BV_NEW(BvRenderEngineVk)();
				if (pEngine->GetGPUs().Size() == 0)
				{
					BV_DELETE(pEngine);
					pEngine = nullptr;
				}
			}

			return pEngine != nullptr;
		}
	}
}