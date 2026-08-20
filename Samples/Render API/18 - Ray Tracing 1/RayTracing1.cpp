#include "RayTracing1.h"
#include "Shaders.h"


void RayTracing1::OnInitialize()
{
	m_AppName = "Ray Tracing 1";
	CreateShaderResourceLayout();
	CreatePipeline();
	CreateResources();
}


void RayTracing1::OnUpdate()
{
}


void RayTracing1::OnUpdateUI()
{
	BeginDrawDefaultUI();
	ImGui::ColorEdit3("Background", &m_BackColor.x);
	EndDrawDefaultUI();
}


void RayTracing1::OnRender()
{
	auto width = m_pWindow->GetWidth();
	auto height = m_pWindow->GetHeight();

	ResourceBarrierDesc barrier;
	barrier.m_pTexture = m_Tex;
	barrier.m_SrcState = ResourceState::kPixelShaderResource;
	barrier.m_DstState = ResourceState::kRWResource;

	m_Context->NewCommandList();
	m_Context->ResourceBarrier(1, &barrier);
	m_Context->SetRayTracingPipeline(m_RayPSO);
	m_Context->SetRWTexture(m_TexView, 0, 0);
	m_Context->SetAccelerationStructure(m_AS, 0, 1);
	m_Context->SetShaderConstantsT<Float3>(m_BackColor, 2, 0);
	m_Context->DispatchRays(m_SBT, 0, 0, 0, 0, width, height, 1);

	barrier.m_SrcState = ResourceState::kRWResource;
	barrier.m_DstState = ResourceState::kPixelShaderResource;
	m_Context->ResourceBarrier(1, &barrier);

	RenderTargetDesc target;
	target.SetColorView(m_SwapChain->GetCurrentTextureView(), ResourceState::kCommon, ResourceState::kPresent).SetClearValues({ 0.1f, 0.1f, 0.3f });

	m_Context->SetRenderTarget(target);
	m_Context->SetGraphicsPipeline(m_PSO);
	m_Context->SetViewport({ 0.0f, 0.0f, (f32)width, (f32)height, 0.0f, 1.0f });
	m_Context->SetScissor(width, height);
	m_Context->SetTexture(m_TexView, 0, 0);
	m_Context->SetSampler(m_Sampler, 0, 1);
	m_Context->Draw(6);
	OnRenderUI();

	m_Context->Execute();

	m_SwapChain->Present(false);

	m_Context->FlushFrame();
}


void RayTracing1::OnShutdown()
{
	m_RayPSO.Reset();
	m_RaySRL.Reset();
	m_PSO.Reset();
	m_SRL.Reset();
	m_Tex.Reset();
	m_TexView.Reset();
	m_Sampler.Reset();
	m_AS.Reset();
	m_SBT.Reset();
	m_ScratchBuffer.Reset();
	m_StagingBuffer.Reset();
}


void RayTracing1::CreateShaderResourceLayout()
{
	{
		ShaderResourceLayoutCreateDesc layoutDesc;
		layoutDesc.AddResourceSet()
			.AddRWTexture(0, ShaderStage::kRayGen)
			.AddAccelerationStructure(1, ShaderStage::kRayGen)
			.AddConstant<Float3>("PC"_sid, 2, ShaderStage::kMiss);

		m_Device->CreateShaderResourceLayout(layoutDesc, &m_RaySRL);
	}

	{
		ShaderResourceLayoutCreateDesc layoutDesc;
		layoutDesc.AddResourceSet()
			.AddTexture(0, ShaderStage::kPixelOrFragment).AddSampler(1, ShaderStage::kPixelOrFragment);

		m_Device->CreateShaderResourceLayout(layoutDesc, &m_SRL);
	}
}


