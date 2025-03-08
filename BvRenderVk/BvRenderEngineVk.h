#pragma once


#include "BDeV/Core/RenderAPI/BvRenderEngine.h"
#include "BDeV/Core/System/Library/BvSharedLib.h"
#include "BvCommonVk.h"


class BvDebugReportVk;
class IBvRenderDeviceVk;


struct BvRenderDeviceCreateDescVk : BvRenderDeviceCreateDesc
{
};


BV_OBJECT_DEFINE_ID(IBvRenderEngineVk, "eb31d72c-fe50-4284-ab0c-a5dbccf3c72d");
class IBvRenderEngineVk : public IBvRenderEngine
{
	BV_NOCOPYMOVE(IBvRenderEngineVk);

public:
	virtual bool CreateRenderDeviceVk(const BvRenderDeviceCreateDescVk& deviceDesc, IBvRenderDeviceVk** ppObj) = 0;
	virtual VkInstance GetHandle() const = 0;
	virtual bool HasDebugUtils() const = 0;

protected:
	IBvRenderEngineVk() {}
	~IBvRenderEngineVk() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvRenderEngineVk);


class BvRenderEngineVk final : public IBvRenderEngineVk
{
	BV_NOCOPYMOVE(BvRenderEngineVk);

public:
	bool CreateRenderDevice(const BvRenderDeviceCreateDesc& deviceCreateDesc, IBvRenderDevice** ppObj) override;
	bool CreateRenderDeviceVk(const BvRenderDeviceCreateDescVk& deviceDesc, IBvRenderDeviceVk** ppObj) override;
	BV_INLINE const BvVector<BvGPUInfo>& GetGPUs() const override { return m_GPUs; }
	BV_INLINE VkInstance GetHandle() const override { return m_Instance; }
	BV_INLINE bool HasDebugUtils() const override { return m_HasDebugUtils; }

	BV_OBJECT_IMPL_INTERFACE(IBvRenderEngineVk, IBvRenderEngine);

	BvRenderEngineVk();
	~BvRenderEngineVk();

private:
	void Create();
	void Destroy();

private:
	BvSharedLib m_VulkanLib;
	VkInstance m_Instance = VK_NULL_HANDLE;
	BvVector<BvGPUInfo> m_GPUs;
	BvVector<IBvRenderDeviceVk*> m_Devices;
	BvDebugReportVk* m_pDebugReport = nullptr;
	bool m_HasDebugUtils = false;
};


namespace BvRenderVk
{
	extern "C"
	{
		BV_API bool CreateRenderEngine(IBvRenderEngine** ppObj);
		BV_API bool CreateRenderEngineVk(IBvRenderEngineVk** ppObj);
	}
}