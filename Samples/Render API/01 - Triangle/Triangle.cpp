#include "Triangle.h"
#include "Shaders.h"


void Triangle::OnInitialize()
{
	m_AppName = "Simple Triangle";
	CreateShaderResourceLayout();
	CreatePipeline();
}


void Triangle::OnUpdate()
{
}


void Triangle::OnRender()
{
	auto width = m_pWindow->GetWidth();
	auto height = m_pWindow->GetHeight();
	auto renderTarget = RenderTargetDesc::AsSwapChain(m_SwapChain->GetCurrentTextureView(), { 0.1f, 0.1f, 0.3f });

	m_Context->NewCommandList();
	m_Context->SetRenderTarget(renderTarget);
	m_Context->SetGraphicsPipeline(m_PSO);
	m_Context->SetViewport({ 0.0f, 0.0f, (f32)width, (f32)height, 0.0f, 1.0f });
	m_Context->SetScissor( 0, 0, width, height );
	m_Context->Draw(3);
	OnRenderUI();
	m_Context->Execute();

	m_SwapChain->Present(false);

	m_Context->FlushFrame();
}


void Triangle::OnShutdown()
{
	m_PSO.Reset();
	m_SRL.Reset();
}


void Triangle::CreateShaderResourceLayout()
{
	ShaderResourceLayoutCreateDesc layoutDesc{};
	m_Device->CreateShaderResourceLayout(layoutDesc, &m_SRL);
}


void Triangle::CreatePipeline()
{
	auto vs = CompileShader(g_pVSShader, g_VSSize, ShaderStage::kVertex);
	auto ps = CompileShader(g_pPSShader, g_PSSize, ShaderStage::kPixelOrFragment);
	GraphicsPipelineStateDesc pipelineDesc;
	pipelineDesc.AddShader(vs)
		.AddShader(ps)
		.AddRenderTargetFormat(m_SwapChain->GetDesc().m_Format)
		.SetShaderResourceLayoutPtr(m_SRL);

	m_Device->CreateGraphicsPipeline(pipelineDesc, &m_PSO);
}


SAMPLE_MAIN(Triangle)