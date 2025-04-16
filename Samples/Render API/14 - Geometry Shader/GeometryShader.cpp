#include "GeometryShader.h"
#include "BDeV/Core/Math/BvGeometryGenerator.h"
#include "BDeV/Core/Utils/BvRandom.h"


struct Vertex
{
	Float3 pos;
	Float4 color;
	Float3 normal;
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


constexpr const char* g_pVSDebugShader =
R"raw(
#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;

layout (location = 0) out vec3 outNormal;

void main() 
{
	outNormal = inNormal;
	gl_Position = vec4(inPos.xyz, 1.0);
}
)raw";
constexpr auto g_VSDebugSize = std::char_traits<char>::length(g_pVSDebugShader);


constexpr const char* g_pGSDebugShader =
R"raw(
#version 450

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

layout (binding = 0) uniform UBO 
{
	mat4 w;
	mat4 vp;
} ubo;

layout (location = 0) in vec3 inNormal[];

layout (location = 0) out vec4 outColor;

void main()
{	
	float normalLength = 0.2;
	for (int i = 0; i < gl_in.length(); ++i)
	{
		vec3 pos = gl_in[i].gl_Position.xyz;
		vec3 normal = inNormal[i].xyz;

		gl_Position = ubo.vp * (ubo.w * vec4(pos, 1.0));
		outColor = vec4(1.0, 0.0, 0.0, 1.0);
		EmitVertex();

		gl_Position = ubo.vp * (ubo.w * vec4(pos + normal * normalLength, 1.0));
		outColor = vec4(0.0, 0.0, 1.0, 1.0);
		EmitVertex();

		EndPrimitive();
	}
}
)raw";
constexpr auto g_GSDebugSize = std::char_traits<char>::length(g_pGSDebugShader);


void GeometryShader::OnInitialize()
{
	m_AppName = "Geometry Shader";
	CreateBuffers();
	CreateRenderTargets();
	CreateShaderResourceLayout();
	CreatePipeline();
}


void GeometryShader::OnInitializeUI()
{
	m_Overlay.SetupPipeline(m_SwapChain->GetDesc().m_Format, m_DepthView->GetDesc().m_Format);
}


void GeometryShader::OnUpdate()
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
	auto w = MatrixRotationX(angleX) * MatrixRotationY(angleY) * MatrixRotationZ(angleZ);
	auto vp = MatrixLookAtLH(VectorSet(0.0f, 0.0f, -5.0f, 1.0f), VectorSet(0.0f, 0.0f, 1.0f, 1.0f), VectorSet(0.0f, 1.0f, 0.0f, 1.0f))
		* MatrixPerspectiveLH_DX(0.1f, 100.0f, float(width) / float(height), kPiDiv4);
	Store44(w * vp, m_pWVP->m);
	Store44(w, m_pNormalData[0].m);
	Store44(vp, m_pNormalData[1].m);
}


void GeometryShader::OnRender()
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
	m_Context->SetVertexBufferView(m_VB, sizeof(Vertex));
	m_Context->SetIndexBufferView(m_IB, IndexFormat::kU32);
	m_Context->DrawIndexed(36);
	RenderNormals();
	OnRenderUI();
	m_Context->Execute();

	m_SwapChain->Present(false);

	m_Context->FlushFrame();
}


void GeometryShader::RenderNormals()
{
	m_Context->SetGraphicsPipeline(m_PSONormal);
	m_Context->SetConstantBuffer(m_UBViewNormal, 0, 0);
	m_Context->DrawIndexed(36);
}


void GeometryShader::OnShutdown()
{
	m_UB.Reset();
	m_VB.Reset();
	m_IB.Reset();
	m_UBView.Reset();
	m_Depth.Reset();
	m_DepthView.Reset();

	m_PSO.Reset();
	m_SRL.Reset();
}


void GeometryShader::CreateShaderResourceLayout()
{
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

	{
		ShaderResourceDesc resourceDesc = ShaderResourceDesc::AsConstantBuffer(0, ShaderStage::kGeometry);

		ShaderResourceSetDesc setDesc{};
		setDesc.m_ResourceCount = 1;
		setDesc.m_pResources = &resourceDesc;

		ShaderResourceLayoutDesc layoutDesc{};
		layoutDesc.m_ShaderResourceSetCount = 1;
		layoutDesc.m_pShaderResourceSets = &setDesc;

		m_SRLNormal = m_Device->CreateShaderResourceLayout(layoutDesc);
	}
}


void GeometryShader::CreatePipeline()
{
	auto vs = CompileShader(g_pVSShader, g_VSSize, ShaderStage::kVertex);
	auto ps = CompileShader(g_pPSShader, g_PSSize, ShaderStage::kPixelOrFragment);
	GraphicsPipelineStateDesc pipelineDesc;
	pipelineDesc.m_Shaders[0] = vs;
	pipelineDesc.m_Shaders[1] = ps;
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

	vs = CompileShader(g_pVSDebugShader, g_VSDebugSize, ShaderStage::kVertex);
	auto gs = CompileShader(g_pGSDebugShader, g_GSDebugSize, ShaderStage::kGeometry);
	pipelineDesc.m_Shaders[0] = vs;
	pipelineDesc.m_Shaders[2] = gs;
	pipelineDesc.m_pShaderResourceLayout = m_SRLNormal;

	inputDescs[1].m_Format = Format::kRGB32_Float;
	inputDescs[1].m_Offset = offsetof(Vertex, normal);

	m_PSONormal = m_Device->CreateGraphicsPipeline(pipelineDesc);
}


void GeometryShader::CreateBuffers()
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
		vertices[i].normal = data.m_Vertices[i].m_Normal;
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

	bufferDesc.m_Size = sizeof(u32) * data.m_Indices.Size();
	bufferDesc.m_UsageFlags = BufferUsage::kIndexBuffer;
	bufferData.m_pContext = m_Context;
	bufferData.m_pData = data.m_Indices.Data();
	bufferData.m_Size = bufferDesc.m_Size;
	m_IB = m_Device->CreateBuffer(bufferDesc, &bufferData);

	bufferDesc.m_Size = sizeof(Float44);
	bufferDesc.m_UsageFlags = BufferUsage::kConstantBuffer;
	bufferDesc.m_MemoryType = MemoryType::kUpload;
	bufferDesc.m_CreateFlags = BufferCreateFlags::kCreateMapped;
	m_UB = m_Device->CreateBuffer(bufferDesc);
	m_pWVP = m_UB->GetMappedDataAsT<Float44>();

	viewDesc.m_pBuffer = m_UB;
	viewDesc.m_Stride = sizeof(Float44);
	viewDesc.m_ElementCount = 1;
	m_UBView = m_Device->CreateBufferView(viewDesc);

	bufferDesc.m_Size = sizeof(Float44) * 2;
	m_UBNormal = m_Device->CreateBuffer(bufferDesc);
	m_pNormalData = m_UBNormal->GetMappedDataAsT<Float44>();
	viewDesc.m_pBuffer = m_UBNormal;
	viewDesc.m_Stride = sizeof(Float44) * 2;
	m_UBViewNormal = m_Device->CreateBufferView(viewDesc);
}


void GeometryShader::CreateRenderTargets()
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


SAMPLE_MAIN(GeometryShader)