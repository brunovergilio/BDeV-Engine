#include "Cubemap.h"
#include "BDeV/Core/Math/BvGeometryGenerator.h"
#include "BDeV/Core/Utils/BvRandom.h"


struct Vertex
{
	Float3 pos;
	Float2 uv;
};


constexpr const char* g_pVSShader =
R"raw(
#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inTexCoords;

layout (location = 0) out vec2 outTexCoords;

layout (binding = 0) uniform UBO 
{
	mat4 wvp;
} ubo;

void main() 
{
	outTexCoords = inTexCoords;
	gl_Position = ubo.wvp * vec4(inPos.xyz, 1.0);
}
)raw";
constexpr auto g_VSSize = std::char_traits<char>::length(g_pVSShader);


constexpr const char* g_pPSShader =
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
constexpr auto g_PSSize = std::char_traits<char>::length(g_pPSShader);


constexpr const char* g_pVSSkyBoxShader =
R"raw(
#version 450

layout (location = 0) in vec3 inPos;

layout (location = 0) out vec3 outTexCoords;

layout (binding = 0) uniform UBO
{
	mat4 wvp;
} ubo;

void main() 
{
	gl_Position = (ubo.wvp * vec4(inPos.xyz, 1.0)).xyww;
	outTexCoords = inPos;
}
)raw";
constexpr auto g_VSSkyBoxSize = std::char_traits<char>::length(g_pVSSkyBoxShader);


constexpr const char* g_pPSSkyBoxShader =
R"raw(
#version 450

layout (location = 0) in vec3 inTexCoords;

layout (location = 0) out vec4 outColor;

layout (binding = 1) uniform textureCube samplerTexture;
layout (binding = 2) uniform sampler samplerObj;

void main()
{
	outColor = texture(samplerCube(samplerTexture, samplerObj), inTexCoords);
}
)raw";
constexpr auto g_PSSkyBoxSize = std::char_traits<char>::length(g_pPSSkyBoxShader);


void Cubemap::OnInitialize()
{
	m_AppName = "Cubemap Texture";
	CreateBuffers();
	CreateTextures();
	CreateRenderTargets();
	CreateShaderResourceLayout();
	CreatePipeline();
	CreateSkyboxResources();
}


void Cubemap::OnInitializeUI()
{
	m_Overlay.SetupPipeline(m_SwapChain->GetDesc().m_Format, m_DepthView->GetDesc().m_Format);
}


void Cubemap::OnUpdate()
{
	auto width = m_pWindow->GetWidth();
	auto height = m_pWindow->GetHeight();
	static f32 speed = 2.0f;
	static f32 angleX = 0.0f;
	static f32 angleY = 0.0f;
	static f32 angleZ = 0.0f;
	static f32 camAngleY = 0.0f;
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
	camAngleY += m_Dt * speed * 0.3f;
	if (camAngleY > k2Pi)
	{
		camAngleY = 0.0f;
	}
	BvVec3 camPos(0.0f, 0.0f, -5.0f);
	BvVec3 lookPos(0.0f, 0.0f, 1.0f);

	BvVec3 eyeDir((lookPos - camPos) * MatrixRotationY(camAngleY));
	BvMatrix v(MatrixLookAt(camPos, eyeDir, VectorSet(0.0f, 1.0f, 0.0f, 1.0f)));
	BvMatrix p(MatrixPerspectiveLH_DX(0.1f, 100.0f, float(width) / float(height), kPiDiv4));
	BvMatrix vp(v * p);
	BvMatrix rot(MatrixRotationX(angleX) * MatrixRotationY(angleY) * MatrixRotationZ(angleZ));

	Store44(rot	* vp, m_pWVP->m);
	Store44(MatrixTranslation(camPos) * vp, m_pWVPSkybox->m);
}


void Cubemap::OnRender()
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
	m_Context->SetTexture(m_TextureView, 0, 1);
	m_Context->SetSampler(m_Sampler, 0, 2);
	m_Context->SetVertexBufferView(m_VB, sizeof(Vertex));
	m_Context->SetIndexBufferView(m_IB, IndexFormat::kU32);
	m_Context->DrawIndexed(36);
	RenderSkybox();
	OnRenderUI();
	m_Context->Execute();

	m_SwapChain->Present(false);

	m_Context->FlushFrame();
}


