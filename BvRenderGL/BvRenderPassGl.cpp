#include "BvRenderPassGl.h"


BvRenderPassGl::BvRenderPassGl(const BvRenderDeviceGl& device, const RenderPassDesc& renderPassDesc)
	: IBvRenderPass(renderPassDesc), m_Device(device)
{
}


BvRenderPassGl::~BvRenderPassGl()
{
}