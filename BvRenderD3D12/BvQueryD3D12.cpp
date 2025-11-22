#include "BvQueryD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BvTypeConversionsD3D12.h"
#include "BvBufferD3D12.h"
#include "BvGPUFenceD3D12.h"


BV_D3D12_DEVICE_RES_DEF(BvQueryD3D12)


BvQueryD3D12::BvQueryD3D12(BvRenderDeviceD3D12* pDevice, QueryType queryType, u32 frameCount)
	: m_pDevice(pDevice), m_QueryType(queryType), m_QueryData(frameCount, {})
{
}


BvQueryD3D12::~BvQueryD3D12()
{
}


bool BvQueryD3D12::GetResult(void* pData, u64 size)
{
	auto& queryData = m_QueryData[m_LatestResultIndex];
	if (!queryData.m_pFence || queryData.m_pFence->GetCompletedValue() < queryData.m_SignalValue)
	{
		return false;
	}

	return queryData.m_pQueryHeap ? queryData.m_pQueryHeap->GetResult(queryData, m_LatestResultIndex, pData, size) : false;
}


QueryDataD3D12* BvQueryD3D12::Allocate(BvQueryHeapManagerD3D12* pQueryHeapManager, u32 frameIndex)
{
	pQueryHeapManager->AllocateQuery(m_QueryType, frameIndex, m_QueryData[frameIndex]);

	return &m_QueryData[frameIndex];
}


void BvQueryD3D12::SetFenceData(BvGPUFenceD3D12* pFence, u64 value)
{
	m_QueryData[m_LatestResultIndex].m_pFence = pFence;
	m_QueryData[m_LatestResultIndex].m_SignalValue = value;
}


void BvQueryD3D12::SetLatestFrameIndex(u32 frameIndex)
{
	m_LatestResultIndex = (m_LatestResultIndex + 1) % m_QueryData.Size();
}


void BvQueryD3D12::Destroy()
{
}


BvQueryHeapD3D12::BvQueryHeapD3D12()
{
}


BvQueryHeapD3D12::BvQueryHeapD3D12(BvRenderDeviceD3D12* pDevice, QueryType queryType, u32 queryCount, u32 frameCount)
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
		m_QuerySize = sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS);
		break;
	}
	case QueryType::kMeshPipelineStatistics:
	{
		m_QuerySize = sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS1);
		break;
	}
	default:
		BV_ASSERT(false, "Query type not implemented yet");
		break;
	}
}


BvQueryHeapD3D12::BvQueryHeapD3D12(BvQueryHeapD3D12&& rhs) noexcept
{
	*this = std::move(rhs);
}


BvQueryHeapD3D12& BvQueryHeapD3D12::operator=(BvQueryHeapD3D12&& rhs) noexcept
{
	m_pDevice = rhs.m_pDevice;
	std::swap(m_QueryHeapData, rhs.m_QueryHeapData);
	m_QueryCount = rhs.m_QueryCount;
	m_FrameCount = rhs.m_FrameCount;
	m_QuerySize = rhs.m_QuerySize;
	m_QueryType = rhs.m_QueryType;

	return *this;
}


BvQueryHeapD3D12::~BvQueryHeapD3D12()
{
	Destroy();
}


void BvQueryHeapD3D12::Allocate(u32 frameIndex, QueryDataD3D12& queryData)
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


void BvQueryHeapD3D12::Reset(u32 frameIndex)
{
	for (auto& heapData : m_QueryHeapData)
	{
		heapData.m_FrameAllocations[frameIndex] = 0;
	}
}


bool BvQueryHeapD3D12::GetResult(const QueryDataD3D12& queryData, u32 frameIndex, void* pData, u64 size)
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
		memcpy(pData, pResultData, m_QueryType == QueryType::kPipelineStatistics ? sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS)
			: sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS1));
		break;
	}
	}

	return true;
}


void BvQueryHeapD3D12::GetBufferInformation(u32 heapIndex, u32 frameIndex, u32 queryIndex, ID3D12Resource*& pBuffer, u64& offset, u64& stride)
{
	auto& heapData = m_QueryHeapData[heapIndex];
	auto frameOffset = m_QuerySize * queryIndex;
	stride = m_QuerySize;
	offset = frameOffset;

	pBuffer = heapData.m_pBuffer->GetHandle();
}


void BvQueryHeapD3D12::Create()
{
	D3D12_QUERY_HEAP_DESC heapDesc{ GetD3D12QueryHeapType(m_QueryType), m_QueryCount * m_FrameCount, 0 };

	ComPtr<ID3D12QueryHeap> heap;
	auto hr = m_pDevice->GetHandle()->CreateQueryHeap(&heapDesc, IID_PPV_ARGS(&heap));
	if (FAILED(hr))
	{
		// TODO: Handle error
		return;
	}

	BufferDesc bufferDesc;
	bufferDesc.m_Size = m_QuerySize * m_QueryCount * m_FrameCount;
	bufferDesc.m_MemoryType = MemoryType::kReadBack;
	bufferDesc.m_CreateFlags = BufferCreateFlags::kCreateMapped;
	BvRCRef<BvBufferD3D12> buffer;
	if (!(m_pDevice->CreateBuffer(bufferDesc, nullptr, &buffer)))
	{
		return;
	}

	m_QueryHeapData.EmplaceBack(BvQueryHeapD3D12::HeapData{ heap, BvVector<u32>(m_FrameCount), buffer });
}


void BvQueryHeapD3D12::Destroy()
{
	for (auto& heapData : m_QueryHeapData)
	{
		if (heapData.m_Pool)
		{
			heapData.m_Pool.Reset();
		}

		if (heapData.m_pBuffer)
		{
			heapData.m_pBuffer->Release();
		}
	}
}


BvQueryHeapManagerD3D12::BvQueryHeapManagerD3D12(BvRenderDeviceD3D12* pDevice, const u32* pQuerySizes, u32 frameCount)
	: m_pDevice(pDevice)
{
	for (auto i = 0; i < kQueryTypeCount; ++i)
	{
		if (pQuerySizes[i])
		{
			m_QueryHeaps[i] = BvQueryHeapD3D12(m_pDevice, (QueryType)i, pQuerySizes[i], frameCount);
		}
	}
}


BvQueryHeapManagerD3D12::~BvQueryHeapManagerD3D12()
{
}


void BvQueryHeapManagerD3D12::AllocateQuery(QueryType queryType, u32 frameIndex, QueryDataD3D12& queryData)
{
	auto queryTypeIndex = (u32)queryType;
	m_QueryHeaps[queryTypeIndex].Allocate(frameIndex, queryData);
}


void BvQueryHeapManagerD3D12::Reset(u32 frameIndex)
{
	for (auto i = 0u; i < kQueryTypeCount; ++i)
	{
		m_QueryHeaps[i].Reset(frameIndex);
	}
}