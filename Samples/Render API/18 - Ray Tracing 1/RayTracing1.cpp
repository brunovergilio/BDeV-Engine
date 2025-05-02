#include "RayTracing1.h"


constexpr const char* g_pRGenShader =
R"raw(
#version 460
#extension GL_EXT_ray_tracing : require

layout(set = 0, binding = 0, rgba32f) uniform image2D outputImage;
layout(set = 0, binding = 1) uniform accelerationStructureEXT topLevelAS;

layout(location = 0) rayPayloadEXT vec4 payload;

void main()
{
    const uvec2 launchID = gl_LaunchIDEXT.xy;
    const uvec2 launchSize = gl_LaunchSizeEXT.xy;

    vec2 uv = vec2(launchID) / vec2(launchSize);
    payload = vec4(uv, 0.0, 1.0); // simple gradient
    traceRayEXT(
        topLevelAS,                     // acceleration structure
        gl_RayFlagsNoneEXT,            // ray flags
        0xFF,                           // cull mask
        0,                              // sbt record offset
        0,                              // sbt record stride
        0,                              // miss index
        vec3(0.0, 0.0, 0.0),            // origin
        0.0,                            // min t
        vec3(0.0, 0.0, 1.0),            // direction
        10000.0,                        // max t
        0                               // payload location
    );

    imageStore(outputImage, ivec2(launchID), payload);
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

void main()
{
    payload = vec4(pc.color, 1.0);
}
)raw";
constexpr auto g_RMissSize = std::char_traits<char>::length(g_pRMissShader);


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
	ImGui::ColorEdit3("Background", m_BackColor.v);
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
}


void RayTracing1::CreateShaderResourceLayout()
{
	{
		ShaderResourceDesc descs[2];
		descs[0] = ShaderResourceDesc::AsRWTexture(0, ShaderStage::kRayGen);
		descs[1] = ShaderResourceDesc::AsAccelerationStructure(1, ShaderStage::kRayGen);

		ShaderResourceConstantDesc constDesc = ShaderResourceConstantDesc::As<Float3>(BV_NAME_ID("PC"), 2, ShaderStage::kMiss);

		ShaderResourceSetDesc setDesc{};
		setDesc.m_ResourceCount = 2;
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


void RayTracing1::CreatePipeline()
{
	{
		auto rgen = CompileShader(g_pRGenShader, g_RGenSize, ShaderStage::kRayGen);
		auto rmis = CompileShader(g_pRMissShader, g_RMissSize, ShaderStage::kMiss);

		IBvShader* ppShaders[] =
		{
			rgen, rmis
		};

		ShaderGroupDesc groupDescs[2];
		groupDescs[0].m_Type = ShaderGroupType::kGeneral;
		groupDescs[0].m_General = 0;

		groupDescs[1].m_Type = ShaderGroupType::kGeneral;
		groupDescs[1].m_General = 1;

		RayTracingPipelineStateDesc pipelineDesc;
		pipelineDesc.m_ShaderGroupCount = 2;
		pipelineDesc.m_pShaderGroupDescs = groupDescs;
		pipelineDesc.m_ShaderCount = 2;
		pipelineDesc.m_ppShaders = ppShaders;
		pipelineDesc.m_pShaderResourceLayout = m_RaySRL;
		pipelineDesc.m_MaxPipelineRayRecursionDepth = 1;

		m_RayPSO = m_Device->CreateRayTracingPipeline(pipelineDesc);
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


void RayTracing1::CreateResources()
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

	RayTracingAccelerationStructureDesc asDesc;
	asDesc.m_Type = RayTracingAccelerationStructureType::kTopLevel;
	asDesc.m_Flags = RayTracingAccelerationStructureFlags::kPreferFastTrace;
	asDesc.m_TLAS.m_InstanceCount = 0;
	m_AS = m_Device->CreateAccelerationStructure(asDesc);
	
	BufferDesc buffDesc;
	buffDesc.m_Size = m_AS->GetBuildSizes().m_Build;
	buffDesc.m_UsageFlags = BufferUsage::kRayTracing;
	m_ScratchBuffer = m_Device->CreateBuffer(buffDesc);

	TLASInstanceDesc dummyInstance;
	m_AS->WriteTopLevelInstances(1, &dummyInstance);
	
	auto pInstanceBuffer = m_AS->GetTopLevelStagingInstanceBuffer();
	TLASBuildDesc build;
	build.m_InstanceCount = 0;
	build.m_pInstanceBuffer = pInstanceBuffer;
	build.m_pTLAS = m_AS;
	build.m_pScratchBuffer = m_ScratchBuffer;
	m_Context->NewCommandList();
	m_Context->BuildTLAS(build);
	m_Context->Execute();
	m_Context->WaitForGPU();

	ShaderBindingTableDesc sbtDesc;
	sbtDesc.m_pPSO = m_RayPSO;
	m_SBT = m_Device->CreateShaderBindingTable(sbtDesc, m_Context);
}


SAMPLE_MAIN(RayTracing1)