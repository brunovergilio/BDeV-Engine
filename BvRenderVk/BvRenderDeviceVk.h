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


//BV_OBJECT_DEFINE_ID(IBvRenderDeviceVk, "ec44c0fd-f4c4-4718-8c6b-5a56f9adc22e");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvRenderDeviceVk);


class BvRenderDeviceVk final : public IBvRenderDevice, public IBvResourceVk
{
public:
	BvRenderDeviceVk(BvRenderEngineVk* pEngine, VkPhysicalDevice physicalDevice, BvDeviceInfoVk* pDeviceInfo, u32 index, const BvGPUInfo& gpuInfo, const BvRenderDeviceCreateDescVk& deviceDesc);
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
	IBvCommandContext* CreateCommandContextImpl(const CommandContextDesc& commandContextDesc) override;

	void WaitIdle() const override;

	void GetCopyableFootprints(const TextureDesc& textureDesc, u32 subresourceCount, SubresourceFootprint* pSubresources, u64* pTotalSize) const override;
	u64 GetDynamicBufferElementSize(BufferUsage usageFlags, u64 elementStride) const override;
	FormatFeatures GetFormatFeatures(Format format) const override;
	BV_INLINE const BvGPUInfo& GetGPUInfo() const override { return m_GPUInfo; }
	BV_INLINE const BvVector<Format>& GetSupportedDisplayFormats() const override { return m_SupportedDisplayFormats; }

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
	void SetupSupportedDisplayFormats();

private:
	BvRenderEngineVk* m_pEngine = nullptr;
	VkDevice m_Device = VK_NULL_HANDLE;
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	BvVector<BvCommandContextVk*> m_GraphicsContexts;
	BvVector<BvCommandContextVk*> m_ComputeContexts;
	BvVector<BvCommandContextVk*> m_TransferContexts;
	BvFixedVector<BvVector<BvCommandContextVk*>, kMaxContextGroupCount> m_Contexts;
	BvRobinSet<IBvResourceVk*> m_DeviceObjects;
	VmaAllocator m_VMA{};
	const BvGPUInfo& m_GPUInfo;
	BvDeviceInfoVk* m_pDeviceInfo = nullptr;
	BvVector<Format> m_SupportedDisplayFormats;
	u32 m_Index = 0;
};


BV_CREATE_CAST_TO_VK(BvRenderDevice)