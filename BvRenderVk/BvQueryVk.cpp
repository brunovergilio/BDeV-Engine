#include "BvQueryVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvBufferVk.h"
#include "BvGPUFenceVk.h"
#include <bit>


BV_VK_DEVICE_RES_DEF(BvQueryVk)


BvQueryVk::BvQueryVk(BvRenderDeviceVk* pDevice, QueryType queryType, u32 frameCount)
	: m_pDevice(pDevice), m_QueryType(queryType), m_QueryData(frameCount, {})
{
}


BvQueryVk::~BvQueryVk()
{
}


bool BvQueryVk::GetResult(void* pData, u64 size)
{
	auto& queryData = m_QueryData[m_LatestResultIndex];
	if (!queryData.m_pFence || queryData.m_pFence->GetCompletedValue() < queryData.m_SignalValue)
	{
		return false;
	}

	return queryData.m_pQueryHeap ? queryData.m_pQueryHeap->GetResult(queryData, m_LatestResultIndex, pData, size) : false;
}


QueryDataVk* BvQueryVk::Allocate(BvQueryHeapManagerVk* pQueryHeapManager, u32 frameIndex)
{
	pQueryHeapManager->AllocateQuery(m_QueryType, frameIndex, m_QueryData[frameIndex]);

	return &m_QueryData[frameIndex];
}


void BvQueryVk::SetFenceData(BvGPUFenceVk* pFence, u64 value)
{
	m_QueryData[m_LatestResultIndex].m_pFence = pFence;
	m_QueryData[m_LatestResultIndex].m_SignalValue = value;
}


void BvQueryVk::SetLatestFrameIndex(u32 frameIndex)
{
	m_LatestResultIndex = (m_LatestResultIndex + 1) % m_QueryData.Size();
}


void BvQueryVk::Destroy()
{
}


BvQueryHeapVk::BvQueryHeapVk()
{
}


BvQueryHeapVk::BvQueryHeapVk(BvRenderDeviceVk* pDevice, QueryType queryType, u32 queryCount, u32 frameCount)
	: m_pDevice(pDevice), m_QueryCount(queryCount), m_FrameCount(frameCount), m_QueryType(queryType)
{
	auto pInfo = m_pDevice->GetDeviceInfo();

	switch (m_QueryType)
	{
	case QueryType::kTimestamp:
	case QueryType::kOcclusion:
	case QueryType::kOcclusionBinary:
		m_QuerySize = sizeof(u64);
		break;
	case QueryType::kPipelineStatistics:
	{
		m_PSOFlags = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT
			| VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT
			| VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT
			| (pInfo->m_DeviceFeatures.features.geometryShader ? (VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT
				| VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT) : 0)
			| VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT
			| VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT
			| VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT
			| (pInfo->m_DeviceFeatures.features.tessellationShader ? (VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT
				| VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT) : 0)
			| VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;

		m_QuerySize = sizeof(u64) * std::popcount(m_PSOFlags);
		break;
	}
	case QueryType::kMeshPipelineStatistics:
	{
		m_PSOFlags = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT
			| VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT
			| VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT
			| (pInfo->m_DeviceFeatures.features.geometryShader ? (VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT
				| VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT) : 0)
			| VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT
			| VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT
			| VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT
			| (pInfo->m_DeviceFeatures.features.tessellationShader ? (VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT
				| VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT) : 0)
			| VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT
			| (pInfo->m_ExtendedFeatures.meshShaderFeatures.meshShaderQueries ? (VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT
				| VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT) : 0);

		m_QuerySize = sizeof(u64) * std::popcount(m_PSOFlags) + (pInfo->m_ExtendedFeatures.meshShaderFeatures.meshShaderQueries ? sizeof(u64) : 0);
		break;
	}
	default:
		BV_ASSERT(false, "Query type not implemented yet");
		break;
	}
}


BvQueryHeapVk::BvQueryHeapVk(BvQueryHeapVk&& rhs) noexcept
{
	*this = std::move(rhs);
}


