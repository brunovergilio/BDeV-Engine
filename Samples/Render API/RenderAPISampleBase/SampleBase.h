#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/HID/BvKeyboard.h"
#include "BDeV/Core/System/HID/BvMouse.h"
#include "UIOverlay.h"
#include "BDeV/Core/RenderAPI/BvRenderEngine.h"
#include "BDeV/Core/RenderAPI/BvTextureLoader.h"
#include "FPSCounter.h"
#include "Camera.h"


class SampleBase
{
public:
	SampleBase() = default;
	virtual ~SampleBase() = default;

	void Initialize();
	void Update();
	void Render();
	void Shutdown();
	BvRCRef<IBvShader> CompileShader(const char* pSource, size_t length, ShaderStage stage);

	virtual void OnInitialize() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnRender() = 0;
	virtual void OnShutdown() = 0;

	virtual void OnInitializeUI();
	virtual void OnUpdateUI();
	virtual void OnRenderUI();

	void BeginDrawDefaultUI();
	void EndDrawDefaultUI();

	virtual bool IsDone() { return m_IsDone; }

protected:
	BvApplication m_App;
	BvSharedLib m_RenderLib;
	BvSharedLib m_ToolsLib;
	BvRenderDeviceCreateDesc m_RenderDeviceDesc;
	BvWindow* m_pWindow = nullptr;
	BvRCRef<IBvRenderEngine> m_pEngine;
	BvRCRef<IBvRenderDevice> m_Device;
	BvRCRef<IBvCommandContext> m_Context;
	BvRCRef<IBvSwapChain> m_SwapChain;
	BvRCRef<IBvShaderCompiler> m_SpvCompiler;
	BvRCRef<IBvTextureLoader> m_TextureLoader;
	UIOverlay m_Overlay;
	Camera m_Camera;
	BvKeyboard m_Keyboard;
	BvMouse m_Mouse;
	FPSCounter m_FPSCounter;
	BvString m_AppName;
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