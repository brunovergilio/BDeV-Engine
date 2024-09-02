#pragma once


#include "BDeV/Core/RenderAPI/BvBufferView.h"
#include "BvBufferVk.h"


class BvRenderDeviceVk;


class BvBufferViewVk final : public BvBufferView
{
	BV_NOCOPYMOVE(BvBufferViewVk);

public:
	BvBufferViewVk(BvRenderDeviceVk* pDevice, const BufferViewDesc & bufferViewDesc);
	~BvBufferViewVk();

	BvRenderDevice* GetDevice() override;

	BV_INLINE VkBufferView GetHandle() const { return m_View; }

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkBufferView m_View = VK_NULL_HANDLE;
};


BV_CREATE_CAST_TO_VK(BvBufferView)