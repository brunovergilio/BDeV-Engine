#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/HID/BvKeyboard.h"
#include "BDeV/Core/System/HID/BvMouse.h"
#include "UIOverlay.h"
#include "BDeV/Core/RenderAPI/BvRenderEngine.h"
#include "FPSCounter.h"


class SampleBase
{
public:
	SampleBase() = default;
	virtual ~SampleBase() = default;

	void Initialize();
	void Update();
	void Render();
	void Shutdown();

	virtual void OnInitialize() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnRender() = 0;
	virtual void OnShutdown() = 0;

	virtual void OnInitializeUI();
	virtual void OnUpdateUI();
	virtual void OnRenderUI();

	virtual bool IsDone() { return m_IsDone; }

protected:
	BvApplication m_App;
	BvSharedLib m_RenderLib;
	BvRenderDeviceCreateDesc m_RenderDeviceDesc;
	BvWindow* m_pWindow = nullptr;
	IBvRenderEngine* m_pEngine = nullptr;
	BvRCRef<IBvRenderDevice> m_Device;
	BvRCRef<IBvCommandContext> m_Context;
	BvRCRef<IBvSwapChain> m_SwapChain;
	BvRCRef<IBvShaderCompiler> m_ShaderCompiler;
	UIOverlay m_Overlay;
	BvKeyboard m_Keyboard;
	BvMouse m_Mouse;
	FPSCounter m_FPSCounter;
	i64 m_Prev = 0;
	i64 m_Curr = 0;
	f32 m_Dt = 0.0f;
	bool m_IsDone = false;
	bool m_UseOverlay = true;
};


#define SAMPLE_MAIN(sampleObj)		\
int main(int argc, char* argv[])	\
{									\
	sampleObj sample;				\
	sample.Initialize();			\
	while (!sample.IsDone())		\
	{								\
		sample.Update();			\
		sample.Render();			\
	}								\
	sample.Shutdown();				\
									\
	return 0;						\
}