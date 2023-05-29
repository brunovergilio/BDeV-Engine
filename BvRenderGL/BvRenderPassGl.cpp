#include "BvRenderPassGl.h"


BvRenderPassGl::BvRenderPassGl(const BvRenderDeviceGl& device, const RenderPassDesc& renderPassDesc)
	: BvRenderPass(renderPassDesc), m_Device(device)
{
}


BvRenderPassGl::~BvRenderPassGl()
{
}