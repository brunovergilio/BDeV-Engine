#pragma once


#include "BvRenderCommon.h"


class IBvQueryHeap : public BvRCObj
{
public:
	virtual const QueryHeapDesc& GetDesc() const = 0;
	virtual u32 GetQuerySize() const = 0;

	BV_INLINE PipelineStatistics GetPipelineStats(void* pSrcData) const
	{
		BV_ASSERT(GetDesc().m_Type == QueryType::kPipelineStatistics, "Query Heap not of type PSO Stats");
		PipelineStatistics stats;
		MapPSOStats(&stats, sizeof(PipelineStatistics), pSrcData);

		return stats;
	}

	BV_INLINE MeshPipelineStatistics GetMeshPipelineStats(void* pSrcData) const
	{
		BV_ASSERT(GetDesc().m_Type == QueryType::kMeshPipelineStatistics, "Query Heap not of type Mesh PSO Stats");
		MeshPipelineStatistics stats;
		MapPSOStats(&stats, sizeof(MeshPipelineStatistics), pSrcData);

		return stats;
	}

protected:
	virtual void MapPSOStats(void* pDstData, u32 dstDataSize, void* pSrcData) const = 0;

	IBvQueryHeap() {}
	~IBvQueryHeap() {}
};
BV_OBJECT_DEFINE_ID(IBvQueryHeap, "82e3f062-7670-4e70-9b8f-3d6698815cee");