#include "MSAARenderPass.h"
#include "BDeV/Core/Math/BvGeometryGenerator.h"
#include "BDeV/Core/Utils/BvRandom.h"


struct Vertex
{
	Float3 pos;
	Float4 color;
};


constexpr const char* g_pVSShader =
R"raw(
#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

layout (binding = 0) uniform UBO 
{
	mat4 wvp;
} ubo;

void main() 
{
	outColor = inColor;
	gl_Position = ubo.wvp * vec4(inPos.xyz, 1.0);
}
)raw";
constexpr auto g_VSSize = std::char_traits<char>::length(g_pVSShader);


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


void MSAARenderPass::OnInitialize()
{
	m_AppName = "MSAA (Render Pass)";
	CreateBuffers();
	CreateRenderTargets();
	CreateRenderPass();
	CreateRenderPassMSAA();
	CreateShaderResourceLayout();
	CreatePipeline();
}


void MSAARenderPass::OnInitializeUI()
{
	m_Overlay.SetupPipeline(m_RenderPass);
	m_Overlay.SetupPipeline(m_RenderPassMSAA, 0, 8);
}


void MSAARenderPass::OnUpdate()
{
	auto width = m_pWindow->GetWidth();
	auto height = m_pWindow->GetHeight();
	static f32 speed = 0.1f;
	static f32 angleX = 0.0f;
	static f32 angleY = 0.0f;
	static f32 angleZ = 0.0f;
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
	Store44(MatrixRotationX(angleX) * MatrixRotationY(angleY) * MatrixRotationZ(angleZ)
		* MatrixLookAtLH(VectorSet(0.0f, 0.0f, -2.0f, 1.0f), VectorSet(0.0f, 0.0f, 1.0f, 1.0f), VectorSet(0.0f, 1.0f, 0.0f, 1.0f))
		* MatrixPerspectiveLH_DX(0.1f, 100.0f, float(width) / float(height), kPiDiv4), m_pWVP->m);
}


void MSAARenderPass::OnUpdateUI()
{
	BeginDrawDefaultUI();
	ImGui::Checkbox("MSAA", &m_UseMSAA);
	EndDrawDefaultUI();
}


void MSAARenderPass::OnRender()
{
	auto width = m_pWindow->GetWidth();
	auto height = m_pWindow->GetHeight();

	RenderPassTargetDesc targets[4];
	if (m_UseMSAA)
	{
		targets[0] = RenderPassTargetDesc(m_ColorViewMS, 0.1f, 0.1f, 0.3f);
		targets[1] = RenderPassTargetDesc(m_DepthViewMS, 1.0f, 0);
		targets[2] = RenderPassTargetDesc(m_SwapChain->GetCurrentTextureView(), 0.1f, 0.1f, 0.3f);
		targets[3] = RenderPassTargetDesc(m_DepthView, 1.0f, 0);
	}
	else
	{
		targets[0] = RenderPassTargetDesc(m_SwapChain->GetCurrentTextureView(), 0.1f, 0.1f, 0.3f);
		targets[1] = RenderPassTargetDesc(m_DepthView, 1.0f, 0);
	}

	m_Context->NewCommandList();
	if (m_UseMSAA)
	{
		m_Context->BeginRenderPass(m_RenderPassMSAA, 4, targets);
	}
	else
	{
		m_Context->BeginRenderPass(m_RenderPass, 2, targets);
	}
	m_Context->SetGraphicsPipeline(m_UseMSAA ? m_PSOMSAA : m_PSO);
	m_Context->SetViewport({ 0.0f, 0.0f, (f32)width, (f32)height, 0.0f, 1.0f });
	m_Context->SetScissor({ 0, 0, width, height });
	m_Context->SetConstantBuffer(m_UBView, 0, 0);
	m_Context->SetVertexBufferView(m_VB, sizeof(Vertex));
	m_Context->SetIndexBufferView(m_IB, IndexFormat::kU32);
	m_Context->DrawIndexed(36);
	OnRenderUI();
	m_Context->EndRenderPass();
	m_Context->Execute();

	m_SwapChain->Present(false);

	m_Context->FlushFrame();
}


void MSAARenderPass::OnRenderUI()
{
	m_Overlay.Render(m_UseMSAA);
}


void MSAARenderPass::OnShutdown()
{
	m_UB.Reset();
	m_VB.Reset();
	m_IB.Reset();
	m_UBView.Reset();
	m_Depth.Reset();
	m_DepthView.Reset();
	m_ColorViewMS.Reset();
	m_ColorMS.Reset();
	m_DepthViewMS.Reset();
	m_DepthMS.Reset();
	m_PSOMSAA.Reset();
	m_PSO.Reset();
	m_RenderPass.Reset();
	m_RenderPassMSAA.Reset();
	m_SRL.Reset();
}


