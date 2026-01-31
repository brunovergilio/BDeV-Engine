#pragma once


#include "BDeV/Core/RenderAPI/BvBufferView.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


class BvBufferViewVk final : public IBvBufferView, public IBvResourceVk
{
public:
	BvBufferViewVk(BvRenderDeviceVk* pDevice, const BufferViewDesc& bufferViewDesc, VkBufferView view);
	~BvBufferViewVk();

	BV_INLINE const BufferViewDesc& GetDesc() const { return m_BufferViewDesc; }
	BV_INLINE VkBufferView GetHandle() const { return m_View; }
	BV_INLINE bool IsValid() const { return m_BufferViewDesc.m_pBuffer != nullptr; }

private:
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkBufferView m_View = VK_NULL_HANDLE;
	BufferViewDesc m_BufferViewDesc;
};
BV_OBJECT_DEFINE_ID(BvBufferViewVk, "d354dda0-8c2f-49b9-9de6-ddc37d7179f5");
BV_CREATE_CAST_TO_VK(BvBufferView)