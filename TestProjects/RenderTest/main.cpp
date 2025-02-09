#include "BDeV/BDeV.h"
#include "BvRenderVk/BvRenderEngineVk.h"


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

BvShader* GetVS(BvRenderDevice* pDevice);
BvShader* GetPS(BvRenderDevice* pDevice);
BvBuffer* CreateVB(BvRenderDevice* pDevice);
BvBuffer* CreateUB(BvRenderDevice* pDevice);


BV_IBVOBJECT_DEFINE_IID(BvTest, "A8000000-C200-0000-0000-000000000000")
class BvTest : public BvObjectBase
{
public:
	BvTest() { printf("Hello "); }
	~BvTest() { printf("and bye\n"); }
};


BV_IBVOBJECT_DEFINE_IID(BvTest2, "A84E00FF-C200-0000-0000-000000000000")
class BvTest2 : public BvTest
{
public:
	BV_IBVOBJECT_IMPL_INTERFACE(BvTest2, BvTest);

	BvTest2() { printf("Hello "); }
	~BvTest2() { printf("and bye\n"); }
};


int main()
{
	{
		BvTest2* pObj;
		BvTest* ptt = BV_OBJECT_CREATE(BvTest2);
		ptt->QueryInterface(BV_IBVOBJECT_IID(BvTest2), (IBvObject**)&pObj);
		ptt->Release();
		pObj->Release();
	}

	BvApplication app;
	app.Initialize();

	BvSharedLib renderToolsLib("BvRenderTools.dll");
	typedef IBvShaderCompiler*(*pFNGetShaderCompiler)();
	pFNGetShaderCompiler compilerFn = renderToolsLib.GetProcAddressT<pFNGetShaderCompiler>("GetSPIRVCompiler");
	g_pCompiler = compilerFn();

	auto pEngine = BvRenderEngineVk::GetInstance();
	auto pDevice = pEngine->CreateRenderDevice(BvRenderDeviceCreateDesc());

	BvKeyboard keyboard;
	auto pKeyboard = &keyboard;

	WindowDesc windowDesc;
	windowDesc.m_X += 100;
	windowDesc.m_Y += 100;
	auto pWindow = app.CreateWindow(windowDesc);

	SwapChainDesc swapChainDesc;
	swapChainDesc.m_Format = Format::kRGBA8_UNorm_SRGB;
	auto pGraphicsContext = pDevice->GetGraphicsContext();
	BvSwapChain* pSwapChain = pDevice->CreateSwapChain(pWindow, swapChainDesc, pGraphicsContext);

	// Create uniform buffer

	ShaderResourceDesc resourceDesc = ShaderResourceDesc::AsConstantBuffer(0, ShaderStage::kVertex);
	//auto pShaderResourceLayout = pDevice->CreateShaderResourceLayout(0, nullptr);
	auto pShaderResourceLayout = pDevice->CreateShaderResourceLayout(1, &resourceDesc);

	auto pVB = CreateVB(pDevice);
	BufferViewDesc vbViewDesc;
	vbViewDesc.m_pBuffer = pVB;
	vbViewDesc.m_Stride = sizeof(PosColorVertex);
	vbViewDesc.m_ElementCount = 3;
	auto pVBView = pDevice->CreateBufferView(vbViewDesc);

	auto pUB = CreateUB(pDevice);
	BufferViewDesc ubViewDesc;
	ubViewDesc.m_pBuffer = pUB;
	ubViewDesc.m_Stride = sizeof(Float44);
	ubViewDesc.m_ElementCount = 1;
	auto pUBView = pDevice->CreateBufferView(ubViewDesc);

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

	auto pPSO = pDevice->CreateGraphicsPipeline(pipelineDesc);

	auto pQuery = pDevice->CreateQuery(QueryType::kTimestamp);

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
			BvConsole::Print("ts - %llu\n", ts);
		}

		//float f = fmodf((float)GetTickCount64(), 360.0f) / 360.0f;
		BvTextureView *pRenderTargets[] = { pSwapChain->GetCurrentTextureView() };
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


BvShader* GetVS(BvRenderDevice* pDevice)
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

	return pDevice->CreateShader(shaderDesc);
}


BvShader* GetPS(BvRenderDevice* pDevice)
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

	return pDevice->CreateShader(shaderDesc);
}

BvBuffer* CreateVB(BvRenderDevice* pDevice)
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
	auto pVB = pDevice->CreateBuffer(vbBufferDesc, &bufferData);

	return pVB;
}

BvBuffer* CreateUB(BvRenderDevice* pDevice)
{
	BufferDesc uniformBufferDesc;
	uniformBufferDesc.m_Size = sizeof(Float44);
	uniformBufferDesc.m_MemoryType = MemoryType::kUpload;
	uniformBufferDesc.m_UsageFlags = BufferUsage::kUniformBuffer;
	auto pUniform = pDevice->CreateBuffer(uniformBufferDesc);
	pUniform->Map();
	
	return pUniform;
}