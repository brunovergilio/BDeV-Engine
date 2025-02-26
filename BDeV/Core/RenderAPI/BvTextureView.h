#pragma once


#include "BvTexture.h"


BV_OBJECT_DEFINE_ID(BvTextureView, "6178f332-f7bc-4465-bf96-a6713c733ece");
class BvTextureView : public IBvRenderDeviceObject
{
	BV_NOCOPYMOVE(BvTextureView);

public:
	BV_INLINE const TextureViewDesc & GetDesc() const { return m_TextureViewDesc; }
	BV_INLINE BvTexture * GetTexture() const { return m_TextureViewDesc.m_pTexture; }

protected:
	BvTextureView(const TextureViewDesc & textureViewDesc)
		: m_TextureViewDesc(textureViewDesc) {}
	~BvTextureView() {}

protected:
	TextureViewDesc m_TextureViewDesc;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvTextureView);