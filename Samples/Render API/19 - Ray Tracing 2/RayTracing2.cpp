#include "RayTracing2.h"
#include "Shaders.h"


void RayTracing2::OnInitialize()
{
	m_AppName = "Ray Tracing 2";
	CreateShaderResourceLayout();
	CreatePipeline();
	CreateResources();
	CreateBLAS();
	CreateTLAS();
	
	m_Camera.SetPos(0.0f, 0.0f, -2.0f);
	m_Camera.SetFlipViewportY(true);
}


void RayTracing2::OnUpdate()
{
	XMStoreFloat4x4(&m_pRayData->viewInv, m_Camera.GetViewInv());
	XMStoreFloat4x4(&m_pRayData->projInv, m_Camera.GetProjInv());
}


void RayTracing2::OnUpdateUI()
{
	BeginDrawDefaultUI();
	ImGui::ColorEdit3("Background", &m_BackColor.x);
	EndDrawDefaultUI();
}


void RayTracing2::OnRender()
{
	auto width = m_pWindow->GetWidth();
	auto height = m_pWindow->GetHeight();

	m_Context->NewCommandList();

	ResourceBarrierDesc barrier;
	barrier.m_pTexture = m_Tex;
	barrier.m_SrcState = ResourceState::kPixelShaderResource;
	barrier.m_DstState = ResourceState::kRWResource;

	m_Context->ResourceBarrier(1, &barrier);
	m_Context->SetRayTracingPipeline(m_RayPSO);
	m_Context->SetRWTexture(m_TexView, 0, 0);
	m_Context->SetAccelerationStructure(m_TLAS, 0, 1);
	m_Context->SetConstantBuffer(m_UBViewRayData, 0, 2);
	m_Context->SetShaderConstantsT<Float3>(m_BackColor, 3, 0);
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


void RayTracing2::OnShutdown()
{
	m_RayPSO.Reset();
	m_RaySRL.Reset();
	m_PSO.Reset();
	m_SRL.Reset();
	m_Tex.Reset();
	m_TexView.Reset();
	m_Sampler.Reset();
	m_TLAS.Reset();
	m_BLAS.Reset();
	m_SBT.Reset();
	m_VBTri.Reset();
	m_IBTri.Reset();
	m_UBRayData.Reset();
	m_UBViewRayData.Reset();
}


void RayTracing2::CreateShaderResourceLayout()
{
	{
		ShaderResourceLayoutCreateDesc layoutDesc;
		layoutDesc.AddResourceSet()
			.AddRWTexture(0, ShaderStage::kRayGen)
			.AddAccelerationStructure(1, ShaderStage::kRayGen)
			.AddConstantBuffer(2, ShaderStage::kRayGen)
			.AddConstant<Float3>("PC"_sid, 3, ShaderStage::kMiss);

		m_Device->CreateShaderResourceLayout(layoutDesc, &m_RaySRL);
	}

	{
		ShaderResourceLayoutCreateDesc layoutDesc;
		layoutDesc.AddResourceSet()
			.AddTexture(0, ShaderStage::kPixelOrFragment)
			.AddSampler(1, ShaderStage::kPixelOrFragment);

		m_Device->CreateShaderResourceLayout(layoutDesc, &m_SRL);
	}
}


void RayTracing2::CreatePipeline()
{
	{
		m_RGen = CompileShader(g_pRGenShader, g_RGenSize, ShaderStage::kRayGen);
		m_Miss = CompileShader(g_pRMissShader, g_RMissSize, ShaderStage::kMiss);
		m_CHit = CompileShader(g_pRCHitShader, g_RCHitSize, ShaderStage::kClosestHit);

		RayTracingPipelineStateDesc pipelineDesc;
		pipelineDesc.AddShader(m_RGen).AddShader(m_Miss).AddShader(m_CHit)
			.AddGeneralShaderGroup("", 0).AddGeneralShaderGroup("", 1).AddTriangleShaderGroup("", 2, ShaderGroupDesc::kUnusedShader);
		pipelineDesc.m_pShaderResourceLayout = m_RaySRL;
		pipelineDesc.m_MaxPipelineRayRecursionDepth = 1;
		pipelineDesc.m_MaxPayloadSize = 16;
		pipelineDesc.m_MaxAttributeSize = 8;
		m_Device->CreateRayTracingPipeline(pipelineDesc, &m_RayPSO);

		ShaderBindingTableDesc sbtDesc;
		sbtDesc.m_pPSO = m_RayPSO;
		m_Device->CreateShaderBindingTable(sbtDesc, m_Context, &m_SBT);
	}

	{
		m_VS = CompileShader(g_pVSShader, g_VSSize, ShaderStage::kVertex);
		m_PS = CompileShader(g_pPSShader, g_PSSize, ShaderStage::kPixelOrFragment);
		GraphicsPipelineStateDesc pipelineDesc;
		pipelineDesc.AddShader(m_VS).AddShader(m_PS);
		pipelineDesc.AddRenderTargetFormat(m_SwapChain->GetDesc().m_Format);
		pipelineDesc.m_pShaderResourceLayout = m_SRL;

		m_Device->CreateGraphicsPipeline(pipelineDesc, &m_PSO);
	}
}


void RayTracing2::CreateResources()
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

	BufferInitData bufferInitData;
	bufferInitData.m_pContext = m_Context;
	BufferDesc bufferDesc;

	Float3 vertices[3] =
	{
		{ 0.5f, -0.5f,  1.0f},
		{-0.5f, -0.5f,  1.0f},
		{ 0.0f,  0.5f,  1.0f},
	};
	bufferDesc.m_Size = sizeof(vertices);
	bufferDesc.m_UsageFlags = BufferUsage::kRayTracing;
	bufferInitData.m_pData = vertices;
	bufferInitData.m_Size = bufferDesc.m_Size;
	m_Device->CreateBuffer(bufferDesc, bufferInitData, &m_VBTri);

	u32 indices[3] = { 0, 1, 2 };
	bufferDesc.m_Size = sizeof(indices);
	bufferDesc.m_UsageFlags = BufferUsage::kRayTracing;
	bufferInitData.m_pData = indices;
	bufferInitData.m_Size = bufferDesc.m_Size;
	m_Device->CreateBuffer(bufferDesc, bufferInitData, &m_IBTri);

	bufferDesc.m_Size = sizeof(RayData);
	bufferDesc.m_UsageFlags = BufferUsage::kConstantBuffer;
	bufferDesc.m_MemoryType = MemoryType::kUpload;
	bufferDesc.m_CreateFlags = BufferCreateFlags::kCreateMapped;
	m_Device->CreateBuffer(bufferDesc, &m_UBRayData);
	m_pRayData = m_UBRayData->GetMappedDataAsT<RayData>();

	BufferViewDesc bufferViewDesc;
	bufferViewDesc.m_ElementCount = 1;
	bufferViewDesc.m_Stride = bufferDesc.m_Size;
	bufferViewDesc.m_pBuffer = m_UBRayData;
	m_Device->CreateBufferView(bufferViewDesc, &m_UBViewRayData);
}


