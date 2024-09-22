#pragma once


#include "BDeV/Core/RenderAPI/BvBuffer.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


class BvBufferVk final : public BvBuffer
{
	BV_NOCOPYMOVE(BvBufferVk);

public:
	BvBufferVk(BvRenderDeviceVk* pDevice, const BufferDesc& bufferDesc, const BufferInitData* pInitData);
	~BvBufferVk();

	void* const Map(const u64 size, const u64 offset) override;
	void Unmap() override;
	void Flush(const u64 size, const u64 offset) const override;
	void Invalidate(const u64 size, const u64 offset) const override;
	BvRenderDevice* GetDevice() override;

	BV_INLINE VkBuffer GetHandle() const { return m_Buffer; }

private:
	void Create(const BufferInitData* pInitData);
	void Destroy();
	void CopyInitDataAndTransitionState(const BufferInitData* pInitData);

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkBuffer m_Buffer = VK_NULL_HANDLE;
	VmaAllocation m_VMAAllocation = nullptr;
	bool m_NeedsFlush = false;
};


BV_CREATE_CAST_TO_VK(BvBuffer)