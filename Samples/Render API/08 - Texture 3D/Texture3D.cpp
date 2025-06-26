#include "Texture3D.h"
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

layout (binding = 1) uniform texture3D samplerTexture;
layout (binding = 2) uniform sampler samplerObj;

layout (push_constant) uniform PushConstants
{
	float depth;
} pushConstants;

void main()
{
	outColor = texture(sampler3D(samplerTexture, samplerObj), vec3(inTexCoords, pushConstants.depth));
}
)raw";
constexpr auto g_PSSize = std::char_traits<char>::length(g_pPSShader);


constexpr Extent3D kDim{ 256, 256, 256 };
constexpr Format kPf = Format::kRGBA8_UNorm;
constexpr u32 kByteSize = GetFormatInfo(kPf).m_BitsPerPixel >> 3;


void Texture3D::OnInitialize()
{
	m_AppName = "3D Texture";
	CreateBuffers();
	CreateTextures();
	CreateRenderTargets();
	CreateShaderResourceLayout();
	CreatePipeline();
}


void Texture3D::OnInitializeUI()
{
	m_Overlay.SetupPipeline(m_SwapChain->GetDesc().m_Format, m_DepthView->GetDesc().m_Format);
}


void Texture3D::OnUpdate()
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


void Texture3D::OnUpdateUI()
{
	BeginDrawDefaultUI();
	ImGui::SliderFloat("Depth Slice", &m_CurrDepthSlice, 0.0f, 1.0f);
	EndDrawDefaultUI();
}


void Texture3D::OnRender()
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
	m_Context->SetShaderConstantsT<f32>(m_CurrDepthSlice, 3, 0);
	m_Context->SetVertexBufferView(m_VB, sizeof(Vertex));
	m_Context->SetIndexBufferView(m_IB, IndexFormat::kU32);
	m_Context->DrawIndexed(36);
	OnRenderUI();
	m_Context->Execute();

	m_SwapChain->Present(false);

	m_Context->FlushFrame();
}


void Texture3D::OnShutdown()
{
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


void Texture3D::CreateShaderResourceLayout()
{
	ShaderResourceDesc resourceDescs[] =
	{
		ShaderResourceDesc::AsConstantBuffer(0, ShaderStage::kVertex),
		ShaderResourceDesc::AsTexture(1, ShaderStage::kPixelOrFragment),
		ShaderResourceDesc::AsSampler(2, ShaderStage::kPixelOrFragment)
	};
	ShaderResourceConstantDesc constantDesc[] =
	{
		ShaderResourceConstantDesc::As<f32>("PushConstants", 3, ShaderStage::kPixelOrFragment)
	};

	ShaderResourceSetDesc setDesc{};
	setDesc.m_ResourceCount = 3;
	setDesc.m_pResources = resourceDescs;
	setDesc.m_ConstantCount = 1;
	setDesc.m_pConstants = constantDesc;

	ShaderResourceLayoutDesc layoutDesc{};
	layoutDesc.m_ShaderResourceSetCount = 1;
	layoutDesc.m_pShaderResourceSets = &setDesc;

	m_SRL = m_Device->CreateShaderResourceLayout(layoutDesc);
}


void Texture3D::CreatePipeline()
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


void Texture3D::CreateBuffers()
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
	m_UB = m_Device->CreateBuffer(bufferDesc, &bufferData);

	viewDesc.m_pBuffer = m_UB;
	viewDesc.m_Stride = sizeof(Float44);
	viewDesc.m_ElementCount = 1;
	m_UBView = m_Device->CreateBufferView(viewDesc);

	m_pWVP = m_UB->GetMappedDataAsT<Float44>();
}


void Texture3D::CreateTextures()
{
	BvVector<u8> textureData(kDim.x * kDim.y * kDim.z * kByteSize);
	for (auto w = 0; w < kDim.x; ++w)
	{
		for (auto h = 0; h < kDim.y; ++h)
		{
			for (auto d = 0; d < kDim.z; ++d)
			{
				i32 index = (d * kDim.x * kDim.y + h * kDim.x + w) * kByteSize;

				f32 fx = (f32)w / (f32)(kDim.x - 1);
				f32 fy = (f32)h / (f32)(kDim.y - 1);
				f32 fz = (f32)d / (f32)(kDim.z - 1);

				textureData[index + 0] = static_cast<uint8_t>(fx * 16.0f);
				textureData[index + 1] = static_cast<uint8_t>(fy * 128.0f);
				textureData[index + 2] = static_cast<uint8_t>(fz * 255.0f);
				textureData[index + 3] = 255;
			}
		}
	}

	TextureDesc texDesc;
	texDesc.m_Size = kDim;
	texDesc.m_ArraySize = 1;
	texDesc.m_Format = kPf;
	texDesc.m_MipLevels = 1;
	texDesc.m_ImageType = TextureType::kTexture3D;
	texDesc.m_ResourceState = ResourceState::kPixelShaderResource;
	texDesc.m_UsageFlags = TextureUsage::kShaderResource;
	texDesc.m_CreateFlags = TextureCreateFlags::kGenerateMips;
	SubresourceData data{ textureData.Data(), kDim.width * kByteSize, kDim.width * kByteSize * kDim.height };
	TextureInitData initData{ m_Context, 1, &data };

	m_Texture = m_Device->CreateTexture(texDesc, &initData);

	TextureViewDesc viewDesc;
	viewDesc.m_pTexture = m_Texture;
	viewDesc.m_Format = texDesc.m_Format;
	viewDesc.m_ViewType = TextureViewType::kTexture3D;

	m_TextureView = m_Device->CreateTextureView(viewDesc);

	m_Sampler = m_Device->CreateSampler(SamplerDesc());
}


void Texture3D::CreateRenderTargets()
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


SAMPLE_MAIN(Texture3D)