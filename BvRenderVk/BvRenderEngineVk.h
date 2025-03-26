#pragma once


#include "BDeV/Core/RenderAPI/BvRenderEngine.h"
#include "BDeV/Core/System/Library/BvSharedLib.h"
#include "BvCommonVk.h"


class BvDebugReportVk;
class BvRenderDeviceVk;


struct BvRenderDeviceCreateDescVk : BvRenderDeviceCreateDesc
{
};


//BV_OBJECT_DEFINE_ID(IBvRenderEngineVk, "eb31d72c-fe50-4284-ab0c-a5dbccf3c72d");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvRenderEngineVk);


class BvRenderEngineVk final : public IBvRenderEngine
{
	BV_NOCOPYMOVE(BvRenderEngineVk);

public:
	IBvRenderDevice* CreateRenderDeviceImpl(const BvRenderDeviceCreateDesc& deviceCreateDesc) override;
	BV_INLINE const BvVector<BvGPUInfo>& GetGPUs() const override { return m_GPUs; }
	BV_INLINE VkInstance GetHandle() const { return m_Instance; }
	BV_INLINE bool HasDebugUtils() const { return m_HasDebugUtils; }

	//BV_OBJECT_IMPL_INTERFACE(IBvRenderEngineVk, IBvRenderEngine);

private:
	friend class BvRenderEngineVkHelper;

	BvRenderEngineVk();
	~BvRenderEngineVk();

	void Create();
	void Destroy();

private:
	BvSharedLib m_VulkanLib;
	VkInstance m_Instance = VK_NULL_HANDLE;
	BvVector<BvGPUInfo> m_GPUs;
	BvVector<BvRenderDeviceVk*> m_Devices;
	BvDebugReportVk* m_pDebugReport = nullptr;
	bool m_HasDebugUtils = false;
};


namespace BvRenderVk
{
	extern "C"
	{
		BV_API IBvRenderEngine* CreateRenderEngine();
		BV_API BvRenderEngineVk* CreateRenderEngineVk();
	}
}