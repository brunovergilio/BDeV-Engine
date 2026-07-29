#include "Fullscreen.h"
#include "Shaders.h"


void Fullscreen::OnInitialize()
{
	m_AppName = "Full Screen Modes";
	CreateShaderResourceLayout();
	CreatePipeline();
}


void Fullscreen::OnUpdate()
{
	if (m_ChangesMade)
	{
		m_SwapChain->SetWindowMode(m_CurrentMode, m_pMonitor);
		m_ChangesMade = false;
	}
}


void Fullscreen::OnUpdateUI()
{
	auto oldMode = m_CurrentMode;
	auto oldMonitor = m_CurrentMonitor;

	auto& monitors = BvMonitor::GetMonitors();

	BeginDrawDefaultUI();
	ImGui::Checkbox("V-Sync", &m_VSync);

	ImGui::TextUnformatted("Window mode");
	ImGui::RadioButton("Windowed", reinterpret_cast<i32*>(&m_CurrentMode), 0);
	ImGui::RadioButton("Borderless Fullscreen", reinterpret_cast<i32*>(&m_CurrentMode), 1);
	ImGui::RadioButton("FullScreen", reinterpret_cast<i32*>(&m_CurrentMode), 2);

	ImGui::TextUnformatted("Monitors");
	for (auto i = 0; i < monitors.Size(); ++i)
	{
		ImGui::RadioButton(monitors[i]->GetName().CStr(), &m_CurrentMonitor, i);
	}
	EndDrawDefaultUI();
	m_pMonitor = monitors[m_CurrentMonitor];

	if (oldMode != m_CurrentMode || oldMonitor != m_CurrentMonitor)
	{
		m_ChangesMade = true;
	}
}


void Fullscreen::OnRender()
{
	auto width = m_pWindow->GetWidth();
	auto height = m_pWindow->GetHeight();
	RenderTargetDesc renderTarget;
	renderTarget.SetColorView(m_SwapChain->GetCurrentTextureView(), ResourceState::kCommon, ResourceState::kPresent).SetClearValues({ 0.1f, 0.1f, 0.3f });

	m_Context->NewCommandList();
	m_Context->SetRenderTarget(renderTarget);
	m_Context->SetGraphicsPipeline(m_PSO);
	m_Context->SetViewport({ 0.0f, 0.0f, (f32)width, (f32)height, 0.0f, 1.0f });
	m_Context->SetScissor(width, height);
	m_Context->Draw(3);
	OnRenderUI();
	m_Context->Execute();

	m_SwapChain->Present(m_VSync);

	m_Context->FlushFrame();
}


void Fullscreen::OnShutdown()
{
	m_PSO.Reset();
	m_SRL.Reset();
}


void Fullscreen::CreateShaderResourceLayout()
{
	ShaderResourceLayoutCreateDesc layoutDesc{};
	m_Device->CreateShaderResourceLayout(layoutDesc, &m_SRL);
}


void Fullscreen::CreatePipeline()
{
	auto vs = CompileShader(g_pVSShader, g_VSSize, ShaderStage::kVertex);
	auto ps = CompileShader(g_pPSShader, g_PSSize, ShaderStage::kPixelOrFragment);
	GraphicsPipelineStateDesc pipelineDesc;
	pipelineDesc.m_Shaders[0] = vs;
	pipelineDesc.m_Shaders[1] = ps;
	pipelineDesc.m_RenderTargetFormats[0] = m_SwapChain->GetDesc().m_Format;
	pipelineDesc.m_pShaderResourceLayout = m_SRL;

	m_Device->CreateGraphicsPipeline(pipelineDesc, &m_PSO);
}


SAMPLE_MAIN(Fullscreen)