#pragma once


#include "BDeV/Core/RenderAPI/BvRenderEngine.h"
#include "BDeV/Core/System/Library/BvSharedLib.h"
#include "BvCommonVk.h"


class BvDebugReportVk;
class BvRenderDeviceVk;


BV_OBJECT_DEFINE_ID(BvRenderEngineVk, "eb31d72c-fe50-4284-ab0c-a5dbccf3c72d");
class BvRenderEngineVk final : public IBvRenderEngine
{
	BV_NOCOPYMOVE(BvRenderEngineVk);

private:
	bool CreateRenderDeviceImpl(const BvRenderDeviceCreateDesc& deviceCreateDesc, const BvUUID& objId, void** ppObj) override;

public:
	BV_INLINE const GPUList& GetGPUs() const override { return m_GPUs; }

	void OnDeviceDestroyed(u32 index);
	BV_INLINE VkInstance GetHandle() const { return m_Instance; }

	//BV_OBJECT_IMPL_INTERFACE(IBvRenderEngineVk, IBvRenderEngine);

private:
	friend class BvRenderEngineVkHelper;

	BvRenderEngineVk();
	~BvRenderEngineVk();

	void Create();
	void Destroy();
	void SelfDestroy() override;

private:
	struct DeviceData
	{
		VkPhysicalDevice m_PhysicalDevice;
		BvRenderDeviceVk* m_pDevice;
		BvDeviceInfoVk* m_pDeviceInfo;
	};

private:
	BvSharedLib m_VulkanLib;
	VkInstance m_Instance = VK_NULL_HANDLE;
	BvFixedVector<DeviceData*, kMaxDevices> m_Devices;
	GPUList m_GPUs;
	BvDebugReportVk* m_pDebugReport = nullptr;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvRenderEngineVk);


namespace BvRenderVk
{
	extern "C"
	{
		BV_API bool CreateRenderEngine(const BvUUID& objId, void** ppObj);
	}
}