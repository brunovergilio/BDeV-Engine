#include "BvBufferViewD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BvTypeConversionsD3D12.h"


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
}


void BvBufferViewD3D12::Destroy()
{
}