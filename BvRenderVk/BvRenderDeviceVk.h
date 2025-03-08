#pragma once


#include "BvRenderEngineVk.h"


class IBvSwapChainVk;
class IBvBufferVk;
class IBvBufferViewVk;
class IBvTextureVk;
class IBvTextureViewVk;
class IBvSamplerVk;
class IBvRenderPassVk;
class IBvShaderResourceLayoutVk;
class IBvShaderVk;
class IBvGraphicsPipelineStateVk;
class IBvComputePipelineStateVk;
class IBvRayTracingPipelineStateVk;
class IBvQueryVk;
class IBvCommandContextVk;
class IBvGPUFenceVk;
class IBvAccelerationStructureVk;
class IBvShaderBindingTableVk;
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


BV_OBJECT_DEFINE_ID(IBvRenderDeviceVk, "ec44c0fd-f4c4-4718-8c6b-5a56f9adc22e");
class IBvRenderDeviceVk : public IBvRenderDevice
{
public:
	virtual bool CreateSwapChainVk(BvWindow* pWindow, const SwapChainDesc& swapChainDesc, IBvCommandContext* pContext, IBvSwapChainVk** ppObj) = 0;
	virtual bool CreateBufferVk(const BufferDesc& desc, const BufferInitData* pInitData, IBvBufferVk** ppObj) = 0;
	virtual bool CreateBufferViewVk(const BufferViewDesc& desc, IBvBufferViewVk** ppObj) = 0;
	virtual bool CreateTextureVk(const TextureDesc& desc, const TextureInitData* pInitData, IBvTextureVk** ppObj) = 0;
	virtual bool CreateTextureViewVk(const TextureViewDesc& desc, IBvTextureViewVk** ppObj) = 0;
	virtual bool CreateSamplerVk(const SamplerDesc& desc, IBvSamplerVk** ppObj) = 0;
	virtual bool CreateRenderPassVk(const RenderPassDesc& renderPassDesc, IBvRenderPassVk** ppObj) = 0;
	virtual bool CreateShaderResourceLayoutVk(const ShaderResourceLayoutDesc& srlDesc, IBvShaderResourceLayoutVk** ppObj) = 0;
	virtual bool CreateShaderVk(const ShaderCreateDesc& shaderDesc, IBvShaderVk** ppObj) = 0;
	virtual bool CreateGraphicsPipelineVk(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc, IBvGraphicsPipelineStateVk** ppObj) = 0;
	virtual bool CreateComputePipelineVk(const ComputePipelineStateDesc& computePipelineStateDesc, IBvComputePipelineStateVk** ppObj) = 0;
	virtual bool CreateRayTracingPipelineVk(const RayTracingPipelineStateDesc& rayTracingPipelineStateDesc, IBvRayTracingPipelineStateVk** ppObj) = 0;
	virtual bool CreateQueryVk(QueryType queryType, IBvQueryVk** ppObj) = 0;
	virtual bool CreateFenceVk(u64 value, IBvGPUFenceVk** ppObj) = 0;
	virtual bool CreateAccelerationStructureVk(const RayTracingAccelerationStructureDesc& asDesc, IBvAccelerationStructureVk** ppObj) = 0;
	virtual bool CreateShaderBindingTableVk(const ShaderBindingTableDesc& sbtDesc, IBvCommandContext* pContext, IBvShaderBindingTableVk** ppObj) = 0;

	virtual bool CreateGraphicsContextVk(u32 index, IBvCommandContextVk** ppObj) = 0;
	virtual bool CreateComputeContextVk(u32 index, IBvCommandContextVk** ppObj) = 0;
	virtual bool CreateTransferContextVk(u32 index, IBvCommandContextVk** ppObj) = 0;

	virtual IBvCommandContextVk* GetGraphicsContextVk(u32 index = 0) const = 0;
	virtual IBvCommandContextVk* GetComputeContextVk(u32 index = 0) const = 0;
	virtual IBvCommandContextVk* GetTransferContextVk(u32 index = 0) const = 0;

