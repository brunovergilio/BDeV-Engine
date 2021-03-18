#pragma once


#include "BvRender/BvRenderEngine.h"
#include "BvRenderVk/BvGPUInfoVk.h"
#include "BvLoaderVk.h"


class BvDebugReportVk;


class BvRenderEngineVk final : public BvRenderEngine
{
	BV_NOCOPYMOVE(BvRenderEngineVk);

public:
	BvRenderEngineVk();
	~BvRenderEngineVk();

	bool Create();
	void Destroy();

	void GetGPUInfo(const u32 index, BvGPUInfo & info) const override final;
	BvRenderDevice * const CreateRenderDevice(const DeviceCreateDesc& deviceDesc, const u32 gpuIndex) override final;
	void DestroyRenderDevice(const u32 gpuIndex) override final;
	
	BV_INLINE const char * const GetAPIName() const override final { return "Vulkan"; }

private:
	bool CreateInstance();
	bool EnumerateGPUs();
	u32 GetQueueFamilyIndex(const VkQueueFlags queueFlags, const u32 gpuIndex) const;

private:
	VkInstance m_Instance = VK_NULL_HANDLE;
	BvVector<VkExtensionProperties> m_ExtensionProperties;
	BvVector<BvGPUInfoVk> m_GPUs{};
#if defined(BV_DEBUG)
	BvDebugReportVk * m_pDebugReport = nullptr;
#endif
	BvLoaderVk m_Loader;
};


/*BV_EXTERN_C BV_API */BvRenderEngine* GetRenderEngineVk();