#pragma once


#include "BvRenderVK/BvCommonVk.h"
#include "BDeV/Container/BvVector.h"
#include "BDeV/Container/BvString.h"


struct BvGPUInfoVk
{
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties m_DeviceProperties{};
	VkPhysicalDeviceFeatures m_DeviceFeatures{};
	VkPhysicalDeviceMemoryProperties m_DeviceMemoryProperties{};
	VkFormatProperties2 m_FormatProperties{ VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};
	struct
	{
		VkPhysicalDevicePointClippingPropertiesKHR pointClippingProps{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES_KHR };
		VkPhysicalDeviceMultiviewPropertiesKHR multiviewProps{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES_KHR };
		VkPhysicalDeviceDepthStencilResolvePropertiesKHR depthStencilResolveProps{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES_KHR };
		VkPhysicalDeviceFragmentShadingRatePropertiesKHR fragmentShadingRateProps{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR };
		VkPhysicalDeviceMaintenance3PropertiesKHR maintenance3Props{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES_KHR };
		VkPhysicalDeviceDescriptorIndexingPropertiesEXT descriptorIndexingProps{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES_EXT };
		VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProps{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR };
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProps{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR };
		VkPhysicalDeviceFloatControlsPropertiesKHR floatControlsProps{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES_KHR };
		VkPhysicalDeviceMeshShaderPropertiesEXT meshShaderProps{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT };
		VkPhysicalDeviceConservativeRasterizationPropertiesEXT convervativeRasterizationProps{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT };
		VkPhysicalDeviceCustomBorderColorPropertiesEXT customBorderColorProps{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_PROPERTIES_EXT };
		VkPhysicalDeviceTimelineSemaphorePropertiesKHR timelineSemaphoreProps{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES_KHR };
	} m_ExtendedProperties;
	struct
	{
		VkPhysicalDeviceMultiviewFeaturesKHR multiviewFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES_KHR };
		VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR };
		VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragmentShadingRateFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR };
		VkPhysicalDeviceDescriptorIndexingFeaturesEXT descriptorIndexingFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT };
		VkPhysicalDeviceBufferDeviceAddressFeaturesKHR bufferDeviceAddressFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR };
		VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR };
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR };
		VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR };
		VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT };
		VkPhysicalDeviceCustomBorderColorFeaturesEXT customBorderColorFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT };
		VkPhysicalDeviceTimelineSemaphoreFeaturesKHR timelineSemaphoreFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR };
		VkPhysicalDeviceImagelessFramebufferFeaturesKHR imagelessFrameBufferFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES_KHR };
	} m_ExtendedFeatures;
	BvVector<VkQueueFamilyProperties> m_QueueFamilyProperties;
	BvVector<VkExtensionProperties> m_SupportedExtensions;
	u32 m_GraphicsQueueIndex = UINT32_MAX;
	u32 m_ComputeQueueIndex = UINT32_MAX;
	u32 m_TransferQueueIndex = UINT32_MAX;
	u32 m_PresentationQueueIndex = UINT32_MAX;
	u32 m_DeviceVersion = 0;

	struct
	{
		bool swapChain : 1;
		bool deviceProperties2 : 1;
		bool memoryRequirements2 : 1;
		bool bindMemory2 : 1;
		bool maintenance1 : 1;
		bool maintenance2 : 1;
		bool multiview : 1;
		bool renderpass2 : 1;
		bool depthStencilResolve : 1;
		bool dynamicRendering : 1;
		bool fragmentShading : 1;
		bool maintenance3 : 1;
		bool descriptorIndexing : 1;
		bool bufferDeviceAddress : 1;
		bool deferredHostOperations : 1;
		bool shaderFloatControls : 1;
		bool spirv1_4 : 1;
		bool meshShader : 1;
		bool accelerationStructure : 1;
		bool rayTracingPipeline : 1;
		bool rayQuery : 1;
		bool pipelineLibrary : 1;
		bool conservativeRasterization : 1;
		bool customBorderColor : 1;
		bool timelineSemaphore : 1;
		bool imageFormatList : 1;
		bool imagelessFrameBuffer : 1;
		bool memoryBudget : 1;
	} m_FeaturesSupported{};

	inline u32 GetGPUVersion() { return m_DeviceVersion & (~VK_API_VERSION_PATCH(0xFFFFFFFF)); }
};