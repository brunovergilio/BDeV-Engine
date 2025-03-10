#pragma once


#include "BDeV/RenderAPI/BvRenderEngine.h"
#include "BvGPUInfoGl.h"


class BvDebugReportGl;


class BvRenderEngineGl final : public IBvRenderEngine
{
	BV_NOCOPYMOVE(BvRenderEngineGl);

public:
	BvRenderEngineGl();
	~BvRenderEngineGl();

	void GetGPUInfo(const u32 index, BvGPUInfo& info) const override final;
	IBvRenderDevice* const CreateRenderDevice(const DeviceCreateDesc& deviceDesc, u32 gpuIndex) override final;

	BV_INLINE const char* const GetEngineName() const override final { return "BvRenderGl"; }

	bool IsExtensionSupported(const char* const pExtension);

private:
	void Create();
	void Destroy();

private:
	IBvRenderDevice* m_pDevice = nullptr;
#if defined(BV_DEBUG)
	BvDebugReportGl* m_pDebugReport = nullptr;
#endif
	BvGPUInfoGl m_GPUInfo{};
};


namespace BvRenderGl
{
#if defined (BV_STATIC_LIB)
	BV_PLUGIN_API IBvRenderEngine* GetRenderEngine();
#else
#if defined(BV_PLUGIN_DLL_EXPORT)
	BV_EXTERN_C
#endif
	BV_PLUGIN_API IBvRenderEngine* CreateRenderEngine();

#if defined(BV_PLUGIN_DLL_EXPORT)
	BV_EXTERN_C
#endif
	BV_PLUGIN_API void DestroyRenderEngine();
#endif
}