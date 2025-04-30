#include "RayTracing4.h"
#include "BDeV/Core/Math/BvGeometryGenerator.h"


struct Vertex
{
	Float3 m_Position;
	Float2 m_UV;
	Float3 m_Normal;
};


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
	vec4 target = rayData.projInv * vec4(d.x, d.y, 1, 1);
	vec4 direction = rayData.viewInv * vec4(normalize(target.xyz), 0);

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

struct ShadowPayload
{
	bool inShadow;
};

layout(location = 0) rayPayloadInEXT vec4 payload;
layout(location = 1) rayPayloadEXT ShadowPayload shadowPayload;

hitAttributeEXT vec2 attribs;

struct Vertex
{
    vec3 pos;
	vec2 uv;
    vec3 normal;
};

layout(set = 0, binding = 1) uniform accelerationStructureEXT topLevelAS;

layout(set = 1, binding = 0) readonly buffer VertexBuffer
{
    vec4 vertices[];
};

layout(set = 1, binding = 1) readonly buffer IndexBuffer
{
    uint indices[];
};

layout(set = 1, binding = 2) uniform HitData
{
	mat4 modelMatrix[2];
	vec3 lightDir;
};

Vertex GetVertex(uint index, uint vertexOffset)
{
    uint baseIndex = 2 * (index + vertexOffset);
    Vertex v;
    v.pos = vertices[baseIndex].xyz;
    v.uv = vec2(vertices[baseIndex].w, vertices[baseIndex + 1].x);
    v.normal = vertices[baseIndex + 1].yzw;
    return v;
}

uint GetIndex(uint primIndex, uint vertexInTri, uint indexOffset)
{
    return indices[indexOffset + primIndex * 3 + vertexInTri];
}

void main()
{
    const uint primIndex = gl_PrimitiveID;
    const uint instanceId = gl_InstanceCustomIndexEXT;

    // Precompute the per-instance offsets only once
    uint vertexOffset = 0;
    uint indexOffset = 0;
    if (instanceId > 0)
    {
        vertexOffset = 24; // 24 vertices for the grid
        indexOffset = 36;  // 36 indices for the grid
    }

    const uint index0 = GetIndex(primIndex, 0, indexOffset);
    const uint index1 = GetIndex(primIndex, 1, indexOffset);
    const uint index2 = GetIndex(primIndex, 2, indexOffset);

    const Vertex v0 = GetVertex(index0, vertexOffset);
    const Vertex v1 = GetVertex(index1, vertexOffset);
    const Vertex v2 = GetVertex(index2, vertexOffset);

	const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

	vec3 normal = normalize(v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z);
	
    normal = normalize((modelMatrix[instanceId] * vec4(normal, 0.0)).xyz); // Transform to world space

	float minTerm = 0.05;
    vec3 baseColor = vec3(1.0, 1.0, 1.0);
	vec3 lDir = normalize(-lightDir);
	float NdotL = dot(normal, lDir);
	if (NdotL > 0.0)
	{
		shadowPayload.inShadow = true; // Assume not shadowed

		// Compute world-space hit position
		vec3 worldPos = gl_WorldRayOriginEXT + gl_HitTEXT * gl_WorldRayDirectionEXT;
		vec3 rayOrigin = worldPos + normal * 0.001; // Offset a little to avoid self-hit
		vec3 rayDirection = lDir;   // Towards light (negate because lightDir points TO surface)

		traceRayEXT(
			topLevelAS, // (you must declare this somewhere, usually a uniform acceleration structure)
			gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT,
			0xFF,       // cull mask
			0, 0, 1,    // SBT record offsets for shadow rays
			rayOrigin,
			0.001,      // min t
			rayDirection,
			10000.0,     // max t
			1           // payload location
		);
		
		if (shadowPayload.inShadow)
		{
			baseColor *= minTerm; // Darken if in shadow
		}
		else
		{
			baseColor *= NdotL; // Otherwise normal lighting
		}
	}
	else
	{
		baseColor *= 0.01; // Darken if in shadow
	}

    payload = vec4(baseColor, 1.0);
}
)raw";
constexpr auto g_RCHitSize = std::char_traits<char>::length(g_pRCHitShader);


constexpr const char* g_pRMissShadowShader =
R"raw(
#version 460
#extension GL_EXT_ray_tracing : require

struct ShadowPayload
{
	bool inShadow;
};

layout(location = 1) rayPayloadInEXT ShadowPayload payload;

