#include "BvShaderResourceLayoutD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BvTypeConversionsD3D12.h"
#include "BDeV/Core/RenderAPI/BvSampler.h"


BV_D3D12_DEVICE_RES_DEF(BvShaderResourceLayoutD3D12)


BvShaderResourceLayoutD3D12::BvShaderResourceLayoutD3D12(BvRenderDeviceD3D12* pDevice, const ShaderResourceLayoutDesc& srlDesc)
	: m_ShaderResourceLayoutDesc(srlDesc), m_pDevice(pDevice)
{
	u32 totalResourceCount = 0;
	u32 totalSamplerCount = 0;
	u32 totalConstantCount = 0;
	// Count all the resources we have so we can make internal copies of them
	for (auto i = 0u; i < m_ShaderResourceLayoutDesc.m_ShaderResourceSetCount; ++i)
	{
		auto& set = m_ShaderResourceLayoutDesc.m_pShaderResourceSets[i];
		totalResourceCount += set.m_ResourceCount;
		for (auto j = 0u; j < set.m_ResourceCount; ++j)
		{
			auto& res = set.m_pResources[j];
			if (res.m_ppStaticSamplers != nullptr)
			{
				totalSamplerCount += res.m_Count;
			}
		}
		totalConstantCount += set.m_ConstantCount;
	}

	// Now we allocate space for the objects we need
	m_pShaderResourceSets = m_ShaderResourceLayoutDesc.m_ShaderResourceSetCount ?
		BV_NEW_ARRAY(ShaderResourceSetDesc, m_ShaderResourceLayoutDesc.m_ShaderResourceSetCount) : nullptr;
	m_pShaderResources = totalResourceCount > 0 ? BV_NEW_ARRAY(ShaderResourceDesc, totalResourceCount) : nullptr;
	m_ppStaticSamplers = totalSamplerCount > 0 ? BV_NEW_ARRAY(IBvSampler*, totalSamplerCount) : nullptr;
	m_pShaderConstants = totalConstantCount > 0 ? BV_NEW_ARRAY(ShaderResourceConstantDesc, totalConstantCount) : nullptr;

	// Start moving data
	u32 currResource = 0;
	u32 currSampler = 0;
	u32 currConstant = 0;

	for (auto i = 0u; i < m_ShaderResourceLayoutDesc.m_ShaderResourceSetCount; ++i)
	{
		auto& set = m_ShaderResourceLayoutDesc.m_pShaderResourceSets[i];
		u32 firstResource = currResource;
		for (auto j = 0u; j < set.m_ResourceCount; ++j)
		{
			auto& res = m_pShaderResources[currResource];
			res = set.m_pResources[j];

			if (res.m_ppStaticSamplers != nullptr)
			{
				memcpy(&m_ppStaticSamplers[currSampler], res.m_ppStaticSamplers, sizeof(IBvSampler*) * res.m_Count);
				res.m_ppStaticSamplers = &m_ppStaticSamplers[currSampler];
				currSampler += res.m_Count;
			}

			++currResource;
		}
		std::sort(m_pShaderResources + firstResource, m_pShaderResources + currResource);

		u32 firstConstant = currConstant;
		for (auto j = 0u; j < set.m_ConstantCount; ++j)
		{
			auto& constant = m_pShaderConstants[currConstant];
			constant = set.m_pConstants[j];
			++currConstant;
		}
		std::sort(m_pShaderConstants + firstConstant, m_pShaderConstants + currConstant);

		m_pShaderResourceSets[i] = set;
		m_pShaderResourceSets[i].m_pResources = &m_pShaderResources[firstResource];
	}
	std::sort(m_pShaderResourceSets, m_pShaderResourceSets + m_ShaderResourceLayoutDesc.m_ShaderResourceSetCount);

	m_ShaderResourceLayoutDesc.m_pShaderResourceSets = m_pShaderResourceSets;

	Create();
}


BvShaderResourceLayoutD3D12::~BvShaderResourceLayoutD3D12()
{
	Destroy();

	if (m_pShaderConstants)
	{
		BV_DELETE_ARRAY(m_pShaderConstants);
	}

	if (m_ppStaticSamplers)
	{
		BV_DELETE_ARRAY(m_ppStaticSamplers);
	}

	if (m_pShaderResources)
	{
		BV_DELETE_ARRAY(m_pShaderResources);
	}

	if (m_pShaderResourceSets)
	{
		BV_DELETE_ARRAY(m_ShaderResourceLayoutDesc.m_pShaderResourceSets);
	}
}


