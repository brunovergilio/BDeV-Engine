#include "WolfRenderer.h"
#include "WolfRendererCommon.h"


void WolfRenderer::Initialize(BvWindow* pWindow)
{
	m_pWindow = pWindow;

	m_RenderLib.Open(kpRenderLib);

	m_ToolsLib.Open("BvRenderTools.dll");

	using EngineFnType = bool(*)(const RenderEngineDesc&, void**);
	auto pFnEngine = m_RenderLib.GetProcAddressT<EngineFnType>("CreateRenderEngine");
	RenderEngineDesc engineDesc;
	pFnEngine(engineDesc, reinterpret_cast<void**>(&m_pEngine));

	using ShaderCompilerFnType = bool(*)(void**);
	ShaderCompilerFnType compilerFn = m_ToolsLib.GetProcAddressT<ShaderCompilerFnType>(kpCompiler);
	compilerFn(reinterpret_cast<void**>(&m_ShaderCompiler));

	using TextureLoaderFnType = bool(*)(void**);
	auto pFnTextureLoader = m_ToolsLib.GetProcAddressT<TextureLoaderFnType>("CreateDDSTextureLoader");
	pFnTextureLoader(reinterpret_cast<void**>(&m_TextureLoader));

	CreateDeviceAndContext();

	SwapChainDesc swapChainDesc;
	swapChainDesc.m_Format = Format::kRGBA8_UNorm_SRGB;
	m_Device->CreateSwapChain(m_pWindow, swapChainDesc, m_Context, &m_SwapChain);

	m_Overlay.Initialize(m_Device, m_Context, m_ShaderCompiler);
	OnInitializeUI();

	CreatePipelineResources();
	CreateTextures();
}


void WolfRenderer::Update(f32 deltaTime)
{
	if (m_UseOverlay && m_Overlay.Update(deltaTime, m_pWindow))
	{
		ImGui::NewFrame();

		OnUpdateUI();

		ImGui::EndFrame();

		ImGui::Render();
	}

	auto [x, y] = m_pWindow->GetSize();
	m_PrimColorConstants.m_PixelSize = Float2(1.0f / f32(i32(x)), 1.0f / f32(i32(y)));
	m_PrimTextureConstants.m_PixelSize = Float2(1.0f / f32(i32(x)), 1.0f / f32(i32(y)));
}


void WolfRenderer::Render()
{
	auto width = m_pWindow->GetWidth();
	auto height = m_pWindow->GetHeight();
	RenderTargetDesc renderTarget;
	renderTarget.SetColorView(m_SwapChain->GetCurrentTextureView(), ResourceState::kCommon, ResourceState::kPresent).SetClearValues({ 0.1f, 0.1f, 0.3f });

	m_Context->NewCommandList();
	m_Context->SetRenderTarget(renderTarget);
	m_Context->SetViewport({ 0.0f, 0.0f, (f32)width, (f32)height, 0.0f, 1.0f });
	m_Context->SetScissor(0, 0, width, height);
	DrawColorPrimitives();
	DrawTexturePrimitives();
	OnRenderUI();
	m_Context->Execute();

	m_SwapChain->Present(false);

	m_Context->FlushFrame();
}


void WolfRenderer::Shutdown()
{
	m_Device->WaitIdle();

	m_Overlay.Shutdown();
	m_SwapChain.Reset();
	m_Context.Reset();
	m_Device.Reset();
	m_pEngine.Reset();
}


BvRCRef<IBvShader> WolfRenderer::CompileShader(const char* pSource, size_t length, ShaderStage stage)
{
	ShaderSourceDesc shaderDesc;
	shaderDesc.m_ShaderLanguage = kShaderLanguage;
	shaderDesc.m_ShaderStage = stage;
	shaderDesc.m_pSourceCode = pSource;
	shaderDesc.m_SourceCodeSize = length;
	BvRCRef<IBvShaderBlob> error;
	BvRCRef<IBvShaderBlob> compiledShader;
	if (!m_ShaderCompiler->Compile(shaderDesc, &compiledShader, &error))
	{
		const char* pErr = (const char*)error->GetBufferPointer();
		BV_ASSERT(false, "Shader error: %s", pErr);
	}

	ShaderDesc desc((const u8*)compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), stage);
	BvRCRef<IBvShader> shader;
	m_Device->CreateShader(desc, &shader);

	return shader;
}


