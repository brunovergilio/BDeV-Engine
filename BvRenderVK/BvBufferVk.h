#pragma once


#include "BDeV/Core/RenderAPI/BvBuffer.h"
#include "BvCommonVk.h"
#include <VulkanMemoryAllocator/include/vk_mem_alloc.h>


class BvRenderDeviceVk;


class BvBufferVk final : public BvBuffer
{
	BV_NOCOPYMOVE(BvBufferVk);

public:
	BvBufferVk(const BvRenderDeviceVk& device, const BufferDesc& bufferDesc, const BufferInitData* pInitData);
	~BvBufferVk();

	void Create(const BufferInitData* pInitData);
	void Destroy();

	void * const Map(const u64 size, const u64 offset) override;
	void Unmap() override;
	void Flush(const u64 size, const u64 offset) const override;
	void Invalidate(const u64 size, const u64 offset) const override;

	BV_INLINE VkBuffer GetHandle() const { return m_Buffer; }

private:
	void CopyInitDataAndTransitionState(const BufferInitData* pInitData);

private:
	const BvRenderDeviceVk & m_Device;
	VkBuffer m_Buffer = VK_NULL_HANDLE;
	VmaAllocation m_VMAAllocation = nullptr;
	bool m_NeedsFlush = false;
};


BV_CREATE_CAST_TO_VK(BvBuffer)