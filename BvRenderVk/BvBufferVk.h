#pragma once


#include "BDeV/Core/RenderAPI/BvBuffer.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


BV_OBJECT_DEFINE_ID(BvBufferVk, "4e59c7e7-48f1-4d95-a852-96391f35aa78");
class BvBufferVk final : public IBvBuffer, public IBvResourceVk
{
	BV_NOCOPYMOVE(BvBufferVk);
	BV_VK_DEVICE_RES_DECL;

public:
	BvBufferVk(BvRenderDeviceVk* pDevice, const BufferDesc& bufferDesc, const BufferInitData* pInitData);
	~BvBufferVk();

	void* const Map(u64 size, u64 offset) override;
	void Unmap() override;
	void Flush(u64 size, u64 offset) const override;
	void Invalidate(u64 size, u64 offset) const override;

	BV_INLINE const BufferDesc& GetDesc() const override { return m_BufferDesc; }
	BV_INLINE void* GetMappedData() const override { return m_pMapped; }
	BV_INLINE VkBuffer GetHandle() const { return m_Buffer; }
	BV_INLINE u64 GetDeviceAddress() const { return m_DeviceAddress; }

	BV_INLINE bool IsValid() const { return m_Buffer != VK_NULL_HANDLE; }

	//BV_OBJECT_IMPL_INTERFACE(IBvBufferVk, IBvBuffer, IBvRenderDeviceObject);

private:
	void Create(const BufferInitData* pInitData);
	void Destroy();
	void CopyInitDataToGPU(const BufferInitData* pInitData);

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	BufferDesc m_BufferDesc;
	VkBuffer m_Buffer = VK_NULL_HANDLE;
	void* m_pMapped = nullptr;
	VkDeviceAddress m_DeviceAddress = 0;
	VmaAllocation m_VMAAllocation = nullptr;
	bool m_NeedsFlush = false;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvBufferVk);


BV_CREATE_CAST_TO_VK(BvBuffer)