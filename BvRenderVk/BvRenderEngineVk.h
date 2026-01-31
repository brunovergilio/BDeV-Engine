#pragma once


#include "BDeV/Core/RenderAPI/BvRenderEngine.h"
#include "BDeV/Core/System/Library/BvSharedLib.h"
#include "BvCommonVk.h"


class BvDebugUtilsVk;
class BvRenderDeviceVk;


class BvRenderEngineVk final : public IBvRenderEngine
{
	BV_NOCOPYMOVE(BvRenderEngineVk);
	friend class BvRenderEngineVkHelper;

private:
	bool CreateRenderDeviceImpl(const RenderDeviceDesc& renderDeviceDesc, void** ppObj) override;

public:
	BV_INLINE const GPUList& GetGPUs() const override { return m_GPUs; }
	BV_INLINE VkInstance GetHandle() const { return m_Instance; }

private:
	BvRenderEngineVk(const RenderEngineDesc& renderEngineDesc);
	~BvRenderEngineVk();

	void Create(const RenderEngineDesc& renderEngineDesc);
	void Destroy();
	void OnDeviceDestroyed(u32 index);

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
	BvDebugUtilsVk* m_pDebugUtils = nullptr;
};
BV_OBJECT_DEFINE_ID(BvRenderEngineVk, "eb31d72c-fe50-4284-ab0c-a5dbccf3c72d");


namespace BvRenderVk
{
	extern "C"
	{
		BV_API bool CreateRenderEngine(const RenderEngineDesc& renderEngineDesc, void** ppObj);
	}
}