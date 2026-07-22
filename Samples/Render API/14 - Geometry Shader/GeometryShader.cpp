#include "GeometryShader.h"
#include "BDeV/Core/Math/BvGeometryGenerator.h"
#include "BDeV/Core/Utils/BvRandom.h"
#include "Shaders.h"


struct Vertex
{
	Float3 pos;
	Float4 color;
	Float3 normal;
};


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
	auto w = BvMatrix::RotationX(angleX) * BvMatrix::RotationY(angleY) * BvMatrix::RotationZ(angleZ);
	auto vp = BvMatrix::LookAtLH(BvVec3(0.0f, 0.0f, -5.0f), BvVec3(0.0f, 0.0f, 1.0f), BvVec3(0.0f, 1.0f, 0.0f))
		* BvMatrix::PerspectiveLH_DX(0.1f, 100.0f, float(width) / float(height), kPiDiv4);
	XMStoreFloat4x4(m_pWVP, w * vp);
	XMStoreFloat4x4(&m_pNormalData[0], w);
	XMStoreFloat4x4(&m_pNormalData[1], vp);
}


void GeometryShader::OnRender()
{
	auto width = m_pWindow->GetWidth();
	auto height = m_pWindow->GetHeight();
	RenderTargetDesc targets[2];
	targets[0].SetColorView(m_SwapChain->GetCurrentTextureView(), ResourceState::kCommon, ResourceState::kPresent).SetClearValues({ 0.1f, 0.1f, 0.3f });
	targets[1].SetDepthStencilView(m_DepthView, ResourceState::kCommon, ResourceState::kDepthStencilRead).SetClearValues({ 1.0f, 0 });

	m_Context->NewCommandList();
	m_Context->SetRenderTargets(2, targets);
	m_Context->SetGraphicsPipeline(m_PSO);
	m_Context->SetViewport({ 0.0f, 0.0f, (f32)width, (f32)height, 0.0f, 1.0f });
	m_Context->SetScissor(width, height);
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

	m_Depth.Reset();
	m_DepthView.Reset();
	m_PSONormal.Reset();
	m_SRLNormal.Reset();
	m_UBNormal.Reset();
	m_UBViewNormal.Reset();
}


void GeometryShader::CreateShaderResourceLayout()
{
	{
		ShaderResourceLayoutCreateDesc layoutDesc;
		layoutDesc.AddResourceSet(0).AddConstantBuffer(0, ShaderStage::kVertex);

		m_Device->CreateShaderResourceLayout(layoutDesc, &m_SRL);
	}

	{
		ShaderResourceLayoutCreateDesc layoutDesc;
		layoutDesc.AddResourceSet(0).AddConstantBuffer(0, ShaderStage::kGeometry);

		m_Device->CreateShaderResourceLayout(layoutDesc, &m_SRLNormal);
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
	pipelineDesc.AddVertexInput("POSITION", 0, Format::kRGB32_Float).AddVertexInput("COLOR", 0, Format::kRGBA32_Float);

	m_Device->CreateGraphicsPipeline(pipelineDesc, &m_PSO);

	vs = CompileShader(g_pVSDebugShader, g_VSDebugSize, ShaderStage::kVertex);
	auto gs = CompileShader(g_pGSDebugShader, g_GSDebugSize, ShaderStage::kGeometry);
	pipelineDesc.m_Shaders[0] = vs;
	pipelineDesc.m_Shaders[2] = gs;
	pipelineDesc.m_pShaderResourceLayout = m_SRLNormal;
	pipelineDesc.m_VertexInputDescs.Clear();
	pipelineDesc.AddVertexInput("POSITION", 0, Format::kRGB32_Float).AddVertexInput("NORMAL", 0, Format::kRGB32_Float, 0, InputRate::kPerVertex, offsetof(Vertex, normal));

	m_Device->CreateGraphicsPipeline(pipelineDesc, &m_PSONormal);
}


void GeometryShader::CreateBuffers()
{
	BvGeometryGenerator gen;
	gen.GenerateBox();
	auto& data = gen.GetData();
	BvVector<Vertex> vertices(data.m_Vertices.Size());
	BvRandom32 rand;

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
	m_pWVP = m_UB->GetMappedDataAsT<Float44>();

	viewDesc.m_pBuffer = m_UB;
	viewDesc.m_Stride = sizeof(Float44);
	viewDesc.m_ElementCount = 1;
	m_Device->CreateBufferView(viewDesc, &m_UBView);

	bufferDesc.m_Size = sizeof(Float44) * 2;
	m_Device->CreateBuffer(bufferDesc, &m_UBNormal);
	m_pNormalData = m_UBNormal->GetMappedDataAsT<Float44>();
	viewDesc.m_pBuffer = m_UBNormal;
	viewDesc.m_Stride = sizeof(Float44) * 2;
	m_Device->CreateBufferView(viewDesc, &m_UBViewNormal);
}


void GeometryShader::CreateRenderTargets()
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


SAMPLE_MAIN(GeometryShader)