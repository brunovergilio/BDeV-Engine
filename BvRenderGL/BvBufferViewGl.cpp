#include "BvBufferViewGl.h"


BvBufferViewGl::BvBufferViewGl(const BvRenderDeviceGl& device, const BufferViewDesc& bufferViewDesc)
	: BvBufferView(bufferViewDesc), m_Device(device)
{
}


BvBufferViewGl::~BvBufferViewGl()
{
}