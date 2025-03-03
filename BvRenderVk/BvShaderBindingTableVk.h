#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/RenderAPI/BvShaderBindingTable.h"


class BvRenderDevice;
class BvRenderDeviceVk;
class BvBufferVk;
class BvCommandContextVk;


BV_OBJECT_DEFINE_ID(BvShaderBindingTableVk, "0bb5abad-c53b-47bd-b939-989cb9d2e58d");
class BvShaderBindingTableVk : public BvShaderBindingTable
{
	BV_NOCOPYMOVE(BvShaderBindingTableVk);

public:
	BvShaderBindingTableVk(BvRenderDeviceVk* pDevice, const ShaderBindingTableDesc& sbtDesc,
		const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& props, BvCommandContextVk* pContext);
	~BvShaderBindingTableVk();

	BvRenderDevice* GetDevice() override;

	void GetAddressRegion(ShaderBindingTableGroupType groupType, u32 index, VkStridedDeviceAddressRegionKHR& addressRegion);
	BV_INLINE bool IsValid() const { return m_pBuffer != nullptr; }

private:
	void Create(BvCommandContextVk* pContext);
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	BvBufferVk* m_pBuffer = nullptr;
	VkStridedDeviceAddressRegionKHR m_Regions[u32(ShaderBindingTableGroupType::kCount)]{};
	u32 m_HandleSize = 0;
	u32 m_GroupHandleAlignment = 0;
	u32 m_BaseGroupAlignment = 0;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvShaderBindingTableVk);


BV_CREATE_CAST_TO_VK(BvShaderBindingTable)