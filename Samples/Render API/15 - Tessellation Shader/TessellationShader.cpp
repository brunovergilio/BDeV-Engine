#include "TessellationShader.h"


constexpr const char* g_pVSShader =
R"raw(
#version 450

void main()
{
}
)raw";
constexpr auto g_VSSize = std::char_traits<char>::length(g_pVSShader);

constexpr const char* g_pHSShader =
R"raw(
#version 450

layout(vertices = 1) out;

layout(push_constant) uniform PC
{
	float tessData[6];
} pc;

void main()
{
    if (gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = pc.tessData[0]; // horizontal subdivisions
        gl_TessLevelOuter[1] = pc.tessData[1]; // vertical subdivisions
        gl_TessLevelOuter[2] = pc.tessData[2];
        gl_TessLevelOuter[3] = pc.tessData[3];
        gl_TessLevelInner[0] = pc.tessData[4];
        gl_TessLevelInner[1] = pc.tessData[5];
    }
}
)raw";
constexpr auto g_HSSize = std::char_traits<char>::length(g_pHSShader);

constexpr const char* g_pDSShader =
R"raw(
#version 450

layout(quads, equal_spacing, cw) in;

layout (location = 0) out vec4 outColor;

void main()
{
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    // Fullscreen quad [-1, 1] in clip space
    gl_Position = vec4(u * 2.0 - 1.0, v * 2.0 - 1.0, 0.0, 1.0);
	outColor = vec4(0.0, 1.0, 0.0, 1.0);
}
)raw";
constexpr auto g_DSSize = std::char_traits<char>::length(g_pDSShader);

constexpr const char* g_pDSTriShader =
R"raw(
#version 450

layout(triangles, equal_spacing, cw) in;

layout (location = 0) out vec4 outColor;

void main()
{
    // Barycentric coordinates: gl_TessCoord = (u, v, w)
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    float w = gl_TessCoord.z;

    // Barycentric - polar mapping trick
    float angle = v * 2.0 * 3.14159265;
    float radius = u;

    // Convert polar to Cartesian
    float x = cos(angle) * radius;
    float y = sin(angle) * radius;

    gl_Position = vec4(x, y, 0.0, 1.0);
	outColor = vec4(0.0, 1.0, 0.0, 1.0);
}
)raw";
constexpr auto g_DSTriSize = std::char_traits<char>::length(g_pDSTriShader);

constexpr const char* g_pPSShader =
R"raw(
#version 450

layout (location = 0) in vec4 inColor;

layout (location = 0) out vec4 outColor;

void main()
{
	outColor = inColor;
}
)raw";
constexpr auto g_PSSize = std::char_traits<char>::length(g_pPSShader);


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
	auto renderTarget = RenderTargetDesc::AsSwapChain(m_SwapChain->GetCurrentTextureView(), { 0.1f, 0.1f, 0.3f });

	m_Context->NewCommandList();
	m_Context->SetRenderTarget(renderTarget);
	m_Context->SetGraphicsPipeline(m_CurrOption == 0 ? m_PSO : m_PSOTri);
	m_Context->SetViewport({ 0.0f, 0.0f, (f32)width, (f32)height, 0.0f, 1.0f });
	m_Context->SetScissor({ 0, 0, width, height });
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
	ShaderResourceConstantDesc constantDesc{ BV_NAME_ID("PC"), 0, sizeof(f32) * 6, ShaderStage::kHullOrControl };
	ShaderResourceSetDesc setDesc{};
	setDesc.m_ConstantCount = 1;
	setDesc.m_pConstants = &constantDesc;

	ShaderResourceLayoutDesc layoutDesc{};
	layoutDesc.m_ShaderResourceSetCount = 1;
	layoutDesc.m_pShaderResourceSets = &setDesc;
	m_SRL = m_Device->CreateShaderResourceLayout(layoutDesc);
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

	m_PSO = m_Device->CreateGraphicsPipeline(pipelineDesc);

	ds = CompileShader(g_pDSTriShader, g_DSTriSize, ShaderStage::kDomainOrEvaluation);
	pipelineDesc.m_Shaders[3] = ds;
	m_PSOTri = m_Device->CreateGraphicsPipeline(pipelineDesc);
}


SAMPLE_MAIN(TessellationShader)