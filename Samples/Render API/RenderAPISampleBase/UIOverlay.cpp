#include "UIOverlay.h"


constexpr const char* pVS =
R"raw(
#version 450

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec4 inColor;

layout (push_constant) uniform PushConstants
{
	//vec2 scale;
	//vec2 translate;
	mat4 wvp;
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
	//gl_Position = vec4(inPos * pushConstants.scale + pushConstants.translate, 0.0, 1.0);
	gl_Position = pushConstants.wvp * vec4(inPos, 0.0, 1.0);
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


void UIOverlay::Initialize(IBvRenderDevice* pDevice, IBvCommandContext* pContext)
{
	m_Device = pDevice;
	m_Context = pContext;

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
	m_Texture.Set(m_Device->CreateTexture(texDesc, &texData));
	
	TextureViewDesc viewDesc;
	viewDesc.m_Format = texDesc.m_Format;
	viewDesc.m_pTexture = m_Texture;
	viewDesc.m_ViewType = TextureViewType::kTexture2D;
	viewDesc.m_SubresourceDesc.mipCount = 1;
	viewDesc.m_SubresourceDesc.layerCount = 1;
	m_TextureView.Set(m_Device->CreateTextureView(viewDesc));

	SamplerDesc samDesc;
	samDesc.m_AddressModeU = samDesc.m_AddressModeV = samDesc.m_AddressModeW = AddressMode::kClamp;
	m_Sampler.Set(m_Device->CreateSampler(samDesc));

	ShaderResourceDesc resources[] =
	{
		ShaderResourceDesc::AsTexture(1, ShaderStage::kPixelOrFragment),
		ShaderResourceDesc::AsSampler(2, ShaderStage::kPixelOrFragment)
	};

	ShaderResourceConstantDesc resConstant{};
	resConstant.m_ShaderStages = ShaderStage::kVertex;
	resConstant.m_Size = sizeof(PushConstBlock);
	
	ShaderResourceSetDesc setDesc{};
	setDesc.m_ResourceCount = 2;
	setDesc.m_pResources = resources;
	setDesc.m_ConstantCount = 1;
	setDesc.m_pConstants = &resConstant;

	ShaderResourceLayoutDesc srlDesc;
	srlDesc.m_ShaderResourceSetCount = 1;
	srlDesc.m_pShaderResourceSets = &setDesc;
	m_SRL.Set(m_Device->CreateShaderResourceLayout(srlDesc));
}


void UIOverlay::SetupPipeline(Format swapChainFormat, Format depthFormat, IBvRenderPass* pRenderPass)
{
	BvRCRef<IBvShaderCompiler> compiler;
	BvSharedLib renderToolsLib("BvRenderTools.dll");
	typedef IBvShaderCompiler*(*pFNGetShaderCompiler)();
	pFNGetShaderCompiler compilerFn = renderToolsLib.GetProcAddressT<pFNGetShaderCompiler>("CreateSPIRVCompiler");
	compiler.Set(compilerFn());

	BvRCRef<IBvShaderBlob> vsBlob;
	BvRCRef<IBvShaderBlob> psBlob;

	ShaderCreateDesc vsDesc;
	vsDesc.m_ShaderStage = ShaderStage::kVertex;
	vsDesc.m_ShaderLanguage = ShaderLanguage::kGLSL;
	vsDesc.m_pSourceCode = pVS;
	vsDesc.m_SourceCodeSize = vsSize;
	compiler->Compile(vsDesc, &vsBlob);

	ShaderCreateDesc psDesc;
	psDesc.m_ShaderStage = ShaderStage::kPixelOrFragment;
	psDesc.m_ShaderLanguage = ShaderLanguage::kGLSL;
	psDesc.m_pSourceCode = pPS;
	psDesc.m_SourceCodeSize = psSize;
	compiler->Compile(psDesc, &psBlob);

	BvRCRef<IBvShader> vs;
	vsDesc.m_pByteCode = (const u8*)vsBlob->GetBufferPointer();
	vsDesc.m_ByteCodeSize = vsBlob->GetBufferSize();
	vs.Set(m_Device->CreateShader(vsDesc));
	vsBlob.Reset();

	BvRCRef<IBvShader> ps;
	psDesc.m_pByteCode = (const u8*)psBlob->GetBufferPointer();
	psDesc.m_ByteCodeSize = psBlob->GetBufferSize();
	ps.Set(m_Device->CreateShader(psDesc));
	psBlob.Reset();
	compiler.Reset();

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
	psoDesc.m_RenderTargetFormats[0] = swapChainFormat;
	psoDesc.m_DepthStencilFormat = depthFormat;
	psoDesc.m_pRenderPass = pRenderPass;

	VertexInputDesc inputDescs[3];
	inputDescs[0].m_Format = Format::kRG32_Float;
	inputDescs[1].m_Format = Format::kRG32_Float;
	inputDescs[2].m_Format = Format::kRGBA8_UNorm;
	psoDesc.m_VertexInputDescCount = 3;
	psoDesc.m_pVertexInputDescs = inputDescs;
	m_Pipeline.Set(m_Device->CreateGraphicsPipeline(psoDesc));
}


bool UIOverlay::Update(f32 dt, BvWindow* pWindow)
{
	m_OverlayTimer -= dt;
	if (m_OverlayTimer >= 0.0f)
	{
		return false;
	}
	m_OverlayTimer = 1.0f / 60.0f;

	auto [width, height] = std::pair(pWindow->GetWidth(), pWindow->GetHeight());

	ImGuiIO& io = ImGui::GetIO();

	io.DisplaySize = ImVec2((f32)width, (f32)height);
	io.DeltaTime = dt;

	BvMouse mouse;
	auto [posX, posY] = mouse.GetPosition(pWindow);
	auto& currState = mouse.GetMouseState();
	io.AddMousePosEvent(posX, posY);
	io.AddMouseButtonEvent(0, EHasFlag(currState.mouseButtonStates, BvMouseButton::kLeft));
	io.AddMouseButtonEvent(1, EHasFlag(currState.mouseButtonStates, BvMouseButton::kRight));
	io.AddMouseButtonEvent(2, EHasFlag(currState.mouseButtonStates, BvMouseButton::kMiddle));
	io.AddMouseWheelEvent(currState.mouseWheelDeltaX, currState.mouseWheelDeltaY);

	return true;
}


void UIOverlay::Render()
{
	ImDrawData* pImDrawData = ImGui::GetDrawData();
	int32_t vertexOffset = 0;
	int32_t indexOffset = 0;

	if ((!pImDrawData) || (pImDrawData->CmdListsCount == 0))
	{
		return;
	}

	// Note: Alignment is done inside buffer creation
	u64 vertexBufferSize = pImDrawData->TotalVtxCount * sizeof(ImDrawVert);
	u64 indexBufferSize = pImDrawData->TotalIdxCount * sizeof(ImDrawIdx);

	// Update buffers only if vertex or index count has been changed compared to current buffer size
	if ((vertexBufferSize == 0) || (indexBufferSize == 0))
	{
		return;
	}

	// Vertex buffer
	if ((!m_VB) || (m_VertexCount < pImDrawData->TotalVtxCount))
	{
		m_VBView.Reset();
		m_VB.Reset();
		CreateVB(vertexBufferSize, m_VertexCount);
		m_VertexCount = pImDrawData->TotalVtxCount;
	}

	// Index buffer
	if ((!m_IB) || (m_IndexCount < pImDrawData->TotalIdxCount))
	{
		m_IBView.Reset();
		m_IB.Reset();
		CreateIB(indexBufferSize, m_IndexCount);
		m_IndexCount = pImDrawData->TotalIdxCount;
	}

	// Upload data
	ImDrawVert* pVtxDst = m_VB->GetMappedDataAsT<ImDrawVert>();
	ImDrawIdx* pIdxDst = m_IB->GetMappedDataAsT<ImDrawIdx>();

	for (int n = 0; n < pImDrawData->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = pImDrawData->CmdLists[n];
		memcpy(pVtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		memcpy(pIdxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
		pVtxDst += cmd_list->VtxBuffer.Size;
		pIdxDst += cmd_list->IdxBuffer.Size;
	}

	// Flush to make writes visible to GPU
	m_VB->Flush();
	m_IB->Flush();

	ImGuiIO& io = ImGui::GetIO();

	m_Context->SetGraphicsPipeline(m_Pipeline);
	m_Context->SetTexture(m_TextureView, 0, 1);
	m_Context->SetSampler(m_Sampler, 0, 2);

	//m_PC.scale = Float2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
	//m_PC.translate = Float2(-1.0f, -1.0f);

	Store44(MatrixOrthographicOffCenterLH_DX(io.DisplaySize.x, 0.0f, 0.0f, io.DisplaySize.y, 0.0f, 1.0f), m_PC.wvp.m);

	m_Context->SetShaderConstantsT<PushConstBlock>(m_PC, 0, 0);

	m_Context->SetVertexBufferView(m_VBView);
	m_Context->SetIndexBufferView(m_IBView, IndexFormat::kU16);

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
			m_Context->SetScissor(scissorRect);
			m_Context->DrawIndexed(pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
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
	m_Pipeline.Reset();
	m_SRL.Reset();
	m_VBView.Reset();
	m_IBView.Reset();
	m_VB.Reset();
	m_IB.Reset();
	m_TextureView.Reset();
	m_Texture.Reset();
	m_Sampler.Reset();
	m_Context.Reset();
	m_Device.Reset();
}


void UIOverlay::CreateVB(u64 size, u32 count)
{
	BufferDesc desc;
	desc.m_MemoryType = MemoryType::kUpload;
	desc.m_CreateFlags = BufferCreateFlags::kCreateMapped;
	desc.m_Size = size;
	desc.m_UsageFlags = BufferUsage::kVertexBuffer;
	m_VB.Set(m_Device->CreateBuffer(desc, nullptr));

	BufferViewDesc viewDesc;
	viewDesc.m_pBuffer = m_VB;
	viewDesc.m_ElementCount = count;
	viewDesc.m_Stride = sizeof(ImDrawVert);
	m_VBView.Set(m_Device->CreateBufferView(viewDesc));
}


void UIOverlay::CreateIB(u64 size, u32 count)
{
	BufferDesc desc;
	desc.m_MemoryType = MemoryType::kUpload;
	desc.m_CreateFlags = BufferCreateFlags::kCreateMapped;
	desc.m_Size = size;
	desc.m_UsageFlags = BufferUsage::kIndexBuffer;
	m_IB.Set(m_Device->CreateBuffer(desc, nullptr));

	BufferViewDesc viewDesc;
	viewDesc.m_pBuffer = m_IB;
	viewDesc.m_ElementCount = count;
	viewDesc.m_Stride = sizeof(ImDrawIdx);
	m_IBView.Set(m_Device->CreateBufferView(viewDesc));
}