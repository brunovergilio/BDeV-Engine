#pragma once


#include "BDeV/Core/RenderAPI/BvRenderCommon.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"

// Platform-dependent stuff
#if BV_PLATFORM_WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#define BV_VULKAN_DLL_NAME "vulkan-1.dll"
#else
	// Other platforms
#endif // #if BV_PLATFORM_WIN32


// Defines for the Vulkan Memory Allocator so it uses functions loaded from Volk
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0


#include "Third Party/volk.h"
#include "Third Party/vk_mem_alloc.h"


// Have to re-include this again because the CreateSemaphore macro
// conflicts with the BvRenderEngine::CreateSemaphore function
#if BV_PLATFORM_WIN32
#include "BDeV/Core/System/BvPlatformHeaders.h"
#endif // #if BV_PLATFORM_WIN32


#define BV_CREATE_CAST_TO_VK(Type) namespace Internal \
{ \
BV_INLINE Type##Vk* ToVk(I##Type* pObj) { return static_cast<Type##Vk*>(pObj); } \
BV_INLINE const Type##Vk* ToVk(const I##Type* pObj) { return static_cast<const Type##Vk*>(pObj); } \
}

#define TO_VK(pObj) Internal::ToVk(pObj)


struct BvDeviceInfoVk
{
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
		VkPhysicalDeviceRobustness2FeaturesEXT robustness2Features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT };
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
		VkPhysicalDeviceMultiviewProperties multiviewProperties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES };
	} m_ExtendedProperties;

	struct
	{
		bool hasSurface2Caps = false;
		VkSurfaceCapabilitiesFullScreenExclusiveEXT fullScreenExclusiveCaps{ VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_FULL_SCREEN_EXCLUSIVE_EXT };
#if BV_PLATFORM_WIN32
		VkSurfaceFullScreenExclusiveWin32InfoEXT fullScreenExclusiveInfoWin32{ VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT };
#endif
	} m_ExtendedSurfaceCaps;

	struct 
	{
		bool vertexAttributeDivisor : 1;
		bool fragmentShading : 1;
		bool meshShader : 1;
		bool accelerationStructure : 1;
		bool rayTracingPipeline : 1;
		bool rayTracingMaintenance : 1;
		bool rayQuery : 1;
		bool conservativeRasterization : 1;
		bool customBorderColor : 1;
		bool memoryBudget : 1;
		bool deferredHostOperations : 1;
		bool deviceAddress : 1;
		bool predication : 1;
		bool depthClipEnable : 1;
		bool trueFullScreen : 1;
		bool globalQueuePriority : 1;
		bool depthBiasControl : 1;
		bool robustness2 : 1;
		bool pushDescriptor : 1;
		bool depthStencilResolve : 1;
	} m_FeatureFlags;

	bool m_HasDebugUtils = false;

	BvVector<VkExtensionProperties> m_SupportedExtensions;
	BvVector<const char*> m_EnabledExtensions;

	BvVector<VkQueueFamilyProperties2> m_QueueFamilyProperties;
	BvVector<u32> m_QueueFamilyIndices{};
};


class IBvResourceVk
{
public:
	virtual void Destroy() = 0;

protected:
	IBvResourceVk() {}
	virtual ~IBvResourceVk() {}
};