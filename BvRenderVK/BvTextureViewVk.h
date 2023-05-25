#pragma once


#include "BDeV/RenderAPI/BvTextureView.h"
#include "BvTextureVk.h"


class BvRenderDeviceVk;


class BvTextureViewVk final : public BvTextureView
{
	BV_NOCOPYMOVE(BvTextureViewVk);

public:
	BvTextureViewVk(const BvRenderDeviceVk & device, const TextureViewDesc & textureViewDesc);
	~BvTextureViewVk() override;

	bool Create();
	void Destroy();

	BV_INLINE VkImageView GetHandle() const { return m_View; }

private:
	const BvRenderDeviceVk & m_Device;
	VkImageView m_View = VK_NULL_HANDLE;
};