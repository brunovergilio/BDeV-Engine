#pragma once

#include "BvRenderEngineD3D12.h"
#include "BDeV/Core/RenderAPI/BvRenderDevice.h"
#include "BDeV/Core/Container/BvRobinSet.h"


class BvSwapChainD3D12;
class BvBufferD3D12;
class BvBufferViewD3D12;
class BvTextureD3D12;
class BvTextureViewD3D12;
class BvSamplerD3D12;
class BvRenderPassD3D12;
class BvShaderResourceLayoutD3D12;
class BvShaderD3D12;
class BvGraphicsPipelineStateD3D12;
class BvComputePipelineStateD3D12;
class BvRayTracingPipelineStateD3D12;
class BvQueryD3D12;
class BvCommandContextD3D12;
class BvGPUFenceD3D12;
class BvAccelerationStructureD3D12;
class BvShaderBindingTableD3D12;
class BvGPUDescriptorHeapD3D12;
class BvCPUDescriptorHeapD3D12;


struct BvRenderDeviceCreateDescD3D12 : BvRenderDeviceCreateDesc
{
};


class BvRenderDeviceD3D12 final : public IBvRenderDevice, public IBvResourceD3D12
{
public:
	BvRenderDeviceD3D12(BvRenderEngineD3D12* pEngine, IDXGIAdapter1* pAdapter, BvDeviceInfoD3D12* pDeviceInfo, u32 index, const BvGPUInfo& gpuInfo, const BvRenderDeviceCreateDescD3D12& deviceDesc);
	~BvRenderDeviceD3D12();

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

	void WaitIdle() const override;

	void GetCopyableFootprints(const TextureDesc& textureDesc, u32 subresourceCount, SubresourceFootprint* pSubresources, u64* pTotalSize) const override;
	u64 GetDynamicBufferElementSize(BufferUsage usageFlags, u64 elementStride) const override;
	FormatFeatures GetFormatFeatures(Format format) const override;
	BV_INLINE const BvGPUInfo& GetGPUInfo() const override { return m_GPUInfo; }
	BV_INLINE const BvVector<Format>& GetSupportedDisplayFormats() const override { return m_SupportedDisplayFormats; }

	BV_INLINE ID3D12Device* GetHandle() const { return m_Device.Get(); }
	BV_INLINE IDXGIAdapter1* GetAdapterHandle() const { return m_pAdapter; }
	BV_INLINE D3D12MA::Allocator* GetAllocator() const { return m_Allocator.Get(); }
	BV_INLINE const BvDeviceInfoD3D12* GetDeviceInfo() const { return m_pDeviceInfo; }
	BV_INLINE BvRenderEngineD3D12* GetEngine() const { return m_pEngine; }
	BV_INLINE BvGPUDescriptorHeapD3D12* GetGPUShaderHeap() const { return m_pGPUShaderHeap; }
	BV_INLINE BvGPUDescriptorHeapD3D12* GetGPUSamplerHeap() const { return m_pGPUSamplerHeap; }
	BV_INLINE BvCPUDescriptorHeapD3D12* GetCPUShaderHeap() const { return m_pCPUShaderHeap; }
	BV_INLINE BvCPUDescriptorHeapD3D12* GetCPUSamplerHeap() const { return m_pCPUSamplerHeap; }

	BV_INLINE bool IsValid() const { return m_Device; }

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
	void Create(const BvRenderDeviceCreateDescD3D12& deviceCreateDesc);
	void Destroy();
	void SelfDestroy() override;

	void CreateAllocator();
	void DestroyAllocator();
	void CreateDescriptorHeaps();
	void SetupSupportedDisplayFormats();
	void CreateCommandSignatures();

private:
	BvRenderEngineD3D12* m_pEngine = nullptr;
	IDXGIAdapter1* m_pAdapter = nullptr;
	ComPtr<ID3D12Device> m_Device;
	BvVector<BvCommandContextD3D12*> m_GraphicsContexts;
	BvVector<BvCommandContextD3D12*> m_ComputeContexts;
	BvVector<BvCommandContextD3D12*> m_TransferContexts;
	BvRobinSet<IBvResourceD3D12*> m_DeviceObjects;
	BvGPUDescriptorHeapD3D12* m_pGPUShaderHeap = nullptr;
	BvGPUDescriptorHeapD3D12* m_pGPUSamplerHeap = nullptr;
	BvCPUDescriptorHeapD3D12* m_pCPUShaderHeap = nullptr;
	BvCPUDescriptorHeapD3D12* m_pCPUSamplerHeap = nullptr;
	ComPtr<D3D12MA::Allocator> m_Allocator = nullptr;
	ComPtr<ID3D12CommandSignature> m_DrawIndirectSig;
	ComPtr<ID3D12CommandSignature> m_DrawIndexedIndirectSig;
	ComPtr<ID3D12CommandSignature> m_DispatchIndirectSig;
	ComPtr<ID3D12CommandSignature> m_DispatchMeshIndirectSig;
	ComPtr<ID3D12CommandSignature> m_DispatchRaysIndirectSig;
	const BvGPUInfo& m_GPUInfo;
	BvDeviceInfoD3D12* m_pDeviceInfo = nullptr;
	RenderDeviceCapabilities m_DeviceCaps;
	BvVector<Format> m_SupportedDisplayFormats;
	u32 m_Index = 0;
};


BV_CREATE_CAST_TO_D3D12(BvRenderDevice)