#pragma once


#include "BDeV/Core/RenderAPI/BvTextureView.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


class BvTextureViewVk final : public IBvTextureView, public IBvResourceVk
{
public:
	BvTextureViewVk(BvRenderDeviceVk* pDevice, const TextureViewDesc& textureViewDesc, VkImageView view);
	~BvTextureViewVk();

	BV_INLINE const TextureViewDesc& GetDesc() const override { return m_TextureViewDesc; }
	BV_INLINE VkImageView GetHandle() const { return m_View; }
	BV_INLINE bool IsValid() const { return m_View != VK_NULL_HANDLE; }

private:
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkImageView m_View = VK_NULL_HANDLE;
	TextureViewDesc m_TextureViewDesc;
};
BV_OBJECT_DEFINE_ID(BvTextureViewVk, "7ce274c0-e876-4b9a-8016-4b8ca8c84640");
BV_CREATE_CAST_TO_VK(BvTextureView)