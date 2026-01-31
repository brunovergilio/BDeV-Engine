#include "BvShaderD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BDeV/Core/RenderAPI/BvShaderCompiler.h"
#include "BDeV/Core/Utils/BvObject.h"


BvShaderD3D12::BvShaderD3D12(BvRenderDeviceD3D12* pDevice, const ShaderDesc& shaderDesc)
	: m_pDevice(pDevice), m_ShaderStage(shaderDesc.m_ShaderStage), m_pEntryPoint(shaderDesc.m_pEntryPoint)
{
	m_ShaderBlob.Resize(shaderDesc.m_ByteCodeSize);
	memcpy(&m_ShaderBlob[0], shaderDesc.m_pByteCode, shaderDesc.m_ByteCodeSize);
}


BvShaderD3D12::~BvShaderD3D12()
{
}


void BvShaderD3D12::Destroy()
{
}