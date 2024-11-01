#pragma once


#include "BDeV/Core/RenderAPI/BvQuery.h"
#include "BvCommonVk.h"
#include "BDeV/Core/Container/BvVector.h"


class BvQueryHeapVk;
class BvQueryHeapManagerVk;
class BvRenderDeviceVk;


constexpr u32 kQueryTypeCount = 3;


class BvQueryVk final : public BvQuery
{
	BV_NOCOPYMOVE(BvQueryVk);

public:
	struct Data
	{
		BvQueryHeapVk* m_pQueryHeap = nullptr;
		u32 m_QueryIndex = 0;
	};

	BvQueryVk(BvRenderDeviceVk* pDevice, QueryType queryType, u32 frameCount);
	~BvQueryVk();

	BvRenderDevice* GetDevice() override;
	BV_INLINE QueryType GetQueryType() const override { return m_QueryType; }

	BvQueryVk::Data* Allocate(BvQueryHeapManagerVk* pQueryHeapManager, u32 queryCount, u32 frameIndex);
	void UpdateResults(u32 frameIndex);
	BV_INLINE u64 GetResult() const { return m_Result; }
	BV_INLINE BvQueryVk::Data* GetQueryData(u32 frameIndex) { return &m_QueryData[frameIndex]; }

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	BvVector<BvQueryVk::Data> m_QueryData;
	u64 m_Result = 0;
	QueryType m_QueryType = QueryType::kTimestamp;
};


class BvQueryHeapVk final
{
	BV_NOCOPY(BvQueryHeapVk);

public:
	BvQueryHeapVk();
	BvQueryHeapVk(const BvRenderDeviceVk* pDevice, QueryType queryType, u32 queryCount, u32 frameCount);
	BvQueryHeapVk(BvQueryHeapVk&& rhs) noexcept;
	BvQueryHeapVk& operator=(BvQueryHeapVk&& rhs) noexcept;
	~BvQueryHeapVk();
	
	u32 Allocate(u32 queryCount, u32 frameIndex);
	void Reset(u32 frameIndex);
	void GetResults(void* pData, u32 queryIndex, u32 queryCount, u64 stride, u32 frameIndex);

	BV_INLINE VkQueryPool GetHandle() const { return m_Pool; }

private:
	void Create(QueryType queryType, u32 queryCount, u32 frameCount);
	void Destroy();
	
private:
	VkDevice m_Device = VK_NULL_HANDLE;
	VkQueryPool m_Pool = VK_NULL_HANDLE;
	BvVector<u32> m_FrameAllocations;
	u32 m_QueryCount = 0;
};


BV_CREATE_CAST_TO_VK(BvQuery)


class BvQueryHeapManagerVk
{
	BV_NOCOPYMOVE(BvQueryHeapManagerVk);

public:
	BvQueryHeapManagerVk(const BvRenderDeviceVk* pDevice, const u32* pQuerySizes, u32 frameCount);
	~BvQueryHeapManagerVk();

	void AllocateQuery(QueryType queryType, u32 queryCount, u32 frameIndex, BvQueryVk::Data& queryData);
	void Reset(u32 frameIndex);

private:
	const BvRenderDeviceVk* m_pDevice = nullptr;
	BvQueryHeapVk m_QueryHeaps[kQueryTypeCount]{};
};