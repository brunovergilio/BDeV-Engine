#pragma once


#include "BvCommonD3D12.h"
#include "BDeV/Core/Container/BvString.h"


class BvRenderDeviceD3D12;
class BvCommandContextD3D12;


namespace D3D12Utils
{
	template<typename T>
	using Obj = std::pair<HRESULT, T>;

	struct SwapChainObj
	{
		ComPtr<IDXGISwapChain3> m_SwapChain;
		BvVector<ID3D12Resource*> m_Textures;
		UINT m_PresentFlags;
	};

	struct BufferObj
	{
		ComPtr<ID3D12Resource> m_Buffer;
		ComPtr<D3D12MA::Allocation> m_Allocation;
		void* m_pMappedMemory;
	};

	struct TextureObj
	{
		ComPtr<ID3D12Resource> m_Texture;
		ComPtr<D3D12MA::Allocation> m_Allocation;
	};

	struct FenceObj
	{
		ComPtr<ID3D12Fence> m_Fence;
		HANDLE m_Event;
	};

	struct GraphicsPSOObj
	{
		ComPtr<ID3D12PipelineState> m_PSO;
		ComPtr<ID3D12RootSignature> m_RootSig;
		D3D_PRIMITIVE_TOPOLOGY m_PrimitiveTopology;
	};

	struct ComputePSOObj
	{
		ComPtr<ID3D12PipelineState> m_PSO;
		ComPtr<ID3D12RootSignature> m_RootSig;
	};

	struct RayTracingPSOObj
	{
		ComPtr<ID3D12StateObject> m_PSO;
		ComPtr<ID3D12RootSignature> m_RootSig;
		BvVector<BvWString> m_GroupNames;
	};

	struct ASObj
	{
		BufferObj m_Buffer;
		BvVector<D3D12_RAYTRACING_GEOMETRY_DESC> m_Geometries;
		RayTracingAccelerationStructureScratchSize m_ScratchSizes;
	};

	struct SBTObj
	{
		BufferObj m_Buffer;
		D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE m_Regions[u32(ShaderBindingTableGroupType::kCount)];
	};

	Obj<SwapChainObj> CreateSwapChainD3D12(BvRenderDeviceD3D12* pDevice, BvWindow* pWindow, SwapChainDesc& swapChainDesc, BvCommandContextD3D12* pContext);
	Obj<BufferObj> CreateBuffer(BvRenderDeviceD3D12* pDevice, const BufferDesc& bufferDesc, u64 minAlignment = 0);
	Obj<TextureObj> CreateTexture(BvRenderDeviceD3D12* pDevice, const TextureDesc& textureDesc, D3D12_RESOURCE_STATES initialState);
	Obj<FenceObj> CreateGPUFence(BvRenderDeviceD3D12* pDevice, const GPUFenceDesc& fenceDesc);
	Obj<GraphicsPSOObj> CreatePipelineState(BvRenderDeviceD3D12* pDevice, const GraphicsPipelineStateDesc& graphicsPipelineStateDesc, ID3D12PipelineLibrary1* pLibrary);
	Obj<ComputePSOObj> CreatePipelineState(BvRenderDeviceD3D12* pDevice, const ComputePipelineStateDesc& computePipelineStateDesc, ID3D12PipelineLibrary1* pLibrary);
	Obj<RayTracingPSOObj> CreatePipelineState(BvRenderDeviceD3D12* pDevice, const RayTracingPipelineStateDesc& rayTracingPipelineStateDesc);
	Obj<ComPtr<ID3D12PipelineLibrary1>> CreatePipelineLibrary(BvRenderDeviceD3D12* pDevice, const PipelineCacheInitData& initData);
	Obj<ASObj> CreateRayTracingAccelerationStructure(BvRenderDeviceD3D12* pDevice, const RayTracingAccelerationStructureDesc& asDesc);
	Obj<SBTObj> CreateShaderBindingTable(BvRenderDeviceD3D12* pDevice, const ShaderBindingTableDesc& sbtDesc, BvCommandContextD3D12* pContext);
	Obj<ComPtr<ID3D12QueryHeap>> CreateQueryHeap(BvRenderDeviceD3D12* pDevice, const QueryHeapDesc& queryHeapDesc);

	void UploadMemoryToGPU(BvRenderDeviceD3D12* pDevice, ID3D12Resource* pBuffer, const BufferInitData& initData);
	void UploadMemoryToGPU(BvRenderDeviceD3D12* pDevice, ID3D12Resource* pTexture, const TextureDesc& textureDesc, const TextureInitData& initData);
}