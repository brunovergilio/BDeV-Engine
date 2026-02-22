#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/RenderAPI/BvShaderBindingTable.h"
#include "BvBufferVk.h"


class BvRenderDeviceVk;
class BvCommandContextVk;


class BvShaderBindingTableVk final : public IBvShaderBindingTable, public IBvResourceVk
{
public:
	BvShaderBindingTableVk(BvRenderDeviceVk* pDevice, const ShaderBindingTableDesc& sbtDesc, VkBuffer buffer, VkDeviceAddress deviceAddress,
		VmaAllocation allocation, const VkStridedDeviceAddressRegionKHR (&regions)[u32(ShaderBindingTableGroupType::kCount)]);
	~BvShaderBindingTableVk();

	BV_INLINE const ShaderBindingTableDesc& GetDesc() const override { return m_SBTDesc; }
	void GetDeviceAddressRange(ShaderBindingTableGroupType type, u32 index, DeviceAddressRange& addressRange) const override;
	void GetDeviceAddressRangeAndStride(ShaderBindingTableGroupType type, u32 index, DeviceAddressRangeAndStride& addressRangeAndStride) const override;
	BV_INLINE bool IsValid() const { return m_Buffer != nullptr; }

	BV_INLINE VkStridedDeviceAddressRegionKHR GetDeviceAddressRangeAndStride(ShaderBindingTableGroupType type, u32 index) const
	{
		auto& region = m_Regions[u32(type)];
		return { region.deviceAddress + index * region.stride, region.stride, region.size };
	}

private:
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkBuffer m_Buffer = VK_NULL_HANDLE;
	VmaAllocation m_Allocation = VK_NULL_HANDLE;
	VkDeviceAddress m_DeviceAddress = 0;
	ShaderBindingTableDesc m_SBTDesc;
	VkStridedDeviceAddressRegionKHR m_Regions[u32(ShaderBindingTableGroupType::kCount)]{};
};
BV_OBJECT_DEFINE_ID(BvShaderBindingTableVk, "0bb5abad-c53b-47bd-b939-989cb9d2e58d");
BV_CREATE_CAST_TO_VK(BvShaderBindingTable)