void RayTracing2::CreateBLAS()
{
	RayTracingAccelerationStructureDesc blasDesc;
	blasDesc.m_Type = RayTracingAccelerationStructureType::kBottomLevel;
	blasDesc.m_Flags = RayTracingAccelerationStructureFlags::kPreferFastTrace;
	auto& geomDesc = blasDesc.m_Geometries.EmplaceBack();
	geomDesc.m_Type = RayTracingGeometryType::kTriangles;
	geomDesc.m_Flags = RayTracingGeometryFlags::kOpaque;
	geomDesc.m_Id = "Triangle"_sid;
	geomDesc.m_Triangle.m_VertexCount = 3;
	geomDesc.m_Triangle.m_VertexFormat = Format::kRGB32_Float;
	geomDesc.m_Triangle.m_VertexStride = sizeof(Float3);
	geomDesc.m_Triangle.m_IndexCount = 3;
	geomDesc.m_Triangle.m_IndexFormat = IndexFormat::kU32;
	m_Device->CreateAccelerationStructure(blasDesc, &m_BLAS);

	BufferDesc bufferDesc;
	bufferDesc.m_Size = RoundToNearestPowerOf2(m_BLAS->GetBuildSizes().m_Build, m_Device->GetBufferOffsetAlignment(BufferUsage::kRayTracing));
	bufferDesc.m_UsageFlags = BufferUsage::kRayTracing;
	BvRCRef<IBvBuffer> scratchAS;
	m_Device->CreateBuffer(bufferDesc, &scratchAS);

	RayTracingAccelerationStructureBuildDesc buildDesc;
	buildDesc.m_Type = RayTracingAccelerationStructureType::kBottomLevel;
	buildDesc.m_pAS = m_BLAS;
	buildDesc.m_pScratchBuffer = scratchAS;
	buildDesc.m_ScratchBufferOffset = 0;
	auto& blasBuildGeomDesc = buildDesc.m_Geometries.EmplaceBack();
	blasBuildGeomDesc.m_Type = RayTracingGeometryType::kTriangles;
	blasBuildGeomDesc.m_Flags = RayTracingGeometryFlags::kOpaque;
	blasBuildGeomDesc.m_Id = "Triangle"_sid;
	blasBuildGeomDesc.m_Triangle.m_pVertexBuffer = m_VBTri;
	blasBuildGeomDesc.m_Triangle.m_pIndexBuffer = m_IBTri;

	m_Context->NewCommandList();
	m_Context->BuildRayTracingAccelerationStructure(buildDesc);
	m_Context->ExecuteAndWait();
}


