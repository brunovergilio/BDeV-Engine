#include "BDeV/BDeV.h"
#include "BvRenderTools/ShaderCompiler/BvShaderCompiler.h"


struct PosColorVertex
{
	Float3 position;
	Float4 color;
};


IBvShaderCompiler* g_pCompiler = nullptr;
IBvShaderBlob* g_pVS = nullptr;
IBvShaderBlob* g_pPS = nullptr;

ShaderByteCodeDesc GetVS();
ShaderByteCodeDesc GetPS();
BvBuffer* CreateVB(BvRenderDevice* pDevice);
BvBuffer* CreateUB(BvRenderDevice* pDevice);


int main()
{
	BvApplication::Initialize();

	BvSharedLib renderToolsLib("BvRenderTools.dll");
	typedef IBvShaderCompiler* (*pFNGetShaderCompiler)();
	pFNGetShaderCompiler compilerFn = renderToolsLib.GetProcAddressT<pFNGetShaderCompiler>("GetShaderCompiler");
	auto pCompiler = compilerFn();
	//ShaderDesc compDesc = { "main", ShaderStage::kVertex, ShaderLanguage::kGLSL };
	//pCompiler->CompileFromFile("D:\\Bruno\\C++\\test.vert", compDesc);
	BvSharedLib renderVkLib("BvRenderVk.dll");
	typedef BvRenderEngine* (*pFNCreateRenderEngine)();
	pFNCreateRenderEngine createeRenderEngineFn = renderVkLib.GetProcAddressT<pFNCreateRenderEngine>("CreateRenderEngine");
	typedef void (*pFNDestroyRenderEngine)();
	pFNDestroyRenderEngine destroyRenderEngineFn = renderVkLib.GetProcAddressT<pFNDestroyRenderEngine>("DestroyRenderEngine");

	g_pCompiler = compilerFn();

	auto pEngine = createeRenderEngineFn();
	auto pDevice = pEngine->CreateRenderDevice();

	BvKeyboard keyboard;
	auto pKeyboard = &keyboard;

	WindowDesc windowDesc;
	windowDesc.m_X += 100;
	windowDesc.m_Y += 100;
	auto pWindow = BvApplication::CreateWindow(windowDesc);

	SwapChainDesc swapChainDesc;
	swapChainDesc.m_Format = Format::kRGBA8_UNorm_SRGB;
	auto pGraphicsContext = pDevice->GetGraphicsContext();
	BvSwapChain* pSwapChain = pDevice->CreateSwapChain(pWindow, swapChainDesc, pGraphicsContext);

	// Create uniform buffer

	ShaderResourceDesc resourceDesc = ShaderResourceDesc::AsConstantBuffer(0, ShaderStage::kVertex);
	auto pShaderResourceLayout = pDevice->CreateShaderResourceLayout(1, &resourceDesc, ShaderResourceConstantDesc());

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
	pipelineDesc.m_ShaderStages.PushBack(GetVS());
	pipelineDesc.m_ShaderStages.PushBack(GetPS());
	pipelineDesc.m_BlendStateDesc.m_BlendAttachments.PushBack(BlendAttachmentStateDesc());
	//pipelineDesc.m_pRenderPass = pRenderPass;
	pipelineDesc.m_RenderTargetFormats[0] = pSwapChain->GetDesc().m_Format;
	pipelineDesc.m_pShaderResourceLayout = pShaderResourceLayout;
	pipelineDesc.m_InputAssemblyStateDesc.m_Topology = Topology::kTriangleList;
	
	pipelineDesc.m_VertexInputDesc.Resize(2);
	pipelineDesc.m_VertexInputDesc[0].m_Format = Format::kRGB32_Float;
	pipelineDesc.m_VertexInputDesc[0].m_Stride = sizeof(PosColorVertex);

	pipelineDesc.m_VertexInputDesc[1].m_Format = Format::kRGBA32_Float;
	pipelineDesc.m_VertexInputDesc[1].m_Location = 1;
	pipelineDesc.m_VertexInputDesc[1].m_Stride = sizeof(PosColorVertex);
	pipelineDesc.m_VertexInputDesc[1].m_Offset = sizeof(Float3);

	auto pPSO = pDevice->CreateGraphicsPipeline(pipelineDesc);

	g_pCompiler->DestroyShader(g_pVS);
	g_pCompiler->DestroyShader(g_pPS);

	auto pQuery = pDevice->CreateQuery(QueryType::kTimestamp);

	auto currIndex = 0;
	u64 frame = 0;
	while (true)
	{
		frame++;
		//PrintF(ConsoleColor::kLightGreen, "Frame %d:\n", frame);
		//PrintF(ConsoleColor::kAqua, "Image index aquired: %d\n", pSwapChain->GetCurrentImageIndex());
		BvApplication::ProcessOSEvents();
		if (!pWindow->IsValid())
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

		Store44(MatrixLookAtLH(VectorSet(0.0f, 0.0f, -5.0f, 1.0f), VectorSet(0.0f, 0.0f, 1.0f, 1.0f), VectorSet(0.0f, 1.0f, 0.0f)) *
			MatrixPerspectiveLH_DX(0.1f, 100.0f, float(width) / float(height), kPiDiv4),
			pUB->GetMappedDataAsT<float>());

		//float f = fmodf((float)GetTickCount64(), 360.0f) / 360.0f;
		BvTextureView *pRenderTargets[] = { pSwapChain->GetCurrentTextureView() };
		ClearColorValue cl[] = { ClearColorValue(
			0.1f,
			0.1f,
			0.3f) };
		auto renderTarget = RenderTargetDesc::AsSwapChain(pRenderTargets[0], *cl);
		pGraphicsContext->BeginQuery(pQuery);
		pGraphicsContext->SetRenderTarget(renderTarget);
		pGraphicsContext->SetGraphicsPipeline(pPSO);
		pGraphicsContext->SetViewport({ 0.0f, 0.0f, (f32)width, (f32)height, 0.0f, 1.0f });
		pGraphicsContext->SetScissor({ 0, 0, width, height });
		pGraphicsContext->SetVertexBufferView(pVBView);
		pGraphicsContext->SetShaderResource(pUBView, 0, 0, 0);
		pGraphicsContext->Draw(3);
		pGraphicsContext->EndQuery(pQuery);
		pGraphicsContext->Signal();

		pSwapChain->Present(false);

		pGraphicsContext->Flush();
	}

	BvApplication::Shutdown();

	return 0;
}


