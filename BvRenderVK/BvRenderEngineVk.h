#pragma once


#include "BDeV/RenderAPI/BvRenderEngine.h"
#include "BvRenderVk/BvGPUInfoVk.h"


class BvDebugReportVk;
class BvRenderDeviceVk;


struct BvRenderDeviceCreateDescVk
{
	u32 m_GraphicsQueueCount = 1;
	u32 m_ComputeQueueCount = 1;
	u32 m_TransferQueueCount = 1;
	// Pool Sizes
};


class BvRenderEngineVk final : public BvRenderEngine
{
	BV_NOCOPYMOVE(BvRenderEngineVk);

public:
	BvRenderEngineVk();
	~BvRenderEngineVk();

	bool Initialize() override;
	void Shutdown() override;

	BV_INLINE u32 GetSupportedGPUCount() const override { return m_GPUs.Size(); }
	void GetGPUInfo(u32 index, BvGPUInfo& info) const override;

	BvRenderDevice* CreateRenderDevice(u32 gpuIndex = 0) override;
	BvRenderDevice* CreateRenderDevice(const BvRenderDeviceCreateDescVk& deviceDesc, u32 gpuIndex = 0);
	
	BV_INLINE const char* GetEngineName() const override { return "BvRenderVk"; }

	BV_INLINE VkInstance GetInstance() const { return m_Instance; }

	bool IsPhysicalDeviceExtensionSupported(const BvGPUInfoVk& gpu, const char* pPhysicalDeviceExtension);

private:
	void SetupDevicePropertiesAndFeatures(u32 gpuIndex);
	u32 GetQueueFamilyIndex(const VkQueueFlags queueFlags, const u32 gpuIndex) const;
	u32 AutoSelectGPU();

private:
	VkInstance m_Instance = VK_NULL_HANDLE;
	BvVector<BvGPUInfoVk> m_GPUs;
	BvVector<BvRenderDeviceVk*> m_RenderDevices;
	BvDebugReportVk* m_pDebugReport = nullptr;
};


namespace BvRenderVk
{
	extern "C"
	{
		BV_API BvRenderEngine* GetRenderEngine();
	}
}