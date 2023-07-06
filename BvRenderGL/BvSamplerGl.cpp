#include "BvSamplerGl.h"
#include "BvTypeConversionsGl.h"
#include "BvRenderDeviceGl.h"


BvSamplerGl::BvSamplerGl(const BvRenderDeviceGl& device, const SamplerDesc& samplerDesc)
	: BvSampler(samplerDesc), m_Device(device)
{
	Create();
}


BvSamplerGl::~BvSamplerGl()
{
	Destroy();
}


void BvSamplerGl::Create()
{
	glCreateSamplers(1, &m_Sampler);
	glSamplerParameteri(m_Sampler, GL_TEXTURE_MIN_FILTER, GetGlMinFilter(m_SamplerDesc.m_MinFilter, m_SamplerDesc.m_MipmapMode));
	glSamplerParameteri(m_Sampler, GL_TEXTURE_MAG_FILTER, GetGlMagFilter(m_SamplerDesc.m_MagFilter));
	glSamplerParameteri(m_Sampler, GL_TEXTURE_WRAP_S, GetGlAddressMode(m_SamplerDesc.m_AddressModeU));
	glSamplerParameteri(m_Sampler, GL_TEXTURE_WRAP_T, GetGlAddressMode(m_SamplerDesc.m_AddressModeV));
	glSamplerParameteri(m_Sampler, GL_TEXTURE_WRAP_R, GetGlAddressMode(m_SamplerDesc.m_AddressModeW));
	glSamplerParameteri(m_Sampler, GL_TEXTURE_COMPARE_MODE, m_SamplerDesc.m_CompareEnable ? GL_COMPARE_REF_TO_TEXTURE : GL_NONE);
	glSamplerParameteri(m_Sampler, GL_TEXTURE_COMPARE_FUNC, GetGlCompareOp(m_SamplerDesc.m_CompareOp));
	if (m_Device.GetGPUInfo().m_ExtendedFeatures.textureFilterAnisotropic)
	{
		float maxSupportedAnisotropy = 1.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxSupportedAnisotropy);
		if (maxSupportedAnisotropy < m_SamplerDesc.m_MaxAnisotropy)
		{
			m_SamplerDesc.m_MaxAnisotropy = maxSupportedAnisotropy;
		}
		glSamplerParameterf(m_Sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxSupportedAnisotropy);
	}
	glSamplerParameterf(m_Sampler, GL_TEXTURE_LOD_BIAS, m_SamplerDesc.m_MipLodBias);
	glSamplerParameterf(m_Sampler, GL_TEXTURE_MIN_LOD, m_SamplerDesc.m_MinLod);
	glSamplerParameterf(m_Sampler, GL_TEXTURE_MAX_LOD, m_SamplerDesc.m_MaxLod);
	glSamplerParameterfv(m_Sampler, GL_TEXTURE_BORDER_COLOR, m_SamplerDesc.m_BorderColor);
}


void BvSamplerGl::Destroy()
{
	if (m_Sampler)
	{
		glDeleteSamplers(1, &m_Sampler);
		m_Sampler = 0;
	}
}