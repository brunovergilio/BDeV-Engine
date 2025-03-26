#include "Buffers.h"
#include "BDeV/Core/Math/BvGeometryGenerator.h"
#include "BDeV/Core/Utils/BvRandom.h"


struct Vertex
{
	Float3 pos;
	Float4 color;
};


constexpr const char* g_pVSShader =
R"raw(
#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

layout (binding = 0) uniform UBO 
{
	mat4 wvp;
} ubo;

void main() 
{
	outColor = inColor;
	gl_Position = ubo.wvp * vec4(inPos.xyz, 1.0);
}
)raw";
constexpr auto g_VSSize = std::char_traits<char>::length(g_pVSShader);


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


void Buffers::OnInitialize()
{
	CreateBuffers();
	CreateRenderTargets();
	CreateShaderResourceLayout();
	CreatePipeline();
}


void Buffers::OnInitializeUI()
{
	m_Overlay.SetupPipeline(m_SwapChain->GetDesc().m_Format, m_DepthView->GetDesc().m_Format);
}


void Buffers::OnUpdate()
{
	auto width = m_pWindow->GetWidth();
	auto height = m_pWindow->GetHeight();
	static f32 speed = 2.0f;
	static f32 angleX = 0.0f;
	static f32 angleY = 0.0f;
	static f32 angleZ = 0.0f;
	angleX += m_Dt * speed;
	if (angleX > k2Pi)
	{
		angleX = 0.0f;
	}
	angleY += m_Dt * speed;
	if (angleY > k2Pi)
	{
		angleY = 0.0f;
	}
	angleZ += m_Dt * speed;
	if (angleZ > k2Pi)
	{
		angleZ = 0.0f;
	}
	Store44(MatrixRotationX(angleX) * MatrixRotationY(angleY) * MatrixRotationZ(angleZ)
		* MatrixLookAtLH(VectorSet(0.0f, 0.0f, -5.0f, 1.0f), VectorSet(0.0f, 0.0f, 1.0f, 1.0f), VectorSet(0.0f, 1.0f, 0.0f, 1.0f))
		* MatrixPerspectiveLH_DX(0.1f, 100.0f, float(width) / float(height), kPiDiv4), m_pWVP->m);
}


void Buffers::OnUpdateUI()
{
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);
	ImGui::Begin("Render Sample", nullptr);
	ImGui::TextUnformatted("Buffers");
	ImGui::Text("FPS: %d", m_FPSCounter.GetFPS());
	ImGui::End();

	ImGui::EndFrame();

	ImGui::Render();
}


void Buffers::OnRender()
{
	auto width = m_pWindow->GetWidth();
	auto height = m_pWindow->GetHeight();
	RenderTargetDesc targets[] =
	{
		RenderTargetDesc::AsSwapChain(m_SwapChain->GetCurrentTextureView(), { 0.1f, 0.1f, 0.3f }),
		RenderTargetDesc::AsDepthStencil(m_DepthView)
	};
	targets[1].m_StateAfter = ResourceState::kDepthStencilRead;

	m_Context->NewCommandList();
	m_Context->SetRenderTargets(2, targets);
	m_Context->SetGraphicsPipeline(m_PSO);
	m_Context->SetViewport({ 0.0f, 0.0f, (f32)width, (f32)height, 0.0f, 1.0f });
	m_Context->SetScissor({ 0, 0, width, height });
	m_Context->SetConstantBuffer(m_UBView, 0, 0);
	m_Context->SetVertexBufferView(m_VBView);
	m_Context->SetIndexBufferView(m_IBView, IndexFormat::kU32);
	m_Context->DrawIndexed(m_IBView->GetDesc().m_ElementCount);
	OnRenderUI();
	m_Context->Execute();

	m_SwapChain->Present(false);

	m_Context->FlushFrame();
}


void Buffers::OnShutdown()
{
	m_UB.Reset();
	m_VB.Reset();
	m_IB.Reset();
	m_VBView.Reset();
	m_IBView.Reset();
	m_UBView.Reset();
	m_Depth.Reset();
	m_DepthView.Reset();

	m_PSO.Reset();
	m_SRL.Reset();
}


void Buffers::CreateShaderResourceLayout()
{
	ShaderResourceDesc resourceDesc = ShaderResourceDesc::AsConstantBuffer(0, ShaderStage::kVertex);

	ShaderResourceSetDesc setDesc{};
	setDesc.m_ResourceCount = 1;
	setDesc.m_pResources = &resourceDesc;

	ShaderResourceLayoutDesc layoutDesc{};
	layoutDesc.m_ShaderResourceSetCount = 1;
	layoutDesc.m_pShaderResourceSets = &setDesc;

	m_SRL = m_Device->CreateShaderResourceLayout(layoutDesc);
}


