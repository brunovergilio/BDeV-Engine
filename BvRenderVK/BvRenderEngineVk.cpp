#include "BvRenderEngineVk.h"
#include "BvUtilsVk.h"
#include "BvRenderDeviceVk.h"
#include "BvDebugReportVk.h"


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


constexpr const char * const g_ExtensionProperties[] =
{
	VK_KHR_SURFACE_EXTENSION_NAME,
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
	VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
#if defined(BV_DEBUG)
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
};


constexpr const char* const g_LayerProperties[] =
{
	"VK_LAYER_KHRONOS_validation"
};


BvRenderEngineVk::BvRenderEngineVk()
{
	Create();
}


BvRenderEngineVk::~BvRenderEngineVk()
{
	Destroy();
}


void BvRenderEngineVk::Create()
{
	if (!CreateInstance())
	{
		Destroy();
		return;
	}

	if (!EnumerateGPUs())
	{
		Destroy();
		return;
	}

	m_Devices.Resize(m_GPUs.Size());

#if defined(BV_DEBUG)
	if (IsInstanceExtensionEnabled(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
	{
		m_pDebugReport = new BvDebugReportVk(m_Instance);
	}
#endif
}


void BvRenderEngineVk::Destroy()
{
	for (auto pDevice : m_Devices)
	{
		if (pDevice)
		{
			auto pDeviceVk = reinterpret_cast<BvRenderDeviceVk*>(pDevice);
			delete pDeviceVk;
		}
	}
	m_Devices.Clear();

#if defined(BV_DEBUG)
	if (m_pDebugReport)
	{
		delete m_pDebugReport;
		m_pDebugReport = nullptr;
	}
#endif

	if (m_Instance)
	{
		vkDestroyInstance(m_Instance, nullptr);
		m_Instance = VK_NULL_HANDLE;
	}
}


void BvRenderEngineVk::GetGPUInfo(const u32 index, BvGPUInfo & gpuInfo) const
{
	BvAssert(index < m_GPUs.Size(), "Invalid GPU index");

	gpuInfo.m_DeviceName = m_GPUs[index].m_DeviceProperties.deviceName;
	gpuInfo.m_DeviceId = m_GPUs[index].m_DeviceProperties.deviceID;
	gpuInfo.m_VendorId = m_GPUs[index].m_DeviceProperties.vendorID;

	switch (gpuInfo.m_VendorId)
	{
	case GPUVendorId::kAMD: gpuInfo.m_Vendor = GPUVendorId::kAMD; break;
	case GPUVendorId::kImgTec: gpuInfo.m_Vendor = GPUVendorId::kImgTec; break;
	case GPUVendorId::kNvidia: gpuInfo.m_Vendor = GPUVendorId::kNvidia; break;
	case GPUVendorId::kARM: gpuInfo.m_Vendor = GPUVendorId::kARM; break;
	case GPUVendorId::kQualcomm: gpuInfo.m_Vendor = GPUVendorId::kQualcomm; break;
	case GPUVendorId::kIntel: gpuInfo.m_Vendor = GPUVendorId::kIntel; break;
	}
	
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


BvRenderDevice * const BvRenderEngineVk::CreateRenderDevice(const DeviceCreateDesc& deviceDesc, u32 gpuIndex)
{
	if (gpuIndex >= m_GPUs.Size())
	{
		gpuIndex = AutoSelectGPU();
	}

	BvAssert(m_Devices[gpuIndex] == nullptr, "Render device already created");
	if (m_Devices[gpuIndex] != nullptr)
	{
		return GetRenderDevice(gpuIndex);
	}

	// We'll choose the best GPU we have
	if (gpuIndex == kAutoSelectGPU)
	{
		for (auto i = 0u; i < m_GPUs.Size(); i++)
		{
			// We want a discrete gpu if we can
			if (m_GPUs[i].m_DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				gpuIndex = i;
				break;
			}
		}
	}

	auto pDevice = new BvRenderDeviceVk(this, m_GPUs[gpuIndex], deviceDesc);
	m_Devices[gpuIndex] = pDevice;

	return pDevice;
}


bool BvRenderEngineVk::IsInstanceExtensionSupported(const char* const pExtension)
{
	for (auto& extension : m_SupportedExtensions)
	{
		if (!strcmp(pExtension, extension.extensionName))
		{
			return true;
		}
	}

	return false;
}


bool BvRenderEngineVk::IsInstanceExtensionEnabled(const char* const pExtension)
{
	for (auto pEnabledExtension : m_EnabledExtensions)
	{
		if (!strcmp(pExtension, pEnabledExtension))
		{
			return true;
		}
	}

	return false;
}


bool BvRenderEngineVk::IsInstanceLayerSupported(const char* const pLayer)
{
	for (auto& layer : m_SupportedLayers)
	{
		if (!strcmp(pLayer, layer.layerName))
		{
			return true;
		}
	}

	return false;
}


bool BvRenderEngineVk::IsPhysicalDeviceExtensionSupported(const BvGPUInfoVk& gpu, const char* const pPhysicalDeviceExtension)
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


bool BvRenderEngineVk::CreateInstance()
{
	auto result = volkInitialize();
	if (result != VK_SUCCESS)
	{
		return false;
	}

	m_InstanceVersion = volkGetInstanceVersion();
	u32 appVersion = m_InstanceVersion >= VK_API_VERSION_1_1 ? m_InstanceVersion : VK_API_VERSION_1_0;

	constexpr const char * const pAppName = "BDeV";
	VkApplicationInfo applicationInfo{};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = pAppName;
	applicationInfo.pEngineName = pAppName;
	applicationInfo.apiVersion = appVersion;

	uint32_t extensionPropertyCount{};
	result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionPropertyCount, nullptr);
	m_SupportedExtensions.Resize(extensionPropertyCount);
	result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionPropertyCount, m_SupportedExtensions.Data());

	uint32_t layerPropertyCount{};
	result = vkEnumerateInstanceLayerProperties(&layerPropertyCount, nullptr);
	m_SupportedLayers.Resize(layerPropertyCount);
	result = vkEnumerateInstanceLayerProperties(&layerPropertyCount, m_SupportedLayers.Data());

	BvVector<const char*> layers;
	for (auto pExtension : g_ExtensionProperties)
	{
		if (IsInstanceExtensionSupported(pExtension))
		{
			m_EnabledExtensions.PushBack(pExtension);
		}
	}

	for (auto pLayer : g_LayerProperties)
	{
		if (IsInstanceLayerSupported(pLayer))
		{
			layers.PushBack(pLayer);
		}
	}


	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	if (m_EnabledExtensions.Size() > 0)
	{
		instanceCreateInfo.ppEnabledExtensionNames = m_EnabledExtensions.Data();
		instanceCreateInfo.enabledExtensionCount = (u32)m_EnabledExtensions.Size();
	}
	if (layers.Size() > 0)
	{
		instanceCreateInfo.ppEnabledLayerNames = layers.Data();
		instanceCreateInfo.enabledLayerCount = (u32)layers.Size();
	}

	result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		return false;
	}

	volkLoadInstance(m_Instance);

	return true;
}


