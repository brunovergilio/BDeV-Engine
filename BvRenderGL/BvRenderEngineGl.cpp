#include "BvRenderEngineGl.h"
#include "BvContextGl.h"


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


BvRenderDevice* const BvRenderEngineGl::CreateRenderDevice(const DeviceCreateDesc& deviceDesc, u32 gpuIndex)
{

}


bool BvRenderEngineGl::IsExtensionSupported(const char* const pExtension)
{

}


void BvRenderEngineGl::Create()
{
	m_pContext = BvRenderGl::CreateGLContext();
}


void BvRenderEngineGl::Destroy()
{
	BvRenderGl::DestroyGLContext();
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