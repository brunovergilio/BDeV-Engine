#include "BvQueryVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvBufferVk.h"
#include "BvGPUFenceVk.h"


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
	switch (m_QueryType)
	{
	case QueryType::kTimestamp:
	case QueryType::kOcclusion:
	case QueryType::kOcclusionBinary:
		m_QuerySize = sizeof(u64);
		break;
	case QueryType::kPipelineStatistics:
		m_QuerySize = sizeof(PipelineStatistics) - (m_pDevice->GetDeviceInfo()->m_ExtendedFeatures.meshShaderFeatures.meshShaderQueries ? 0 : sizeof(u64) * 3);
		break;
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
	m_QueryType = rhs.m_QueryType;
	m_QuerySize = rhs.m_QuerySize;

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

	auto offset = m_QuerySize * queryData.m_QueryIndex;
	auto pResultData = reinterpret_cast<u8*>(heapData.m_pBuffer->GetMappedData()) + offset;

	memcpy(pData, pResultData, m_QuerySize);

	return true;
}


void BvQueryHeapVk::GetBufferInformation(u32 heapIndex, u32 frameIndex, u32 queryIndex, VkBuffer& buffer, u64& offset, u64& stride, u32 poolIndex)
{
	auto& heapData = m_QueryHeapData[heapIndex];
	if (m_QueryType != QueryType::kPipelineStatistics)
	{
		stride = m_QuerySize;
		offset = stride * queryIndex;
	}
	else
	{
		if (poolIndex == 0)
		{
			stride = m_QuerySize - (heapData.m_Pool[1] != VK_NULL_HANDLE ? sizeof(u64) : 0);
			offset = m_QuerySize * queryIndex;
		}
		else if (poolIndex == 1 && heapData.m_Pool)
		{
			stride = sizeof(u64);
			offset = m_QuerySize * queryIndex + (m_QuerySize - sizeof(u64));
		}
	}

	buffer = heapData.m_pBuffer->GetHandle();
}


void BvQueryHeapVk::Create()
{
	bool hasMeshShaderQueries = m_QuerySize == sizeof(PipelineStatistics);
	VkQueryPoolCreateInfo qpCI{ VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
	qpCI.queryCount = m_QueryCount * m_FrameCount;
	qpCI.queryType = GetVkQueryType(m_QueryType);
	if (m_QueryType == QueryType::kPipelineStatistics)
	{
		qpCI.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT
			| VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT
			| VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT
			| VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT
			| VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT
			| VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT
			| VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT
			| VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT
			| VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT
			| VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT
			| VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT
			| (hasMeshShaderQueries ? (VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT
			| VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT) : 0);
	}

	VkQueryPool pool;
	auto result = vkCreateQueryPool(m_pDevice->GetHandle(), &qpCI, nullptr, &pool);
	if (result != VK_SUCCESS)
	{
		// TODO: Handle error
		return;
	}
	VkQueryPool meshPool = VK_NULL_HANDLE;
	if (m_QueryType == QueryType::kPipelineStatistics && hasMeshShaderQueries)
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
	BvBufferVk* pBuffer;
	if (!(pBuffer = m_pDevice->CreateBuffer<BvBufferVk>(bufferDesc, nullptr)))
	{
		return;
	}

	m_QueryHeapData.EmplaceBack(BvQueryHeapVk::HeapData{ { pool, meshPool }, BvVector<u32>(m_FrameCount), pBuffer });

	vkResetQueryPool(m_pDevice->GetHandle(), pool, 0, m_QueryCount * m_FrameCount);
	if (meshPool != VK_NULL_HANDLE)
	{
		vkResetQueryPool(m_pDevice->GetHandle(), meshPool, 0, m_QueryCount * m_FrameCount);
	}
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