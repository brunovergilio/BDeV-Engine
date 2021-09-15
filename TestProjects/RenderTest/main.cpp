#include "BvRenderVK/BvRenderEngineVk.h"
#include "BvCore/System/Window/BvNativeWindow.h"
#include "BvRenderVK/BvSwapChainVk.h"
#include "BvRender/BvShaderResource.h"
#include "BvCore/System/File/BvFile.h"
#include "BvRenderVK/BvFramebufferVk.h"
#include "BvRenderVK/BvRenderPassVk.h"
#include "BvRenderVK/BvTextureViewVk.h"
#include "BvRenderVK/BvSemaphoreVk.h"
#include "BvRenderVK/BvCommandBufferVk.h"
#include "BvRenderVK/BvFenceVk.h"
#include "BvCore/System/File/BvFileSystem.h"
#include "BvShaderTools/BvShaderCompiler.h"
#include "BvCore/System/Input/BvKeyboard.h"
#include "BvMath/BvMath.h"


struct PosColorVertex
{
	Float3 position;
	Float4 color;
};


IBvShaderCompiler* g_pCompiler = GetShaderCompiler();
IBvShaderBlob* g_pVS = nullptr;
IBvShaderBlob* g_pPS = nullptr;

ShaderByteCodeDesc GetVS();
ShaderByteCodeDesc GetPS();
BvBuffer* CreateVB(BvRenderDevice* pDevice);
BvBuffer* CreateUB(BvRenderDevice* pDevice);

