#include "BDeV/RenderAPI/BvRenderEngine.h"
#include "BDeV/System/Window/BvWindow.h"
#include "BDeV/RenderAPI/BvSwapChain.h"
#include "BDeV/RenderAPI/BvShaderResource.h"
#include "BDeV/System/File/BvFile.h"
#include "BDeV/RenderAPI/BvRenderPass.h"
#include "BDeV/RenderAPI/BvTextureView.h"
#include "BDeV/RenderAPI/BvSemaphore.h"
#include "BDeV/RenderAPI/BvCommandBuffer.h"
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
	BvSharedLib renderVkLib("BvRenderVk.dll");
	typedef BvRenderEngine* (*pFNCreateRenderEngine)();
	typedef void(*pFNDestroyRenderEngine)();
	pFNCreateRenderEngine createRenderEngineFn = renderVkLib.GetProcAddressT<pFNCreateRenderEngine>("CreateRenderEngine");
	pFNDestroyRenderEngine destroyRenderEngineFn = renderVkLib.GetProcAddressT<pFNDestroyRenderEngine>("DestroyRenderEngine");

	g_pCompiler = compilerFn();

	auto pEngine = createRenderEngineFn();
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
	swapChainDesc.m_pName = "Test";
	auto pGraphicsQueue = pDevice->GetGraphicsQueue();
	BvSwapChain* pSwapChain = pDevice->CreateSwapChain(pWindow, swapChainDesc, *pGraphicsQueue);

	// Create uniform buffer

	RenderPassDesc renderPassDesc;
	RenderPassTargetDesc renderPassTargetDesc;
	renderPassTargetDesc.m_Format = Format::kBGRA8_UNorm;
	renderPassTargetDesc.m_StateAfter = ResourceState::kPresent;
	renderPassDesc.m_RenderTargets.PushBack(renderPassTargetDesc);
	auto pRenderPass = pDevice->CreateRenderPass(renderPassDesc);

	BvVector<BvCommandPool*> commandPools; commandPools.Resize(pSwapChain->GetDesc().m_SwapChainImageCount);
	for (auto&& pCommandPool : commandPools) { pCommandPool = pDevice->CreateCommandPool(); }
	BvVector<BvCommandBuffer*> commandBuffers; commandBuffers.Resize(commandPools.Size());
	for (auto i = 0; i < commandBuffers.Size(); i++) { commandBuffers[i] = commandPools[i]->AllocateCommandBuffer(); }

	ShaderResourceLayoutDesc shaderResourceLayoutDesc;
	shaderResourceLayoutDesc.AddResource(ShaderResourceType::kConstantBuffer, ShaderStage::kVertex, 0);
	auto pShaderResourceLayout = pDevice->CreateShaderResourceLayout(shaderResourceLayoutDesc);

	auto pShaderResourceSetPool = pDevice->CreateShaderResourceSetPool();
	auto pSet = pShaderResourceSetPool->AllocateSet(pShaderResourceLayout);

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

	pSet->SetBuffer(0, pUBView);
	pSet->Update();

	GraphicsPipelineStateDesc pipelineDesc;
	pipelineDesc.m_ShaderStages.PushBack(GetVS());
	pipelineDesc.m_ShaderStages.PushBack(GetPS());
	pipelineDesc.m_BlendStateDesc.m_BlendAttachments.PushBack(BlendAttachmentStateDesc());
	//pipelineDesc.m_pRenderPass = pRenderPass;
	pipelineDesc.m_RenderTargetFormats[0] = pSwapChain->GetDesc().m_Format;
	pipelineDesc.m_pShaderResourceLayout = pShaderResourceLayout;
	pipelineDesc.m_InputAssemblyStateDesc.m_Topology = Topology::kTriangleList;
	
	pipelineDesc.m_VertexBindingDesc.Resize(1);
	pipelineDesc.m_VertexBindingDesc[0].m_Binding = 0;
	pipelineDesc.m_VertexBindingDesc[0].m_InputRate = InputRate::kPerVertex;
	pipelineDesc.m_VertexBindingDesc[0].m_Stride = sizeof(PosColorVertex);

	pipelineDesc.m_VertexAttributeDesc.Resize(2);
	pipelineDesc.m_VertexAttributeDesc[0].m_Binding = 0;
	pipelineDesc.m_VertexAttributeDesc[0].m_Format = Format::kRGB32_Float;
	pipelineDesc.m_VertexAttributeDesc[0].m_Location = 0;
	pipelineDesc.m_VertexAttributeDesc[1].m_Binding = 0;
	pipelineDesc.m_VertexAttributeDesc[1].m_Format = Format::kRGBA32_Float;
	pipelineDesc.m_VertexAttributeDesc[1].m_Location = 1;
	pipelineDesc.m_VertexAttributeDesc[1].m_Offset = sizeof(Float3);

	auto pPSO = pDevice->CreateGraphicsPipeline(pipelineDesc);

	g_pCompiler->DestroyShader(g_pVS);
	g_pCompiler->DestroyShader(g_pPS);

	auto swapChainCount = pSwapChain->GetDesc().m_SwapChainImageCount;
	BvVector<BvSemaphore*> renderSemaphores;
	renderSemaphores.Resize(swapChainCount);
	for (auto&& smr : renderSemaphores) { smr = pDevice->CreateSemaphore(); }
	
	BvVector<u64> renderSemaphoreValues;
	renderSemaphoreValues.Resize(swapChainCount);
	
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

		currIndex = pSwapChain->GetCurrentImageIndex();
		renderSemaphores[currIndex]->Wait(renderSemaphoreValues[currIndex]++);
		commandPools[currIndex]->Reset();

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
		commandBuffers[currIndex]->Reset();
		commandBuffers[currIndex]->Begin();
		ResourceBarrierDesc barrier;
		barrier.pTexture = pRenderTargets[0]->GetTexture();
		barrier.srcLayout = ResourceState::kPresent;
		barrier.dstLayout = ResourceState::kRenderTarget;
		commandBuffers[currIndex]->ResourceBarrier(1, &barrier);
		//commandBuffers[currIndex]->BeginRenderPass(pRenderPass, pRenderTargets, cl);
		commandBuffers[currIndex]->SetRenderTargets(1, pRenderTargets, cl);
		commandBuffers[currIndex]->SetPipeline(pPSO);
		commandBuffers[currIndex]->SetViewport({ 0.0f, 0.0f, (f32)width, (f32)height, 0.0f, 1.0f });
		commandBuffers[currIndex]->SetScissor({ 0, 0, width, height });
		commandBuffers[currIndex]->SetVertexBufferView(pVBView);
		commandBuffers[currIndex]->SetShaderResourceParams(1, &pSet);
		commandBuffers[currIndex]->Draw(3);
		//commandBuffers[currIndex]->EndRenderPass();
		barrier.srcLayout = ResourceState::kRenderTarget;
		barrier.dstLayout = ResourceState::kPresent;
		commandBuffers[currIndex]->ResourceBarrier(1, &barrier);
		commandBuffers[currIndex]->End();

		BvCommandBuffer * pCBs[] = { commandBuffers[currIndex] };
		BvSemaphore * pSignals[] = { renderSemaphores[currIndex] };
		u64 pSignalValues[] = { renderSemaphoreValues[currIndex] };
		SubmitInfo si;
		si.commandBufferCount = 1;
		si.ppCommandBuffers = pCBs;
		si.signalSemaphoreCount = 1;
		si.ppSignalSemaphores = pSignals;
		si.pSignalValues = pSignalValues;

		pGraphicsQueue->Submit(si);
		pGraphicsQueue->Execute();
		pSwapChain->Present(false);
	}

	destroyRenderEngineFn();

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

	BufferDesc stagingBufferDesc;
	stagingBufferDesc.m_Size = sizeof(PosColorVertex) * 3;
	stagingBufferDesc.m_MemoryFlags = MemoryFlags::kUpload;
	stagingBufferDesc.m_UsageFlags = BufferUsage::kTransferSrc;
	auto pStaging = pDevice->CreateBuffer(stagingBufferDesc);
	memcpy(pStaging->Map(), verts, stagingBufferDesc.m_Size);

	BufferDesc vbBufferDesc;
	vbBufferDesc.m_Size = sizeof(PosColorVertex) * 3;
	vbBufferDesc.m_MemoryFlags = MemoryFlags::kDeviceLocal;
	vbBufferDesc.m_UsageFlags = BufferUsage::kVertexBuffer | BufferUsage::kTransferDst;
	auto pVB = pDevice->CreateBuffer(vbBufferDesc);

	auto pCP = pDevice->CreateCommandPool();
	auto pCB = pCP->AllocateCommandBuffer();

	pCB->Begin();
	pCB->CopyBuffer(pStaging, pVB);
	pCB->End();

	auto pQueue = pDevice->GetGraphicsQueue();
	SubmitInfo submitInfo;
	submitInfo.ppCommandBuffers = &pCB;
	submitInfo.commandBufferCount = 1;
	pQueue->Submit(submitInfo);
	pQueue->Execute();
	pQueue->WaitIdle();

	return pVB;
}

BvBuffer* CreateUB(BvRenderDevice* pDevice)
{
	BufferDesc uniformBufferDesc;
	uniformBufferDesc.m_Size = sizeof(Float44);
	uniformBufferDesc.m_MemoryFlags = MemoryFlags::kUpload;
	uniformBufferDesc.m_UsageFlags = BufferUsage::kUniformBuffer;
	auto pUniform = pDevice->CreateBuffer(uniformBufferDesc);
	pUniform->Map();
	
	return pUniform;
}