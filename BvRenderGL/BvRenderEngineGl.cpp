#include "BvRenderEngineGl.h"
#include "BvContextGl.h"
#include "BvRenderDeviceGl.h"
#include "BvDebugReportGl.h"


class BvRenderEngineGlHelper
{
public:
	static BvRenderEngineGl* Create()
	{
		static bool initialized = false;
		if (!initialized)
		{
			initialized = true;
			s_pEngine = BV_NEW(BvRenderEngineGl)();
			if (s_pEngine->GetGPUs().Size() == 0)
			{
				Destroy();
				s_pEngine = nullptr;
			}
		}
		else if (s_pEngine)
		{
			s_pEngine->AddRef();
		}

		return s_pEngine;
	}

	static void Destroy()
	{
		BV_DELETE_IN_PLACE(s_pEngine);
	}

private:
	static BvRenderEngineGl* s_pEngine;
};
BvRenderEngineGl* BvRenderEngineGlHelper::s_pEngine = nullptr;


const GPUList& BvRenderEngineGl::GetGPUs() const
{
	return m_GPUs;
}


bool BvRenderEngineGl::CreateRenderDeviceImpl(const BvRenderDeviceCreateDesc& deviceCreateDesc, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || !(BV_GL_IS_TYPE_VALID(objId, BvRenderDevice)))
	{
		return false;
	}

	BV_ASSERT_ONCE(m_pDevice == nullptr, "Render Device has already been created");
	if (!m_pDevice)
	{
		m_pDevice = BV_NEW(BvRenderDeviceGl)(this, *m_GPUs[0], deviceCreateDesc);
		
		if (!m_pDevice->IsValid())
		{
			m_pDevice->Release();
			m_pDevice = nullptr;

			return false;
		}
	}

	*ppObj = m_pDevice;

	return true;
}


BvRenderEngineGl::BvRenderEngineGl()
{
	Create();
}


BvRenderEngineGl::~BvRenderEngineGl()
{
	Destroy();
}


void BvRenderEngineGl::Create()
{
	auto result = InitializeOpenGL(m_GPUInfo);
	if (!result)
	{
		BV_ERROR("OpenGL", "Couldn't initialize OpenGL!");
		return;
	}

	auto& pGPU = m_GPUs.EmplaceBack(BV_NEW(BvGPUInfo)());
	strncpy(pGPU->m_DeviceName, m_GPUInfo.deviceName, BvGPUInfo::kMaxDeviceNameSize);
	auto& group = pGPU->m_ContextGroups.EmplaceBack();
	group.m_DedicatedCommandType = CommandType::kGraphics;
	group.m_GroupIndex = 0;
	group.m_MaxContextCount = 16;
	group.m_SupportedCommandTypes.EmplaceBack(CommandType::kGraphics);
	group.m_SupportedCommandTypes.EmplaceBack(CommandType::kCompute);
	group.m_SupportedCommandTypes.EmplaceBack(CommandType::kTransfer);

	if (GLEW_NVX_gpu_memory_info)
	{
		GLint dedicated = 0;
		glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &dedicated);
		pGPU->m_DeviceMemory = dedicated * 1024ull;
	}
	else if (GLEW_ATI_meminfo)
	{
		GLint texMem[4]{};
		glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, texMem);
		pGPU->m_DeviceMemory = texMem[0] * 1024ull;
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

	if (m_GPUs.Size() > 0)
	{
		BV_DELETE(m_GPUs[0]);
		m_GPUs.Clear();
	}

	ShutdownOpenGL();
}


void BvRenderEngineGl::SelfDestroy()
{
	BvRenderEngineGlHelper::Destroy();
}


namespace BvRenderGl
{
	extern "C"
	{
		BV_API bool CreateRenderEngine(const BvUUID& objId, void** ppObj)
		{
			if (!ppObj || (objId != BV_OBJECT_ID(BvRenderEngineGl) && objId != BV_OBJECT_ID(IBvRenderEngine)))
			{
				return false;
			}

			*ppObj = BvRenderEngineGlHelper::Create();

			return *ppObj != nullptr;
		}
	}
}