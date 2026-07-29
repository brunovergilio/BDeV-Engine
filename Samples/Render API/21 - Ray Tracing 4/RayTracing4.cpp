#include "RayTracing4.h"
#include "BDeV/Core/Math/BvGeometryGenerator.h"
#include "Shaders.h"


struct Vertex
{
	Float3 m_Position;
	Float2 m_UV;
	Float3 m_Normal;
};


void RayTracing4::OnInitialize()
{
	m_AppName = "Ray Tracing 4";
	CreateShaderResourceLayout();
	CreatePipeline();
	CreateResources();
	CreateBLAS();
	CreateTLAS();

	m_Camera.SetPos(0.0f, 0.0f, -2.0f);
	m_Camera.SetFlipViewportY(true);
}


void RayTracing4::OnUpdate()
{
	static f32 speed = 2.0f;
	static f32 angleX = 0.0f;
	static f32 angleY = 0.0f;
	static f32 angleZ = 0.0f;

	if (m_Animate)
	{
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

		m_WorldPos[0] = BvMatrix::RotationX(angleX) * BvMatrix::RotationY(angleY) * BvMatrix::RotationZ(angleZ);
		XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(m_Instances[0].m_Transform), m_WorldPos[0]);
	}

	m_WorldPos[1] = BvMatrix::Translation(0.0f, -2.0f, 0.0f);
	XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(m_Instances[1].m_Transform), m_WorldPos[1]);

	XMStoreFloat4x4(&m_pRayData->viewInv, m_Camera.GetViewInv());
	XMStoreFloat4x4(&m_pRayData->projInv, m_Camera.GetProjInv());

	BvVec3 lightDir = BvVec3(0.5f, -0.5f, 1.0f).Normalize();
	XMStoreFloat4x4(&m_pHitData->world[0], m_WorldPos[0]);
	XMStoreFloat4x4(&m_pHitData->world[1], m_WorldPos[1]);
	XMStoreFloat3(&m_pHitData->lightDir, lightDir);
}


void RayTracing4::OnUpdateUI()
{
	BeginDrawDefaultUI();
	ImGui::Checkbox("Animate Cube", &m_Animate);
	ImGui::ColorEdit3("Background", &m_BackColor.x);
	EndDrawDefaultUI();
}


