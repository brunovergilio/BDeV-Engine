#include "BvSwapChainGl.h"
#include "BvContextGl.h"
#include "BvCommandContextGl.h"
#include "BDeV/Core/System/Window/BvWindow.h"
#include "BDeV/Core/System/Window/BvMonitor.h"


BvSwapChainGl::BvSwapChainGl(BvRenderDeviceGl* pDevice, BvWindow* pWindow, const SwapChainDesc& swapChaindesc, BvCommandContextGl* pCommandContext)
	: m_SwapChainDesc(swapChaindesc), m_pDevice(pDevice), m_Context(pWindow), m_pWindow(pWindow),
	m_pCommandContext(static_cast<BvCommandContextGl*>(pCommandContext))
{
	m_SwapChainDesc.m_SwapChainImageCount = 2;
}


BvSwapChainGl::~BvSwapChainGl()
{
}


void BvSwapChainGl::Create()
{
}


void BvSwapChainGl::Destroy()
{
}


void BvSwapChainGl::Present(bool vSync)
{
	if (m_SwapChainDesc.m_VSync != vSync)
	{
		m_SwapChainDesc.m_VSync = vSync;
	}

	m_Context.SwapBuffers(i32(vSync));
}


void BvSwapChainGl::SetWindowMode(SwapChainMode mode, BvMonitor* pMonitor)
{
	auto pCurrMonitor = m_pWindow->GetWindowDesc().m_pMonitor;
	if (!pMonitor)
	{
		pMonitor = pCurrMonitor;
	}

	if (m_SwapChainDesc.m_WindowMode == mode)
	{
		if (mode == SwapChainMode::kWindowed || pMonitor == pCurrMonitor)
		{
			return;
		}
	}

	m_SwapChainDesc.m_WindowMode = mode;

	m_pWindow->SetFullscreen(mode != SwapChainMode::kWindowed, pMonitor);
}