#pragma once


#include "BDeV/Core/RenderAPI/BvRenderEngine.h"
#include "BvGPUInfoVk.h"


class BvDebugReportVk;
class BvRenderDeviceVk;


struct BvRenderDeviceCreateDescVk : BvRenderDeviceCreateDesc
{
	u32 m_GraphicsQueueCount = 1;
	u32 m_ComputeQueueCount = 0;
	u32 m_TransferQueueCount = 0;
	bool m_UseDebug = true;
};


class BvRenderEngineVk final : public BvRenderEngine
{
	BV_NOCOPYMOVE(BvRenderEngineVk);

public:
	BV_INLINE u32 GetSupportedGPUCount() const override { return m_GPUs.Size(); }
	void GetGPUInfo(u32 index, BvGPUInfo& info) const override;
	BvRenderDevice* CreateRenderDevice(const BvRenderDeviceCreateDesc* pDeviceCreateDesc = nullptr) override;
	BvRenderDeviceVk* CreateRenderDeviceVk(const BvRenderDeviceCreateDescVk& deviceDesc);

	BV_INLINE VkInstance GetHandle() const { return m_Instance; }

	static BvRenderEngineVk* GetInstance();

private:
	BvRenderEngineVk();
	~BvRenderEngineVk();

	void Create();
	void Destroy();

	bool IsPhysicalDeviceExtensionSupported(const BvGPUInfoVk& gpu, const char* pPhysicalDeviceExtension);
	bool SetupDeviceExtraPropertiesAndFeatures(BvGPUInfoVk& gpu);
	void SetupQueueInfo(BvGPUInfoVk& gpu);
	u32 GetQueueFamilyIndex(BvGPUInfoVk& gpu, VkQueueFlags queueFlags, bool& isAsync) const;
	bool QueueSupportsPresent(BvGPUInfoVk& gpu, u32 index) const;
	u32 AutoSelectGPU();

private:
	VkInstance m_Instance = VK_NULL_HANDLE;
	BvVector<BvGPUInfoVk> m_GPUs;
	BvRenderDeviceVk* m_pDevice = nullptr;
	BvDebugReportVk* m_pDebugReport = nullptr;
};