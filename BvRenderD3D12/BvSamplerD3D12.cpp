#include "BvSamplerD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BvTypeConversionsD3D12.h"
#include "BvDescriptorHeapD3D12.h"


BV_D3D12_DEVICE_RES_DEF(BvSamplerD3D12)


BvSamplerD3D12::BvSamplerD3D12(BvRenderDeviceD3D12* pDevice, const SamplerDesc& samplerDesc)
	: m_SamplerDesc(samplerDesc), m_pDevice(pDevice)
{
	auto pHeap = m_pDevice->GetCPUSamplerHeap();
	m_Sampler = pHeap->Allocate();
}


BvSamplerD3D12::~BvSamplerD3D12()
{
}