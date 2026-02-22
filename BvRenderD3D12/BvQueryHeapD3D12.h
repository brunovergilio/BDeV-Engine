#pragma once


#include "BDeV/Core/RenderAPI/BvQueryHeap.h"
#include "BvCommonD3D12.h"
#include "BDeV/Core/Container/BvVector.h"


class BvRenderDeviceD3D12;


class BvQueryHeapD3D12 final : public IBvQueryHeap, public IBvResourceD3D12
{
public:
	BvQueryHeapD3D12(BvRenderDeviceD3D12* pDevice, const QueryHeapDesc& queryHeapDesc, ComPtr<ID3D12QueryHeap>& queryHeap);
	~BvQueryHeapD3D12();

	BV_INLINE const QueryHeapDesc& GetDesc() const override { return m_QueryHeapDesc; }
	BV_INLINE u32 GetQuerySize() const override { return m_QuerySize; }
	void MapPSOStats(void* pDstData, u32 dstDataSize, void* pSrcData) const override;
	BV_INLINE auto GetHandle(u32 index = 0) const { return m_QueryHeap.Get(); }

private:
	void Destroy();

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	QueryHeapDesc m_QueryHeapDesc;
	ComPtr<ID3D12QueryHeap> m_QueryHeap;
	u32 m_QuerySize = 0;
};
BV_OBJECT_DEFINE_ID(BvQueryHeapD3D12, "842f2026-34c0-471f-a6fc-4f3a639bdab9");
BV_CREATE_CAST_TO_D3D12(BvQueryHeap)