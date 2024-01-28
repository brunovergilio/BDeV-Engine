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
	BV_INLINE GLenum GetInternalFormat() const { return m_InternalFormat; }
	BV_INLINE GLenum GetTarget() const { return m_Target; }

protected:
	const BvRenderDeviceGl& m_Device;
	GLuint m_Texture = 0;
	GLenum m_InternalFormat = 0;
	GLenum m_Target = 0;
};