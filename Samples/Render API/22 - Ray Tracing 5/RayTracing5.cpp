#include "RayTracing5.h"
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

struct RayPayload
{
	vec3 hitValue;
	int  depth;
	vec3 attenuation;
	int  done;
	vec3 rayOrigin;
	float u;
	vec3 rayDir;
	float v;
};

layout(binding = 0, rgba32f) uniform image2D outputImage;
layout(binding = 1) uniform accelerationStructureEXT topLevelAS;
layout(binding = 2) uniform RayData
{
	mat4 viewInv;
	mat4 projInv;
} rayData;

layout(location = 0) rayPayloadEXT RayPayload payload;

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

	payload.depth       = 0;
	payload.hitValue    = vec3(0);
	payload.attenuation = vec3(1.f, 1.f, 1.f);
	payload.done        = 1;
	payload.rayOrigin   = origin.xyz;
	payload.rayDir      = direction.xyz;
	payload.u			= d.x;
	payload.v			= d.y;

	vec3 hitValue = vec3(0.0);
	for (;;)
	{
		vec3 attenuation = payload.attenuation; // Attenuation will be applied on bouncing rays.
		traceRayEXT(topLevelAS, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, origin.xyz, tmin, direction.xyz, tmax, 0);
		hitValue += payload.hitValue * attenuation;

		payload.depth++;
		if(payload.done == 1 || payload.depth >= 10)
		  break;

		origin.xyz    = payload.rayOrigin;
		direction.xyz = payload.rayDir;
		payload.done      = 1;  // Will stop if a reflective material isn't hit
	}


    imageStore(outputImage, ivec2(gl_LaunchIDEXT.xy), vec4(hitValue, 1.0));
}
)raw";
constexpr auto g_RGenSize = std::char_traits<char>::length(g_pRGenShader);


constexpr const char* g_pRMissShader =
R"raw(
#version 460
#extension GL_EXT_ray_tracing : require

struct RayPayload
{
	vec3 hitValue;
	int  depth;
	vec3 attenuation;
	int  done;
	vec3 rayOrigin;
	float u;
	vec3 rayDir;
	float v;
};

layout(location = 0) rayPayloadInEXT RayPayload payload;

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
	vec2 uv = vec2(payload.u, payload.v);
	vec3 color = verticalGradient(pc.color, 1.0 - uv.y);
    payload.hitValue = color;
}
)raw";
constexpr auto g_RMissSize = std::char_traits<char>::length(g_pRMissShader);


constexpr const char* g_pRCHitShader =
R"raw(
#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

struct RayPayload
{
	vec3 hitValue;
	int  depth;
	vec3 attenuation;
	int  done;
	vec3 rayOrigin;
	float u;
	vec3 rayDir;
	float v;
};

struct ShadowPayload
{
	bool inShadow;
};

layout(location = 0) rayPayloadInEXT RayPayload payload;
layout(location = 1) rayPayloadEXT ShadowPayload shadowPayload;

hitAttributeEXT vec2 attribs;

struct Vertex
{
    vec3 pos;
	vec2 uv;
    vec3 normal;
};

layout(set = 0, binding = 1) uniform accelerationStructureEXT topLevelAS;

layout(buffer_reference, scalar) buffer Vertices {Vertex v[]; };
layout(buffer_reference, scalar) buffer Indices {uvec3 i[]; };


layout(set = 1, binding = 0) uniform VertexBuffer
{
    Vertices vertices;
};

layout(set = 1, binding = 1) uniform IndexBuffer
{
    Indices indices;
};

layout(set = 1, binding = 2) uniform HitData
{
	mat4 modelMatrix[3];
	vec3 lightDir;
};

Vertex GetVertex(uint index, uint vertexOffset)
{
    uint baseIndex = index + vertexOffset;
    Vertex v = vertices.v[baseIndex];
    return v;
}

