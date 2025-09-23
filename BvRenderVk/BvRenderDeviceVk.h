#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/RenderAPI/BvRenderDevice.h"
#include "BDeV/Core/Container/BvRobinSet.h"


class BvRenderEngineVk;
class BvCommandContextVk;


BV_OBJECT_DEFINE_ID(BvRenderDeviceVk, "ec44c0fd-f4c4-4718-8c6b-5a56f9adc22e");
class BvRenderDeviceVk final : public IBvRenderDevice, public IBvResourceVk
{
public:
	BvRenderDeviceVk(BvRenderEngineVk* pEngine, VkPhysicalDevice physicalDevice, BvDeviceInfoVk* pDeviceInfo, u32 index, const BvGPUInfo& gpuInfo, const BvRenderDeviceCreateDescVk& deviceDesc);

private:
	~BvRenderDeviceVk();

	bool CreateSwapChainImpl(BvWindow* pWindow, const SwapChainDesc& desc, IBvCommandContext* pContext, const BvUUID& objId, void** ppObj) override;
	bool CreateBufferImpl(const BufferDesc& desc, const BufferInitData* pInitData, const BvUUID& objId, void** ppObj) override;
	bool CreateBufferViewImpl(const BufferViewDesc& desc, const BvUUID& objId, void** ppObj) override;
	bool CreateTextureImpl(const TextureDesc& desc, const TextureInitData* pInitData, const BvUUID& objId, void** ppObj) override;
	bool CreateTextureViewImpl(const TextureViewDesc& desc, const BvUUID& objId, void** ppObj) override;
	bool CreateSamplerImpl(const SamplerDesc& desc, const BvUUID& objId, void** ppObj) override;
	bool CreateRenderPassImpl(const RenderPassDesc& renderPassDesc, const BvUUID& objId, void** ppObj) override;
	bool CreateShaderResourceLayoutImpl(const ShaderResourceLayoutDesc& srlDesc, const BvUUID& objId, void** ppObj) override;
	bool CreateShaderImpl(const ShaderCreateDesc& shaderDesc, const BvUUID& objId, void** ppObj) override;
	bool CreateGraphicsPipelineImpl(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc, const BvUUID& objId, void** ppObj) override;
	bool CreateComputePipelineImpl(const ComputePipelineStateDesc& computePipelineStateDesc, const BvUUID& objId, void** ppObj) override;
	bool CreateRayTracingPipelineImpl(const RayTracingPipelineStateDesc& rayTracingPipelineStateDesc, const BvUUID& objId, void** ppObj) override;
	bool CreateQueryImpl(QueryType queryType, const BvUUID& objId, void** ppObj) override;
	bool CreateFenceImpl(u64 value, const BvUUID& objId, void** ppObj) override;
	bool CreateAccelerationStructureImpl(const RayTracingAccelerationStructureDesc& asDesc, const BvUUID& objId, void** ppObj) override;
	bool CreateShaderBindingTableImpl(const ShaderBindingTableDesc& sbtDesc, IBvCommandContext* pContext, const BvUUID& objId, void** ppObj) override;
	bool CreateCommandContextImpl(const CommandContextDesc& commandContextDesc, const BvUUID& objId, void** ppObj) override;

public:
	void WaitIdle() const override;

	void GetCopyableFootprints(const TextureDesc& textureDesc, u32 subresourceCount, SubresourceFootprint* pSubresources, u64* pTotalSize) const override;
	u64 GetDynamicBufferElementSize(BufferUsage usageFlags, u64 elementStride) const override;
	FormatFeatures GetFormatFeatures(Format format) const override;
	BV_INLINE const BvGPUInfo& GetGPUInfo() const override { return m_GPUInfo; }
	BV_INLINE const BvVector<Format>& GetSupportedDisplayFormats() const override { return m_SupportedDisplayFormats; }

	BV_INLINE VkDevice GetHandle() const { return m_Device; }
	BV_INLINE VkPhysicalDevice GetPhysicalDeviceHandle() const { return m_PhysicalDevice; }
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
BV_OBJECT_ENABLE_ID_OPERATOR(BvRenderDeviceVk);


BV_CREATE_CAST_TO_VK(BvRenderDevice)