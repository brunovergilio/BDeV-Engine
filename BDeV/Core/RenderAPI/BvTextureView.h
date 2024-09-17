#pragma once


#include "BvTexture.h"


class BvTextureView : public IBvRenderDeviceChild
{
	BV_NOCOPYMOVE(BvTextureView);

public:
	BV_INLINE const TextureViewDesc & GetDesc() const { return m_TextureViewDesc; }
	BV_INLINE BvTexture * GetTexture() const { return m_TextureViewDesc.m_pTexture; }

protected:
	BvTextureView(const TextureViewDesc & textureViewDesc)
		: m_TextureViewDesc(textureViewDesc) {}
	virtual ~BvTextureView() = 0 {}

protected:
	TextureViewDesc m_TextureViewDesc;
};