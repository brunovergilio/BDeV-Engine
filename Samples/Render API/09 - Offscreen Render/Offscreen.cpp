#include "Offscreen.h"
#include "BDeV/Core/Math/BvGeometryGenerator.h"
#include "BDeV/Core/Utils/BvRandom.h"


struct Vertex
{
	Float3 pos;
	Float4 color;
};


constexpr const char* g_pVSShaderOffscreen =
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
constexpr auto g_VSSizeOffscreen = std::char_traits<char>::length(g_pVSShaderOffscreen);


constexpr const char* g_pPSShaderOffscreen =
R"raw(
#version 450

layout (location = 0) in vec4 inColor;

layout (location = 0) out vec4 outColor;

void main()
{
	outColor = inColor;
}
)raw";
constexpr auto g_PSSizeOffscreen = std::char_traits<char>::length(g_pPSShaderOffscreen);


constexpr const char* g_pVSShader =
R"raw(
#version 450

layout (location = 0) out vec2 outTexCoords;

layout (push_constant) uniform PushConstants
{
	vec4 posUV[6];
} pushConstants;

void main() 
{
	outTexCoords = vec2(pushConstants.posUV[gl_VertexIndex].zw);
	gl_Position = vec4(pushConstants.posUV[gl_VertexIndex].xy, 0.0, 1.0);
}
)raw";
constexpr auto g_VSSize = std::char_traits<char>::length(g_pVSShader);


constexpr const char* g_pPSShaderColor =
R"raw(
#version 450

layout (location = 0) in vec2 inTexCoords;

layout (location = 0) out vec4 outColor;

layout (binding = 1) uniform texture2D samplerTexture;
layout (binding = 2) uniform sampler samplerObj;

void main()
{
	outColor = texture(sampler2D(samplerTexture, samplerObj), inTexCoords);
}
)raw";
constexpr auto g_PSSizeColor = std::char_traits<char>::length(g_pPSShaderColor);


constexpr const char* g_pPSShaderDepth =
R"raw(
#version 450

layout (location = 0) in vec2 inTexCoords;

layout (location = 0) out vec4 outColor;

layout (binding = 1) uniform texture2D samplerTexture;
layout (binding = 2) uniform sampler samplerObj;

void main()
{
	float d = 1.0f - texture(sampler2D(samplerTexture, samplerObj), inTexCoords).r;
	outColor = vec4(d, d, d, 1.0f);
}
)raw";
constexpr auto g_PSSizeDepth = std::char_traits<char>::length(g_pPSShaderDepth);


void Offscreen::OnInitialize()
{
	m_AppName = "Offscreen Rendering";
	CreateBuffers();
	CreateRenderTargets();
	CreateShaderResourceLayout();
	CreatePipeline();

	m_PCColor.m_PosUV[0].x = 0.0f;
	m_PCColor.m_PosUV[0].y = 1.0f;
	m_PCColor.m_PosUV[0].z = 0.0f;
	m_PCColor.m_PosUV[0].w = 0.0f;

	m_PCColor.m_PosUV[1].x = 1.0f;
	m_PCColor.m_PosUV[1].y = 1.0f;
	m_PCColor.m_PosUV[1].z = 1.0f;
	m_PCColor.m_PosUV[1].w = 0.0f;

	m_PCColor.m_PosUV[2].x = 1.0f;
	m_PCColor.m_PosUV[2].y = -1.0f;
	m_PCColor.m_PosUV[2].z = 1.0f;
	m_PCColor.m_PosUV[2].w = 1.0f;

	m_PCColor.m_PosUV[3].x = 0.0f;
	m_PCColor.m_PosUV[3].y = 1.0f;
	m_PCColor.m_PosUV[3].z = 0.0f;
	m_PCColor.m_PosUV[3].w = 0.0f;

	m_PCColor.m_PosUV[4].x = 1.0f;
	m_PCColor.m_PosUV[4].y = -1.0f;
	m_PCColor.m_PosUV[4].z = 1.0f;
	m_PCColor.m_PosUV[4].w = 1.0f;

	m_PCColor.m_PosUV[5].x = 0.0f;
	m_PCColor.m_PosUV[5].y = -1.0f;
	m_PCColor.m_PosUV[5].z = 0.0f;
	m_PCColor.m_PosUV[5].w = 1.0f;

	m_PCDepth.m_PosUV[0].x = -1.0f;
	m_PCDepth.m_PosUV[0].y = 1.0f;
	m_PCDepth.m_PosUV[0].z = 0.0f;
	m_PCDepth.m_PosUV[0].w = 0.0f;

	m_PCDepth.m_PosUV[1].x = 0.0f;
	m_PCDepth.m_PosUV[1].y = 1.0f;
	m_PCDepth.m_PosUV[1].z = 1.0f;
	m_PCDepth.m_PosUV[1].w = 0.0f;

	m_PCDepth.m_PosUV[2].x = 0.0f;
	m_PCDepth.m_PosUV[2].y = -1.0f;
	m_PCDepth.m_PosUV[2].z = 1.0f;
	m_PCDepth.m_PosUV[2].w = 1.0f;

	m_PCDepth.m_PosUV[3].x = -1.0f;
	m_PCDepth.m_PosUV[3].y = 1.0f;
	m_PCDepth.m_PosUV[3].z = 0.0f;
	m_PCDepth.m_PosUV[3].w = 0.0f;

	m_PCDepth.m_PosUV[4].x = 0.0f;
	m_PCDepth.m_PosUV[4].y = -1.0f;
	m_PCDepth.m_PosUV[4].z = 1.0f;
	m_PCDepth.m_PosUV[4].w = 1.0f;

	m_PCDepth.m_PosUV[5].x = -1.0f;
	m_PCDepth.m_PosUV[5].y = -1.0f;
	m_PCDepth.m_PosUV[5].z = 0.0f;
	m_PCDepth.m_PosUV[5].w = 1.0f;
}


