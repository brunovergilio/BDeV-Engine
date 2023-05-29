#include "BvSwapChainGl.h"
#include "BvContextGl.h"

BvSwapChainGl::BvSwapChainGl(BvWindow* pWindow, const SwapChainDesc& swapChainParams)
	: BvSwapChain(pWindow, swapChainParams), m_pContext(new BvContextGl(pWindow))
{
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
}