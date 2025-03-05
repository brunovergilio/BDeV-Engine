#include "BvBufferViewGl.h"


BvBufferViewGl::BvBufferViewGl(const BvRenderDeviceGl& device, const BufferViewDesc& bufferViewDesc)
	: IBvBufferView(bufferViewDesc), m_Device(device)
{
}


BvBufferViewGl::~BvBufferViewGl()
{
}