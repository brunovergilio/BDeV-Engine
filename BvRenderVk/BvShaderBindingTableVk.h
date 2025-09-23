#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/RenderAPI/BvShaderBindingTable.h"
#include "BvBufferVk.h"


class BvRenderDeviceVk;
class BvCommandContextVk;


BV_OBJECT_DEFINE_ID(BvShaderBindingTableVk, "0bb5abad-c53b-47bd-b939-989cb9d2e58d");
class BvShaderBindingTableVk final : public IBvShaderBindingTable, public IBvResourceVk
{
	BV_NOCOPYMOVE(BvShaderBindingTableVk);
	BV_VK_DEVICE_RES_DECL;

public:
	BvShaderBindingTableVk(BvRenderDeviceVk* pDevice, const ShaderBindingTableDesc& sbtDesc,
		const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& props, BvCommandContextVk* pContext);
	~BvShaderBindingTableVk();

	BV_INLINE const ShaderBindingTableDesc& GetDesc() const override { return m_SBTDesc; }
	void GetDeviceAddressRange(ShaderBindingTableGroupType type, u32 index, DeviceAddressRange& addressRange) const override;
	void GetDeviceAddressRangeAndStride(ShaderBindingTableGroupType type, u32 index, DeviceAddressRangeAndStride& addressRangeAndStride) const override;
	BV_INLINE bool IsValid() const { return m_Buffer != nullptr; }

	//BV_OBJECT_IMPL_INTERFACE(IBvShaderBindingTableVk, IBvShaderBindingTable, IBvRenderDeviceObject);

private:
	void Create(BvCommandContextVk* pContext);
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	BvRCRef<BvBufferVk> m_Buffer = nullptr;
	ShaderBindingTableDesc m_SBTDesc;
	VkStridedDeviceAddressRegionKHR m_Regions[u32(ShaderBindingTableGroupType::kCount)]{};
	u32 m_HandleSize = 0;
	u32 m_GroupHandleAlignment = 0;
	u32 m_BaseGroupAlignment = 0;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvShaderBindingTableVk);


BV_CREATE_CAST_TO_VK(BvShaderBindingTable)