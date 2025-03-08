#include "UIOverlay.h"


constexpr const char* pVS =
R"raw(
#version 450

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec4 inColor;

layout (push_constant) uniform PushConstants
{
	vec2 scale;
	vec2 translate;
} pushConstants;

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec4 outColor;

out gl_PerVertex 
{
	vec4 gl_Position;   
};

void main() 
{
	outUV = inUV;
	outColor = inColor;
	gl_Position = vec4(inPos * pushConstants.scale + pushConstants.translate, 0.0, 1.0);
}
)raw";
constexpr u32 vsSize = std::char_traits<char>::length(pVS);


constexpr const char* pPS =
R"raw(
#version 450

layout (binding = 1) uniform texture2D fontTexture;
layout (binding = 2) uniform sampler fontSampler;

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

void main() 
{
	outColor = inColor * texture(sampler2D(fontTexture, fontSampler), inUV);
}
)raw";
constexpr u32 psSize = std::char_traits<char>::length(pPS);


UIOverlay::UIOverlay()
{
}


UIOverlay::~UIOverlay()
{
}


void UIOverlay::Initialize(IBvRenderDevice* pDevice, IBvCommandContext* pContext, Format swapChainFormat, IBvRenderPass* pRenderPass)
{
	BvObjectHandle<IBvShaderCompiler> compiler;
	BvSharedLib renderToolsLib("BvRenderTools.dll");
	typedef bool(*pFNGetShaderCompiler)(IBvShaderCompiler**);
	pFNGetShaderCompiler compilerFn = renderToolsLib.GetProcAddressT<pFNGetShaderCompiler>("CreateSPIRVCompiler");
	compilerFn(&compiler);

	BvObjectHandle<IBvShaderBlob> vsBlob;
	BvObjectHandle<IBvShaderBlob> psBlob;

	ShaderCreateDesc vsDesc;
	vsDesc.m_ShaderStage = ShaderStage::kVertex;
	vsDesc.m_ShaderLanguage = ShaderLanguage::kGLSL;
	vsDesc.m_ShaderTarget = ShaderTarget::kSPIRV_1_0;
	vsDesc.m_pSourceCode = pVS;
	vsDesc.m_SourceCodeSize = vsSize;
	compiler->Compile(vsDesc, &vsBlob);

	ShaderCreateDesc psDesc;
	psDesc.m_ShaderStage = ShaderStage::kPixelOrFragment;
	psDesc.m_ShaderLanguage = ShaderLanguage::kGLSL;
	psDesc.m_ShaderTarget = ShaderTarget::kSPIRV_1_0;
	psDesc.m_pSourceCode = pPS;
	psDesc.m_SourceCodeSize = psSize;
	compiler->Compile(psDesc, &psBlob);

	BvObjectHandle<IBvShader> vs;
	vsDesc.m_pByteCode = (const u8*)vsBlob->GetBufferPointer();
	vsDesc.m_ByteCodeSize = vsBlob->GetBufferSize();
	m_Device->CreateShader(vsDesc, &vs);

	BvObjectHandle<IBvShader> ps;
	psDesc.m_pByteCode = (const u8*)psBlob->GetBufferPointer();
	psDesc.m_ByteCodeSize = psBlob->GetBufferSize();
	m_Device->CreateShader(psDesc, &ps);

	m_Device = pDevice;

	// Init ImGui
	ImGui::CreateContext();

	// Color scheme
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.0f, 0.0f, 0.0f, 0.1f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.8f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
	style.Colors[ImGuiCol_Button] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);

	u8* pFontData;
	i32 w, h;
	
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->GetTexDataAsRGBA32(&pFontData, &w, &h);
	u64 uploadSize = w * h * 4;

	TextureDesc texDesc;
	texDesc.m_Size = { u32(w), u32(h), 1 };
	texDesc.m_Format = Format::kRGBA8_UNorm;
	texDesc.m_UsageFlags = TextureUsage::kShaderResource;
	texDesc.m_ResourceState = ResourceState::kShaderResource;

	SubresourceData texSubRes;
	texSubRes.m_pData = pFontData;
	texSubRes.m_RowPitch = w * 4;
	texSubRes.m_SlicePitch = texSubRes.m_RowPitch * h;

	TextureInitData texData;
	texData.m_SubresourceCount = 1;
	texData.m_pSubresources = &texSubRes;
	texData.m_pContext = pContext;
	m_Device->CreateTexture(texDesc, &texData, &m_Texture);
	
	TextureViewDesc viewDesc;
	viewDesc.m_Format = texDesc.m_Format;
	viewDesc.m_pTexture = m_Texture;
	viewDesc.m_ViewType = TextureViewType::kTexture2D;
	viewDesc.m_SubresourceDesc.mipCount = 1;
	viewDesc.m_SubresourceDesc.layerCount = 1;
	m_Device->CreateTextureView(viewDesc, &m_TextureView);

	SamplerDesc samDesc;
	samDesc.m_AddressModeU = samDesc.m_AddressModeV = samDesc.m_AddressModeW = AddressMode::kClamp;
	m_Device->CreateSampler(samDesc, &m_Sampler);

	ShaderResourceDesc resources[] =
	{
		ShaderResourceDesc::AsTexture(1, ShaderStage::kPixelOrFragment),
		ShaderResourceDesc::AsSampler(2, ShaderStage::kPixelOrFragment)
	};

	ShaderResourceConstantDesc resConstant;
	resConstant.m_ShaderStages = ShaderStage::kVertex;
	resConstant.m_Size = sizeof(PushConstBlock);
	
	ShaderResourceSetDesc setDesc{};
	setDesc.m_ResourceCount = 2;
	setDesc.m_pResources = resources;

	ShaderResourceLayoutDesc srlDesc;
	srlDesc.m_ShaderResourceSetCount = 1;
	srlDesc.m_pShaderResourceSets = &setDesc;
	srlDesc.m_pShaderResourceConstant = &resConstant;
	m_Device->CreateShaderResourceLayout(srlDesc, &m_SRL);

	GraphicsPipelineStateDesc psoDesc;
	psoDesc.m_Shaders[0] = vs;
	psoDesc.m_Shaders[1] = ps;
	psoDesc.m_InputAssemblyStateDesc.m_Topology = Topology::kTriangleList;
	auto& blendState = psoDesc.m_BlendStateDesc.m_BlendAttachments[0];
	blendState.m_BlendEnable = true;
	blendState.m_SrcBlend = BlendFactor::kSrcAlpha;
	blendState.m_DstBlend = BlendFactor::kInvkSrcAlpha;
	blendState.m_SrcBlendAlpha = BlendFactor::kInvkSrcAlpha;
	blendState.m_DstBlendAlpha = BlendFactor::kZero;
	psoDesc.m_pShaderResourceLayout = m_SRL;
	psoDesc.m_RenderTargetFormats[0] = Format::kRGBA8_UNorm;
	psoDesc.m_pRenderPass = pRenderPass;
	
	VertexInputDesc inputDescs[3];
	inputDescs[0].m_Format = Format::kRG32_Float;
	inputDescs[1].m_Format = Format::kRG32_Float;
	inputDescs[1].m_Offset = VertexInputDesc::kAutoOffset;
	inputDescs[2].m_Format = Format::kRGBA8_UNorm;
	inputDescs[2].m_Offset = VertexInputDesc::kAutoOffset;
	psoDesc.m_VertexInputDescCount = 3;
	psoDesc.m_pVertexInputDescs = inputDescs;
	m_Device->CreateGraphicsPipeline(psoDesc, &m_Pipeline);
}


