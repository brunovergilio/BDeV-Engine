#pragma once


#include "BDeV/Core/RenderAPI/BvRenderEngine.h"
#include "BDeV/Core/System/Library/BvSharedLib.h"
#include "BvCommonVk.h"


class BvDebugReportVk;
class BvRenderDeviceVk;


struct BvRenderDeviceCreateDescVk : BvRenderDeviceCreateDesc
{
};


class BvRenderEngineVk final : public BvRenderEngine
{
	BV_NOCOPYMOVE(BvRenderEngineVk);

public:
	BvRenderDevice* CreateRenderDevice(const BvRenderDeviceCreateDesc& deviceCreateDesc) override;
	BvRenderDeviceVk* CreateRenderDeviceVk(const BvRenderDeviceCreateDescVk& deviceDesc);

	static BvRenderEngineVk* GetInstance();

	BV_INLINE VkInstance GetHandle() const { return m_Instance; }
	BV_INLINE bool HasDebugUtils() const { return m_HasDebugUtils; }

private:
	BvRenderEngineVk();
	~BvRenderEngineVk();

	void Create();
	void Destroy();

private:
	BvSharedLib m_VulkanLib;
	VkInstance m_Instance = VK_NULL_HANDLE;
	BvVector<BvRenderDeviceVk*> m_Devices;
	BvDebugReportVk* m_pDebugReport = nullptr;
	bool m_HasDebugUtils = false;
};


namespace BvRenderVk
{
	extern "C"
	{
		BV_API BvRenderEngine* GetRenderEngine();
	}
}