void Buffers::CreatePipeline()
{
	CompileShaders();

	GraphicsPipelineStateDesc pipelineDesc;
	pipelineDesc.m_Shaders[0] = m_VS;
	pipelineDesc.m_Shaders[1] = m_PS;
	pipelineDesc.m_RenderTargetFormats[0] = m_SwapChain->GetDesc().m_Format;
	pipelineDesc.m_DepthStencilFormat = m_DepthView->GetDesc().m_Format;
	pipelineDesc.m_DepthStencilDesc.m_DepthTestEnable = true;
	pipelineDesc.m_DepthStencilDesc.m_DepthWriteEnable = true;
	pipelineDesc.m_DepthStencilDesc.m_DepthOp = CompareOp::kLessEqual;
	pipelineDesc.m_pShaderResourceLayout = m_SRL;

	VertexInputDesc inputDescs[2];
	inputDescs[0].m_Format = Format::kRGB32_Float;
	inputDescs[1].m_Format = Format::kRGBA32_Float;

	pipelineDesc.m_VertexInputDescCount = 2;
	pipelineDesc.m_pVertexInputDescs = inputDescs;

	m_PSO = m_Device->CreateGraphicsPipeline(pipelineDesc);

	m_VS.Reset();
	m_PS.Reset();
}


void Buffers::CompileShaders()
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


void Buffers::CreateBuffers()
{
	BvGeometryGenerator gen;
	gen.GenerateBox();
	auto& data = gen.GetData();
	BvVector<Vertex> vertices(data.m_Vertices.Size());
	BvPCG rand;

	for (auto i = 0u; i < vertices.Size(); ++i)
	{
		vertices[i].pos = data.m_Vertices[i].m_Position;
		vertices[i].color = Float4(rand.NextF<f32>(), rand.NextF<f32>(), rand.NextF<f32>(), 1.0f);
	}

	BufferDesc bufferDesc;
	BufferInitData bufferData;
	BufferViewDesc viewDesc;

	bufferDesc.m_Size = sizeof(Vertex) * vertices.Size();
	bufferDesc.m_UsageFlags = BufferUsage::kVertexBuffer;
	bufferData.m_pContext = m_Context;
	bufferData.m_pData = vertices.Data();
	bufferData.m_Size = bufferDesc.m_Size;
	m_VB = m_Device->CreateBuffer(bufferDesc, &bufferData);

	viewDesc.m_pBuffer = m_VB;
	viewDesc.m_Stride = sizeof(Vertex);
	viewDesc.m_ElementCount = vertices.Size();
	m_VBView = m_Device->CreateBufferView(viewDesc);

	bufferDesc.m_Size = sizeof(u32) * data.m_Indices.Size();
	bufferDesc.m_UsageFlags = BufferUsage::kIndexBuffer;
	bufferData.m_pContext = m_Context;
	bufferData.m_pData = data.m_Indices.Data();
	bufferData.m_Size = bufferDesc.m_Size;
	m_IB = m_Device->CreateBuffer(bufferDesc, &bufferData);

	viewDesc.m_pBuffer = m_IB;
	viewDesc.m_Stride = sizeof(u32);
	viewDesc.m_ElementCount = data.m_Indices.Size();
	m_IBView = m_Device->CreateBufferView(viewDesc);

	bufferDesc.m_Size = sizeof(Float44);
	bufferDesc.m_UsageFlags = BufferUsage::kConstantBuffer;
	bufferDesc.m_MemoryType = MemoryType::kUpload;
	bufferDesc.m_CreateFlags = BufferCreateFlags::kCreateMapped;
	m_UB = m_Device->CreateBuffer(bufferDesc, &bufferData);

	viewDesc.m_pBuffer = m_UB;
	viewDesc.m_Stride = sizeof(Float44);
	viewDesc.m_ElementCount = 1;
	m_UBView = m_Device->CreateBufferView(viewDesc);

	m_pWVP = m_UB->GetMappedDataAsT<Float44>();
}


void Buffers::CreateRenderTargets()
{
	auto w = m_pWindow->GetWidth();
	auto h = m_pWindow->GetHeight();

	TextureDesc desc;
	desc.m_Size = { w, h, 1 };
	desc.m_Format = Format::kD24_UNorm_S8_UInt;
	desc.m_UsageFlags = TextureUsage::kDepthStencilTarget;
	m_Depth = m_Device->CreateTexture(desc, nullptr);

	TextureViewDesc viewDesc;
	viewDesc.m_Format = desc.m_Format;
	viewDesc.m_pTexture = m_Depth;
	m_DepthView = m_Device->CreateTextureView(viewDesc);
}


SAMPLE_MAIN(Buffers)