void Offscreen::OnUpdate()
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
		* MatrixLookAtLH(VectorSet(0.0f, 0.0f, -3.0f, 1.0f), VectorSet(0.0f, 0.0f, 1.0f, 1.0f), VectorSet(0.0f, 1.0f, 0.0f, 1.0f))
		* MatrixPerspectiveLH_DX(0.1f, 100.0f, float(width) / float(height), kPiDiv4), m_pWVP->m);
}


void Offscreen::RenderOffscreen()
{
	auto width = m_pWindow->GetWidth();
	auto height = m_pWindow->GetHeight();
	RenderTargetDesc targets[] =
	{
		RenderTargetDesc::AsRenderTarget(m_ColorView, { 1.0f, 1.0f, 1.0f }),
		RenderTargetDesc::AsDepthStencil(m_DepthView)
	};

	m_Context->SetRenderTargets(2, targets);
	m_Context->SetGraphicsPipeline(m_PSOOffscreen);
	m_Context->SetViewport({ 0.0f, 0.0f, (f32)width, (f32)height, 0.0f, 1.0f });
	m_Context->SetScissor({ 0, 0, width, height });
	m_Context->SetConstantBuffer(m_UBView, 0, 0);
	m_Context->SetVertexBufferView(m_VB, sizeof(Vertex));
	m_Context->SetIndexBufferView(m_IB, IndexFormat::kU32);
	m_Context->DrawIndexed(36);
}


void Offscreen::OnRender()
{
	f32 width = m_pWindow->GetWidth();
	f32 height = m_pWindow->GetHeight();

	RenderTargetDesc mainTarget = RenderTargetDesc::AsSwapChain(m_SwapChain->GetCurrentTextureView(), { 0.1f, 0.1f, 0.3f });

	m_Context->NewCommandList();
	RenderOffscreen();
	m_Context->SetRenderTarget(mainTarget);

	m_Context->SetGraphicsPipeline(m_PSOColor);
	m_Context->SetShaderConstantsT<PCData>(m_PCColor, 3, 0);
	m_Context->SetTexture(m_ColorView, 0, 1);
	m_Context->SetSampler(m_Sampler, 0, 2);
	m_Context->Draw(6);

	m_Context->SetGraphicsPipeline(m_PSODepth);
	m_Context->SetShaderConstantsT<PCData>(m_PCDepth, 3, 0);
	m_Context->SetTexture(m_DepthView, 0, 1);
	m_Context->SetSampler(m_Sampler, 0, 2);
	m_Context->Draw(6);

	OnRenderUI();
	m_Context->Execute();

	m_SwapChain->Present(false);

	m_Context->FlushFrame();
}


void Offscreen::OnShutdown()
{
	m_UB.Reset();
	m_VB.Reset();
	m_IB.Reset();
	m_UBView.Reset();
	m_Depth.Reset();
	m_DepthView.Reset();

	m_PSOColor.Reset();
	m_PSODepth.Reset();
	m_SRL.Reset();
}


void Offscreen::CreateShaderResourceLayout()
{
	{
		ShaderResourceDesc resourceDesc = ShaderResourceDesc::AsConstantBuffer(0, ShaderStage::kVertex);

		ShaderResourceSetDesc setDesc{};
		setDesc.m_ResourceCount = 1;
		setDesc.m_pResources = &resourceDesc;

		ShaderResourceLayoutDesc layoutDesc{};
		layoutDesc.m_ShaderResourceSetCount = 1;
		layoutDesc.m_pShaderResourceSets = &setDesc;

		m_SRLOffscreen = m_Device->CreateShaderResourceLayout(layoutDesc);
	}

	{
		ShaderResourceDesc resourceDescs[] =
		{
			ShaderResourceDesc::AsTexture(1, ShaderStage::kPixelOrFragment),
			ShaderResourceDesc::AsSampler(2, ShaderStage::kPixelOrFragment)
		};

		ShaderResourceConstantDesc constantDesc = ShaderResourceConstantDesc::As<PCData>(BV_NAME_ID("PC"), 3, ShaderStage::kVertex);

		ShaderResourceSetDesc setDesc{};
		setDesc.m_ResourceCount = 2;
		setDesc.m_pResources = resourceDescs;
		setDesc.m_ConstantCount = 1;
		setDesc.m_pConstants = &constantDesc;

		ShaderResourceLayoutDesc layoutDesc{};
		layoutDesc.m_ShaderResourceSetCount = 1;
		layoutDesc.m_pShaderResourceSets = &setDesc;

		m_SRL = m_Device->CreateShaderResourceLayout(layoutDesc);
	}
}


