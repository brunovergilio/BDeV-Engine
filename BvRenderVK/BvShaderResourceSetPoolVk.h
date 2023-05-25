#pragma once


#include "BvCommonVk.h"
#include "BDeV/RenderAPI/BvShaderResource.h"


class BvRenderDeviceVk;
class BvShaderResourceParamsVk;


class BvShaderResourceSetPoolVk final : public BvShaderResourceSetPool
{
public:
	BvShaderResourceSetPoolVk(const BvRenderDeviceVk & device, const ShaderResourceSetPoolDesc & desc);
	~BvShaderResourceSetPoolVk();

	void Create(const ShaderResourceSetPoolDesc & desc);
	void Destroy();

	void AllocateSets(u32 count, BvShaderResourceParams ** ppSets,
		const BvShaderResourceLayout * const pLayout, u32 set = 0) override;
	void FreeSets(u32 count, BvShaderResourceParams ** ppSets) override;

	BV_INLINE VkDescriptorPool GetHandle() const { return m_DescriptorPool; }

private:
	const BvRenderDeviceVk & m_Device;
	VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
	BvRobinMap<const BvShaderResourceLayout*, BvVector<BvShaderResourceParamsVk*>> m_UsedSets;
	BvRobinMap<const BvShaderResourceLayout*, BvVector<BvShaderResourceParamsVk*>> m_FreeSets;
};