#include "BvRenderEngineGl.h"
#include "BvContextGl.h"
#include "BvRenderDeviceGl.h"
#include "BvDebugReportGl.h"


BvRenderEngineGl::BvRenderEngineGl()
{
	Create();
}


BvRenderEngineGl::~BvRenderEngineGl()
{
	Destroy();
}


void BvRenderEngineGl::GetGPUInfo(const u32 index, BvGPUInfo& info) const
{
}


IBvRenderDevice* const BvRenderEngineGl::CreateRenderDevice(const DeviceCreateDesc& deviceDesc, u32 gpuIndex)
{
	if (!m_pDevice)
	{
		m_pDevice = new BvRenderDeviceGl(deviceDesc, m_GPUInfo);
	}

	return m_pDevice;
}


bool BvRenderEngineGl::IsExtensionSupported(const char* const pExtension)
{
	return false;
}


void BvRenderEngineGl::Create()
{
	auto result = InitializeOpenGL(m_GPUInfo);
	if (!result)
	{
		BV_ERROR("Couldn't initialize OpenGL!");
	}

#if defined(BV_DEBUG)
	m_pDebugReport = new BvDebugReportGl();
#endif
}


void BvRenderEngineGl::Destroy()
{
#if defined(BV_DEBUG)
	if (m_pDebugReport)
	{
		delete m_pDebugReport;
		m_pDebugReport = nullptr;
	}
#endif
}


namespace BvRenderGl
{
#if defined (BV_STATIC_LIB)
	BvRenderEngine* GetRenderEngine()
	{
		static BvRenderEngineGl s_Engine;
		return &s_Engine;
	}
#else
	static BvRenderEngineGl* s_pRenderEngineGl = nullptr;

	BvRenderEngine* CreateRenderEngine()
	{
		if (!s_pRenderEngineGl)
		{
			s_pRenderEngineGl = new BvRenderEngineGl();
		}
		return s_pRenderEngineGl;
	}

	void DestroyRenderEngine()
	{
		delete s_pRenderEngineGl;
		s_pRenderEngineGl = nullptr;
	}
#endif
}