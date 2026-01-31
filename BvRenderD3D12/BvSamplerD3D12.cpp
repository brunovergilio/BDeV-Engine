#include "BvSamplerD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BvTypeConversionsD3D12.h"
#include "BvDescriptorHeapD3D12.h"


BvSamplerD3D12::BvSamplerD3D12(BvRenderDeviceD3D12* pDevice, const SamplerDesc& samplerDesc)
	: m_SamplerDesc(samplerDesc), m_pDevice(pDevice)
{
	auto pHeap = m_pDevice->GetCPUSamplerHeap();
	m_Sampler = pHeap->Allocate();

	auto desc = GetD3D12SamplerDesc(m_SamplerDesc);
	m_pDevice->GetHandle()->CreateSampler(&desc, m_Sampler);
}


BvSamplerD3D12::~BvSamplerD3D12()
{
}


void BvSamplerD3D12::Destroy()
{
}