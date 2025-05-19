#include "RayTracing2.h"


constexpr const char* g_pRGenShader =
R"raw(
#version 460
#extension GL_EXT_ray_tracing : require

layout(binding = 0, rgba32f) uniform image2D outputImage;
layout(binding = 1) uniform accelerationStructureEXT topLevelAS;
layout(binding = 2) uniform RayData
{
	mat4 viewInv;
	mat4 projInv;
} rayData;

layout(location = 0) rayPayloadEXT vec4 payload;

void main()
{
	const vec2 pixelCenter = vec2(gl_LaunchIDEXT.xy) + vec2(0.5);
	const vec2 inUV = pixelCenter/vec2(gl_LaunchSizeEXT.xy);
	vec2 d = inUV * 2.0 - 1.0;

	vec4 origin = rayData.viewInv * vec4(0,0,0,1);
	vec4 target = rayData.projInv * vec4(d.x, d.y, 1, 1) ;
	vec4 direction = rayData.viewInv*vec4(normalize(target.xyz), 0) ;

	float tmin = 0.001;
	float tmax = 10000.0;

	payload = vec4(d, 0.0, 1.0);

    traceRayEXT(topLevelAS, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, origin.xyz, tmin, direction.xyz, tmax, 0);

    imageStore(outputImage, ivec2(gl_LaunchIDEXT.xy), payload);
}
)raw";
constexpr auto g_RGenSize = std::char_traits<char>::length(g_pRGenShader);


constexpr const char* g_pRMissShader =
R"raw(
#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadInEXT vec4 payload;

layout(push_constant) uniform PC
{
	vec3 color;
} pc;

vec3 verticalGradient(vec3 topColor, float uvY)
{
    return topColor * uvY;
}

void main()
{
	vec2 uv = payload.xy;
	vec3 color = verticalGradient(pc.color, 1.0 - uv.y);
    payload = vec4(color, 1.0);
}
)raw";
constexpr auto g_RMissSize = std::char_traits<char>::length(g_pRMissShader);


constexpr const char* g_pRCHitShader =
R"raw(
#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadInEXT vec4 payload;

hitAttributeEXT vec2 attribs;

void main()
{
    float u = attribs.x;
    float v = attribs.y;
    float w = 1.0 - u - v;

    payload = vec4(w, u, v, 1.0);
}
)raw";
constexpr auto g_RCHitSize = std::char_traits<char>::length(g_pRCHitShader);


constexpr const char* g_pVSShader =
R"raw(
#version 450

layout (location = 0) out vec2 outUV;

vec2 vertices[] = 
{
	vec2(-1.0f,  1.0f),
	vec2( 1.0f,  1.0f),
	vec2( 1.0f, -1.0f),
	vec2(-1.0f,  1.0f),
	vec2( 1.0f, -1.0f),
	vec2(-1.0f, -1.0f)
};

vec2 uvs[] = 
{
	vec2(0.0f, 0.0f),
	vec2(1.0f, 0.0f),
	vec2(1.0f, 1.0f),
	vec2(0.0f, 0.0f),
	vec2(1.0f, 1.0f),
	vec2(0.0f, 1.0f)
};

void main()
{
	gl_Position = vec4(vertices[gl_VertexIndex], 0.0f, 1.0f);
	outUV = uvs[gl_VertexIndex];
}
)raw";
constexpr auto g_VSSize = std::char_traits<char>::length(g_pVSShader);

constexpr const char* g_pPSShader =
R"raw(
#version 450

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

layout (binding = 0) uniform texture2D samplerTexture;
layout (binding = 1) uniform sampler samplerObj;

void main()
{
	outColor = texture(sampler2D(samplerTexture, samplerObj), inUV);
}
)raw";
constexpr auto g_PSSize = std::char_traits<char>::length(g_pPSShader);


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
	Store(m_Camera.GetViewInv(), m_pRayData->viewInv);
	Store(m_Camera.GetProjInv(), m_pRayData->projInv);
}