void WolfRenderer::CreateDeviceAndContext()
{
	{
		u32 gpuIndex = 0;
		auto& gpus = m_pEngine->GetGPUs();
		for (auto i = 0; i < gpus.Size(); ++i)
		{
			auto& gpu = *gpus[i];
			if (gpu.m_Type == GPUType::kDiscrete)
			{
				gpuIndex = i;
				break;
			}
		}
		auto& gpu = *gpus[gpuIndex];
		for (auto i = 0; i < gpu.m_ContextGroups.Size(); ++i)
		{
			if (gpu.m_ContextGroups[i].SupportsCommandType(CommandType::kGraphics))
			{
				auto& group = m_RenderDeviceDesc.m_ContextGroups.EmplaceBack();
				group.m_ContextCount = 1;
				group.m_GroupIndex = i;
				break;
			}
		}
		m_RenderDeviceDesc.m_GPUIndex = gpuIndex;
	}

	m_pEngine->CreateRenderDevice(m_RenderDeviceDesc, &m_Device);
	CommandContextDesc ccd(CommandType::kGraphics, false);
	m_Device->CreateCommandContext(ccd, &m_Context);
}


void WolfRenderer::OnInitializeUI()
{
	m_Overlay.SetupPipeline(m_SwapChain->GetDesc().m_Format);
}


void WolfRenderer::OnUpdateUI()
{
	BeginDrawDefaultUI();
	EndDrawDefaultUI();
}


void WolfRenderer::OnRenderUI()
{
	m_Overlay.Render();
}


void WolfRenderer::BeginDrawDefaultUI()
{
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);
	ImGui::Begin("Wolf3D Debug Window", nullptr);
}


void WolfRenderer::EndDrawDefaultUI()
{
	ImGui::End();
}


void WolfRenderer::ToggleOverlay(bool enabled)
{
	m_UseOverlay = enabled;
}


u32 WolfRenderer::CreateTexture(const WolfImage& image)
{
	SubresourceData sd;
	sd.m_pData = image.m_Pixels.Data();
	sd.m_SlicePitch = image.m_Pixels.Size();
	sd.m_RowPitch = sd.m_SlicePitch / image.m_Height;

	TextureInitData tid;
	tid.m_pContext = m_Context;
	tid.m_SubresourceCount = 1;
	tid.m_pSubresources = &sd;
	tid.m_ResourceState = ResourceState::kPixelShaderResource;

	auto& textureData = m_Textures.EmplaceBack();
	TextureDesc td;
	td.SetSize({ u32(image.m_Width), u32(image.m_Height), 1 }).SetUsageFlags(TextureUsage::kShaderResource).SetFormat(Format::kRGBA8_UNorm);
	m_Device->CreateTexture(td, tid, &textureData.m_Texture);

	TextureViewDesc tvd;
	tvd.SetFormat(td.GetFormat()).SetTexturePtr(textureData.m_Texture);
	m_Device->CreateTextureView(tvd, &textureData.m_TextureView);

	return m_Textures.Size();
}


u32 WolfRenderer::CreateTexture(const u8* pData, u32 size)
{
	SubresourceData sd;
	sd.m_pData = pData;
	sd.m_SlicePitch = size;
	sd.m_RowPitch = sd.m_SlicePitch / 64;

	TextureInitData tid;
	tid.m_pContext = m_Context;
	tid.m_SubresourceCount = 1;
	tid.m_pSubresources = &sd;
	tid.m_ResourceState = ResourceState::kPixelShaderResource;

	auto& textureData = m_Textures.EmplaceBack();
	TextureDesc td;
	td.SetSize({ 64, 64, 1 }).SetUsageFlags(TextureUsage::kShaderResource).SetFormat(Format::kRGBA8_UNorm);
	m_Device->CreateTexture(td, tid, &textureData.m_Texture);

	TextureViewDesc tvd;
	tvd.SetFormat(td.GetFormat()).SetTexturePtr(textureData.m_Texture);
	m_Device->CreateTextureView(tvd, &textureData.m_TextureView);

	return m_Textures.Size();
}

