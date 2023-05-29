#pragma once


#include "BDeV/RenderAPI/BvBufferView.h"
#include "BvBufferGl.h"


class BvRenderDeviceGl;


class BvBufferViewGl final : public BvBufferView
{
	BV_NOCOPYMOVE(BvBufferViewGl);

public:
	BvBufferViewGl(const BvRenderDeviceGl& device, const BufferViewDesc& bufferViewDesc);
	~BvBufferViewGl() override;

private:
	const BvRenderDeviceGl& m_Device;
};