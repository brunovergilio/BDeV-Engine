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


int main()
{
	auto e = CreateRenderEngineVk();
	auto d = (BvRenderDeviceVk*)e->CreateRenderDevice(0, DeviceCreateDesc());

	WindowDesc wd;
	wd.m_Name = "Test";
	BvNativeWindow w(wd);
	w.Show();

	auto q = d->GetGraphicsQueue()[0];
	BvSwapChain* sc = d->CreateSwapChain(w, SwapChainDesc(), *q);

	RenderPassDesc rpD;
	RenderPassTargetDesc rptD;
	rptD.m_Format = Format::kBGRA8_UNorm;
	rptD.m_IsOffscreen = false;
	rpD.m_RenderTargets.PushBack(rptD);
	auto rp = d->CreateRenderPass(rpD);

	BvCommandPool* cps[2];
	cps[0] = d->CreateCommandPool(CommandPoolDesc());
	cps[1] = d->CreateCommandPool(CommandPoolDesc());
	BvVector<BvCommandBuffer*> cbs[2]; cbs[0].Resize(sc->GetDesc().m_SwapChainImageCount); cbs[1].Resize(sc->GetDesc().m_SwapChainImageCount);
	cps[0]->AllocateCommandBuffers(cbs[0].Size(), cbs[0].Data());
	cps[1]->AllocateCommandBuffers(cbs[1].Size(), cbs[1].Data());

	ShaderResourceLayoutDesc srld;
	//srld.AddResource(ShaderResourceType::kConstantBuffer, ShaderStage::kVertex | ShaderStage::kPixel, 0);
	//srld.AddResource(ShaderResourceType::kStructuredBuffer, ShaderStage::kCompute, 0, 0);
	//srld.AddResource(ShaderResourceType::kTexture, ShaderStage::kPixel, 1, 1);
	auto srl = d->CreateShaderResourceLayout(srld);

	BvFileSystem fileSys;
	BvFile vs = fileSys.OpenFile(R"raw(C:\Programming\C++\Graphics\Vulkan\Assets - Shaders\SimpleTriangle\vs.spv)raw");
	u8 * pvs = new u8[vs.GetSize()];
	vs.Read(pvs, vs.GetSize());
	BvFile ps = fileSys.OpenFile("C:\\Programming\\C++\\Graphics\\Vulkan\\Assets - Shaders\\SimpleTriangle\\fs.spv");
	u8 * pps = new u8[ps.GetSize()];
	ps.Read(pps, ps.GetSize());

	BvString errBlob;
	auto compiler = CreateShaderCompiler();
	ShaderDesc vsDesc = { "main", ShaderStage::kVertex, ShaderLanguage::kGLSL };
	auto compVs = compiler->CompileFromFile(R"raw(C:\Programming\C++\Graphics\Vulkan\Assets - Shaders\SimpleTriangle\vs.vert)raw", vsDesc, &errBlob);
	ShaderDesc psDesc = { "main", ShaderStage::kPixelOrFragment, ShaderLanguage::kGLSL };
	auto compPs = compiler->CompileFromFile(R"raw(C:\Programming\C++\Graphics\Vulkan\Assets - Shaders\SimpleTriangle\fs.frag)raw", psDesc, &errBlob);

	GraphicsPipelineStateDesc gpsd;
	//gpsd.m_ShaderStages.PushBack({ vs.GetSize(), pvs, "main", ShaderStage::kVertex });
	//gpsd.m_ShaderStages.PushBack({ ps.GetSize(), pps, "main", ShaderStage::kPixelOrFragment });
	gpsd.m_ShaderStages.PushBack({ compVs->GetBufferSize(), compVs->GetBufferPointer(), "main", ShaderStage::kVertex });
	gpsd.m_ShaderStages.PushBack({ compPs->GetBufferSize(), compPs->GetBufferPointer(), "main", ShaderStage::kPixelOrFragment });
	gpsd.m_BlendStateDesc.m_BlendAttachments.PushBack(BlendAttachmentStateDesc());
	gpsd.m_pRenderPass = rp;
	gpsd.m_pShaderResourceLayout = srl;

	auto ppso = d->CreateGraphicsPipeline(gpsd);

	compiler->DestroyShader(compVs);
	compiler->DestroyShader(compPs);
	DestroyShaderCompiler(compiler);

	auto scs = sc->GetDesc().m_SwapChainImageCount;
	BvVector<BvSemaphore *> smsr[2]; smsr[0].Resize(scs); smsr[1].Resize(scs);
	for (auto&& smr : smsr[0]) { smr = d->CreateSemaphore(0); }
	for (auto&& smr : smsr[1]) { smr = d->CreateSemaphore(0); }
	BvVector<u64> smvs[2]; smvs[0].Resize(scs); smvs[1].Resize(scs);
	MSG msg{};
	//auto scs = sc->GetTextures().Size();
	auto cbIndex = 0;
	auto poolIndex = 0;
	BvCommandBuffer** ppCb = cbs[1].Data();
	while (true)
	{
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (!(w.IsResizing() || w.IsPaused()))
		{
			if (GetAsyncKeyState('B') & 0x8000)
			{
				w.ChangeMode(WindowMode::kBordeless);
				continue;
			}
			if (GetAsyncKeyState('W') & 0x8000)
			{
				w.ChangeMode(WindowMode::kWindowed);
				continue;
			}
			if (GetAsyncKeyState('U') & 0x8000)
			{
				w.Resize(640, 480);
				continue;
			}
			if (GetAsyncKeyState('I') & 0x8000)
			{
				w.Resize(800, 600);
				continue;
			}
			if (GetAsyncKeyState('M') & 0x8000)
			{
				w.Move(100, 100);
				continue;
			}
			if (GetAsyncKeyState('N') & 0x8000)
			{
				w.Move(200, 200);
				continue;
			}

			//printf("%ud %ud - %d %d\n", w.GetWidth(), w.GetHeight(), w.GetPosX(), w.GetPosY());

			if (cbIndex % scs == 0)
			{
				poolIndex ^= 1;
				ppCb = cbs[poolIndex].Data();
				for (auto i = 0; i < smsr[poolIndex].Size(); i++)
				{
					smsr[poolIndex][i]->Wait(smvs[poolIndex][i], UINT64_MAX);
				}
				cps[poolIndex]->Reset();
				cbIndex = 0;
			}
			float f = fmodf((float)GetTickCount(), 360.0f) / 360.0f;

			BvTextureView *tex[] = { sc->GetTextureView(sc->GetCurrentImageIndex()) };
			ClearColorValue cl[] = { ClearColorValue(
				0.1f,
				0.1f,
				0.3f) };
			ppCb[cbIndex]->Begin();
			ResourceBarrierDesc barrier;
			barrier.pTexture = tex[0]->GetTexture();
			barrier.srcLayout = ResourceState::kPresent;
			barrier.dstLayout = ResourceState::kRenderTarget;
			ppCb[cbIndex]->ResourceBarrier(1, &barrier);
			ppCb[cbIndex]->BeginRenderPass(rp, tex, cl);
			ppCb[cbIndex]->SetPipeline(ppso);
			ppCb[cbIndex]->SetViewport({ 0.0f, 0.0f, (f32)w.GetWidth(), (f32)w.GetHeight(), 0.0f, 1.0f });
			ppCb[cbIndex]->SetScissor({ 0, 0, w.GetWidth(), w.GetHeight() });
			ppCb[cbIndex]->Draw(3);
			ppCb[cbIndex]->EndRenderPass();
			ppCb[cbIndex]->End();

			BvCommandBuffer * sicb[] = { ppCb[cbIndex] };
			BvSemaphore * sissm[] = { smsr[poolIndex][cbIndex] };
			u64 sismv[] = { smvs[poolIndex][cbIndex] };
			SubmitInfo si;
			si.commandBufferCount = 1;
			si.ppCommandBuffers = sicb;
			si.signalSemaphoreCount = 1;
			si.ppSignalSemaphores = sissm;
			si.pSignalValues = sismv;

			q->Submit(si);
			sc->Present(false);
			cbIndex++;
		}
	}

	DestroyRenderEngineVk(e);

	return 0;
}