void WolfRenderer::CreatePipelineResources()
{
	{
		ShaderResourceLayoutCreateDesc srlDesc;
		srlDesc.AddResourceSet().AddConstant<PrimColor::Constants>("Constants"_sid, ShaderStage::kVertex);
		m_Device->CreateShaderResourceLayout(srlDesc, &m_SRLColorPrim);

		auto vsSquare = CompileShader(PrimColor::g_pVSSquareShaderColor, PrimColor::g_VSSquaredShaderColorSize, ShaderStage::kVertex);
		auto vsLine = CompileShader(PrimColor::g_pVSLineShaderColor, PrimColor::g_VSLineShaderColorSize, ShaderStage::kVertex);
		auto ps = CompileShader(PrimColor::g_pPSShaderColor, PrimColor::g_PSShaderColorSize, ShaderStage::kPixelOrFragment);
		GraphicsPipelineStateDesc psoDesc;
		psoDesc.AddShader(vsLine)
			.AddShader(ps)
			.AddRenderTargetFormat(m_SwapChain->GetDesc().m_Format)
			.SetShaderResourceLayoutPtr(m_SRLColorPrim)
			.SetInputAssemblyState(Topology::kLineList);

		m_Device->CreateGraphicsPipeline(psoDesc, &m_PSOColorLine);

		psoDesc.m_Shaders[0] = vsSquare;
		psoDesc.SetInputAssemblyState(Topology::kTriangleStrip);
		m_Device->CreateGraphicsPipeline(psoDesc, &m_PSOColorSquare);
	}

	{
		ShaderResourceLayoutCreateDesc srlDesc;
		srlDesc.AddResourceSet().AddConstant<PrimTexture::Constants>("Constants"_sid, ShaderStage::kVertex)
			.AddTexture(1, ShaderStage::kPixelOrFragment).AddSampler(2, ShaderStage::kPixelOrFragment)
			.AddConstant<Float4>("ColorMultiplier"_sid, 3, ShaderStage::kPixelOrFragment);
		m_Device->CreateShaderResourceLayout(srlDesc, &m_SRLTexturePrim);

		auto vsSquare = CompileShader(PrimTexture::g_pVSSquareShaderTexture, PrimTexture::g_VSSquaredShaderTextureSize, ShaderStage::kVertex);
		auto vsLine = CompileShader(PrimTexture::g_pVSLineShaderTexture, PrimTexture::g_VSLineShaderTextureSize, ShaderStage::kVertex);
		auto ps = CompileShader(PrimTexture::g_pPSShaderTexture, PrimTexture::g_PSShaderTextureSize, ShaderStage::kPixelOrFragment);
		GraphicsPipelineStateDesc psoDesc;
		psoDesc.AddShader(vsLine)
			.AddShader(ps)
			.AddRenderTarget(m_SwapChain->GetDesc().m_Format, true, BlendFactor::kSrcAlpha, BlendFactor::kInvkSrcAlpha, BlendOp::kAdd,
				BlendFactor::kZero, BlendFactor::kZero, BlendOp::kAdd)
			.SetShaderResourceLayoutPtr(m_SRLTexturePrim)
			.SetInputAssemblyState(Topology::kLineList);

		m_Device->CreateGraphicsPipeline(psoDesc, &m_PSOTextureLine);

		psoDesc.m_Shaders[0] = vsSquare;
		psoDesc.SetInputAssemblyState(Topology::kTriangleStrip);
		m_Device->CreateGraphicsPipeline(psoDesc, &m_PSOTextureSquare);
	}
}


