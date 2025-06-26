#include "MeshShader.h"
#include "BDeV/Core/Math/BvGeometryGenerator.h"
#include "BDeV/Core/Utils/BvRandom.h"


constexpr const char* g_pASShader =
R"raw(
#version 460
#extension GL_EXT_mesh_shader : require

layout(local_size_x = 64) in;

struct CubeData {
    vec3 position;
    float scale;
};

layout(binding = 0) readonly buffer CubeBuffer {
    CubeData cubes[];
};

layout(binding = 1) buffer VisibleBuffer {
    uint visibleIndices[];
};

layout(push_constant) uniform PushConstants {
    mat4 viewProj;
    vec3 cameraPos;
    float time;
} pc;

shared uint visibleCount;

// Extract frustum planes from view-projection matrix
vec4 getPlane(vec4 row1, vec4 row2, bool add) {
    return add ? (row1 + row2) : (row1 - row2);
}

bool sphereInFrustum(vec3 center, float radius) {
    mat4 m = pc.viewProj;

    vec4 planes[6];
    planes[0] = getPlane(m[3], m[0], false); // Left
    planes[1] = getPlane(m[3], m[0], true);  // Right
    planes[2] = getPlane(m[3], m[1], false); // Bottom
    planes[3] = getPlane(m[3], m[1], true);  // Top
    planes[4] = getPlane(m[3], m[2], false); // Near
    planes[5] = getPlane(m[3], m[2], true);  // Far

    for (int i = 0; i < 6; ++i) {
        vec4 p = planes[i];
        float d = dot(p.xyz, center) + p.w;
        if (d < -radius) return false;
    }
    return true;
}

void main() {
    uint index = gl_GlobalInvocationID.x;

    if (gl_LocalInvocationIndex == 0) visibleCount = 0;
    memoryBarrierShared();
    barrier();

    if (index >= cubes.length()) return;

    CubeData cube = cubes[index];
    float radius = cube.scale * 0.5 * sqrt(3.0); // bounding sphere

    if (sphereInFrustum(cube.position, radius))
	{
        uint writeIndex = atomicAdd(visibleCount, 1);
        visibleIndices[writeIndex] = index;
    }

    barrier();

    if (gl_LocalInvocationID.x == 0)
	{
		if (visibleCount > 0)
		{
			EmitMeshTasksEXT(visibleCount, 1, 1);
		}
    }
}
)raw";
constexpr auto g_ASSize = std::char_traits<char>::length(g_pASShader);

constexpr const char* g_pMSShader =
R"raw(
#version 460
#extension GL_EXT_mesh_shader : require

layout(triangles) out;
layout(max_vertices = 8, max_primitives = 12) out;

struct CubeData {
    vec3 position;
    float scale;
};

layout(binding = 0) readonly buffer CubeBuffer {
    CubeData cubes[];
};

layout(binding = 1) readonly buffer VisibleBuffer {
    uint visibleIndices[];
};

layout(push_constant) uniform PushConstants {
    mat4 viewProj;
    vec3 cameraPos;
    float time;
} pc;

layout(location = 0) out vec4 outColor[];

void main() {
    uint visibleIndex = gl_WorkGroupID.x;
    CubeData cube = cubes[visibleIndices[visibleIndex]];

    // Build a small cube from scratch
    vec3 offsets[8] = vec3[](
        vec3(-1,-1,-1), vec3(-1,1,-1), vec3(1,1,-1), vec3(1,-1,-1),
        vec3(-1,-1, 1), vec3(-1,1, 1), vec3(1,1, 1), vec3(1,-1, 1)
    );

    for (uint i = 0; i < 8; ++i) {
        vec3 pos = cube.position + offsets[i] * cube.scale * 0.5;
        gl_MeshVerticesEXT[i].gl_Position = pc.viewProj * vec4(pos, 1.0);
        outColor[i] = vec4(0.2, 0.6, 1.0, 1.0);
    }

	uint tris[36] = uint[](
	    // Back face (Z-)
	    0, 1, 2,  0, 2, 3,
	
	    // Front face (Z+)
	    4, 6, 5,  4, 7, 6,
	
	    // Left face (X-)
	    0, 5, 1,  0, 4, 5,
	
	    // Right face (X+)
	    3, 2, 6,  3, 6, 7,
	
	    // Top face (Y+)
	    1, 5, 6,  1, 6, 2,
	
	    // Bottom face (Y-)
	    0, 3, 7,  0, 7, 4
	);

    for (uint i = 0; i < 12; ++i) {
        gl_PrimitiveTriangleIndicesEXT[i] = uvec3(tris[i*3], tris[i*3+1], tris[i*3+2]);
    }

    SetMeshOutputsEXT(8, 12);
}
)raw";
constexpr auto g_MSSize = std::char_traits<char>::length(g_pMSShader);

constexpr const char* g_pPSShader =
R"raw(
#version 450

layout (location = 0) in vec4 inColor;

layout (location = 0) out vec4 outColor;

void main()
{
	outColor = inColor;
}
)raw";
constexpr auto g_PSSize = std::char_traits<char>::length(g_pPSShader);


