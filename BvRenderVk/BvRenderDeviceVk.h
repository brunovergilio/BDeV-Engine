#pragma once


#include "BvRenderEngineVk.h"


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
class BvQueryVk;
class BvFramebufferManagerVk;
class BvCommandContextVk;
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


class BvRenderDeviceVk final : public BvRenderDevice
{
public:
	BvRenderDeviceVk(BvRenderEngineVk* pEngine, VkPhysicalDevice physicalDevice, const BvRenderDeviceCreateDescVk& deviceDesc);
	~BvRenderDeviceVk();

	BvSwapChain* CreateSwapChain(BvWindow* pWindow, const SwapChainDesc& swapChainDesc, BvCommandContext* pContext) override;
	BvBuffer* CreateBuffer(const BufferDesc& desc, const BufferInitData* pInitData = nullptr) override;
	BvBufferView* CreateBufferView(const BufferViewDesc& desc) override;
	BvTexture* CreateTexture(const TextureDesc& desc, const TextureInitData* pInitData = nullptr) override;
	BvTextureView* CreateTextureView(const TextureViewDesc& desc) override;
	BvSampler* CreateSampler(const SamplerDesc& desc) override;
	BvRenderPass* CreateRenderPass(const RenderPassDesc & renderPassDesc) override;
	BvShaderResourceLayout* CreateShaderResourceLayout(u32 shaderResourceCount,	const ShaderResourceDesc* pShaderResourceDescs,
		const ShaderResourceConstantDesc& shaderResourceConstantDesc) override;
	BvShader* CreateShader(const ShaderCreateDesc& shaderDesc) override;
	BvGraphicsPipelineState* CreateGraphicsPipeline(const GraphicsPipelineStateDesc & graphicsPipelineStateDesc) override;
	BvComputePipelineState* CreateComputePipeline(const ComputePipelineStateDesc& computePipelineStateDesc) override;
	BvQuery* CreateQuery(QueryType queryType) override;

	BvSwapChainVk* CreateSwapChainVk(BvWindow* pWindow, const SwapChainDesc& swapChainDesc, BvCommandContext* pContext);
	BvBufferVk* CreateBufferVk(const BufferDesc& desc, const BufferInitData* pInitData = nullptr);
	BvBufferViewVk* CreateBufferViewVk(const BufferViewDesc& desc);
	BvTextureVk* CreateTextureVk(const TextureDesc& desc, const TextureInitData* pInitData = nullptr);
	BvTextureViewVk* CreateTextureViewVk(const TextureViewDesc& desc);
	BvSamplerVk* CreateSamplerVk(const SamplerDesc& desc);
	BvRenderPassVk* CreateRenderPassVk(const RenderPassDesc& renderPassDesc);
	BvShaderResourceLayoutVk* CreateShaderResourceLayoutVk(u32 shaderResourceCount, const ShaderResourceDesc* pShaderResourceDescs,
		const ShaderResourceConstantDesc& shaderResourceConstantDesc);
	BvShaderVk* CreateShaderVk(const ShaderCreateDesc& shaderDesc);
	BvGraphicsPipelineStateVk* CreateGraphicsPipelineVk(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc);
	BvComputePipelineStateVk* CreateComputePipelineVk(const ComputePipelineStateDesc& computePipelineStateDesc);
	BvQueryVk* CreateQueryVk(QueryType queryType);

	void Release(IBvRenderDeviceChild* pDeviceObj) override;

	void WaitIdle() const override;

	const u32 GetMemoryTypeIndex(const u32 memoryTypeBits, const VkMemoryPropertyFlags properties) const;
	const VkFormat GetBestDepthFormat(const VkFormat format = VK_FORMAT_UNDEFINED) const;
	bool HasFormatSupport(Format format);

	BvCommandContext* GetGraphicsContext(u32 index = 0) const override;
	BvCommandContext* GetComputeContext(u32 index = 0) const override;
	BvCommandContext* GetTransferContext(u32 index = 0) const override;
	
	void GetCopyableFootprints(const TextureDesc& textureDesc, u32 subresourceCount, SubresourceFootprint* pSubresources, u64* pTotalSize) const override;
	bool SupportsQueryType(QueryType queryType, CommandType commandType) const override;
	bool IsFormatSupported(Format format) const override;
	BV_INLINE RenderDeviceCapabilities GetDeviceCaps() const override { return m_DeviceCaps; }

	BV_INLINE const VkDevice GetHandle() const { return m_Device; }
	BV_INLINE const VkPhysicalDevice GetPhysicalDeviceHandle() const { return m_PhysicalDevice; }
	BV_INLINE VkInstance GetInstanceHandle() const { return m_pEngine->GetHandle(); }
	BV_INLINE VmaAllocator GetAllocator() const { return m_VMA; }
	BV_INLINE BvFramebufferManagerVk* GetFramebufferManager() const { return m_pFramebufferManager; }
	BV_INLINE BvQueryHeapManagerVk* GetQueryHeapManager() const { return m_pQueryHeapManager; }
	BV_INLINE const BvDeviceInfoVk* GetDeviceInfo() const { return m_pDeviceInfo; }

private:
	void Create(const BvRenderDeviceCreateDescVk& deviceCreateDesc);
	void Destroy();

	void CreateVMA();
	void DestroyVMA();

private:
	BvRenderEngineVk* m_pEngine = nullptr;
	VkDevice m_Device = VK_NULL_HANDLE;
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	BvVector<BvCommandContextVk*> m_GraphicsContexts;
	BvVector<BvCommandContextVk*> m_ComputeContexts;
	BvVector<BvCommandContextVk*> m_TransferContexts;
	BvFramebufferManagerVk* m_pFramebufferManager;
	BvQueryHeapManagerVk* m_pQueryHeapManager;
	BvVector<IBvRenderDeviceChild*> m_DeviceObjects;
	VmaAllocator m_VMA{};
	BvDeviceInfoVk* m_pDeviceInfo = nullptr;
	RenderDeviceCapabilities m_DeviceCaps;
};


BV_CREATE_CAST_TO_VK(BvRenderDevice)