void WolfRenderer::CreateTextures()
{
	constexpr auto kSize = 64;
	constexpr auto kDefaultTextureCount = 3;

	m_Textures.Resize(kDefaultTextureCount);

	BvVector<u8> buff(kSize * kSize * 4 * kDefaultTextureCount);

	{
		auto pBuf = &buff[0];
		for (auto h = 0; h < kSize; h++)
		{
			f32 v = f32(h) / f32(kSize);
			for (auto w = 0; w < kSize; w++)
			{
				f32 u = f32(w) / f32(kSize);

				auto i = (h * kSize + w) * 4;
				u8* pColor = &pBuf[i];
				pColor[0] = std::clamp(i32(0.0f), 0, 255);
				pColor[1] = std::clamp(i32(u * 255), 0, 255);
				pColor[2] = std::clamp(i32(v * 255), 0, 255);
				pColor[3] = 255;
			}
		}

		SubresourceData sd;
		sd.m_pData = pBuf;
		sd.m_RowPitch = 4 * kSize;
		sd.m_SlicePitch = sd.m_RowPitch * kSize;

		TextureInitData tid;
		tid.m_pContext = m_Context;
		tid.m_SubresourceCount = 1;
		tid.m_pSubresources = &sd;
		tid.m_ResourceState = ResourceState::kPixelShaderResource;

		TextureDesc td;
		td.SetSize({ kSize, kSize, 1 }).SetUsageFlags(TextureUsage::kShaderResource).SetFormat(Format::kRGBA8_UNorm);
		m_Device->CreateTexture(td, tid, &m_Textures[0].m_Texture);

		TextureViewDesc tvd;
		tvd.SetFormat(td.GetFormat()).SetTexturePtr(m_Textures[0].m_Texture);
		m_Device->CreateTextureView(tvd, &m_Textures[0].m_TextureView);
	}

	{
		auto pBuf = &buff[kSize * kSize * 4];
		for (auto h = 0; h < kSize; h++)
		{
			f32 v = f32(h) / f32(kSize);
			for (auto w = 0; w < kSize; w++)
			{
				f32 u = f32(w) / f32(kSize);

				auto i = (h * kSize + w) * 4;
				u8* pColor = &pBuf[i];
				pColor[0] = std::clamp(i32(u * 255), 0, 255);
				pColor[1] = std::clamp(i32(v * 255), 0, 255);
				pColor[2] = std::clamp(i32(0.0f), 0, 255);
				pColor[3] = 255;
			}
		}

		SubresourceData sd;
		sd.m_pData = pBuf;
		sd.m_RowPitch = 4 * kSize;
		sd.m_SlicePitch = sd.m_RowPitch * kSize;

		TextureInitData tid;
		tid.m_pContext = m_Context;
		tid.m_SubresourceCount = 1;
		tid.m_pSubresources = &sd;
		tid.m_ResourceState = ResourceState::kPixelShaderResource;

		TextureDesc td;
		td.SetSize({ kSize, kSize, 1 }).SetUsageFlags(TextureUsage::kShaderResource).SetFormat(Format::kRGBA8_UNorm);
		m_Device->CreateTexture(td, tid, &m_Textures[1].m_Texture);

		TextureViewDesc tvd;
		tvd.SetFormat(td.GetFormat()).SetTexturePtr(m_Textures[1].m_Texture);
		m_Device->CreateTextureView(tvd, &m_Textures[1].m_TextureView);
	}

	{
		auto pBuf = &buff[kSize * kSize * 4 * 2];
		for (auto h = 0; h < kSize; h++)
		{
			f32 v = f32(h) / f32(kSize);
			for (auto w = 0; w < kSize; w++)
			{
				f32 u = f32(w) / f32(kSize);

				auto i = (h * kSize + w) * 4;
				u8* pColor = &pBuf[i];
				pColor[0] = std::clamp(i32(u * 127), 0, 255);
				pColor[1] = std::clamp(i32(v * 127), 0, 255);
				pColor[2] = std::clamp(255, 0, 255);
				pColor[3] = 255;
			}
		}

		SubresourceData sd;
		sd.m_pData = pBuf;
		sd.m_RowPitch = 4 * kSize;
		sd.m_SlicePitch = sd.m_RowPitch * kSize;

		TextureInitData tid;
		tid.m_pContext = m_Context;
		tid.m_SubresourceCount = 1;
		tid.m_pSubresources = &sd;
		tid.m_ResourceState = ResourceState::kPixelShaderResource;

		TextureDesc td;
		td.SetSize({ kSize, kSize, 1 }).SetUsageFlags(TextureUsage::kShaderResource).SetFormat(Format::kRGBA8_UNorm);
		m_Device->CreateTexture(td, tid, &m_Textures[2].m_Texture);

		TextureViewDesc tvd;
		tvd.SetFormat(td.GetFormat()).SetTexturePtr(m_Textures[2].m_Texture);
		m_Device->CreateTextureView(tvd, &m_Textures[2].m_TextureView);
	}

	SamplerDesc sd;
	sd.SetFilter(Filter::kPoint, Filter::kPoint, MipMapFilter::kPoint).SetAddressMode(AddressMode::kMirror, AddressMode::kMirror, AddressMode::kMirror);
	m_Device->CreateSampler(sd, &m_PointSampler);
}


