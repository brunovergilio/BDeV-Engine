#pragma once


#include "BDeV/Core/RenderAPI/BvBuffer.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


class BvBufferVk final : public IBvBuffer, public IBvResourceVk
{
public:
	BvBufferVk(BvRenderDeviceVk* pDevice, const BufferDesc& bufferDesc, VkBuffer buffer, VkDeviceAddress deviceAddress,
		VmaAllocation memory, void* pMappedMemory, bool needsFlush);
	~BvBufferVk();

	void* const Map(u64 size = kU64Max, u64 offset = 0) override;
	void Unmap() override;
	void Flush(u64 size = kU64Max, u64 offset = 0) const override;
	void Invalidate(u64 size = kU64Max, u64 offset = 0) const override;

	BV_INLINE const BufferDesc& GetDesc() const override { return m_BufferDesc; }
	BV_INLINE void* GetMappedData() const override { return m_pMapped; }
	BV_INLINE VkBuffer GetHandle() const { return m_Buffer; }
	BV_INLINE u64 GetDeviceAddress() const { return m_DeviceAddress; }

	BV_INLINE bool IsValid() const { return m_Buffer != VK_NULL_HANDLE; }

private:
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	BufferDesc m_BufferDesc;
	VkBuffer m_Buffer = VK_NULL_HANDLE;
	VmaAllocation m_VMAAllocation = nullptr;
	VkDeviceAddress m_DeviceAddress = 0;
	void* m_pMapped = nullptr;
	bool m_NeedsFlush = false;
};
BV_OBJECT_DEFINE_ID(BvBufferVk, "4e59c7e7-48f1-4d95-a852-96391f35aa78");
BV_CREATE_CAST_TO_VK(BvBuffer)