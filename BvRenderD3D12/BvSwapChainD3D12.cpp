#include "BvSwapChainD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BvTextureD3D12.h"
#include "BvTextureViewD3D12.h"
#include "BvGPUFenceD3D12.h"
#include "BvCommandContextD3D12.h"
#include <dxgi1_5.h>
#include "BDeV/Core/System/Window/BvWindow.h"
#include "BDeV/Core/System/Window/BvMonitor.h"
#include "BDeV/Core/RenderAPI/BvRenderAPIUtils.h"


BvSwapChainD3D12::BvSwapChainD3D12(BvRenderDeviceD3D12* pDevice, BvWindow* pWindow, const SwapChainDesc& swapChainParams, BvCommandContextD3D12* pContext,
	ComPtr<IDXGISwapChain3>& swapChain, BvVector<ID3D12Resource*>& textures, UINT presentFlags)
	: m_pDevice(pDevice), m_SwapChainDesc(swapChainParams), m_pCommandContext(pContext), m_pWindow(pWindow), m_SwapChain(std::move(swapChain)),
	m_PresentFlags(presentFlags), m_Width(pWindow->GetWidth()), m_Height(pWindow->GetHeight())
{
	CreateTextureResources(textures);

	AcquireImage();
}


BvSwapChainD3D12::~BvSwapChainD3D12()
{
	Destroy();
}


void BvSwapChainD3D12::AcquireImage()
{
	m_CurrImageIndex = m_SwapChain->GetCurrentBackBufferIndex();
}


void BvSwapChainD3D12::Present(bool vSync)
{
	m_SwapChain->Present(vSync ? 1 : 0, m_SwapChainDesc.m_WindowMode != SwapChainMode::kFullscreen ? m_PresentFlags : 0);

	auto [w, h] = m_pWindow->GetSize();
	if (m_Width != w || m_Height != h)
	{
		Resize();
	}

	AcquireImage();
}


void BvSwapChainD3D12::SetWindowMode(SwapChainMode mode, BvMonitor* pMonitor)
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
	
	m_pWindow->SetFullscreen(mode != SwapChainMode::kWindowed, pMonitor);
	Resize();

	m_SwapChainDesc.m_WindowMode = mode;
	SetTrueFullscreen(mode == SwapChainMode::kFullscreen);
}


//std::pair<u32, u32> BvSwapChainD3D12::GetRefreshRate(IDXGIFactory2* pFactory2, DXGI_FORMAT format)
//{
//	auto pMonitor = BvMonitor::FromWindow(m_pWindow);
//	auto pAdapter = m_pDevice->GetAdapterHandle();
//	ComPtr<IDXGIOutput> output;
//	u32 outputIndex = 0;
//	BvVector<DXGI_MODE_DESC1> displayModes;
//	u32 num{}, den{};
//	while (SUCCEEDED(pAdapter->EnumOutputs(outputIndex++, &output)))
//	{
//		DXGI_OUTPUT_DESC od{};
//		ComPtr<IDXGIOutput1> output1;
//
//		if (SUCCEEDED(output.As(&output1))
//			&& SUCCEEDED(output1->GetDesc(&od))
//			&& pMonitor->GetHandle() == ::MonitorFromRect(&od.DesktopCoordinates, MONITOR_DEFAULTTONEAREST))
//		{
//			u32 numModes = 0;
//			if (SUCCEEDED(output1->GetDisplayModeList1(format, 0, &numModes, nullptr)))
//			{
//				displayModes.Resize(numModes);
//				if (SUCCEEDED(output1->GetDisplayModeList1(format, 0, &numModes, displayModes.Data())))
//				{
//					output1 = nullptr;
//					output = nullptr;
//					break;
//				}
//			}
//		}
//
//		output1 = nullptr;
//		output = nullptr;
//	}
//
//	for (auto& dm : displayModes)
//	{
//		num = std::max(num, dm.RefreshRate.Numerator);
//		den = std::max(den, dm.RefreshRate.Denominator);
//	}
//
//	return std::pair<u32, u32>(num, den);
//}


void BvSwapChainD3D12::CreateTextureResources(BvVector<ID3D12Resource*>& textures)
{
	auto [w, h] = m_pWindow->GetSize();

	TextureDesc textureDesc;
	textureDesc.m_Size = { w, h, 1 };
	textureDesc.m_Format = m_SwapChainDesc.m_Format;
	textureDesc.m_UsageFlags = TextureUsage::kRenderTarget | TextureUsage::kShaderResource;

	TextureViewDesc textureViewDesc;
	textureViewDesc.m_Format = textureDesc.m_Format;

	m_SwapChainTextures.Reserve(textures.Size());
	m_SwapChainTextureViews.Reserve(textures.Size());
	for (auto i = 0u; i < textures.Size(); i++)
	{
		m_SwapChainTextures.EmplaceBack(BV_RC_CREATE(BvTextureD3D12, m_pDevice, textureDesc, textures[i]));

		textureViewDesc.m_pTexture = m_SwapChainTextures[i];
		m_SwapChainTextureViews.EmplaceBack(BV_RC_CREATE(BvTextureViewD3D12, m_pDevice, textureViewDesc));
	}
}


void BvSwapChainD3D12::DestroyTextureResources()
{
	for (auto i = 0u; i < m_SwapChainTextures.Size(); i++)
	{
		m_SwapChainTextureViews[i]->Release();
		m_SwapChainTextures[i]->Release();
	}
	m_SwapChainTextureViews.Clear();
	m_SwapChainTextures.Clear();
}


void BvSwapChainD3D12::Destroy()
{
	SetTrueFullscreen(false);

	DestroyTextureResources();
}


void BvSwapChainD3D12::Resize()
{
	if (m_pWindow->IsClosed() || m_pWindow->IsMinimized())
	{
		return;
	}
	m_pCommandContext->GetCommandQueue()->WaitIdle();
	DestroyTextureResources();

	BOOL fs{};
	if (SUCCEEDED(m_SwapChain->GetFullscreenState(&fs, nullptr)) && fs)
	{
		m_SwapChain->SetFullscreenState(false, nullptr);
	}

	auto [w, h] = m_pWindow->GetSize();
	DXGI_SWAP_CHAIN_DESC1 scd{};
	m_SwapChain->GetDesc1(&scd);
	m_SwapChain->ResizeBuffers(0, 0, 0, scd.Format, scd.Flags);

	m_Width = w;
	m_Height = h;

	BvVector<ID3D12Resource*> backBuffers(scd.BufferCount);
	for (auto i = 0; i < backBuffers.Size(); i++)
	{
		m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));
	}
	CreateTextureResources(backBuffers);
}


void BvSwapChainD3D12::SetTrueFullscreen(bool value)
{
	if ((m_SwapChainDesc.m_WindowMode == SwapChainMode::kFullscreen && value) || (m_SwapChainDesc.m_WindowMode != SwapChainMode::kFullscreen && !value))
	{
		return;
	}

	if (FAILED(m_SwapChain->SetFullscreenState(value, nullptr)) && value)
	{
		m_SwapChainDesc.m_WindowMode = SwapChainMode::kBorderlessFullscreen;
	}
}