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

		bool hasTable = false;
		u32 tableRootIndex = 0;
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
				if (!hasTable)
				{
					hasTable = true;
					tableRootIndex = paramCount++;
				}

				m_RootSignatureMap.Emplace({ resource.m_Binding, set.m_Index }, tableRootIndex);
			}
		}
	}

	m_RootParams.Resize(paramCount);
	m_Ranges.Resize(rangeCount);
	m_RootParamsBindlessFlags.Resize(rangeCount, false);

	u32 rangeIndex = 0;
	u32 paramIndex = 0;
	u32 staticSamplerIndex = 0;
	ShaderStage allStages = ShaderStage::kUnknown;
	for (auto i = 0u; i < m_ShaderResourceLayoutDesc.m_ShaderResourceSets.Size(); ++i)
	{
		auto& set = m_ShaderResourceLayoutDesc.m_ShaderResourceSets[i];
		u32 currRangeIndex = rangeIndex;
		ShaderStage tableStages = ShaderStage::kUnknown;
		for (auto j = 0u; j < set.m_Constants.Size(); ++j)
		{
			auto& constant = set.m_Constants[j];
			allStages |= constant.m_ShaderStages;
			m_RootParams[paramIndex++].InitAsConstants(constant.m_Size >> 2, constant.m_Binding, set.m_Index, GetD3D12ShaderVisibility(constant.m_ShaderStages));
		}

		for (auto j = 0u; j < set.m_Resources.Size(); ++j)
		{
			auto& resource = set.m_Resources[j];
			if (resource.m_StaticSamplers.Size() > 0)
			{
				continue;
			}

			allStages |= resource.m_ShaderStages;
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
				m_Ranges[rangeIndex++] = CD3DX12_DESCRIPTOR_RANGE1(GetD3D12DescriptorRangeType(resource.m_ShaderResourceType), resource.m_Count, resource.m_Binding, set.m_Index,
					D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
				tableStages |= resource.m_ShaderStages;
			}
		}

		if (currRangeIndex != rangeIndex)
		{
			m_RootParamsBindlessFlags[paramIndex] = set.m_Type == ShaderResourceSetDesc::Type::kBindless;
			m_RootParams[paramIndex++].InitAsDescriptorTable(rangeIndex - currRangeIndex, &m_Ranges[currRangeIndex], GetD3D12ShaderVisibility(tableStages));
		}
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