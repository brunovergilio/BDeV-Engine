#include "BvTextureGl.h"
#include "BvTypeConversionsGl.h"


BvTextureGl::BvTextureGl(const BvRenderDeviceGl& device, const TextureDesc& textureDesc)
	: BvTexture(textureDesc), m_Device(device), m_InternalFormat(GetGlFormat(m_TextureDesc.m_Format)), m_Target(GetGlTextureTarget(m_TextureDesc))
{
	BvAssert(m_InternalFormat != 0, "Format not supported for OpenGL");
	Create();
}


BvTextureGl::~BvTextureGl()
{
	Destroy();
}


void BvTextureGl::Create()
{
	GLsizei levels = (GLsizei)m_TextureDesc.m_MipLevels;
	GLsizei width = (GLsizei)m_TextureDesc.m_Size.width;
	GLsizei height = (GLsizei)m_TextureDesc.m_Size.height;
	GLsizei depth = (GLsizei)m_TextureDesc.m_Size.width;

	// Original
	//glGenTextures(1, &m_Texture);
	//glBindTexture(m_Target, m_Texture);
	//switch (m_Target)
	//{
	//case GL_TEXTURE_1D:
	//	glTexStorage1D(m_Target, levels, m_InternalFormat, width);
	//	break;
	//case GL_TEXTURE_1D_ARRAY:
	//case GL_TEXTURE_2D:
	//	glTexStorage2D(m_Target, levels, m_InternalFormat, width, height);
	//	break;
	//case GL_TEXTURE_2D_MULTISAMPLE:
	//	glTexStorage2DMultisample(m_Target, m_TextureDesc.m_SampleCount, m_InternalFormat, width, height, GL_FALSE);
	//	break;
	//case GL_TEXTURE_2D_ARRAY:
	//case GL_TEXTURE_CUBE_MAP:
	//case GL_TEXTURE_CUBE_MAP_ARRAY:
	//case GL_TEXTURE_3D:
	//	glTexStorage3D(m_Target, levels, m_InternalFormat, width, height, depth);
	//	break;
	//case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
	//	glTexStorage3DMultisample(m_Target, m_TextureDesc.m_SampleCount, m_InternalFormat, width, height, depth, GL_FALSE);
	//	break;
	//}

	//if (m_TextureDesc.m_Format == Format::kA8_UNorm)
	//{
	//	glTexParameteri(m_Target, GL_TEXTURE_SWIZZLE_R, GL_ZERO);
	//	glTexParameteri(m_Target, GL_TEXTURE_SWIZZLE_G, GL_ZERO);
	//	glTexParameteri(m_Target, GL_TEXTURE_SWIZZLE_B, GL_ZERO);
	//	glTexParameteri(m_Target, GL_TEXTURE_SWIZZLE_A, GL_RED);
	//}
	//else if (m_TextureDesc.m_Format == Format::kBGRA8_UNorm)
	//{
	//	glTexParameteri(m_Target, GL_TEXTURE_SWIZZLE_R, GL_BLUE);
	//	glTexParameteri(m_Target, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
	//	glTexParameteri(m_Target, GL_TEXTURE_SWIZZLE_B, GL_RED);
	//	glTexParameteri(m_Target, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
	//}

	// DSA
	glCreateTextures(m_Target, 1, &m_Texture);
	switch (m_Target)
	{
	case GL_TEXTURE_1D:
		glTextureStorage1D(m_Texture, levels, m_InternalFormat, width);
		break;
	case GL_TEXTURE_1D_ARRAY:
	case GL_TEXTURE_2D:
		glTextureStorage2D(m_Texture, levels, m_InternalFormat, width, height);
		break;
	case GL_TEXTURE_2D_MULTISAMPLE:
		glTextureStorage2DMultisample(m_Texture, m_TextureDesc.m_SampleCount, m_InternalFormat, width, height, GL_FALSE);
		break;
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_CUBE_MAP:
	case GL_TEXTURE_CUBE_MAP_ARRAY:
	case GL_TEXTURE_3D:
		glTextureStorage3D(m_Texture, levels, m_InternalFormat, width, height, depth);
		break;
	case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
		glTextureStorage3DMultisample(m_Texture, m_TextureDesc.m_SampleCount, m_InternalFormat, width, height, depth, GL_FALSE);
		break;
	}

	if (m_TextureDesc.m_Format == Format::kA8_UNorm)
	{
		glTextureParameteri(m_Texture, GL_TEXTURE_SWIZZLE_R, GL_ZERO);
		glTextureParameteri(m_Texture, GL_TEXTURE_SWIZZLE_G, GL_ZERO);
		glTextureParameteri(m_Texture, GL_TEXTURE_SWIZZLE_B, GL_ZERO);
		glTextureParameteri(m_Texture, GL_TEXTURE_SWIZZLE_A, GL_RED);
	}
	else if (m_TextureDesc.m_Format == Format::kBGRA8_UNorm)
	{
		glTextureParameteri(m_Texture, GL_TEXTURE_SWIZZLE_R, GL_BLUE);
		glTextureParameteri(m_Texture, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
		glTextureParameteri(m_Texture, GL_TEXTURE_SWIZZLE_B, GL_RED);
		glTextureParameteri(m_Texture, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
	}

	glBindTexture(m_Target, 0);
}


void BvTextureGl::Destroy()
{
	if (m_Texture)
	{
		glDeleteTextures(1, &m_Texture);
		m_Texture = 0;
	}
}