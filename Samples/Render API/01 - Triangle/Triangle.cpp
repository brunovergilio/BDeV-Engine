#include "Triangle.h"


constexpr const char* g_pVSShader =
R"raw(
#version 450

vec2 vertices[] = 
{
	vec2(-0.7f, -0.7f),
	vec2( 0.0f,  0.7f),
	vec2( 0.7f, -0.7f)
};

void main()
{
	gl_Position = vec4(vertices[gl_VertexIndex], 0.0f, 1.0f);
}
)raw";
constexpr auto g_VSSize = std::char_traits<char>::length(g_pVSShader);


constexpr const char* g_pPSShader =
R"raw(
#version 450

layout (location = 0) out vec4 outColor;

void main()
{
	outColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
)raw";
constexpr auto g_PSSize = std::char_traits<char>::length(g_pPSShader);


void Triangle::OnInitialize()
{
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
	m_Context->SetScissor({ 0, 0, width, height });
	m_Context->Draw(3);
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
	ShaderResourceLayoutDesc layoutDesc{};
	m_SRL = m_Device->CreateShaderResourceLayout(layoutDesc);
}


void Triangle::CreatePipeline()
{
	CompileShaders();

	GraphicsPipelineStateDesc pipelineDesc;
	pipelineDesc.m_Shaders[0] = m_VS;
	pipelineDesc.m_Shaders[1] = m_PS;
	pipelineDesc.m_RenderTargetFormats[0] = m_SwapChain->GetDesc().m_Format;
	pipelineDesc.m_pShaderResourceLayout = m_SRL;

	m_PSO = m_Device->CreateGraphicsPipeline(pipelineDesc);

	m_VS.Reset();
	m_PS.Reset();
}


void Triangle::CompileShaders()
{
	BvSharedLib renderToolsLib("BvRenderTools.dll");
	using ShaderCompilerFnType = IBvShaderCompiler*(*)();
	ShaderCompilerFnType compilerFn = renderToolsLib.GetProcAddressT<ShaderCompilerFnType>("CreateSPIRVCompiler");
	BvRCRef<IBvShaderCompiler> compiler = compilerFn();

	ShaderCreateDesc shaderDesc;
	shaderDesc.m_ShaderLanguage = ShaderLanguage::kGLSL;
	shaderDesc.m_ShaderStage = ShaderStage::kVertex;
	shaderDesc.m_pSourceCode = g_pVSShader;
	shaderDesc.m_SourceCodeSize = g_VSSize;
	BvRCRef<IBvShaderBlob> compiledShader;
	auto result = compiler->Compile(shaderDesc, &compiledShader);
	BV_ASSERT(result, "Invalid Shader");
	shaderDesc.m_pByteCode = (const u8*)compiledShader->GetBufferPointer();
	shaderDesc.m_ByteCodeSize = compiledShader->GetBufferSize();
	m_VS = m_Device->CreateShader(shaderDesc);
	compiledShader.Reset();

	shaderDesc.m_ShaderStage = ShaderStage::kPixelOrFragment;
	shaderDesc.m_pSourceCode = g_pPSShader;
	shaderDesc.m_SourceCodeSize = g_PSSize;
	result = compiler->Compile(shaderDesc, &compiledShader);
	BV_ASSERT(result, "Invalid Shader");
	shaderDesc.m_pByteCode = (const u8*)compiledShader->GetBufferPointer();
	shaderDesc.m_ByteCodeSize = compiledShader->GetBufferSize();
	m_PS = m_Device->CreateShader(shaderDesc);
	compiledShader.Reset();
}


SAMPLE_MAIN(Triangle)