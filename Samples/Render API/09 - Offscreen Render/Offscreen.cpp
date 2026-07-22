#include "Offscreen.h"
#include "BDeV/Core/Math/BvGeometryGenerator.h"
#include "BDeV/Core/Utils/BvRandom.h"
#include "Shaders.h"


struct Vertex
{
	XMFLOAT3 pos;
	XMFLOAT4 color;
};


void Offscreen::OnInitialize()
{
	m_AppName = "Offscreen Rendering";
	CreateBuffers();
	CreateRenderTargets();
	CreateShaderResourceLayout();
	CreatePipeline();

	m_PCColor.m_PosUV[0].x = -1.0f;
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

	m_PCColor.m_PosUV[3].x = -1.0f;
	m_PCColor.m_PosUV[3].y = 1.0f;
	m_PCColor.m_PosUV[3].z = 0.0f;
	m_PCColor.m_PosUV[3].w = 0.0f;

	m_PCColor.m_PosUV[4].x = 1.0f;
	m_PCColor.m_PosUV[4].y = -1.0f;
	m_PCColor.m_PosUV[4].z = 1.0f;
	m_PCColor.m_PosUV[4].w = 1.0f;

	m_PCColor.m_PosUV[5].x = -1.0f;
	m_PCColor.m_PosUV[5].y = -1.0f;
	m_PCColor.m_PosUV[5].z = 0.0f;
	m_PCColor.m_PosUV[5].w = 1.0f;

	f32 width = m_pWindow->GetWidth();
	m_ScreenSizeAndMid = XMFLOAT2(width, m_ScreenMid);
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
	XMStoreFloat4x4(m_pWVP, BvMatrix::RotationX(angleX) * BvMatrix::RotationY(angleY) * BvMatrix::RotationZ(angleZ)
		* BvMatrix::LookAtLH(BvVec3(0.0f, 0.0f, -5.0f), BvVec3(0.0f, 0.0f, 1.0f), BvVec3(0.0f, 1.0f, 0.0f))
		* BvMatrix::PerspectiveLH_DX(0.1f, 100.0f, float(width) / float(height), kPiDiv4));
}


void Offscreen::OnUpdateUI()
{
	BeginDrawDefaultUI();
	ImGui::SliderFloat("Screen Divisor", &m_ScreenMid, 0.0f, 1.0f);
	EndDrawDefaultUI();
}


void Offscreen::RenderOffscreen()
{
	auto width = m_pWindow->GetWidth();
	auto height = m_pWindow->GetHeight();
	RenderTargetDesc targets[] =
	{
		RenderTargetDesc::AsRenderTarget(m_ColorView, { 0.1f, 0.1f, 0.3f }),
		RenderTargetDesc::AsDepthStencil(m_DepthView)
	};

	m_Context->SetRenderTargets(2, targets);
	m_Context->SetGraphicsPipeline(m_PSOOffscreen);
	m_Context->SetViewport({ 0.0f, 0.0f, (f32)width, (f32)height, 0.0f, 1.0f });
	m_Context->SetScissor({ 0, 0, i32(width), i32(height) });
	m_Context->SetConstantBuffer(m_UBView, 0, 0);
	m_Context->SetVertexBufferView(m_VB, sizeof(Vertex));
	m_Context->SetIndexBufferView(m_IB, IndexFormat::kU32);
	m_Context->DrawIndexed(36);
}


void Offscreen::OnRender()
{
	f32 width = m_pWindow->GetWidth();
	f32 height = m_pWindow->GetHeight();
	m_ScreenSizeAndMid.y = m_ScreenMid;

	RenderTargetDesc mainTarget = RenderTargetDesc::AsSwapChain(m_SwapChain->GetCurrentTextureView(), { 0.1f, 0.1f, 0.3f });

	m_Context->NewCommandList();
	RenderOffscreen();
	m_Context->SetRenderTarget(mainTarget);

	m_Context->SetGraphicsPipeline(m_PSO);
	m_Context->SetShaderConstantsT<PCData>(m_PCColor, 3, 0);
	m_Context->SetShaderConstantsT<XMFLOAT2>(m_ScreenSizeAndMid, 4, 0);
	m_Context->SetTexture(m_ColorView, 0, 1);
	m_Context->SetTexture(m_DepthView, 0, 2);
	m_Context->SetSampler(m_Sampler, 0, 0);
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
	m_DepthView.Reset();
	m_Depth.Reset();
	m_ColorView.Reset();
	m_Color.Reset();
	m_PSO.Reset();
	m_PSOOffscreen.Reset();
	m_SRL.Reset();
	m_SRLOffscreen.Reset();
	m_Sampler.Reset();
}


