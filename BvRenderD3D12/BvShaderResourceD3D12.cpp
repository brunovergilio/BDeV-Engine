#include "BvShaderResourceD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BvTypeConversionsD3D12.h"
#include "BDeV/Core/RenderAPI/BvSampler.h"


BvShaderResourceLayoutD3D12::BvShaderResourceLayoutD3D12(BvRenderDeviceD3D12* pDevice, const ShaderResourceLayoutCreateDesc& srlDesc)
	: m_ShaderResourceLayoutDesc(srlDesc), m_pDevice(pDevice)
{
	u32 paramCount = 0;
	u32 rangeCount = 0;
	for (auto i = 0u; i < m_ShaderResourceLayoutDesc.m_ShaderResourceSets.Size(); ++i)
	{
		auto& set = m_ShaderResourceLayoutDesc.m_ShaderResourceSets[i];
		for (auto j = 0u; j < set.m_Constants.Size(); ++j)
		{
			auto& constant = set.m_Constants[j];
			m_RootSignatureMap.Emplace({ constant.m_Binding, set.m_Index }, paramCount++);
		}

		u32 shaderTableRootIndex = kU32Max;
		u32 samplerTableRootIndex = kU32Max;
		for (auto j = 0u; j < set.m_Resources.Size(); ++j)
		{
			auto& resource = set.m_Resources[j];
			if (resource.m_StaticSamplers.Size() > 0)
			{
				continue;
			}

			if (resource.m_ShaderResourceType == ShaderResourceType::kDynamicConstantBuffer
				|| resource.m_ShaderResourceType == ShaderResourceType::kDynamicStructuredBuffer
				|| resource.m_ShaderResourceType == ShaderResourceType::kDynamicRWStructuredBuffer)
			{
				m_RootSignatureMap.Emplace({ resource.m_Binding, set.m_Index }, paramCount++);
			}
			else
			{
				rangeCount++;
				u32 tableRootIndex = kU32Max;

				// For each set, we may need 2 separate descriptor tables, one for CBV / SRV / UAV, and another for Samplers
				if (resource.m_ShaderResourceType != ShaderResourceType::kSampler)
				{
					if (shaderTableRootIndex == kU32Max)
					{
						shaderTableRootIndex = paramCount++;
					}
					tableRootIndex = shaderTableRootIndex;
				}
				else
				{
					if (samplerTableRootIndex == kU32Max)
					{
						samplerTableRootIndex = paramCount++;
					}
					tableRootIndex = samplerTableRootIndex;
				}

				m_RootSignatureMap.Emplace({ resource.m_Binding, set.m_Index }, tableRootIndex);
			}
		}
	}

	m_RootParams.Resize(paramCount);
	m_Ranges.Resize(rangeCount);
	m_RootParamsBindlessFlags.Resize(paramCount, false);

	BvVector<CD3DX12_DESCRIPTOR_RANGE1> shaderRanges; shaderRanges.Reserve(rangeCount);
	BvVector<CD3DX12_DESCRIPTOR_RANGE1> samplerRanges; samplerRanges.Reserve(rangeCount);

	u32 rangeIndex = 0;
	u32 paramIndex = 0;
	for (auto i = 0u; i < m_ShaderResourceLayoutDesc.m_ShaderResourceSets.Size(); ++i)
	{
		auto& set = m_ShaderResourceLayoutDesc.m_ShaderResourceSets[i];
		for (auto j = 0u; j < set.m_Constants.Size(); ++j)
		{
			auto& constant = set.m_Constants[j];
			m_RootParams[paramIndex++].InitAsConstants(constant.m_Size >> 2, constant.m_Binding, set.m_Index, GetD3D12ShaderVisibility(constant.m_ShaderStages));
		}

		u32 shaderParamIndex = kU32Max;
		u32 samplerParamIndex = kU32Max;
		ShaderStage shaderTableStages = ShaderStage::kUnknown;
		ShaderStage samplerTableStages = ShaderStage::kUnknown;
		for (auto j = 0u; j < set.m_Resources.Size(); ++j)
		{
			auto& resource = set.m_Resources[j];
			if (resource.m_StaticSamplers.Size() > 0)
			{
				continue;
			}

			auto visibility = GetD3D12ShaderVisibility(resource.m_ShaderStages);
			if (resource.m_ShaderResourceType == ShaderResourceType::kDynamicConstantBuffer)
			{
				m_RootParams[paramIndex++].InitAsConstantBufferView(resource.m_Binding, set.m_Index, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, visibility);
			}
			else if (resource.m_ShaderResourceType == ShaderResourceType::kDynamicStructuredBuffer)
			{
				m_RootParams[paramIndex++].InitAsShaderResourceView(resource.m_Binding, set.m_Index, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, visibility);
			}
			else if (resource.m_ShaderResourceType == ShaderResourceType::kDynamicRWStructuredBuffer)
			{
				m_RootParams[paramIndex++].InitAsUnorderedAccessView(resource.m_Binding, set.m_Index, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, visibility);
			}
			else
			{
				auto rangeType = GetD3D12DescriptorRangeType(resource.m_ShaderResourceType);
				auto tableParamIndex = kU32Max;
				if (rangeType != D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
				{
					if (shaderParamIndex == kU32Max)
					{
						shaderParamIndex = paramIndex++;
					}

					shaderTableStages |= resource.m_ShaderStages;
					tableParamIndex = shaderParamIndex;
					shaderRanges.EmplaceBack(rangeType, resource.m_Count, resource.m_Binding, set.m_Index, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
				}
				else
				{
					if (samplerParamIndex == kU32Max)
					{
						samplerParamIndex = paramIndex++;
					}

					samplerTableStages |= resource.m_ShaderStages;
					tableParamIndex = samplerParamIndex;
					samplerRanges.EmplaceBack(rangeType, resource.m_Count, resource.m_Binding, set.m_Index, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
				}

				m_RootParamsBindlessFlags[tableParamIndex] = set.m_Type == ShaderResourceSetDesc::Type::kBindless;
			}
		}

		auto moveRangesFn = [this, &rangeIndex](BvVector<CD3DX12_DESCRIPTOR_RANGE1>& src, u32 rootIndex, ShaderStage tableStages)
			{
				if (src.Size() == 0)
				{
					return;
				}

				auto currRangeIndex = rangeIndex;
				for (const auto& i : src)
				{
					m_Ranges[rangeIndex++] = i;
				}

				m_RootParams[rootIndex].InitAsDescriptorTable(rangeIndex - currRangeIndex, &m_Ranges[currRangeIndex], GetD3D12ShaderVisibility(tableStages));
			};

		if (shaderParamIndex < samplerParamIndex)
		{
			moveRangesFn(shaderRanges, shaderParamIndex, shaderTableStages);
			moveRangesFn(samplerRanges, samplerParamIndex, samplerTableStages);
		}
		else
		{
			moveRangesFn(samplerRanges, samplerParamIndex, samplerTableStages);
			moveRangesFn(shaderRanges, shaderParamIndex, shaderTableStages);
		}

		shaderRanges.Clear();
		samplerRanges.Clear();
	}
}


BvShaderResourceLayoutD3D12::~BvShaderResourceLayoutD3D12()
{
}


u32 BvShaderResourceLayoutD3D12::GetRootSignatureSlot(u32 binding, u32 set) const
{
	auto it = m_RootSignatureMap.FindKey({ binding, set });
	if (it != m_RootSignatureMap.cend())
	{
		return it->second;
	}

	return kU32Max;
}


void BvShaderResourceLayoutD3D12::Destroy()
{
}