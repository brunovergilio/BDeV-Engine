#pragma once


#include "BvTexture.h"


enum class TextureViewType : u8
{
	kTexture1D,
	kTexture1DArray,
	kTexture2D,
	kTexture2DArray,
	kTextureCube,
	kTextureCubeArray,
	kTexture3D,
};


struct TextureViewDesc
{
	BvTexture * m_pTexture = nullptr;
	SubresourceDesc m_SubresourceDesc;
	TextureViewType m_ViewType = TextureViewType::kTexture2D;
	Format m_Format = Format::kUnknown;
};


class BvTextureView
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