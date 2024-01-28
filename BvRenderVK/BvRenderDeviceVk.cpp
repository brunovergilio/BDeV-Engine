#include "BvRenderDeviceVk.h"
#include "BvUtilsVk.h"
#include "BvCommandQueueVk.h"
#include "BvSwapChainVk.h"
#include "BvRenderPassVk.h"
#include "BvCommandPoolVk.h"
#include "BvShaderResourceVk.h"
#include "BvPipelineStateVk.h"
#include "BvSemaphoreVk.h"
#include "BvBufferVk.h"
#include "BvBufferViewVk.h"
#include "BvShaderResourceSetPoolVk.h"
#include "BvRenderVK/BvFramebufferVk.h"
#include "BvTypeConversionsVk.h"


BvRenderDeviceVk::BvRenderDeviceVk(BvRenderEngineVk* pEngine, const BvGPUInfoVk & gpuInfo, const DeviceCreateDesc& deviceDesc)
	: m_pEngine(pEngine), m_GPUInfo(gpuInfo), m_pFactory(new BvRenderDeviceFactory())
{
	Create(deviceDesc);
}


BvRenderDeviceVk::~BvRenderDeviceVk()
{
	Destroy();
}


void BvRenderDeviceVk::SetupDeviceFeatures(VkDeviceCreateInfo& deviceCreateInfo, VkPhysicalDeviceFeatures& enabledFeatures, BvVector<const char*>& enabledExtensions)
{
	// ===================================================
	// Default features
#define VK_ADD_FEATURE_IF_SUPPORTED(feature) if (m_GPUInfo.m_DeviceFeatures.feature) enabledFeatures.feature = VK_TRUE;
	//VK_ADD_FEATURE_IF_SUPPORTED(robustBufferAccess);
	VK_ADD_FEATURE_IF_SUPPORTED(fullDrawIndexUint32);
	VK_ADD_FEATURE_IF_SUPPORTED(imageCubeArray);
	//VK_ADD_FEATURE_IF_SUPPORTED(independentBlend);
	VK_ADD_FEATURE_IF_SUPPORTED(geometryShader);
	VK_ADD_FEATURE_IF_SUPPORTED(tessellationShader);
	VK_ADD_FEATURE_IF_SUPPORTED(sampleRateShading);
	VK_ADD_FEATURE_IF_SUPPORTED(dualSrcBlend);
	//VK_ADD_FEATURE_IF_SUPPORTED(logicOp);
	VK_ADD_FEATURE_IF_SUPPORTED(multiDrawIndirect);
	//VK_ADD_FEATURE_IF_SUPPORTED(drawIndirectFirstInstance);
	VK_ADD_FEATURE_IF_SUPPORTED(depthClamp);
	VK_ADD_FEATURE_IF_SUPPORTED(depthBiasClamp);
	VK_ADD_FEATURE_IF_SUPPORTED(fillModeNonSolid);
	VK_ADD_FEATURE_IF_SUPPORTED(depthBounds);
	//VK_ADD_FEATURE_IF_SUPPORTED(wideLines);
	//VK_ADD_FEATURE_IF_SUPPORTED(largePoints);
	//VK_ADD_FEATURE_IF_SUPPORTED(alphaToOne);
	VK_ADD_FEATURE_IF_SUPPORTED(multiViewport);
	VK_ADD_FEATURE_IF_SUPPORTED(samplerAnisotropy);
	VK_ADD_FEATURE_IF_SUPPORTED(textureCompressionETC2);
	VK_ADD_FEATURE_IF_SUPPORTED(textureCompressionASTC_LDR);
	VK_ADD_FEATURE_IF_SUPPORTED(textureCompressionBC);
	VK_ADD_FEATURE_IF_SUPPORTED(occlusionQueryPrecise);
	VK_ADD_FEATURE_IF_SUPPORTED(pipelineStatisticsQuery);
	VK_ADD_FEATURE_IF_SUPPORTED(vertexPipelineStoresAndAtomics);
	VK_ADD_FEATURE_IF_SUPPORTED(fragmentStoresAndAtomics);
	//VK_ADD_FEATURE_IF_SUPPORTED(shaderTessellationAndGeometryPointSize);
	//VK_ADD_FEATURE_IF_SUPPORTED(shaderImageGatherExtended);
	VK_ADD_FEATURE_IF_SUPPORTED(shaderStorageImageExtendedFormats);
	//VK_ADD_FEATURE_IF_SUPPORTED(shaderStorageImageMultisample);
	VK_ADD_FEATURE_IF_SUPPORTED(shaderStorageImageReadWithoutFormat);
	VK_ADD_FEATURE_IF_SUPPORTED(shaderStorageImageWriteWithoutFormat);
	VK_ADD_FEATURE_IF_SUPPORTED(shaderUniformBufferArrayDynamicIndexing);
	VK_ADD_FEATURE_IF_SUPPORTED(shaderSampledImageArrayDynamicIndexing);
	VK_ADD_FEATURE_IF_SUPPORTED(shaderStorageBufferArrayDynamicIndexing);
	VK_ADD_FEATURE_IF_SUPPORTED(shaderStorageImageArrayDynamicIndexing);
	VK_ADD_FEATURE_IF_SUPPORTED(shaderClipDistance);
	VK_ADD_FEATURE_IF_SUPPORTED(shaderCullDistance);
	//VK_ADD_FEATURE_IF_SUPPORTED(shaderFloat64);
	//VK_ADD_FEATURE_IF_SUPPORTED(shaderInt64);
	//VK_ADD_FEATURE_IF_SUPPORTED(shaderInt16);
	//VK_ADD_FEATURE_IF_SUPPORTED(shaderResourceResidency);
	//VK_ADD_FEATURE_IF_SUPPORTED(shaderResourceMinLod);
	VK_ADD_FEATURE_IF_SUPPORTED(sparseBinding);
	VK_ADD_FEATURE_IF_SUPPORTED(sparseResidencyBuffer);
	VK_ADD_FEATURE_IF_SUPPORTED(sparseResidencyImage2D);
	VK_ADD_FEATURE_IF_SUPPORTED(sparseResidencyImage3D);
	VK_ADD_FEATURE_IF_SUPPORTED(sparseResidency2Samples);
	VK_ADD_FEATURE_IF_SUPPORTED(sparseResidency4Samples);
	VK_ADD_FEATURE_IF_SUPPORTED(sparseResidency8Samples);
	VK_ADD_FEATURE_IF_SUPPORTED(sparseResidency16Samples);
	VK_ADD_FEATURE_IF_SUPPORTED(sparseResidencyAliased);
	VK_ADD_FEATURE_IF_SUPPORTED(variableMultisampleRate);
	//VK_ADD_FEATURE_IF_SUPPORTED(inheritedQueries);
#undef VK_ADD_FEATURE_IF_SUPPORTED

	// ===================================================
	// Extended features
	if (m_GPUInfo.m_FeaturesSupported.swapChain)
	{
		enabledExtensions.PushBack(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

	if (!m_GPUInfo.m_FeaturesSupported.deviceProperties2)
	{
		return;
	}

	void** pNextFeature = const_cast<void**>(&deviceCreateInfo.pNext);
	BvGPUInfoVk& gpuInfo = const_cast<BvGPUInfoVk&>(m_GPUInfo);

	if (m_GPUInfo.m_FeaturesSupported.vertexAttributeDivisor)
	{
		enabledExtensions.PushBack(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);

		*pNextFeature = &gpuInfo.m_ExtendedFeatures.vertexAttributeDivisorFeatures;
		pNextFeature = &gpuInfo.m_ExtendedFeatures.vertexAttributeDivisorFeatures.pNext;
	}

	if (m_GPUInfo.m_FeaturesSupported.memoryRequirements2)
	{
		enabledExtensions.PushBack(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
	}

	if (m_GPUInfo.m_FeaturesSupported.bindMemory2)
	{
		enabledExtensions.PushBack(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
	}

	if (m_GPUInfo.m_FeaturesSupported.maintenance1)
	{
		enabledExtensions.PushBack(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
	}

	if (m_GPUInfo.m_FeaturesSupported.maintenance2)
	{
		enabledExtensions.PushBack(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
	}

	if (m_GPUInfo.m_FeaturesSupported.multiview)
	{
		enabledExtensions.PushBack(VK_KHR_MULTIVIEW_EXTENSION_NAME);

		*pNextFeature = &gpuInfo.m_ExtendedFeatures.multiviewFeatures;
		pNextFeature = &gpuInfo.m_ExtendedFeatures.multiviewFeatures.pNext;
	}

	if (m_GPUInfo.m_FeaturesSupported.renderpass2)
	{
		enabledExtensions.PushBack(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
	}

	if (m_GPUInfo.m_FeaturesSupported.dynamicRendering)
	{
		enabledExtensions.PushBack(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

		*pNextFeature = &gpuInfo.m_ExtendedFeatures.dynamicRenderingFeatures;
		pNextFeature = &gpuInfo.m_ExtendedFeatures.dynamicRenderingFeatures.pNext;
	}

	if (m_GPUInfo.m_FeaturesSupported.fragmentShading)
	{
		enabledExtensions.PushBack(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);

		*pNextFeature = &gpuInfo.m_ExtendedFeatures.fragmentShadingRateFeatures;
		pNextFeature = &gpuInfo.m_ExtendedFeatures.fragmentShadingRateFeatures.pNext;
	}

	if (m_GPUInfo.m_FeaturesSupported.maintenance3)
	{
		enabledExtensions.PushBack(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
	}

	if (m_GPUInfo.m_FeaturesSupported.descriptorIndexing)
	{
		enabledExtensions.PushBack(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);

		*pNextFeature = &gpuInfo.m_ExtendedFeatures.descriptorIndexingFeatures;
		pNextFeature = &gpuInfo.m_ExtendedFeatures.descriptorIndexingFeatures.pNext;
	}

	if (m_GPUInfo.m_FeaturesSupported.bufferDeviceAddress)
	{
		enabledExtensions.PushBack(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);

		*pNextFeature = &gpuInfo.m_ExtendedFeatures.bufferDeviceAddressFeatures;
		pNextFeature = &gpuInfo.m_ExtendedFeatures.bufferDeviceAddressFeatures.pNext;
	}

	if (m_GPUInfo.m_FeaturesSupported.deferredHostOperations)
	{
		enabledExtensions.PushBack(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
	}

	if (m_GPUInfo.m_FeaturesSupported.shaderFloatControls)
	{
		enabledExtensions.PushBack(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
	}

	if (m_GPUInfo.m_FeaturesSupported.spirv1_4)
	{
		enabledExtensions.PushBack(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
	}

	if (m_GPUInfo.m_FeaturesSupported.meshShader)
	{
		enabledExtensions.PushBack(VK_EXT_MESH_SHADER_EXTENSION_NAME);

		*pNextFeature = &gpuInfo.m_ExtendedFeatures.meshShaderFeatures;
		pNextFeature = &gpuInfo.m_ExtendedFeatures.meshShaderFeatures.pNext;
	}

	if (m_GPUInfo.m_FeaturesSupported.accelerationStructure)
	{
		enabledExtensions.PushBack(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

		*pNextFeature = &gpuInfo.m_ExtendedFeatures.accelerationStructureFeatures;
		pNextFeature = &gpuInfo.m_ExtendedFeatures.accelerationStructureFeatures.pNext;
	}

	if (m_GPUInfo.m_FeaturesSupported.rayTracingPipeline)
	{
		enabledExtensions.PushBack(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);

		*pNextFeature = &gpuInfo.m_ExtendedFeatures.rayTracingPipelineFeatures;
		pNextFeature = &gpuInfo.m_ExtendedFeatures.rayTracingPipelineFeatures.pNext;
	}

	if (m_GPUInfo.m_FeaturesSupported.rayQuery)
	{
		enabledExtensions.PushBack(VK_KHR_RAY_QUERY_EXTENSION_NAME);

		*pNextFeature = &gpuInfo.m_ExtendedFeatures.rayQueryFeatures;
		pNextFeature = &gpuInfo.m_ExtendedFeatures.rayQueryFeatures.pNext;
	}

	if (m_GPUInfo.m_FeaturesSupported.pipelineLibrary)
	{
		enabledExtensions.PushBack(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
	}

	if (m_GPUInfo.m_FeaturesSupported.conservativeRasterization)
	{
		enabledExtensions.PushBack(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
	}

	if (m_GPUInfo.m_FeaturesSupported.customBorderColor)
	{
		enabledExtensions.PushBack(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);

		*pNextFeature = &gpuInfo.m_ExtendedFeatures.customBorderColorFeatures;
		pNextFeature = &gpuInfo.m_ExtendedFeatures.customBorderColorFeatures.pNext;
	}

	if (m_GPUInfo.m_FeaturesSupported.timelineSemaphore)
	{
		enabledExtensions.PushBack(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

		*pNextFeature = &gpuInfo.m_ExtendedFeatures.timelineSemaphoreFeatures;
		pNextFeature = &gpuInfo.m_ExtendedFeatures.timelineSemaphoreFeatures.pNext;
	}

	if (m_GPUInfo.m_FeaturesSupported.imageFormatList)
	{
		enabledExtensions.PushBack(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
	}

	if (m_GPUInfo.m_FeaturesSupported.imagelessFrameBuffer)
	{
		enabledExtensions.PushBack(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);

		*pNextFeature = &gpuInfo.m_ExtendedFeatures.imagelessFrameBufferFeatures;
		pNextFeature = &gpuInfo.m_ExtendedFeatures.imagelessFrameBufferFeatures.pNext;
	}

	if (m_GPUInfo.m_FeaturesSupported.memoryBudget)
	{
		enabledExtensions.PushBack(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
	}
}


void BvRenderDeviceVk::Create(const DeviceCreateDesc & deviceCreateDesc)
{
	BvAssert(deviceCreateDesc.m_GraphicsQueueCount + deviceCreateDesc.m_ComputeQueueCount + deviceCreateDesc.m_TransferQueueCount > 0,
		"No device queues");

	// ===========================================================
	// Prepare Device Queues and create the logical device
	BvVector<VkDeviceQueueCreateInfo> queueInfos;
	VkDeviceQueueCreateInfo queueCreateInfo{};

	constexpr u32 kMaxQueueCount = 64;
	BvAssert(deviceCreateDesc.m_GraphicsQueueCount <= kMaxQueueCount, "Graphics queue count greater than limit");
	BvAssert(deviceCreateDesc.m_ComputeQueueCount <= kMaxQueueCount, "Compute queue count greater than limit");
	BvAssert(deviceCreateDesc.m_TransferQueueCount <= kMaxQueueCount, "Transfer queue count greater than limit");

	constexpr float queuePriorities[kMaxQueueCount] =
	{
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
	};

	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.pQueuePriorities = queuePriorities;
	queueCreateInfo.queueFamilyIndex = m_GPUInfo.m_GraphicsQueueIndex;
	if (deviceCreateDesc.m_GraphicsQueueCount > 0)
	{
		queueCreateInfo.queueCount = deviceCreateDesc.m_GraphicsQueueCount;
		queueInfos.PushBack(queueCreateInfo);
	}
	if (m_GPUInfo.m_GraphicsQueueIndex != m_GPUInfo.m_ComputeQueueIndex
		&& deviceCreateDesc.m_ComputeQueueCount > 0)
	{
		queueCreateInfo.queueFamilyIndex = m_GPUInfo.m_ComputeQueueIndex;
		queueCreateInfo.queueCount = deviceCreateDesc.m_ComputeQueueCount;
		queueInfos.PushBack(queueCreateInfo);
	}
	if (m_GPUInfo.m_GraphicsQueueIndex != m_GPUInfo.m_TransferQueueIndex
		&& m_GPUInfo.m_ComputeQueueIndex != m_GPUInfo.m_TransferQueueIndex
		&& deviceCreateDesc.m_TransferQueueCount > 0)
	{
		queueCreateInfo.queueFamilyIndex = m_GPUInfo.m_TransferQueueIndex;
		queueCreateInfo.queueCount = deviceCreateDesc.m_TransferQueueCount;
		queueInfos.PushBack(queueCreateInfo);
	}


	// Check if queues are the same and use the highest queueCount from the ones set in the DeviceDesc
	// Also check whether a presentation queue has been included
	bool hasPresentationQueue = false;
	for (auto && queueInfo : queueInfos)
	{
		if (queueInfo.queueFamilyIndex == m_GPUInfo.m_GraphicsQueueIndex
			&& deviceCreateDesc.m_GraphicsQueueCount > queueInfo.queueCount)
		{
			queueInfo.queueCount = deviceCreateDesc.m_GraphicsQueueCount;
		}
		if (queueInfo.queueFamilyIndex == m_GPUInfo.m_ComputeQueueIndex
			&& deviceCreateDesc.m_ComputeQueueCount > queueInfo.queueCount)
		{
			queueInfo.queueCount = deviceCreateDesc.m_ComputeQueueCount;
		}
		if (queueInfo.queueFamilyIndex == m_GPUInfo.m_TransferQueueIndex
			&& deviceCreateDesc.m_TransferQueueCount > queueInfo.queueCount)
		{
			queueInfo.queueCount = deviceCreateDesc.m_TransferQueueCount;
		}
		if (queueInfo.queueFamilyIndex == m_GPUInfo.m_PresentationQueueIndex)
		{
			hasPresentationQueue = true;
		}
	}

	BvVector<const char *> enabledExtensions;
	if (deviceCreateDesc.m_GraphicsQueueCount > 0)
	{
		// If we have at least one graphics queue, and we still don't have a presentation queue, add one
		if (!hasPresentationQueue)
		{
			queueCreateInfo.queueFamilyIndex = m_GPUInfo.m_PresentationQueueIndex;
			queueCreateInfo.queueCount = 1;
			queueInfos.PushBack(queueCreateInfo);
		}
	}

	VkPhysicalDeviceFeatures enabledFeatures{};
	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = queueInfos.Data();
	deviceCreateInfo.queueCreateInfoCount = static_cast<u32>(queueInfos.Size());
	SetupDeviceFeatures(deviceCreateInfo, enabledFeatures, enabledExtensions);
	deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.Data();
	deviceCreateInfo.enabledExtensionCount = static_cast<u32>(enabledExtensions.Size());

	VkResult result = vkCreateDevice(m_GPUInfo.m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device);
	BvAssert(result == VK_SUCCESS, "Couldn't create a logical device");
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		return;
	}

	volkLoadDevice(m_Device);

	m_GraphicsQueues.Resize(deviceCreateDesc.m_GraphicsQueueCount);
	m_ComputeQueues.Resize(deviceCreateDesc.m_ComputeQueueCount);
	m_TransferQueues.Resize(deviceCreateDesc.m_TransferQueueCount);
	for (auto i = 0; i < deviceCreateDesc.m_GraphicsQueueCount; i++)
	{
		m_GraphicsQueues[i] = new BvCommandQueueVk(*this, QueueFamilyType::kGraphics, i);
	}
	for (auto i = 0; i < deviceCreateDesc.m_ComputeQueueCount; i++)
	{
		m_ComputeQueues[i] = new BvCommandQueueVk(*this, QueueFamilyType::kCompute, i);
	}
	for (auto i = 0; i < deviceCreateDesc.m_TransferQueueCount; i++)
	{
		m_TransferQueues[i] = new BvCommandQueueVk(*this, QueueFamilyType::kTransfer, i);
	}

	CreateVMA();

	m_pRenderPassManager = new BvRenderPassManagerVk();
	m_pFramebufferManager = new BvFramebufferManagerVk();
}


void BvRenderDeviceVk::Destroy()
{
	vkDeviceWaitIdle(m_Device);

	//m_pFactory->DestroyAllOfType<BvGraphicsPipelineStateVk>();
	//m_pFactory->DestroyAllOfType<BvShaderResourceSetPoolVk>();
	//m_pFactory->DestroyAllOfType<BvShaderResourceLayoutVk>();
	//m_pFactory->DestroyAllOfType<BvCommandPoolVk>();
	//m_pFactory->DestroyAllOfType<BvRenderPassVk>();
	//m_pFactory->DestroyAllOfType<BvSemaphoreVk>();
	//m_pFactory->DestroyAllOfType<BvTextureViewVk>();
	//m_pFactory->DestroyAllOfType<BvTextureVk>();
	//m_pFactory->DestroyAllOfType<BvBufferViewVk>();
	//m_pFactory->DestroyAllOfType<BvBufferVk>();
	//m_pFactory->DestroyAllOfType<BvSwapChainVk>();

	if (m_pFactory)
	{
		delete m_pFactory;
		m_pFactory = nullptr;
	}

	delete m_pFramebufferManager;
	delete m_pRenderPassManager;

	for (auto&& pQueue : m_GraphicsQueues)
	{
		delete pQueue;
	}
	for (auto&& pQueue : m_ComputeQueues)
	{
		delete pQueue;
	}
	for (auto&& pQueue : m_TransferQueues)
	{
		delete pQueue;
	}

	DestroyVMA();

	if (m_Device)
	{
		vkDestroyDevice(m_Device, nullptr);
		m_Device = VK_NULL_HANDLE;
	}
}


BvSwapChain * BvRenderDeviceVk::CreateSwapChain(BvWindow* pWindow, const SwapChainDesc & swapChainDesc, BvCommandQueue & commandQueue)
{
	auto pQueue = static_cast<BvCommandQueueVk *>(&commandQueue);

	return m_pFactory->Create<BvSwapChainVk>(*this, *pQueue, pWindow, swapChainDesc);
}


BvBuffer* BvRenderDeviceVk::CreateBuffer(const BufferDesc& desc)
{
	return m_pFactory->Create<BvBufferVk>(*this, desc);
}


BvBufferView* BvRenderDeviceVk::CreateBufferView(const BufferViewDesc& desc)
{
	return m_pFactory->Create<BvBufferViewVk>(*this, desc);
}


BvTexture* BvRenderDeviceVk::CreateTexture(const TextureDesc& desc)
{
	return m_pFactory->Create<BvTextureVk>(*this, desc);
}


BvTextureView* BvRenderDeviceVk::CreateTextureView(const TextureViewDesc& desc)
{
	return m_pFactory->Create<BvTextureViewVk>(*this, desc);
}


BvSemaphore * BvRenderDeviceVk::CreateSemaphore(const u64 initialValue)
{
	// Always use timeline semaphores when available
	return m_pFactory->Create<BvSemaphoreVk>(*this, m_GPUInfo.m_FeaturesSupported.timelineSemaphore, initialValue);
}


BvRenderPass * BvRenderDeviceVk::CreateRenderPass(const RenderPassDesc & renderPassDesc)
{
	return m_pRenderPassManager->GetRenderPass(*this, renderPassDesc);
}


BvCommandPool * BvRenderDeviceVk::CreateCommandPool(const CommandPoolDesc & commandPoolDesc)
{
	return m_pFactory->Create<BvCommandPoolVk>(*this, commandPoolDesc);
}


BvShaderResourceLayout * BvRenderDeviceVk::CreateShaderResourceLayout(const ShaderResourceLayoutDesc & shaderResourceLayoutDesc)
{
	return m_pFactory->Create<BvShaderResourceLayoutVk>(*this, shaderResourceLayoutDesc);
}


BvShaderResourceSetPool* BvRenderDeviceVk::CreateShaderResourceSetPool(const ShaderResourceSetPoolDesc& shaderResourceSetPoolDesc)
{
	return m_pFactory->Create<BvShaderResourceSetPoolVk>(*this, shaderResourceSetPoolDesc);
}


BvGraphicsPipelineState * BvRenderDeviceVk::CreateGraphicsPipeline(const GraphicsPipelineStateDesc & graphicsPipelineStateDesc)
{
	return m_pFactory->Create<BvGraphicsPipelineStateVk>(*this, graphicsPipelineStateDesc, VkPipelineCache(VK_NULL_HANDLE));
}


void BvRenderDeviceVk::WaitIdle() const
{
	auto result = vkDeviceWaitIdle(m_Device);
}


const u32 BvRenderDeviceVk::GetMemoryTypeIndex(const u32 memoryTypeBits, const VkMemoryPropertyFlags properties) const
{
	u32 index = UINT32_MAX;
	u32 typeBits = memoryTypeBits;

	auto & memoryProperties = m_GPUInfo.m_DeviceMemoryProperties;
	// Iterate over all memory types available for the device used in this example
	for (u32 i = 0; i < memoryProperties.memoryTypeCount; i++)
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


const VkFormat BvRenderDeviceVk::GetBestDepthFormat(const VkFormat format /*= VK_FORMAT_UNDEFINED*/) const
{
	VkFormat chosenFormat = VK_FORMAT_UNDEFINED;

	// Since all depth formats may be optional, we need to find a suitable depth format to use
	// Start with the highest precision packed format
	VkFormat depthFormats[] =
	{
		VK_FORMAT_D24_UNORM_S8_UINT, // 24-bit depths are better for optimal performance
		VK_FORMAT_D32_SFLOAT_S8_UINT, // 32-bit depth is costlier, use if you need more precision
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

	VkFormatProperties formatProps;
	for (auto & depthFormat : depthFormats)
	{
		vkGetPhysicalDeviceFormatProperties(m_GPUInfo.m_PhysicalDevice, depthFormat, &formatProps);
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


bool BvRenderDeviceVk::QueueFamilySupportsPresent(const QueueFamilyType queueFamilyType) const
{
	u32 queueFamilyIndex = UINT32_MAX;

	switch (queueFamilyType)
	{
	case QueueFamilyType::kGraphics:	queueFamilyIndex = m_GPUInfo.m_GraphicsQueueIndex; break;
	case QueueFamilyType::kCompute:		queueFamilyIndex = m_GPUInfo.m_ComputeQueueIndex; break;
	case QueueFamilyType::kTransfer:	queueFamilyIndex = m_GPUInfo.m_TransferQueueIndex; break;
	}

#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	return vkGetPhysicalDeviceWin32PresentationSupportKHR(m_GPUInfo.m_PhysicalDevice, queueFamilyIndex) == VK_TRUE;
#else
#endif
}


bool BvRenderDeviceVk::HasImageSupport(Format format)
{
	const auto& vkFormatMap = GetVkFormatMap(format);

	VkFormatProperties formatProperties{};
	vkGetPhysicalDeviceFormatProperties(m_GPUInfo.m_PhysicalDevice, GetVkFormat(format), &formatProperties);

	VkFormatFeatureFlags featureFlags = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
	if (vkFormatMap.aspectFlags & VK_IMAGE_ASPECT_COLOR_BIT)
	{
		featureFlags |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
	}
	else if ((vkFormatMap.aspectFlags & VK_IMAGE_ASPECT_DEPTH_BIT) || (vkFormatMap.aspectFlags & VK_IMAGE_ASPECT_STENCIL_BIT))
	{
		featureFlags |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}

	return (formatProperties.optimalTilingFeatures & featureFlags) == featureFlags
		|| (formatProperties.linearTilingFeatures & featureFlags) == featureFlags;
}


BvCommandQueue* BvRenderDeviceVk::GetGraphicsQueue(const u32 index) const
{
	return m_GraphicsQueues[index];
}


BvCommandQueue* BvRenderDeviceVk::GetComputeQueue(const u32 index) const
{
	return m_ComputeQueues[index];
}


BvCommandQueue* BvRenderDeviceVk::GetTransferQueue(const u32 index) const
{
	return m_TransferQueues[index];
}


void BvRenderDeviceVk::CreateVMA()
{
	VmaAllocatorCreateInfo vmaACI{};
	vmaACI.instance = m_pEngine->GetInstance();
	vmaACI.device = m_Device;
	vmaACI.physicalDevice = m_GPUInfo.m_PhysicalDevice;
	vmaACI.vulkanApiVersion = m_GPUInfo.m_DeviceProperties.apiVersion;

	if (m_GPUInfo.m_FeaturesSupported.bufferDeviceAddress)
	{
		vmaACI.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	}
	if (m_GPUInfo.m_FeaturesSupported.memoryBudget)
	{
		vmaACI.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
	}

	VmaVulkanFunctions functions{};
	functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
	functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
	functions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
	functions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
	functions.vkAllocateMemory = vkAllocateMemory;
	functions.vkFreeMemory = vkFreeMemory;
	functions.vkMapMemory = vkMapMemory;
	functions.vkUnmapMemory = vkUnmapMemory;
	functions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
	functions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
	functions.vkBindBufferMemory = vkBindBufferMemory;
	functions.vkBindImageMemory = vkBindImageMemory;
	functions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
	functions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
	functions.vkCreateBuffer = vkCreateBuffer;
	functions.vkDestroyBuffer = vkDestroyBuffer;
	functions.vkCreateImage = vkCreateImage;
	functions.vkDestroyImage = vkDestroyImage;
	functions.vkCmdCopyBuffer = vkCmdCopyBuffer;
#if VMA_DEDICATED_ALLOCATION || VMA_VULKAN_VERSION >= 1001000
	functions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
	functions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
#endif
#if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
	functions.vkBindBufferMemory2KHR = vkBindBufferMemory2KHR;
	functions.vkBindImageMemory2KHR = vkBindImageMemory2KHR;
#endif
#if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
	functions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR;
#endif
#if VMA_VULKAN_VERSION >= 1003000
	functions.vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements;
	functions.vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements;
#endif
	vmaACI.pVulkanFunctions = &functions;

	vmaCreateAllocator(&vmaACI, &m_VMA);
}


void BvRenderDeviceVk::DestroyVMA()
{
	vmaDestroyAllocator(m_VMA);
}