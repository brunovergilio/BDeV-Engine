#include "BvShaderVk.h"
#include "BvRenderDeviceVk.h"
#include "BDeV/Core/RenderAPI/BvShaderCompiler.h"


BvShaderVk::BvShaderVk(BvRenderDeviceVk* pDevice, const ShaderCreateDesc& shaderCreateDesc)
	: m_pDevice(pDevice), m_ShaderStage(shaderCreateDesc.m_ShaderStage), m_pEntryPoint(shaderCreateDesc.m_pEntryPoint)
{
	if (shaderCreateDesc.m_pByteCode)
	{
		m_ShaderBlob.Resize(shaderCreateDesc.m_ByteCodeSize);
		memcpy(&m_ShaderBlob[0], shaderCreateDesc.m_pByteCode, shaderCreateDesc.m_ByteCodeSize);
	}
	else
	{
		if (shaderCreateDesc.m_pSourceCode && shaderCreateDesc.pShaderCompiler)
		{
			Compile(shaderCreateDesc);
		}
	}
}


BvShaderVk::~BvShaderVk()
{
}


IBvRenderDevice* BvShaderVk::GetDevice()
{
	return m_pDevice;
}


void BvShaderVk::Compile(const ShaderCreateDesc& shaderCreateDesc)
{
	IBvShaderBlob* pBlob = nullptr;
	if (shaderCreateDesc.pShaderCompiler->Compile(shaderCreateDesc, &pBlob))
	{
		m_ShaderBlob.Resize(pBlob->GetBufferSize());
		memcpy(&m_ShaderBlob[0], pBlob->GetBufferPointer(), m_ShaderBlob.Size());
	}
}