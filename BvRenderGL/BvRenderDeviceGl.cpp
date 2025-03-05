#include "BvRenderDeviceGl.h"
#include "BvRenderEngineGl.h"
#include "BvSwapChainGl.h"


BvRenderDeviceGl::BvRenderDeviceGl(const DeviceCreateDesc& deviceDesc, const BvGPUInfoGl& gpuInfo)
	: m_GPUInfo(gpuInfo)
{
}


BvRenderDeviceGl::~BvRenderDeviceGl()
{
}


void BvRenderDeviceGl::Create(const DeviceCreateDesc& deviceCreateDesc)
{
}


void BvRenderDeviceGl::Destroy()
{
}


IBvSwapChain* BvRenderDeviceGl::CreateSwapChain(BvWindow* pWindow, const SwapChainDesc& swapChainDesc, BvCommandQueue& commandQueue)
{
	return new BvSwapChainGl(*this, pWindow, swapChainDesc);
}


IBvBuffer* BvRenderDeviceGl::CreateBuffer(const BufferDesc& desc)
{
	return nullptr;
}


IBvBufferView* BvRenderDeviceGl::CreateBufferView(const BufferViewDesc& desc)
{
	return nullptr;
}


IBvTexture* BvRenderDeviceGl::CreateTexture(const TextureDesc& desc)
{
	return nullptr;
}


IBvTextureView* BvRenderDeviceGl::CreateTextureView(const TextureViewDesc& desc)
{
	return nullptr;
}


BvSemaphore* BvRenderDeviceGl::CreateSemaphore(const u64 initialValue)
{
	return nullptr;
}


IBvRenderPass* BvRenderDeviceGl::CreateRenderPass(const RenderPassDesc& renderPassDesc)
{
	return nullptr;
}


BvCommandPool* BvRenderDeviceGl::CreateCommandPool(const CommandPoolDesc& commandPoolDesc)
{
	return nullptr;
}


IBvShaderResourceLayout* BvRenderDeviceGl::CreateShaderResourceLayout(const ShaderResourceLayoutDesc& shaderResourceLayoutDesc)
{
	return nullptr;
}


BvShaderResourceSetPool* BvRenderDeviceGl::CreateShaderResourceSetPool(const ShaderResourceSetPoolDesc& shaderResourceSetPoolDesc /*= ShaderResourceSetPoolDesc()*/)
{
	return nullptr;
}


IBvGraphicsPipelineState* BvRenderDeviceGl::CreateGraphicsPipeline(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc)
{
	return nullptr;
}


void BvRenderDeviceGl::WaitIdle() const
{
}