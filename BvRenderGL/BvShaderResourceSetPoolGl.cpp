#include "BvShaderResourceSetPoolGl.h"


BvShaderResourceSetPoolGl::BvShaderResourceSetPoolGl(const BvRenderDeviceGl& device, const ShaderResourceSetPoolDesc& desc)
	: BvShaderResourceSetPool(desc), m_Device(device)
{
}


BvShaderResourceSetPoolGl::~BvShaderResourceSetPoolGl()
{
}