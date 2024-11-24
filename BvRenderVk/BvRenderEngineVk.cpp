#include "BvRenderEngineVk.h"
#include "BvUtilsVk.h"
#include "BvRenderDeviceVk.h"
#include "BvDebugReportVk.h"


bool IsInstanceExtensionSupported(const BvVector<VkExtensionProperties>& extensions, const char* const pExtension)
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


bool IsInstanceLayerSupported(const BvVector<VkLayerProperties>& layers, const char* const pLayer)
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


void BvRenderEngineVk::GetGPUInfo(u32 index, BvGPUInfo& gpuInfo) const
{
	BV_ASSERT(index < m_GPUs.Size(), "Invalid GPU index");

	gpuInfo.m_DeviceName = m_GPUs[index].m_DeviceProperties.properties.deviceName;
	gpuInfo.m_DeviceId = m_GPUs[index].m_DeviceProperties.properties.deviceID;
	gpuInfo.m_VendorId = m_GPUs[index].m_DeviceProperties.properties.vendorID;

	switch (gpuInfo.m_VendorId)
	{
	case 0x1002:
	case 0x1022:
		gpuInfo.m_Vendor = GPUVendorId::kAMD; break;
	case 0x1010:
		gpuInfo.m_Vendor = GPUVendorId::kImgTec; break;
	case 0x10DE:
		gpuInfo.m_Vendor = GPUVendorId::kNvidia; break;
	case 0x13B5:
		gpuInfo.m_Vendor = GPUVendorId::kARM; break;
	case 0x5143:
		gpuInfo.m_Vendor = GPUVendorId::kQualcomm; break;
	case 0x163C:
	case 0x8086:
	case 0x8087:
		gpuInfo.m_Vendor = GPUVendorId::kIntel; break;
	}
	
	const auto & memoryProperties = m_GPUs[index].m_DeviceMemoryProperties;
	for (u32 i = 0; i < memoryProperties.memoryProperties.memoryTypeCount; i++)
	{
		if (memoryProperties.memoryProperties.memoryTypes[i].propertyFlags & VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		{
			gpuInfo.m_DeviceMemory += memoryProperties.memoryProperties.memoryHeaps[memoryProperties.memoryProperties.memoryTypes[i].heapIndex].size;
		}
	}
}


BvRenderDevice* BvRenderEngineVk::CreateRenderDevice(const BvRenderDeviceCreateDesc* pDeviceCreateDesc)
{
	auto pDeviceCreateDescVk = static_cast<const BvRenderDeviceCreateDescVk*>(pDeviceCreateDesc);
	return CreateRenderDeviceVk(pDeviceCreateDescVk ? *pDeviceCreateDescVk : BvRenderDeviceCreateDescVk());
}


BvRenderDeviceVk* BvRenderEngineVk::CreateRenderDeviceVk(const BvRenderDeviceCreateDescVk& deviceDesc)
{
	BV_ASSERT(m_pDevice == nullptr, "Render device already created");
	if (m_pDevice != nullptr)
	{
		return m_pDevice;
	}

	u32 gpuIndex = deviceDesc.m_GPUIndex;
	if (gpuIndex >= m_GPUs.Size())
	{
		gpuIndex = AutoSelectGPU();
	}

	m_pDevice = BV_NEW(BvRenderDeviceVk)(this, m_GPUs[gpuIndex], deviceDesc);

	return m_pDevice;
}


BvRenderEngineVk* BvRenderEngineVk::GetInstance()
{
	static BvRenderEngineVk engine;
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
		BvDebugVkResult(result);
		return;
	}

	volkLoadInstance(m_Instance);

	if (IsInstanceExtensionSupported(supportedExtensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
	{
		m_pDebugReport = BV_NEW(BvDebugReportVk)(m_Instance);
	}

	// ===========================================================
	// Get a list of the GPUs with support for Vulkan
	u32 physicalDeviceCount = 0;
	result = vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, nullptr);
	if (result != VK_SUCCESS || physicalDeviceCount == 0)
	{
		BvDebugVkResult(result);
		Destroy();
		return;
	}

	BvVector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	result = vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, physicalDevices.Data());
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		Destroy();
		return;
	}

	BvVector<BvGPUInfoVk> gpus;
	for (size_t i = 0; i < physicalDevices.Size(); ++i)
	{
		auto& gpu = gpus.EmplaceBack();

		// =================================
		// Store the physical device
		gpu.m_PhysicalDevice = physicalDevices[i];

		if (!SetupDeviceExtraPropertiesAndFeatures(gpu))
		{
			gpus.PopBack();
			continue;
		}

		SetupQueueInfo(gpu);
	}

	if (gpus.Size() > 0)
	{
		m_GPUs = std::move(gpus);
	}
	else
	{
		Destroy();
	}
}


