#pragma once


#include "BvCommonVk.h"
#include "BvRender/BvShaderResource.h"


class BvRenderDeviceVk;


class BvShaderResourceSetPoolVk final : public BvShaderResourceSetPool
{
public:
	BvShaderResourceSetPoolVk(const BvRenderDeviceVk & device, const ShaderResourceSetPoolDesc & desc);
	~BvShaderResourceSetPoolVk();

	void Create(const ShaderResourceSetPoolDesc & desc);
	void Destroy();

	void AllocateSets(const u32 count, BvShaderResourceSet ** ppSets,
		const BvShaderResourceLayout * const pLayout, const u32 set = 0) override final;
	void FreeSets(const u32 count, BvShaderResourceSet ** ppSets) override final;

	BV_INLINE VkDescriptorPool GetHandle() const { return m_DescriptorPool; }

private:
	const BvRenderDeviceVk & m_Device;
	VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
};