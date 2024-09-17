#include "BvShaderBlob.h"


BvShaderBlob::BvShaderBlob(BvVector<u8>& shaderBlob)
	: m_ShaderBlob(std::move(shaderBlob))
{
}


BvShaderBlob::BvShaderBlob(const char* pErrorString)
{
	auto size = strlen(pErrorString);
	m_ShaderBlob.Resize(size + 1);
	memcpy(&m_ShaderBlob[0], pErrorString, size);
}


BvShaderBlob::~BvShaderBlob()
{
}