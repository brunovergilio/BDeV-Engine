#include "CubemapArray.h"
#include "BDeV/Core/Math/BvGeometryGenerator.h"
#include "Shaders.h"


struct Vertex
{
	XMFLOAT3 pos;
	XMFLOAT2 uv;
};


void CubemapArray::OnInitialize()
{
	m_AppName = "Cubemap Texture Array";
	CreateBuffers();
	CreateTextures();
	CreateRenderTargets();
	CreateShaderResourceLayout();
	CreatePipeline();
	CreateSkyboxResources();
}


void CubemapArray::OnInitializeUI()
{
	m_Overlay.SetupPipeline(m_SwapChain->GetDesc().m_Format, m_DepthView->GetDesc().m_Format);
}


void CubemapArray::OnUpdate()
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

	BvVec3 eyeDir((lookPos - camPos) * BvMatrix::RotationY(camAngleY));
	BvMatrix v(BvMatrix::LookAtLH(camPos, eyeDir, BvVec3(0.0f, 1.0f, 0.0f)));
	BvMatrix p(BvMatrix::PerspectiveLH_DX(0.1f, 1000.0f, float(width) / float(height), kPiDiv4));
	BvMatrix vp(v * p);
	BvMatrix rot(BvMatrix::RotationX(angleX) * BvMatrix::RotationY(angleY) * BvMatrix::RotationZ(angleZ));

	XMStoreFloat4x4(m_pWVP, rot * vp);
	XMStoreFloat4x4(m_pWVPSkybox, BvMatrix::Translation(camPos) * vp);
}

void CubemapArray::OnUpdateUI()
{
	BeginDrawDefaultUI();
	ImGui::SliderInt("Cubemap Index", &m_CurrCubemapIndex, 0, m_MaxCubemapIndex);
	EndDrawDefaultUI();
}


void CubemapArray::OnRender()
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
	m_Context->SetVertexBufferView(m_VB, sizeof(Vertex));
	m_Context->SetIndexBufferView(m_IB, IndexFormat::kU32);
	m_Context->DrawIndexed(36);
	RenderSkybox();
	OnRenderUI();
	m_Context->Execute();

	m_SwapChain->Present(false);

	m_Context->FlushFrame();
}


