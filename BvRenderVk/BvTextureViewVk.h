#pragma once


#include "BDeV/Core/RenderAPI/BvTextureView.h"
#include "BvTextureVk.h"


class IBvRenderDeviceVk;


BV_OBJECT_DEFINE_ID(IBvTextureViewVk, "7ce274c0-e876-4b9a-8016-4b8ca8c84640");
class IBvTextureViewVk : public IBvTextureView
{
	BV_NOCOPYMOVE(IBvTextureViewVk);

public:
	virtual VkImageView GetHandle() const = 0;
	virtual bool IsValid() const = 0;

protected:
	IBvTextureViewVk() {}
	~IBvTextureViewVk() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvTextureViewVk);


class BvTextureViewVk final : public IBvTextureViewVk
{
	BV_NOCOPYMOVE(BvTextureViewVk);

public:
	BvTextureViewVk(IBvRenderDeviceVk* pDevice, const TextureViewDesc& textureViewDesc);
	~BvTextureViewVk();

	IBvRenderDevice* GetDevice() override;
	BV_INLINE const TextureViewDesc& GetDesc() const override { return m_TextureViewDesc; }
	BV_INLINE VkImageView GetHandle() const override { return m_View; }
	BV_INLINE bool IsValid() const override { return m_View != VK_NULL_HANDLE; }

	BV_OBJECT_IMPL_INTERFACE(IBvTextureViewVk, IBvTextureView, IBvRenderDeviceObject);

private:
	bool Create();
	void Destroy();

private:
	IBvRenderDeviceVk* m_pDevice = nullptr;
	VkImageView m_View = VK_NULL_HANDLE;
	TextureViewDesc m_TextureViewDesc;
};


BV_CREATE_CAST_TO_VK(IBvTextureView)