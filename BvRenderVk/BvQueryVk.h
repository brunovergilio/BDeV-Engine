#pragma once


#include "BDeV/Core/RenderAPI/BvQuery.h"
#include "BvCommonVk.h"
#include "BDeV/Core/Container/BvVector.h"


class BvQueryHeapVk;
class BvQueryHeapManagerVk;
class BvRenderDeviceVk;
class BvBufferVk;
class BvGPUFenceVk;


BV_OBJECT_DEFINE_ID(BvQueryVk, "842f2026-34c0-471f-a6fc-4f3a639bdab9");
class BvQueryVk final : public BvQuery
{
	BV_NOCOPYMOVE(BvQueryVk);

public:
	struct Data
	{
		BvQueryHeapVk* m_pQueryHeap = nullptr;
		u32 m_HeapIndex = 0;
		u32 m_QueryIndex = 0;
		BvGPUFenceVk* m_pFence = nullptr;
		u64 m_SignalValue = 0;
	};

	BvQueryVk(BvRenderDeviceVk* pDevice, QueryType queryType, u32 frameCount);
	~BvQueryVk();

	BvRenderDevice* GetDevice() override;
	BV_INLINE QueryType GetQueryType() const override { return m_QueryType; }
	bool GetResult(void* pData, u64 size) override;

	BvQueryVk::Data* Allocate(BvQueryHeapManagerVk* pQueryHeapManager, u32 frameIndex);
	void SetFenceData(BvGPUFenceVk* pFence, u64 value);
	void SetLatestFrameIndex(u32 frameIndex);

	BV_INLINE const BvQueryVk::Data* GetQueryData(u32 frameIndex) const { return &m_QueryData[frameIndex]; }

	BV_OBJECT_IMPL_INTERFACE(BvQueryVk, BvQuery, IBvRenderDeviceObject);

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	BvVector<BvQueryVk::Data> m_QueryData;
	QueryType m_QueryType = QueryType::kTimestamp;
	u32 m_LatestResultIndex = 0;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvQueryVk);


BV_CREATE_CAST_TO_VK(BvQuery)


class BvQueryHeapVk final
{
	BV_NOCOPY(BvQueryHeapVk);

public:
	BvQueryHeapVk();
	BvQueryHeapVk(BvRenderDeviceVk* pDevice, QueryType queryType, u32 queryCount, u32 frameCount);
	BvQueryHeapVk(BvQueryHeapVk&& rhs) noexcept;
	BvQueryHeapVk& operator=(BvQueryHeapVk&& rhs) noexcept;
	~BvQueryHeapVk();
	
	void Allocate(u32 frameIndex, BvQueryVk::Data& queryData);
	void Reset(u32 frameIndex);
	bool GetResult(const BvQueryVk::Data& queryData, u32 frameIndex, void* pData, u64 size);
	void GetBufferInformation(u32 heapIndex, u32 frameIndex, u32 queryIndex, VkBuffer& buffer, u64& offset, u64& stride);
	u64 GetQuerySize() const;

	BV_INLINE VkQueryPool GetHandle(u32 heapIndex) const { return m_QueryHeapData[heapIndex].m_Pool; }

	struct HeapData
	{
		VkQueryPool m_Pool;
		BvVector<u32> m_FrameAllocations;
		BvBufferVk* m_pBuffer;
	};

private:
	void Create();
	void Destroy();
	
private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	BvVector<HeapData> m_QueryHeapData;
	u32 m_QueryCount = 0;
	u32 m_FrameCount = 0;
	QueryType m_QueryType = QueryType::kTimestamp;
};



class BvQueryHeapManagerVk
{
	BV_NOCOPYMOVE(BvQueryHeapManagerVk);

public:
	BvQueryHeapManagerVk(BvRenderDeviceVk* pDevice, const u32* pQuerySizes, u32 frameCount);
	~BvQueryHeapManagerVk();

	void AllocateQuery(QueryType queryType, u32 frameIndex, BvQueryVk::Data& queryData);
	void Reset(u32 frameIndex);

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	BvQueryHeapVk m_QueryHeaps[kQueryTypeCount]{};
};