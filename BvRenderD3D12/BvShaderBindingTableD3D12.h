#pragma once


#include "BvCommonD3D12.h"
#include "BDeV/Core/RenderAPI/BvShaderBindingTable.h"


class BvRenderDeviceD3D12;


class BvShaderBindingTableD3D12 final : public IBvShaderBindingTable, public IBvResourceD3D12
{
public:
	BvShaderBindingTableD3D12(BvRenderDeviceD3D12* pDevice, const ShaderBindingTableDesc& sbtDesc, ComPtr<ID3D12Resource>& buffer,
		ComPtr<D3D12MA::Allocation>& allocation, const D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE(&regions)[u32(ShaderBindingTableGroupType::kCount)]);
	~BvShaderBindingTableD3D12();

	BV_INLINE const ShaderBindingTableDesc& GetDesc() const override { return m_SBTDesc; }
	void GetDeviceAddressRange(ShaderBindingTableGroupType type, u32 index, DeviceAddressRange& addressRange) const override;
	void GetDeviceAddressRangeAndStride(ShaderBindingTableGroupType type, u32 index, DeviceAddressRangeAndStride& addressRangeAndStride) const override;

	BV_INLINE D3D12_GPU_VIRTUAL_ADDRESS_RANGE GetDeviceAddressRange(u32 index) const
	{
		auto& region = m_Regions[0];
		return { region.StartAddress + index * region.StrideInBytes, region.SizeInBytes };
	}

	BV_INLINE D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE GetDeviceAddressRangeAndStride(ShaderBindingTableGroupType type, u32 index) const
	{
		auto& region = m_Regions[u32(type)];
		return { region.StartAddress + index * region.StrideInBytes, region.SizeInBytes, region.StrideInBytes };
	}

private:
	void Destroy();

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	ComPtr<ID3D12Resource> m_Buffer;
	ComPtr<D3D12MA::Allocation> m_Allocation;
	ShaderBindingTableDesc m_SBTDesc;
	D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE m_Regions[u32(ShaderBindingTableGroupType::kCount)]{};
};
BV_OBJECT_DEFINE_ID(BvShaderBindingTableD3D12, "83DD337A-A84B-40BB-A0A8-C0E47FC3638D");
BV_CREATE_CAST_TO_D3D12(BvShaderBindingTable)