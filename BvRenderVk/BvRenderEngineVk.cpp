#include "BvRenderEngineVk.h"
#include "BvUtilsVk.h"
#include "BvRenderDeviceVk.h"
#include "BvDebugReportVk.h"
#include "BDeV/Core/Container/BvVector.h"


class BvRenderEngineVkHelper
{
public:
	static BvRenderEngineVk* Create()
	{
		static bool initialized = false;
		if (!initialized)
		{
			initialized = true;
			s_pEngine = BV_NEW(BvRenderEngineVk)();
			if (s_pEngine->GetGPUs().Size() == 0)
			{
				Destroy();
				s_pEngine = nullptr;
			}
		}
		else if (s_pEngine)
		{
			s_pEngine->AddRef();
		}

		return s_pEngine;
	}

	static void Destroy()
	{
		BV_DELETE_IN_PLACE(s_pEngine);
	}

private:
	static BvRenderEngineVk* s_pEngine;
};
BvRenderEngineVk* BvRenderEngineVkHelper::s_pEngine = nullptr;


bool IsInstanceExtensionSupported(const BvVector<VkExtensionProperties>& extensions, const char* pExtension);
bool IsInstanceLayerSupported(const BvVector<VkLayerProperties>& layers, const char* pLayer);
bool IsPhysicalDeviceExtensionSupported(const BvVector<VkExtensionProperties>& supportedExtensions, const char* pExtensionName);
bool IsDeviceSuitable(VkPhysicalDevice gpu);
void SetupDeviceInfo(VkPhysicalDevice physicalDevice, BvDeviceInfoVk& deviceInfo, BvGPUInfo& gpuInfo);


