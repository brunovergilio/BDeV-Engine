#include "Texture3D.h"
#include "BDeV/Core/Math/BvGeometryGenerator.h"
#include "BDeV/Core/Utils/BvRandom.h"
#include "Shaders.h"


struct Vertex
{
	XMFLOAT3 pos;
	XMFLOAT2 uv;
};


constexpr Extent3D kDim{ 256, 256, 256 };
constexpr Format kPf = Format::kRGBA8_UNorm;
constexpr u32 kByteSize = BvRenderUtils::GetFormatInfo(kPf).m_BitsPerPixel >> 3;


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
	XMStoreFloat4x4(m_pWVP, BvMatrix::RotationX(angleX) * BvMatrix::RotationY(angleY) * BvMatrix::RotationZ(angleZ)
		* BvMatrix::LookAtLH(BvVec3(0.0f, 0.0f, -5.0f), BvVec3(0.0f, 0.0f, 1.0f), BvVec3(0.0f, 1.0f, 0.0f))
		* BvMatrix::PerspectiveLH_DX(0.1f, 100.0f, float(width) / float(height), kPiDiv4));
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
	m_Context->SetScissor({ 0, 0, i32(width), i32(height) });
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
	ShaderResourceLayoutCreateDesc layoutDesc;
	layoutDesc.AddResourceSet().AddConstant<i32>("PushConstants", 3, ShaderStage::kPixelOrFragment)
		.AddConstantBuffer(0, ShaderStage::kVertex).AddTexture(1, ShaderStage::kPixelOrFragment).AddSampler(2, ShaderStage::kPixelOrFragment);

	m_Device->CreateShaderResourceLayout(layoutDesc, &m_SRL);
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
	pipelineDesc.AddVertexInput("POSITION", 0, Format::kRGB32_Float).AddVertexInput("TEXCOORD", 0, Format::kRG32_Float);

	m_Device->CreateGraphicsPipeline(pipelineDesc, &m_PSO);
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
	m_Device->CreateBuffer(bufferDesc, bufferData, &m_VB);

	bufferDesc.m_Size = sizeof(u32) * data.m_Indices.Size();
	bufferDesc.m_UsageFlags = BufferUsage::kIndexBuffer;
	bufferData.m_pContext = m_Context;
	bufferData.m_pData = data.m_Indices.Data();
	bufferData.m_Size = bufferDesc.m_Size;
	m_Device->CreateBuffer(bufferDesc, bufferData, &m_IB);

	bufferDesc.m_Size = sizeof(XMFLOAT4X4);
	bufferDesc.m_UsageFlags = BufferUsage::kConstantBuffer;
	bufferDesc.m_MemoryType = MemoryType::kUpload;
	bufferDesc.m_CreateFlags = BufferCreateFlags::kCreateMapped;
	m_Device->CreateBuffer(bufferDesc, bufferData, &m_UB);

	viewDesc.m_pBuffer = m_UB;
	viewDesc.m_Stride = sizeof(XMFLOAT4X4);
	viewDesc.m_ElementCount = 1;
	m_Device->CreateBufferView(viewDesc, &m_UBView);

	m_pWVP = m_UB->GetMappedDataAsT<XMFLOAT4X4>();
}


void Texture3D::CreateTextures()
{
	BvVector<u8> textureData(kDim.m_X * kDim.m_Y * kDim.m_Z * kByteSize);
	for (auto w = 0; w < kDim.m_X; ++w)
	{
		for (auto h = 0; h < kDim.m_Y; ++h)
		{
			for (auto d = 0; d < kDim.m_Z; ++d)
			{
				i32 index = (d * kDim.m_X * kDim.m_Y + h * kDim.m_X + w) * kByteSize;

				f32 fx = (f32)w / (f32)(kDim.m_X - 1);
				f32 fy = (f32)h / (f32)(kDim.m_Y - 1);
				f32 fz = (f32)d / (f32)(kDim.m_Z - 1);

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
	texDesc.m_UsageFlags = TextureUsage::kShaderResource;
	SubresourceData data{ textureData.Data(), kDim.m_Width * kByteSize, kDim.m_Width * kByteSize * kDim.m_Height };
	TextureInitData initData{ m_Context, 1, &data };

	m_Device->CreateTexture(texDesc, initData, &m_Texture);

	TextureViewDesc viewDesc;
	viewDesc.m_pTexture = m_Texture;
	viewDesc.m_Format = texDesc.m_Format;
	viewDesc.m_ViewType = TextureViewType::kTexture3D;

	m_Device->CreateTextureView(viewDesc, &m_TextureView);

	m_Device->CreateSampler(SamplerDesc(), &m_Sampler);
}


void Texture3D::CreateRenderTargets()
{
	auto w = m_pWindow->GetWidth();
	auto h = m_pWindow->GetHeight();

	TextureDesc desc;
	desc.m_Size = { w, h, 1 };
	desc.m_Format = Format::kD24_UNorm_S8_UInt;
	desc.m_UsageFlags = TextureUsage::kDepthStencilTarget;
	m_Device->CreateTexture(desc, &m_Depth);

	TextureViewDesc viewDesc;
	viewDesc.m_Format = desc.m_Format;
	viewDesc.m_pTexture = m_Depth;
	m_Device->CreateTextureView(viewDesc, &m_DepthView);
}


SAMPLE_MAIN(Texture3D)