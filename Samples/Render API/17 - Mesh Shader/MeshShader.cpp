#include "MeshShader.h"
#include "BDeV/Core/Math/BvGeometryGenerator.h"
#include "BDeV/Core/Utils/BvRandom.h"
#include "Shaders.h"


constexpr u32 kCubeCount = 64;


void MeshShader::OnInitialize()
{
	m_AppName = "Mesh Shader";
	m_MeshQueriesSupported = EHasFlag(m_Device->GetGPUInfo().m_DeviceCaps, RenderDeviceCapabilities::kMeshQuery);

	CreateRenderTargets();
	CreateShaderResourceLayout();
	CreatePipeline();
	CreateBuffers();

	m_Camera.SetPos(0.0f, 0.0f, -10.0f);
}


void MeshShader::OnInitializeUI()
{
	m_Overlay.SetupPipeline(m_SwapChain->GetDesc().m_Format, m_DepthView->GetDesc().m_Format);
}


void MeshShader::OnUpdate()
{
	auto width = m_pWindow->GetWidth();
	auto height = m_pWindow->GetHeight();
	XMStoreFloat4x4(&m_PC.viewProj, m_Camera.GetViewProj());
	XMStoreFloat3(&m_PC.cameraPos, m_Camera.GetPos());

	if (m_MeshQueriesSupported)
	{
		m_QueryBuffer->Invalidate();
		m_PSOStats = m_Query->GetMeshPipelineStats(m_pQueryData + m_QuerySize * m_Context->GetCurrentFrameIndex());
	}
}


void MeshShader::OnUpdateUI()
{
	BeginDrawDefaultUI();
	if (m_MeshQueriesSupported)
	{
		ImGui::Text("Input Assembly Vertices: %llu", m_PSOStats.m_InputAssemblyVertices);
		ImGui::Text("Input Assembly Primitives: %llu", m_PSOStats.m_InputAssemblyPrimitives);
		ImGui::Text("Vertex Shader Invocations: %llu", m_PSOStats.m_VertexShaderInvocations);
		ImGui::Text("Geometry Shader Invocations: %llu", m_PSOStats.m_GeometryShaderInvocations);
		ImGui::Text("Geometry Shader Primitives: %llu", m_PSOStats.m_GeometryShaderPrimitives);
		ImGui::Text("Clipping Invocations: %llu", m_PSOStats.m_ClippingInvocations);
		ImGui::Text("Clipping Primitives: %llu", m_PSOStats.m_ClippingPrimitives);
		ImGui::Text("Pixel Shader Invocations: %llu", m_PSOStats.m_PixelOrFragmentShaderInvocations);
		ImGui::Text("Hull Shader Invocations: %llu", m_PSOStats.m_HullOrControlShaderInvocations);
		ImGui::Text("Domain Shader Invocations: %llu", m_PSOStats.m_DomainOrEvaluationShaderInvocations);
		ImGui::Text("Compute Shader Invocations: %llu", m_PSOStats.m_ComputeShaderInvocations);
		ImGui::Text("Task Shader Invocations: %llu", m_PSOStats.m_TaskOrAmplificationShaderInvocations);
		ImGui::Text("Mesh Shader Invocations: %llu", m_PSOStats.m_MeshShaderInvocations);
		ImGui::Text("Mesh Shader Primitives: %llu", m_PSOStats.m_MeshShaderPrimitives);
	}
	EndDrawDefaultUI();
}


void MeshShader::OnRender()
{
	auto width = m_pWindow->GetWidth();
	auto height = m_pWindow->GetHeight();
	auto currFrame = m_Context->GetCurrentFrameIndex();

	RenderTargetDesc targets[2];
	targets[0].SetColorView(m_SwapChain->GetCurrentTextureView(), ResourceState::kCommon, ResourceState::kPresent).SetClearValues({ 0.1f, 0.1f, 0.3f });
	targets[1].SetDepthStencilView(m_DepthView, ResourceState::kCommon, ResourceState::kDepthStencilRead).SetClearValues({ 1.0f, 0 });

	m_Context->NewCommandList();
	if (m_MeshQueriesSupported)
	{
		m_Context->ResetQueryHeap(m_Query, currFrame, 1);
		m_Context->BeginQuery(m_Query, currFrame);
	}
	m_Context->SetRenderTargets(2, targets);
	m_Context->SetGraphicsPipeline(m_PSO);
	m_Context->SetViewport({ 0.0f, 0.0f, (f32)width, (f32)height, 0.0f, 1.0f });
	m_Context->SetScissor(width, height);
	m_Context->SetStructuredBuffer(m_SBView1, 0, 0);
	m_Context->SetRWStructuredBuffer(m_SBView2, 0, 1);
	m_Context->SetShaderConstantsT<PushConstants>(m_PC, 2, 0);
	m_Context->DispatchMesh(1);
	OnRenderUI();
	if (m_MeshQueriesSupported)
	{
		m_Context->EndQuery(m_Query, currFrame);
		m_Context->ResolveQueryData(m_Query, currFrame, 1, m_QueryBuffer, m_QuerySize * currFrame);
	}
	m_Context->Execute();

	m_SwapChain->Present(false);

	m_Context->FlushFrame();
}


