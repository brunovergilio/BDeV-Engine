#include "BvShaderBindingTableD3D12.h"


BvShaderBindingTableD3D12::BvShaderBindingTableD3D12(BvRenderDeviceD3D12* pDevice, const ShaderBindingTableDesc& sbtDesc, ComPtr<ID3D12Resource>& buffer,
	ComPtr<D3D12MA::Allocation>& allocation, const D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE(&regions)[u32(ShaderBindingTableGroupType::kCount)])
	: m_pDevice(pDevice), m_SBTDesc(sbtDesc), m_Buffer(std::move(buffer)), m_Allocation(std::move(allocation))
{
	for (auto i = 0; i < u32(ShaderBindingTableGroupType::kCount); ++i)
	{
		m_Regions[i] = regions[i];
	}
}


BvShaderBindingTableD3D12::~BvShaderBindingTableD3D12()
{
	Destroy();
}


void BvShaderBindingTableD3D12::GetDeviceAddressRange(ShaderBindingTableGroupType type, u32 index, DeviceAddressRange& addressRange) const
{
	auto& region = m_Regions[u32(type)];
	addressRange.m_Address = region.StartAddress + index * region.StrideInBytes;
	addressRange.m_Size = region.SizeInBytes;
}


void BvShaderBindingTableD3D12::GetDeviceAddressRangeAndStride(ShaderBindingTableGroupType type, u32 index, DeviceAddressRangeAndStride& addressRangeAndStride) const
{
	auto& region = m_Regions[u32(type)];
	addressRangeAndStride.m_Address = region.StartAddress + index * region.StrideInBytes;
	addressRangeAndStride.m_Size = region.SizeInBytes;
	addressRangeAndStride.m_Stride = region.StrideInBytes;
}


void BvShaderBindingTableD3D12::Destroy()
{
	m_Buffer = nullptr;
	m_Allocation = nullptr;
}