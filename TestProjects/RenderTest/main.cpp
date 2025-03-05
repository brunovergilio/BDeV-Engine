#include "BDeV/BDeV.h"
#include "BvRenderVk/BvRenderEngineVk.h"
#include "BvRenderVk/BvRenderDeviceVk.h"
#include "BvRenderVk/BvSwapChainVk.h"


static const char* pVSShader =
R"raw(
	#version 450

	vec2 vertices[] = 
	{
		vec2(-0.7f,  0.7f),
		vec2( 0.0f, -0.7f),
		vec2( 0.7f,  0.7f)
	};

	void main()
	{
		gl_Position = vec4(vertices[gl_VertexIndex], 0.0f, 1.0f);
	}
		)raw";


static const char* pVSShader2 =
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


static const char* pPSShader =
R"raw(
			#version 450

			layout (location = 0) out vec4 outColor;

			void main()
			{
				outColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
			}
		)raw";

static const char* pPSShader2 =
R"raw(
			#version 450

			layout (location = 0) in vec4 inColor;

			layout (location = 0) out vec4 outColor;

			void main()
			{
				outColor = inColor;
			}
		)raw";


struct PosColorVertex
{
	Float3 position;
	Float4 color;
};


IBvShaderCompiler* g_pCompiler = nullptr;

IBvShader* GetVS(IBvRenderDevice* pDevice);
IBvShader* GetPS(IBvRenderDevice* pDevice);
IBvBuffer* CreateVB(IBvRenderDevice* pDevice);
IBvBuffer* CreateUB(IBvRenderDevice* pDevice);