void MSAARenderPass::CreateShaderResourceLayout()
{
	ShaderResourceDesc resourceDesc = ShaderResourceDesc::AsConstantBuffer(0, ShaderStage::kVertex);

	ShaderResourceSetDesc setDesc{};
	setDesc.m_ResourceCount = 1;
	setDesc.m_pResources = &resourceDesc;

	ShaderResourceLayoutDesc layoutDesc{};
	layoutDesc.m_ShaderResourceSetCount = 1;
	layoutDesc.m_pShaderResourceSets = &setDesc;

	m_SRL = m_Device->CreateShaderResourceLayout(layoutDesc);
}


void MSAARenderPass::CreatePipeline()
{
	auto vs = CompileShader(g_pVSShader, g_VSSize, ShaderStage::kVertex);
	auto ps = CompileShader(g_pPSShader, g_PSSize, ShaderStage::kPixelOrFragment);
	GraphicsPipelineStateDesc pipelineDesc;
	pipelineDesc.m_Shaders[0] = vs;
	pipelineDesc.m_Shaders[1] = ps;
	pipelineDesc.m_DepthStencilDesc.m_DepthTestEnable = true;
	pipelineDesc.m_DepthStencilDesc.m_DepthWriteEnable = true;
	pipelineDesc.m_DepthStencilDesc.m_DepthOp = CompareOp::kLessEqual;
	pipelineDesc.m_pShaderResourceLayout = m_SRL;
	pipelineDesc.m_pRenderPass = m_RenderPass;

	VertexInputDesc inputDescs[2];
	inputDescs[0].m_Format = Format::kRGB32_Float;
	inputDescs[1].m_Format = Format::kRGBA32_Float;

	pipelineDesc.m_VertexInputDescCount = 2;
	pipelineDesc.m_pVertexInputDescs = inputDescs;

	m_PSO = m_Device->CreateGraphicsPipeline(pipelineDesc);
	
	pipelineDesc.m_SampleCount = 8;
	pipelineDesc.m_pRenderPass = m_RenderPassMSAA;
	m_PSOMSAA = m_Device->CreateGraphicsPipeline(pipelineDesc);
}


void MSAARenderPass::CreateBuffers()
{
	BvGeometryGenerator gen;
	gen.GenerateBox();
	auto& data = gen.GetData();
	BvVector<Vertex> vertices(data.m_Vertices.Size());
	BvPCG rand;

	for (auto i = 0u; i < vertices.Size(); ++i)
	{
		vertices[i].pos = data.m_Vertices[i].m_Position;
		vertices[i].color = Float4(rand.NextF<f32>(), rand.NextF<f32>(), rand.NextF<f32>(), 1.0f);
	}

	BufferDesc bufferDesc;
	BufferInitData bufferData;
	BufferViewDesc viewDesc;

	bufferDesc.m_Size = sizeof(Vertex) * vertices.Size();
	bufferDesc.m_UsageFlags = BufferUsage::kVertexBuffer;
	bufferData.m_pContext = m_Context;
	bufferData.m_pData = vertices.Data();
	bufferData.m_Size = bufferDesc.m_Size;
	m_VB = m_Device->CreateBuffer(bufferDesc, &bufferData);

	bufferDesc.m_Size = sizeof(u32) * data.m_Indices.Size();
	bufferDesc.m_UsageFlags = BufferUsage::kIndexBuffer;
	bufferData.m_pContext = m_Context;
	bufferData.m_pData = data.m_Indices.Data();
	bufferData.m_Size = bufferDesc.m_Size;
	m_IB = m_Device->CreateBuffer(bufferDesc, &bufferData);

	bufferDesc.m_Size = sizeof(Float44);
	bufferDesc.m_UsageFlags = BufferUsage::kConstantBuffer;
	bufferDesc.m_MemoryType = MemoryType::kUpload;
	bufferDesc.m_CreateFlags = BufferCreateFlags::kCreateMapped;
	m_UB = m_Device->CreateBuffer(bufferDesc, &bufferData);

	viewDesc.m_pBuffer = m_UB;
	viewDesc.m_Stride = sizeof(Float44);
	viewDesc.m_ElementCount = 1;
	m_UBView = m_Device->CreateBufferView(viewDesc);

	m_pWVP = m_UB->GetMappedDataAsT<Float44>();
}


