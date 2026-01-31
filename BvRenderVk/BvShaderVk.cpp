#include "BvShaderVk.h"
#include "BvRenderDeviceVk.h"
#include "BDeV/Core/RenderAPI/BvShaderCompiler.h"


BvShaderVk::BvShaderVk(BvRenderDeviceVk* pDevice, const ShaderDesc& shaderDesc)
	: m_pDevice(pDevice), m_ShaderStage(shaderDesc.m_ShaderStage), m_pEntryPoint(shaderDesc.m_pEntryPoint)
{
	m_ShaderBlob.Resize(shaderDesc.m_ByteCodeSize);
	memcpy(&m_ShaderBlob[0], shaderDesc.m_pByteCode, shaderDesc.m_ByteCodeSize);
}


BvShaderVk::~BvShaderVk()
{
	Destroy();
}


void BvShaderVk::Destroy()
{
	if (m_ShaderBlob.Size())
	{
		m_ShaderBlob.Destroy();
	}
}