void RayTracing2::OnUpdateUI()
{
	BeginDrawDefaultUI();
	ImGui::ColorEdit3("Background", m_BackColor.v);
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

	RenderTargetDesc targets[] =
	{
		RenderTargetDesc::AsSwapChain(m_SwapChain->GetCurrentTextureView(), { 0.1f, 0.1f, 0.3f }),
	};

	m_Context->SetRenderTargets(1, targets);
	m_Context->SetGraphicsPipeline(m_PSO);
	m_Context->SetViewport({ 0.0f, 0.0f, (f32)width, (f32)height, 0.0f, 1.0f });
	m_Context->SetScissor({ 0, 0, width, height });
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
		ShaderResourceDesc descs[3];
		descs[0] = ShaderResourceDesc::AsRWTexture(0, ShaderStage::kRayGen);
		descs[1] = ShaderResourceDesc::AsAccelerationStructure(1, ShaderStage::kRayGen);
		descs[2] = ShaderResourceDesc::AsConstantBuffer(2, ShaderStage::kRayGen);

		ShaderResourceConstantDesc constDesc = ShaderResourceConstantDesc::As<Float3>(BV_NAME_ID("PC"), 3, ShaderStage::kMiss);

		ShaderResourceSetDesc setDesc{};
		setDesc.m_ResourceCount = 3;
		setDesc.m_pResources = descs;
		setDesc.m_ConstantCount = 1;
		setDesc.m_pConstants = &constDesc;

		ShaderResourceLayoutDesc layoutDesc{};
		layoutDesc.m_ShaderResourceSetCount = 1;
		layoutDesc.m_pShaderResourceSets = &setDesc;

		m_RaySRL = m_Device->CreateShaderResourceLayout(layoutDesc);
	}

	{
		ShaderResourceDesc resourceDescs[] =
		{
			ShaderResourceDesc::AsTexture(0, ShaderStage::kPixelOrFragment),
			ShaderResourceDesc::AsSampler(1, ShaderStage::kPixelOrFragment)
		};

		ShaderResourceSetDesc setDesc{};
		setDesc.m_ResourceCount = 2;
		setDesc.m_pResources = resourceDescs;

		ShaderResourceLayoutDesc layoutDesc{};
		layoutDesc.m_ShaderResourceSetCount = 1;
		layoutDesc.m_pShaderResourceSets = &setDesc;

		m_SRL = m_Device->CreateShaderResourceLayout(layoutDesc);
	}
}


void RayTracing2::CreatePipeline()
{
	{
		auto rgen = CompileShader(g_pRGenShader, g_RGenSize, ShaderStage::kRayGen);
		auto rmis = CompileShader(g_pRMissShader, g_RMissSize, ShaderStage::kMiss);
		auto rcht = CompileShader(g_pRCHitShader, g_RCHitSize, ShaderStage::kClosestHit);

		IBvShader* ppShaders[] =
		{
			rgen, rmis, rcht
		};

		ShaderGroupDesc groupDescs[3];
		groupDescs[0].m_Type = ShaderGroupType::kGeneral;
		groupDescs[0].m_General = 0;

		groupDescs[1].m_Type = ShaderGroupType::kGeneral;
		groupDescs[1].m_General = 1;

		groupDescs[2].m_Type = ShaderGroupType::kTriangles;
		groupDescs[2].m_ClosestHit = 2;

		RayTracingPipelineStateDesc pipelineDesc;
		pipelineDesc.m_ShaderGroupCount = 3;
		pipelineDesc.m_pShaderGroupDescs = groupDescs;
		pipelineDesc.m_ShaderCount = 3;
		pipelineDesc.m_ppShaders = ppShaders;
		pipelineDesc.m_pShaderResourceLayout = m_RaySRL;
		pipelineDesc.m_MaxPipelineRayRecursionDepth = 1;
		m_RayPSO = m_Device->CreateRayTracingPipeline(pipelineDesc);

		ShaderBindingTableDesc sbtDesc;
		sbtDesc.m_pPSO = m_RayPSO;
		m_SBT = m_Device->CreateShaderBindingTable(sbtDesc, m_Context);
	}

	{
		auto vs = CompileShader(g_pVSShader, g_VSSize, ShaderStage::kVertex);
		auto ps = CompileShader(g_pPSShader, g_PSSize, ShaderStage::kPixelOrFragment);
		GraphicsPipelineStateDesc pipelineDesc;
		pipelineDesc.m_Shaders[0] = vs;
		pipelineDesc.m_Shaders[1] = ps;
		pipelineDesc.m_RenderTargetFormats[0] = m_SwapChain->GetDesc().m_Format;
		pipelineDesc.m_pShaderResourceLayout = m_SRL;

		m_PSO = m_Device->CreateGraphicsPipeline(pipelineDesc);
	}
}


void RayTracing2::CreateResources()
{
	TextureDesc desc;
	desc.m_Size = { m_pWindow->GetWidth(), m_pWindow->GetHeight(), 1 };
	desc.m_Format = Format::kRGBA32_Float;
	desc.m_ResourceState = ResourceState::kPixelShaderResource;
	desc.m_UsageFlags = TextureUsage::kUnorderedAccess | TextureUsage::kShaderResource;
	TextureInitData initData{};
	initData.m_pContext = m_Context;
	m_Tex = m_Device->CreateTexture(desc, &initData);

	TextureViewDesc viewDesc;
	viewDesc.m_Format = desc.m_Format;
	viewDesc.m_pTexture = m_Tex;
	m_TexView = m_Device->CreateTextureView(viewDesc);

	m_Sampler = m_Device->CreateSampler(SamplerDesc());

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
	m_VBTri = m_Device->CreateBuffer(bufferDesc, &bufferInitData);

	u32 indices[3] = { 0, 1, 2 };
	bufferDesc.m_Size = sizeof(indices);
	bufferDesc.m_UsageFlags = BufferUsage::kRayTracing;
	bufferInitData.m_pData = indices;
	bufferInitData.m_Size = bufferDesc.m_Size;
	m_IBTri = m_Device->CreateBuffer(bufferDesc, &bufferInitData);

	bufferDesc.m_Size = sizeof(RayData);
	bufferDesc.m_UsageFlags = BufferUsage::kConstantBuffer;
	bufferDesc.m_MemoryType = MemoryType::kUpload;
	bufferDesc.m_CreateFlags = BufferCreateFlags::kCreateMapped;
	m_UBRayData = m_Device->CreateBuffer(bufferDesc);
	m_pRayData = m_UBRayData->GetMappedDataAsT<RayData>();

	BufferViewDesc bufferViewDesc;
	bufferViewDesc.m_ElementCount = 1;
	bufferViewDesc.m_Stride = bufferDesc.m_Size;
	bufferViewDesc.m_pBuffer = m_UBRayData;
	m_UBViewRayData = m_Device->CreateBufferView(bufferViewDesc);
}