int main()
{
	RayTracingAccelerationStructureDesc de, df;
	de = df;
	BvApplication app;
	app.Initialize();
	app.RegisterRawInput(false, true);

	BvSharedLib renderToolsLib("BvRenderTools.dll");
	typedef bool(*pFNGetShaderCompiler)(IBvShaderCompiler**);
	pFNGetShaderCompiler compilerFn = renderToolsLib.GetProcAddressT<pFNGetShaderCompiler>("CreateSPIRVCompiler");
	compilerFn(&g_pCompiler);

	IBvRenderEngineVk* pEngine;
	BvRenderVk::CreateRenderEngineVk(&pEngine);
	BvRenderDeviceVk* pDevice;
	pEngine->CreateRenderDevice(BvRenderDeviceCreateDescVk(), &pDevice);

	BvKeyboard keyboard;
	auto pKeyboard = &keyboard;

	WindowDesc windowDesc;
	windowDesc.m_X += 100;
	windowDesc.m_Y += 100;
	auto pWindow = app.CreateWindow(windowDesc);

	SwapChainDesc swapChainDesc;
	swapChainDesc.m_Format = Format::kRGBA8_UNorm_SRGB;
	auto pGraphicsContext = pDevice->GetGraphicsContext();
	//BvObjectHandle<BvSwapChainVk> scvk;
	//pDevice->CreateSwapChainVk(pWindow, swapChainDesc, pGraphicsContext, &scvk);

	IBvSwapChain* pSwapChain;
	pDevice->CreateSwapChain(pWindow, swapChainDesc, pGraphicsContext, &pSwapChain);

	ShaderResourceDesc resourceDesc = ShaderResourceDesc::AsConstantBuffer(0, ShaderStage::kVertex);
	ShaderResourceSetDesc setDesc;
	setDesc.m_Index = 0;
	setDesc.m_ResourceCount = 1;
	setDesc.m_pResources = &resourceDesc;
	ShaderResourceLayoutDesc layoutDesc;
	layoutDesc.m_ShaderResourceSetCount = 1;
	layoutDesc.m_pShaderResourceSets = &setDesc;
	IBvShaderResourceLayout* pShaderResourceLayout;
	pDevice->CreateShaderResourceLayout(layoutDesc, &pShaderResourceLayout);

	auto pVB = CreateVB(pDevice);
	BufferViewDesc vbViewDesc;
	vbViewDesc.m_pBuffer = pVB;
	vbViewDesc.m_Stride = sizeof(PosColorVertex);
	vbViewDesc.m_ElementCount = 3;
	IBvBufferView* pVBView;
	pDevice->CreateBufferView(vbViewDesc, &pVBView);

	auto pUB = CreateUB(pDevice);
	BufferViewDesc ubViewDesc;
	ubViewDesc.m_pBuffer = pUB;
	ubViewDesc.m_Stride = sizeof(Float44);
	ubViewDesc.m_ElementCount = 1;
	IBvBufferView* pUBView;
	pDevice->CreateBufferView(ubViewDesc, &pUBView);

	GraphicsPipelineStateDesc pipelineDesc;
	pipelineDesc.m_Shaders[0] = GetVS(pDevice);
	pipelineDesc.m_Shaders[1] = GetPS(pDevice);
	//pipelineDesc.m_BlendStateDesc.m_BlendAttachments.PushBack(BlendAttachmentStateDesc());
	//pipelineDesc.m_pRenderPass = pRenderPass;
	pipelineDesc.m_RenderTargetFormats[0] = pSwapChain->GetDesc().m_Format;
	pipelineDesc.m_pShaderResourceLayout = pShaderResourceLayout;
	pipelineDesc.m_InputAssemblyStateDesc.m_Topology = Topology::kTriangleList;
	
	VertexInputDesc inputDescs[2]{};
	inputDescs[0].m_Format = Format::kRGB32_Float;
	
	inputDescs[1].m_Format = Format::kRGBA32_Float;
	inputDescs[1].m_Offset = sizeof(Float3);
	
	pipelineDesc.m_VertexInputDescCount = 2;
	pipelineDesc.m_pVertexInputDescs = inputDescs;

	IBvGraphicsPipelineState* pPSO;
	pDevice->CreateGraphicsPipeline(pipelineDesc, &pPSO);

	IBvQuery* pQuery;
	pDevice->CreateQuery(QueryType::kTimestamp, &pQuery);

	auto currIndex = 0;
	u64 frame = 0;
	while (true)
	{
		frame++;
		//PrintF(ConsoleColor::kLightGreen, "Frame %d:\n", frame);
		//PrintF(ConsoleColor::kAqua, "Image index aquired: %d\n", pSwapChain->GetCurrentImageIndex());
		app.ProcessOSEvents();
		if (!pWindow->IsVisible())
		{
			break;
		}

		if (pKeyboard->KeyWentDown(BvKey::kReturn))
		{
			break;
		}
		if (pKeyboard->KeyWentDown(BvKey::kB))
		{
			//window.ChangeMode(WindowMode::kWindowedFullscreen);
			continue;
		}
		if (pKeyboard->KeyWentDown(BvKey::kW))
		{
			//window.ChangeMode(WindowMode::kWindowed);
			continue;
		}
		if (pKeyboard->KeyWentDown(BvKey::kU))
		{
			pWindow->Resize(640, 480);
			continue;
		}
		if (pKeyboard->KeyWentDown(BvKey::kI))
		{
			pWindow->Resize(800, 600);
			continue;
		}
		if (pKeyboard->KeyWentDown(BvKey::kM))
		{
			pWindow->Move(100, 100);
			continue;
		}
		if (pKeyboard->KeyWentDown(BvKey::kN))
		{
			pWindow->Move(200, 200);
			continue;
		}

		auto width = pWindow->GetWidth();
		auto height = pWindow->GetHeight();

		Store44(MatrixLookAtLH(VectorSet(0.0f, 0.0f, -5.0f, 1.0f), VectorSet(0.0f, 0.0f, 1.0f, 1.0f), VectorSet(0.0f, 1.0f, 0.0f, 1.0f)) *
			MatrixPerspectiveLH_DX(0.1f, 100.0f, float(width) / float(height), kPiDiv4),
			pUB->GetMappedDataAsT<float>());

		u64 ts = 0;
		if (pQuery->GetResult(&ts, 8))
		{
			//BvConsole::Print("ts - %llu\n", ts);
		}

		//float f = fmodf((float)GetTickCount64(), 360.0f) / 360.0f;
		IBvTextureView *pRenderTargets[] = { pSwapChain->GetCurrentTextureView() };
		ClearColorValue cl[] = { ClearColorValue(
			0.1f,
			0.1f,
			0.3f) };
		auto renderTarget = RenderTargetDesc::AsSwapChain(pRenderTargets[0], *cl);
		pGraphicsContext->NewCommandList();
		pGraphicsContext->BeginQuery(pQuery);
		pGraphicsContext->SetRenderTarget(renderTarget);
		pGraphicsContext->SetGraphicsPipeline(pPSO);
		pGraphicsContext->SetViewport({ 0.0f, 0.0f, (f32)width, (f32)height, 0.0f, 1.0f });
		pGraphicsContext->SetScissor({ 0, 0, width, height });
		pGraphicsContext->SetVertexBufferView(pVBView);
		pGraphicsContext->SetConstantBuffer(pUBView, 0, 0, 0);
		pGraphicsContext->Draw(3);
		pGraphicsContext->EndQuery(pQuery);
		pGraphicsContext->Execute();

		pSwapChain->Present(false);

		pGraphicsContext->FlushFrame();
	}

	app.Shutdown();

	return 0;
}


