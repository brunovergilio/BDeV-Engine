#include "BvRenderPassD3D12.h"
#include "BvRenderDeviceD3D12.h"


BvRenderPassD3D12::BvRenderPassD3D12(BvRenderDeviceD3D12* pDevice, const RenderPassDesc& renderPassDesc)
	: m_RenderPassDesc(renderPassDesc), m_pDevice(pDevice)
{
}


BvRenderPassD3D12::~BvRenderPassD3D12()
{
}


void BvRenderPassD3D12::Destroy()
{
}