void BvRenderEngineVk::Destroy()
{
	if (m_pDevice)
	{
		BV_DELETE(m_pDevice);
	}

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


bool BvRenderEngineVk::IsPhysicalDeviceExtensionSupported(const BvGPUInfoVk& gpu, const char* pPhysicalDeviceExtension)
{
	for (auto& physicalDeviceExtension : gpu.m_SupportedExtensions)
	{
		if (!strcmp(pPhysicalDeviceExtension, physicalDeviceExtension.extensionName))
		{
			return true;
		}
	}

	return false;
}


bool BvRenderEngineVk::SetupDeviceExtraPropertiesAndFeatures(BvGPUInfoVk& gpu)
{
	// =================================
	// Get / Store supported extensions
	u32 extensionCount = 0;
	auto result = vkEnumerateDeviceExtensionProperties(gpu.m_PhysicalDevice, nullptr, &extensionCount, nullptr);
	if (result != VK_SUCCESS)
	{
		return false;
	}

	gpu.m_SupportedExtensions.Resize(extensionCount);
	result = vkEnumerateDeviceExtensionProperties(gpu.m_PhysicalDevice, nullptr, &extensionCount, gpu.m_SupportedExtensions.Data());
	if (result != VK_SUCCESS)
	{
		return false;
	}

	gpu.m_FeaturesSupported.swapChain = IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_SWAPCHAIN_EXTENSION_NAME); // Also needs VK_KHR_SURFACE_EXTENSION_NAME
	if (!gpu.m_FeaturesSupported.swapChain)
	{
		return false;
	}

	gpu.m_DeviceProperties.pNext = &gpu.m_DeviceProperties1_1;
	gpu.m_DeviceProperties1_1.pNext = &gpu.m_DeviceProperties1_2;
	gpu.m_DeviceProperties1_2.pNext = &gpu.m_DeviceProperties1_3;

	gpu.m_DeviceFeatures.pNext = &gpu.m_DeviceFeatures1_1;
	gpu.m_DeviceFeatures1_1.pNext = &gpu.m_DeviceFeatures1_2;
	gpu.m_DeviceFeatures1_2.pNext = &gpu.m_DeviceFeatures1_3;

	void** pNextProperty = &gpu.m_DeviceProperties1_3.pNext;
	void** pNextFeature = &gpu.m_DeviceFeatures1_3.pNext;
	void** pNextMemoryProperty = &gpu.m_DeviceMemoryProperties.pNext;

	gpu.m_FeaturesSupported.vertexAttributeDivisor = IsPhysicalDeviceExtensionSupported(gpu, VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
	gpu.m_FeaturesSupported.fragmentShading = IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
	gpu.m_FeaturesSupported.meshShader = IsPhysicalDeviceExtensionSupported(gpu, VK_EXT_MESH_SHADER_EXTENSION_NAME);
	gpu.m_FeaturesSupported.accelerationStructure = IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
	gpu.m_FeaturesSupported.rayTracingPipeline = IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
	gpu.m_FeaturesSupported.rayQuery = IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_RAY_QUERY_EXTENSION_NAME);
	gpu.m_FeaturesSupported.conservativeRasterization = IsPhysicalDeviceExtensionSupported(gpu, VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
	gpu.m_FeaturesSupported.customBorderColor = IsPhysicalDeviceExtensionSupported(gpu, VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
	gpu.m_FeaturesSupported.memoryBudget = IsPhysicalDeviceExtensionSupported(gpu, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
	gpu.m_FeaturesSupported.deferredHostOperations = IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
	gpu.m_FeaturesSupported.predication = IsPhysicalDeviceExtensionSupported(gpu, VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
	gpu.m_FeaturesSupported.depthClipEnable = IsPhysicalDeviceExtensionSupported(gpu, VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME);

	if (gpu.m_FeaturesSupported.vertexAttributeDivisor)
	{
		*pNextProperty = &gpu.m_ExtendedProperties.vertexAttributeDivisorProperties;
		pNextProperty = &gpu.m_ExtendedProperties.vertexAttributeDivisorProperties.pNext;

		*pNextFeature = &gpu.m_ExtendedFeatures.vertexAttributeDivisorFeatures;
		pNextFeature = &gpu.m_ExtendedFeatures.vertexAttributeDivisorFeatures.pNext;

		gpu.m_EnabledExtensions.PushBack(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
	}

	if (gpu.m_FeaturesSupported.fragmentShading)
	{
		*pNextProperty = &gpu.m_ExtendedProperties.fragmentShadingRateProps;
		pNextProperty = &gpu.m_ExtendedProperties.fragmentShadingRateProps.pNext;

		*pNextFeature = &gpu.m_ExtendedFeatures.fragmentShadingRateFeatures;
		pNextFeature = &gpu.m_ExtendedFeatures.fragmentShadingRateFeatures.pNext;

		gpu.m_EnabledExtensions.PushBack(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
	}

	if (gpu.m_FeaturesSupported.meshShader)
	{
		*pNextProperty = &gpu.m_ExtendedProperties.meshShaderProps;
		pNextProperty = &gpu.m_ExtendedProperties.meshShaderProps.pNext;

		*pNextFeature = &gpu.m_ExtendedFeatures.meshShaderFeatures;
		pNextFeature = &gpu.m_ExtendedFeatures.meshShaderFeatures.pNext;

		gpu.m_EnabledExtensions.PushBack(VK_EXT_MESH_SHADER_EXTENSION_NAME);
	}

	if (gpu.m_FeaturesSupported.accelerationStructure && gpu.m_FeaturesSupported.deferredHostOperations)
	{
		*pNextProperty = &gpu.m_ExtendedProperties.accelerationStructureProps;
		pNextProperty = &gpu.m_ExtendedProperties.accelerationStructureProps.pNext;

		*pNextFeature = &gpu.m_ExtendedFeatures.accelerationStructureFeatures;
		pNextFeature = &gpu.m_ExtendedFeatures.accelerationStructureFeatures.pNext;

		gpu.m_EnabledExtensions.PushBack(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

		if (gpu.m_FeaturesSupported.rayTracingPipeline)
		{
			*pNextProperty = &gpu.m_ExtendedProperties.rayTracingPipelineProps;
			pNextProperty = &gpu.m_ExtendedProperties.rayTracingPipelineProps.pNext;

			*pNextFeature = &gpu.m_ExtendedFeatures.rayTracingPipelineFeatures;
			pNextFeature = &gpu.m_ExtendedFeatures.rayTracingPipelineFeatures.pNext;

			gpu.m_EnabledExtensions.PushBack(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
		}

		if (gpu.m_FeaturesSupported.rayQuery)
		{
			*pNextFeature = &gpu.m_ExtendedFeatures.rayQueryFeatures;
			pNextFeature = &gpu.m_ExtendedFeatures.rayQueryFeatures.pNext;

			gpu.m_EnabledExtensions.PushBack(VK_KHR_RAY_QUERY_EXTENSION_NAME);
		}
	}

	if (gpu.m_FeaturesSupported.conservativeRasterization)
	{
		*pNextProperty = &gpu.m_ExtendedProperties.convervativeRasterizationProps;
		pNextProperty = &gpu.m_ExtendedProperties.convervativeRasterizationProps.pNext;

		gpu.m_EnabledExtensions.PushBack(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
	}

	if (gpu.m_FeaturesSupported.customBorderColor)
	{
		*pNextProperty = &gpu.m_ExtendedProperties.customBorderColorProps;
		pNextProperty = &gpu.m_ExtendedProperties.customBorderColorProps.pNext;

		*pNextFeature = &gpu.m_ExtendedFeatures.customBorderColorFeatures;
		pNextFeature = &gpu.m_ExtendedFeatures.customBorderColorFeatures.pNext;

		gpu.m_EnabledExtensions.PushBack(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
	}

	if (gpu.m_FeaturesSupported.memoryBudget)
	{
		*pNextMemoryProperty = &gpu.m_ExtendedProperties.memoryBudgetProperties;
		pNextMemoryProperty = &gpu.m_ExtendedProperties.memoryBudgetProperties.pNext;

		gpu.m_EnabledExtensions.PushBack(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
	}

	if (gpu.m_FeaturesSupported.deferredHostOperations)
	{
		gpu.m_EnabledExtensions.PushBack(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
	}

	if (gpu.m_FeaturesSupported.predication)
	{
		*pNextFeature = &gpu.m_ExtendedFeatures.conditionalRenderingFeatures;
		pNextFeature = &gpu.m_ExtendedFeatures.conditionalRenderingFeatures.pNext;

		gpu.m_EnabledExtensions.PushBack(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
	}

	if (gpu.m_FeaturesSupported.depthClipEnable)
	{
		*pNextFeature = &gpu.m_ExtendedFeatures.depthClibEnableFeature;
		pNextFeature = &gpu.m_ExtendedFeatures.depthClibEnableFeature.pNext;

		gpu.m_EnabledExtensions.PushBack(VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME);
	}

	gpu.m_EnabledExtensions.PushBack(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	// =================================
	// Store properties / features / memory properties
	vkGetPhysicalDeviceFeatures2(gpu.m_PhysicalDevice, &gpu.m_DeviceFeatures);
	vkGetPhysicalDeviceProperties2(gpu.m_PhysicalDevice, &gpu.m_DeviceProperties);
	vkGetPhysicalDeviceMemoryProperties2(gpu.m_PhysicalDevice, &gpu.m_DeviceMemoryProperties);

	// We need support for these features or the code won't work
	if (!(gpu.m_DeviceFeatures1_2.timelineSemaphore && gpu.m_DeviceFeatures1_3.dynamicRendering))
	{
		return false;
	}

	return true;
}


void BvRenderEngineVk::SetupQueueInfo(BvGPUInfoVk& gpu)
{
	// =================================
	// Get the queue families
	u32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties2(gpu.m_PhysicalDevice, &queueFamilyCount, nullptr);

	gpu.m_QueueFamilyProperties.Resize(queueFamilyCount, VkQueueFamilyProperties2{ VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2 });
	vkGetPhysicalDeviceQueueFamilyProperties2(gpu.m_PhysicalDevice, &queueFamilyCount, gpu.m_QueueFamilyProperties.Data());

	bool isAsync;
	u32 queueIndex;
	if ((queueIndex = GetQueueFamilyIndex(gpu, VK_QUEUE_GRAPHICS_BIT, isAsync)) != kU32Max)
	{
		gpu.m_GraphicsQueueInfo.m_QueueFamilyIndex = queueIndex;
		gpu.m_GraphicsQueueInfo.m_QueueCount = gpu.m_QueueFamilyProperties[queueIndex].queueFamilyProperties.queueCount;
		gpu.m_GraphicsQueueInfo.m_SupportsPresent = QueueSupportsPresent(gpu, queueIndex);
	}
	if ((queueIndex = GetQueueFamilyIndex(gpu, VK_QUEUE_COMPUTE_BIT, isAsync)) != kU32Max && isAsync)
	{
		gpu.m_ComputeQueueInfo.m_QueueFamilyIndex = queueIndex;
		gpu.m_ComputeQueueInfo.m_QueueCount = gpu.m_QueueFamilyProperties[queueIndex].queueFamilyProperties.queueCount;
		gpu.m_ComputeQueueInfo.m_SupportsPresent = QueueSupportsPresent(gpu, queueIndex);
	}
	if ((queueIndex = GetQueueFamilyIndex(gpu, VK_QUEUE_TRANSFER_BIT, isAsync)) != kU32Max && isAsync)
	{
		gpu.m_TransferQueueInfo.m_QueueFamilyIndex = queueIndex;
		gpu.m_TransferQueueInfo.m_QueueCount = gpu.m_QueueFamilyProperties[queueIndex].queueFamilyProperties.queueCount;
		gpu.m_TransferQueueInfo.m_SupportsPresent = QueueSupportsPresent(gpu, queueIndex);
	}
}


u32 BvRenderEngineVk::GetQueueFamilyIndex(BvGPUInfoVk& gpu, VkQueueFlags queueFlags, bool& isAsync) const
{
	auto& queueFamilyProperties = gpu.m_QueueFamilyProperties;

	u32 index = kU32Max;

	isAsync = false;

	// Dedicated queue for compute
	// Try to find a queue family index that supports compute but not graphics
	if (queueFlags == VK_QUEUE_COMPUTE_BIT)
	{
		for (u32 i = 0; i < static_cast<u32>(queueFamilyProperties.Size()); i++)
		{
			if ((queueFamilyProperties[i].queueFamilyProperties.queueFlags & queueFlags)
				&& ((queueFamilyProperties[i].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
			{
				index = i;
				isAsync = true;
				break;
			}
		}
	}

	// Dedicated queue for transfer
	// Try to find a queue family index that supports transfer but not graphics and compute
	else if (queueFlags == VK_QUEUE_TRANSFER_BIT)
	{
		for (u32 i = 0; i < static_cast<u32>(queueFamilyProperties.Size()); i++)
		{
			if ((queueFamilyProperties[i].queueFamilyProperties.queueFlags & queueFlags)
				&& ((queueFamilyProperties[i].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
				&& ((queueFamilyProperties[i].queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
			{
				index = i;
				isAsync = true;
				break;
			}
		}
	}

	if (index == kU32Max)
	{
		// For other queue types or if no separate compute or transfer queue is present, return the first one to support the requested flags
		for (u32 i = 0; i < static_cast<u32>(queueFamilyProperties.Size()); i++)
		{
			if (queueFamilyProperties[i].queueFamilyProperties.queueFlags & queueFlags)
			{
				index = i;
				break;
			}
		}
	}

	return index;
}


bool BvRenderEngineVk::QueueSupportsPresent(BvGPUInfoVk& gpu, u32 index) const
{
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	if (vkGetPhysicalDeviceWin32PresentationSupportKHR(gpu.m_PhysicalDevice, index))
	{
		return true;
	}
#endif

	return false;
}


u32 BvRenderEngineVk::AutoSelectGPU()
{
	u32 gpuIndex = 0;
	for (; gpuIndex < m_GPUs.Size(); gpuIndex++)
	{
		if (m_GPUs[gpuIndex].m_DeviceProperties.properties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			break;
		}
	}

	return gpuIndex;
}