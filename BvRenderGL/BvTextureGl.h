#pragma once


#include "BDeV/RenderAPI/BvTexture.h"
#include "BvCommonGl.h"


class BvRenderDeviceGl;
class BvSwapChainGl;


class BvTextureGl final : public BvTexture
{
	BV_NOCOPYMOVE(BvTextureGl);

public:
	BvTextureGl(const BvRenderDeviceGl& device, const TextureDesc& textureDesc);
	~BvTextureGl();

	void Create();
	void Destroy();

	BV_INLINE ClassType GetClassType() const override { return ClassType::kTexture; }
	BV_INLINE GLuint GetHandle() const { return m_Texture; }

protected:
	const BvRenderDeviceGl& m_Device;
	GLuint m_Texture = 0;
};