bool BvRenderEngineVk::CreateRenderDeviceImpl(const BvRenderDeviceCreateDesc& deviceCreateDesc, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_VK_IS_TYPE_VALID(objId, BvRenderDevice)))
	{
		return false;
	}

	BvRenderDeviceCreateDescVk descVk;
	memcpy(&descVk, &deviceCreateDesc, sizeof(BvRenderDeviceCreateDesc));
	u32 gpuIndex = descVk.m_GPUIndex;
	if (gpuIndex >= m_GPUs.Size())
	{
		gpuIndex = 0;
		for (auto i = 0; i < m_GPUs.Size(); ++i)
		{
			if (m_GPUs[i]->m_Type == GPUType::kDiscrete)
			{
				gpuIndex = i;
				break;
			}
		}
	}

	auto& pDevice = m_Devices[gpuIndex]->m_pDevice;
	BV_ASSERT_ONCE(pDevice == nullptr, "Render Device has already been created");
	if (!pDevice)
	{
		pDevice = BV_NEW(BvRenderDeviceVk)(this, m_Devices[gpuIndex]->m_PhysicalDevice, m_Devices[gpuIndex]->m_pDeviceInfo, gpuIndex, *m_GPUs[gpuIndex], descVk);
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


void BvRenderEngineVk::OnDeviceDestroyed(u32 index)
{
	m_Devices[index]->m_pDevice = nullptr;
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
		{ VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME, false }
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

	bool hasSurface2Caps = IsInstanceExtensionSupported(supportedExtensions, VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
	bool hasDebugUtils = IsInstanceExtensionSupported(supportedExtensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	BV_ASSERT(physicalDevices.Size() <= kMaxDevices, "Increase kMaxDevices count");

	m_GPUs.Resize(physicalDevices.Size(), {});
	m_Devices.Resize(physicalDevices.Size());
	for (auto i = 0; i < m_GPUs.Size(); ++i)
	{
		m_GPUs[i] = BV_NEW(BvGPUInfo)({});
		auto& pDeviceData = m_Devices[i] = BV_NEW(DeviceData)();
		pDeviceData->m_PhysicalDevice = physicalDevices[i];
		pDeviceData->m_pDeviceInfo = BV_NEW(BvDeviceInfoVk)();
		pDeviceData->m_pDeviceInfo->m_ExtendedSurfaceCaps.hasSurface2Caps = hasSurface2Caps;
		pDeviceData->m_pDeviceInfo->m_HasDebugUtils = hasDebugUtils;
		SetupDeviceInfo(physicalDevices[i], *pDeviceData->m_pDeviceInfo, *m_GPUs[i]);
	}

	if (hasDebugUtils)
	{
#if BV_DEBUG
		m_pDebugReport = BV_NEW(BvDebugReportVk)(m_Instance);
#endif
	}
}


void BvRenderEngineVk::Destroy()
{
	if (m_Instance)
	{
		for (auto pDeviceData : m_Devices)
		{
			auto pDevice = pDeviceData->m_pDevice;
			if (pDevice)
			{
				static_cast<IBvResourceVk*>(pDevice)->Destroy();
			}

			BV_DELETE(pDeviceData->m_pDeviceInfo);
			BV_DELETE(pDeviceData);
		}

		for (auto pGPU : m_GPUs)
		{
			BV_DELETE(pGPU);
		}

		if (m_pDebugReport)
		{
			BV_DELETE(m_pDebugReport);
		}

		vkDestroyInstance(m_Instance, nullptr);
		m_Instance = VK_NULL_HANDLE;

		m_VulkanLib.Close();
	}
}


void BvRenderEngineVk::SelfDestroy()
{
	BvRenderEngineVkHelper::Destroy();
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


bool IsPhysicalDeviceExtensionSupported(const BvVector<VkExtensionProperties>& supportedExtensions, const char* pExtensionName)
{
	for (auto& physicalDeviceExtension : supportedExtensions)
	{
		if (!strcmp(pExtensionName, physicalDeviceExtension.extensionName))
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


void SetupDeviceInfo(VkPhysicalDevice physicalDevice, BvDeviceInfoVk& deviceInfo, BvGPUInfo& gpuInfo)
{
	RenderDeviceCapabilities caps = RenderDeviceCapabilities::kNone;

	// =================================
	// Get / Store supported extensions
	u32 extensionCount = 0;
	auto result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
	BV_ASSERT(result == VK_SUCCESS, "Failed to setup device info");

	deviceInfo.m_SupportedExtensions.Resize(extensionCount);
	result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, deviceInfo.m_SupportedExtensions.Data());
	BV_ASSERT(result == VK_SUCCESS, "Failed to setup device info");

	bool swapChain = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME); // Also needs VK_KHR_SURFACE_EXTENSION_NAME
	BV_ASSERT(result == VK_SUCCESS, "No swap chain extension");

	deviceInfo.m_DeviceProperties.pNext = &deviceInfo.m_DeviceProperties1_1;
	deviceInfo.m_DeviceProperties1_1.pNext = &deviceInfo.m_DeviceProperties1_2;
	deviceInfo.m_DeviceProperties1_2.pNext = &deviceInfo.m_DeviceProperties1_3;

	deviceInfo.m_DeviceFeatures.pNext = &deviceInfo.m_DeviceFeatures1_1;
	deviceInfo.m_DeviceFeatures1_1.pNext = &deviceInfo.m_DeviceFeatures1_2;
	deviceInfo.m_DeviceFeatures1_2.pNext = &deviceInfo.m_DeviceFeatures1_3;

	void** pNextProperty = &deviceInfo.m_DeviceProperties1_3.pNext;
	void** pNextFeature = &deviceInfo.m_DeviceFeatures1_3.pNext;
	void** pNextMemoryProperty = &deviceInfo.m_DeviceMemoryProperties.pNext;

	bool vertexAttributeDivisor = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
	bool fragmentShading = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
	bool meshShader = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_EXT_MESH_SHADER_EXTENSION_NAME);
	bool accelerationStructure = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
	bool rayTracingPipeline = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
	bool rayQuery = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_KHR_RAY_QUERY_EXTENSION_NAME);
	bool conservativeRasterization = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
	bool customBorderColor = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
	bool memoryBudget = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
	bool deferredHostOperations = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
	bool deviceAddress = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
	bool predication = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
	bool depthClipEnable = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME);
	bool trueFullScreen = IsPhysicalDeviceExtensionSupported(deviceInfo.m_SupportedExtensions, VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME)
		&& deviceInfo.m_ExtendedSurfaceCaps.hasSurface2Caps;

	deviceInfo.m_EnabledExtensions.PushBack(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	if (vertexAttributeDivisor)
	{
		*pNextFeature = &deviceInfo.m_ExtendedFeatures.vertexAttributeDivisorFeatures;
		pNextFeature = &deviceInfo.m_ExtendedFeatures.vertexAttributeDivisorFeatures.pNext;

		*pNextProperty = &deviceInfo.m_ExtendedProperties.vertexAttributeDivisorProperties;
		pNextProperty = &deviceInfo.m_ExtendedProperties.vertexAttributeDivisorProperties.pNext;

		deviceInfo.m_EnabledExtensions.PushBack(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
	}

	if (fragmentShading)
	{
		*pNextFeature = &deviceInfo.m_ExtendedFeatures.fragmentShadingRateFeatures;
		pNextFeature = &deviceInfo.m_ExtendedFeatures.fragmentShadingRateFeatures.pNext;
		*pNextProperty = &deviceInfo.m_ExtendedProperties.fragmentShadingRateProps;
		pNextProperty = &deviceInfo.m_ExtendedProperties.fragmentShadingRateProps.pNext;

		deviceInfo.m_EnabledExtensions.PushBack(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
	}

	if (meshShader)
	{
		*pNextFeature = &deviceInfo.m_ExtendedFeatures.meshShaderFeatures;
		pNextFeature = &deviceInfo.m_ExtendedFeatures.meshShaderFeatures.pNext;

		*pNextProperty = &deviceInfo.m_ExtendedProperties.meshShaderProps;
		pNextProperty = &deviceInfo.m_ExtendedProperties.meshShaderProps.pNext;

		deviceInfo.m_EnabledExtensions.PushBack(VK_EXT_MESH_SHADER_EXTENSION_NAME);
	}

	if (accelerationStructure && deferredHostOperations && deviceAddress)
	{
		*pNextFeature = &deviceInfo.m_ExtendedFeatures.accelerationStructureFeatures;
		pNextFeature = &deviceInfo.m_ExtendedFeatures.accelerationStructureFeatures.pNext;

		*pNextProperty = &deviceInfo.m_ExtendedProperties.accelerationStructureProps;
		pNextProperty = &deviceInfo.m_ExtendedProperties.accelerationStructureProps.pNext;

		deviceInfo.m_EnabledExtensions.PushBack(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

		if (rayTracingPipeline)
		{

			*pNextFeature = &deviceInfo.m_ExtendedFeatures.rayTracingPipelineFeatures;
			pNextFeature = &deviceInfo.m_ExtendedFeatures.rayTracingPipelineFeatures.pNext;

			*pNextProperty = &deviceInfo.m_ExtendedProperties.rayTracingPipelineProps;
			pNextProperty = &deviceInfo.m_ExtendedProperties.rayTracingPipelineProps.pNext;

			deviceInfo.m_EnabledExtensions.PushBack(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
		}

		if (rayQuery)
		{
			*pNextFeature = &deviceInfo.m_ExtendedFeatures.rayQueryFeatures;
			pNextFeature = &deviceInfo.m_ExtendedFeatures.rayQueryFeatures.pNext;

			deviceInfo.m_EnabledExtensions.PushBack(VK_KHR_RAY_QUERY_EXTENSION_NAME);
		}
	}

	if (conservativeRasterization)
	{
		*pNextProperty = &deviceInfo.m_ExtendedProperties.convervativeRasterizationProps;
		pNextProperty = &deviceInfo.m_ExtendedProperties.convervativeRasterizationProps.pNext;

		deviceInfo.m_EnabledExtensions.PushBack(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
	}

	if (customBorderColor)
	{
		*pNextFeature = &deviceInfo.m_ExtendedFeatures.customBorderColorFeatures;
		pNextFeature = &deviceInfo.m_ExtendedFeatures.customBorderColorFeatures.pNext;

		*pNextProperty = &deviceInfo.m_ExtendedProperties.customBorderColorProps;
		pNextProperty = &deviceInfo.m_ExtendedProperties.customBorderColorProps.pNext;

		deviceInfo.m_EnabledExtensions.PushBack(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
	}

	if (memoryBudget)
	{
		*pNextMemoryProperty = &deviceInfo.m_ExtendedProperties.memoryBudgetProperties;
		pNextMemoryProperty = &deviceInfo.m_ExtendedProperties.memoryBudgetProperties.pNext;

		deviceInfo.m_EnabledExtensions.PushBack(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
	}

	if (deferredHostOperations)
	{
		deviceInfo.m_EnabledExtensions.PushBack(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
	}

	if (predication)
	{
		*pNextFeature = &deviceInfo.m_ExtendedFeatures.conditionalRenderingFeatures;
		pNextFeature = &deviceInfo.m_ExtendedFeatures.conditionalRenderingFeatures.pNext;

		deviceInfo.m_EnabledExtensions.PushBack(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
	}

	if (depthClipEnable)
	{
		*pNextFeature = &deviceInfo.m_ExtendedFeatures.depthClibEnableFeature;
		pNextFeature = &deviceInfo.m_ExtendedFeatures.depthClibEnableFeature.pNext;

		deviceInfo.m_EnabledExtensions.PushBack(VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME);
	}

	if (trueFullScreen)
	{
		deviceInfo.m_EnabledExtensions.PushBack(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);
	}

	*pNextProperty = &deviceInfo.m_ExtendedProperties.multiviewProperties;
	pNextProperty = &deviceInfo.m_ExtendedProperties.multiviewProperties.pNext;

	// =================================
	// Store features / properties / memory properties
	vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceInfo.m_DeviceFeatures);
	vkGetPhysicalDeviceProperties2(physicalDevice, &deviceInfo.m_DeviceProperties);
	vkGetPhysicalDeviceMemoryProperties2(physicalDevice, &deviceInfo.m_DeviceMemoryProperties);

	if (deviceInfo.m_DeviceFeatures.features.fillModeNonSolid)
	{
		caps |= RenderDeviceCapabilities::kWireframe;
	}
	if (deviceInfo.m_DeviceFeatures.features.geometryShader)
	{
		caps |= RenderDeviceCapabilities::kGeometryShader;
	}
	if (deviceInfo.m_DeviceFeatures.features.tessellationShader)
	{
		caps |= RenderDeviceCapabilities::kTesselationShader;
	}
	if (deviceInfo.m_DeviceFeatures.features.depthBounds)
	{
		caps |= RenderDeviceCapabilities::kDepthBoundsTest;
	}
	if (deviceInfo.m_DeviceProperties.properties.limits.timestampComputeAndGraphics)
	{
		caps |= RenderDeviceCapabilities::kTimestampQueries;
	}
	if (deviceInfo.m_DeviceFeatures.features.multiDrawIndirect)
	{
		caps |= RenderDeviceCapabilities::kIndirectDrawCount;
	}
	if (deviceInfo.m_DeviceFeatures1_2.samplerFilterMinmax)
	{
		caps |= RenderDeviceCapabilities::kSamplerMinMaxReduction;
	}
	if (customBorderColor)
	{
		caps |= RenderDeviceCapabilities::kCustomBorderColor;
	}
	if (predication)
	{
		caps |= RenderDeviceCapabilities::kPredication;
	}
	if (conservativeRasterization)
	{
		caps |= RenderDeviceCapabilities::kConservativeRasterization;
	}
	if (fragmentShading)
	{
		caps |= RenderDeviceCapabilities::kShadingRate;
	}
	if (meshShader && deviceInfo.m_ExtendedFeatures.meshShaderFeatures.meshShader && deviceInfo.m_ExtendedFeatures.meshShaderFeatures.taskShader)
	{
		caps |= RenderDeviceCapabilities::kMeshShader;
		if (deviceInfo.m_ExtendedFeatures.meshShaderFeatures.meshShaderQueries)
		{
			caps |= RenderDeviceCapabilities::kMeshQuery;
		}
	}
	if (rayTracingPipeline)
	{
		caps |= RenderDeviceCapabilities::kRayTracing;
	}
	if (rayQuery)
	{
		caps |= RenderDeviceCapabilities::kRayQuery;
	}
	if (deviceInfo.m_DeviceFeatures1_1.multiview)
	{
		caps |= RenderDeviceCapabilities::kMultiView;
	}
	if (trueFullScreen)
	{
		caps |= RenderDeviceCapabilities::kTrueFullScreen;
	}

	strcpy(gpuInfo.m_DeviceName, deviceInfo.m_DeviceProperties.properties.deviceName);
	gpuInfo.m_DeviceId = deviceInfo.m_DeviceProperties.properties.deviceID;
	gpuInfo.m_VendorId = deviceInfo.m_DeviceProperties.properties.vendorID;

	for (u32 i = 0; i < deviceInfo.m_DeviceMemoryProperties.memoryProperties.memoryHeapCount; i++)
	{
		if (deviceInfo.m_DeviceMemoryProperties.memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
		{
			gpuInfo.m_DeviceMemory += deviceInfo.m_DeviceMemoryProperties.memoryProperties.memoryHeaps[i].size;
		}
	}

	u32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyCount, nullptr);
	deviceInfo.m_QueueFamilyProperties.Resize(queueFamilyCount, { VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2 });
	vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyCount, deviceInfo.m_QueueFamilyProperties.Data());

	auto hasFlagFn = [](VkQueueFlags queueFlags, VkQueueFlagBits flag)
		{
			return (queueFlags & flag);
		};

	deviceInfo.m_QueueFamilyIndices.Reserve(deviceInfo.m_QueueFamilyProperties.Size());
	for (auto i = 0; i < deviceInfo.m_QueueFamilyProperties.Size(); ++i)
	{
		auto& props = deviceInfo.m_QueueFamilyProperties[i].queueFamilyProperties;
		if (props.queueFlags == 0 || props.queueCount == 0)
		{
			continue;
		}
		deviceInfo.m_QueueFamilyIndices.EmplaceBack(i);

		bool hasGraphics = hasFlagFn(props.queueFlags, VK_QUEUE_GRAPHICS_BIT);
		bool hasCompute = hasFlagFn(props.queueFlags, VK_QUEUE_COMPUTE_BIT);
		bool hasTransfer = hasFlagFn(props.queueFlags, VK_QUEUE_TRANSFER_BIT);
		bool hasVideoDecode = hasFlagFn(props.queueFlags, VK_QUEUE_VIDEO_DECODE_BIT_KHR);
		bool hasVideoEncode = hasFlagFn(props.queueFlags, VK_QUEUE_VIDEO_ENCODE_BIT_KHR);

		auto& contextGroup = gpuInfo.m_ContextGroups.EmplaceBack(ContextGroup{});
		contextGroup.m_GroupIndex = i;
		contextGroup.m_MaxContextCount = props.queueCount;
		if (hasGraphics)
		{
			gpuInfo.m_GraphicsContextCount += props.queueCount;
			contextGroup.m_SupportedCommandTypes.EmplaceBack(CommandType::kGraphics);
		}
		if (hasCompute)
		{
			gpuInfo.m_ComputeContextCount += props.queueCount;
			contextGroup.m_SupportedCommandTypes.EmplaceBack(CommandType::kCompute);
		}
		if (hasTransfer)
		{
			contextGroup.m_SupportedCommandTypes.EmplaceBack(CommandType::kTransfer);
			gpuInfo.m_TransferContextCount += props.queueCount;
		}
		if (hasVideoDecode)
		{
			contextGroup.m_SupportedCommandTypes.EmplaceBack(CommandType::kVideoDecode);
		}
		if (hasVideoEncode)
		{
			contextGroup.m_SupportedCommandTypes.EmplaceBack(CommandType::kVideoEncode);
		}

		bool hasVideo = hasVideoDecode || hasVideoEncode;
		// Mark queues that are dedicated
		if (hasGraphics)
		{
			contextGroup.m_DedicatedCommandType = !hasVideo ? CommandType::kGraphics : CommandType::kNone;
		}
		else if (hasCompute)
		{
			contextGroup.m_DedicatedCommandType = !hasVideo ? CommandType::kCompute : CommandType::kNone;
		}
		else if (hasVideo)
		{
			if (!hasVideoEncode)
			{
				contextGroup.m_DedicatedCommandType = CommandType::kVideoDecode;
			}
			else if (!hasVideoDecode)
			{
				contextGroup.m_DedicatedCommandType = CommandType::kVideoEncode;
			}
			else
			{
				contextGroup.m_DedicatedCommandType = CommandType::kNone;
			}
		}
		else if (hasTransfer)
		{
			contextGroup.m_DedicatedCommandType = CommandType::kTransfer;
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

	switch (deviceInfo.m_DeviceProperties.properties.deviceType)
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

	gpuInfo.m_DeviceCaps = caps;
}


namespace BvRenderVk
{
	extern "C"
	{
		BV_API bool CreateRenderEngine(const BvUUID& objId, void** ppObj)
		{
			if (!ppObj || (objId != BV_OBJECT_ID(BvRenderEngineVk) && objId != BV_OBJECT_ID(IBvRenderEngine)))
			{
				return false;
			}

			*ppObj = BvRenderEngineVkHelper::Create();
			
			return *ppObj != nullptr;
		}
	}
}