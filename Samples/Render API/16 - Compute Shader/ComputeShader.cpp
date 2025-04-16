#include "ComputeShader.h"


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

constexpr const char* g_pCSShader =
R"raw(
#version 450

layout (local_size_x = 8, local_size_y = 8) in;

layout (rgba8, binding = 0) uniform writeonly image2D outImage;
layout (push_constant) uniform PushConstants
{
    vec2 resolution;
    float time;
} pc;

void main() {
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    if (pixel.x >= int(pc.resolution.x) || pixel.y >= int(pc.resolution.y))
	{
		return;
	}

    vec2 uv = pixel / pc.resolution;
    vec3 color = 0.5 + 0.5 * cos(pc.time + uv.xyx * 6.2831 + vec3(0, 2, 4));

    imageStore(outImage, pixel, vec4(color, 1.0));
}
)raw";
constexpr auto g_CSSize = std::char_traits<char>::length(g_pCSShader);


void ComputeShader::OnInitialize()
{
	m_AppName = "Compute Shader";
	CreateShaderResourceLayout();
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
}


void ComputeShader::OnRender()
{
	auto width = m_pWindow->GetWidth();
	auto height = m_pWindow->GetHeight();

	m_Context->NewCommandList();

	ResourceBarrierDesc barriers[2];
	barriers[0].m_pTexture = m_RWTex;
	barriers[0].m_SrcLayout = ResourceState::kPixelShaderResource;
	barriers[0].m_DstLayout = ResourceState::kRWResource;
	m_Context->ResourceBarrier(1, barriers);
	DoCompute(width, height);

	//barriers[1].m_pTexture = m_SwapChain->GetCurrentTextureView()->GetDesc().m_pTexture;
	//barriers[1].m_SrcLayout = ResourceState::kPresent;
	//barriers[1].m_DstLayout = ResourceState::kTransferDst;
	//m_Context->CopyTexture(m_RWTex, barriers[1].m_pTexture);

	barriers[0].m_SrcLayout = ResourceState::kRWResource;
	barriers[0].m_DstLayout = ResourceState::kPixelShaderResource;

	//barriers[1].m_SrcLayout = ResourceState::kTransferDst;
	//barriers[1].m_DstLayout = ResourceState::kPresent;
	m_Context->ResourceBarrier(1, barriers);

	RenderTargetDesc targets[] =
	{
		RenderTargetDesc::AsSwapChain(m_SwapChain->GetCurrentTextureView(), { 0.1f, 0.1f, 0.3f }),
	};

	m_Context->SetRenderTargets(1, targets);
	m_Context->SetGraphicsPipeline(m_PSO);
	m_Context->SetViewport({ 0.0f, 0.0f, (f32)width, (f32)height, 0.0f, 1.0f });
	m_Context->SetScissor({ 0, 0, width, height });
	m_Context->SetTexture(m_RWTexView, 0, 0);
	m_Context->SetSampler(m_Sampler, 0, 1);
	m_Context->Draw(6);
	OnRenderUI();

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
}


void ComputeShader::CreateShaderResourceLayout()
{
	{
		ShaderResourceConstantDesc constantDesc{ BV_NAME_ID("PC"), 1, sizeof(PC), ShaderStage::kCompute };
		ShaderResourceDesc resourceDesc = ShaderResourceDesc::AsRWTexture(0, ShaderStage::kCompute);
		ShaderResourceSetDesc setDesc{};
		setDesc.m_ResourceCount = 1;
		setDesc.m_pResources = &resourceDesc;
		setDesc.m_ConstantCount = 1;
		setDesc.m_pConstants = &constantDesc;

		ShaderResourceLayoutDesc layoutDesc{};
		layoutDesc.m_ShaderResourceSetCount = 1;
		layoutDesc.m_pShaderResourceSets = &setDesc;
		m_SRLCompute = m_Device->CreateShaderResourceLayout(layoutDesc);
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


void ComputeShader::CreatePipeline()
{
	{
		auto cs = CompileShader(g_pCSShader, g_CSSize, ShaderStage::kCompute);
		ComputePipelineStateDesc pipelineDesc;
		pipelineDesc.m_pShader = cs;
		pipelineDesc.m_pShaderResourceLayout = m_SRLCompute;

		m_PSOCompute = m_Device->CreateComputePipeline(pipelineDesc);
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


void ComputeShader::CreateTextures()
{
	auto w = m_pWindow->GetWidth();
	auto h = m_pWindow->GetHeight();

	TextureDesc desc;
	desc.m_Size = { w, h, 1 };
	desc.m_Format = GetFormatInfo(m_SwapChain->GetDesc().m_Format).m_SRGBOrLinearVariant;
	desc.m_UsageFlags = TextureUsage::kUnorderedAccess | TextureUsage::kShaderResource;
	desc.m_ResourceState = ResourceState::kPixelShaderResource;
	TextureInitData initData{};
	initData.m_pContext = m_Context;
	m_RWTex = m_Device->CreateTexture(desc, &initData);

	TextureViewDesc viewDesc;
	viewDesc.m_Format = desc.m_Format;
	viewDesc.m_pTexture = m_RWTex;
	m_RWTexView = m_Device->CreateTextureView(viewDesc);

	m_Sampler = m_Device->CreateSampler(SamplerDesc());
}


SAMPLE_MAIN(ComputeShader)