void Cubemap::OnShutdown()
{
	m_SamplerSkybox.Reset();
	m_TextureViewSkybox.Reset();
	m_TextureSkybox.Reset();
	m_UBSkybox.Reset();
	m_UBViewSkybox.Reset();
	m_PSOSkybox.Reset();

	m_Sampler.Reset();
	m_TextureView.Reset();
	m_Texture.Reset();
	m_UB.Reset();
	m_VB.Reset();
	m_IB.Reset();
	m_UBView.Reset();
	m_Depth.Reset();
	m_DepthView.Reset();
	m_PSO.Reset();
	m_SRL.Reset();
}


void Cubemap::RenderSkybox()
{
	m_Context->SetGraphicsPipeline(m_PSOSkybox);
	m_Context->SetConstantBuffer(m_UBViewSkybox, 0, 0);
	m_Context->SetTexture(m_TextureViewSkybox, 0, 1);
	m_Context->SetSampler(m_SamplerSkybox, 0, 2);
	m_Context->DrawIndexed(36);
}


void Cubemap::CreateShaderResourceLayout()
{
	ShaderResourceDesc resourceDescs[] =
	{
		ShaderResourceDesc::AsConstantBuffer(0, ShaderStage::kVertex),
		ShaderResourceDesc::AsTexture(1, ShaderStage::kPixelOrFragment),
		ShaderResourceDesc::AsSampler(2, ShaderStage::kPixelOrFragment)
	};

	ShaderResourceSetDesc setDesc{};
	setDesc.m_ResourceCount = 3;
	setDesc.m_pResources = resourceDescs;

	ShaderResourceLayoutDesc layoutDesc{};
	layoutDesc.m_ShaderResourceSetCount = 1;
	layoutDesc.m_pShaderResourceSets = &setDesc;

	m_SRL = m_Device->CreateShaderResourceLayout(layoutDesc);
}


void Cubemap::CreatePipeline()
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
	inputDescs[1].m_Format = Format::kRG32_Float;

	pipelineDesc.m_VertexInputDescCount = 2;
	pipelineDesc.m_pVertexInputDescs = inputDescs;

	m_PSO = m_Device->CreateGraphicsPipeline(pipelineDesc);
}


