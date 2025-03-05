#pragma once


#include "BDeV/Core/RenderAPI/BvBufferView.h"
#include "BvBufferVk.h"


class BvRenderDeviceVk;


BV_OBJECT_DEFINE_ID(IBvBufferViewVk, "d354dda0-8c2f-49b9-9de6-ddc37d7179f5");
class IBvBufferViewVk : public IBvBufferView
{
	BV_NOCOPYMOVE(IBvBufferViewVk);

public:
	virtual VkBufferView GetHandle() const = 0;
	virtual bool IsValid() const = 0;

protected:
	IBvBufferViewVk() {}
	~IBvBufferViewVk() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvBufferViewVk);


class BvBufferViewVk final : public IBvBufferViewVk
{
	BV_NOCOPYMOVE(BvBufferViewVk);

public:
	BvBufferViewVk(BvRenderDeviceVk* pDevice, const BufferViewDesc& bufferViewDesc);
	~BvBufferViewVk();

	IBvRenderDevice* GetDevice() override;
	BV_INLINE const BufferViewDesc& GetDesc() const override { return m_BufferViewDesc; }
	BV_INLINE VkBufferView GetHandle() const override { return m_View; }
	BV_INLINE bool IsValid() const override { return m_BufferViewDesc.m_pBuffer != nullptr; }

	BV_OBJECT_IMPL_INTERFACE(IBvBufferViewVk, IBvBufferView, IBvRenderDeviceObject);

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkBufferView m_View = VK_NULL_HANDLE;
	BufferViewDesc m_BufferViewDesc;
};


BV_CREATE_CAST_TO_VK(IBvBufferView)