#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/RenderAPI/BvRenderDevice.h"
#include "BDeV/Core/Container/BvRobinSet.h"
#include "BDeV/Core/System/Threading/BvSync.h"


class BvCommandContextVk;


class BvRenderDeviceVk final : public IBvRenderDevice, public IBvResourceVk
{
public:
	BvRenderDeviceVk(VkInstance instance, VkPhysicalDevice physicalDevice, BvDeviceInfoVk* pDeviceInfo, u32 index, const BvGPUInfo& gpuInfo, const RenderDeviceDesc& renderDeviceDesc);

private:
	~BvRenderDeviceVk();

	bool CreateSwapChainImpl(BvWindow* pWindow, const SwapChainDesc& desc, IBvCommandContext* pContext, void** ppObj) override;
	bool CreateBufferImpl(const BufferDesc& desc, const BufferInitData* pInitData, void** ppObj) override;
	bool CreateBufferViewImpl(const BufferViewDesc& desc, void** ppObj) override;
	bool CreateTextureImpl(const TextureDesc& desc, const TextureInitData* pInitData, void** ppObj) override;
	bool CreateTextureViewImpl(const TextureViewDesc& desc, void** ppObj) override;
	bool CreateSamplerImpl(const SamplerDesc& desc, void** ppObj) override;
	bool CreateRenderPassImpl(const RenderPassDesc& renderPassDesc, void** ppObj) override;
	bool CreateShaderResourceLayoutImpl(const ShaderResourceLayoutCreateDesc& srlDesc, void** ppObj) override;
	bool CreateShaderImpl(const ShaderDesc& shaderDesc, void** ppObj) override;
	bool CreateGraphicsPipelineImpl(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc, IBvPipelineCache* pPipelineCache, void** ppObj) override;
	bool CreateComputePipelineImpl(const ComputePipelineStateDesc& computePipelineStateDesc, IBvPipelineCache* pPipelineCache, void** ppObj) override;
	bool CreateRayTracingPipelineImpl(const RayTracingPipelineStateDesc& rayTracingPipelineStateDesc, IBvPipelineCache* pPipelineCache, void** ppObj) override;
	bool CreateQueryHeapImpl(const QueryHeapDesc& queryHeapDesc, void** ppObj) override;
	bool CreateFenceImpl(const GPUFenceDesc& fenceDesc, void** ppObj) override;
	bool CreateAccelerationStructureImpl(const RayTracingAccelerationStructureDesc& asDesc, void** ppObj) override;
	bool CreateShaderBindingTableImpl(const ShaderBindingTableDesc& sbtDesc, IBvCommandContext* pContext, void** ppObj) override;
	bool CreateCommandContextImpl(const CommandContextDesc& commandContextDesc, void** ppObj) override;
	bool CreatePipelineCacheImpl(const PipelineCacheInitData* pInitData, void** ppObj) override;

public:
	void WaitIdle() const override;

	void GetCopyableFootprints(const TextureDesc& textureDesc, u32 subresourceCount, SubresourceFootprint* pSubresources, u64* pTotalSize,
		u64 baseOffset = 0, u64 firstSubresource = 0) const override;
	u64 GetDynamicBufferElementSize(BufferUsage usageFlags, u64 elementStride) const override;
	FormatFeatures GetFormatFeatures(Format format) const override;
	BV_INLINE const BvGPUInfo& GetGPUInfo() const override { return m_GPUInfo; }
	BV_INLINE const BvVector<Format>& GetSupportedDisplayFormats() const override { return m_SupportedDisplayFormats; }

	void OnVkHandleDestroyed(u64 handle, bool isTextureView);

	BV_INLINE VkDevice GetHandle() const { return m_Device; }
	BV_INLINE VkPhysicalDevice GetPhysicalDeviceHandle() const { return m_PhysicalDevice; }
	BV_INLINE VmaAllocator GetAllocator() const { return m_VMA; }
	BV_INLINE const BvDeviceInfoVk* GetDeviceInfo() const { return m_pDeviceInfo; }
	//BV_INLINE BvRenderEngineVk* GetEngine() const { return m_pEngine; }
	BV_INLINE bool IsValid() const { return m_Device != VK_NULL_HANDLE; }
	BV_INLINE operator VkDevice() const { return m_Device; }
	BV_INLINE VkInstance GetInstance() const { return m_Instance; }
	BV_INLINE u32 GetMaxSetsPerDescriptorPool() const { return m_MaxSetsPerDescriptorPool; }
	BV_INLINE const u32* GetQueryPoolSizes() const { return m_QueryPoolSizes; }
	BV_INLINE u32 GetAccelerationStructureQueryPoolSize() const { return m_AccelerationStructureQueryPoolSize; }

private:
	void Create(const RenderDeviceDesc& renderDeviceDesc);
	void Destroy();

	void CreateVMA();
	void DestroyVMA();
	void SetupSupportedDisplayFormats();

private:
	//BvRenderEngineVk* m_pEngine = nullptr;
	VkInstance m_Instance = VK_NULL_HANDLE;
	VkDevice m_Device = VK_NULL_HANDLE;
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	BvFixedVector<BvVector<BvCommandContextVk*>, kMaxContextGroupCount> m_Contexts;
	VmaAllocator m_VMA{};
	const BvGPUInfo& m_GPUInfo;
	BvDeviceInfoVk* m_pDeviceInfo = nullptr;
	BvVector<Format> m_SupportedDisplayFormats;
	u32 m_Index = 0;
	u32 m_MaxSetsPerDescriptorPool = 0;
	u32 m_QueryPoolSizes[kQueryTypeCount]{};
	u32 m_AccelerationStructureQueryPoolSize = 0;
};
BV_OBJECT_DEFINE_ID(BvRenderDeviceVk, "ec44c0fd-f4c4-4718-8c6b-5a56f9adc22e");
BV_CREATE_CAST_TO_VK(BvRenderDevice)