void UIOverlay::Update(u32 w, u32 h)
{
	if (m_Width != w || m_Height != h)
	{
		m_Width = w;
		m_Height = h;

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)(m_Width), (float)(m_Height));
	}

	ImDrawData* imDrawData = ImGui::GetDrawData();
	if (!imDrawData)
	{
		return;
	};

	// Note: Alignment is done inside buffer creation
	u64 vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
	u64 indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

	// Update buffers only if vertex or index count has been changed compared to current buffer size
	if ((vertexBufferSize == 0) || (indexBufferSize == 0))
	{
		return;
	}

	// Vertex buffer
	if ((!m_VB) || (m_VertexCount < imDrawData->TotalVtxCount))
	{
		m_VB.Reset();
		m_VBView.Reset();
		CreateVB(vertexBufferSize, m_VertexCount);
		m_VertexCount = imDrawData->TotalVtxCount;
	}

	// Index buffer
	if ((!m_IB) || (m_IndexCount < imDrawData->TotalIdxCount))
	{
		m_IB.Reset();
		m_IBView.Reset();
		CreateIB(indexBufferSize, m_IndexCount);
		m_IndexCount = imDrawData->TotalIdxCount;
	}

	// Upload data
	ImDrawVert* pVtxDst = m_VB->GetMappedDataAsT<ImDrawVert>();
	ImDrawIdx* pIdxDst = m_IB->GetMappedDataAsT<ImDrawIdx>();

	for (int n = 0; n < imDrawData->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = imDrawData->CmdLists[n];
		memcpy(pVtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		memcpy(pIdxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
		pVtxDst += cmd_list->VtxBuffer.Size;
		pIdxDst += cmd_list->IdxBuffer.Size;
	}

	// Flush to make writes visible to GPU
	m_VB->Flush();
	m_IB->Flush();
}


void UIOverlay::Render(IBvCommandContext* pContext)
{
	ImDrawData* pImDrawData = ImGui::GetDrawData();
	int32_t vertexOffset = 0;
	int32_t indexOffset = 0;

	if ((!pImDrawData) || (pImDrawData->CmdListsCount == 0)) {
		return;
	}

	ImGuiIO& io = ImGui::GetIO();

	pContext->SetGraphicsPipeline(m_Pipeline);
	pContext->SetTexture(m_TextureView, 0, 0);
	pContext->SetSampler(m_Sampler, 0, 1);

	m_PC.scale = Float2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
	m_PC.translate = Float2(-1.0f, -1.0f);
	pContext->SetShaderConstantsT<PushConstBlock>(m_PC);

	u64 offsets[1] = { 0 };
	pContext->SetVertexBufferView(m_VBView);
	pContext->SetIndexBufferView(m_IBView, IndexFormat::kU16);

	for (auto i = 0; i < pImDrawData->CmdListsCount; i++)
	{
		const ImDrawList* pDrawList = pImDrawData->CmdLists[i];
		for (auto j = 0; j < pDrawList->CmdBuffer.Size; j++)
		{
			const ImDrawCmd* pcmd = &pDrawList->CmdBuffer[j];
			Rect scissorRect;
			scissorRect.x = std::max((i32)(pcmd->ClipRect.x), 0);
			scissorRect.y = std::max((i32)(pcmd->ClipRect.y), 0);
			scissorRect.width = (u32)(pcmd->ClipRect.z - pcmd->ClipRect.x);
			scissorRect.height = (u32)(pcmd->ClipRect.w - pcmd->ClipRect.y);
			pContext->SetScissor(scissorRect);
			pContext->DrawIndexed(pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
			indexOffset += pcmd->ElemCount;
		}
		vertexOffset += pDrawList->VtxBuffer.Size;
	}
}


void UIOverlay::Shutdown()
{
	if (ImGui::GetCurrentContext())
	{
		ImGui::DestroyContext();
	}
}


void UIOverlay::CreateVB(u64 size, u32 count)
{
	BufferDesc desc;
	desc.m_CreateFlags = BufferCreateFlags::kCreateMapped;
	desc.m_Size = size;
	desc.m_UsageFlags = BufferUsage::kVertexBuffer;
	m_Device->CreateBuffer(desc, nullptr, &m_VB);

	BufferViewDesc viewDesc;
	viewDesc.m_pBuffer = m_VB;
	viewDesc.m_ElementCount = count;
	viewDesc.m_Stride = sizeof(ImDrawVert);
	m_Device->CreateBufferView(viewDesc, &m_VBView);
}


void UIOverlay::CreateIB(u64 size, u32 count)
{
	BufferDesc desc;
	desc.m_CreateFlags = BufferCreateFlags::kCreateMapped;
	desc.m_Size = size;
	desc.m_UsageFlags = BufferUsage::kIndexBuffer;
	m_Device->CreateBuffer(desc, nullptr, &m_IB);

	BufferViewDesc viewDesc;
	viewDesc.m_pBuffer = m_IB;
	viewDesc.m_ElementCount = count;
	viewDesc.m_Stride = sizeof(ImDrawIdx);
	m_Device->CreateBufferView(viewDesc, &m_IBView);
}