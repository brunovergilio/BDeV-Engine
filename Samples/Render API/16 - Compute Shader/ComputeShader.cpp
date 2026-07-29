#include "ComputeShader.h"
#include "Shaders.h"


void ComputeShader::OnInitialize()
{
	m_AppName = "Compute Shader";
	CreateShaderResourceLayout();
	CreateBuffers();
	CreatePipeline();
	CreateTextures();
}


void ComputeShader::OnUpdate()
{
	i32 width = m_pWindow->GetWidth();
	i32 height = m_pWindow->GetHeight();

	m_PC.res = Float2(width, height);
	if (m_Dt <= 0.1666f)
	{
		m_PC.time += m_Dt;
	}

	m_QueryBuffer->Invalidate();
	m_PSOStats = m_Query->GetPipelineStats(m_pQueryData + m_QuerySize * m_Context->GetCurrentFrameIndex());
}


void ComputeShader::OnUpdateUI()
{
	BeginDrawDefaultUI();
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
	EndDrawDefaultUI();
}


void ComputeShader::OnRender()
{
	auto width = m_pWindow->GetWidth();
	auto height = m_pWindow->GetHeight();
	auto currFrame = m_Context->GetCurrentFrameIndex();

	m_Context->NewCommandList();
	m_Context->BeginQuery(m_Query, currFrame);
	ResourceBarrierDesc barrier;
	barrier.m_pTexture = m_RWTex;
	barrier.m_SrcState = ResourceState::kPixelShaderResource;
	barrier.m_DstState = ResourceState::kRWResource;
	m_Context->ResourceBarrier(1, &barrier);
	DoCompute(width, height);

	barrier.m_SrcState = ResourceState::kRWResource;
	barrier.m_DstState = ResourceState::kPixelShaderResource;

	m_Context->ResourceBarrier(1, &barrier);

	RenderTargetDesc target;
	target.SetColorView(m_SwapChain->GetCurrentTextureView(), ResourceState::kCommon, ResourceState::kPresent).SetClearValues({ 0.1f, 0.1f, 0.3f });

	m_Context->SetRenderTarget(target);
	m_Context->SetGraphicsPipeline(m_PSO);
	m_Context->SetViewport({ 0.0f, 0.0f, (f32)width, (f32)height, 0.0f, 1.0f });
	m_Context->SetScissor(width, height);
	m_Context->SetTexture(m_RWTexView, 0, 0);
	m_Context->SetSampler(m_Sampler, 0, 1);
	m_Context->Draw(6);
	OnRenderUI();

	m_Context->EndQuery(m_Query, currFrame);
	m_Context->ResolveQueryData(m_Query, currFrame, 1, m_QueryBuffer, m_QuerySize * currFrame);
	m_Context->Execute();

	m_SwapChain->Present(false);

	m_Context->FlushFrame();
}


void ComputeShader::DoCompute(u32 w, u32 h)
{
	m_Context->SetComputePipeline(m_PSOCompute);
	m_Context->SetRWTexture(m_RWTexView, 0, 0);
	m_Context->SetShaderConstantsT<PC>(m_PC, 1, 0);
	m_Context->Dispatch(w >> 3, h >> 3);
}


void ComputeShader::OnShutdown()
{
	m_PSOCompute.Reset();
	m_SRLCompute.Reset();
	m_PSO.Reset();
	m_SRL.Reset();
	m_RWTexView.Reset();
	m_RWTex.Reset();
	m_Sampler.Reset();
	m_Query.Reset();
	m_QueryBuffer.Reset();
}


void ComputeShader::CreateShaderResourceLayout()
{
	{
		ShaderResourceLayoutCreateDesc layoutDesc;
		layoutDesc.AddResourceSet().AddRWTexture(0, ShaderStage::kCompute).AddConstant("PC"_sid, 1, sizeof(PC), ShaderStage::kCompute);
		m_Device->CreateShaderResourceLayout(layoutDesc, &m_SRLCompute);
	}

	{
		ShaderResourceLayoutCreateDesc layoutDesc;
		layoutDesc.AddResourceSet().AddTexture(0, ShaderStage::kPixelOrFragment).AddSampler(1, ShaderStage::kPixelOrFragment);
		m_Device->CreateShaderResourceLayout(layoutDesc, &m_SRL);
	}
}


void ComputeShader::CreatePipeline()
{
	{
		auto cs = CompileShader(g_pCSShader, g_CSSize, ShaderStage::kCompute);
		ComputePipelineStateDesc pipelineDesc;
		pipelineDesc.m_pShader = cs;
		pipelineDesc.m_pShaderResourceLayout = m_SRLCompute;

		m_Device->CreateComputePipeline(pipelineDesc, &m_PSOCompute);
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


void ComputeShader::CreateBuffers()
{
	QueryHeapDesc qhd(QueryType::kPipelineStatistics, m_Context->GetFrameCount());
	m_Device->CreateQueryHeap(qhd, &m_Query);
	m_QuerySize = m_Query->GetQuerySize();

	BufferDesc desc;
	desc.m_Size = m_QuerySize * m_Context->GetFrameCount();
	desc.m_MemoryType = MemoryType::kReadBack;
	desc.m_CreateFlags = BufferCreateFlags::kCreateMapped;
	m_Device->CreateBuffer(desc, &m_QueryBuffer);
	m_pQueryData = m_QueryBuffer->GetMappedDataAsT<u8>();
}


void ComputeShader::CreateTextures()
{
	auto w = m_pWindow->GetWidth();
	auto h = m_pWindow->GetHeight();

	TextureDesc desc;
	desc.m_Size = { w, h, 1 };
	desc.m_Format = BvRenderUtils::GetFormatInfo(m_SwapChain->GetDesc().m_Format).m_SRGBOrLinearVariant;
	desc.m_UsageFlags = TextureUsage::kUnorderedAccess | TextureUsage::kShaderResource;
	TextureInitData initData{};
	initData.m_pContext = m_Context;
	initData.m_ResourceState = ResourceState::kPixelShaderResource;
	m_Device->CreateTexture(desc, initData, &m_RWTex);

	TextureViewDesc viewDesc;
	viewDesc.m_Format = desc.m_Format;
	viewDesc.m_pTexture = m_RWTex;
	m_Device->CreateTextureView(viewDesc, &m_RWTexView);

	m_Device->CreateSampler(SamplerDesc(), &m_Sampler);
}


SAMPLE_MAIN(ComputeShader)