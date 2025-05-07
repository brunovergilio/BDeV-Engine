#pragma once


#include "BDeV/Core/RenderAPI/BvRenderEngine.h"
#include "BDeV/Core/System/Library/BvSharedLib.h"
#include "BvCommonD3D12.h"


class BvRenderDeviceD3D12;


class BvRenderEngineD3D12 final : public IBvRenderEngine
{
public:
	IBvRenderDevice* CreateRenderDeviceImpl(const BvRenderDeviceCreateDesc& deviceCreateDesc) override;
	void OnDeviceDestroyed(u32 index);
	BV_INLINE const BvVector<BvGPUInfo>& GetGPUs() const override { return m_GPUs; }

private:
	friend class BvRenderEngineD3D12Helper;

	BvRenderEngineD3D12();
	~BvRenderEngineD3D12();

	void Create();
	void Destroy();
	void SelfDestroy() override;

private:
	ComPtr<IDXGIFactory1> m_Factory = nullptr;
	BvVector<BvGPUInfo> m_GPUs;
	BvVector<std::pair<ComPtr<IDXGIAdapter1>, BvRenderDeviceD3D12*>> m_Devices;
};


namespace BvRenderD3D12
{
	extern "C"
	{
		BV_API IBvRenderEngine* CreateRenderEngine();
		BV_API BvRenderEngineD3D12* CreateRenderEngineD3D12();
	}
}