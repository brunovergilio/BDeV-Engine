#pragma once


#include "BDeV/Core/RenderAPI/BvRenderEngine.h"
#include "BDeV/Core/System/Library/BvSharedLib.h"
#include "BvCommonVk.h"


class BvDebugReportVk;
class BvRenderDeviceVk;


struct BvRenderDeviceCreateDescVk : BvRenderDeviceCreateDesc
{
};


BV_OBJECT_DEFINE_ID(BvRenderEngineVk, "eb31d72c-fe50-4284-ab0c-a5dbccf3c72d");
class BvRenderEngineVk final : public BvRenderEngine
{
	BV_NOCOPYMOVE(BvRenderEngineVk);

public:
	bool CreateRenderDevice(const BvRenderDeviceCreateDesc& deviceCreateDesc, BvRenderDevice** ppObj) override;
	bool CreateRenderDeviceVk(const BvRenderDeviceCreateDescVk& deviceDesc, BvRenderDeviceVk** ppObj);

	static BvRenderEngineVk* GetInstance();

	BvRenderEngineVk();
	~BvRenderEngineVk();

	BV_INLINE VkInstance GetHandle() const { return m_Instance; }
	BV_INLINE bool HasDebugUtils() const { return m_HasDebugUtils; }

	BV_OBJECT_IMPL_INTERFACE(BvRenderEngineVk, BvRenderEngine);

private:
	void Create();
	void Destroy();

private:
	BvSharedLib m_VulkanLib;
	VkInstance m_Instance = VK_NULL_HANDLE;
	BvVector<BvRenderDeviceVk*> m_Devices;
	BvDebugReportVk* m_pDebugReport = nullptr;
	bool m_HasDebugUtils = false;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvRenderEngineVk);


namespace BvRenderVk
{
	extern "C"
	{
		BV_API bool CreateRenderEngine(BvRenderEngine** ppObj);
	}
}