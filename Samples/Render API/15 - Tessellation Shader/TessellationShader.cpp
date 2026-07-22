#include "TessellationShader.h"
#include "Shaders.h"


void TessellationShader::OnInitialize()
{
	m_AppName = "Tessellation Shader";
	CreateShaderResourceLayout();
	CreatePipeline();
}


void TessellationShader::OnUpdate()
{
}


void TessellationShader::OnUpdateUI()
{
	constexpr const char* pItems[] =
	{
		"Quads",
		"Triangles"
	};

	BeginDrawDefaultUI();
	ImGui::Combo("Tess Sample", &m_CurrOption, pItems, 2);
	ImGui::SliderFloat("Tess Outer 0", &m_TessData[0], 1.0f, 64.0f);
	ImGui::SliderFloat("Tess Outer 1", &m_TessData[1], 1.0f, 64.0f);
	ImGui::SliderFloat("Tess Outer 2", &m_TessData[2], 1.0f, 64.0f);
	if (m_CurrOption == 0)
	{
		ImGui::SliderFloat("Tess Outer 3", &m_TessData[3], 1.0f, 64.0f);
	}

	ImGui::SliderFloat("Tess Inner 0", &m_TessData[4], 1.0f, 64.0f);
	if (m_CurrOption == 0)
	{
		ImGui::SliderFloat("Tess Inner 1", &m_TessData[5], 1.0f, 64.0f);
	}
	EndDrawDefaultUI();
}


void TessellationShader::OnRender()
{
	auto width = m_pWindow->GetWidth();
	auto height = m_pWindow->GetHeight();
	RenderTargetDesc renderTarget;
	renderTarget.SetColorView(m_SwapChain->GetCurrentTextureView(), ResourceState::kCommon, ResourceState::kPresent).SetClearValues({ 0.1f, 0.1f, 0.3f });

	m_Context->NewCommandList();
	m_Context->SetRenderTarget(renderTarget);
	m_Context->SetGraphicsPipeline(m_CurrOption == 0 ? m_PSO : m_PSOTri);
	m_Context->SetViewport({ 0.0f, 0.0f, (f32)width, (f32)height, 0.0f, 1.0f });
	m_Context->SetScissor(width, height);
	m_Context->SetShaderConstants(sizeof(f32) * 6, m_TessData, 0, 0);
	m_Context->Draw(1);
	OnRenderUI();
	m_Context->Execute();

	m_SwapChain->Present(false);

	m_Context->FlushFrame();
}


void TessellationShader::OnShutdown()
{
	m_PSO.Reset();
	m_PSOTri.Reset();
	m_SRL.Reset();
}


void TessellationShader::CreateShaderResourceLayout()
{
	ShaderResourceLayoutCreateDesc layoutDesc;
	layoutDesc.AddResourceSet().AddConstant("PC"_sid, 0, sizeof(f32) * 6, ShaderStage::kHullOrControl);
	m_Device->CreateShaderResourceLayout(layoutDesc, &m_SRL);
}


void TessellationShader::CreatePipeline()
{
	auto vs = CompileShader(g_pVSShader, g_VSSize, ShaderStage::kVertex);
	auto hs = CompileShader(g_pHSShader, g_HSSize, ShaderStage::kHullOrControl);
	auto ds = CompileShader(g_pDSShader, g_DSSize, ShaderStage::kDomainOrEvaluation);
	auto ps = CompileShader(g_pPSShader, g_PSSize, ShaderStage::kPixelOrFragment);
	GraphicsPipelineStateDesc pipelineDesc;
	pipelineDesc.m_Shaders[0] = vs;
	pipelineDesc.m_Shaders[1] = ps;
	pipelineDesc.m_Shaders[2] = hs;
	pipelineDesc.m_Shaders[3] = ds;
	pipelineDesc.m_RenderTargetFormats[0] = m_SwapChain->GetDesc().m_Format;
	pipelineDesc.m_pShaderResourceLayout = m_SRL;
	pipelineDesc.m_PatchControlPoints = 1;
	pipelineDesc.m_InputAssemblyStateDesc.m_Topology = Topology::kPatchList;
	pipelineDesc.m_RasterizerStateDesc.m_FillMode = FillMode::kWireframe;

	m_Device->CreateGraphicsPipeline(pipelineDesc, &m_PSO);

	hs = CompileShader(g_pHSTriShader, g_HSTriSize, ShaderStage::kHullOrControl);
	ds = CompileShader(g_pDSTriShader, g_DSTriSize, ShaderStage::kDomainOrEvaluation);
	pipelineDesc.m_Shaders[2] = hs;
	pipelineDesc.m_Shaders[3] = ds;
	m_Device->CreateGraphicsPipeline(pipelineDesc, &m_PSOTri);
}


SAMPLE_MAIN(TessellationShader)