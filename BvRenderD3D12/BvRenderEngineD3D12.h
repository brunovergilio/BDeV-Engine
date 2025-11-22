#pragma once


#include "BDeV/Core/RenderAPI/BvRenderEngine.h"
#include "BvCommonD3D12.h"


class BvRenderDeviceD3D12;


struct BvDeviceInfoD3D12
{
	D3D12_FEATURE_DATA_D3D12_OPTIONS m_Options{};
	D3D12_FEATURE_DATA_D3D12_OPTIONS1 m_Options1{};
	D3D12_FEATURE_DATA_D3D12_OPTIONS2 m_Options2{};
	D3D12_FEATURE_DATA_D3D12_OPTIONS3 m_Options3{};
	D3D12_FEATURE_DATA_D3D12_OPTIONS4 m_Options4{};
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 m_Options5{};
	D3D12_FEATURE_DATA_D3D12_OPTIONS6 m_Options6{};
	D3D12_FEATURE_DATA_D3D12_OPTIONS7 m_Options7{};
	D3D12_FEATURE_DATA_D3D12_OPTIONS8 m_Options8{};
	D3D12_FEATURE_DATA_D3D12_OPTIONS9 m_Options9{};
	D3D12_FEATURE_DATA_D3D12_OPTIONS10 m_Options10{};
	D3D12_FEATURE_DATA_D3D12_OPTIONS11 m_Options11{};
	D3D12_FEATURE_DATA_D3D12_OPTIONS12 m_Options12{};
	D3D12_FEATURE_DATA_D3D12_OPTIONS13 m_Options13{};
	D3D_FEATURE_LEVEL m_FeatureLevel{};
};


class BvRenderEngineD3D12 final : public IBvRenderEngine
{
public:
	bool CreateRenderDeviceImpl(const BvRenderDeviceCreateDesc& deviceCreateDesc, const BvUUID& objId, void** ppObj) override;
	void OnDeviceDestroyed(u32 index);
	BV_INLINE const GPUList& GetGPUs() const override { return m_GPUs; }

private:
	friend class BvRenderEngineD3D12Helper;

	BvRenderEngineD3D12();
	~BvRenderEngineD3D12();

	void Create();
	void Destroy();
	void SelfDestroy() override;

private:
	struct DeviceData
	{
		ComPtr<IDXGIAdapter1> m_Adapter;
		BvDeviceInfoD3D12* m_pDeviceInfo;
		BvRenderDeviceD3D12* m_pDevice;
	};

	ComPtr<IDXGIFactory1> m_Factory = nullptr;
	BvFixedVector<DeviceData*, kMaxDevices> m_Devices;
	GPUList m_GPUs;
};


namespace BvRenderD3D12
{
	extern "C"
	{
		BV_API IBvRenderEngine* CreateRenderEngine();
		BV_API BvRenderEngineD3D12* CreateRenderEngineD3D12();
	}
}