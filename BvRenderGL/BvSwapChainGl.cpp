#include "BvSwapChainGl.h"
#include "BvContextGl.h"

BvSwapChainGl::BvSwapChainGl(const BvRenderDeviceGl& device, BvWindow* pWindow, const SwapChainDesc& swapChainParams)
	: IBvSwapChain(pWindow, swapChainParams), m_Device(device), m_pContext(new BvContextGl(pWindow))
{
	m_SwapChainDesc.m_SwapChainImageCount = 2;
}


BvSwapChainGl::~BvSwapChainGl()
{
	delete m_pContext;
}


void BvSwapChainGl::Create()
{
}


void BvSwapChainGl::Destroy()
{

}


void BvSwapChainGl::Present(bool vSync)
{
	m_pContext->SwapBuffers(i32(vSync));
}