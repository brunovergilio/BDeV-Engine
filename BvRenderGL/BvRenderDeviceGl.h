#pragma once


#include "BDeV/Core/RenderAPI/BvRenderDevice.h"
#include "BDeV/Core/Container/BvRobinSet.h"
#include "BDeV/Core/System/Threading/BvSync.h"
#include "BvContextGl.h"


class BvRenderEngineGl;
class BvCommandContextGl;
class BvContextGl;


BV_OBJECT_DEFINE_ID(BvRenderDeviceGl, "b632960a-c02f-4464-a336-b4d37b2c21f7");
class BvRenderDeviceGl final : public IBvRenderDevice, public IBvResourceGl
{
public:
	BvRenderDeviceGl(BvRenderEngineGl* pEngine, const BvGPUInfo& gpuInfo, const BvRenderDeviceCreateDesc& deviceDesc);

private:
	~BvRenderDeviceGl();

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

	BV_INLINE BvRenderEngineGl* GetEngine() const { return m_pEngine; }
	BV_INLINE bool IsValid() const { return m_MasterContext.IsValid(); }

	//BV_OBJECT_IMPL_INTERFACE(IBvRenderDeviceGl, IBvRenderDevice);

	template<typename T, typename... Args>
	BV_INLINE T* CreateResource(Args&&... args)
	{
		{
			BvScopedContextGl gl(&m_MasterContext, m_ContextLock);
			auto pObj = BV_NEW(T)(std::forward<Args>(args)...);
		}
		BvScopedLock lock(m_ResourceLock);
		m_DeviceObjects.Emplace(pObj);
		return pObj;
	}

	template<typename T>
	BV_INLINE void DestroyResource(T* pObj)
	{
		{
			BvScopedContextGl gl(&m_MasterContext);
			BV_DELETE(pObj);
		}
		BvScopedLock lock(m_ResourceLock);
		m_DeviceObjects.Erase(pObj);
	}

private:
	void Create(const BvRenderDeviceCreateDesc& deviceDesc);
	void Destroy();
	void SelfDestroy() override;

	void CreateVMA();
	void DestroyVMA();
	void SetupSupportedDisplayFormats();

private:
	BvRenderEngineGl* m_pEngine = nullptr;
	BvContextGl m_MasterContext;
	BvAdaptiveMutex m_ResourceLock;
	BvAdaptiveMutex m_ContextLock;
	BvVector<BvCommandContextGl*> m_CommandContexts;
	BvRobinSet<IBvResourceGl*> m_DeviceObjects;
	const BvGPUInfo& m_GPUInfo;
	BvVector<Format> m_SupportedDisplayFormats;
	u32 m_Index = 0;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvRenderDeviceGl);