#pragma once


#include "BDeV/Core/RenderAPI/BvBuffer.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


BV_OBJECT_DEFINE_ID(IBvBufferVk, "4e59c7e7-48f1-4d95-a852-96391f35aa78");
class IBvBufferVk : public IBvBuffer
{
	BV_NOCOPYMOVE(IBvBufferVk);

public:
	virtual const BufferDesc& GetDesc() const = 0;
	virtual void* GetMappedData() const = 0;
	virtual VkBuffer GetHandle() const = 0;
	virtual bool IsValid() const = 0;

protected:
	IBvBufferVk() {}
	~IBvBufferVk() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvBufferVk);


class BvBufferVk final : public IBvBufferVk
{
	BV_NOCOPYMOVE(BvBufferVk);

public:
	BvBufferVk(BvRenderDeviceVk* pDevice, const BufferDesc& bufferDesc, const BufferInitData* pInitData);
	~BvBufferVk();

	void* const Map(u64 size, u64 offset) override;
	void Unmap() override;
	void Flush(u64 size, u64 offset) const override;
	void Invalidate(u64 size, u64 offset) const override;
	IBvRenderDevice* GetDevice() override;
	BV_INLINE const BufferDesc& GetDesc() const override { return m_BufferDesc; }
	BV_INLINE void* GetMappedData() const override { return m_pMapped; }
	BV_INLINE VkBuffer GetHandle() const override { return m_Buffer; }
	BV_INLINE u64 GetDeviceAddress() const override { return m_DeviceAddress; }

	BV_INLINE bool IsValid() const override { return m_Buffer != VK_NULL_HANDLE; }

	BV_OBJECT_IMPL_INTERFACE(IBvBufferVk, IBvBuffer, IBvRenderDeviceObject);

private:
	void Create(const BufferInitData* pInitData);
	void Destroy();
	void CopyInitDataAndTransitionState(const BufferInitData* pInitData);

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	BufferDesc m_BufferDesc;
	VkBuffer m_Buffer = VK_NULL_HANDLE;
	void* m_pMapped = nullptr;
	VkDeviceAddress m_DeviceAddress = 0;
	VmaAllocation m_VMAAllocation = nullptr;
	bool m_NeedsFlush = false;
};


BV_CREATE_CAST_TO_VK(IBvBuffer)