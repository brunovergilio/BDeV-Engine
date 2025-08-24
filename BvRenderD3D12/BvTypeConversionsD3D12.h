#pragma once


#include "BDeV/Core/RenderAPI/BvRenderCommon.h"
#include "BvCommonD3D12.h"
#include "BDeV/Core/RenderAPI/BvShaderResource.h"


D3D12_RESOURCE_FLAGS GetD3D12ResourceFlags(BufferUsage usage);
D3D12_RESOURCE_FLAGS GetD3D12ResourceFlags(TextureUsage usage);
CD3DX12_RESOURCE_DESC GetD3D12ResourceDesc(const TextureDesc& textureDesc);
D3D12_HEAP_TYPE GetD3D12HeapType(MemoryType memoryType);
D3D12_RESOURCE_STATES GetD3D12ResourceState(ResourceState resourceState);
D3D12_DESCRIPTOR_RANGE_TYPE GetD3D12DescriptorRangeType(ShaderResourceType type);
D3D12_SHADER_VISIBILITY GetD3D12ShaderVisibility(ShaderStage shaderStages);
CD3DX12_STATIC_SAMPLER_DESC GetD3D12StaticSamplerDesc(const SamplerDesc& samplerDesc, u32 binding = 0, u32 set = 0, D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL);
D3D12_FILTER GetD3D12Filter(const SamplerDesc& samplerDesc);
D3D12_TEXTURE_ADDRESS_MODE GetD3D12TextureAddressMode(AddressMode addressMode);
D3D12_COMPARISON_FUNC GetD3D12ComparisonFunc(CompareOp compareOp);
D3D12_STATIC_BORDER_COLOR GetD3D12StaticBorderColor(const SamplerDesc& samplerDesc);
D3D12_SAMPLER_DESC GetD3D12SamplerDesc(const SamplerDesc& samplerDesc);
D3D12_ROOT_SIGNATURE_FLAGS GetD3D12RootSignatureFlags(ShaderStage shaderStages);
D3D12_INPUT_CLASSIFICATION GetD3D12InputClassification(InputRate inputRate);
D3D12_BLEND GetD3D12Blend(BlendFactor blendFactor);
D3D12_BLEND_OP GetD3D12BlendOp(BlendOp blendOp);
D3D12_LOGIC_OP GetD3D12LogicOp(LogicOp logicOp);
D3D12_STENCIL_OP GetD3D12StencilOp(StencilOp stencilOp);
D3D12_FILL_MODE GetD3D12FillMode(FillMode fillMode);
D3D12_CULL_MODE GetD3D12CullMode(CullMode cullMode);
D3D12_COMMAND_LIST_TYPE GetD3D12CommandListType(CommandType commandType);
D3D_PRIMITIVE_TOPOLOGY GetD3D12PrimitiveTopology(Topology topology, u32 patchControlPoints = 0);
D3D12_PRIMITIVE_TOPOLOGY_TYPE GetD3D12PrimitiveTopologyType(Topology topology);