void MeshShader::OnShutdown()
{
	m_SB1.Reset();
	m_SB2.Reset();
	m_SBView1.Reset();
	m_SBView2.Reset();
	m_Depth.Reset();
	m_DepthView.Reset();
	m_Query.Reset();
	m_QueryBuffer.Reset();
	m_PSO.Reset();
	m_SRL.Reset();
}


void MeshShader::CreateShaderResourceLayout()
{
	ShaderResourceLayoutCreateDesc layoutDesc;
	layoutDesc.AddResourceSet()
		.AddStructuredBuffer(0, ShaderStage::kAmplificationOrTask | ShaderStage::kMesh)
		.AddRWStructuredBuffer(1, ShaderStage::kAmplificationOrTask | ShaderStage::kMesh)
		.AddConstant<PushConstants>("PC"_sid, 2, ShaderStage::kAmplificationOrTask | ShaderStage::kMesh);
	m_Device->CreateShaderResourceLayout(layoutDesc, &m_SRL);
}


void MeshShader::CreatePipeline()
{
	auto as = CompileShader(g_pASShader, g_ASSize, ShaderStage::kAmplificationOrTask);
	auto ms = CompileShader(g_pMSShader, g_MSSize, ShaderStage::kMesh);
	auto ps = CompileShader(g_pPSShader, g_PSSize, ShaderStage::kPixelOrFragment);
	GraphicsPipelineStateDesc pipelineDesc;
	pipelineDesc.m_Shaders[0] = as;
	pipelineDesc.m_Shaders[1] = ms;
	pipelineDesc.m_Shaders[2] = ps;
	pipelineDesc.m_RenderTargetFormats[0] = m_SwapChain->GetDesc().m_Format;
	pipelineDesc.m_DepthStencilFormat = m_DepthView->GetDesc().m_Format;
	pipelineDesc.m_DepthStencilDesc.m_DepthTestEnable = true;
	pipelineDesc.m_DepthStencilDesc.m_DepthWriteEnable = true;
	pipelineDesc.m_DepthStencilDesc.m_DepthOp = CompareOp::kLessEqual;
	pipelineDesc.m_pShaderResourceLayout = m_SRL;
	pipelineDesc.m_RasterizerStateDesc.m_CullMode = CullMode::kBack;

	m_Device->CreateGraphicsPipeline(pipelineDesc, &m_PSO);
}


void MeshShader::CreateRenderTargets()
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


void MeshShader::CreateBuffers()
{
	BufferDesc desc;
	BufferInitData data;
	BufferViewDesc viewDesc;

	BvRandom32 rand;
	BvVector<Float4> cubes(kCubeCount);
	constexpr f32 kSpacing = 2.0f;
	constexpr f32 kBaseScale = 1.0f;
	constexpr i32 kCubesPerLine = 8;
	f32 totalSpacePerLine = kCubesPerLine * kBaseScale + kSpacing * (kCubesPerLine - 1);
	f32 negHalfSpace = -0.5f * totalSpacePerLine;
	for (auto w = 0; w < kCubesPerLine; ++w)
	{
		for (auto d = 0; d < kCubesPerLine; ++d)
		{
			f32 x = negHalfSpace + kBaseScale * 0.5f + w * (kSpacing + kBaseScale);
			f32 z = negHalfSpace + kBaseScale * 0.5f + d * (kSpacing + kBaseScale);
			auto& cube = cubes[d + kCubesPerLine * w];
			cube = Float4(x, 0.0f, z, rand.NextF(0.8f, 1.5f));
		}
	}

	desc.m_Size = sizeof(Float4) * kCubeCount;
	desc.m_UsageFlags = BufferUsage::kStructuredBuffer;
	data.m_pContext = m_Context;
	data.m_pData = cubes.Data();
	data.m_Size = desc.m_Size;
	data.m_ResourceState = ResourceState::kShaderResource;
	m_Device->CreateBuffer(desc, data, &m_SB1);

	viewDesc.m_pBuffer = m_SB1;
	viewDesc.m_Stride = sizeof(Float4);
	viewDesc.m_ElementCount = kCubeCount;
	m_Device->CreateBufferView(viewDesc, &m_SBView1);

	desc.m_Size = sizeof(u32) * kCubeCount;
	desc.m_UsageFlags = BufferUsage::kRWStructuredBuffer;
	data.m_pData = nullptr;
	data.m_Size = 0;
	data.m_ResourceState = ResourceState::kRWResource;
	m_Device->CreateBuffer(desc, data, &m_SB2);

	viewDesc.m_pBuffer = m_SB2;
	viewDesc.m_Stride = sizeof(u32);
	m_Device->CreateBufferView(viewDesc, &m_SBView2);

	if (m_MeshQueriesSupported)
	{
		QueryHeapDesc qhd(QueryType::kMeshPipelineStatistics, m_Context->GetFrameCount());
		m_Device->CreateQueryHeap(qhd, &m_Query);
		m_QuerySize = m_Query->GetQuerySize();

		BufferDesc queryBufferDesc;
		queryBufferDesc.m_Size = m_QuerySize * m_Context->GetFrameCount();
		queryBufferDesc.m_MemoryType = MemoryType::kReadBack;
		queryBufferDesc.m_CreateFlags = BufferCreateFlags::kCreateMapped;
		m_Device->CreateBuffer(queryBufferDesc, &m_QueryBuffer);
		m_pQueryData = m_QueryBuffer->GetMappedDataAsT<u8>();
	}
}


SAMPLE_MAIN(MeshShader)