void WolfRenderer::DrawColorPrimitives()
{
	for (auto& batch : m_ColorBatches)
	{
		m_Context->SetGraphicsPipeline(m_PSOColorSquare);
		for (auto& square : batch.m_ColorSquares)
		{
			m_PrimColorConstants.m_Color = square.m_Color;
			m_PrimColorConstants.m_Dims = Float4(square.m_Pos.x, square.m_Pos.y, square.m_Size.x, square.m_Size.y);
			m_Context->SetShaderConstantsT<PrimColor::Constants>(m_PrimColorConstants, 0, 0);
			m_Context->Draw(4);
		}
		batch.m_ColorSquares.Clear();

		m_Context->SetGraphicsPipeline(m_PSOColorLine);
		for (auto& line : batch.m_ColorLines)
		{
			m_PrimColorConstants.m_Color = line.m_Color;
			m_PrimColorConstants.m_Dims = Float4(line.m_FromPos.x, line.m_FromPos.y, line.m_ToPos.x, line.m_ToPos.y);
			m_Context->SetShaderConstantsT<PrimColor::Constants>(m_PrimColorConstants, 0, 0);
			m_Context->Draw(2);
		}
		batch.m_ColorLines.Clear();
	}
	m_ColorBatches.Clear();
}


void WolfRenderer::DrawTexturePrimitives()
{
	for (auto& batch : m_TextureBatches)
	{
		m_Context->SetGraphicsPipeline(m_PSOTextureSquare);
		for (auto& square : batch.m_Squares)
		{
			m_PrimTextureConstants.m_UVs[0] = square.m_UVs[0];
			m_PrimTextureConstants.m_UVs[1] = square.m_UVs[1];
			m_PrimTextureConstants.m_Dims = Float4(square.m_Pos.x, square.m_Pos.y, square.m_Size.x, square.m_Size.y);
			m_Context->SetShaderConstantsT<PrimTexture::Constants>(m_PrimTextureConstants, 0, 0);
			m_Context->SetTexture(m_Textures[std::clamp(square.m_TextureIndex, 0u, u32(m_Textures.Size() - 1))].m_TextureView, 0, 1);
			m_Context->SetSampler(m_PointSampler, 0, 2);
			m_Context->SetShaderConstantsT<Float4>(square.m_ColorMultiplier, 3, 0);
			m_Context->Draw(4);
		}
		batch.m_Squares.Clear();

		m_Context->SetGraphicsPipeline(m_PSOTextureLine);
		for (auto& line : batch.m_Lines)
		{
			m_PrimTextureConstants.m_UVs[0] = line.m_UVs[0];
			m_PrimTextureConstants.m_UVs[1] = line.m_UVs[1];
			m_PrimTextureConstants.m_Dims = Float4(line.m_FromPos.x, line.m_FromPos.y, line.m_ToPos.x, line.m_ToPos.y);
			m_Context->SetShaderConstantsT<PrimTexture::Constants>(m_PrimTextureConstants, 0, 0);
			m_Context->SetTexture(m_Textures[std::clamp(line.m_TextureIndex, 0u, u32(m_Textures.Size() - 1))].m_TextureView, 0, 1);
			m_Context->SetSampler(m_PointSampler, 0, 2);
			m_Context->SetShaderConstantsT<Float4>(line.m_ColorMultiplier, 3, 0);
			m_Context->Draw(2);
		}
		batch.m_Lines.Clear();
	}
	m_TextureBatches.Clear();
}