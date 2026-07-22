#include "MSAARenderPass.h"
#include "BDeV/Core/Math/BvGeometryGenerator.h"
#include "BDeV/Core/Utils/BvRandom.h"
#include "Shaders.h"


bool g_HasDepthResolve = false;


struct Vertex
{
	XMFLOAT3 pos;
	XMFLOAT4 color;
};



void MSAARenderPass::OnInitialize()
{
	m_AppName = "MSAA (Render Pass)";
	g_HasDepthResolve = EHasFlag(m_Device->GetGPUInfo().m_DeviceCaps, RenderDeviceCapabilities::kDepthStencilResolve);
	CreateBuffers();
	CreateRenderTargets();
	CreateRenderPass();
	CreateRenderPassMSAA();
	CreateShaderResourceLayout();
	CreatePipeline();
}


void MSAARenderPass::OnInitializeUI()
{
	m_Overlay.SetupPipeline(m_RenderPass);
	m_Overlay.SetupPipeline(m_RenderPassMSAA, 0, 8);
}


void MSAARenderPass::OnUpdate()
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


void MSAARenderPass::OnUpdateUI()
{
	BeginDrawDefaultUI();
	ImGui::Checkbox("MSAA", &m_UseMSAA);
	EndDrawDefaultUI();
}


void MSAARenderPass::OnRender()
{
	auto width = m_pWindow->GetWidth();
	auto height = m_pWindow->GetHeight();

	RenderPassTargetDesc targets[4];
	if (m_UseMSAA)
	{
		targets[0] = RenderPassTargetDesc(m_ColorViewMS, 0.1f, 0.1f, 0.3f);
		targets[1] = RenderPassTargetDesc(m_DepthViewMS, 1.0f, 0);
		targets[2] = RenderPassTargetDesc(m_SwapChain->GetCurrentTextureView(), 0.1f, 0.1f, 0.3f);
		targets[3] = RenderPassTargetDesc(m_DepthView, 1.0f, 0);
	}
	else
	{
		targets[0] = RenderPassTargetDesc(m_SwapChain->GetCurrentTextureView(), 0.1f, 0.1f, 0.3f);
		targets[1] = RenderPassTargetDesc(m_DepthView, 1.0f, 0);
	}

	m_Context->NewCommandList();
	if (m_UseMSAA)
	{
		m_Context->BeginRenderPass(m_RenderPassMSAA, 4, targets);
	}
	else
	{
		m_Context->BeginRenderPass(m_RenderPass, 2, targets);
	}
	m_Context->SetGraphicsPipeline(m_UseMSAA ? m_PSOMSAA : m_PSO);
	m_Context->SetViewport({ 0.0f, 0.0f, (f32)width, (f32)height, 0.0f, 1.0f });
	m_Context->SetScissor({ 0, 0, i32(width), i32(height) });
	m_Context->SetConstantBuffer(m_UBView, 0, 0);
	m_Context->SetVertexBufferView(m_VB, sizeof(Vertex));
	m_Context->SetIndexBufferView(m_IB, IndexFormat::kU32);
	m_Context->DrawIndexed(36);
	OnRenderUI();
	m_Context->EndRenderPass();
	m_Context->Execute();

	m_SwapChain->Present(false);

	m_Context->FlushFrame();
}


void MSAARenderPass::OnRenderUI()
{
	m_Overlay.Render(m_UseMSAA);
}


void MSAARenderPass::OnShutdown()
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
	m_RenderPass.Reset();
	m_RenderPassMSAA.Reset();
	m_SRL.Reset();
}


void MSAARenderPass::CreateShaderResourceLayout()
{
	ShaderResourceLayoutCreateDesc layoutDesc;
	layoutDesc.AddResourceSet().AddConstantBuffer(0, ShaderStage::kVertex);

	m_Device->CreateShaderResourceLayout(layoutDesc, &m_SRL);
}


void MSAARenderPass::CreatePipeline()
{
	auto vs = CompileShader(g_pVSShader, g_VSSize, ShaderStage::kVertex);
	auto ps = CompileShader(g_pPSShader, g_PSSize, ShaderStage::kPixelOrFragment);
	GraphicsPipelineStateDesc pipelineDesc;
	pipelineDesc.m_Shaders[0] = vs;
	pipelineDesc.m_Shaders[1] = ps;
	pipelineDesc.m_DepthStencilDesc.m_DepthTestEnable = true;
	pipelineDesc.m_DepthStencilDesc.m_DepthWriteEnable = true;
	pipelineDesc.m_DepthStencilDesc.m_DepthOp = CompareOp::kLessEqual;
	pipelineDesc.m_pShaderResourceLayout = m_SRL;
	pipelineDesc.m_pRenderPass = m_RenderPass;
	pipelineDesc.AddVertexInput("POSITION", 0, Format::kRGB32_Float).AddVertexInput("COLOR", 0, Format::kRGBA32_Float);

	m_Device->CreateGraphicsPipeline(pipelineDesc, &m_PSO);

	pipelineDesc.m_SampleCount = 8;
	pipelineDesc.m_pRenderPass = m_RenderPassMSAA;
	m_Device->CreateGraphicsPipeline(pipelineDesc, &m_PSOMSAA);
}


void MSAARenderPass::CreateBuffers()
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


void MSAARenderPass::CreateRenderTargets()
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


void MSAARenderPass::CreateRenderPass()
{
	RenderPassDesc rpDesc;
	rpDesc.AddAttachment(m_SwapChain->GetDesc().m_Format, ResourceState::kPresent);
	rpDesc.AddAttachment(m_Depth->GetDesc().m_Format, ResourceState::kDepthStencilRead);
	rpDesc.AddSubpass()
		.AddColorAttachment(0, ResourceState::kRenderTarget).SetDepthStencilAttachment(1, ResourceState::kDepthStencilWrite);

	m_Device->CreateRenderPass(rpDesc, &m_RenderPass);
}


void MSAARenderPass::CreateRenderPassMSAA()
{
	RenderPassDesc rpDesc;
	rpDesc.AddAttachment(m_ColorMS->GetDesc().m_Format, ResourceState::kRenderTarget)
		.SetStoreOp(StoreOp::kDontCare).SetSampleCount(8);
	rpDesc.AddAttachment(m_DepthMS->GetDesc().m_Format, ResourceState::kDepthStencilWrite)
		.SetStoreOp(StoreOp::kDontCare).SetSampleCount(8);;
	rpDesc.AddAttachment(m_SwapChain->GetDesc().m_Format, ResourceState::kPresent);
	rpDesc.AddAttachment(m_Depth->GetDesc().m_Format, ResourceState::kDepthStencilRead);
	rpDesc.AddSubpass()
		.AddColorAttachment(0, ResourceState::kRenderTarget).SetDepthStencilAttachment(1, ResourceState::kDepthStencilWrite)
		.AddResolveAttachment(2, ResourceState::kRenderTarget, ResolveMode::kAverage).SetDepthStencilResolveAttachment(3, ResourceState::kDepthStencilWrite, ResolveMode::kMin);

	m_Device->CreateRenderPass(rpDesc, &m_RenderPassMSAA);
}


SAMPLE_MAIN(MSAARenderPass)