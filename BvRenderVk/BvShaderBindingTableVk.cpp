#include "BvShaderBindingTableVk.h"
#include "BvRenderDeviceVk.h"
#include "BvPipelineStateVk.h"
#include "BvCommandContextVk.h"


BV_VK_DEVICE_RES_DEF(BvShaderBindingTableVk)


BvShaderBindingTableVk::BvShaderBindingTableVk(BvRenderDeviceVk* pDevice, const ShaderBindingTableDesc& sbtDesc,
	const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& props, BvCommandContextVk* pContext)
	: m_SBTDesc(sbtDesc), m_pDevice(pDevice), m_HandleSize(props.shaderGroupHandleSize),
	m_GroupHandleAlignment(props.shaderGroupHandleAlignment), m_BaseGroupAlignment(props.shaderGroupBaseAlignment)
{
	Create(pContext);
}


BvShaderBindingTableVk::~BvShaderBindingTableVk()
{
	Destroy();
}


void BvShaderBindingTableVk::GetDeviceAddressRange(ShaderBindingTableGroupType type, u32 index, DeviceAddressRange& addressRange) const
{
	auto& region = m_Regions[u32(type)];
	addressRange.m_Address = region.deviceAddress + index * region.stride;
	addressRange.m_Size = region.size;
}


void BvShaderBindingTableVk::GetDeviceAddressRangeAndStride(ShaderBindingTableGroupType type, u32 index, DeviceAddressRangeAndStride& addressRangeAndStride) const
{
	auto& region = m_Regions[u32(type)];
	addressRangeAndStride.m_Address = region.deviceAddress + index * region.stride;
	addressRangeAndStride.m_Size = region.size;
	addressRangeAndStride.m_Stride = region.stride;
}


void BvShaderBindingTableVk::Create(BvCommandContextVk* pContext)
{
	auto& stages = TO_VK(m_SBTDesc.m_pPSO)->GetShaderStages();
	BvVector<u32> groupIndices[4];
	auto& psoDesc = m_SBTDesc.m_pPSO->GetDesc();
	for (auto g = 0; g < psoDesc.m_ShaderGroupCount; ++g)
	{
		auto index = 0;
		auto& currGroup = psoDesc.m_pShaderGroupDescs[g];
		if (currGroup.m_Type == ShaderGroupType::kGeneral)
		{
			auto stage = stages[currGroup.m_General];
			switch (stage)
			{
			case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
				index = 0;
				break;
			case VK_SHADER_STAGE_MISS_BIT_KHR:
				index = 1;
				break;
			case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
				index = 3;
				break;
			}
		}
		else
		{
			index = 2;
		}

		groupIndices[index].PushBack(g);
	}

	u32 sbtSize = psoDesc.m_ShaderGroupCount * m_HandleSize;
	BvVector<u8> shaderHandleData(sbtSize);

	auto result = vkGetRayTracingShaderGroupHandlesKHR(m_pDevice->GetHandle(), TO_VK(m_SBTDesc.m_pPSO)->GetHandle(), 0,
		psoDesc.m_ShaderGroupCount, sbtSize, shaderHandleData.Data());
	if (result != VK_SUCCESS)
	{
		return;
	}

	u64 handleSizeAligned = RoundToNearestPowerOf2(m_HandleSize, m_GroupHandleAlignment);
	m_Regions[0].stride = RoundToNearestPowerOf2(handleSizeAligned, m_BaseGroupAlignment);
	m_Regions[0].size = m_Regions[0].stride;
	m_Regions[1].stride = handleSizeAligned;
	m_Regions[1].size = RoundToNearestPowerOf2(handleSizeAligned * groupIndices[1].Size(), m_BaseGroupAlignment);
	m_Regions[2].stride = handleSizeAligned;
	m_Regions[2].size = RoundToNearestPowerOf2(handleSizeAligned * groupIndices[2].Size(), m_BaseGroupAlignment);
	m_Regions[3].stride = handleSizeAligned;
	m_Regions[3].size = RoundToNearestPowerOf2(handleSizeAligned * groupIndices[3].Size(), m_BaseGroupAlignment);

	BufferDesc bufferDesc;
	bufferDesc.m_UsageFlags = BufferUsage::kRayTracing;
	bufferDesc.m_Size = m_Regions[0].stride * groupIndices[0].Size() + m_Regions[1].size + m_Regions[2].size + m_Regions[3].size;
	
	BvVector<u8> bufferData(bufferDesc.m_Size);
	auto pStart = bufferData.Data();
	u64 currOffset = 0;
	for (auto g = 0u; g < 4; ++g)
	{
		auto pBuffer = pStart + currOffset;
		auto& group = groupIndices[g];
		auto stride = m_Regions[g].stride;
		for (auto i = 0u; i < group.Size(); ++i)
		{
			auto pSrc = shaderHandleData.Data() + (m_HandleSize * group[i]);
			memcpy(pBuffer, pSrc, m_HandleSize);
			pBuffer += stride;
		}
		currOffset += g == 0 ? m_Regions[g].stride * groupIndices[g].Size() : m_Regions[g].size;
	}

	BufferInitData initData;
	initData.m_pContext = pContext;
	initData.m_pData = bufferData.Data();
	initData.m_Size = bufferData.Size();

	m_Buffer = BvRCRaw(BV_NEW(BvBufferVk)(m_pDevice, bufferDesc, &initData));
	auto deviceAddress = m_Buffer->GetDeviceAddress();
	currOffset = 0;
	for (auto g = 0; g < 4; ++g)
	{
		if (groupIndices[g].Size() == 0)
		{
			m_Regions[g] = {};
			continue;
		}

		m_Regions[g].deviceAddress = deviceAddress + currOffset;
		currOffset += g == 0 ? m_Regions[g].stride * groupIndices[g].Size() : m_Regions[g].size;
	}
}


void BvShaderBindingTableVk::Destroy()
{
	m_Buffer.Reset();
}