uvec3 GetIndices(uint primIndex, uint indexOffset)
{
	uint basePrimIndex = indexOffset / 3;
    return indices.i[basePrimIndex + primIndex];
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

	const uvec3 ind = GetIndices(primIndex, indexOffset);

    const Vertex v0 = GetVertex(ind.x, vertexOffset);
    const Vertex v1 = GetVertex(ind.y, vertexOffset);
    const Vertex v2 = GetVertex(ind.z, vertexOffset);

	const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

	vec3 normal = normalize(v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z);
	
    normal = normalize((modelMatrix[instanceId] * vec4(normal, 0.0)).xyz); // Transform to world space
	vec3 worldPos = (modelMatrix[instanceId] * vec4(v0.pos * barycentrics.x + v1.pos * barycentrics.y + v2.pos * barycentrics.z, 1.0)).xyz;

    vec3 baseColor = vec3(0.0);
	float attenuation = 1.0;

	vec3 lDir = normalize(-lightDir);
	float NdotL = dot(normal, lDir);

	//if (instanceId < 2)
	{
		if (NdotL > 0.0)
		{
			// Compute world-space hit position
			vec3 rayOrigin = (gl_WorldRayOriginEXT + gl_HitTEXT * gl_WorldRayDirectionEXT) + normal * 0.001; // Offset a little to avoid self-hit
			vec3 rayDirection = lDir;   // Towards light (negate because lightDir points TO surface)
			uint flags = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT;

			shadowPayload.inShadow = true; // Assume shadowed
			traceRayEXT(
				topLevelAS, // (you must declare this somewhere, usually a uniform acceleration structure)
				flags,
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
				attenuation = 0.3; // Darken if in shadow
			}
			else
			{
				baseColor = vec3(NdotL);
			}
		}
	}

	// Reflection
	if(instanceId == 2)
	{
		vec3 origin = worldPos;
		vec3 rayDir = reflect(gl_WorldRayDirectionEXT, normal);
		payload.attenuation *= 0.7;
		payload.done      = 0;
		payload.rayOrigin = origin;
		payload.rayDir    = rayDir;
	}
	//else
	{
		payload.hitValue = baseColor * attenuation;
	}
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


void RayTracing5::OnInitialize()
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


void RayTracing5::OnUpdate()
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

	m_WorldPos[2] = BvMatrix::Scale(0.3f) * BvMatrix::RotationY(kPiDiv2) * BvMatrix::RotationZ(kPiDiv2) * BvMatrix::Translation(3.0f, 0.0f, 0.0f);
	Store(m_WorldPos[2], m_Instances[2].m_Transform);

	Store(m_Camera.GetViewInv(), m_pRayData->viewInv);
	Store(m_Camera.GetProjInv(), m_pRayData->projInv);

	BvVec3 lightDir = BvVec3(0.5f, -0.5f, 1.0f).Normalize();
	Store(m_WorldPos[0], m_pHitData->world[0]);
	Store(m_WorldPos[1], m_pHitData->world[1]);
	Store(m_WorldPos[2], m_pHitData->world[2]);
	Store(lightDir, m_pHitData->lightDir);
}


void RayTracing5::OnUpdateUI()
{
	BeginDrawDefaultUI();
	ImGui::Checkbox("Animate Cube", &m_Animate);
	ImGui::ColorEdit3("Background", m_BackColor.v);
	EndDrawDefaultUI();
}


void RayTracing5::OnRender()
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

	m_Context->SetConstantBuffer(m_UBViewVertex, 1, 0);
	m_Context->SetConstantBuffer(m_UBViewIndex, 1, 1);
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


void RayTracing5::OnShutdown()
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
	m_UBViewVertex.Reset();
	m_UBViewIndex.Reset();
	m_UBVertex.Reset();
	m_UBIndex.Reset();
	m_VB.Reset();
	m_IB.Reset();
	m_UBViewRayData.Reset();
	m_UBRayData.Reset();
	m_UBViewHitData.Reset();
	m_UBHitData.Reset();
}


