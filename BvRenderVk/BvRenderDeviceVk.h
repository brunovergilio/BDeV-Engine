#pragma once


#include "BvRenderEngineVk.h"
#include "BDeV/Core/Container/BvRobinSet.h"


class BvSwapChainVk;
class BvBufferVk;
class BvBufferViewVk;
class BvTextureVk;
class BvTextureViewVk;
class BvSamplerVk;
class BvRenderPassVk;
class BvShaderResourceLayoutVk;
class BvShaderVk;
class BvGraphicsPipelineStateVk;
class BvComputePipelineStateVk;
class BvRayTracingPipelineStateVk;
class BvQueryVk;
class BvCommandContextVk;
class BvGPUFenceVk;
class BvAccelerationStructureVk;
class BvShaderBindingTableVk;
class BvQueryHeapManagerVk;


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
	struct QueueInfo
	{
		u32 m_QueueFamilyIndex;
		u32 m_QueueCount;
		bool m_SupportsPresent;
	};
	QueueInfo m_GraphicsQueueInfo{};
	QueueInfo m_ComputeQueueInfo{};
	QueueInfo m_TransferQueueInfo{};
};


//BV_OBJECT_DEFINE_ID(IBvRenderDeviceVk, "ec44c0fd-f4c4-4718-8c6b-5a56f9adc22e");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvRenderDeviceVk);


class BvRenderDeviceVk final : public IBvRenderDevice, public IBvResourceVk
{
public:
	BvRenderDeviceVk(BvRenderEngineVk* pEngine, VkPhysicalDevice physicalDevice, u32 index, const BvGPUInfo& gpuInfo, const BvRenderDeviceCreateDescVk& deviceDesc);
	~BvRenderDeviceVk();

	IBvSwapChain* CreateSwapChainImpl(BvWindow* pWindow, const SwapChainDesc& swapChainDesc, IBvCommandContext* pContext) override;
	IBvBuffer* CreateBufferImpl(const BufferDesc& desc, const BufferInitData* pInitData) override;
	IBvBufferView* CreateBufferViewImpl(const BufferViewDesc& desc) override;
	IBvTexture* CreateTextureImpl(const TextureDesc& desc, const TextureInitData* pInitData) override;
	IBvTextureView* CreateTextureViewImpl(const TextureViewDesc& desc) override;
	IBvSampler* CreateSamplerImpl(const SamplerDesc& desc) override;
	IBvRenderPass* CreateRenderPassImpl(const RenderPassDesc& renderPassDesc) override;
	IBvShaderResourceLayout* CreateShaderResourceLayoutImpl(const ShaderResourceLayoutDesc& srlDesc) override;
	IBvShader* CreateShaderImpl(const ShaderCreateDesc& shaderDesc) override;
	IBvGraphicsPipelineState* CreateGraphicsPipelineImpl(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc) override;
	IBvComputePipelineState* CreateComputePipelineImpl(const ComputePipelineStateDesc& computePipelineStateDesc) override;
	IBvRayTracingPipelineState* CreateRayTracingPipelineImpl(const RayTracingPipelineStateDesc& rayTracingPipelineStateDesc) override;
	IBvQuery* CreateQueryImpl(QueryType queryType) override;
	IBvGPUFence* CreateFenceImpl(u64 value) override;
	IBvAccelerationStructure* CreateAccelerationStructureImpl(const RayTracingAccelerationStructureDesc& asDesc) override;
	IBvShaderBindingTable* CreateShaderBindingTableImpl(const ShaderBindingTableDesc& sbtDesc, IBvCommandContext* pContext) override;
	IBvCommandContext* GetGraphicsContextImpl(u32 index = 0) override;
	IBvCommandContext* GetComputeContextImpl(u32 index = 0) override;
	IBvCommandContext* GetTransferContextImpl(u32 index = 0) override;

	void WaitIdle() const override;

	void GetCopyableFootprints(const TextureDesc& textureDesc, u32 subresourceCount, SubresourceFootprint* pSubresources, u64* pTotalSize) const override;
	u64 GetDynamicBufferElementSize(BufferUsage usageFlags, u64 elementStride) const override;
	bool SupportsQueryType(QueryType queryType, CommandType commandType) const override;
	FormatFeatures GetFormatFeatures(Format format) const override;
	BV_INLINE RenderDeviceCapabilities GetDeviceCaps() const override { return m_DeviceCaps; }
	BV_INLINE const BvGPUInfo& GetGPUInfo() const override { return m_GPUInfo; }

	BV_INLINE VkDevice GetHandle() const { return m_Device; }
	BV_INLINE VkPhysicalDevice GetPhysicalDeviceHandle() const { return m_PhysicalDevice; }
	BV_INLINE VkInstance GetInstanceHandle() const { return m_pEngine->GetHandle(); }
	BV_INLINE VmaAllocator GetAllocator() const { return m_VMA; }
	BV_INLINE const BvDeviceInfoVk* GetDeviceInfo() const { return m_pDeviceInfo; }
	BV_INLINE BvRenderEngineVk* GetEngine() const { return m_pEngine; }
	BV_INLINE bool IsValid() const { return m_Device != VK_NULL_HANDLE; }

	//BV_OBJECT_IMPL_INTERFACE(IBvRenderDeviceVk, IBvRenderDevice);

	template<typename T, typename... Args>
	BV_INLINE T* CreateResource(Args&&... args)
	{
		auto pObj = BV_NEW(T)(std::forward<Args>(args)...);
		m_DeviceObjects.Emplace(pObj);
		return pObj;
	}

	template<typename T>
	BV_INLINE void DestroyResource(T* pObj)
	{
		m_DeviceObjects.Erase(pObj);
		BV_DELETE(pObj);
	}

private:
	void Create(const BvRenderDeviceCreateDescVk& deviceCreateDesc);
	void Destroy();
	void SelfDestroy() override;

	void CreateVMA();
	void DestroyVMA();

private:
	BvRenderEngineVk* m_pEngine = nullptr;
	VkDevice m_Device = VK_NULL_HANDLE;
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	BvVector<BvCommandContextVk*> m_GraphicsContexts;
	BvVector<BvCommandContextVk*> m_ComputeContexts;
	BvVector<BvCommandContextVk*> m_TransferContexts;
	BvRobinSet<IBvResourceVk*> m_DeviceObjects;
	VmaAllocator m_VMA{};
	const BvGPUInfo& m_GPUInfo;
	BvDeviceInfoVk* m_pDeviceInfo = nullptr;
	RenderDeviceCapabilities m_DeviceCaps;
	u32 m_Index = 0;
};


BV_CREATE_CAST_TO_VK(BvRenderDevice)