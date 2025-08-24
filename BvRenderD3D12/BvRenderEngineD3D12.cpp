#include "BvRenderEngineD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BDeV/Core/Utils/BvText.h"


bool SetupDeviceInfo(IDXGIAdapter1* pAdapter, BvDeviceInfoD3D12& deviceInfo, BvGPUInfo& gpuInfo);


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
			if (m_GPUs[i]->m_Type == GPUType::kDiscrete)
			{
				gpuIndex = i;
				break;
			}
		}
	}

	auto& deviceData = *m_Devices[gpuIndex];
	auto& pDevice = deviceData.m_pDevice;
	BV_ASSERT_ONCE(pDevice == nullptr, "Render Device has already been created");
	if (!pDevice)
	{
		pDevice = BV_NEW(BvRenderDeviceD3D12)(this, deviceData.m_Adapter.Get(), deviceData.m_pDeviceInfo, gpuIndex, *m_GPUs[gpuIndex], descD3D12);
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
			BvGPUInfo gpuInfo{};
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
				gpuInfo.m_Type = GPUType::kIntegrated;
				gpuInfo.m_Vendor = GPUVendor::kARM; break;
			case 0x5143:
				gpuInfo.m_Type = GPUType::kIntegrated;
				gpuInfo.m_Vendor = GPUVendor::kQualcomm; break;
			case 0x163C:
			case 0x8086:
			case 0x8087:
				gpuInfo.m_Type = GPUType::kIntegrated;
				gpuInfo.m_Vendor = GPUVendor::kIntel; break;
			}

			BvDeviceInfoD3D12 deviceInfo{};
			if (SetupDeviceInfo(adapter.Get(), deviceInfo, gpuInfo))
			{
				auto pDeviceData = m_Devices.EmplaceBack(BV_NEW(DeviceData)());
				pDeviceData->m_Adapter = adapter;
				pDeviceData->m_pDeviceInfo = BV_NEW(BvDeviceInfoD3D12)();
				*pDeviceData->m_pDeviceInfo = deviceInfo;

				auto pGPUInfo = m_GPUs.EmplaceBack(BV_NEW(BvGPUInfo)());
				*pGPUInfo = gpuInfo;
			}
		}
	}
}