BvQueryHeapVk& BvQueryHeapVk::operator=(BvQueryHeapVk&& rhs) noexcept
{
	m_pDevice = rhs.m_pDevice;
	std::swap(m_QueryHeapData, rhs.m_QueryHeapData);
	m_QueryCount = rhs.m_QueryCount;
	m_FrameCount = rhs.m_FrameCount;
	m_QuerySize = rhs.m_QuerySize;
	m_PSOFlags = rhs.m_PSOFlags;
	m_QueryType = rhs.m_QueryType;

	return *this;
}


BvQueryHeapVk::~BvQueryHeapVk()
{
	Destroy();
}


void BvQueryHeapVk::Allocate(u32 frameIndex, QueryDataVk& queryData)
{
	u32 index = kU32Max;
	u32 heapIndex = 0;
	for (auto& heapData : m_QueryHeapData)
	{
		if (heapData.m_FrameAllocations[frameIndex] < m_QueryCount)
		{
			index = (m_QueryCount * frameIndex) + heapData.m_FrameAllocations[frameIndex]++;
			break;
		}

		heapIndex++;
	}

	if (index == kU32Max)
	{
		Create();
		auto& heapData = m_QueryHeapData[m_QueryHeapData.Size() - 1];
		index = (m_QueryCount * frameIndex) + heapData.m_FrameAllocations[frameIndex]++;
	}

	queryData.m_pQueryHeap = this;
	queryData.m_HeapIndex = heapIndex;
	queryData.m_QueryIndex = index;
}


void BvQueryHeapVk::Reset(u32 frameIndex)
{
	for (auto& heapData : m_QueryHeapData)
	{
		if (heapData.m_FrameAllocations[frameIndex] == 0)
		{
			continue;
		}

		for (auto pool : heapData.m_Pool)
		{
			if (pool != VK_NULL_HANDLE)
			{
				vkResetQueryPool(m_pDevice->GetHandle(), pool, frameIndex * m_QueryCount, heapData.m_FrameAllocations[frameIndex]);
			}
		}

		heapData.m_FrameAllocations[frameIndex] = 0;
	}
}


bool BvQueryHeapVk::GetResult(const QueryDataVk& queryData, u32 frameIndex, void* pData, u64 size)
{
	auto& heapData = m_QueryHeapData[queryData.m_HeapIndex];
	if (size < m_QuerySize)
	{
		return false;
	}

	heapData.m_pBuffer->Invalidate(size, 0);

	auto offset = m_QuerySize * queryData.m_QueryIndex;
	auto pResultData = reinterpret_cast<u64*>(reinterpret_cast<u8*>(heapData.m_pBuffer->GetMappedData()) + offset);
	
	switch (m_QueryType)
	{
	case QueryType::kTimestamp:
	case QueryType::kOcclusion:
	case QueryType::kOcclusionBinary:
	{
		auto pValue = reinterpret_cast<u64*>(pData);
		*pValue = *pResultData;
		break;
	}
	case QueryType::kPipelineStatistics:
	case QueryType::kMeshPipelineStatistics:
	{
		auto& psoStats = *reinterpret_cast<PipelineStatistics*>(pData);
		i32 currIndex = 0;
		psoStats.m_InputAssemblyVertices = pResultData[currIndex++];
		psoStats.m_InputAssemblyPrimitives = pResultData[currIndex++];
		psoStats.m_VertexShaderInvocations = pResultData[currIndex++];
		if (m_PSOFlags & (VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT | VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT))
		{
			psoStats.m_GeometryShaderInvocations = pResultData[currIndex++];
			psoStats.m_GeometryShaderPrimitives = pResultData[currIndex++];
		}
		psoStats.m_ClippingInvocations = pResultData[currIndex++];
		psoStats.m_ClippingPrimitives = pResultData[currIndex++];
		psoStats.m_PixelOrFragmentShaderInvocations = pResultData[currIndex++];
		if (m_PSOFlags & (VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT | VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT))
		{
			psoStats.m_HullOrControlShaderInvocations = pResultData[currIndex++];
			psoStats.m_DomainOrEvaluationShaderInvocations = pResultData[currIndex++];
		}
		psoStats.m_ComputeShaderInvocations = pResultData[currIndex++];
		if (m_PSOFlags & (VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT | VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT))
		{
			psoStats.m_TaskOrAmplificationShaderInvocations = pResultData[currIndex++];
			psoStats.m_MeshShaderInvocations = pResultData[currIndex++];
			psoStats.m_MeshShaderPrimitives = pResultData[currIndex++];
		}
		break;
	}
	}

	return true;
}


