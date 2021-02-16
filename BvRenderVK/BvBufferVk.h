#pragma once


#include "BvRender/BvBuffer.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


class BvBufferVk : public BvBuffer
{
	BV_NOCOPYMOVE(BvBufferVk);

public:
	BvBufferVk(const BvRenderDeviceVk & device, const BufferDesc & bufferDesc);
	~BvBufferVk();

	void Create();
	void Destroy();

	void * const Map(const u64 size, const u64 offset) override final;
	void Unmap() override final;
	void Flush(const u64 size, const u64 offset) const override final;
	void Invalidate(const u64 size, const u64 offset) const override final;

	BV_INLINE VkBuffer GetHandle() const { return m_Buffer; }

private:
	const BvRenderDeviceVk & m_Device;
	VkBuffer m_Buffer = VK_NULL_HANDLE;
	VkDeviceMemory m_Memory = VK_NULL_HANDLE;
};