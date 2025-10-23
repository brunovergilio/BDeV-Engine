#include "SampleBase.h"


void SampleBase::Initialize()
{
	m_App.Initialize();
	m_App.RegisterRawInput(true, true);
	m_RenderLib.Open("BvRenderVk.dll");
	m_ToolsLib.Open("BvRenderTools.dll");

	using EngineFnType = bool(*)(const BvUUID&, void**);
	auto pFnEngine = m_RenderLib.GetProcAddressT<EngineFnType>("CreateRenderEngine");
	IBvRenderEngine::Create(pFnEngine, &m_pEngine);

	using ShaderCompilerFnType = bool(*)(const BvUUID&, void**);
	ShaderCompilerFnType compilerFn = m_ToolsLib.GetProcAddressT<ShaderCompilerFnType>("CreateSPIRVCompiler");
	IBvShaderCompiler::Create(compilerFn, &m_SpvCompiler);

	using TextureLoaderFnType = bool(*)(const BvUUID&, void**);
	auto pFnTextureLoader = m_ToolsLib.GetProcAddressT<TextureLoaderFnType>("CreateDDSTextureLoader");
	IBvTextureLoader::Create(pFnTextureLoader, &m_TextureLoader);

	CreateDeviceAndContext();

	WindowDesc windowDesc;
	windowDesc.m_X += 100;
	windowDesc.m_Y += 100;
	m_pWindow = m_App.CreateWindow(windowDesc);

	SwapChainDesc swapChainDesc;
	swapChainDesc.m_Format = Format::kRGBA8_UNorm_SRGB;
	m_Device->CreateSwapChain(m_pWindow, swapChainDesc, m_Context, &m_SwapChain);

	m_Overlay.Initialize(m_Device, m_Context, m_SpvCompiler);
	OnInitialize();
	OnInitializeUI();
	m_Curr = BvTime::GetCurrentTimestampInUs();
}


void SampleBase::Update()
{
	m_Dt = f32(m_Curr - m_Prev) * 0.000001f;
	m_FPSCounter.Update(m_Dt);
	m_Prev = m_Curr;
	m_Curr = BvTime::GetCurrentTimestampInUs();

	m_App.ProcessOSEvents();
	if (m_pWindow->IsClosed())
	{
		m_IsDone = true;
		return;
	}

	if (m_Keyboard.KeyIsDown(BvKey::kW))
	{
		m_Camera.Walk(m_Dt * 8.0f);
	}
	if (m_Keyboard.KeyIsDown(BvKey::kS))
	{
		m_Camera.Walk(-m_Dt * 8.0f);
	}
	if (m_Keyboard.KeyIsDown(BvKey::kA))
	{
		m_Camera.Strafe(-m_Dt * 8.0f);
	}
	if (m_Keyboard.KeyIsDown(BvKey::kD))
	{
		m_Camera.Strafe(m_Dt * 8.0f);
	}
	if (m_Keyboard.KeyIsDown(BvKey::kR))
	{
		m_Camera.Fly(m_Dt * 8.0f);
	}
	if (m_Keyboard.KeyIsDown(BvKey::kF))
	{
		m_Camera.Fly(-m_Dt * 8.0f);
	}
	auto& mouseState = m_Mouse.GetMouseState();
	if (m_Keyboard.KeyIsDown(BvKey::kLControl))
	{
		if (mouseState.relativeMousePositionY != 0.0f)
		{
			m_Camera.Pitch(-mouseState.relativeMousePositionY * m_Dt);
		}
		if (mouseState.relativeMousePositionX != 0.0f)
		{
			m_Camera.RotateY(-mouseState.relativeMousePositionX * m_Dt);
		}
	}

	m_Camera.SetPerspective(0.1f, 100.0f, f32(m_pWindow->GetWidth()) / f32(m_pWindow->GetHeight()), kPiDiv4);
	m_Camera.Update();

	OnUpdate();
	if (m_UseOverlay && m_Overlay.Update(m_Dt, m_pWindow))
	{
		ImGui::NewFrame();

		OnUpdateUI();

		ImGui::EndFrame();

		ImGui::Render();
	}
}


void SampleBase::Render()
{
	if (m_IsDone)
	{
		return;
	}

	OnRender();
}


void SampleBase::Shutdown()
{
	OnShutdown();

	m_Overlay.Shutdown();
	m_SwapChain.Reset();
	m_Context.Reset();
	m_App.DestroyWindow(m_pWindow);
	m_Device.Reset();
	m_pEngine.Reset();

	m_App.Shutdown();
}


BvRCRef<IBvShader> SampleBase::CompileShader(const char* pSource, size_t length, ShaderStage stage)
{
	ShaderCreateDesc shaderDesc;
	shaderDesc.m_ShaderLanguage = ShaderLanguage::kGLSL;
	shaderDesc.m_ShaderStage = stage;
	shaderDesc.m_pSourceCode = pSource;
	shaderDesc.m_SourceCodeSize = length;
	BvRCRef<IBvShaderBlob> error;
	BvRCRef<IBvShaderBlob> compiledShader;
	if (!m_SpvCompiler->Compile(shaderDesc, &compiledShader, &error))
	{
		const char* pErr = (const char*)error->GetBufferPointer();
		BV_ASSERT(false, "Shader error: %s", pErr);
	}
	shaderDesc.m_pByteCode = (const u8*)compiledShader->GetBufferPointer();
	shaderDesc.m_ByteCodeSize = compiledShader->GetBufferSize();
	BvRCRef<IBvShader> shader;
	m_Device->CreateShader(shaderDesc, &shader);

	return shader;
}


void SampleBase::CreateDeviceAndContext()
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


void SampleBase::OnInitializeUI()
{
	m_Overlay.SetupPipeline(m_SwapChain->GetDesc().m_Format);
}


void SampleBase::OnUpdateUI()
{
	BeginDrawDefaultUI();
	EndDrawDefaultUI();
}


void SampleBase::OnRenderUI()
{
	m_Overlay.Render();
}


void SampleBase::BeginDrawDefaultUI()
{
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);
	ImGui::Begin("Render Sample", nullptr);
	ImGui::TextUnformatted(m_AppName.CStr());
	ImGui::Text("FPS: %d", m_FPSCounter.GetFPS());
}


void SampleBase::EndDrawDefaultUI()
{
	ImGui::End();
}