bool BvRenderEngineVk::EnumerateGPUs()
{
	// ===========================================================
	// Get a list of the GPUs with support for Vulkan
	u32 physicalDeviceCount = 0;
	VkResult result = vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, nullptr);
	if (result != VK_SUCCESS || physicalDeviceCount == 0)
	{
		BvDebugVkResult(result);
		return false;
	}

	m_GPUs.Resize(physicalDeviceCount);

	BvVector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	result = vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, physicalDevices.Data());
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
		vkGetPhysicalDeviceProperties(m_GPUs[i].m_PhysicalDevice, &m_GPUs[i].m_DeviceProperties);
		vkGetPhysicalDeviceFeatures(m_GPUs[i].m_PhysicalDevice, &m_GPUs[i].m_DeviceFeatures);
		vkGetPhysicalDeviceMemoryProperties(m_GPUs[i].m_PhysicalDevice, &m_GPUs[i].m_DeviceMemoryProperties);

		// =================================
		// Get / Store supported extensions
		u32 extensionCount = 0;
		result = vkEnumerateDeviceExtensionProperties(m_GPUs[i].m_PhysicalDevice, nullptr, &extensionCount, nullptr);
		if (result != VK_SUCCESS)
		{
			BvDebugVkResult(result);
			return false;
		}

		m_GPUs[i].m_SupportedExtensions.Resize(extensionCount);
		result = vkEnumerateDeviceExtensionProperties(m_GPUs[i].m_PhysicalDevice, nullptr, &extensionCount, m_GPUs[i].m_SupportedExtensions.Data());
		if (result != VK_SUCCESS)
		{
			BvDebugVkResult(result);
			return false;
		}

		SetupDevicePropertiesAndFeatures(i);

		// =================================
		// Get the queue families
		u32 queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, nullptr);

		m_GPUs[i].m_QueueFamilyProperties.Resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, m_GPUs[i].m_QueueFamilyProperties.Data());

		m_GPUs[i].m_GraphicsQueueIndex = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT, i);
		m_GPUs[i].m_ComputeQueueIndex = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT, i);
		m_GPUs[i].m_TransferQueueIndex = GetQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT, i);

		for (auto j = 0U; j < m_GPUs[i].m_QueueFamilyProperties.Size(); j++)
		{
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
			if (vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevices[i], j))
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


