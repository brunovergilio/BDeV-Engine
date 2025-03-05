#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/RenderAPI/BvShaderBindingTable.h"


class IBvRenderDeviceVk;
class IBvBufferVk;
class IBvCommandContextVk;


BV_OBJECT_DEFINE_ID(IBvShaderBindingTableVk, "0bb5abad-c53b-47bd-b939-989cb9d2e58d");
class IBvShaderBindingTableVk : public IBvShaderBindingTable
{
	BV_NOCOPYMOVE(IBvShaderBindingTableVk);

public:
	virtual void GetAddressRegion(ShaderBindingTableGroupType groupType, u32 index, VkStridedDeviceAddressRegionKHR& addressRegion) const = 0;
	virtual bool IsValid() const = 0;

protected:
	IBvShaderBindingTableVk() {}
	~IBvShaderBindingTableVk() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvShaderBindingTableVk);


class BvShaderBindingTableVk final : public IBvShaderBindingTableVk
{
	BV_NOCOPYMOVE(BvShaderBindingTableVk);

public:
	BvShaderBindingTableVk(IBvRenderDeviceVk* pDevice, const ShaderBindingTableDesc& sbtDesc,
		const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& props, IBvCommandContextVk* pContext);
	~BvShaderBindingTableVk();

	IBvRenderDevice* GetDevice() override;
	BV_INLINE const ShaderBindingTableDesc& GetDesc() const override { return m_SBTDesc; }
	void GetAddressRegion(ShaderBindingTableGroupType groupType, u32 index, VkStridedDeviceAddressRegionKHR& addressRegion) const override;
	BV_INLINE bool IsValid() const override { return m_pBuffer != nullptr; }

	BV_OBJECT_IMPL_INTERFACE(IBvShaderBindingTableVk, IBvShaderBindingTable, IBvRenderDeviceObject);

private:
	void Create(IBvCommandContextVk* pContext);
	void Destroy();

private:
	IBvRenderDeviceVk* m_pDevice = nullptr;
	IBvBufferVk* m_pBuffer = nullptr;
	ShaderBindingTableDesc m_SBTDesc;
	VkStridedDeviceAddressRegionKHR m_Regions[u32(ShaderBindingTableGroupType::kCount)]{};
	u32 m_HandleSize = 0;
	u32 m_GroupHandleAlignment = 0;
	u32 m_BaseGroupAlignment = 0;
};


BV_CREATE_CAST_TO_VK(IBvShaderBindingTable)