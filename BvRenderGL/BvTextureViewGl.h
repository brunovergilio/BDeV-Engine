#pragma once


#include "BDeV/RenderAPI/BvTextureView.h"
#include "BvTextureGl.h"


class BvRenderDeviceGl;


class BvTextureViewGl final : public BvTextureView
{
	BV_NOCOPYMOVE(BvTextureViewGl);

public:
	BvTextureViewGl(const BvRenderDeviceGl& device, const TextureViewDesc& textureViewDesc);
	~BvTextureViewGl() override;

private:
	const BvRenderDeviceGl& m_Device;
};