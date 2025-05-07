#pragma once


#include "BDeV/Core/RenderAPI/BvRenderCommon.h"
#include "BvCommonD3D12.h"


D3D12_RESOURCE_FLAGS GetD3D12ResourceFlags(BufferUsage usage);
D3D12_RESOURCE_FLAGS GetD3D12ResourceFlags(TextureUsage usage);
CD3DX12_RESOURCE_DESC GetD3D12ResourceDesc(const TextureDesc& textureDesc);
D3D12_HEAP_TYPE GetD3D12HeapType(MemoryType memoryType);
D3D12_RESOURCE_STATES GetD3D12ResourceState(ResourceState resourceState);