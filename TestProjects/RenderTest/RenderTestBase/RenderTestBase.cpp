//#include "RenderTestBase.h"
//#include "BvRenderVK/BvRenderEngineVk.h"
//
//
//RenderTestBase::RenderTestBase()
//{
//	m_pEngine = GetRenderEngineVk();
//
//	m_pDevice = m_pEngine->CreateRenderDevice();
//}
//
//
//RenderTestBase::~RenderTestBase()
//{
//	m_pDevice->WaitIdle();
//}
//
//
//void RenderTestBase::Run()
//{
//	Init();
//	while (m_pSwapchain->GetWindow().IsActive())
//	{
//		Process();
//	}
//	Shutdown();
//}