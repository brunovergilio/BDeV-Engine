#include "BvTextureViewGl.h"


BvTextureViewGl::BvTextureViewGl(const BvRenderDeviceGl& device, const TextureViewDesc& textureViewDesc)
	: IBvTextureView(textureViewDesc), m_Device(device)
{
}


BvTextureViewGl::~BvTextureViewGl()
{
}