int main()
{
	auto pEngine = GetRenderEngineVk();
	auto pDevice = pEngine->CreateRenderDevice();

	//BufferDesc bufferDesc;
	//bufferDesc.m_UsageFlags = BufferUsage::kVertexBuffer | BufferUsage::kTransferDst;
	//auto pBuffer = pDevice->CreateBuffer(bufferDesc);

	SwapChainDesc swapChainDesc;
	swapChainDesc.m_pName = "Test";
	auto pGraphicsQueue = pDevice->GetGraphicsQueue();
	BvSwapChain* pSwapChain = pDevice->CreateSwapChain(swapChainDesc, *pGraphicsQueue);
	BvKeyboard keyboard;
	auto& window = pSwapChain->GetWindow();
	keyboard.Link(window);

	// Create uniform buffer

	RenderPassDesc renderPassDesc;
	RenderPassTargetDesc renderPassTargetDesc;
	renderPassTargetDesc.m_Format = Format::kBGRA8_UNorm;
	renderPassTargetDesc.m_IsOffscreen = false;
	renderPassDesc.m_RenderTargets.PushBack(renderPassTargetDesc);
	auto pRenderPass = pDevice->CreateRenderPass(renderPassDesc);

	BvCommandPool* pCommandPools[2];
	pCommandPools[0] = pDevice->CreateCommandPool();
	pCommandPools[1] = pDevice->CreateCommandPool();
	BvVector<BvCommandBuffer*> commandBuffers[2];
	commandBuffers[0].Resize(pSwapChain->GetDesc().m_SwapChainImageCount);
	commandBuffers[1].Resize(pSwapChain->GetDesc().m_SwapChainImageCount);
	pCommandPools[0]->AllocateCommandBuffers(commandBuffers[0].Size(), commandBuffers[0].Data());
	pCommandPools[1]->AllocateCommandBuffers(commandBuffers[1].Size(), commandBuffers[1].Data());

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

	Store44(MatrixLookAtLH(VectorSet(0.0f, 0.0f, -5.0f, 1.0f), VectorSet(0.0f, 0.0f, 1.0f, 1.0f), VectorSet(0.0f, 1.0f, 0.0f)) *
		MatrixPerspectiveLH_DX(0.1f, 100.0f, float(swapChainDesc.m_WindowDesc.m_Width) / float(swapChainDesc.m_WindowDesc.m_Height), kPiDiv4),
		pUB->GetMappedDataAsT<float>());

	pSet->SetBuffer(0, pUBView);
	pSet->Update();

	GraphicsPipelineStateDesc pipelineDesc;
	pipelineDesc.m_ShaderStages.PushBack(GetVS());
	pipelineDesc.m_ShaderStages.PushBack(GetPS());
	pipelineDesc.m_BlendStateDesc.m_BlendAttachments.PushBack(BlendAttachmentStateDesc());
	pipelineDesc.m_pRenderPass = pRenderPass;
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
	BvVector<BvSemaphore *> renderSemaphores[2];
	renderSemaphores[0].Resize(swapChainCount);
	for (auto&& smr : renderSemaphores[0]) { smr = pDevice->CreateSemaphore(0); }
	renderSemaphores[1].Resize(swapChainCount);
	for (auto&& smr : renderSemaphores[1]) { smr = pDevice->CreateSemaphore(0); }
	
	BvVector<u64> renderSemaphoreValues[2];
	renderSemaphoreValues[0].Resize(swapChainCount);
	renderSemaphoreValues[1].Resize(swapChainCount);
	
	auto cbIndex = 0;
	auto cpIndex = 0;
	BvCommandBuffer** ppCurrCBs = commandBuffers[1].Data();
	while (true)
	{
		keyboard.Update();
		if (!window.IsResizing())
		{
			if (keyboard.IsKeyPressed(BvKey::kReturn))
			{
				break;
			}
			if (keyboard.IsKeyPressed(BvKey::kB))
			{
				window.ChangeMode(WindowMode::kBordeless);
				continue;
			}
			if (keyboard.IsKeyPressed(BvKey::kW))
			{
				window.ChangeMode(WindowMode::kWindowed);
				continue;
			}
			if (keyboard.IsKeyPressed(BvKey::kU))
			{
				window.Resize(640, 480);
				continue;
			}
			if (keyboard.IsKeyPressed(BvKey::kI))
			{
				window.Resize(800, 600);
				continue;
			}
			if (keyboard.IsKeyPressed(BvKey::kM))
			{
				window.Move(100, 100);
				continue;
			}
			if (keyboard.IsKeyPressed(BvKey::kN))
			{
				window.Move(200, 200);
				continue;
			}

			if (cbIndex % swapChainCount == 0)
			{
				cpIndex ^= 1;
				ppCurrCBs = commandBuffers[cpIndex].Data();
				for (auto i = 0; i < renderSemaphores[cpIndex].Size(); i++)
				{
					renderSemaphores[cpIndex][i]->Wait(renderSemaphoreValues[cpIndex][i]);
				}
				pCommandPools[cpIndex]->Reset();
				cbIndex = 0;
			}

			//float f = fmodf((float)GetTickCount64(), 360.0f) / 360.0f;
			BvTextureView *pRenderTargets[] = { pSwapChain->GetTextureView(pSwapChain->GetCurrentImageIndex()) };
			ClearColorValue cl[] = { ClearColorValue(
				0.1f,
				0.1f,
				0.3f) };
			ppCurrCBs[cbIndex]->Begin();
			ResourceBarrierDesc barrier;
			barrier.pTexture = pRenderTargets[0]->GetTexture();
			barrier.srcLayout = ResourceState::kPresent;
			barrier.dstLayout = ResourceState::kRenderTarget;
			ppCurrCBs[cbIndex]->ResourceBarrier(1, &barrier);
			ppCurrCBs[cbIndex]->BeginRenderPass(pRenderPass, pRenderTargets, cl);
			ppCurrCBs[cbIndex]->SetPipeline(pPSO);
			ppCurrCBs[cbIndex]->SetViewport({ 0.0f, 0.0f, (f32)window.GetWidth(), (f32)window.GetHeight(), 0.0f, 1.0f });
			ppCurrCBs[cbIndex]->SetScissor({ 0, 0, window.GetWidth(), window.GetHeight() });
			ppCurrCBs[cbIndex]->SetVertexBufferView(pVBView);
			ppCurrCBs[cbIndex]->SetShaderResourceParams(1, &pSet);
			ppCurrCBs[cbIndex]->Draw(3);
			ppCurrCBs[cbIndex]->EndRenderPass();
			ppCurrCBs[cbIndex]->End();

			BvCommandBuffer * pCBs[] = { ppCurrCBs[cbIndex] };
			BvSemaphore * pSignals[] = { renderSemaphores[cpIndex][cbIndex] };
			u64 pSignalValues[] = { renderSemaphoreValues[cpIndex][cbIndex] };
			SubmitInfo si;
			si.commandBufferCount = 1;
			si.ppCommandBuffers = pCBs;
			si.signalSemaphoreCount = 1;
			si.ppSignalSemaphores = pSignals;
			si.pSignalValues = pSignalValues;

			pGraphicsQueue->Submit(si);
			pGraphicsQueue->Execute();
			pSwapChain->Present(false);
			cbIndex++;
		}
	}

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
	g_pVS = g_pCompiler->CompileFromMemory((u8*)pShader, strlen(pShader), compDesc, &errBlob);

	ShaderByteCodeDesc desc;
	desc.m_pByteCode = (u8*)g_pVS->GetBufferPointer();
	desc.m_ByteCodeSize = g_pVS->GetBufferSize();
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
	g_pPS = g_pCompiler->CompileFromMemory((u8*)pShader, strlen(pShader), compDesc, &errBlob);

	ShaderByteCodeDesc desc;
	desc.m_pByteCode = (u8*)g_pPS->GetBufferPointer();
	desc.m_ByteCodeSize = g_pPS->GetBufferSize();
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