void RayTracing5::CreateShaderResourceLayout()
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
		descs1[0] = ShaderResourceDesc::AsConstantBuffer(0, ShaderStage::kClosestHit);
		descs1[1] = ShaderResourceDesc::AsConstantBuffer(1, ShaderStage::kClosestHit);
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


void RayTracing5::CreatePipeline()
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


void RayTracing5::CreateResources()
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

	bufferDesc.m_Size = sizeof(u32) * indices.Size();
	bufferDesc.m_UsageFlags = BufferUsage::kRayTracing | BufferUsage::kStructuredBuffer;
	bufferInitData.m_pData = indices.Data();
	bufferInitData.m_Size = bufferDesc.m_Size;
	m_IB = m_Device->CreateBuffer(bufferDesc, &bufferInitData);

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

	bufferDesc.m_Size = sizeof(u64);
	m_UBVertex = m_Device->CreateBuffer(bufferDesc);
	*m_UBVertex->GetMappedDataAsT<u64>() = m_VB->GetDeviceAddress();

	bufferViewDesc.m_Stride = bufferDesc.m_Size;
	bufferViewDesc.m_pBuffer = m_UBVertex;
	m_UBViewVertex = m_Device->CreateBufferView(bufferViewDesc);

	bufferDesc.m_Size = sizeof(u64);
	m_UBIndex = m_Device->CreateBuffer(bufferDesc);
	*m_UBIndex->GetMappedDataAsT<u64>() = m_IB->GetDeviceAddress();

	bufferViewDesc.m_Stride = bufferDesc.m_Size;
	bufferViewDesc.m_pBuffer = m_UBIndex;
	m_UBViewIndex = m_Device->CreateBufferView(bufferViewDesc);
}


void RayTracing5::CreateBLAS()
{
	BvRCRef<IBvAccelerationStructure> blas[2];
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
		blasDesc.m_Flags = RayTracingAccelerationStructureFlags::kPreferFastTrace | RayTracingAccelerationStructureFlags::kAllowCompaction;
		blasDesc.m_BLAS.m_GeometryCount = 1;
		blasDesc.m_BLAS.m_pGeometries = &geomDesc;
		blas[0] = m_Device->CreateAccelerationStructure(blasDesc);
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
		blasDesc.m_Flags = RayTracingAccelerationStructureFlags::kPreferFastTrace | RayTracingAccelerationStructureFlags::kAllowCompaction;
		blasDesc.m_BLAS.m_GeometryCount = 1;
		blasDesc.m_BLAS.m_pGeometries = &geomDesc;
		blas[1] = m_Device->CreateAccelerationStructure(blasDesc);
	}

	u64 sizes[] = { blas[0]->GetBuildSizes().m_Build, blas[1]->GetBuildSizes().m_Build };

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
	blasBuildDescs[0].m_pBLAS = blas[0];
	blasBuildDescs[0].m_pScratchBuffer = scratchBlas;
	blasBuildDescs[0].m_GeometryCount = 1;
	blasBuildDescs[0].m_pGeometries = &blasBuildGeomDescs[0];

	blasBuildDescs[1].m_pBLAS = blas[1];
	blasBuildDescs[1].m_pScratchBuffer = scratchBlas;
	blasBuildDescs[1].m_ScratchBufferOffset = sizes[0];
	blasBuildDescs[1].m_GeometryCount = 1;
	blasBuildDescs[1].m_pGeometries = &blasBuildGeomDescs[1];

	BufferDesc bfd;
	bfd.m_Size = sizeof(u64) * 2;
	bfd.m_CreateFlags = BufferCreateFlags::kCreateMapped;
	bfd.m_MemoryType = MemoryType::kReadBack;
	BvRCRef<IBvBuffer> compactBuffer = m_Device->CreateBuffer(bfd);

	ASPostBuildDesc postBuild;
	postBuild.m_pDstBuffer = compactBuffer;

	m_Context->NewCommandList();
	m_Context->BuildBLAS(blasBuildDescs[0], &postBuild);
	postBuild.m_DstBufferOffset = 8;
	m_Context->BuildBLAS(blasBuildDescs[1], &postBuild);
	m_Context->ExecuteAndWait();

	u64 compactedSizes[2]{};
	auto pValues = compactBuffer->GetMappedDataAsT<u64>();
	compactedSizes[0] = pValues[0];
	compactedSizes[1] = pValues[1];

	BvConsole::Print("Size Diff: %llu | %llu\n", blas[0]->GetSize(), compactedSizes[0]);
	BvConsole::Print("Size Diff: %llu | %llu\n", blas[1]->GetSize(), compactedSizes[1]);

	{
		auto blasDesc = blas[0]->GetDesc();
		blasDesc.m_CompactedSize = compactedSizes[0];
		m_BLAS[0] = m_Device->CreateAccelerationStructure(blasDesc);

		blasDesc = blas[1]->GetDesc();
		blasDesc.m_CompactedSize = compactedSizes[1];
		m_BLAS[1] = m_Device->CreateAccelerationStructure(blasDesc);

		m_Context->NewCommandList();

		AccelerationStructureCopyDesc copy;
		copy.m_CopyMode = AccelerationStructureCopyMode::kCompact;
		copy.m_pSrc = blas[0];
		copy.m_pDst = m_BLAS[0];
		m_Context->CopyBLAS(copy);
		
		copy.m_pSrc = blas[1];
		copy.m_pDst = m_BLAS[1];
		m_Context->CopyBLAS(copy);

		m_Context->ExecuteAndWait();
	}
}