void Offscreen::CreatePipeline()
{
	{
		auto vs = CompileShader(g_pVSShaderOffscreen, g_VSSizeOffscreen, ShaderStage::kVertex);
		auto ps = CompileShader(g_pPSShaderOffscreen, g_PSSizeOffscreen, ShaderStage::kPixelOrFragment);
		GraphicsPipelineStateDesc pipelineDesc;
		pipelineDesc.m_Shaders[0] = vs;
		pipelineDesc.m_Shaders[1] = ps;
		pipelineDesc.m_RenderTargetFormats[0] = m_Color->GetDesc().m_Format;
		pipelineDesc.m_DepthStencilFormat = m_DepthView->GetDesc().m_Format;
		pipelineDesc.m_DepthStencilDesc.m_DepthTestEnable = true;
		pipelineDesc.m_DepthStencilDesc.m_DepthWriteEnable = true;
		pipelineDesc.m_DepthStencilDesc.m_DepthOp = CompareOp::kLessEqual;
		pipelineDesc.m_pShaderResourceLayout = m_SRLOffscreen;

		VertexInputDesc inputDescs[2];
		inputDescs[0].m_Format = Format::kRGB32_Float;
		inputDescs[1].m_Format = Format::kRGBA32_Float;

		pipelineDesc.m_VertexInputDescCount = 2;
		pipelineDesc.m_pVertexInputDescs = inputDescs;

		m_PSOOffscreen = m_Device->CreateGraphicsPipeline(pipelineDesc);
	}

	{
		auto vs = CompileShader(g_pVSShader, g_VSSize, ShaderStage::kVertex);
		auto psC = CompileShader(g_pPSShaderColor, g_PSSizeColor, ShaderStage::kPixelOrFragment);
		GraphicsPipelineStateDesc pipelineDesc;
		pipelineDesc.m_Shaders[0] = vs;
		pipelineDesc.m_Shaders[1] = psC;
		pipelineDesc.m_RenderTargetFormats[0] = m_SwapChain->GetDesc().m_Format;
		pipelineDesc.m_pShaderResourceLayout = m_SRL;

		m_PSOColor = m_Device->CreateGraphicsPipeline(pipelineDesc);
		
		auto psD = CompileShader(g_pPSShaderDepth, g_PSSizeDepth, ShaderStage::kPixelOrFragment);
		pipelineDesc.m_Shaders[1] = psD;
		m_PSODepth = m_Device->CreateGraphicsPipeline(pipelineDesc);

	}
}


void Offscreen::CreateBuffers()
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
	m_UB = m_Device->CreateBuffer(bufferDesc, &bufferData);

	viewDesc.m_pBuffer = m_UB;
	viewDesc.m_Stride = sizeof(Float44);
	viewDesc.m_ElementCount = 1;
	m_UBView = m_Device->CreateBufferView(viewDesc);

	m_pWVP = m_UB->GetMappedDataAsT<Float44>();
}


void Offscreen::CreateRenderTargets()
{
	auto w = m_pWindow->GetWidth();
	auto h = m_pWindow->GetHeight();

	TextureDesc desc;
	TextureViewDesc viewDesc;

	desc.m_Size = { w, h, 1 };
	desc.m_Format = Format::kRGBA8_UNorm_SRGB;
	desc.m_UsageFlags = TextureUsage::kRenderTarget | TextureUsage::kShaderResource;
	m_Color = m_Device->CreateTexture(desc, nullptr);

	viewDesc.m_Format = desc.m_Format;
	viewDesc.m_pTexture = m_Color;
	m_ColorView = m_Device->CreateTextureView(viewDesc);

	desc.m_Size = { w, h, 1 };
	desc.m_Format = Format::kD24_UNorm_S8_UInt;
	desc.m_UsageFlags = TextureUsage::kDepthStencilTarget | TextureUsage::kShaderResource;
	m_Depth = m_Device->CreateTexture(desc, nullptr);

	viewDesc.m_Format = desc.m_Format;
	viewDesc.m_pTexture = m_Depth;
	m_DepthView = m_Device->CreateTextureView(viewDesc);

	m_Sampler = m_Device->CreateSampler(SamplerDesc());
}


SAMPLE_MAIN(Offscreen)