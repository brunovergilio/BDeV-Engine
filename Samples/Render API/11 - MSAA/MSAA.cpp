#include "MSAA.h"
#include "BDeV/Core/Math/BvGeometryGenerator.h"
#include "BDeV/Core/Utils/BvRandom.h"
#include "Shaders.h"


bool g_HasDepthResolve = false;


struct Vertex
{
	XMFLOAT3 pos;
	XMFLOAT4 color;
};


void MSAA::OnInitialize()
{
	m_AppName = "MSAA";
	g_HasDepthResolve = EHasFlag(m_Device->GetGPUInfo().m_DeviceCaps, RenderDeviceCapabilities::kDepthStencilResolve);
	CreateBuffers();
	CreateRenderTargets();
	CreateShaderResourceLayout();
	CreatePipeline();
}


void MSAA::OnInitializeUI()
{
	m_Overlay.SetupPipeline(m_SwapChain->GetDesc().m_Format, m_DepthView->GetDesc().m_Format);
	m_Overlay.SetupPipeline(m_SwapChain->GetDesc().m_Format, m_DepthView->GetDesc().m_Format, 8);
}


void MSAA::OnUpdate()
{
	auto width = m_pWindow->GetWidth();
	auto height = m_pWindow->GetHeight();
	static f32 speed = 0.1f;
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


void MSAA::OnUpdateUI()
{
	BeginDrawDefaultUI();
	ImGui::Checkbox("MSAA", &m_UseMSAA);
	EndDrawDefaultUI();
}


void MSAA::OnRender()
{
	auto width = m_pWindow->GetWidth();
	auto height = m_pWindow->GetHeight();
	BvFixedVector<RenderTargetDesc, kMaxRenderTargetsWithDepth> targets;
	if (m_UseMSAA)
	{
		targets.EmplaceBack().SetColorView(m_ColorViewMS, ResourceState::kCommon, ResourceState::kRenderTarget).SetStoreOp(StoreOp::kDontCare)
			.SetResolveView(m_SwapChain->GetCurrentTextureView(), ResourceState::kCommon, ResourceState::kPresent, ResolveMode::kAverage)
			.SetClearValues({ 0.1f, 0.1f, 0.3f });

		targets.EmplaceBack().SetDepthStencilView(m_DepthViewMS, ResourceState::kCommon, ResourceState::kDepthStencilWrite).SetStoreOp(StoreOp::kDontCare)
			.SetResolveView(m_DepthView, ResourceState::kCommon, ResourceState::kDepthStencilWrite, ResolveMode::kMin)
			.SetClearValues({ 1.0f, 0 });
	}
	else
	{
		targets.EmplaceBack().SetColorView(m_SwapChain->GetCurrentTextureView(), ResourceState::kCommon, ResourceState::kPresent).SetClearValues({ 0.1f, 0.1f, 0.3f });
		targets.EmplaceBack().SetDepthStencilView(m_DepthView, ResourceState::kCommon, ResourceState::kDepthStencilWrite).SetClearValues({ 1.0f, 0 });
	}

	m_Context->NewCommandList();
	m_Context->SetRenderTargets(targets.Size(), targets.Data());
	m_Context->SetGraphicsPipeline(m_UseMSAA ? m_PSOMSAA : m_PSO);
	m_Context->SetViewport({ 0.0f, 0.0f, (f32)width, (f32)height, 0.0f, 1.0f });
	m_Context->SetScissor({ 0, 0, i32(width), i32(height) });
	m_Context->SetConstantBuffer(m_UBView, 0, 0);
	m_Context->SetVertexBufferView(m_VB, sizeof(Vertex));
	m_Context->SetIndexBufferView(m_IB, IndexFormat::kU32);
	m_Context->DrawIndexed(36);
	OnRenderUI();
	m_Context->Execute();

	m_SwapChain->Present(false);

	m_Context->FlushFrame();
}


void MSAA::OnRenderUI()
{
	m_Overlay.Render(m_UseMSAA);
}


void MSAA::OnShutdown()
{
	m_UB.Reset();
	m_VB.Reset();
	m_IB.Reset();
	m_UBView.Reset();
	m_Depth.Reset();
	m_DepthView.Reset();
	m_ColorViewMS.Reset();
	m_ColorMS.Reset();
	m_DepthViewMS.Reset();
	m_DepthMS.Reset();
	m_PSOMSAA.Reset();
	m_PSO.Reset();
	m_SRL.Reset();
}


void MSAA::CreateShaderResourceLayout()
{
	ShaderResourceLayoutCreateDesc layoutDesc;
	layoutDesc.AddResourceSet().AddConstantBuffer(0, ShaderStage::kVertex);

	m_Device->CreateShaderResourceLayout(layoutDesc, &m_SRL);
}


void MSAA::CreatePipeline()
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
	pipelineDesc.AddVertexInput("POSITION", 0, Format::kRGB32_Float).AddVertexInput("COLOR", 0, Format::kRGBA32_Float);

	m_Device->CreateGraphicsPipeline(pipelineDesc, &m_PSO);
	pipelineDesc.m_SampleCount = 8;
	m_Device->CreateGraphicsPipeline(pipelineDesc, &m_PSOMSAA);
}


void MSAA::CreateBuffers()
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


void MSAA::CreateRenderTargets()
{
	auto w = m_pWindow->GetWidth();
	auto h = m_pWindow->GetHeight();

	TextureDesc desc;
	desc.m_Size = { w, h, 1 };
	desc.m_Format = Format::kD24_UNorm_S8_UInt;
	desc.m_UsageFlags = TextureUsage::kDepthStencilTarget;
	m_Device->CreateTexture(desc, &m_Depth);

	desc.m_SampleCount = 8;
	m_Device->CreateTexture(desc, &m_DepthMS);

	TextureViewDesc viewDesc;
	viewDesc.m_Format = desc.m_Format;
	viewDesc.m_pTexture = m_Depth;
	m_Device->CreateTextureView(viewDesc, &m_DepthView);
	
	viewDesc.m_pTexture = m_DepthMS;
	m_Device->CreateTextureView(viewDesc, &m_DepthViewMS);

	desc.m_Format = m_SwapChain->GetDesc().m_Format;
	desc.m_UsageFlags = TextureUsage::kRenderTarget;
	m_Device->CreateTexture(desc, &m_ColorMS);

	viewDesc.m_Format = desc.m_Format;
	viewDesc.m_pTexture = m_ColorMS;
	m_Device->CreateTextureView(viewDesc, &m_ColorViewMS);
}


SAMPLE_MAIN(MSAA)