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


BvSwapChain* BvRenderDeviceGl::CreateSwapChain(BvWindow* pWindow, const SwapChainDesc& swapChainDesc, BvCommandQueue& commandQueue)
{
	return new BvSwapChainGl(*this, pWindow, swapChainDesc);
}


BvBuffer* BvRenderDeviceGl::CreateBuffer(const BufferDesc& desc)
{
	return nullptr;
}


BvBufferView* BvRenderDeviceGl::CreateBufferView(const BufferViewDesc& desc)
{
	return nullptr;
}


BvTexture* BvRenderDeviceGl::CreateTexture(const TextureDesc& desc)
{
	return nullptr;
}


BvTextureView* BvRenderDeviceGl::CreateTextureView(const TextureViewDesc& desc)
{
	return nullptr;
}


BvSemaphore* BvRenderDeviceGl::CreateSemaphore(const u64 initialValue)
{
	return nullptr;
}


BvRenderPass* BvRenderDeviceGl::CreateRenderPass(const RenderPassDesc& renderPassDesc)
{
	return nullptr;
}


BvCommandPool* BvRenderDeviceGl::CreateCommandPool(const CommandPoolDesc& commandPoolDesc)
{
	return nullptr;
}


BvShaderResourceLayout* BvRenderDeviceGl::CreateShaderResourceLayout(const ShaderResourceLayoutDesc& shaderResourceLayoutDesc)
{
	return nullptr;
}


BvShaderResourceSetPool* BvRenderDeviceGl::CreateShaderResourceSetPool(const ShaderResourceSetPoolDesc& shaderResourceSetPoolDesc /*= ShaderResourceSetPoolDesc()*/)
{
	return nullptr;
}


BvGraphicsPipelineState* BvRenderDeviceGl::CreateGraphicsPipeline(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc)
{
	return nullptr;
}


void BvRenderDeviceGl::WaitIdle() const
{
}