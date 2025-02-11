#pragma once


#include "BvRenderCommon.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BvRenderDeviceObject.h"


BV_OBJECT_DEFINE_ID(BvTexture, "b3aad9aa-5a42-434c-87c8-cf744c8eeefa");
class BvTexture : public IBvRenderDeviceObject
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
BV_OBJECT_ENABLE_ID_OPERATOR(BvTexture);