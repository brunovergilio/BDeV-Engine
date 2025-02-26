#pragma once


#include "BDeV/Core/RenderAPI/BvBuffer.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


BV_OBJECT_DEFINE_ID(BvBufferVk, "4e59c7e7-48f1-4d95-a852-96391f35aa78");
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
	BV_INLINE VkDeviceAddress GetDeviceAddress() const { return m_DeviceAddress; }
	BV_INLINE bool IsValid() const { return m_Buffer != VK_NULL_HANDLE; }

	BV_OBJECT_IMPL_INTERFACE(BvBufferVk, BvBuffer, IBvRenderDeviceObject);

private:
	void Create(const BufferInitData* pInitData);
	void Destroy();
	void CopyInitDataAndTransitionState(const BufferInitData* pInitData);

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkBuffer m_Buffer = VK_NULL_HANDLE;
	VkDeviceAddress m_DeviceAddress = 0;
	VmaAllocation m_VMAAllocation = nullptr;
	bool m_NeedsFlush = false;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvBufferVk);


BV_CREATE_CAST_TO_VK(BvBuffer)