void RayTracing5::CreateTLAS()
{
	RayTracingAccelerationStructureDesc tlasDesc;
	tlasDesc.m_Type = RayTracingAccelerationStructureType::kTopLevel;
	tlasDesc.m_Flags = RayTracingAccelerationStructureFlags::kPreferFastBuild | RayTracingAccelerationStructureFlags::kAllowUpdate;
	tlasDesc.m_TLAS.m_InstanceCount = 3;
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

	m_Instances[2].m_pBLAS = m_BLAS[1];
	m_Instances[2].m_Flags = RayTracingInstanceFlags::kTriangleCullDisable;
	m_Instances[2].m_InstanceMask = 0xFF;
	m_Instances[2].m_InstanceId = 2;
	m_Instances[2].m_ShaderBindingTableIndex = 0;

	m_TLAS->WriteTopLevelInstances(3, m_Instances);

	TLASBuildDesc tlasBuilDesc;
	tlasBuilDesc.m_InstanceCount = 3;
	tlasBuilDesc.m_pInstanceBuffer = m_TLAS->GetTopLevelStagingInstanceBuffer();
	tlasBuilDesc.m_pTLAS = m_TLAS;
	tlasBuilDesc.m_pScratchBuffer = m_ScratchTLAS;

	m_Context->NewCommandList();
	m_Context->BuildTLAS(tlasBuilDesc);
	m_Context->ExecuteAndWait();
}


void RayTracing5::UpdateTLAS()
{
	m_TLAS->WriteTopLevelInstances(3, m_Instances);

	TLASBuildDesc tlasBuilDesc;
	tlasBuilDesc.m_InstanceCount = 3;
	tlasBuilDesc.m_pInstanceBuffer = m_TLAS->GetTopLevelStagingInstanceBuffer();
	tlasBuilDesc.m_pTLAS = m_TLAS;
	tlasBuilDesc.m_pScratchBuffer = m_ScratchTLAS;
	tlasBuilDesc.m_Update = true;

	m_Context->BuildTLAS(tlasBuilDesc);
}


SAMPLE_MAIN(RayTracing5)