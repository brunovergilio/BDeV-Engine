#pragma once


#include "BvRenderCommon.h"
#include "BDeV/Core/Utils/BvUtils.h"


class BvTexture
{
	BV_NOCOPYMOVE(BvTexture);

public:
	enum class ClassType : u8
	{
		kTexture,
		kSwapChainTexture
	};

	BV_INLINE const TextureDesc & GetDesc() const { return m_TextureDesc; }

	virtual ClassType GetClassType() const = 0;

protected:
	BvTexture(const TextureDesc & textureDesc)
		: m_TextureDesc(textureDesc) {}
	virtual ~BvTexture() = 0 {}

protected:
	TextureDesc m_TextureDesc;
};