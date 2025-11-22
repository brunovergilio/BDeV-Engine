#pragma once


#include "BDeV/Core/RenderAPI/BvBuffer.h"
#include "BvCommonD3D12.h"


class BvRenderDeviceD3D12;


BV_OBJECT_DEFINE_ID(BvBufferD3D12, "2b3dc85f-6ac1-4916-a990-41f2cf739614");
class BvBufferD3D12 : public IBvBuffer, public IBvResourceD3D12
{
	BV_NOCOPYMOVE(BvBufferD3D12);
	BV_D3D12_DEVICE_RES_DECL;

public:
	BvBufferD3D12(BvRenderDeviceD3D12* pDevice, const BufferDesc& bufferDesc, const BufferInitData* pInitData);
	~BvBufferD3D12();

	void* const Map(u64 size, u64 offset) override;
	void Unmap() override;
	BV_INLINE void Flush(u64 size = 0, u64 offset = 0) const override {}
	BV_INLINE void Invalidate(u64 size = 0, u64 offset = 0) const override {}

	BV_INLINE const BufferDesc& GetDesc() const override { return m_BufferDesc; }
	BV_INLINE void* GetMappedData() const override { return m_pMapped; }
	BV_INLINE ID3D12Resource* GetHandle() const { return m_Buffer.Get(); }
	BV_INLINE u64 GetDeviceAddress() const { return m_Buffer->GetGPUVirtualAddress(); }

	BV_INLINE bool IsValid() const { return m_Buffer; }

private:
	void Create(const BufferInitData* pInitData);
	void Destroy();
	void CopyInitDataToGPU(const BufferInitData* pInitData);

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	BufferDesc m_BufferDesc;
	ComPtr<ID3D12Resource> m_Buffer;
	ComPtr<D3D12MA::Allocation> m_Allocation;
	void* m_pMapped = nullptr;
	D3D12_RANGE m_Range{};
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvBufferD3D12);


BV_CREATE_CAST_TO_D3D12(BvBuffer)