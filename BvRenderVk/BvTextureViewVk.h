#pragma once


#include "BDeV/Core/RenderAPI/BvTextureView.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


BV_OBJECT_DEFINE_ID(BvTextureViewVk, "7ce274c0-e876-4b9a-8016-4b8ca8c84640");
class BvTextureViewVk final : public IBvTextureView, public IBvResourceVk
{
	BV_NOCOPYMOVE(BvTextureViewVk);
	BV_VK_DEVICE_RES_DECL;

public:
	BvTextureViewVk(BvRenderDeviceVk* pDevice, const TextureViewDesc& textureViewDesc);
	~BvTextureViewVk();

	BV_INLINE const TextureViewDesc& GetDesc() const override { return m_TextureViewDesc; }
	BV_INLINE VkImageView GetHandle() const { return m_View; }
	BV_INLINE bool IsValid() const { return m_View != VK_NULL_HANDLE; }

	//BV_OBJECT_IMPL_INTERFACE(IBvTextureViewVk, IBvTextureView, IBvRenderDeviceObject);

private:
	bool Create();
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkImageView m_View = VK_NULL_HANDLE;
	TextureViewDesc m_TextureViewDesc;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvTextureViewVk);


BV_CREATE_CAST_TO_VK(BvTextureView)