#include "BvRenderEngineD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BDeV/Core/Container/BvText.h"


class BvRenderEngineD3D12Helper
{
public:
	static BvRenderEngineD3D12* Create()
	{
		static bool initialized = false;
		if (!initialized)
		{
			initialized = true;
			s_pEngine = BV_NEW(BvRenderEngineD3D12)();
			if (s_pEngine->GetGPUs().Size() == 0)
			{
				BV_DELETE(s_pEngine);
				s_pEngine = nullptr;
			}
		}
		else if (s_pEngine)
		{
			s_pEngine->AddRef();
		}

		return s_pEngine;
	}

	static void Destroy()
	{
		BV_DELETE(s_pEngine);
	}

private:
	static BvRenderEngineD3D12* s_pEngine;
};
BvRenderEngineD3D12* BvRenderEngineD3D12Helper::s_pEngine = nullptr;


IBvRenderDevice* BvRenderEngineD3D12::CreateRenderDeviceImpl(const BvRenderDeviceCreateDesc& deviceCreateDesc)
{
	BvRenderDeviceCreateDescD3D12 descD3D12;
	memcpy(&descD3D12, &deviceCreateDesc, sizeof(BvRenderDeviceCreateDesc));
	u32 gpuIndex = descD3D12.m_GPUIndex;
	if (gpuIndex >= m_GPUs.Size())
	{
		gpuIndex = 0;
		for (auto i = 0; i < m_GPUs.Size(); ++i)
		{
			if (m_GPUs[i].m_Type == GPUType::kDiscrete)
			{
				gpuIndex = i;
				break;
			}
		}
	}

	auto& pDevice = m_Devices[gpuIndex].second;
	BV_ASSERT_ONCE(pDevice == nullptr, "Render Device has already been created");
	if (!pDevice)
	{
		pDevice = BV_NEW(BvRenderDeviceD3D12)(this, m_Devices[gpuIndex].first.Get(), gpuIndex, m_GPUs[gpuIndex], descD3D12);
		if (!pDevice->IsValid())
		{
			pDevice->Release();
			pDevice = nullptr;
		}
	}

	return pDevice;
}


void BvRenderEngineD3D12::OnDeviceDestroyed(u32 index)
{

}


BvRenderEngineD3D12::BvRenderEngineD3D12()
{
	Create();
}


BvRenderEngineD3D12::~BvRenderEngineD3D12()
{
	Destroy();
}


void BvRenderEngineD3D12::Create()
{
	auto hr = CreateDXGIFactory1(IID_PPV_ARGS(&m_Factory));
	if (FAILED(hr))
	{
		return;
	}

	ComPtr<IDXGIAdapter1> adapter;
	for (auto i = 0; hr == S_OK; ++i)
	{
		hr = m_Factory->EnumAdapters1(i, adapter.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			continue;
		}

		DXGI_ADAPTER_DESC1 adapterDesc;
		hr = adapter->GetDesc1(&adapterDesc);
		if (FAILED(hr))
		{
			continue;
		}

		if (adapterDesc.Flags == 0)
		{
			auto& gpuInfo = m_GPUs.EmplaceBack();
			BvTextUtilities::ConvertWideCharToUTF8Char(adapterDesc.Description, ArraySize(adapterDesc.Description),
				gpuInfo.m_DeviceName, ArraySize(gpuInfo.m_DeviceName));
			gpuInfo.m_DeviceMemory = adapterDesc.DedicatedVideoMemory;
			gpuInfo.m_DeviceId = adapterDesc.DeviceId;
			gpuInfo.m_GraphicsContextCount = kU32Max;
			gpuInfo.m_ComputeContextCount = kU32Max;
			gpuInfo.m_TransferContextCount = kU32Max;
			gpuInfo.m_VendorId = adapterDesc.VendorId;
			gpuInfo.m_Type = GPUType::kDiscrete;

			switch (gpuInfo.m_VendorId)
			{
			case 0x1002:
			case 0x1022:
				gpuInfo.m_Vendor = GPUVendor::kAMD; break;
			case 0x1010:
				gpuInfo.m_Vendor = GPUVendor::kImgTec; break;
			case 0x10DE:
				gpuInfo.m_Vendor = GPUVendor::kNvidia; break;
			case 0x13B5:
				gpuInfo.m_Vendor = GPUVendor::kARM; break;
			case 0x5143:
				gpuInfo.m_Vendor = GPUVendor::kQualcomm; break;
			case 0x163C:
			case 0x8086:
			case 0x8087:
				gpuInfo.m_Type = GPUType::kIntegrated;
				gpuInfo.m_Vendor = GPUVendor::kIntel; break;
			}

			auto& device = m_Devices.EmplaceBack();
			device.first = adapter;
		}
	}
}


void BvRenderEngineD3D12::Destroy()
{
	if (m_Factory)
	{
		for (auto& devicePair : m_Devices)
		{
			auto pDevice = devicePair.second;
			if (pDevice)
			{
				BV_ASSERT(false, "NOT IMPLEMENTED");
				//static_cast<IBvResourceD3D12*>(pDevice)->Destroy();
			}
		}

		m_Factory = nullptr;
	}
}


void BvRenderEngineD3D12::SelfDestroy()
{
	BvRenderEngineD3D12Helper::Destroy();
}


namespace BvRenderD3D12
{
	extern "C"
	{
		BV_API IBvRenderEngine* CreateRenderEngine()
		{
			return BvRenderEngineD3D12Helper::Create();
		}

		BV_API BvRenderEngineD3D12* CreateRenderEngineD3D12()
		{
			return BvRenderEngineD3D12Helper::Create();
		}
	}
}