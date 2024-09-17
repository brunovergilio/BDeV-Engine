#pragma once


#include "BvRenderCommon.h"
#include "BDeV/Core/Utils/BvUtils.h"


class BvTexture : public IBvRenderDeviceChild
{
	BV_NOCOPYMOVE(BvTexture);

public:
	BV_INLINE const TextureDesc & GetDesc() const { return m_TextureDesc; }

protected:
	BvTexture(const TextureDesc & textureDesc)
		: m_TextureDesc(textureDesc) {}
	virtual ~BvTexture() = 0 {}

protected:
	TextureDesc m_TextureDesc;
};