ShaderByteCodeDesc GetVS()
{
	static const char* pShader =
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

	ShaderDesc compDesc = { "main", ShaderStage::kVertex, ShaderLanguage::kGLSL };
	BvString errBlob;
	auto result = g_pCompiler->CompileFromMemory((u8*)pShader, strlen(pShader), compDesc, g_pVS);
	BvAssert(result, "Invalid Shader");

	ShaderByteCodeDesc desc;
	desc.m_pByteCode = g_pVS->GetBufferPointer();
	desc.m_ByteCodeSize = g_pVS->GetBufferSize();
	desc.m_pEntryPoint = "main";
	desc.m_ShaderStage = ShaderStage::kVertex;

	return desc;
}


ShaderByteCodeDesc GetPS()
{
	static const char* pShader =
		R"raw(
			#version 450

			layout (location = 0) in vec4 inColor;

			layout (location = 0) out vec4 outColor;

			void main()
			{
				outColor = inColor;
			}
		)raw";

	ShaderDesc compDesc = { "main", ShaderStage::kPixelOrFragment, ShaderLanguage::kGLSL };
	BvString errBlob;
	auto result = g_pCompiler->CompileFromMemory((u8*)pShader, strlen(pShader), compDesc, g_pPS);
	BvAssert(result, "Invalid Shader");

	ShaderByteCodeDesc desc;
	desc.m_pByteCode = g_pPS->GetBufferPointer();
	desc.m_ByteCodeSize = g_pPS->GetBufferSize();
	desc.m_pEntryPoint = "main";
	desc.m_ShaderStage = ShaderStage::kPixelOrFragment;

	return desc;
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