void RayTracing1::CreatePipeline()
{
	{
		m_RGen = CompileShader(g_pRGenShader, g_RGenSize, ShaderStage::kRayGen);
		m_Miss = CompileShader(g_pRMissShader, g_RMissSize, ShaderStage::kMiss);

		ShaderGroupDesc groupDescs[2];
		groupDescs[0].m_Type = ShaderGroupType::kGeneral;
		groupDescs[0].m_General = 0;

		groupDescs[1].m_Type = ShaderGroupType::kGeneral;
		groupDescs[1].m_General = 1;

		RayTracingPipelineStateDesc pipelineDesc;
		pipelineDesc.AddShader(m_RGen).AddShader(m_Miss).AddGeneralShaderGroup("", 0).AddGeneralShaderGroup("", 1);
		pipelineDesc.m_pShaderResourceLayout = m_RaySRL;
		pipelineDesc.m_MaxPipelineRayRecursionDepth = 1;
		pipelineDesc.m_MaxPayloadSize = sizeof(Float4);

		m_Device->CreateRayTracingPipeline(pipelineDesc, &m_RayPSO);
	}

	{
		m_VS = CompileShader(g_pVSShader, g_VSSize, ShaderStage::kVertex);
		m_PS = CompileShader(g_pPSShader, g_PSSize, ShaderStage::kPixelOrFragment);
		GraphicsPipelineStateDesc pipelineDesc;
		pipelineDesc.m_Shaders[0] = m_VS;
		pipelineDesc.m_Shaders[1] = m_PS;
		pipelineDesc.m_RenderTargetFormats[0] = m_SwapChain->GetDesc().m_Format;
		pipelineDesc.m_pShaderResourceLayout = m_SRL;

		m_Device->CreateGraphicsPipeline(pipelineDesc, &m_PSO);
	}
}


void RayTracing1::CreateResources()
{
	TextureDesc desc;
	desc.m_Size = { m_pWindow->GetWidth(), m_pWindow->GetHeight(), 1 };
	desc.m_Format = Format::kRGBA32_Float;
	desc.m_UsageFlags = TextureUsage::kUnorderedAccess | TextureUsage::kShaderResource;
	TextureInitData initData{};
	initData.m_pContext = m_Context;
	initData.m_ResourceState = ResourceState::kPixelShaderResource;
	m_Device->CreateTexture(desc, initData, &m_Tex);

	TextureViewDesc viewDesc;
	viewDesc.m_Format = desc.m_Format;
	viewDesc.m_pTexture = m_Tex;
	m_Device->CreateTextureView(viewDesc, &m_TexView);

	m_Device->CreateSampler(SamplerDesc(), &m_Sampler);

	RayTracingAccelerationStructureDesc asDesc;
	asDesc.m_Type = RayTracingAccelerationStructureType::kTopLevel;
	asDesc.m_Flags = RayTracingAccelerationStructureFlags::kPreferFastTrace;
	asDesc.m_Geometries.EmplaceBack().m_Instance.m_InstanceCount = 0;
	m_Device->CreateAccelerationStructure(asDesc, &m_AS);
	
	BufferDesc buffDesc;
	buffDesc.m_Size = m_AS->GetBuildSizes().m_Build;
	buffDesc.m_UsageFlags = BufferUsage::kRayTracing;
	m_Device->CreateBuffer(buffDesc, &m_ScratchBuffer);

	buffDesc.m_UsageFlags = BufferUsage::kRayTracing;
	buffDesc.m_Size = sizeof(RayTracingAccelerationStructureInstanceDesc);
	buffDesc.m_MemoryType = MemoryType::kUpload;
	m_Device->CreateBuffer(buffDesc, &m_StagingBuffer);

	RayTracingAccelerationStructureInstanceDesc dummyInstance;
	m_AS->WriteTopLevelInstances(m_StagingBuffer, 1, &dummyInstance);
	
	RayTracingAccelerationStructureBuildDesc build;
	auto& instance = build.m_Geometries.EmplaceBack();
	instance.m_Instance.m_Count = 0;
	instance.m_Instance.m_pBuffer = m_StagingBuffer;
	build.m_Type = RayTracingAccelerationStructureType::kTopLevel;
	build.m_pAS = m_AS;
	build.m_pScratchBuffer = m_ScratchBuffer;

	m_Context->NewCommandList();
	m_Context->BuildRayTracingAccelerationStructure(build);
	m_Context->ExecuteAndWait();

	ShaderBindingTableDesc sbtDesc;
	sbtDesc.m_pPSO = m_RayPSO;
	m_Device->CreateShaderBindingTable(sbtDesc, m_Context, &m_SBT);
}


SAMPLE_MAIN(RayTracing1)