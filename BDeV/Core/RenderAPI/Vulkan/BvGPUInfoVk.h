#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/Container/BvString.h"


struct BvGPUInfoVk
{
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;

	VkPhysicalDeviceFeatures2 m_DeviceFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	VkPhysicalDeviceVulkan11Features m_DeviceFeatures1_1{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
	VkPhysicalDeviceVulkan12Features m_DeviceFeatures1_2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
	VkPhysicalDeviceVulkan13Features m_DeviceFeatures1_3{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
	VkPhysicalDeviceMemoryProperties2 m_DeviceMemoryProperties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2 };
	struct
	{
		VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragmentShadingRateFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR };
		VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR };
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR };
		VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR };
		VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT };
		VkPhysicalDeviceCustomBorderColorFeaturesEXT customBorderColorFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT };
		VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT vertexAttributeDivisorFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT };
		VkPhysicalDeviceConditionalRenderingFeaturesEXT conditionalRenderingFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT };
		VkPhysicalDeviceDepthClipEnableFeaturesEXT depthClibEnableFeature{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT };
	} m_ExtendedFeatures;

	VkPhysicalDeviceProperties2 m_DeviceProperties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
	VkPhysicalDeviceVulkan11Properties m_DeviceProperties1_1{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES };
	VkPhysicalDeviceVulkan12Properties m_DeviceProperties1_2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES };
	VkPhysicalDeviceVulkan13Properties m_DeviceProperties1_3{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES };
	struct
	{
		VkPhysicalDeviceFragmentShadingRatePropertiesKHR fragmentShadingRateProps{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR };
		VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProps{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR };
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProps{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR };
		VkPhysicalDeviceMeshShaderPropertiesEXT meshShaderProps{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT };
		VkPhysicalDeviceConservativeRasterizationPropertiesEXT convervativeRasterizationProps{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT };
		VkPhysicalDeviceCustomBorderColorPropertiesEXT customBorderColorProps{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_PROPERTIES_EXT };
		VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT vertexAttributeDivisorProperties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES_EXT };
		VkPhysicalDeviceSamplerFilterMinmaxProperties samplerFilterMinmaxProperties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES };
		VkPhysicalDeviceMemoryBudgetPropertiesEXT memoryBudgetProperties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT };
	} m_ExtendedProperties;

	BvVector<VkExtensionProperties> m_SupportedExtensions;
	BvVector<const char*> m_EnabledExtensions;

	BvVector<VkQueueFamilyProperties2> m_QueueFamilyProperties;
	BvVector<u32> m_QueueFamilyIndices;
	struct QueueInfo
	{
		u32 m_QueueFamilyIndex;
		u32 m_QueueCount;
		bool m_SupportsPresent;
	};
	QueueInfo m_GraphicsQueueInfo{};
	QueueInfo m_ComputeQueueInfo{};
	QueueInfo m_TransferQueueInfo{};
	u32 m_DeviceVersion = 0;

	struct
	{
		bool swapChain : 1;
		bool vertexAttributeDivisor : 1;
		bool fragmentShading : 1;
		bool meshShader : 1;
		bool accelerationStructure : 1;
		bool rayTracingPipeline : 1;
		bool rayQuery : 1;
		bool conservativeRasterization : 1;
		bool customBorderColor : 1;
		bool memoryBudget : 1;
		bool deferredHostOperations : 1;
		bool predication : 1;
		bool depthClipEnable : 1;
	} m_FeaturesSupported{};

	inline u32 GetGPUVersion() { return m_DeviceVersion & (~VK_API_VERSION_PATCH(0xFFFFFFFF)); }
};