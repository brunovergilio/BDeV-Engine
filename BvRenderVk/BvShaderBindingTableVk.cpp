#include "BvShaderBindingTableVk.h"
#include "BvRenderDeviceVk.h"
#include "BvPipelineStateVk.h"
#include "BvBufferVk.h"
#include "BvCommandContextVk.h"


BvShaderBindingTableVk::BvShaderBindingTableVk(IBvRenderDeviceVk* pDevice, const ShaderBindingTableDesc& sbtDesc,
	const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& props, IBvCommandContextVk* pContext)
	: m_SBTDesc(sbtDesc), m_pDevice(pDevice), m_HandleSize(props.shaderGroupHandleSize),
	m_GroupHandleAlignment(props.shaderGroupHandleAlignment), m_BaseGroupAlignment(props.shaderGroupBaseAlignment)
{
	Create(pContext);
}


BvShaderBindingTableVk::~BvShaderBindingTableVk()
{
	Destroy();
}


IBvRenderDevice* BvShaderBindingTableVk::GetDevice()
{
	return m_pDevice;
}


void BvShaderBindingTableVk::GetAddressRegion(ShaderBindingTableGroupType groupType, u32 index, VkStridedDeviceAddressRegionKHR& addressRegion) const
{
	addressRegion = m_Regions[u32(groupType)];
	addressRegion.deviceAddress += addressRegion.stride * index;
}


void BvShaderBindingTableVk::Create(IBvCommandContextVk* pContext)
{
	BvVector<u32> groupIndices[4];
	auto& psoDesc = m_SBTDesc.m_pPSO->GetDesc();
	for (auto i = 0; i < psoDesc.m_ShaderGroupCount; ++i)
	{
		auto index = 0;
		auto& currGroup = psoDesc.m_pShaderGroupDescs[i];
		if (currGroup.m_Type == ShaderGroupType::kGeneral)
		{
			auto pShader = psoDesc.m_ppShaders[currGroup.m_General];
			switch (pShader->GetShaderStage())
			{
			case ShaderStage::kRayGen:
				index = 0;
				break;
			case ShaderStage::kMiss:
				index = 1;
				break;
			case ShaderStage::kCallable:
				index = 3;
				break;
			}
		}
		else
		{
			index = 2;
		}

		groupIndices[index].PushBack(i);
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
	bufferDesc.m_Size = m_Regions[0].size * groupIndices[0].Size() + m_Regions[1].size + m_Regions[2].size + m_Regions[3].size;
	
	BvVector<u8> bufferData(bufferDesc.m_Size);
	auto pBuffer = bufferData.Data();
	for (auto g = 0u; g < 4; ++g)
	{
		auto& group = groupIndices[g];
		auto stride = m_Regions[g].stride;
		for (auto i = 0u; i < group.Size(); ++i)
		{
			auto pSrc = shaderHandleData.Data() + (m_HandleSize * group[i]);
			memcpy(pBuffer, pSrc, m_HandleSize);
			pBuffer += stride;
		}
	}

	BufferInitData initData;
	initData.m_pContext = pContext;
	initData.m_pData = bufferData.Data();
	initData.m_Size = bufferData.Size();

	m_pDevice->CreateBufferVk(bufferDesc, &initData, &m_pBuffer);
	auto deviceAddress = m_pBuffer->GetDeviceAddress();
	m_Regions[0].deviceAddress = deviceAddress;
	m_Regions[1].deviceAddress = deviceAddress + m_Regions[0].size * groupIndices[0].Size();
	m_Regions[2].deviceAddress = deviceAddress + m_Regions[0].size + m_Regions[1].size;
	m_Regions[3].deviceAddress = deviceAddress + m_Regions[0].size + m_Regions[1].size + m_Regions[2].size;
}


void BvShaderBindingTableVk::Destroy()
{
	if (m_pBuffer)
	{
		m_pBuffer->Release();
		m_pBuffer = nullptr;
	}
}