void Cubemap::CreateBuffers()
{
	BvGeometryGenerator gen;
	gen.GenerateBox();
	auto& data = gen.GetData();
	BvVector<Vertex> vertices(data.m_Vertices.Size());
	BvRandom32 rand;

	for (auto i = 0u; i < vertices.Size(); ++i)
	{
		vertices[i].pos = data.m_Vertices[i].m_Position;
		vertices[i].uv = data.m_Vertices[i].m_UV;
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
	m_UB = m_Device->CreateBuffer(bufferDesc, nullptr);

	viewDesc.m_pBuffer = m_UB;
	viewDesc.m_Stride = sizeof(Float44);
	viewDesc.m_ElementCount = 1;
	m_UBView = m_Device->CreateBufferView(viewDesc);

	m_pWVP = m_UB->GetMappedDataAsT<Float44>();
}


void Cubemap::CreateTextures()
{
	BvRCRef<IBvTextureBlob> pTextureBlob = m_TextureLoader->LoadTextureFromFile("water.dds");
	auto& info = pTextureBlob->GetInfo();
	TextureDesc texDesc;
	texDesc.m_Size = { info.m_Width, info.m_Height, info.m_Depth };
	texDesc.m_ArraySize = info.m_ArraySize;
	texDesc.m_Format = info.m_Format;
	texDesc.m_MipLevels = info.m_MipLevels;
	texDesc.m_ImageType = info.m_TextureType;
	texDesc.m_ResourceState = ResourceState::kPixelShaderResource;
	texDesc.m_UsageFlags = TextureUsage::kShaderResource;
	auto& subresources = pTextureBlob->GetSubresources();
	TextureInitData initData{ m_Context, u32(subresources.Size()), subresources.Data() };

	m_Texture = m_Device->CreateTexture(texDesc, &initData);

	TextureViewDesc viewDesc;
	viewDesc.m_pTexture = m_Texture;
	viewDesc.m_Format = texDesc.m_Format;

	m_TextureView = m_Device->CreateTextureView(viewDesc);

	m_Sampler = m_Device->CreateSampler(SamplerDesc());
}


void Cubemap::CreateRenderTargets()
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


void Cubemap::CreateSkyboxResources()
{
	BvRCRef<IBvTextureBlob> pTextureBlob = m_TextureLoader->LoadTextureFromFile("purplevalley_cube.dds");
	auto& info = pTextureBlob->GetInfo();
	TextureDesc texDesc;
	texDesc.m_Size = { info.m_Width, info.m_Height, info.m_Depth };
	texDesc.m_ArraySize = info.m_ArraySize;
	texDesc.m_Format = info.m_Format;
	texDesc.m_MipLevels = info.m_MipLevels;
	texDesc.m_ImageType = info.m_TextureType;
	texDesc.m_ResourceState = ResourceState::kPixelShaderResource;
	texDesc.m_UsageFlags = TextureUsage::kShaderResource;
	texDesc.m_CreateFlags = TextureCreateFlags::kCreateCubemap;
	auto& subresources = pTextureBlob->GetSubresources();
	TextureInitData initData{ m_Context, u32(subresources.Size()), subresources.Data() };

	m_TextureSkybox = m_Device->CreateTexture(texDesc, &initData);

	TextureViewDesc textureViewDesc;
	textureViewDesc.m_pTexture = m_TextureSkybox;
	textureViewDesc.m_Format = texDesc.m_Format;
	textureViewDesc.m_ViewType = TextureViewType::kTextureCube;

	m_TextureViewSkybox = m_Device->CreateTextureView(textureViewDesc);

	SamplerDesc samDesc;
	samDesc.m_AddressModeU = samDesc.m_AddressModeV = samDesc.m_AddressModeW = AddressMode::kClamp;
	samDesc.m_AnisotropyEnable = true;
	samDesc.m_MaxAnisotropy = 16;
	m_SamplerSkybox = m_Device->CreateSampler(samDesc);

	BufferDesc bufferDesc;
	BufferViewDesc viewDesc;

	bufferDesc.m_Size = sizeof(Float44);
	bufferDesc.m_UsageFlags = BufferUsage::kConstantBuffer;
	bufferDesc.m_MemoryType = MemoryType::kUpload;
	bufferDesc.m_CreateFlags = BufferCreateFlags::kCreateMapped;
	m_UBSkybox = m_Device->CreateBuffer(bufferDesc, nullptr);
	m_pWVPSkybox = m_UBSkybox->GetMappedDataAsT<Float44>();

	viewDesc.m_pBuffer = m_UBSkybox;
	viewDesc.m_Stride = sizeof(Float44);
	viewDesc.m_ElementCount = 1;
	m_UBViewSkybox = m_Device->CreateBufferView(viewDesc);

	auto vs = CompileShader(g_pVSSkyBoxShader, g_VSSkyBoxSize, ShaderStage::kVertex);
	auto ps = CompileShader(g_pPSSkyBoxShader, g_PSSkyBoxSize, ShaderStage::kPixelOrFragment);
	GraphicsPipelineStateDesc pipelineDesc;
	pipelineDesc.m_Shaders[0] = vs;
	pipelineDesc.m_Shaders[1] = ps;
	pipelineDesc.m_RenderTargetFormats[0] = m_SwapChain->GetDesc().m_Format;
	pipelineDesc.m_DepthStencilFormat = m_DepthView->GetDesc().m_Format;
	pipelineDesc.m_DepthStencilDesc.m_DepthTestEnable = true;
	pipelineDesc.m_DepthStencilDesc.m_DepthWriteEnable = false;
	pipelineDesc.m_DepthStencilDesc.m_DepthOp = CompareOp::kLessEqual;
	pipelineDesc.m_pShaderResourceLayout = m_SRL;
	pipelineDesc.m_RasterizerStateDesc.m_CullMode = CullMode::kFront;

	VertexInputDesc inputDescs[1];
	inputDescs[0].m_Format = Format::kRGB32_Float;

	pipelineDesc.m_VertexInputDescCount = 1;
	pipelineDesc.m_pVertexInputDescs = inputDescs;

	m_PSOSkybox = m_Device->CreateGraphicsPipeline(pipelineDesc);
}


SAMPLE_MAIN(Cubemap)