void BvShaderResourceLayoutD3D12::Create()
{
	BvVector<CD3DX12_ROOT_PARAMETER> params;
	BvVector<CD3DX12_DESCRIPTOR_RANGE> ranges;
	BvVector<CD3DX12_STATIC_SAMPLER_DESC> staticSamplers;

	{
		u32 paramCount = 0;
		u32 rangeCount = 0;
		u32 staticSamplerCount = 0;
		for (auto i = 0u; i < m_ShaderResourceLayoutDesc.m_ShaderResourceSetCount; ++i)
		{
			auto& set = m_ShaderResourceLayoutDesc.m_pShaderResourceSets[i];
			bool hasTable = false;
			for (auto j = 0u; j < set.m_ResourceCount; ++j)
			{
				auto& resource = set.m_pResources[j];
				if (resource.m_ppStaticSamplers != nullptr)
				{
					staticSamplerCount += resource.m_Count;
				}
				else if (!(resource.m_ShaderResourceType == ShaderResourceType::kDynamicConstantBuffer
					|| resource.m_ShaderResourceType == ShaderResourceType::kDynamicStructuredBuffer
					|| resource.m_ShaderResourceType == ShaderResourceType::kDynamicRWStructuredBuffer))
				{
					rangeCount++;
					if (!hasTable)
					{
						paramCount++;
						hasTable = true;
					}
				}
				else
				{
					paramCount++;
				}
			}
			paramCount += set.m_ConstantCount;
		}

		params.Resize(paramCount);
		ranges.Resize(rangeCount);
		staticSamplers.Resize(staticSamplerCount);
	}

	u32 rangeIndex = 0;
	u32 paramIndex = 0;
	u32 staticSamplerIndex = 0;
	ShaderStage allStages = ShaderStage::kUnknown;
	for (auto i = 0u; i < m_ShaderResourceLayoutDesc.m_ShaderResourceSetCount; ++i)
	{
		auto& set = m_ShaderResourceLayoutDesc.m_pShaderResourceSets[i];
		u32 currRangeIndex = rangeIndex;
		bool isBindful = false;
		bool isBindless = false;
		ShaderStage tableStages = ShaderStage::kUnknown;
		for (auto j = 0u; j < set.m_ConstantCount; ++j)
		{
			auto& constant = set.m_pConstants[j];
			allStages |= constant.m_ShaderStages;
			params[paramIndex++].InitAsConstants(constant.m_Size >> 2, constant.m_Binding, set.m_Index, GetD3D12ShaderVisibility(constant.m_ShaderStages));
		}

		for (auto j = 0u; j < set.m_ResourceCount; ++j)
		{
			auto& resource = set.m_pResources[j];
			allStages |= resource.m_ShaderStages;
			auto visibility = GetD3D12ShaderVisibility(resource.m_ShaderStages);
			if (resource.m_ppStaticSamplers != nullptr)
			{
				for (auto k = 0; k < resource.m_Count; ++k)
				{
					staticSamplers[staticSamplerIndex++] = GetD3D12StaticSamplerDesc(resource.m_ppStaticSamplers[k]->GetDesc(), resource.m_Binding,
						set.m_Index, visibility);
				}
			}
			else
			{
				if (resource.m_ShaderResourceType == ShaderResourceType::kDynamicConstantBuffer)
				{
					params[paramIndex++].InitAsConstantBufferView(resource.m_Binding, set.m_Index, visibility);
				}
				else if (resource.m_ShaderResourceType == ShaderResourceType::kDynamicStructuredBuffer)
				{
					params[paramIndex++].InitAsShaderResourceView(resource.m_Binding, set.m_Index, visibility);
				}
				else if (resource.m_ShaderResourceType == ShaderResourceType::kDynamicRWStructuredBuffer)
				{
					params[paramIndex++].InitAsUnorderedAccessView(resource.m_Binding, set.m_Index, visibility);
				}
				else
				{
					ranges[rangeIndex++] = CD3DX12_DESCRIPTOR_RANGE(GetD3D12DescriptorRangeType(resource.m_ShaderResourceType), resource.m_Count, resource.m_Binding, set.m_Index);
					tableStages |= resource.m_ShaderStages;
				}
			}

			if (resource.m_Bindless)
			{
				isBindless = true;
			}
			else
			{
				isBindful = true;
			}
		}

		if (isBindful && isBindless)
		{
			// I decided to keep these separate for the moment, as dealing with both on the same set would be dreadful
			BV_ASSERT(false, "Can't have bindful and bindless resources on the same set");
			return;
		}

		if (currRangeIndex != rangeIndex)
		{
			params[paramIndex++].InitAsDescriptorTable(rangeIndex - currRangeIndex, &ranges[currRangeIndex], GetD3D12ShaderVisibility(tableStages));
		}
	}

	D3D12_ROOT_SIGNATURE_FLAGS rootFlags = GetD3D12RootSignatureFlags(allStages);
	CD3DX12_ROOT_SIGNATURE_DESC desc(params.Size(), params.Data(), staticSamplers.Size(), staticSamplers.Data(), rootFlags);
	ComPtr<ID3DBlob> blob, errorBlob;
	auto hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &blob, &errorBlob);
	if (FAILED(hr))
	{
		// TODO: Handle error
		return;
	}

	hr = m_pDevice->GetHandle()->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&m_RootSig));
	if (FAILED(hr))
	{
		// TODO: Handle error
		return;
	}
}


void BvShaderResourceLayoutD3D12::Destroy()
{
}