void BvRenderEngineD3D12::Destroy()
{
	if (m_Factory)
	{
		for (auto pDeviceData : m_Devices)
		{
			auto pDevice = pDeviceData->m_pDevice;
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


bool SetupDeviceInfo(IDXGIAdapter1* pAdapter, BvDeviceInfoD3D12& deviceInfo, BvGPUInfo& gpuInfo)
{
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	D3D_FEATURE_LEVEL availableFeatureLevel = D3D_FEATURE_LEVEL_11_0;
	// Create a temporary device so we can query its capabilities
	ComPtr<ID3D12Device> device;
	for (auto i = 0; i < ArraySize(levels); ++i)
	{
		if (SUCCEEDED(D3D12CreateDevice(pAdapter, levels[i], IID_PPV_ARGS(&device))))
		{
			availableFeatureLevel = levels[i];
			break;
		}
	}

	if (!device)
	{
		return false;
	}

	deviceInfo.m_FeatureLevel = availableFeatureLevel;
#define GET_FEATURE_OPTIONS(option) device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS##option, &deviceInfo.m_Options##option, sizeof(deviceInfo.m_Options##option))
	GET_FEATURE_OPTIONS();
	GET_FEATURE_OPTIONS(1);
	GET_FEATURE_OPTIONS(2);
	GET_FEATURE_OPTIONS(3);
	GET_FEATURE_OPTIONS(4);
	GET_FEATURE_OPTIONS(5);
	GET_FEATURE_OPTIONS(6);
	GET_FEATURE_OPTIONS(7);
	GET_FEATURE_OPTIONS(8);
	GET_FEATURE_OPTIONS(9);
	GET_FEATURE_OPTIONS(10);
	GET_FEATURE_OPTIONS(11);
	GET_FEATURE_OPTIONS(12);
	GET_FEATURE_OPTIONS(13);
#undef GET_FEATURE_OPTIONS

	gpuInfo.m_DeviceCaps |= RenderDeviceCapabilities::kWireframe | RenderDeviceCapabilities::kGeometryShader | RenderDeviceCapabilities::kTesselationShader
		| RenderDeviceCapabilities::kTimestampQueries | RenderDeviceCapabilities::kIndirectDrawCount
		| RenderDeviceCapabilities::kCustomBorderColor | RenderDeviceCapabilities::kPredication | RenderDeviceCapabilities::kTrueFullScreen;
	if (deviceInfo.m_Options2.DepthBoundsTestSupported)
	{
		gpuInfo.m_DeviceCaps |= RenderDeviceCapabilities::kDepthBoundsTest;
	}
	if (deviceInfo.m_Options.TiledResourcesTier >= D3D12_TILED_RESOURCES_TIER_2)
	{
		gpuInfo.m_DeviceCaps |= RenderDeviceCapabilities::kSamplerMinMaxReduction;
	}
	if (deviceInfo.m_Options.ConservativeRasterizationTier != D3D12_CONSERVATIVE_RASTERIZATION_TIER_NOT_SUPPORTED)
	{
		gpuInfo.m_DeviceCaps |= RenderDeviceCapabilities::kConservativeRasterization;
	}
	if (deviceInfo.m_Options6.VariableShadingRateTier != D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED)
	{
		gpuInfo.m_DeviceCaps |= RenderDeviceCapabilities::kShadingRate;
	}
	if (deviceInfo.m_Options7.MeshShaderTier != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED)
	{
		gpuInfo.m_DeviceCaps |= RenderDeviceCapabilities::kMeshShader;
	}
	if (deviceInfo.m_Options9.MeshShaderPipelineStatsSupported)
	{
		gpuInfo.m_DeviceCaps |= RenderDeviceCapabilities::kMeshQuery;
	}
	if (deviceInfo.m_Options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
	{
		gpuInfo.m_DeviceCaps |= RenderDeviceCapabilities::kRayTracing | RenderDeviceCapabilities::kRayQuery;
	}

	{
		auto& graphics = gpuInfo.m_ContextGroups.EmplaceBack();
		graphics.m_DedicatedCommandType = CommandType::kGraphics;
		graphics.m_SupportedCommandTypes.EmplaceBack(CommandType::kGraphics);
		graphics.m_SupportedCommandTypes.EmplaceBack(CommandType::kCompute);
		graphics.m_SupportedCommandTypes.EmplaceBack(CommandType::kTransfer);
		graphics.m_GroupIndex = gpuInfo.m_ContextGroups.Size() - 1;
		graphics.m_MaxContextCount = 16;

		auto& compute = gpuInfo.m_ContextGroups.EmplaceBack();
		compute.m_DedicatedCommandType = CommandType::kCompute;
		compute.m_SupportedCommandTypes.EmplaceBack(CommandType::kCompute);
		compute.m_SupportedCommandTypes.EmplaceBack(CommandType::kTransfer);
		compute.m_GroupIndex = gpuInfo.m_ContextGroups.Size() - 1;
		compute.m_MaxContextCount = 16;

		auto& transfer = gpuInfo.m_ContextGroups.EmplaceBack();
		transfer.m_DedicatedCommandType = CommandType::kTransfer;
		transfer.m_SupportedCommandTypes.EmplaceBack(CommandType::kTransfer);
		transfer.m_GroupIndex = gpuInfo.m_ContextGroups.Size() - 1;
		transfer.m_MaxContextCount = 16;
	}

	// Check for video queues - this probably doesn't mean I have video support, although it should
	{
		ComPtr<ID3D12VideoDevice> videoDevice;
		if (SUCCEEDED(device.As(&videoDevice)))
		{
			ComPtr<ID3D12CommandQueue> queue;
			D3D12_COMMAND_QUEUE_DESC queueDesc{};
			queueDesc.Type = D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE;
			auto hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue));
			if (SUCCEEDED(hr))
			{
				auto& videoDecode = gpuInfo.m_ContextGroups.EmplaceBack();
				videoDecode.m_DedicatedCommandType = CommandType::kVideoDecode;
				videoDecode.m_SupportedCommandTypes.EmplaceBack(CommandType::kVideoDecode);
				videoDecode.m_GroupIndex = gpuInfo.m_ContextGroups.Size() - 1;
				videoDecode.m_MaxContextCount = 16;
				queue = nullptr;
			}

			queueDesc.Type = D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE;
			hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue));
			if (SUCCEEDED(hr))
			{
				auto& videoEncode = gpuInfo.m_ContextGroups.EmplaceBack();
				videoEncode.m_DedicatedCommandType = CommandType::kVideoEncode;
				videoEncode.m_SupportedCommandTypes.EmplaceBack(CommandType::kVideoEncode);
				videoEncode.m_GroupIndex = gpuInfo.m_ContextGroups.Size() - 1;
				videoEncode.m_MaxContextCount = 16;
				queue = nullptr;
			}
		}
	}

	return true;
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