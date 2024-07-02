#include "BDeV/RenderAPI/BvRenderEngine.h"
#include "BDeV/System/Window/BvWindow.h"
#include "BDeV/RenderAPI/BvSwapChain.h"
#include "BDeV/RenderAPI/BvShaderResource.h"
#include "BDeV/System/File/BvFile.h"
#include "BDeV/RenderAPI/BvRenderPass.h"
#include "BDeV/RenderAPI/BvTextureView.h"
#include "BDeV/RenderAPI/BvSemaphore.h"
#include "BDeV/System/File/BvFileSystem.h"
#include "BvRenderTools/BvShaderCompiler.h"
#include "BDeV/System/HID/BvKeyboard.h"
#include "BDeV/Math/BvMath.h"
#include "BDeV/System/Platform/BvPlatform.h"
#include "BDeV/System/Library/BvSharedLib.h"
#include "BDeV/Container/BvRobinSet.h"


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
	BvPlatform::Initialize();

	BvSharedLib renderToolsLib("BvRenderTools.dll");
	typedef IBvShaderCompiler* (*pFNGetShaderCompiler)();
	pFNGetShaderCompiler compilerFn = renderToolsLib.GetProcAddressT<pFNGetShaderCompiler>("GetShaderCompiler");
	auto pCompiler = compilerFn();
	ShaderDesc compDesc = { "main", ShaderStage::kVertex, ShaderLanguage::kGLSL };
	pCompiler->CompileFromFile("D:\\Bruno\\C++\\test.vert", compDesc);
	BvSharedLib renderVkLib("BvRenderVk.dll");
	typedef BvRenderEngine* (*pFNCreateRenderEngine)();
	pFNCreateRenderEngine getRenderEngineFn = renderVkLib.GetProcAddressT<pFNCreateRenderEngine>("CreateRenderEngine");
	typedef void (*pFNDestroyRenderEngine)();
	pFNDestroyRenderEngine get2RenderEngineFn = renderVkLib.GetProcAddressT<pFNDestroyRenderEngine>("DestroyRenderEngine");

	g_pCompiler = compilerFn();

	auto pEngine = getRenderEngineFn();
	auto pDevice = pEngine->CreateRenderDevice();

	//BufferDesc bufferDesc;
	//bufferDesc.m_UsageFlags = BufferUsage::kVertexBuffer | BufferUsage::kTransferDst;
	//auto pBuffer = pDevice->CreateBuffer(bufferDesc);

	auto pKeyboard = Input::GetKeyboard();

	WindowDesc windowDesc;
	windowDesc.m_X += 100;
	windowDesc.m_Y += 100;
	auto pWindow = BvPlatform::CreateWindow(windowDesc);

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
		BvPlatform::ProcessOSEvents();
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
		ResourceBarrierDesc barrier;
		barrier.m_pTexture = pRenderTargets[0]->GetTexture();
		barrier.m_SrcLayout = ResourceState::kPresent;
		barrier.m_DstLayout = ResourceState::kRenderTarget;
		//pGraphicsContext->BeginRenderPass(pRenderPass, pRenderTargets, cl);
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
		//pGraphicsContext->EndRenderPass();
		pGraphicsContext->Signal();

		pSwapChain->Present(false);

		pGraphicsContext->Flush();
	}

	BvPlatform::Shutdown();

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
	g_pVS = g_pCompiler->CompileFromMemory((u8*)pShader, strlen(pShader), compDesc);
	BvAssert(g_pVS->IsValid(), "Invalid Shader");

	ShaderByteCodeDesc desc;
	desc.m_pByteCode = g_pVS->GetShaderBlob().Data();
	desc.m_ByteCodeSize = g_pVS->GetShaderBlob().Size();
	desc.m_EntryPoint = "main";
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
	g_pPS = g_pCompiler->CompileFromMemory((u8*)pShader, strlen(pShader), compDesc);
	BvAssert(g_pPS->IsValid(), "Invalid Shader");

	ShaderByteCodeDesc desc;
	desc.m_pByteCode = g_pPS->GetShaderBlob().Data();
	desc.m_ByteCodeSize = g_pPS->GetShaderBlob().Size();
	desc.m_EntryPoint = "main";
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