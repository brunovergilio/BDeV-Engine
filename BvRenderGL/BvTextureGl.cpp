#include "BvTextureGl.h"
#include "BvTypeConversionsGl.h"


BvTextureGl::BvTextureGl(const BvRenderDeviceGl& device, const TextureDesc& textureDesc)
	: BvTexture(textureDesc), m_Device(device)
{
	Create();
}


BvTextureGl::~BvTextureGl()
{
	Destroy();
}


void BvTextureGl::Create()
{
	auto target = GetGlTextureTarget(m_TextureDesc);
	auto format = GetGlFormat(m_TextureDesc.m_Format);
	BvAssert(format != 0, "Format not supported for OpenGL");

	glCreateTextures(target, 1, &m_Texture);
	switch (target)
	{
	case GL_TEXTURE_1D:
		glTextureStorage1D(m_Texture, m_TextureDesc.m_MipLevels, format, m_TextureDesc.m_Size.width);
		break;
	case GL_TEXTURE_1D_ARRAY:
	case GL_TEXTURE_2D:
		glTextureStorage2D(m_Texture, m_TextureDesc.m_MipLevels, format, m_TextureDesc.m_Size.width, m_TextureDesc.m_Size.height);
		break;
	case GL_TEXTURE_2D_MULTISAMPLE:
		glTextureStorage2DMultisample(m_Texture, m_TextureDesc.m_SampleCount, format, m_TextureDesc.m_Size.width, m_TextureDesc.m_Size.height, GL_FALSE);
		break;
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_CUBE_MAP:
	case GL_TEXTURE_CUBE_MAP_ARRAY:
	case GL_TEXTURE_3D:
		glTextureStorage3D(m_Texture, m_TextureDesc.m_MipLevels, format, m_TextureDesc.m_Size.width, m_TextureDesc.m_Size.height, m_TextureDesc.m_Size.depthOrLayerCount);
		break;
	case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
		glTextureStorage3DMultisample(m_Texture, m_TextureDesc.m_SampleCount, format, m_TextureDesc.m_Size.width, m_TextureDesc.m_Size.height, m_TextureDesc.m_Size.depthOrLayerCount, GL_FALSE);
		break;
	}
}


void BvTextureGl::Destroy()
{
	if (m_Texture)
	{
		glDeleteTextures(1, &m_Texture);
		m_Texture = 0;
	}
}