void Offscreen::CreateShaderResourceLayout()
{
	{
		ShaderResourceLayoutCreateDesc layoutDesc;
		layoutDesc.AddResourceSet().AddConstantBuffer(0, ShaderStage::kVertex);

		m_Device->CreateShaderResourceLayout(layoutDesc, &m_SRLOffscreen);
	}

	{
		ShaderResourceDesc resourceDescs[] =
		{
			ShaderResourceDesc::AsSampler(0, ShaderStage::kPixelOrFragment),
			ShaderResourceDesc::AsTexture(1, ShaderStage::kPixelOrFragment),
			ShaderResourceDesc::AsTexture(2, ShaderStage::kPixelOrFragment),
		};

		ShaderResourceConstantDesc constantDescs[] =
		{
			ShaderResourceConstantDesc::As<PCData>("PC"_sid, 3, ShaderStage::kVertex),
			ShaderResourceConstantDesc::As<XMFLOAT2>("PC2"_sid, 4, ShaderStage::kPixelOrFragment),
		};

		ShaderResourceLayoutCreateDesc layoutDesc;
		layoutDesc.AddResourceSet()
			.AddConstant<PCData>("PC"_sid, 3, ShaderStage::kVertex).AddConstant<XMFLOAT2>("PC2"_sid, 4, ShaderStage::kPixelOrFragment)
			.AddSampler(0, ShaderStage::kPixelOrFragment).AddTexture(1, ShaderStage::kPixelOrFragment).AddTexture(2, ShaderStage::kPixelOrFragment);

		m_Device->CreateShaderResourceLayout(layoutDesc, &m_SRL);
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
		pipelineDesc.AddVertexInput("POSITION", 0, Format::kRGB32_Float).AddVertexInput("COLOR", 0, Format::kRGBA32_Float);

		m_Device->CreateGraphicsPipeline(pipelineDesc, &m_PSOOffscreen);
	}

	{
		auto vs = CompileShader(g_pVSShader, g_VSSize, ShaderStage::kVertex);
		auto ps = CompileShader(g_pPSShader, g_PSSize, ShaderStage::kPixelOrFragment);
		GraphicsPipelineStateDesc pipelineDesc;
		pipelineDesc.m_Shaders[0] = vs;
		pipelineDesc.m_Shaders[1] = ps;
		pipelineDesc.m_RenderTargetFormats[0] = m_SwapChain->GetDesc().m_Format;
		pipelineDesc.m_pShaderResourceLayout = m_SRL;

		m_Device->CreateGraphicsPipeline(pipelineDesc, &m_PSO);

	}
}


void Offscreen::CreateBuffers()
{
	BvGeometryGenerator gen;
	gen.GenerateBox();
	auto& data = gen.GetData();
	BvVector<Vertex> vertices(data.m_Vertices.Size());
	BvRandom32 rand;

	for (auto i = 0u; i < vertices.Size(); ++i)
	{
		vertices[i].pos = data.m_Vertices[i].m_Position;
		vertices[i].color = XMFLOAT4(rand.NextF<f32>(), rand.NextF<f32>(), rand.NextF<f32>(), 1.0f);
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
	m_Device->CreateBuffer(bufferDesc, &m_UB);

	viewDesc.m_pBuffer = m_UB;
	viewDesc.m_Stride = sizeof(XMFLOAT4X4);
	viewDesc.m_ElementCount = 1;
	m_Device->CreateBufferView(viewDesc, &m_UBView);

	m_pWVP = m_UB->GetMappedDataAsT<XMFLOAT4X4>();
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
	m_Device->CreateTexture(desc, &m_Color);

	viewDesc.m_Format = desc.m_Format;
	viewDesc.m_pTexture = m_Color;
	m_Device->CreateTextureView(viewDesc, &m_ColorView);

	desc.m_Size = { w, h, 1 };
	desc.m_Format = Format::kD24_UNorm_S8_UInt;
	desc.m_UsageFlags = TextureUsage::kDepthStencilTarget | TextureUsage::kShaderResource;
	m_Device->CreateTexture(desc, &m_Depth);

	viewDesc.m_Format = desc.m_Format;
	viewDesc.m_pTexture = m_Depth;
	m_Device->CreateTextureView(viewDesc, &m_DepthView);

	m_Device->CreateSampler(SamplerDesc(), &m_Sampler);
}


SAMPLE_MAIN(Offscreen)