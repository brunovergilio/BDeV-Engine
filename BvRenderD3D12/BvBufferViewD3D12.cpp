#include "BvBufferViewD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BvTypeConversionsD3D12.h"
#include "BvBufferD3D12.h"
#include "BvDescriptorHeapD3D12.h"


BvBufferViewD3D12::BvBufferViewD3D12(BvRenderDeviceD3D12* pDevice, const BufferViewDesc& bufferViewDesc)
	: m_BufferViewDesc(bufferViewDesc), m_pDevice(pDevice)
{
	Create();
}


BvBufferViewD3D12::~BvBufferViewD3D12()
{
	Destroy();
}


void BvBufferViewD3D12::Create()
{
	BV_ASSERT(m_BufferViewDesc.m_pBuffer != nullptr, "Invalid buffer handle");

	auto pRes = TO_D3D12(m_BufferViewDesc.m_pBuffer)->GetHandle();
	auto pShaderHeap = m_pDevice->GetCPUShaderHeap();
	auto pDevice = m_pDevice->GetHandle();
	auto flags = m_BufferViewDesc.m_pBuffer->GetDesc().m_UsageFlags;

	if (EHasFlag(flags, BufferUsage::kConstantBuffer))
	{
		auto cbv = GetD3D12CBVDesc(m_BufferViewDesc);
		m_CBV = pShaderHeap->Allocate();
		pDevice->CreateConstantBufferView(&cbv, m_CBV);
	}

	if (EHasFlag(flags, BufferUsage::kStructuredBuffer))
	{
		auto srv = GetD3D12SRVDesc(m_BufferViewDesc);
		m_SRV = pShaderHeap->Allocate();
		pDevice->CreateShaderResourceView(pRes, &srv, m_SRV);
	}

	if (EHasAnyFlags(flags, BufferUsage::kRWStructuredBuffer | BufferUsage::kRayTracing))
	{
		auto uav = GetD3D12UAVDesc(m_BufferViewDesc);
		m_UAV = pShaderHeap->Allocate();
		pDevice->CreateUnorderedAccessView(pRes, nullptr, &uav, m_UAV);
	}
}


void BvBufferViewD3D12::Destroy()
{
	u32 numHandles = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE handles[3];
	if (m_CBV.ptr)
	{
		handles[numHandles++] = m_CBV;
	}
	if (m_SRV.ptr)
	{
		handles[numHandles++] = m_SRV;
	}
	if (m_UAV.ptr)
	{
		handles[numHandles++] = m_UAV;
	}

	if (numHandles)
	{
		m_pDevice->OnD3D12HandleDestroyed(numHandles, handles);
	}
}