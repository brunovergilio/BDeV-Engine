#pragma once


#include "BDeV/Core/RenderAPI/BvTexture.h"
#include "BvCommonGl.h"


class BvRenderDeviceGl;
class BvSwapChainGl;


class BvTextureGl final : public IBvTexture, public IBvResourceGl
{
	BV_NOCOPYMOVE(BvTextureGl);

public:
	BvTextureGl(BvRenderDeviceGl* pDevice, const TextureDesc& textureDesc, bool isSwapChain = false);
	~BvTextureGl();

	void Create();
	void Destroy();

	BV_INLINE const TextureDesc& GetDesc() const override { return m_TextureDesc; }
	BV_INLINE GLuint GetHandle() const { return m_Texture; }
	BV_INLINE GLenum GetInternalFormat() const { return m_InternalFormat; }
	BV_INLINE GLenum GetTarget() const { return m_Target; }

protected:
	BvRenderDeviceGl* m_pDevice = nullptr;
	TextureDesc m_TextureDesc;
	GLuint m_Texture = 0;
	GLenum m_InternalFormat = 0;
	GLenum m_Target = 0;
};