void CubemapArray::OnShutdown()
{
	m_SamplerSkybox.Reset();
	m_TextureViewSkybox.Reset();
	m_TextureSkybox.Reset();
	m_UBSkybox.Reset();
	m_UBViewSkybox.Reset();
	m_SRLSkybox.Reset();
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


void CubemapArray::RenderSkybox()
{
	m_Context->SetGraphicsPipeline(m_PSOSkybox);
	m_Context->SetConstantBuffer(m_UBViewSkybox, 0, 0);
	m_Context->SetTexture(m_TextureViewSkybox, 0, 1);
	m_Context->SetSampler(m_SamplerSkybox, 0, 2);
	m_Context->SetShaderConstantsT<i32>(m_CurrCubemapIndex, 3, 0);
	m_Context->DrawIndexed(36);
}


void CubemapArray::CreateShaderResourceLayout()
{
	ShaderResourceLayoutCreateDesc layoutDesc;
	layoutDesc.AddResourceSet()
		.AddConstantBuffer(0, ShaderStage::kVertex)
		.AddTexture(1, ShaderStage::kPixelOrFragment)
		.AddSampler(2, ShaderStage::kPixelOrFragment);

	m_Device->CreateShaderResourceLayout(layoutDesc, &m_SRL);
}


void CubemapArray::CreatePipeline()
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


void CubemapArray::CreateBuffers()
{
	BvGeometryGenerator gen;
	gen.GenerateBox();
	auto& data = gen.GetData();
	BvVector<Vertex> vertices(data.m_Vertices.Size());

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
	m_Device->CreateBuffer(bufferDesc, &m_UB);

	viewDesc.m_pBuffer = m_UB;
	viewDesc.m_Stride = sizeof(XMFLOAT4X4);
	viewDesc.m_ElementCount = 1;
	m_Device->CreateBufferView(viewDesc, &m_UBView);

	m_pWVP = m_UB->GetMappedDataAsT<XMFLOAT4X4>();
}


void CubemapArray::CreateTextures()
{
	BvRCRef<IBvTextureBlob> pTextureBlob;
	m_TextureLoader->LoadTextureFromFile("water.dds", &pTextureBlob);
	auto& info = pTextureBlob->GetInfo();
	TextureDesc texDesc;
	texDesc.m_Size = { info.m_Width, info.m_Height, info.m_Depth };
	texDesc.m_ArraySize = info.m_ArraySize;
	texDesc.m_Format = info.m_Format;
	texDesc.m_MipLevels = info.m_MipLevels;
	texDesc.m_ImageType = info.m_TextureType;
	texDesc.m_UsageFlags = TextureUsage::kShaderResource;
	auto& subresources = pTextureBlob->GetSubresources();
	TextureInitData initData;
	initData.m_pContext = m_Context;
	initData.m_SubresourceCount = subresources.Size();
	initData.m_pSubresources = subresources.Data();
	initData.m_ResourceState = ResourceState::kPixelShaderResource;

	m_Device->CreateTexture(texDesc, initData, &m_Texture);

	TextureViewDesc viewDesc;
	viewDesc.m_pTexture = m_Texture;
	viewDesc.m_Format = texDesc.m_Format;

	m_Device->CreateTextureView(viewDesc, &m_TextureView);

	m_Device->CreateSampler(SamplerDesc(), &m_Sampler);
}


void CubemapArray::CreateRenderTargets()
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


void CubemapArray::CreateSkyboxResources()
{
	BvRCRef<IBvTextureBlob> pTextureBlob;
	m_TextureLoader->LoadTextureFromFile("cubearraymip.dds", &pTextureBlob);
	auto& info = pTextureBlob->GetInfo();
	TextureDesc texDesc;
	texDesc.m_Size = { info.m_Width, info.m_Height, info.m_Depth };
	texDesc.m_ArraySize = info.m_ArraySize;
	texDesc.m_Format = info.m_Format;
	texDesc.m_MipLevels = info.m_MipLevels;
	texDesc.m_ImageType = info.m_TextureType;
	texDesc.m_UsageFlags = TextureUsage::kShaderResource;
	texDesc.m_CreateFlags = TextureCreateFlags::kCreateCubemap;
	auto& subresources = pTextureBlob->GetSubresources();
	TextureInitData initData;
	initData.m_pContext = m_Context;
	initData.m_SubresourceCount = subresources.Size();
	initData.m_pSubresources = subresources.Data();
	initData.m_ResourceState = ResourceState::kPixelShaderResource;

	m_Device->CreateTexture(texDesc, initData, &m_TextureSkybox);

	m_MaxCubemapIndex = info.m_ArraySize / 6 - 1;

	TextureViewDesc textureViewDesc;
	textureViewDesc.m_pTexture = m_TextureSkybox;
	textureViewDesc.m_Format = texDesc.m_Format;
	textureViewDesc.m_ViewType = TextureViewType::kTextureCubeArray;

	m_Device->CreateTextureView(textureViewDesc, &m_TextureViewSkybox);

	SamplerDesc samDesc;
	samDesc.m_AddressModeU = samDesc.m_AddressModeV = samDesc.m_AddressModeW = AddressMode::kClamp;
	samDesc.m_AnisotropyEnable = true;
	samDesc.m_MaxAnisotropy = 16;
	m_Device->CreateSampler(samDesc, &m_SamplerSkybox);

	BufferDesc bufferDesc;
	BufferViewDesc viewDesc;

	bufferDesc.m_Size = sizeof(XMFLOAT4X4);
	bufferDesc.m_UsageFlags = BufferUsage::kConstantBuffer;
	bufferDesc.m_MemoryType = MemoryType::kUpload;
	bufferDesc.m_CreateFlags = BufferCreateFlags::kCreateMapped;
	m_Device->CreateBuffer(bufferDesc, &m_UBSkybox);
	m_pWVPSkybox = m_UBSkybox->GetMappedDataAsT<XMFLOAT4X4>();

	viewDesc.m_pBuffer = m_UBSkybox;
	viewDesc.m_Stride = sizeof(XMFLOAT4X4);
	viewDesc.m_ElementCount = 1;
	m_Device->CreateBufferView(viewDesc, &m_UBViewSkybox);

	ShaderResourceLayoutCreateDesc layoutDesc;
	layoutDesc.AddResourceSet().AddConstant<i32>("PushConstants", 3, ShaderStage::kPixelOrFragment)
		.AddConstantBuffer(0, ShaderStage::kVertex).AddTexture(1, ShaderStage::kPixelOrFragment).AddSampler(2, ShaderStage::kPixelOrFragment);

	m_Device->CreateShaderResourceLayout(layoutDesc, &m_SRLSkybox);

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
	pipelineDesc.m_pShaderResourceLayout = m_SRLSkybox;
	pipelineDesc.m_RasterizerStateDesc.m_CullMode = CullMode::kFront;
	pipelineDesc.AddVertexInput("POSITION", 0, Format::kRGB32_Float);

	m_Device->CreateGraphicsPipeline(pipelineDesc, &m_PSOSkybox);
}


SAMPLE_MAIN(CubemapArray)