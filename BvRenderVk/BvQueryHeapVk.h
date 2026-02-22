#pragma once


#include "BDeV/Core/RenderAPI/BvQueryHeap.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


class BvQueryHeapVk final : public IBvQueryHeap, public IBvResourceVk
{
public:
	BvQueryHeapVk(BvRenderDeviceVk* pDevice, const QueryHeapDesc& queryHeapDesc, const VkQueryPool* pPools, VkQueryPipelineStatisticFlags psoFlags);
	~BvQueryHeapVk();

	BV_INLINE const QueryHeapDesc& GetDesc() const override { return m_QueryHeapDesc; }
	BV_INLINE u32 GetQuerySize() const override { return m_QuerySize; }
	void MapPSOStats(void* pDstData, u32 dstDataSize, void* pSrcData) const override;
	BV_INLINE VkQueryPool GetHandle(u32 index = 0) const { return m_QueryPools[index]; }

private:
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	QueryHeapDesc m_QueryHeapDesc;
	VkQueryPool m_QueryPools[2]{};
	u32 m_QuerySize = 0;
	VkQueryPipelineStatisticFlags m_PSOFlags = 0;
};
BV_OBJECT_DEFINE_ID(BvQueryHeapVk, "842f2026-34c0-471f-a6fc-4f3a639bdab9");
BV_CREATE_CAST_TO_VK(BvQueryHeap)