constexpr u32 kCubeCount = 64;


void MeshShader::OnInitialize()
{
	m_AppName = "Mesh Shader";
	CreateRenderTargets();
	CreateShaderResourceLayout();
	CreatePipeline();
	CreateBuffers();

	m_Query = m_Device->CreateQuery(QueryType::kMeshPipelineStatistics);

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
	Store44(m_Camera.GetViewProj(), m_PC.viewProj.m);
	Store3(m_Camera.GetPos(), m_PC.cameraPos.v);

	m_Query->GetResult(m_PSOStats);
}


void MeshShader::OnUpdateUI()
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
	ImGui::Text("Task Shader Invocations: %llu", m_PSOStats.m_TaskOrAmplificationShaderInvocations);
	ImGui::Text("Mesh Shader Invocations: %llu", m_PSOStats.m_MeshShaderInvocations);
	ImGui::Text("Mesh Shader Primitives: %llu", m_PSOStats.m_MeshShaderPrimitives);
	EndDrawDefaultUI();
}


void MeshShader::OnRender()
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
	m_Context->BeginQuery(m_Query);
	m_Context->SetRenderTargets(2, targets);
	m_Context->SetGraphicsPipeline(m_PSO);
	m_Context->SetViewport({ 0.0f, 0.0f, (f32)width, (f32)height, 0.0f, 1.0f });
	m_Context->SetScissor({ 0, 0, width, height });
	m_Context->SetStructuredBuffer(m_SBView1, 0, 0);
	m_Context->SetRWStructuredBuffer(m_SBView2, 0, 1);
	m_Context->SetShaderConstantsT<PushConstants>(m_PC, 2, 0);
	m_Context->DispatchMesh(1);
	OnRenderUI();
	m_Context->EndQuery(m_Query);
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

	m_PSO.Reset();
	m_SRL.Reset();
}


void MeshShader::CreateShaderResourceLayout()
{
	{
		ShaderResourceDesc resourceDescs[] =
		{
			ShaderResourceDesc::AsRWStructuredBuffer(0, ShaderStage::kAmplificationOrTask | ShaderStage::kMesh),
			ShaderResourceDesc::AsRWStructuredBuffer(1, ShaderStage::kAmplificationOrTask | ShaderStage::kMesh)
		};

		ShaderResourceConstantDesc constDesc =
			ShaderResourceConstantDesc::As<PushConstants>(BV_NAME_ID("PC"), 2, ShaderStage::kAmplificationOrTask | ShaderStage::kMesh);

		ShaderResourceSetDesc setDesc{};
		setDesc.m_ResourceCount = 2;
		setDesc.m_pResources = resourceDescs;
		setDesc.m_ConstantCount = 1;
		setDesc.m_pConstants = &constDesc;

		ShaderResourceLayoutDesc layoutDesc{};
		layoutDesc.m_ShaderResourceSetCount = 1;
		layoutDesc.m_pShaderResourceSets = &setDesc;

		m_SRL = m_Device->CreateShaderResourceLayout(layoutDesc);
	}
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

	m_PSO = m_Device->CreateGraphicsPipeline(pipelineDesc);
}


void MeshShader::CreateRenderTargets()
{
	auto w = m_pWindow->GetWidth();
	auto h = m_pWindow->GetHeight();

	TextureDesc desc;
	desc.m_Size = { w, h, 1 };
	desc.m_Format = Format::kD24_UNorm_S8_UInt;
	desc.m_UsageFlags = TextureUsage::kDepthStencilTarget;
	m_Depth = m_Device->CreateTexture(desc, nullptr);

	TextureViewDesc viewDesc;
	viewDesc.m_Format = desc.m_Format;
	viewDesc.m_pTexture = m_Depth;
	m_DepthView = m_Device->CreateTextureView(viewDesc);
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
			cube.v3 = Float3(x, 0.0f, z);
			cube.w = rand.NextF(0.8f, 1.5f);
		}
	}

	desc.m_Size = sizeof(Float4) * kCubeCount;
	desc.m_UsageFlags = BufferUsage::kStructuredBuffer;
	desc.m_ResourceState = ResourceState::kShaderResource;
	data.m_pContext = m_Context;
	data.m_pData = cubes.Data();
	data.m_Size = desc.m_Size;
	m_SB1 = m_Device->CreateBuffer(desc, &data);

	viewDesc.m_pBuffer = m_SB1;
	viewDesc.m_Stride = sizeof(Float4);
	viewDesc.m_ElementCount = kCubeCount;
	m_SBView1 = m_Device->CreateBufferView(viewDesc);

	desc.m_Size = sizeof(u32) * kCubeCount;
	desc.m_UsageFlags = BufferUsage::kRWStructuredBuffer;
	desc.m_ResourceState = ResourceState::kRWResource;
	m_SB2 = m_Device->CreateBuffer(desc);

	viewDesc.m_pBuffer = m_SB2;
	viewDesc.m_Stride = sizeof(u32);
	m_SBView2 = m_Device->CreateBufferView(viewDesc);
}


SAMPLE_MAIN(MeshShader)