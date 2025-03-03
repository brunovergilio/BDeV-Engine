#include "BvQueryVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvBufferVk.h"
#include "BvGPUFenceVk.h"


BvQueryVk::BvQueryVk(BvRenderDeviceVk* pDevice, QueryType queryType, u32 frameCount)
	: m_pDevice(pDevice), m_QueryType(queryType), m_QueryData(frameCount, {})
{
}


BvQueryVk::~BvQueryVk()
{
}


BvRenderDevice* BvQueryVk::GetDevice()
{
	return m_pDevice;
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


BvQueryVk::Data* BvQueryVk::Allocate(BvQueryHeapManagerVk* pQueryHeapManager, u32 frameIndex)
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


BvQueryHeapVk::BvQueryHeapVk()
{
}


BvQueryHeapVk::BvQueryHeapVk(BvRenderDeviceVk* pDevice, QueryType queryType, u32 queryCount, u32 frameCount)
	: m_pDevice(pDevice), m_QueryCount(queryCount), m_FrameCount(frameCount), m_QueryType(queryType)
{
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

	return *this;
}


BvQueryHeapVk::~BvQueryHeapVk()
{
	Destroy();
}


void BvQueryHeapVk::Allocate(u32 frameIndex, BvQueryVk::Data& queryData)
{
	VkQueryPool pool = VK_NULL_HANDLE;
	u32 index = kU32Max;

	u32 heapIndex = 0;
	for (auto& heapData : m_QueryHeapData)
	{
		if (heapData.m_FrameAllocations[frameIndex] < m_QueryCount)
		{
			pool = heapData.m_Pool;
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
		pool = heapData.m_Pool;
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

		vkResetQueryPool(m_pDevice->GetHandle(), heapData.m_Pool, frameIndex * m_QueryCount, heapData.m_FrameAllocations[frameIndex]);

		heapData.m_FrameAllocations[frameIndex] = 0;
	}
}


bool BvQueryHeapVk::GetResult(const BvQueryVk::Data& queryData, u32 frameIndex, void* pData, u64 size)
{
	auto& heapData = m_QueryHeapData[queryData.m_HeapIndex];
	auto querySize = GetQuerySize();
	if (size < querySize)
	{
		return false;
	}

	auto offset = querySize * queryData.m_QueryIndex;
	auto pResultData = reinterpret_cast<u8*>(heapData.m_pBuffer->GetMappedData()) + offset;

	memcpy(pData, pResultData, querySize);

	return true;
}


void BvQueryHeapVk::GetBufferInformation(u32 heapIndex, u32 frameIndex, u32 queryIndex, VkBuffer& buffer, u64& offset, u64& stride)
{
	auto& heapData = m_QueryHeapData[heapIndex];
	stride = GetQuerySize();
	offset = stride * queryIndex;
	buffer = heapData.m_pBuffer->GetHandle();
}


u64 BvQueryHeapVk::GetQuerySize() const
{
	switch (m_QueryType)
	{
	case QueryType::kTimestamp:
	case QueryType::kOcclusion:
	case QueryType::kOcclusionBinary:
		return sizeof(u64);
	default:
		BV_ASSERT(false, "Query not yet implemented");
		return 0;
	}
}


void BvQueryHeapVk::Create()
{
	VkQueryPoolCreateInfo qpCI{ VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
	qpCI.queryCount = m_QueryCount * m_FrameCount;
	qpCI.queryType = GetVkQueryType(m_QueryType);

	VkQueryPool pool;
	auto result = vkCreateQueryPool(m_pDevice->GetHandle(), &qpCI, nullptr, &pool);
	if (result != VK_SUCCESS)
	{
		// TODO: Handle error
		return;
	}

	BufferDesc bufferDesc;
	bufferDesc.m_Size = GetQuerySize() * m_QueryCount * m_FrameCount;
	bufferDesc.m_MemoryType = MemoryType::kReadBack;
	bufferDesc.m_CreateFlags = BufferCreateFlags::kCreateMapped;
	BvBufferVk* pBuffer;
	if (!m_pDevice->CreateBufferVk(bufferDesc, nullptr, &pBuffer))
	{
		return;
	}

	m_QueryHeapData.EmplaceBack(BvQueryHeapVk::HeapData{ pool, BvVector<u32>(m_FrameCount), pBuffer });

	vkResetQueryPool(m_pDevice->GetHandle(), pool, 0, m_QueryCount * m_FrameCount);
}


void BvQueryHeapVk::Destroy()
{
	for (auto& heapData : m_QueryHeapData)
	{
		if (heapData.m_Pool)
		{
			vkDestroyQueryPool(m_pDevice->GetHandle(), heapData.m_Pool, nullptr);
			heapData.m_Pool = nullptr;
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


void BvQueryHeapManagerVk::AllocateQuery(QueryType queryType, u32 frameIndex, BvQueryVk::Data& queryData)
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