	virtual VkDevice GetHandle() const = 0;
	virtual VkPhysicalDevice GetPhysicalDeviceHandle() const = 0;
	virtual VkInstance GetInstanceHandle() const = 0;
	virtual VmaAllocator GetAllocator() const = 0;
	virtual const BvDeviceInfoVk* GetDeviceInfo() const = 0;
	virtual IBvRenderEngineVk* GetEngine() const = 0;
	virtual bool IsValid() const = 0;

protected:
	IBvRenderDeviceVk() {}
	~IBvRenderDeviceVk() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvRenderDeviceVk);


class BvRenderDeviceVk final : public IBvRenderDeviceVk
{
public:
	BvRenderDeviceVk(BvRenderEngineVk* pEngine, VkPhysicalDevice physicalDevice, const BvRenderDeviceCreateDescVk& deviceDesc);
	~BvRenderDeviceVk();

	bool CreateSwapChain(BvWindow* pWindow, const SwapChainDesc& swapChainDesc, IBvCommandContext* pContext, IBvSwapChain** ppObj) override;
	bool CreateBuffer(const BufferDesc& desc, const BufferInitData* pInitData, IBvBuffer** ppObj) override;
	bool CreateBufferView(const BufferViewDesc& desc, IBvBufferView** ppObj) override;
	bool CreateTexture(const TextureDesc& desc, const TextureInitData* pInitData, IBvTexture** ppObj) override;
	bool CreateTextureView(const TextureViewDesc& desc, IBvTextureView** ppObj) override;
	bool CreateSampler(const SamplerDesc& desc, IBvSampler** ppObj) override;
	bool CreateRenderPass(const RenderPassDesc& renderPassDesc, IBvRenderPass** ppObj) override;
	bool CreateShaderResourceLayout(const ShaderResourceLayoutDesc& srlDesc, IBvShaderResourceLayout** ppObj) override;
	bool CreateShader(const ShaderCreateDesc& shaderDesc, IBvShader** ppObj) override;
	bool CreateGraphicsPipeline(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc, IBvGraphicsPipelineState** ppObj) override;
	bool CreateComputePipeline(const ComputePipelineStateDesc& computePipelineStateDesc, IBvComputePipelineState** ppObj) override;
	bool CreateRayTracingPipeline(const RayTracingPipelineStateDesc& rayTracingPipelineStateDesc, IBvRayTracingPipelineState** ppObj) override;
	bool CreateQuery(QueryType queryType, IBvQuery** ppObj) override;
	bool CreateFence(u64 value, IBvGPUFence** ppObj) override;
	bool CreateAccelerationStructure(const RayTracingAccelerationStructureDesc& asDesc, IBvAccelerationStructure** ppObj) override;
	bool CreateShaderBindingTable(const ShaderBindingTableDesc& sbtDesc, IBvCommandContext* pContext, IBvShaderBindingTable** ppObj) override;

	bool CreateSwapChainVk(BvWindow* pWindow, const SwapChainDesc& swapChainDesc, IBvCommandContext* pContext, IBvSwapChainVk** ppObj) override;
	bool CreateBufferVk(const BufferDesc& desc, const BufferInitData* pInitData, IBvBufferVk** ppObj) override;
	bool CreateBufferViewVk(const BufferViewDesc& desc, IBvBufferViewVk** ppObj) override;
	bool CreateTextureVk(const TextureDesc& desc, const TextureInitData* pInitData, IBvTextureVk** ppObj) override;
	bool CreateTextureViewVk(const TextureViewDesc& desc, IBvTextureViewVk** ppObj) override;
	bool CreateSamplerVk(const SamplerDesc& desc, IBvSamplerVk** ppObj) override;
	bool CreateRenderPassVk(const RenderPassDesc& renderPassDesc, IBvRenderPassVk** ppObj) override;
	bool CreateShaderResourceLayoutVk(const ShaderResourceLayoutDesc& srlDesc, IBvShaderResourceLayoutVk** ppObj) override;
	bool CreateShaderVk(const ShaderCreateDesc& shaderDesc, IBvShaderVk** ppObj) override;
	bool CreateGraphicsPipelineVk(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc, IBvGraphicsPipelineStateVk** ppObj) override;
	bool CreateComputePipelineVk(const ComputePipelineStateDesc& computePipelineStateDesc, IBvComputePipelineStateVk** ppObj) override;
	bool CreateRayTracingPipelineVk(const RayTracingPipelineStateDesc& rayTracingPipelineStateDesc, IBvRayTracingPipelineStateVk** ppObj) override;
	bool CreateQueryVk(QueryType queryType, IBvQueryVk** ppObj) override;
	bool CreateFenceVk(u64 value, IBvGPUFenceVk** ppObj) override;
	bool CreateAccelerationStructureVk(const RayTracingAccelerationStructureDesc& asDesc, IBvAccelerationStructureVk** ppObj) override;
	bool CreateShaderBindingTableVk(const ShaderBindingTableDesc& sbtDesc, IBvCommandContext* pContext, IBvShaderBindingTableVk** ppObj) override;