void MSAARenderPass::CreateRenderTargets()
{
	auto w = m_pWindow->GetWidth();
	auto h = m_pWindow->GetHeight();

	TextureDesc desc;
	desc.m_Size = { w, h, 1 };
	desc.m_Format = Format::kD24_UNorm_S8_UInt;
	desc.m_UsageFlags = TextureUsage::kDepthStencilTarget;
	m_Depth = m_Device->CreateTexture(desc, nullptr);

	desc.m_SampleCount = 8;
	m_DepthMS = m_Device->CreateTexture(desc, nullptr);

	TextureViewDesc viewDesc;
	viewDesc.m_Format = desc.m_Format;
	viewDesc.m_pTexture = m_Depth;
	m_DepthView = m_Device->CreateTextureView(viewDesc);

	viewDesc.m_pTexture = m_DepthMS;
	m_DepthViewMS = m_Device->CreateTextureView(viewDesc);

	desc.m_Format = m_SwapChain->GetDesc().m_Format;
	desc.m_UsageFlags = TextureUsage::kRenderTarget;
	m_ColorMS = m_Device->CreateTexture(desc, nullptr);

	viewDesc.m_Format = desc.m_Format;
	viewDesc.m_pTexture = m_ColorMS;
	m_ColorViewMS = m_Device->CreateTextureView(viewDesc);
}


void MSAARenderPass::CreateRenderPass()
{
	RenderPassAttachment attachments[2];
	attachments[0].m_Format = m_SwapChain->GetDesc().m_Format;
	attachments[0].m_StateAfter = ResourceState::kPresent;

	attachments[1].m_Format = m_Depth->GetDesc().m_Format;
	attachments[1].m_StateAfter = ResourceState::kDepthStencilRead;

	AttachmentRef mainRefs[2];
	mainRefs[0].m_Index = 0;
	mainRefs[0].m_ResourceState = ResourceState::kRenderTarget;
	mainRefs[1].m_Index = 1;
	mainRefs[1].m_ResourceState = ResourceState::kDepthStencilWrite;

	SubpassDesc subpass;
	subpass.m_ColorAttachmentCount = 1;
	subpass.m_pColorAttachments = &mainRefs[0];
	subpass.m_pDepthStencilAttachment = mainRefs + 1;

	RenderPassDesc rpDesc;
	rpDesc.m_AttachmentCount = 2;
	rpDesc.m_pAttachments = attachments;
	rpDesc.m_SubpassCount = 1;
	rpDesc.m_pSubpasses = &subpass;

	m_RenderPass = m_Device->CreateRenderPass(rpDesc);
}


void MSAARenderPass::CreateRenderPassMSAA()
{
	RenderPassAttachment attachments[4];
	attachments[0].m_Format = m_ColorMS->GetDesc().m_Format;
	attachments[0].m_StoreOp = StoreOp::kDontCare;
	attachments[0].m_StateAfter = ResourceState::kRenderTarget;
	attachments[0].m_SampleCount = 8;

	attachments[1].m_Format = m_DepthMS->GetDesc().m_Format;
	attachments[1].m_StoreOp = StoreOp::kDontCare;
	attachments[1].m_StateAfter = ResourceState::kDepthStencilWrite;
	attachments[1].m_SampleCount = 8;

	attachments[2].m_Format = m_SwapChain->GetDesc().m_Format;
	attachments[2].m_StateAfter = ResourceState::kPresent;

	attachments[3].m_Format = m_Depth->GetDesc().m_Format;
	attachments[3].m_StateAfter = ResourceState::kDepthStencilRead;

	AttachmentRef msaaRefs[2];
	msaaRefs[0].m_Index = 0;
	msaaRefs[0].m_ResourceState = ResourceState::kRenderTarget;
	msaaRefs[1].m_Index = 1;
	msaaRefs[1].m_ResourceState = ResourceState::kDepthStencilWrite;

	AttachmentRef colorResolveRef;
	colorResolveRef.m_Index = 2;
	colorResolveRef.m_ResourceState = ResourceState::kRenderTarget;

	AttachmentRef depthResolveRef;
	depthResolveRef.m_Index = 3;
	depthResolveRef.m_ResourceState = ResourceState::kDepthStencilWrite;
	depthResolveRef.m_ResolveMode = ResolveMode::kMin;

	SubpassDesc subpass;
	subpass.m_ColorAttachmentCount = 1;
	subpass.m_pColorAttachments = &msaaRefs[0];
	subpass.m_pDepthStencilAttachment = msaaRefs + 1;
	subpass.m_pResolveAttachments = &colorResolveRef;
	subpass.m_pDepthStencilResolveAttachment = &depthResolveRef;

	RenderPassDesc rpDesc;
	rpDesc.m_AttachmentCount = 4;
	rpDesc.m_pAttachments = attachments;
	rpDesc.m_SubpassCount = 1;
	rpDesc.m_pSubpasses = &subpass;

	m_RenderPassMSAA = m_Device->CreateRenderPass(rpDesc);
}


SAMPLE_MAIN(MSAARenderPass)