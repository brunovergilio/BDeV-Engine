#pragma once


#include "BDeV/Core/RenderAPI/BvRenderCommon.h"
#include "BvCommonD3D12.h"


class BvRenderDeviceD3D12;
class BvCommandListD3D12;
class BvFrameDataD3D12;


class BvCommandAllocatorD3D12 final
{
	BV_NOCOPY(BvCommandAllocatorD3D12);

public:
	BvCommandAllocatorD3D12();
	BvCommandAllocatorD3D12(BvRenderDeviceD3D12* pDevice, CommandType commandType);
	BvCommandAllocatorD3D12(BvCommandAllocatorD3D12&& rhs) noexcept;
	BvCommandAllocatorD3D12& operator=(BvCommandAllocatorD3D12&& rhs) noexcept;
	~BvCommandAllocatorD3D12();

	void Create();
	void Destroy();

	BvCommandListD3D12* GetCommandList(BvFrameDataD3D12* pFrameData);

	void Reset();

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	BvVector<BvCommandListD3D12*> m_CommandLists;
	u32 m_ActiveCommandBufferCount = 0;
	CommandType m_CommandType = CommandType::kNone;
};