#pragma once


#include "BDeV/Core/RenderAPI/BvSwapChain.h"
#include "BDeV/Core/System/Threading/BvSync.h"
#include "BvSemaphoreVk.h"
#include "BvTextureViewVk.h"


class BvTextureVk;
class BvTextureViewVk;
class BvCommandQueueVk;


class BvSwapChainVk final : public BvSwapChain
{
public:
	BvSwapChainVk(BvRenderDeviceVk* pDevice, BvWindow* pWindow, const SwapChainDesc& swapChainParams, BvCommandContext* pContext);
	~BvSwapChainVk();

	void Present(bool vSync) override;

	BvRenderDevice* GetDevice() override;
	BV_INLINE u32 GetCurrentImageIndex() const override { return m_CurrImageIndex; }
	BV_INLINE BvTextureView* GetTextureView(u32 index) const override { return m_SwapChainTextureViews[index]; }
	BV_INLINE const BvSemaphoreVk* GetCurrentImageAcquiredSemaphore() const { return &m_ImageAcquiredSemaphores[m_CurrImageIndex]; }
	BV_INLINE const BvSemaphoreVk* GetCurrentRenderCompleteSemaphore() const { return &m_RenderCompleteSemaphores[m_CurrImageIndex]; }

private:
	bool Create();
	void Destroy();

	void CreateSurface();
	void DestroySurface();
	void Resize();
	void AcquireImage();
	void CreateSynchronizationResources();
	void DestroySynchronizationResources();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	BvCommandQueueVk* m_pCommandQueue = nullptr;

	VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
	VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;

	BvVector<BvTextureVk*> m_SwapChainTextures;
	BvVector<BvTextureViewVk*> m_SwapChainTextureViews;

	BvVector<BvSemaphoreVk> m_ImageAcquiredSemaphores;
	BvVector<BvSemaphoreVk> m_RenderCompleteSemaphores;
	u32 m_CurrSemaphoreIndex = 0;

	u32 m_CurrImageIndex = 0;
};


BV_CREATE_CAST_TO_VK(BvSwapChain)