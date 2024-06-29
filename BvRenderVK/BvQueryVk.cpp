#include "BvQueryVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"


BvQueryVk::BvQueryVk(QueryType queryType, u32 frameCount)
	: m_QueryType(queryType), m_QueryData(frameCount)
{
}


BvQueryVk::~BvQueryVk()
{
}


BvQueryVk::Data* BvQueryVk::Allocate(BvQueryHeapManagerVk* pQueryHeapManager, u32 queryCount, u32 frameIndex)
{
	pQueryHeapManager->AllocateQuery(m_QueryType, queryCount, frameIndex, m_QueryData[frameIndex]);

	return &m_QueryData[frameIndex];
}


void BvQueryVk::UpdateResults(u32 frameIndex)
{
	u64 queryResults[2];
	if (auto pQueryHeap = m_QueryData[frameIndex].m_pQueryHeap)
	{
		auto index = m_QueryData[frameIndex].m_QueryIndex;
		pQueryHeap->GetResults(queryResults, index, 1, sizeof(u64) * 2, frameIndex);
		m_Result = queryResults[0];
	}
}


BvQueryHeapVk::BvQueryHeapVk()
{
}


BvQueryHeapVk::BvQueryHeapVk(const BvRenderDeviceVk* pDevice, QueryType queryType, u32 queryCount, u32 frameCount)
	: m_Device(pDevice->GetHandle()), m_QueryCount(queryCount), m_FrameAllocations(frameCount)
{
	Create(queryType, queryCount, frameCount);
}


BvQueryHeapVk::BvQueryHeapVk(BvQueryHeapVk&& rhs) noexcept
{
	*this = std::move(rhs);
}


BvQueryHeapVk& BvQueryHeapVk::operator=(BvQueryHeapVk&& rhs) noexcept
{
	m_Device = rhs.m_Device;
	std::swap(m_Pool, rhs.m_Pool);
	std::swap(m_FrameAllocations, rhs.m_FrameAllocations);
	m_QueryCount = rhs.m_QueryCount;

	return *this;
}


BvQueryHeapVk::~BvQueryHeapVk()
{
	Destroy();
}


void BvQueryHeapVk::Create(QueryType queryType, u32 queryCount, u32 frameCount)
{
	VkQueryPoolCreateInfo qpCI{ VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
	qpCI.queryCount = queryCount * frameCount;
	qpCI.queryType = GetVkQueryType(queryType);

	auto result = vkCreateQueryPool(m_Device, &qpCI, nullptr, &m_Pool);
	if (result != VK_SUCCESS)
	{
		// TODO: Handle error
		return;
	}

	vkResetQueryPool(m_Device, m_Pool, 0, queryCount * frameCount);
}


void BvQueryHeapVk::Destroy()
{
	if (m_Pool)
	{
		vkDestroyQueryPool(m_Device, m_Pool, nullptr);
		m_Pool = nullptr;
	}
}


u32 BvQueryHeapVk::Allocate(u32 queryCount, u32 frameIndex)
{
	auto index = kU32Max;
	auto& currAllocationCount = m_FrameAllocations[frameIndex];
	if (currAllocationCount + queryCount <= m_QueryCount)
	{
		index = frameIndex * m_QueryCount + currAllocationCount;
		currAllocationCount += queryCount;
	}

	return index;
}


void BvQueryHeapVk::Reset(u32 frameIndex)
{
	if (m_FrameAllocations[frameIndex] == 0)
	{
		return;
	}

	vkResetQueryPool(m_Device, m_Pool, frameIndex * m_QueryCount, m_QueryCount);

	m_FrameAllocations[frameIndex] = 0;
}


void BvQueryHeapVk::GetResults(void* pData, u32 queryIndex, u32 queryCount, u64 stride, u32 frameIndex)
{
	auto result = vkGetQueryPoolResults(m_Device, m_Pool, queryIndex, queryCount,
		stride * queryCount, pData, stride, VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT);
	if (result != VK_SUCCESS && result != VK_NOT_READY)
	{
		// TODO: Handle error
	}
}


BvQueryHeapManagerVk::BvQueryHeapManagerVk(const BvRenderDeviceVk* pDevice, const u32* pQuerySizes, u32 frameCount)
	: m_pDevice(pDevice)
{
	for (auto i = 0; i < kQueryTypeCount; ++i)
	{
		m_QueryHeaps[i] = BvQueryHeapVk(m_pDevice, (QueryType)i, pQuerySizes[i], frameCount);
	}
}


BvQueryHeapManagerVk::~BvQueryHeapManagerVk()
{
}


void BvQueryHeapManagerVk::AllocateQuery(QueryType queryType, u32 queryCount, u32 frameIndex, BvQueryVk::Data& queryData)
{
	auto queryTypeIndex = (u32)queryType;
	auto index = m_QueryHeaps[queryTypeIndex].Allocate(queryCount, frameIndex);
	BvAssert(index != kU32Max, "No more queries available in pool - increase size");

	queryData.m_pQueryHeap = &m_QueryHeaps[queryTypeIndex];
	queryData.m_QueryIndex = index;
}


void BvQueryHeapManagerVk::Reset(u32 frameIndex)
{
	for (auto i = 0u; i < kQueryTypeCount; ++i)
	{
		m_QueryHeaps[i].Reset(frameIndex);
	}
}