	void WaitIdle() const override;

	bool CreateGraphicsContext(u32 index, IBvCommandContext** ppObj) override;
	bool CreateComputeContext(u32 index, IBvCommandContext** ppObj) override;
	bool CreateTransferContext(u32 index, IBvCommandContext** ppObj) override;

	bool CreateGraphicsContextVk(u32 index, IBvCommandContextVk** ppObj) override;
	bool CreateComputeContextVk(u32 index, IBvCommandContextVk** ppObj) override;
	bool CreateTransferContextVk(u32 index, IBvCommandContextVk** ppObj) override;

	IBvCommandContext* GetGraphicsContext(u32 index = 0) const override;
	IBvCommandContext* GetComputeContext(u32 index = 0) const override;
	IBvCommandContext* GetTransferContext(u32 index = 0) const override;

	IBvCommandContextVk* GetGraphicsContextVk(u32 index = 0) const override;
	IBvCommandContextVk* GetComputeContextVk(u32 index = 0) const override;
	IBvCommandContextVk* GetTransferContextVk(u32 index = 0) const override;

	void GetCopyableFootprints(const TextureDesc& textureDesc, u32 subresourceCount, SubresourceFootprint* pSubresources, u64* pTotalSize) const override;
	u64 GetDynamicBufferElementSize(BufferUsage usageFlags, u64 elementStride) const override;
	bool SupportsQueryType(QueryType queryType, CommandType commandType) const override;
	FormatFeatures GetFormatFeatures(Format format) const override;
	BV_INLINE RenderDeviceCapabilities GetDeviceCaps() const override { return m_DeviceCaps; }

	BV_INLINE VkDevice GetHandle() const override { return m_Device; }
	BV_INLINE VkPhysicalDevice GetPhysicalDeviceHandle() const  override { return m_PhysicalDevice; }
	BV_INLINE VkInstance GetInstanceHandle() const  override { return m_pEngine->GetHandle(); }
	BV_INLINE VmaAllocator GetAllocator() const  override { return m_VMA; }
	BV_INLINE const BvDeviceInfoVk* GetDeviceInfo() const override { return m_pDeviceInfo; }
	BV_INLINE BvRenderEngineVk* GetEngine() const  override { return m_pEngine; }
	BV_INLINE bool IsValid() const  override { return m_Device != VK_NULL_HANDLE; }

	BV_OBJECT_IMPL_INTERFACE(IBvRenderDeviceVk, IBvRenderDevice);

private:
	void Create(const BvRenderDeviceCreateDescVk& deviceCreateDesc);
	void Destroy();

	void CreateVMA();
	void DestroyVMA();

private:
	BvRenderEngineVk* m_pEngine = nullptr;
	VkDevice m_Device = VK_NULL_HANDLE;
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	BvVector<IBvCommandContextVk*> m_GraphicsContexts;
	BvVector<IBvCommandContextVk*> m_ComputeContexts;
	BvVector<IBvCommandContextVk*> m_TransferContexts;
	BvVector<IBvRenderDeviceObject*> m_DeviceObjects;
	VmaAllocator m_VMA{};
	BvDeviceInfoVk* m_pDeviceInfo = nullptr;
	RenderDeviceCapabilities m_DeviceCaps;
};


BV_CREATE_CAST_TO_VK(IBvRenderDevice)