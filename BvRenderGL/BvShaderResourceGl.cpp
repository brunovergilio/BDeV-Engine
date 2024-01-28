#include "BvShaderResourceGl.h"


BvShaderResourceLayoutGl::BvShaderResourceLayoutGl(const BvRenderDeviceGl& device, const ShaderResourceLayoutDesc& shaderResourceLayoutDesc)
	: BvShaderResourceLayout(shaderResourceLayoutDesc), m_Device(device)
{
}


BvShaderResourceLayoutGl::~BvShaderResourceLayoutGl()
{
}


BvShaderResourceParamsGl::BvShaderResourceParamsGl(const BvRenderDeviceGl& device, const BvShaderResourceLayoutGl& layout)
	: m_Device(device), m_Layout(layout)
{
	Create();
}


BvShaderResourceParamsGl::~BvShaderResourceParamsGl()
{
	Destroy();
}


void BvShaderResourceParamsGl::Create()
{
}


void BvShaderResourceParamsGl::Destroy()
{
}


void BvShaderResourceParamsGl::SetBuffers(const u32 binding, const u32 count, const BvBufferView* const* const ppBuffers, const u32 startIndex /*= 0*/)
{
}


void BvShaderResourceParamsGl::SetTextures(const u32 binding, const u32 count, const BvTextureView* const* const ppTextures, const u32 startIndex /*= 0*/)
{
}


void BvShaderResourceParamsGl::SetSamplers(const u32 binding, const u32 count, const BvSampler* const* const ppSamplers, const u32 startIndex /*= 0*/)
{
}


void BvShaderResourceParamsGl::Update()
{
}


void BvShaderBufferVariableGl::SetBuffers(const u32 count, const BvBufferView* const* const ppBuffers, const u32 startIndex /*= 0*/)
{
}


void BvShaderBufferVariableGl::SetTextures(const u32 count, const BvTextureView* const* const ppTextures, const u32 startIndex /*= 0*/)
{
}


void BvShaderBufferVariableGl::SetSamplers(const u32 count, const BvSampler* const* const ppSamplers, const u32 startIndex /*= 0*/)
{
}


void BvShaderImageVariableGl::SetBuffers(const u32 count, const BvBufferView* const* const ppBuffers, const u32 startIndex /*= 0*/)
{
}


void BvShaderImageVariableGl::SetTextures(const u32 count, const BvTextureView* const* const ppTextures, const u32 startIndex /*= 0*/)
{
}


void BvShaderImageVariableGl::SetSamplers(const u32 count, const BvSampler* const* const ppSamplers, const u32 startIndex /*= 0*/)
{
}


void BvShaderBufferViewVariableGl::SetBuffers(const u32 count, const BvBufferView* const* const ppBuffers, const u32 startIndex /*= 0*/)
{
}


void BvShaderBufferViewVariableGl::SetTextures(const u32 count, const BvTextureView* const* const ppTextures, const u32 startIndex /*= 0*/)
{
}


void BvShaderBufferViewVariableGl::SetSamplers(const u32 count, const BvSampler* const* const ppSamplers, const u32 startIndex /*= 0*/)
{
}