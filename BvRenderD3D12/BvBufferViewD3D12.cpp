#include "BvBufferViewD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BvTypeConversionsD3D12.h"
#include "BvBufferD3D12.h"
#include "BvDescriptorHeapD3D12.h"


BV_D3D12_DEVICE_RES_DEF(BvBufferViewD3D12)


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

	auto cbv = GetD3D12CBVDesc(m_BufferViewDesc);
	auto srv = GetD3D12SRVDesc(m_BufferViewDesc);
	auto uav = GetD3D12UAVDesc(m_BufferViewDesc);

	auto pHeap = m_pDevice->GetCPUShaderHeap();
	m_CBV = pHeap->Allocate();
	m_SRV = pHeap->Allocate();
	m_UAV = pHeap->Allocate();

	auto pDevice = m_pDevice->GetHandle();
	pDevice->CreateConstantBufferView(&cbv, m_CBV);
	pDevice->CreateShaderResourceView(pRes, &srv, m_SRV);
	pDevice->CreateUnorderedAccessView(pRes, nullptr, &uav, m_UAV);
}


void BvBufferViewD3D12::Destroy()
{
}