void RayTracing4::OnRender()
{
	auto width = m_pWindow->GetWidth();
	auto height = m_pWindow->GetHeight();

	m_Context->NewCommandList();

	ResourceBarrierDesc barriers[2];
	barriers[0].m_pTexture = m_Tex;
	barriers[0].m_SrcState = ResourceState::kPixelShaderResource;
	barriers[0].m_DstState = ResourceState::kRWResource;

	barriers[1].m_pBuffer = m_ScratchTLAS;
	barriers[1].m_Type = ResourceBarrierDesc::Type::kMemory;
	barriers[1].m_SrcState = ResourceState::kASBuildWrite;
	barriers[1].m_DstState = ResourceState::kASBuildRead;

	if (m_Animate)
	{
		UpdateTLAS();
	}

	m_Context->ResourceBarrier(2, barriers);
	m_Context->SetRayTracingPipeline(m_RayPSO);
	m_Context->SetRWTexture(m_TexView, 0, 0);
	m_Context->SetAccelerationStructure(m_TLAS, 0, 1);
	m_Context->SetConstantBuffer(m_UBViewRayData, 0, 2);
	m_Context->SetShaderConstantsT<Float3>(m_BackColor, 3, 0);

	m_Context->SetStructuredBuffer(m_VBView, 1, 0);
	m_Context->SetStructuredBuffer(m_IBView, 1, 1);
	m_Context->SetConstantBuffer(m_UBViewHitData, 1, 2);

	m_Context->DispatchRays(m_SBT, 0, 0, 0, 0, width, height, 1);

	barriers[0].m_SrcState = ResourceState::kRWResource;
	barriers[0].m_DstState = ResourceState::kPixelShaderResource;
	m_Context->ResourceBarrier(1, barriers);

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


void RayTracing4::OnShutdown()
{
	m_RayPSO.Reset();
	m_RaySRL.Reset();
	m_PSO.Reset();
	m_SRL.Reset();
	m_Tex.Reset();
	m_TexView.Reset();
	m_Sampler.Reset();
	m_TLAS.Reset();
	m_BLAS[0].Reset();
	m_BLAS[1].Reset();
	m_SBT.Reset();
	m_ScratchTLAS.Reset();
	m_StagingBuffer.Reset();
	m_VBView.Reset();
	m_VB.Reset();
	m_IBView.Reset();
	m_IB.Reset();
	m_UBViewRayData.Reset();
	m_UBRayData.Reset();
	m_UBViewHitData.Reset();
	m_UBHitData.Reset();
}


void RayTracing4::CreateShaderResourceLayout()
{
	{
		ShaderResourceLayoutCreateDesc layoutDesc;
		layoutDesc.AddResourceSet()
			.AddRWTexture(0, ShaderStage::kRayGen)
			.AddAccelerationStructure(1, ShaderStage::kRayGen | ShaderStage::kClosestHit)
			.AddConstantBuffer(2, ShaderStage::kRayGen)
			.AddConstant<Float3>("PC"_sid, 3, ShaderStage::kMiss);
		layoutDesc.AddResourceSet()
			.AddStructuredBuffer(0, ShaderStage::kClosestHit)
			.AddStructuredBuffer(1, ShaderStage::kClosestHit)
			.AddConstantBuffer(2, ShaderStage::kClosestHit);

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


void RayTracing4::CreatePipeline()
{
	{
		m_RGen = CompileShader(g_pRGenShader, g_RGenSize, ShaderStage::kRayGen);
		m_Miss = CompileShader(g_pRMissShader, g_RMissSize, ShaderStage::kMiss);
		m_CHit = CompileShader(g_pRCHitShader, g_RCHitSize, ShaderStage::kClosestHit);
		m_MissShadow = CompileShader(g_pRMissShadowShader, g_RMissShadowSize, ShaderStage::kMiss);

		RayTracingPipelineStateDesc pipelineDesc;
		pipelineDesc.AddShader(m_RGen).AddShader(m_Miss).AddShader(m_CHit).AddShader(m_MissShadow)
			.AddGeneralShaderGroup("", 0).AddGeneralShaderGroup("", 1).AddTriangleShaderGroup("", 2, ShaderGroupDesc::kUnusedShader).AddGeneralShaderGroup("", 3);
		pipelineDesc.m_pShaderResourceLayout = m_RaySRL;
		pipelineDesc.m_MaxPipelineRayRecursionDepth = 2;
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


void RayTracing4::CreateResources()
{
	TextureDesc desc;
	desc.m_Size = { m_pWindow->GetWidth(), m_pWindow->GetHeight(), 1 };
	desc.m_Format = Format::kRGBA32_Float;
	desc.m_UsageFlags = TextureUsage::kUnorderedAccess | TextureUsage::kShaderResource;
	TextureInitData initData{};
	initData.m_ResourceState = ResourceState::kPixelShaderResource;
	initData.m_pContext = m_Context;
	m_Device->CreateTexture(desc, initData, &m_Tex);

	TextureViewDesc viewDesc;
	viewDesc.m_Format = desc.m_Format;
	viewDesc.m_pTexture = m_Tex;
	m_Device->CreateTextureView(viewDesc, &m_TexView);

	m_Device->CreateSampler(SamplerDesc(), &m_Sampler);

	BufferInitData bufferInitData;
	bufferInitData.m_pContext = m_Context;
	BufferDesc bufferDesc;
	BufferViewDesc bufferViewDesc;
	bufferViewDesc.m_ElementCount = 1;
	
	BvGeometryGenerator boxGen;
	boxGen.GenerateBox();
	auto& box = boxGen.GetData();

	BvGeometryGenerator gridGen;
	gridGen.GenerateGrid(16.0f, 16.0f, 8, 8);
	auto& grid = gridGen.GetData();

	BvVector<Vertex> vertices(box.m_Vertices.Size() + grid.m_Vertices.Size());
	auto currIndex = 0;
	for (auto i = 0; i < box.m_Vertices.Size(); ++i, ++currIndex)
	{
		vertices[currIndex].m_Position = box.m_Vertices[i].m_Position;
		vertices[currIndex].m_UV = box.m_Vertices[i].m_UV;
		vertices[currIndex].m_Normal = box.m_Vertices[i].m_Normal;
	}
	for (auto i = 0; i < grid.m_Vertices.Size(); ++i, ++currIndex)
	{
		vertices[currIndex].m_Position = grid.m_Vertices[i].m_Position;
		vertices[currIndex].m_UV = grid.m_Vertices[i].m_UV;
		vertices[currIndex].m_Normal = grid.m_Vertices[i].m_Normal;
	}

	BvVector<u32> indices(box.m_Indices.Size() + grid.m_Indices.Size());
	currIndex = 0;
	for (auto i = 0; i < box.m_Indices.Size(); ++i, ++currIndex)
	{
		indices[currIndex] = box.m_Indices[i];
	}
	for (auto i = 0; i < grid.m_Indices.Size(); ++i, ++currIndex)
	{
		indices[currIndex] = grid.m_Indices[i];
	}

	bufferDesc.m_Size = sizeof(Vertex) * vertices.Size();
	bufferDesc.m_UsageFlags = BufferUsage::kRayTracing | BufferUsage::kStructuredBuffer;
	bufferInitData.m_pData = vertices.Data();
	bufferInitData.m_Size = bufferDesc.m_Size;
	m_Device->CreateBuffer(bufferDesc, bufferInitData, &m_VB);

	bufferViewDesc.m_Stride = bufferDesc.m_Size;
	bufferViewDesc.m_pBuffer = m_VB;
	m_Device->CreateBufferView(bufferViewDesc, &m_VBView);

	bufferDesc.m_Size = sizeof(u32) * indices.Size();
	bufferDesc.m_UsageFlags = BufferUsage::kRayTracing | BufferUsage::kStructuredBuffer;
	bufferInitData.m_pData = indices.Data();
	bufferInitData.m_Size = bufferDesc.m_Size;
	m_Device->CreateBuffer(bufferDesc, bufferInitData, &m_IB);

	bufferViewDesc.m_Stride = bufferDesc.m_Size;
	bufferViewDesc.m_pBuffer = m_IB;
	m_Device->CreateBufferView(bufferViewDesc, &m_IBView);

	bufferDesc.m_Size = sizeof(RayData);
	bufferDesc.m_UsageFlags = BufferUsage::kConstantBuffer;
	bufferDesc.m_MemoryType = MemoryType::kUpload;
	bufferDesc.m_CreateFlags = BufferCreateFlags::kCreateMapped;
	m_Device->CreateBuffer(bufferDesc, &m_UBRayData);
	m_pRayData = m_UBRayData->GetMappedDataAsT<RayData>();

	bufferViewDesc.m_Stride = bufferDesc.m_Size;
	bufferViewDesc.m_pBuffer = m_UBRayData;
	m_Device->CreateBufferView(bufferViewDesc, &m_UBViewRayData);

	bufferDesc.m_Size = sizeof(HitData);
	m_Device->CreateBuffer(bufferDesc, &m_UBHitData);
	m_pHitData = m_UBHitData->GetMappedDataAsT<HitData>();

	bufferViewDesc.m_Stride = bufferDesc.m_Size;
	bufferViewDesc.m_pBuffer = m_UBHitData;
	m_Device->CreateBufferView(bufferViewDesc, &m_UBViewHitData);
}


void RayTracing4::CreateBLAS()
{
	{
		RayTracingAccelerationStructureDesc blasDesc;
		blasDesc.m_Type = RayTracingAccelerationStructureType::kBottomLevel;
		blasDesc.m_Flags = RayTracingAccelerationStructureFlags::kPreferFastTrace;
		auto& geomDesc = blasDesc.m_Geometries.EmplaceBack();
		geomDesc.m_Type = RayTracingGeometryType::kTriangles;
		geomDesc.m_Flags = RayTracingGeometryFlags::kOpaque;
		geomDesc.m_Id = "Cube"_sid;
		geomDesc.m_Triangle.m_VertexCount = 24;
		geomDesc.m_Triangle.m_VertexFormat = Format::kRGB32_Float;
		geomDesc.m_Triangle.m_VertexStride = sizeof(Vertex);
		geomDesc.m_Triangle.m_IndexCount = 36;
		geomDesc.m_Triangle.m_IndexFormat = IndexFormat::kU32;
		m_Device->CreateAccelerationStructure(blasDesc, &m_BLAS[0]);
	}

	{
		RayTracingAccelerationStructureDesc blasDesc;
		blasDesc.m_Type = RayTracingAccelerationStructureType::kBottomLevel;
		blasDesc.m_Flags = RayTracingAccelerationStructureFlags::kPreferFastTrace;
		auto& geomDesc = blasDesc.m_Geometries.EmplaceBack();
		geomDesc.m_Type = RayTracingGeometryType::kTriangles;
		geomDesc.m_Flags = RayTracingGeometryFlags::kOpaque;
		geomDesc.m_Id = "Grid"_sid;
		geomDesc.m_Triangle.m_VertexCount = m_VB->GetDesc().m_Size / sizeof(Vertex) - 24;
		geomDesc.m_Triangle.m_VertexFormat = Format::kRGB32_Float;
		geomDesc.m_Triangle.m_VertexStride = sizeof(Vertex);
		geomDesc.m_Triangle.m_IndexCount = m_IB->GetDesc().m_Size / sizeof(u32) - 36;
		geomDesc.m_Triangle.m_IndexFormat = IndexFormat::kU32;
		m_Device->CreateAccelerationStructure(blasDesc, &m_BLAS[1]);
	}

	const auto alignment = m_Device->GetBufferOffsetAlignment(BufferUsage::kRayTracing);
	u64 sizes[] = { RoundToNearestPowerOf2(m_BLAS[0]->GetBuildSizes().m_Build, alignment), RoundToNearestPowerOf2(m_BLAS[1]->GetBuildSizes().m_Build, alignment) };

	BufferDesc bufferDesc;
	bufferDesc.m_Size = sizes[0] + sizes[1];
	bufferDesc.m_UsageFlags = BufferUsage::kRayTracing;
	BvRCRef<IBvBuffer> scratchBlas;
	m_Device->CreateBuffer(bufferDesc, &scratchBlas);

	RayTracingAccelerationStructureBuildDesc buildDescs[2];
	buildDescs[0].m_Type = RayTracingAccelerationStructureType::kBottomLevel;
	buildDescs[0].m_pAS = m_BLAS[0];
	buildDescs[0].m_pScratchBuffer = scratchBlas;
	buildDescs[0].m_ScratchBufferOffset = 0;
	auto& blasBuildGeomDesc0 = buildDescs[0].m_Geometries.EmplaceBack();
	blasBuildGeomDesc0.m_Type = RayTracingGeometryType::kTriangles;
	blasBuildGeomDesc0.m_Flags = RayTracingGeometryFlags::kOpaque;
	blasBuildGeomDesc0.m_Id = "Cube"_sid;
	blasBuildGeomDesc0.m_Triangle.m_pVertexBuffer = m_VB;
	blasBuildGeomDesc0.m_Triangle.m_pIndexBuffer = m_IB;

	buildDescs[1].m_Type = RayTracingAccelerationStructureType::kBottomLevel;
	buildDescs[1].m_pAS = m_BLAS[1];
	buildDescs[1].m_pScratchBuffer = scratchBlas;
	buildDescs[1].m_ScratchBufferOffset = sizes[0];
	auto& blasBuildGeomDesc1 = buildDescs[1].m_Geometries.EmplaceBack();
	blasBuildGeomDesc1.m_Type = RayTracingGeometryType::kTriangles;
	blasBuildGeomDesc1.m_Flags = RayTracingGeometryFlags::kOpaque;
	blasBuildGeomDesc1.m_Id = "Grid"_sid;
	blasBuildGeomDesc1.m_Triangle.m_pVertexBuffer = m_VB;
	blasBuildGeomDesc1.m_Triangle.m_VertexOffset = sizeof(Vertex) * 24;
	blasBuildGeomDesc1.m_Triangle.m_pIndexBuffer = m_IB;
	blasBuildGeomDesc1.m_Triangle.m_IndexOffset = sizeof(u32) * 36;

	m_Context->NewCommandList();
	m_Context->BuildRayTracingAccelerationStructures(2, buildDescs);
	m_Context->ExecuteAndWait();
}


void RayTracing4::CreateTLAS()
{
	RayTracingAccelerationStructureDesc tlasDesc;
	tlasDesc.m_Type = RayTracingAccelerationStructureType::kTopLevel;
	tlasDesc.m_Flags = RayTracingAccelerationStructureFlags::kPreferFastTrace | RayTracingAccelerationStructureFlags::kAllowUpdate;
	auto& instanceDesc = tlasDesc.m_Geometries.EmplaceBack();
	instanceDesc.m_Instance.m_InstanceCount = 2;
	instanceDesc.m_Flags = RayTracingGeometryFlags::kOpaque;
	instanceDesc.m_Id = "Instance"_sid;
	m_Device->CreateAccelerationStructure(tlasDesc, &m_TLAS);

	BufferDesc bufferDesc;
	auto [build, update] = m_TLAS->GetBuildSizes();
	bufferDesc.m_Size = RoundToNearestPowerOf2(std::max(build, update), m_Device->GetBufferOffsetAlignment(BufferUsage::kRayTracing));
	bufferDesc.m_UsageFlags = BufferUsage::kRayTracing;
	m_Device->CreateBuffer(bufferDesc, &m_ScratchTLAS);

	bufferDesc.m_Size = sizeof(RayTracingAccelerationStructureInstanceDesc) * 2;
	bufferDesc.m_MemoryType = MemoryType::kUpload;
	m_Device->CreateBuffer(bufferDesc, &m_StagingBuffer);

	for (auto i = 0; i < 2; i++)
	{
		m_Instances[i].m_AccelerationStructure = m_BLAS[i]->GetDeviceAddress();
		m_Instances[i].m_Flags = RayTracingInstanceFlags::kTriangleCullDisable;
		m_Instances[i].m_InstanceMask = 0xFF;
		m_Instances[i].m_InstanceId = i;
		m_Instances[i].m_ShaderBindingTableIndex = 0;
	}

	m_TLAS->WriteTopLevelInstances(m_StagingBuffer, 2, m_Instances);

	m_TLASUpdate.m_Type = RayTracingAccelerationStructureType::kTopLevel;
	m_TLASUpdate.m_pAS = m_TLAS;
	m_TLASUpdate.m_pScratchBuffer = m_ScratchTLAS;
	auto& tlasBuildGeomDesc = m_TLASUpdate.m_Geometries.EmplaceBack();
	tlasBuildGeomDesc.m_Instance.m_Count = 2;
	tlasBuildGeomDesc.m_Instance.m_pBuffer = m_StagingBuffer;

	m_Context->NewCommandList();
	m_Context->BuildRayTracingAccelerationStructure(m_TLASUpdate);
	m_Context->ExecuteAndWait();
	m_TLASUpdate.m_Update = true;
}


void RayTracing4::UpdateTLAS()
{
	m_TLAS->WriteTopLevelInstances(m_StagingBuffer, 2, m_Instances);

	m_Context->BuildRayTracingAccelerationStructure(m_TLASUpdate);
}


SAMPLE_MAIN(RayTracing4)