#include "DynamicBuffers.h"
#include "BDeV/Core/Math/BvGeometryGenerator.h"
#include "BDeV/Core/Utils/BvRandom.h"
#include "Shaders.h"


struct Vertex
{
	XMFLOAT3 pos;
	XMFLOAT4 color;
};


void DynamicBuffers::OnInitialize()
{
	m_AppName = "Dynamic Buffers";
	CreateBuffers();
	CreateRenderTargets();
	CreateShaderResourceLayout();
	CreatePipeline();
}


void DynamicBuffers::OnInitializeUI()
{
	m_Overlay.SetupPipeline(m_SwapChain->GetDesc().m_Format, m_DepthView->GetDesc().m_Format);
}


void DynamicBuffers::OnUpdate()
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

	auto pData = m_UB->GetMappedDataAsT<u8>();
	BvSVec offset(3.0f);
	for (auto x = 0; x < m_ItemCountPerDimension; ++x)
	{
		for (auto y = 0; y < m_ItemCountPerDimension; ++y)
		{
			for (auto z = 0; z < m_ItemCountPerDimension; ++z)
			{
				auto index = x * m_ItemCountPerDimension * m_ItemCountPerDimension + y * m_ItemCountPerDimension + z;
				auto pWVP = reinterpret_cast<XMFLOAT4X4*>(pData + (index * m_ItemStride));
				BvVec3 pos((x), (y), (z));
				pos *= offset;
				pos += BvSVec((-(f32(m_ItemCountPerDimension) * offset) * 0.5f) + offset * 0.5f);

				XMStoreFloat4x4(pWVP, XMMatrixRotationX(angleX) * XMMatrixRotationY(angleY) * XMMatrixRotationZ(angleZ) * XMMatrixTranslationFromVector(pos)
					* m_Camera.GetViewProj());
			}
		}
	}
}


void DynamicBuffers::OnUpdateUI()
{
	BeginDrawDefaultUI();
	ImGui::SliderInt("Items", &m_ItemCountPerDimension, 2, m_MaxItemsPerDimension);
	EndDrawDefaultUI();
}


void DynamicBuffers::OnRender()
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
	m_Context->SetVertexBufferView(m_VB, sizeof(Vertex));
	m_Context->SetIndexBufferView(m_IB, IndexFormat::kU32);
	auto numItems = m_ItemCountPerDimension * m_ItemCountPerDimension * m_ItemCountPerDimension;
	for (auto i = 0; i < numItems; ++i)
	{
		m_Context->SetDynamicConstantBuffer(m_UBView, u32(m_ItemStride) * i, 0, 0);
		m_Context->DrawIndexed(36);
	}
	OnRenderUI();
	m_Context->Execute();

	m_SwapChain->Present(false);

	m_Context->FlushFrame();
}


void DynamicBuffers::OnShutdown()
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


void DynamicBuffers::CreateShaderResourceLayout()
{
	ShaderResourceLayoutCreateDesc layoutDesc{};
	layoutDesc.AddResourceSet().SetType(ShaderResourceSetDesc::Type::kDynamic).AddDynamicConstantBuffer(0, ShaderStage::kVertex);

	m_Device->CreateShaderResourceLayout(layoutDesc, &m_SRL);
}


void DynamicBuffers::CreatePipeline()
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

	pipelineDesc.AddVertexInput("POSITION", 0, Format::kRGB32_Float)
		.AddVertexInput("COLOR", 0, Format::kRGBA32_Float);

	m_Device->CreateGraphicsPipeline(pipelineDesc, &m_PSO);
}


void DynamicBuffers::CreateBuffers()
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

	m_ItemStride = m_Device->GetDynamicBufferElementSize(BufferUsage::kConstantBuffer, sizeof(XMFLOAT4X4));
	bufferDesc.m_Size = m_ItemStride * size_t(std::pow(m_MaxItemsPerDimension, 3));
	bufferDesc.m_UsageFlags = BufferUsage::kConstantBuffer;
	bufferDesc.m_MemoryType = MemoryType::kUpload;
	bufferDesc.m_CreateFlags = BufferCreateFlags::kCreateMapped;
	m_Device->CreateBuffer(bufferDesc, &m_UB);

	viewDesc.m_pBuffer = m_UB;
	viewDesc.m_Stride = sizeof(XMFLOAT4X4);
	viewDesc.m_ElementCount = 1;
	m_Device->CreateBufferView(viewDesc, &m_UBView);
}


void DynamicBuffers::CreateRenderTargets()
{
	auto w = m_pWindow->GetWidth();
	auto h = m_pWindow->GetHeight();

	TextureDesc desc;
	desc.m_Size = { w, h, 1 };
	desc.m_Format = Format::kD24_UNorm_S8_UInt;
	desc.m_UsageFlags = TextureUsage::kDepthStencilTarget;
	m_Device->CreateTexture(desc, nullptr, &m_Depth);

	TextureViewDesc viewDesc;
	viewDesc.m_Format = desc.m_Format;
	viewDesc.m_pTexture = m_Depth;
	m_Device->CreateTextureView(viewDesc, &m_DepthView);
}


SAMPLE_MAIN(DynamicBuffers)