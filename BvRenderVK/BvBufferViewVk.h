#pragma once


#include "BvRender/BvBufferView.h"
#include "BvBufferVk.h"


class BvRenderDeviceVk;


class BvBufferViewVk final : public BvBufferView
{
	BV_NOCOPYMOVE(BvBufferViewVk);

public:
	BvBufferViewVk(const BvRenderDeviceVk & device, const BufferViewDesc & bufferViewDesc);
	~BvBufferViewVk() override final;

	void Create();
	void Destroy();

	BV_INLINE VkBufferView GetHandle() const { return m_View; }

private:
	const BvRenderDeviceVk & m_Device;
	VkBufferView m_View = VK_NULL_HANDLE;
};