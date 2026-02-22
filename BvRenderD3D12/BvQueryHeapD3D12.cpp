#include "BvQueryHeapD3D12.h"
#include "BvRenderDeviceD3D12.h"


BvQueryHeapD3D12::BvQueryHeapD3D12(BvRenderDeviceD3D12* pDevice, const QueryHeapDesc& queryHeapDesc, ComPtr<ID3D12QueryHeap>& queryHeap)
	: m_pDevice(pDevice), m_QueryHeapDesc(queryHeapDesc), m_QueryHeap(std::move(queryHeap))
{
	switch (m_QueryHeapDesc.m_Type)
	{
	case QueryType::kTimestamp:
	case QueryType::kOcclusion:
	case QueryType::kOcclusionBinary:
		m_QuerySize = sizeof(u64);
		break;
	case QueryType::kPipelineStatistics:
		m_QuerySize = sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS);
		break;
	case QueryType::kMeshPipelineStatistics:
		m_QuerySize = sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS1);
		break;
	default:
		m_QuerySize = 0;
		break;
	}
}


BvQueryHeapD3D12::~BvQueryHeapD3D12()
{
	Destroy();
}


void BvQueryHeapD3D12::MapPSOStats(void* pDstData, u32 dstDataSize, void* pSrcData) const
{
	memcpy(pDstData, pSrcData, dstDataSize);
}


void BvQueryHeapD3D12::Destroy()
{
	m_QueryHeap = nullptr;
}