void RayTracing2::CreateBLAS()
{
	BLASGeometryDesc geomDesc;
	geomDesc.m_Type = RayTracingGeometryType::kTriangles;
	geomDesc.m_Flags = RayTracingGeometryFlags::kOpaque;
	geomDesc.m_Id = BV_NAME_ID("Triangle");
	geomDesc.m_Triangle.m_VertexCount = 3;
	geomDesc.m_Triangle.m_VertexFormat = Format::kRGB32_Float;
	geomDesc.m_Triangle.m_VertexStride = sizeof(Float3);
	geomDesc.m_Triangle.m_IndexCount = 3;
	geomDesc.m_Triangle.m_IndexFormat = IndexFormat::kU32;

	RayTracingAccelerationStructureDesc blasDesc;
	blasDesc.m_Type = RayTracingAccelerationStructureType::kBottomLevel;
	blasDesc.m_Flags = RayTracingAccelerationStructureFlags::kPreferFastTrace;
	blasDesc.m_BLAS.m_GeometryCount = 1;
	blasDesc.m_BLAS.m_pGeometries = &geomDesc;
	m_BLAS = m_Device->CreateAccelerationStructure(blasDesc);

	BufferDesc bufferDesc;
	bufferDesc.m_Size = m_BLAS->GetBuildSizes().m_Build;
	bufferDesc.m_UsageFlags = BufferUsage::kRayTracing;
	BvRCRef<IBvBuffer> scratchBlas = m_Device->CreateBuffer(bufferDesc);

	BLASBuildGeometryDesc blasBuildGeomDesc;
	blasBuildGeomDesc.m_Type = RayTracingGeometryType::kTriangles;
	blasBuildGeomDesc.m_Flags = RayTracingGeometryFlags::kOpaque;
	blasBuildGeomDesc.m_Id = BV_NAME_ID("Triangle");
	blasBuildGeomDesc.m_Triangle.m_pVertexBuffer = m_VBTri;
	blasBuildGeomDesc.m_Triangle.m_pIndexBuffer = m_IBTri;

	BLASBuildDesc blasBuildDesc;
	blasBuildDesc.m_pBLAS = m_BLAS;
	blasBuildDesc.m_pScratchBuffer = scratchBlas;
	blasBuildDesc.m_GeometryCount = 1;
	blasBuildDesc.m_pGeometries = &blasBuildGeomDesc;

	m_Context->NewCommandList();
	m_Context->BuildBLAS(blasBuildDesc);
	m_Context->ExecuteAndWait();
}


void RayTracing2::CreateTLAS()
{
	RayTracingAccelerationStructureDesc tlasDesc;
	tlasDesc.m_Type = RayTracingAccelerationStructureType::kTopLevel;
	tlasDesc.m_Flags = RayTracingAccelerationStructureFlags::kPreferFastTrace;
	tlasDesc.m_TLAS.m_InstanceCount = 1;
	tlasDesc.m_TLAS.m_Flags = RayTracingGeometryFlags::kOpaque;
	m_TLAS = m_Device->CreateAccelerationStructure(tlasDesc);

	BufferDesc bufferDesc;
	bufferDesc.m_Size = m_TLAS->GetBuildSizes().m_Build;
	bufferDesc.m_UsageFlags = BufferUsage::kRayTracing;
	BvRCRef<IBvBuffer> scratchTlas = m_Device->CreateBuffer(bufferDesc);

	TLASInstanceDesc tlasInstance;
	tlasInstance.m_pBLAS = m_BLAS;
	tlasInstance.m_Flags = RayTracingInstanceFlags::kTriangleCullDisable;
	tlasInstance.m_InstanceMask = 0xFF;
	tlasInstance.m_InstanceId = 0;
	tlasInstance.m_ShaderBindingTableIndex = 0;
	m_TLAS->WriteTopLevelInstances(1, &tlasInstance);

	TLASBuildDesc tlasBuilDesc;
	tlasBuilDesc.m_InstanceCount = 1;
	tlasBuilDesc.m_pInstanceBuffer = m_TLAS->GetTopLevelStagingInstanceBuffer();
	tlasBuilDesc.m_pTLAS = m_TLAS;
	tlasBuilDesc.m_pScratchBuffer = scratchTlas;

	m_Context->NewCommandList();
	m_Context->BuildTLAS(tlasBuilDesc);
	m_Context->ExecuteAndWait();
}


SAMPLE_MAIN(RayTracing2)