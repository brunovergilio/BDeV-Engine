#pragma once


#include "BDeV/Core/RenderAPI/BvBufferView.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


BV_OBJECT_DEFINE_ID(BvBufferViewVk, "d354dda0-8c2f-49b9-9de6-ddc37d7179f5");
class BvBufferViewVk final : public IBvBufferView, public IBvResourceVk
{
	BV_NOCOPYMOVE(BvBufferViewVk);
	BV_VK_DEVICE_RES_DECL;

public:
	BvBufferViewVk(BvRenderDeviceVk* pDevice, const BufferViewDesc& bufferViewDesc);
	~BvBufferViewVk();

	BV_INLINE const BufferViewDesc& GetDesc() const { return m_BufferViewDesc; }
	BV_INLINE VkBufferView GetHandle() const { return m_View; }
	BV_INLINE bool IsValid() const { return m_BufferViewDesc.m_pBuffer != nullptr; }

	//BV_OBJECT_IMPL_INTERFACE(IBvBufferViewVk, IBvBufferView, IBvRenderDeviceObject);

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkBufferView m_View = VK_NULL_HANDLE;
	BufferViewDesc m_BufferViewDesc;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvBufferViewVk);


BV_CREATE_CAST_TO_VK(BvBufferView)