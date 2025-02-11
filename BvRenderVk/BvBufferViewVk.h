#pragma once


#include "BDeV/Core/RenderAPI/BvBufferView.h"
#include "BvBufferVk.h"


class BvRenderDeviceVk;


BV_OBJECT_DEFINE_ID(BvBufferViewVk, "d354dda0-8c2f-49b9-9de6-ddc37d7179f5");
class BvBufferViewVk final : public BvBufferView
{
	BV_NOCOPYMOVE(BvBufferViewVk);

public:
	BvBufferViewVk(BvRenderDeviceVk* pDevice, const BufferViewDesc & bufferViewDesc);
	~BvBufferViewVk();

	BvRenderDevice* GetDevice() override;

	BV_INLINE VkBufferView GetHandle() const { return m_View; }
	BV_INLINE bool IsValid() const { return m_BufferViewDesc.m_pBuffer != nullptr; }

	BV_OBJECT_IMPL_INTERFACE(BvBufferViewVk, BvBufferView, IBvRenderDeviceObject);

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkBufferView m_View = VK_NULL_HANDLE;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvBufferViewVk);


BV_CREATE_CAST_TO_VK(BvBufferView)