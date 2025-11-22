#pragma once


#include "BDeV/Core/RenderAPI/BvQuery.h"
#include "BvCommonD3D12.h"
#include "BDeV/Core/Container/BvVector.h"


class BvQueryHeapD3D12;
class BvQueryHeapManagerD3D12;
class BvRenderDeviceD3D12;
class BvBufferD3D12;
class BvGPUFenceD3D12;


struct QueryDataD3D12
{
	BvQueryHeapD3D12* m_pQueryHeap = nullptr;
	u32 m_HeapIndex = 0;
	u32 m_QueryIndex = 0;
	BvGPUFenceD3D12* m_pFence = nullptr;
	u64 m_SignalValue = 0;
};


BV_OBJECT_DEFINE_ID(BvQueryD3D12, "842f2026-34c0-471f-a6fc-4f3a639bdab9");
class BvQueryD3D12 final : public IBvQuery, public IBvResourceD3D12
{
	BV_NOCOPYMOVE(BvQueryD3D12);
	BV_D3D12_DEVICE_RES_DECL;

public:
	BvQueryD3D12(BvRenderDeviceD3D12* pDevice, QueryType queryType, u32 frameCount);
	~BvQueryD3D12();

	bool GetResult(void* pData, u64 size) override;
	QueryDataD3D12* Allocate(BvQueryHeapManagerD3D12* pQueryHeapManager, u32 frameIndex);
	void SetFenceData(BvGPUFenceD3D12* pFence, u64 value);
	void SetLatestFrameIndex(u32 frameIndex);
	BV_INLINE const QueryDataD3D12* GetQueryData(u32 frameIndex) const { return &m_QueryData[frameIndex]; }
	BV_INLINE QueryType GetQueryType() const { return m_QueryType; }

	//BV_OBJECT_IMPL_INTERFACE(IBvQueryD3D12, IBvQuery, IBvRenderDeviceObject);

private:
	void Destroy();

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	BvVector<QueryDataD3D12> m_QueryData;
	QueryType m_QueryType = QueryType::kTimestamp;
	u32 m_LatestResultIndex = 0;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvQueryD3D12);


BV_CREATE_CAST_TO_D3D12(BvQuery)


class BvQueryHeapD3D12 final
{
	BV_NOCOPY(BvQueryHeapD3D12);

public:
	BvQueryHeapD3D12();
	BvQueryHeapD3D12(BvRenderDeviceD3D12* pDevice, QueryType queryType, u32 queryCount, u32 frameCount);
	BvQueryHeapD3D12(BvQueryHeapD3D12&& rhs) noexcept;
	BvQueryHeapD3D12& operator=(BvQueryHeapD3D12&& rhs) noexcept;
	~BvQueryHeapD3D12();

	void Allocate(u32 frameIndex, QueryDataD3D12& queryData);
	void Reset(u32 frameIndex);
	bool GetResult(const QueryDataD3D12& queryData, u32 frameIndex, void* pData, u64 size);
	void GetBufferInformation(u32 heapIndex, u32 frameIndex, u32 queryIndex, ID3D12Resource*& pBuffer, u64& offset, u64& stride);

	BV_INLINE ID3D12QueryHeap* GetHandle(u32 heapIndex) const { return m_QueryHeapData[heapIndex].m_Pool.Get(); }

	struct HeapData
	{
		ComPtr<ID3D12QueryHeap> m_Pool;
		BvVector<u32> m_FrameAllocations;
		BvBufferD3D12* m_pBuffer;
	};

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	BvVector<HeapData> m_QueryHeapData;
	u32 m_QueryCount = 0;
	u32 m_FrameCount = 0;
	u64 m_QuerySize = 0;
	QueryType m_QueryType = QueryType::kTimestamp;
};


struct QueryD3D12
{
	ID3D12QueryHeap* m_Pool;
	u32 m_Index;
};


class BvQueryHeapManagerD3D12
{
	BV_NOCOPYMOVE(BvQueryHeapManagerD3D12);

public:
	BvQueryHeapManagerD3D12(BvRenderDeviceD3D12* pDevice, const u32* pQuerySizes, u32 frameCount);
	~BvQueryHeapManagerD3D12();

	void AllocateQuery(QueryType queryType, u32 frameIndex, QueryDataD3D12& queryData);
	void Reset(u32 frameIndex);

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	BvQueryHeapD3D12 m_QueryHeaps[kQueryTypeCount]{};
};