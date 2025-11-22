#include "BvSwapChainD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BvTextureD3D12.h"
#include "BvTextureViewD3D12.h"
#include "BvGPUFenceD3D12.h"
#include "BvCommandContextD3D12.h"
#include <dxgi1_5.h>
#include "BDeV/Core/System/Window/BvWindow.h"
#include "BDeV/Core/System/Window/BvMonitor.h"


BvSwapChainD3D12::BvSwapChainD3D12(BvRenderDeviceD3D12* pDevice, BvWindow* pWindow, const SwapChainDesc& swapChainParams, BvCommandContextD3D12* pContext)
	: m_pDevice(pDevice), m_SwapChainDesc(swapChainParams), m_pCommandContext(pContext)
{
	Create();
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
	m_SwapChain->Present(vSync ? 1 : 0, m_PresentFlags);
}


void BvSwapChainD3D12::SetWindowMode(SwapChainMode mode, BvMonitor* pMonitor)
{
}


void BvSwapChainD3D12::SetCurrentFence(BvGPUFenceD3D12* pFence, u64 value)
{
}


bool BvSwapChainD3D12::Create()
{
	auto [width, height] = m_pWindow->GetSize();
	auto fi = GetFormatInfo(m_SwapChainDesc.m_Format);

	DXGI_SWAP_CHAIN_DESC1 scd{};
	scd.Width = width;
	scd.Height = height;
	scd.Format = DXGI_FORMAT(fi.m_IsSRGBFormat ? fi.m_SRGBOrLinearVariant : m_SwapChainDesc.m_Format);
	//scd.Stereo = FALSE;
	//scd.SampleDesc.Quality = 0;
	scd.SampleDesc.Count = 1;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_UNORDERED_ACCESS;
	scd.BufferCount = m_SwapChainDesc.m_SwapChainImageCount + (m_SwapChainDesc.m_WindowMode == SwapChainMode::kFullscreen ? 1 : 0);
	scd.Scaling = DXGI_SCALING_NONE;
	scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	scd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ComPtr<IDXGIFactory2> factory2;
	auto hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory2));
	if (FAILED(hr))
	{
		// TODO: Handle error
		return false;
	}

	{
		ComPtr<IDXGIFactory5> factory5;
		hr = factory2.As(&factory5);
		if (SUCCEEDED(hr))
		{
			BOOL allowTearing = FALSE;
			hr = factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
			if (SUCCEEDED(hr) && allowTearing)
			{
				scd.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

				factory2->MakeWindowAssociation(m_pWindow->GetHandle(), DXGI_MWA_NO_ALT_ENTER);
			}
		}
	}

	auto [num, den] = GetRefreshRate(factory2.Get(), scd.Format);
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC scfd{};
	scfd.Windowed = m_SwapChainDesc.m_WindowMode != SwapChainMode::kFullscreen ? TRUE : FALSE;
	scfd.RefreshRate = { num, den };
	scfd.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scfd.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

	if (m_SwapChainDesc.m_WindowMode != SwapChainMode::kFullscreen)
	{
		m_PresentFlags = DXGI_PRESENT_ALLOW_TEARING;
	}

	ComPtr<IDXGISwapChain1> swapChain1;
	auto hWnd = m_pWindow->GetHandle();
	hr = factory2->CreateSwapChainForHwnd(m_pCommandContext->GetCommandQueue(), hWnd, &scd, nullptr/*&scfd*/, nullptr, &swapChain1);
	if (FAILED(hr))
	{
		// TODO: Handle error
		return false;
	}

	hr = swapChain1.As(&m_SwapChain);
	if (FAILED(hr))
	{
		// TODO: Handle error
		return false;
	}

	ComPtr<IDXGIFactory1> factory1;
	hr = factory2->GetParent(IID_PPV_ARGS(&factory1));
	if (SUCCEEDED(hr))
	{
		factory1->MakeWindowAssociation(hWnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
	}

	return true;
}


std::pair<u32, u32> BvSwapChainD3D12::GetRefreshRate(IDXGIFactory2* pFactory2, DXGI_FORMAT format)
{
	auto pMonitor = BvMonitor::FromWindow(m_pWindow);
	auto pAdapter = m_pDevice->GetAdapterHandle();
	ComPtr<IDXGIOutput> output;
	u32 outputIndex = 0;
	BvVector<DXGI_MODE_DESC1> displayModes;
	u32 num{}, den{};
	while (SUCCEEDED(pAdapter->EnumOutputs(outputIndex++, &output)))
	{
		DXGI_OUTPUT_DESC od{};
		ComPtr<IDXGIOutput1> output1;

		if (SUCCEEDED(output.As(&output1))
			&& SUCCEEDED(output1->GetDesc(&od))
			&& pMonitor->GetHandle() == ::MonitorFromRect(&od.DesktopCoordinates, MONITOR_DEFAULTTONEAREST))
		{
			u32 numModes = 0;
			if (SUCCEEDED(output1->GetDisplayModeList1(format, 0, &numModes, nullptr)))
			{
				displayModes.Resize(numModes);
				if (SUCCEEDED(output1->GetDisplayModeList1(format, 0, &numModes, displayModes.Data())))
				{
					output1 = nullptr;
					output = nullptr;
					break;
				}
			}
		}

		output1 = nullptr;
		output = nullptr;
	}

	for (auto& dm : displayModes)
	{
		num = std::max(num, dm.RefreshRate.Numerator);
		den = std::max(den, dm.RefreshRate.Denominator);
	}

	return std::pair<u32, u32>(num, den);
}


void BvSwapChainD3D12::Destroy()
{
}


void BvSwapChainD3D12::Resize()
{
	m_pCommandQueue->WaitIdle();
	if (m_pWindow->IsClosed() || m_pWindow->IsMinimized())
	{
		return;
	}

	BOOL fs;
	if (SUCCEEDED(m_SwapChain->GetFullscreenState(&fs, nullptr) && fs))
	{
		m_SwapChain->SetFullscreenState(false, nullptr);
	}

	auto [w, h] = m_pWindow->GetSize();
	DXGI_SWAP_CHAIN_DESC1 scd{};
	m_SwapChain->GetDesc1(&scd);
	m_SwapChain->ResizeBuffers1(scd.BufferCount, w, h, scd.Format, scd.Flags, nullptr, nullptr);
	//Create();
}


void BvSwapChainD3D12::SetTrueFullscreen(bool value)
{
	if (m_SwapChainDesc.m_WindowMode == SwapChainMode::kFullscreen && value)
	{
		return;
	}

	m_SwapChain->SetFullscreenState(value, nullptr);
}