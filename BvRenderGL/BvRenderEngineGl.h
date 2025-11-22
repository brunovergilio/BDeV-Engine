#pragma once


#include "BDeV/Core/RenderAPI/BvRenderEngine.h"
#include "BvGPUInfoGl.h"


class BvRenderDeviceGl;
class BvDebugReportGl;


BV_OBJECT_DEFINE_ID(BvRenderEngineGl, "dfe816d7-ebc5-466c-963d-c14d2f6ed846");
class BvRenderEngineGl final : public IBvRenderEngine
{
	BV_NOCOPYMOVE(BvRenderEngineGl);

public:
	const GPUList& GetGPUs() const override;
	bool CreateRenderDeviceImpl(const BvRenderDeviceCreateDesc& deviceCreateDesc, const BvUUID& objId, void** ppObj) override;

private:
	friend class BvRenderEngineGlHelper;

	BvRenderEngineGl();
	~BvRenderEngineGl();

	void Create();
	void Destroy();
	void SelfDestroy() override;

private:
#if defined(BV_DEBUG)
	BvDebugReportGl* m_pDebugReport = nullptr;
#endif
	BvRenderDeviceGl* m_pDevice = nullptr;
	BvDeviceInfoGl m_GPUInfo{};
	GPUList m_GPUs;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvRenderEngineGl);


namespace BvRenderGl
{
	extern "C"
	{
		BV_API bool CreateRenderEngine(const BvUUID& objId, void** ppObj);
	}
}