void main()
{
	payload.inShadow = false;
}
)raw";
constexpr auto g_RMissShadowSize = std::char_traits<char>::length(g_pRMissShadowShader);


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

		m_WorldPos[0] = BvMatrix(MatrixRotationX(angleX) * MatrixRotationY(angleY) * MatrixRotationZ(angleZ));
		Store(m_WorldPos[0], m_Instances[0].m_Transform);
	}

	m_WorldPos[1] = BvMatrix::Translation(0.0f, -2.0f, 0.0f);
	Store(m_WorldPos[1], m_Instances[1].m_Transform);

	Store(m_Camera.GetViewInv(), m_pRayData->viewInv);
	Store(m_Camera.GetProjInv(), m_pRayData->projInv);

	BvVec3 lightDir = BvVec3(0.5f, -0.5f, 1.0f).Normalize();
	Store(m_WorldPos[0], m_pHitData->world[0]);
	Store(m_WorldPos[1], m_pHitData->world[1]);
	Store(lightDir, m_pHitData->lightDir);
}


void RayTracing4::OnUpdateUI()
{
	BeginDrawDefaultUI();
	ImGui::Checkbox("Animate Cube", &m_Animate);
	ImGui::ColorEdit3("Background", m_BackColor.v);
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
		ShaderResourceDesc descs[3];
		descs[0] = ShaderResourceDesc::AsRWTexture(0, ShaderStage::kRayGen);
		descs[1] = ShaderResourceDesc::AsAccelerationStructure(1, ShaderStage::kRayGen | ShaderStage::kClosestHit);
		descs[2] = ShaderResourceDesc::AsConstantBuffer(2, ShaderStage::kRayGen);

		ShaderResourceConstantDesc constDesc = ShaderResourceConstantDesc::As<Float3>(BV_NAME_ID("PC"), 3, ShaderStage::kMiss);

		ShaderResourceSetDesc setDesc{};
		setDesc.m_ResourceCount = 3;
		setDesc.m_pResources = descs;
		setDesc.m_ConstantCount = 1;
		setDesc.m_pConstants = &constDesc;

		ShaderResourceDesc descs1[3];
		descs1[0] = ShaderResourceDesc::AsStructuredBuffer(0, ShaderStage::kClosestHit);
		descs1[1] = ShaderResourceDesc::AsStructuredBuffer(1, ShaderStage::kClosestHit);
		descs1[2] = ShaderResourceDesc::AsConstantBuffer(2, ShaderStage::kClosestHit);
		ShaderResourceSetDesc setDesc1{};
		setDesc1.m_Index = 1;
		setDesc1.m_ResourceCount = 3;
		setDesc1.m_pResources = descs1;

		ShaderResourceSetDesc setDescs[] = { setDesc, setDesc1 };

		ShaderResourceLayoutDesc layoutDesc{};
		layoutDesc.m_ShaderResourceSetCount = 2;
		layoutDesc.m_pShaderResourceSets = setDescs;

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


void RayTracing4::CreatePipeline()
{
	{
		auto rgen = CompileShader(g_pRGenShader, g_RGenSize, ShaderStage::kRayGen);
		auto rmis = CompileShader(g_pRMissShader, g_RMissSize, ShaderStage::kMiss);
		auto rcht = CompileShader(g_pRCHitShader, g_RCHitSize, ShaderStage::kClosestHit);

		auto rmisShadow = CompileShader(g_pRMissShadowShader, g_RMissShadowSize, ShaderStage::kMiss);

		IBvShader* ppShaders[] =
		{
			rgen, rmis, rcht, rmisShadow
		};

		ShaderGroupDesc groupDescs[5];
		groupDescs[0].m_Type = ShaderGroupType::kGeneral;
		groupDescs[0].m_General = 0;

		groupDescs[1].m_Type = ShaderGroupType::kGeneral;
		groupDescs[1].m_General = 1;

		groupDescs[2].m_Type = ShaderGroupType::kTriangles;
		groupDescs[2].m_ClosestHit = 2;

		groupDescs[3].m_Type = ShaderGroupType::kGeneral;
		groupDescs[3].m_General = 3;

		RayTracingPipelineStateDesc pipelineDesc;
		pipelineDesc.m_ShaderGroupCount = 4;
		pipelineDesc.m_pShaderGroupDescs = groupDescs;
		pipelineDesc.m_ShaderCount = 4;
		pipelineDesc.m_ppShaders = ppShaders;
		pipelineDesc.m_pShaderResourceLayout = m_RaySRL;
		pipelineDesc.m_MaxPipelineRayRecursionDepth = 2;
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


void RayTracing4::CreateResources()
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
	m_VB = m_Device->CreateBuffer(bufferDesc, &bufferInitData);

	bufferViewDesc.m_Stride = bufferDesc.m_Size;
	bufferViewDesc.m_pBuffer = m_VB;
	m_VBView = m_Device->CreateBufferView(bufferViewDesc);

	bufferDesc.m_Size = sizeof(u32) * indices.Size();
	bufferDesc.m_UsageFlags = BufferUsage::kRayTracing | BufferUsage::kStructuredBuffer;
	bufferInitData.m_pData = indices.Data();
	bufferInitData.m_Size = bufferDesc.m_Size;
	m_IB = m_Device->CreateBuffer(bufferDesc, &bufferInitData);

	bufferViewDesc.m_Stride = bufferDesc.m_Size;
	bufferViewDesc.m_pBuffer = m_IB;
	m_IBView = m_Device->CreateBufferView(bufferViewDesc);

	bufferDesc.m_Size = sizeof(RayData);
	bufferDesc.m_UsageFlags = BufferUsage::kConstantBuffer;
	bufferDesc.m_MemoryType = MemoryType::kUpload;
	bufferDesc.m_CreateFlags = BufferCreateFlags::kCreateMapped;
	m_UBRayData = m_Device->CreateBuffer(bufferDesc);
	m_pRayData = m_UBRayData->GetMappedDataAsT<RayData>();

	bufferViewDesc.m_Stride = bufferDesc.m_Size;
	bufferViewDesc.m_pBuffer = m_UBRayData;
	m_UBViewRayData = m_Device->CreateBufferView(bufferViewDesc);

	bufferDesc.m_Size = sizeof(HitData);
	m_UBHitData = m_Device->CreateBuffer(bufferDesc);
	m_pHitData = m_UBHitData->GetMappedDataAsT<HitData>();

	bufferViewDesc.m_Stride = bufferDesc.m_Size;
	bufferViewDesc.m_pBuffer = m_UBHitData;
	m_UBViewHitData = m_Device->CreateBufferView(bufferViewDesc);
}


void RayTracing4::CreateBLAS()
{
	{
		BLASGeometryDesc geomDesc;
		geomDesc.m_Type = RayTracingGeometryType::kTriangles;
		geomDesc.m_Flags = RayTracingGeometryFlags::kOpaque;
		geomDesc.m_Id = BV_NAME_ID("Cube");
		geomDesc.m_Triangle.m_VertexCount = 24;
		geomDesc.m_Triangle.m_VertexFormat = Format::kRGB32_Float;
		geomDesc.m_Triangle.m_VertexStride = sizeof(Vertex);
		geomDesc.m_Triangle.m_IndexCount = 36;
		geomDesc.m_Triangle.m_IndexFormat = IndexFormat::kU32;

		RayTracingAccelerationStructureDesc blasDesc;
		blasDesc.m_Type = RayTracingAccelerationStructureType::kBottomLevel;
		blasDesc.m_Flags = RayTracingAccelerationStructureFlags::kPreferFastTrace;
		blasDesc.m_BLAS.m_GeometryCount = 1;
		blasDesc.m_BLAS.m_pGeometries = &geomDesc;
		m_BLAS[0] = m_Device->CreateAccelerationStructure(blasDesc);
	}

	{
		BLASGeometryDesc geomDesc;
		geomDesc.m_Type = RayTracingGeometryType::kTriangles;
		geomDesc.m_Flags = RayTracingGeometryFlags::kOpaque;
		geomDesc.m_Id = BV_NAME_ID("Grid");
		geomDesc.m_Triangle.m_VertexCount = m_VB->GetDesc().m_Size / sizeof(Vertex) - 24;
		geomDesc.m_Triangle.m_VertexFormat = Format::kRGB32_Float;
		geomDesc.m_Triangle.m_VertexStride = sizeof(Vertex);
		geomDesc.m_Triangle.m_IndexCount = m_IB->GetDesc().m_Size / sizeof(u32) - 36;
		geomDesc.m_Triangle.m_IndexFormat = IndexFormat::kU32;

		RayTracingAccelerationStructureDesc blasDesc;
		blasDesc.m_Type = RayTracingAccelerationStructureType::kBottomLevel;
		blasDesc.m_Flags = RayTracingAccelerationStructureFlags::kPreferFastTrace;
		blasDesc.m_BLAS.m_GeometryCount = 1;
		blasDesc.m_BLAS.m_pGeometries = &geomDesc;
		m_BLAS[1] = m_Device->CreateAccelerationStructure(blasDesc);
	}

	u64 sizes[] = { m_BLAS[0]->GetBuildSizes().m_Build, m_BLAS[1]->GetBuildSizes().m_Build };

	BufferDesc bufferDesc;
	bufferDesc.m_Size = sizes[0] + sizes[1];
	bufferDesc.m_UsageFlags = BufferUsage::kRayTracing;
	BvRCRef<IBvBuffer> scratchBlas = m_Device->CreateBuffer(bufferDesc);

	BLASBuildGeometryDesc blasBuildGeomDescs[2];
	blasBuildGeomDescs[0].m_Type = RayTracingGeometryType::kTriangles;
	blasBuildGeomDescs[0].m_Flags = RayTracingGeometryFlags::kOpaque;
	blasBuildGeomDescs[0].m_Id = BV_NAME_ID("Cube");
	blasBuildGeomDescs[0].m_Triangle.m_pVertexBuffer = m_VB;
	blasBuildGeomDescs[0].m_Triangle.m_pIndexBuffer = m_IB;

	blasBuildGeomDescs[1].m_Type = RayTracingGeometryType::kTriangles;
	blasBuildGeomDescs[1].m_Flags = RayTracingGeometryFlags::kOpaque;
	blasBuildGeomDescs[1].m_Id = BV_NAME_ID("Grid");
	blasBuildGeomDescs[1].m_Triangle.m_pVertexBuffer = m_VB;
	blasBuildGeomDescs[1].m_Triangle.m_VertexOffset = sizeof(Vertex) * 24;
	blasBuildGeomDescs[1].m_Triangle.m_pIndexBuffer = m_IB;
	blasBuildGeomDescs[1].m_Triangle.m_IndexOffset = sizeof(u32) * 36;

	BLASBuildDesc blasBuildDescs[2];
	blasBuildDescs[0].m_pBLAS = m_BLAS[0];
	blasBuildDescs[0].m_pScratchBuffer = scratchBlas;
	blasBuildDescs[0].m_GeometryCount = 1;
	blasBuildDescs[0].m_pGeometries = &blasBuildGeomDescs[0];

	blasBuildDescs[1].m_pBLAS = m_BLAS[1];
	blasBuildDescs[1].m_pScratchBuffer = scratchBlas;
	blasBuildDescs[1].m_ScratchBufferOffset = sizes[0];
	blasBuildDescs[1].m_GeometryCount = 1;
	blasBuildDescs[1].m_pGeometries = &blasBuildGeomDescs[1];

	m_Context->NewCommandList();
	m_Context->BuildBLAS(blasBuildDescs[0]);
	m_Context->BuildBLAS(blasBuildDescs[1]);
	m_Context->Execute();
	m_Context->WaitForGPU();
}


void RayTracing4::CreateTLAS()
{
	RayTracingAccelerationStructureDesc tlasDesc;
	tlasDesc.m_Type = RayTracingAccelerationStructureType::kTopLevel;
	tlasDesc.m_Flags = RayTracingAccelerationStructureFlags::kPreferFastBuild | RayTracingAccelerationStructureFlags::kAllowUpdate;
	tlasDesc.m_TLAS.m_InstanceCount = 2;
	tlasDesc.m_TLAS.m_Flags = RayTracingGeometryFlags::kOpaque;
	m_TLAS = m_Device->CreateAccelerationStructure(tlasDesc);

	BufferDesc bufferDesc;
	auto [build, update] = m_TLAS->GetBuildSizes();
	bufferDesc.m_Size = std::max(build, update);
	bufferDesc.m_UsageFlags = BufferUsage::kRayTracing;
	m_ScratchTLAS = m_Device->CreateBuffer(bufferDesc);

	m_Instances[0].m_pBLAS = m_BLAS[0];
	m_Instances[0].m_Flags = RayTracingInstanceFlags::kTriangleCullDisable;
	m_Instances[0].m_InstanceMask = 0xFF;
	m_Instances[0].m_InstanceId = 0;
	m_Instances[0].m_ShaderBindingTableIndex = 0;
	
	m_Instances[1].m_pBLAS = m_BLAS[1];
	m_Instances[1].m_Flags = RayTracingInstanceFlags::kTriangleCullDisable;
	m_Instances[1].m_InstanceMask = 0xFF;
	m_Instances[1].m_InstanceId = 1;
	m_Instances[1].m_ShaderBindingTableIndex = 0;
	m_TLAS->WriteTopLevelInstances(2, m_Instances);

	TLASBuildDesc tlasBuilDesc;
	tlasBuilDesc.m_InstanceCount = 2;
	tlasBuilDesc.m_pInstanceBuffer = m_TLAS->GetTopLevelStagingInstanceBuffer();
	tlasBuilDesc.m_pTLAS = m_TLAS;
	tlasBuilDesc.m_pScratchBuffer = m_ScratchTLAS;

	m_Context->NewCommandList();
	m_Context->BuildTLAS(tlasBuilDesc);
	m_Context->Execute();
	m_Context->WaitForGPU();
}


void RayTracing4::UpdateTLAS()
{
	m_TLAS->WriteTopLevelInstances(2, m_Instances);

	TLASBuildDesc tlasBuilDesc;
	tlasBuilDesc.m_InstanceCount = 2;
	tlasBuilDesc.m_pInstanceBuffer = m_TLAS->GetTopLevelStagingInstanceBuffer();
	tlasBuilDesc.m_pTLAS = m_TLAS;
	tlasBuilDesc.m_pScratchBuffer = m_ScratchTLAS;
	tlasBuilDesc.m_Update = true;

	m_Context->BuildTLAS(tlasBuilDesc);
}


SAMPLE_MAIN(RayTracing4)