#include "BvShaderVk.h"
#include "BvRenderDeviceVk.h"
#include "BDeV/Core/RenderAPI/BvShaderCompiler.h"


BV_VK_DEVICE_RES_DEF(BvShaderVk)


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


void BvShaderVk::Compile(const ShaderCreateDesc& shaderCreateDesc)
{
	IBvShaderBlob* pBlob = shaderCreateDesc.pShaderCompiler->Compile(shaderCreateDesc);
	if (pBlob)
	{
		m_ShaderBlob.Resize(pBlob->GetBufferSize());
		memcpy(&m_ShaderBlob[0], pBlob->GetBufferPointer(), m_ShaderBlob.Size());
	}
	pBlob->Release();
}


void BvShaderVk::Destroy()
{
}