void BvRenderEngineVk::SetupDevicePropertiesAndFeatures(u32 gpuIndex)
{
	auto& gpu = m_GPUs[gpuIndex];
	gpu.m_FeaturesSupported.swapChain = IsInstanceExtensionEnabled(VK_KHR_SURFACE_EXTENSION_NAME) && IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	gpu.m_FeaturesSupported.deviceProperties2 = IsInstanceExtensionEnabled(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
	gpu.m_FeaturesSupported.memoryRequirements2 = IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
	gpu.m_FeaturesSupported.bindMemory2 = IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
	if (gpu.m_FeaturesSupported.deviceProperties2)
	{
		VkPhysicalDeviceProperties2KHR deviceProperties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
		VkPhysicalDeviceFeatures2KHR deviceFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };

		void** pNextProperty = &deviceProperties.pNext;
		void** pNextFeature = &deviceFeatures.pNext;

		gpu.m_FeaturesSupported.vertexAttributeDivisor = IsPhysicalDeviceExtensionSupported(gpu, VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
		gpu.m_FeaturesSupported.maintenance1 = IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_MAINTENANCE1_EXTENSION_NAME);
		gpu.m_FeaturesSupported.maintenance2 = IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_MAINTENANCE2_EXTENSION_NAME);
		gpu.m_FeaturesSupported.imageFormatList = IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
		gpu.m_FeaturesSupported.imagelessFrameBuffer = gpu.m_FeaturesSupported.imageFormatList && IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
		gpu.m_FeaturesSupported.multiview = IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_MULTIVIEW_EXTENSION_NAME);
		gpu.m_FeaturesSupported.renderpass2 = gpu.m_FeaturesSupported.maintenance2 &&
		gpu.m_FeaturesSupported.multiview && IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
		gpu.m_FeaturesSupported.depthStencilResolve = gpu.m_FeaturesSupported.renderpass2 && IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
		gpu.m_FeaturesSupported.dynamicRendering = gpu.m_FeaturesSupported.depthStencilResolve && IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
		gpu.m_FeaturesSupported.fragmentShading = gpu.m_FeaturesSupported.renderpass2 && IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
		gpu.m_FeaturesSupported.maintenance3 = IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_MAINTENANCE3_EXTENSION_NAME);
		gpu.m_FeaturesSupported.descriptorIndexing = gpu.m_FeaturesSupported.maintenance3 && IsPhysicalDeviceExtensionSupported(gpu, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
		gpu.m_FeaturesSupported.bufferDeviceAddress = IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
		gpu.m_FeaturesSupported.deferredHostOperations = IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
		gpu.m_FeaturesSupported.shaderFloatControls = IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
		gpu.m_FeaturesSupported.spirv1_4 = gpu.m_FeaturesSupported.shaderFloatControls && IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_SPIRV_1_4_EXTENSION_NAME);
		gpu.m_FeaturesSupported.meshShader = gpu.m_FeaturesSupported.spirv1_4 && IsPhysicalDeviceExtensionSupported(gpu, VK_EXT_MESH_SHADER_EXTENSION_NAME);
		gpu.m_FeaturesSupported.accelerationStructure = gpu.m_FeaturesSupported.descriptorIndexing && gpu.m_FeaturesSupported.bufferDeviceAddress &&
		gpu.m_FeaturesSupported.deferredHostOperations && IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
		gpu.m_FeaturesSupported.rayTracingPipeline = gpu.m_FeaturesSupported.accelerationStructure &&
		gpu.m_FeaturesSupported.spirv1_4 && IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
		gpu.m_FeaturesSupported.rayQuery = gpu.m_FeaturesSupported.accelerationStructure &&
		gpu.m_FeaturesSupported.spirv1_4 && IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_RAY_QUERY_EXTENSION_NAME);
		gpu.m_FeaturesSupported.pipelineLibrary = IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
		gpu.m_FeaturesSupported.conservativeRasterization = IsPhysicalDeviceExtensionSupported(gpu, VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
		gpu.m_FeaturesSupported.customBorderColor = IsPhysicalDeviceExtensionSupported(gpu, VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
		gpu.m_FeaturesSupported.timelineSemaphore = IsPhysicalDeviceExtensionSupported(gpu, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
		gpu.m_FeaturesSupported.memoryBudget = IsPhysicalDeviceExtensionSupported(gpu, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);

		if (gpu.m_FeaturesSupported.vertexAttributeDivisor)
		{
			*pNextProperty = &gpu.m_ExtendedProperties.vertexAttributeDivisorProperties;
			pNextProperty = &gpu.m_ExtendedProperties.vertexAttributeDivisorProperties.pNext;

			*pNextFeature = &gpu.m_ExtendedFeatures.vertexAttributeDivisorFeatures;
			pNextFeature = &gpu.m_ExtendedFeatures.vertexAttributeDivisorFeatures.pNext;
		}

		if (gpu.m_FeaturesSupported.maintenance2)
		{
			*pNextProperty = &gpu.m_ExtendedProperties.pointClippingProps;
			pNextProperty = &gpu.m_ExtendedProperties.pointClippingProps.pNext;
		}

		if (gpu.m_FeaturesSupported.multiview)
		{
			*pNextProperty = &gpu.m_ExtendedProperties.multiviewProps;
			pNextProperty = &gpu.m_ExtendedProperties.multiviewProps.pNext;

			*pNextFeature = &gpu.m_ExtendedFeatures.multiviewFeatures;
			pNextFeature = &gpu.m_ExtendedFeatures.multiviewFeatures.pNext;
		}

		if (gpu.m_FeaturesSupported.depthStencilResolve)
		{
			*pNextProperty = &gpu.m_ExtendedProperties.depthStencilResolveProps;
			pNextProperty = &gpu.m_ExtendedProperties.depthStencilResolveProps.pNext;
		}

		if (gpu.m_FeaturesSupported.dynamicRendering)
		{
			*pNextFeature = &gpu.m_ExtendedFeatures.dynamicRenderingFeatures;
			pNextFeature = &gpu.m_ExtendedFeatures.dynamicRenderingFeatures.pNext;
		}

		if (gpu.m_FeaturesSupported.fragmentShading)
		{
			*pNextProperty = &gpu.m_ExtendedProperties.fragmentShadingRateProps;
			pNextProperty = &gpu.m_ExtendedProperties.fragmentShadingRateProps.pNext;

			*pNextFeature = &gpu.m_ExtendedFeatures.fragmentShadingRateFeatures;
			pNextFeature = &gpu.m_ExtendedFeatures.fragmentShadingRateFeatures.pNext;
		}
		
		if (gpu.m_FeaturesSupported.descriptorIndexing)
		{
			*pNextProperty = &gpu.m_ExtendedProperties.descriptorIndexingProps;
			pNextProperty = &gpu.m_ExtendedProperties.descriptorIndexingProps.pNext;

			*pNextFeature = &gpu.m_ExtendedFeatures.descriptorIndexingFeatures;
			pNextFeature = &gpu.m_ExtendedFeatures.descriptorIndexingFeatures.pNext;
		}

		if (gpu.m_FeaturesSupported.bufferDeviceAddress)
		{
			*pNextFeature = &gpu.m_ExtendedFeatures.bufferDeviceAddressFeatures;
			pNextFeature = &gpu.m_ExtendedFeatures.bufferDeviceAddressFeatures.pNext;
		}

		if (gpu.m_FeaturesSupported.accelerationStructure)
		{
			*pNextProperty = &gpu.m_ExtendedProperties.accelerationStructureProps;
			pNextProperty = &gpu.m_ExtendedProperties.accelerationStructureProps.pNext;

			*pNextFeature = &gpu.m_ExtendedFeatures.accelerationStructureFeatures;
			pNextFeature = &gpu.m_ExtendedFeatures.accelerationStructureFeatures.pNext;
		}

		if (gpu.m_FeaturesSupported.rayTracingPipeline)
		{
			*pNextProperty = &gpu.m_ExtendedProperties.rayTracingPipelineProps;
			pNextProperty = &gpu.m_ExtendedProperties.rayTracingPipelineProps.pNext;

			*pNextFeature = &gpu.m_ExtendedFeatures.rayTracingPipelineFeatures;
			pNextFeature = &gpu.m_ExtendedFeatures.rayTracingPipelineFeatures.pNext;
		}

		if (gpu.m_FeaturesSupported.rayQuery)
		{
			*pNextFeature = &gpu.m_ExtendedFeatures.rayQueryFeatures;
			pNextFeature = &gpu.m_ExtendedFeatures.rayQueryFeatures.pNext;
		}

		if (gpu.m_FeaturesSupported.meshShader)
		{
			*pNextProperty = &gpu.m_ExtendedProperties.meshShaderProps;
			pNextProperty = &gpu.m_ExtendedProperties.meshShaderProps.pNext;

			*pNextFeature = &gpu.m_ExtendedFeatures.meshShaderFeatures;
			pNextFeature = &gpu.m_ExtendedFeatures.meshShaderFeatures.pNext;
		}

		if (gpu.m_FeaturesSupported.conservativeRasterization)
		{
			*pNextProperty = &gpu.m_ExtendedProperties.convervativeRasterizationProps;
			pNextProperty = &gpu.m_ExtendedProperties.convervativeRasterizationProps.pNext;
		}

		if (gpu.m_FeaturesSupported.customBorderColor)
		{
			*pNextProperty = &gpu.m_ExtendedProperties.customBorderColorProps;
			pNextProperty = &gpu.m_ExtendedProperties.customBorderColorProps.pNext;

			*pNextFeature = &gpu.m_ExtendedFeatures.customBorderColorFeatures;
			pNextFeature = &gpu.m_ExtendedFeatures.customBorderColorFeatures.pNext;
		}

		if (gpu.m_FeaturesSupported.timelineSemaphore)
		{
			*pNextProperty = &gpu.m_ExtendedProperties.timelineSemaphoreProps;
			pNextProperty = &gpu.m_ExtendedProperties.timelineSemaphoreProps.pNext;

			*pNextFeature = &gpu.m_ExtendedFeatures.timelineSemaphoreFeatures;
			pNextFeature = &gpu.m_ExtendedFeatures.timelineSemaphoreFeatures.pNext;
		}

		if (gpu.m_FeaturesSupported.imagelessFrameBuffer)
		{
			*pNextFeature = &gpu.m_ExtendedFeatures.imagelessFrameBufferFeatures;
			pNextFeature = &gpu.m_ExtendedFeatures.imagelessFrameBufferFeatures.pNext;
		}

		vkGetPhysicalDeviceProperties2KHR(m_GPUs[gpuIndex].m_PhysicalDevice, &deviceProperties);
		vkGetPhysicalDeviceFeatures2KHR(m_GPUs[gpuIndex].m_PhysicalDevice, &deviceFeatures);
	}
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


u32 BvRenderEngineVk::AutoSelectGPU()
{
	VkDeviceSize currMaxHeapSize = 0;
	u32 chosenGPUIndex = 0;
	
	for (auto i = 0u; i < m_GPUs.Size(); i++)
	{
		if (m_GPUs[i].m_DeviceProperties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			VkDeviceSize gpuSize = 0;
			for (auto j = 0; j < m_GPUs[i].m_DeviceMemoryProperties.memoryHeapCount; j++)
			{
				if (m_GPUs[j].m_DeviceMemoryProperties.memoryHeaps[j].flags & VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
				{
					gpuSize += m_GPUs[j].m_DeviceMemoryProperties.memoryHeaps[j].size;
				}
			}

			if (gpuSize > currMaxHeapSize)
			{
				currMaxHeapSize = gpuSize;
				chosenGPUIndex = i;
			}
		}
	}

	return 0;
}


namespace BvRenderVk
{
	BvRenderEngine* GetRenderEngine()
	{
		static BvRenderEngineVk s_Engine;
		return &s_Engine;
	}
}