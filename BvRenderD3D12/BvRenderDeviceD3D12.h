#pragma once

#include "BvRenderEngineD3D12.h"
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


struct BvRenderDeviceCreateDescD3D12 : BvRenderDeviceCreateDesc
{
};


class BvRenderDeviceD3D12 final : public IBvRenderDevice, public IBvResourceD3D12
{
public:
	BvRenderDeviceD3D12(BvRenderEngineD3D12* pEngine, IDXGIAdapter1* pAdapter, BvDeviceInfoD3D12* pDeviceInfo, u32 index, const BvGPUInfo& gpuInfo, const BvRenderDeviceCreateDescD3D12& deviceDesc);
	~BvRenderDeviceD3D12();

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
	BV_INLINE const BvVector<Format>& GetSupportedDisplayFormats() const override { return m_SupportedDisplayFormats; }

	BV_INLINE ID3D12Device* GetHandle() const { return m_Device.Get(); }
	BV_INLINE IDXGIAdapter1* GetAdapterHandle() const { return m_pAdapter; }
	BV_INLINE D3D12MA::Allocator* GetAllocator() const { return m_Allocator.Get(); }
	BV_INLINE const BvDeviceInfoD3D12* GetDeviceInfo() const { return m_pDeviceInfo; }
	BV_INLINE BvRenderEngineD3D12* GetEngine() const { return m_pEngine; }
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