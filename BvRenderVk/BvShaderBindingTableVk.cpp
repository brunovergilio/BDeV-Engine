#include "BvShaderBindingTableVk.h"
#include "BvRenderDeviceVk.h"
#include "BvUtilsVk.h"


BvShaderBindingTableVk::BvShaderBindingTableVk(BvRenderDeviceVk* pDevice, const ShaderBindingTableDesc& sbtDesc, VkBuffer buffer,
	VkDeviceAddress deviceAddress, VmaAllocation allocation, const VkStridedDeviceAddressRegionKHR(&regions)[u32(ShaderBindingTableGroupType::kCount)])
	: m_SBTDesc(sbtDesc), m_pDevice(pDevice), m_Buffer(buffer), m_Allocation(allocation), m_DeviceAddress(deviceAddress)
{
	for (auto i = 0; i < u32(ShaderBindingTableGroupType::kCount); ++i)
	{
		m_Regions[i] = regions[i];
	}
}


BvShaderBindingTableVk::~BvShaderBindingTableVk()
{
	Destroy();
}


void BvShaderBindingTableVk::GetDeviceAddressRange(ShaderBindingTableGroupType type, u32 index, DeviceAddressRange& addressRange) const
{
	auto& region = m_Regions[u32(type)];
	addressRange.m_Address = region.deviceAddress + index * region.stride;
	addressRange.m_Size = region.size;
}


void BvShaderBindingTableVk::GetDeviceAddressRangeAndStride(ShaderBindingTableGroupType type, u32 index, DeviceAddressRangeAndStride& addressRangeAndStride) const
{
	auto& region = m_Regions[u32(type)];
	addressRangeAndStride.m_Address = region.deviceAddress + index * region.stride;
	addressRangeAndStride.m_Size = region.size;
	addressRangeAndStride.m_Stride = region.stride;
}


void BvShaderBindingTableVk::Destroy()
{
	if (m_Buffer)
	{
		VkHelpers::DestroyDeviceObject(*m_pDevice, m_Buffer, m_pDevice->GetAllocator(), m_Allocation);
		m_Buffer = VK_NULL_HANDLE;
	}
}