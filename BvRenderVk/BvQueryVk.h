#pragma once


#include "BDeV/Core/RenderAPI/BvQuery.h"
#include "BvCommonVk.h"
#include "BDeV/Core/Container/BvVector.h"


class BvQueryHeapVk;
class BvQueryHeapManagerVk;
class IBvRenderDeviceVk;
class IBvBufferVk;
class IBvGPUFenceVk;


struct QueryDataVk
{
	BvQueryHeapVk* m_pQueryHeap = nullptr;
	u32 m_HeapIndex = 0;
	u32 m_QueryIndex = 0;
	IBvGPUFenceVk* m_pFence = nullptr;
	u64 m_SignalValue = 0;
};


BV_OBJECT_DEFINE_ID(IBvQueryVk, "842f2026-34c0-471f-a6fc-4f3a639bdab9");
class IBvQueryVk : public IBvQuery
{
	BV_NOCOPYMOVE(IBvQueryVk);

public:
	IBvQueryVk() {}
	~IBvQueryVk() {}

	virtual QueryDataVk* Allocate(BvQueryHeapManagerVk* pQueryHeapManager, u32 frameIndex) = 0;
	virtual void SetFenceData(IBvGPUFenceVk* pFence, u64 value) = 0;
	virtual void SetLatestFrameIndex(u32 frameIndex) = 0;
	virtual const QueryDataVk* GetQueryData(u32 frameIndex) const = 0;
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvQueryVk);


class BvQueryVk final : public IBvQueryVk
{
	BV_NOCOPYMOVE(BvQueryVk);

public:
	BvQueryVk(IBvRenderDeviceVk* pDevice, QueryType queryType, u32 frameCount);
	~BvQueryVk();

	IBvRenderDevice* GetDevice() override;
	bool GetResult(void* pData, u64 size) override;
	QueryDataVk* Allocate(BvQueryHeapManagerVk* pQueryHeapManager, u32 frameIndex) override;
	void SetFenceData(IBvGPUFenceVk* pFence, u64 value) override;
	void SetLatestFrameIndex(u32 frameIndex) override;
	BV_INLINE const QueryDataVk* GetQueryData(u32 frameIndex) const override { return &m_QueryData[frameIndex]; }
	BV_INLINE QueryType GetQueryType() const override { return m_QueryType; }

	BV_OBJECT_IMPL_INTERFACE(IBvQueryVk, IBvQuery, IBvRenderDeviceObject);

private:
	IBvRenderDeviceVk* m_pDevice = nullptr;
	BvVector<QueryDataVk> m_QueryData;
	QueryType m_QueryType = QueryType::kTimestamp;
	u32 m_LatestResultIndex = 0;
};


BV_CREATE_CAST_TO_VK(IBvQuery)


class BvQueryHeapVk final
{
	BV_NOCOPY(BvQueryHeapVk);

public:
	BvQueryHeapVk();
	BvQueryHeapVk(IBvRenderDeviceVk* pDevice, QueryType queryType, u32 queryCount, u32 frameCount);
	BvQueryHeapVk(BvQueryHeapVk&& rhs) noexcept;
	BvQueryHeapVk& operator=(BvQueryHeapVk&& rhs) noexcept;
	~BvQueryHeapVk();
	
	void Allocate(u32 frameIndex, QueryDataVk& queryData);
	void Reset(u32 frameIndex);
	bool GetResult(const QueryDataVk& queryData, u32 frameIndex, void* pData, u64 size);
	void GetBufferInformation(u32 heapIndex, u32 frameIndex, u32 queryIndex, VkBuffer& buffer, u64& offset, u64& stride);
	u64 GetQuerySize() const;

	BV_INLINE VkQueryPool GetHandle(u32 heapIndex) const { return m_QueryHeapData[heapIndex].m_Pool; }

	struct HeapData
	{
		VkQueryPool m_Pool;
		BvVector<u32> m_FrameAllocations;
		IBvBufferVk* m_pBuffer;
	};

private:
	void Create();
	void Destroy();
	
private:
	IBvRenderDeviceVk* m_pDevice = nullptr;
	BvVector<HeapData> m_QueryHeapData;
	u32 m_QueryCount = 0;
	u32 m_FrameCount = 0;
	QueryType m_QueryType = QueryType::kTimestamp;
};



class BvQueryHeapManagerVk
{
	BV_NOCOPYMOVE(BvQueryHeapManagerVk);

public:
	BvQueryHeapManagerVk(IBvRenderDeviceVk* pDevice, const u32* pQuerySizes, u32 frameCount);
	~BvQueryHeapManagerVk();

	void AllocateQuery(QueryType queryType, u32 frameIndex, QueryDataVk& queryData);
	void Reset(u32 frameIndex);

private:
	IBvRenderDeviceVk* m_pDevice = nullptr;
	BvQueryHeapVk m_QueryHeaps[kQueryTypeCount]{};
};