void BvQueryHeapVk::GetBufferInformation(u32 heapIndex, u32 frameIndex, u32 queryIndex, VkBuffer& buffer, u64& offset, u64& stride, u32 poolIndex)
{
	auto& heapData = m_QueryHeapData[heapIndex];
	auto frameOffset = m_QuerySize * queryIndex;
	if (m_PSOFlags == 0)
	{
		stride = m_QuerySize;
		offset = frameOffset;
	}
	else
	{
		if (poolIndex == 0)
		{
			stride = m_QuerySize - (heapData.m_Pool[1] != VK_NULL_HANDLE ? sizeof(u64) : 0);
			offset = frameOffset;
		}
		else if (poolIndex == 1)
		{
			BV_ASSERT(heapData.m_Pool[1] != VK_NULL_HANDLE, "Invalid mesh pool");
			stride = sizeof(u64);
			offset = frameOffset + (m_QuerySize - sizeof(u64));
		}
	}

	buffer = heapData.m_pBuffer->GetHandle();
}


void BvQueryHeapVk::Create()
{
	VkQueryPoolCreateInfo qpCI{ VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
	qpCI.queryCount = m_QueryCount * m_FrameCount;
	qpCI.queryType = GetVkQueryType(m_QueryType);
	if (m_QueryType == QueryType::kPipelineStatistics || m_QueryType == QueryType::kMeshPipelineStatistics)
	{
		qpCI.pipelineStatistics = m_PSOFlags;
	}

	VkQueryPool pool;
	auto result = vkCreateQueryPool(m_pDevice->GetHandle(), &qpCI, nullptr, &pool);
	if (result != VK_SUCCESS)
	{
		// TODO: Handle error
		return;
	}
	VkQueryPool meshPool = VK_NULL_HANDLE;
	if (m_QueryType == QueryType::kMeshPipelineStatistics &&
		(m_PSOFlags & (VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT | VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT)))
	{
		qpCI.queryType = VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT;
		result = vkCreateQueryPool(m_pDevice->GetHandle(), &qpCI, nullptr, &meshPool);
		if (result != VK_SUCCESS)
		{
			vkDestroyQueryPool(m_pDevice->GetHandle(), pool, nullptr);

			// TODO: Handle error
			return;
		}
	}

	BufferDesc bufferDesc;
	bufferDesc.m_Size = m_QuerySize * m_QueryCount * m_FrameCount;
	bufferDesc.m_MemoryType = MemoryType::kReadBack;
	bufferDesc.m_CreateFlags = BufferCreateFlags::kCreateMapped;
	BvRCRef<BvBufferVk> pBuffer;
	if (!(m_pDevice->CreateBuffer(bufferDesc, nullptr, &pBuffer)))
	{
		return;
	}

	vkResetQueryPool(m_pDevice->GetHandle(), pool, 0, m_QueryCount * m_FrameCount);
	if (meshPool != VK_NULL_HANDLE)
	{
		vkResetQueryPool(m_pDevice->GetHandle(), meshPool, 0, m_QueryCount * m_FrameCount);
	}

	m_QueryHeapData.EmplaceBack(BvQueryHeapVk::HeapData{ { pool, meshPool }, BvVector<u32>(m_FrameCount), pBuffer });
}


void BvQueryHeapVk::Destroy()
{
	for (auto& heapData : m_QueryHeapData)
	{
		for (auto& pool : heapData.m_Pool)
		{
			if (pool != VK_NULL_HANDLE)
			{
				vkDestroyQueryPool(m_pDevice->GetHandle(), pool, nullptr);
				pool = VK_NULL_HANDLE;
			}
		}

		if (heapData.m_pBuffer)
		{
			heapData.m_pBuffer->Release();
		}
	}
}


BvQueryHeapManagerVk::BvQueryHeapManagerVk(BvRenderDeviceVk* pDevice, const u32* pQuerySizes, u32 frameCount)
	: m_pDevice(pDevice)
{
	for (auto i = 0; i < kQueryTypeCount; ++i)
	{
		if (pQuerySizes[i])
		{
			m_QueryHeaps[i] = BvQueryHeapVk(m_pDevice, (QueryType)i, pQuerySizes[i], frameCount);
		}
	}
}


BvQueryHeapManagerVk::~BvQueryHeapManagerVk()
{
}


void BvQueryHeapManagerVk::AllocateQuery(QueryType queryType, u32 frameIndex, QueryDataVk& queryData)
{
	auto queryTypeIndex = (u32)queryType;
	m_QueryHeaps[queryTypeIndex].Allocate(frameIndex, queryData);
}


void BvQueryHeapManagerVk::Reset(u32 frameIndex)
{
	for (auto i = 0u; i < kQueryTypeCount; ++i)
	{
		m_QueryHeaps[i].Reset(frameIndex);
	}
}


BvQueryASVk::~BvQueryASVk()
{
	Destroy();
}


BvQueryASVk::BvQueryASVk(BvRenderDeviceVk* pDevice, u32 queryCount, u32 frameCount, VkQueryType queryType)
	: m_pDevice(pDevice), m_QueryCount(queryCount), m_FrameCount(frameCount), m_QueryType(queryType)
{
}


QueryVk BvQueryASVk::Allocate(u32 frameIndex)
{
	u32 index = kU32Max;
	u32 poolIndex = 0;
	u32 baseOffset = m_QueryCount * frameIndex;
	for (auto& pool : m_QueryPools)
	{
		if (pool.m_FrameAllocations[frameIndex] < m_QueryCount)
		{
			index = baseOffset + pool.m_FrameAllocations[frameIndex]++;
			break;
		}

		poolIndex++;
	}

	if (index == kU32Max)
	{
		Create();
		auto& poolData = m_QueryPools.Back();
		index = baseOffset + poolData.m_FrameAllocations[frameIndex]++;
	}

	return { m_QueryPools[poolIndex].m_Pool, index };
}


void BvQueryASVk::Reset(u32 frameIndex)
{
	for (auto& pool : m_QueryPools)
	{
		if (auto queriesToReset = pool.m_FrameAllocations[frameIndex])
		{
			vkResetQueryPool(m_pDevice->GetHandle(), pool.m_Pool, m_QueryCount * frameIndex, queriesToReset);
		}
	}
}


void BvQueryASVk::Create()
{
	VkQueryPoolCreateInfo qpCI{ VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
	qpCI.queryCount = m_QueryCount * m_FrameCount;
	qpCI.queryType = m_QueryType;

	VkQueryPool pool;
	auto result = vkCreateQueryPool(m_pDevice->GetHandle(), &qpCI, nullptr, &pool);
	if (result != VK_SUCCESS)
	{
		// TODO: Handle error
		return;
	}

	vkResetQueryPool(m_pDevice->GetHandle(), pool, 0, m_QueryCount * m_FrameCount);

	auto& poolData = m_QueryPools.EmplaceBack();
	poolData.m_Pool = pool;
	poolData.m_FrameAllocations.Resize(m_FrameCount, 0);
}


void BvQueryASVk::Destroy()
{
	for (auto pool: m_QueryPools)
	{
		vkDestroyQueryPool(m_pDevice->GetHandle(), pool.m_Pool, nullptr);
	}
	m_QueryPools.Clear();
}