IBvShader* GetVS(IBvRenderDevice* pDevice)
{
	ShaderCreateDesc shaderDesc;
	shaderDesc.m_ShaderStage = ShaderStage::kVertex;
	shaderDesc.m_ShaderLanguage = ShaderLanguage::kGLSL;
	shaderDesc.m_pSourceCode = pVSShader2;
	shaderDesc.m_SourceCodeSize = strlen(pVSShader2);
	IBvShaderBlob* shader;
	auto result = g_pCompiler->Compile(shaderDesc, &shader);
	BV_ASSERT(result, "Invalid Shader");

	shaderDesc.m_pByteCode = (const u8*)shader->GetBufferPointer();
	shaderDesc.m_ByteCodeSize = shader->GetBufferSize();

	IBvShader* pShader;
	pDevice->CreateShader(shaderDesc, &pShader);
	shader->Release();

	return pShader;
}


IBvShader* GetPS(IBvRenderDevice* pDevice)
{
	ShaderCreateDesc shaderDesc;
	shaderDesc.m_ShaderStage = ShaderStage::kPixelOrFragment;
	shaderDesc.m_ShaderLanguage = ShaderLanguage::kGLSL;
	shaderDesc.m_pSourceCode = pPSShader2;
	shaderDesc.m_SourceCodeSize = strlen(pPSShader2);
	IBvShaderBlob* shader;
	auto result = g_pCompiler->Compile(shaderDesc, &shader);
	BV_ASSERT(result, "Invalid Shader");

	shaderDesc.m_pByteCode = (const u8*)shader->GetBufferPointer();
	shaderDesc.m_ByteCodeSize = shader->GetBufferSize();

	IBvShader* pShader;
	pDevice->CreateShader(shaderDesc, &pShader);
	shader->Release();

	return pShader;
}

IBvBuffer* CreateVB(IBvRenderDevice* pDevice)
{
	PosColorVertex verts[] =
	{
		{{  0.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }},
		{{  1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }},
		{{ -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }}
	};

	BufferDesc vbBufferDesc;
	vbBufferDesc.m_Size = sizeof(verts);
	vbBufferDesc.m_UsageFlags = BufferUsage::kVertexBuffer;
	BufferInitData bufferData;
	bufferData.m_pContext = pDevice->GetGraphicsContext();
	bufferData.m_pData = verts;
	bufferData.m_Size = sizeof(verts);
	IBvBuffer* pVB;
	pDevice->CreateBuffer(vbBufferDesc, &bufferData, &pVB);

	return pVB;
}

IBvBuffer* CreateUB(IBvRenderDevice* pDevice)
{
	BufferDesc uniformBufferDesc;
	uniformBufferDesc.m_Size = sizeof(Float44);
	uniformBufferDesc.m_MemoryType = MemoryType::kUpload;
	uniformBufferDesc.m_UsageFlags = BufferUsage::kConstantBuffer;
	uniformBufferDesc.m_CreateFlags = BufferCreateFlags::kCreateMapped;
	IBvBuffer* pUniform;
	pDevice->CreateBuffer(uniformBufferDesc, nullptr, &pUniform);
	
	return pUniform;
}