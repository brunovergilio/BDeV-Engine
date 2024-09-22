#pragma once


#include "BDeV/Core/RenderAPI/BvTextureView.h"
#include "BvTextureVk.h"


class BvRenderDeviceVk;


class BvTextureViewVk final : public BvTextureView
{
	BV_NOCOPYMOVE(BvTextureViewVk);

public:
	BvTextureViewVk(BvRenderDeviceVk* pDevice, const TextureViewDesc & textureViewDesc);
	~BvTextureViewVk();
	
	BvRenderDevice* GetDevice() override;

	BV_INLINE VkImageView GetHandle() const { return m_View; }

private:
	bool Create();
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkImageView m_View = VK_NULL_HANDLE;
};


BV_CREATE_CAST_TO_VK(BvTextureView)