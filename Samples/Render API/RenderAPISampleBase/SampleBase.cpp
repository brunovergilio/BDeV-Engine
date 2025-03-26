#include "SampleBase.h"


void SampleBase::Initialize()
{
	m_App.Initialize();
	m_App.RegisterRawInput(true, true);
	m_RenderLib.Open("BvRenderVk.dll");

	using EngineFnType = IBvRenderEngine*(*)();
	auto pFnEngine = m_RenderLib.GetProcAddressT<EngineFnType>("CreateRenderEngine");
	m_pEngine = pFnEngine();

	m_Device = m_pEngine->CreateRenderDevice(m_RenderDeviceDesc);
	m_Context = m_Device->GetGraphicsContext(0);

	WindowDesc windowDesc;
	windowDesc.m_X += 100;
	windowDesc.m_Y += 100;
	m_pWindow = m_App.CreateWindow(windowDesc);

	SwapChainDesc swapChainDesc;
	swapChainDesc.m_Format = Format::kRGBA8_UNorm_SRGB;
	m_SwapChain = m_Device->CreateSwapChain(m_pWindow, swapChainDesc, m_Context);

	m_Overlay.Initialize(m_Device, m_Context);
	OnInitialize();
	OnInitializeUI();
	m_Curr = BvTime::GetCurrentTimestampInUs();
}


void SampleBase::Update()
{
	m_Dt = f32(m_Curr - m_Prev) * 0.000001f;
	m_FPSCounter.Update(m_Dt);
	m_Prev = m_Curr;
	m_Curr = BvTime::GetCurrentTimestampInUs();

	m_App.ProcessOSEvents();
	if (m_pWindow->IsClosed())
	{
		m_IsDone = true;
		return;
	}

	OnUpdate();
	if (m_UseOverlay && m_Overlay.Update(m_Dt, m_pWindow))
	{
		OnUpdateUI();
	}
}


void SampleBase::Render()
{
	if (m_IsDone)
	{
		return;
	}

	OnRender();
}


void SampleBase::Shutdown()
{
	OnShutdown();

	m_Overlay.Shutdown();
	m_SwapChain.Reset();
	m_Context.Reset();
	m_App.DestroyWindow(m_pWindow);
	m_Device.Reset();

	m_App.Shutdown();
}


void SampleBase::OnInitializeUI()
{
	m_Overlay.SetupPipeline(m_SwapChain->GetDesc().m_Format);
}


void SampleBase::OnUpdateUI()
{
}


void SampleBase::OnRenderUI()
{
	m_Overlay.Render();
}