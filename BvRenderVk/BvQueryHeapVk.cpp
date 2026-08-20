#include "BvQueryHeapVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvUtilsVk.h"
#include <bit>


BvQueryHeapVk::BvQueryHeapVk(BvRenderDeviceVk* pDevice, const QueryHeapDesc& queryHeapDesc, const VkQueryPool* pPools, VkQueryPipelineStatisticFlags psoFlags)
	: m_pDevice(pDevice), m_QueryHeapDesc(queryHeapDesc), m_QueryPools{ pPools[0], pPools[1] }, m_PSOFlags(psoFlags)
{
	auto pDeviceInfo = pDevice->GetDeviceInfo();

	switch (queryHeapDesc.m_Type)
	{
	case QueryType::kTimestamp:
	case QueryType::kOcclusion:
	case QueryType::kOcclusionBinary:
		m_QuerySize = sizeof(u64);
	case QueryType::kPipelineStatistics:
	{
		m_QuerySize = sizeof(u64) * std::popcount(psoFlags);
	}
	case QueryType::kMeshPipelineStatistics:
	{
		m_QuerySize = sizeof(u64) * std::popcount(psoFlags) + (pDeviceInfo->m_ExtendedFeatures.meshShaderFeatures.meshShaderQueries ? sizeof(u64) : 0);
	}
	}
}


BvQueryHeapVk::~BvQueryHeapVk()
{
	Destroy();
}


void BvQueryHeapVk::MapPSOStats(void* pDstData, u32 dstDataSize, void* pSrcData) const
{
	const bool isMesh = m_PSOFlags & VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT;

	u64* pDstValues = reinterpret_cast<u64*>(pDstData);
	const u64* pSrcValues = reinterpret_cast<u64*>(pSrcData);
	const u32 u64ValueCount = dstDataSize / 8;

	if (!isMesh)
	{
		const bool gs = m_PSOFlags & VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT;
		const bool ts = m_PSOFlags & VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT;

		for (auto dstIndex = 0, srcIndex = 0; dstIndex < u64ValueCount; dstIndex++, srcIndex++)
		{
			if (dstIndex == 3 && !gs)
			{
				dstIndex += 2;
			}
			else if (dstIndex == 8 && !ts)
			{
				dstIndex += 2;
			}

			pDstValues[dstIndex] = pSrcValues[srcIndex];
		}
	}
	else
	{
		pDstValues[7] = pSrcValues[0];
		pDstValues[11] = pSrcValues[1];
		pDstValues[12] = pSrcValues[2];
		pDstValues[13] = pSrcValues[3];
	}
}


void BvQueryHeapVk::Destroy()
{
	if (m_QueryPools[0])
	{
		VkHelpers::DestroyDeviceObject(*m_pDevice, m_QueryPools[0]);
		if (m_QueryPools[1])
		{
			VkHelpers::DestroyDeviceObject(*m_pDevice, m_QueryPools[1]);
		}
		m_QueryPools[0] = VK_NULL_HANDLE;
	}
}