void RayTracing2::CreateTLAS()
{
	RayTracingAccelerationStructureDesc tlasDesc;
	tlasDesc.m_Type = RayTracingAccelerationStructureType::kTopLevel;
	tlasDesc.m_Flags = RayTracingAccelerationStructureFlags::kPreferFastTrace;
	auto& instanceDesc = tlasDesc.m_Geometries.EmplaceBack();
	instanceDesc.m_Instance.m_InstanceCount = 1;
	instanceDesc.m_Flags = RayTracingGeometryFlags::kOpaque;
	instanceDesc.m_Id = "Instance"_sid;
	m_Device->CreateAccelerationStructure(tlasDesc, &m_TLAS);

	BufferDesc bufferDesc;
	bufferDesc.m_Size = RoundToNearestPowerOf2(m_TLAS->GetBuildSizes().m_Build, m_Device->GetBufferOffsetAlignment(BufferUsage::kRayTracing));
	bufferDesc.m_UsageFlags = BufferUsage::kRayTracing;
	BvRCRef<IBvBuffer> scratchAS;
	m_Device->CreateBuffer(bufferDesc, &scratchAS);

	BvRCRef<IBvBuffer> stagingBuffer;
	bufferDesc.m_Size = sizeof(RayTracingAccelerationStructureInstanceDesc);
	bufferDesc.m_MemoryType = MemoryType::kUpload;
	m_Device->CreateBuffer(bufferDesc, &stagingBuffer);

	RayTracingAccelerationStructureInstanceDesc tlasInstance;
	tlasInstance.m_AccelerationStructure = m_BLAS->GetDeviceAddress();
	tlasInstance.m_Flags = RayTracingInstanceFlags::kTriangleCullDisable;
	tlasInstance.m_InstanceMask = 0xFF;
	tlasInstance.m_InstanceId = 0;
	tlasInstance.m_ShaderBindingTableIndex = 0;
	m_TLAS->WriteTopLevelInstances(stagingBuffer, 1, &tlasInstance);

	RayTracingAccelerationStructureBuildDesc buildDesc;
	buildDesc.m_Type = RayTracingAccelerationStructureType::kTopLevel;
	buildDesc.m_pAS = m_TLAS;
	buildDesc.m_pScratchBuffer = scratchAS;
	auto& tlasBuildGeomDesc = buildDesc.m_Geometries.EmplaceBack();
	tlasBuildGeomDesc.m_Instance.m_Count = 1;
	tlasBuildGeomDesc.m_Instance.m_pBuffer = stagingBuffer;

	m_Context->NewCommandList();
	m_Context->BuildRayTracingAccelerationStructure(buildDesc);
	m_Context->ExecuteAndWait();
}


SAMPLE_MAIN(RayTracing2)