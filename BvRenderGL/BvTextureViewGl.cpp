#include "BvTextureViewGl.h"


BvTextureViewGl::BvTextureViewGl(const BvRenderDeviceGl& device, const TextureViewDesc& textureViewDesc)
	: BvTextureView(textureViewDesc), m_Device(device)
{
}


BvTextureViewGl::~BvTextureViewGl()
{
}