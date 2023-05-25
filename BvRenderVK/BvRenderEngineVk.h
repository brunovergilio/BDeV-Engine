#pragma once


#include "BDeV/RenderAPI/BvRenderEngine.h"
#include "BvRenderVk/BvGPUInfoVk.h"
#include "BvLoaderVk.h"


class BvDebugReportVk;


class BvRenderEngineVk final : public BvRenderEngine
{
	BV_NOCOPYMOVE(BvRenderEngineVk);

public:
	BvRenderEngineVk();
	~BvRenderEngineVk();

	void GetGPUInfo(const u32 index, BvGPUInfo & info) const override final;
	BvRenderDevice * const CreateRenderDevice(const DeviceCreateDesc& deviceDesc, u32 gpuIndex) override final;
	
	BV_INLINE const char * const GetEngineName() const override final { return "BvRenderVk"; }
	BV_INLINE u32 GetInstanceVersion() const { return m_InstanceVersion; }
	BV_INLINE VkInstance GetInstance() const { return m_Instance; }

	bool IsInstanceExtensionSupported(const char* const pExtension);
	bool IsInstanceExtensionEnabled(const char* const pExtension);
	bool IsInstanceLayerSupported(const char* const pLayer);
	bool IsPhysicalDeviceExtensionSupported(const BvGPUInfoVk& gpu, const char* const pPhysicalDeviceExtension);

private:
	void Create();
	void Destroy();

	bool CreateInstance();
	bool EnumerateGPUs();
	void SetupDevicePropertiesAndFeatures(u32 gpuIndex);
	u32 GetQueueFamilyIndex(const VkQueueFlags queueFlags, const u32 gpuIndex) const;
	u32 AutoSelectGPU();

private:
	VkInstance m_Instance = VK_NULL_HANDLE;
	BvVector<VkExtensionProperties> m_SupportedExtensions;
	BvVector<VkLayerProperties> m_SupportedLayers;
	BvVector<const char*> m_EnabledExtensions;
	BvVector<BvGPUInfoVk> m_GPUs{};
#if defined(BV_DEBUG)
	BvDebugReportVk * m_pDebugReport = nullptr;
#endif
	u32 m_InstanceVersion = 0;
};


namespace BvRenderVk
{
#if defined (BV_STATIC_LIB)
	BV_PLUGIN_API BvRenderEngine* GetRenderEngine();
#else
#if defined(BV_PLUGIN_DLL_EXPORT)
	BV_EXTERN_C
#endif
	BV_PLUGIN_API BvRenderEngine* CreateRenderEngine();

#if defined(BV_PLUGIN_DLL_EXPORT)
	BV_EXTERN_C
#endif
	BV_PLUGIN_API void DestroyRenderEngine();
#endif
}