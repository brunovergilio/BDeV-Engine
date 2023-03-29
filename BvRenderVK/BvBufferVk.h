#pragma once


#include "BDeV/Render/BvBuffer.h"
#include "BvCommonVk.h"
#include <vma/vk_mem_alloc.h>


class BvRenderDeviceVk;


class BvBufferVk final : public BvBuffer
{
	BV_NOCOPYMOVE(BvBufferVk);

public:
	BvBufferVk(const BvRenderDeviceVk & device, const BufferDesc & bufferDesc);
	~BvBufferVk();

	void Create();
	void Destroy();

	void * const Map(const u64 size, const u64 offset) override;
	void Unmap() override;
	void Flush(const u64 size, const u64 offset) const override;
	void Invalidate(const u64 size, const u64 offset) const override;

	BV_INLINE VkBuffer GetHandle() const { return m_Buffer; }

private:
	const BvRenderDeviceVk & m_Device;
	VkBuffer m_Buffer = VK_NULL_HANDLE;
	VmaAllocation m_VMAAllocation = nullptr;
};