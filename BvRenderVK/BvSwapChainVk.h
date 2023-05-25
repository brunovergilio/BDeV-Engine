#pragma once


#include "BDeV/RenderAPI/BvSwapChain.h"
#include "BvRenderDeviceVk.h"
#include "BvTextureViewVk.h"
#include "BDeV/System/Threading/BvSync.h"


class BvTextureVk;
class BvSwapChainTextureVk;
class BvSyncObjectVk;
class BvSemaphoreVk;
class BvCommandQueueVk;


class BvSwapChainVk final : public BvSwapChain
{
public:
	BvSwapChainVk(const BvRenderDeviceVk & renderDevice, BvCommandQueueVk & commandQueue, BvWindow* pWindow, const SwapChainDesc & swapChainParams);
	~BvSwapChainVk();

	bool Create();
	void Destroy();

	void Present(bool vSync) override final;

	BV_INLINE u32 GetCurrentImageIndex() const override final { return m_CurrImageIndex; }
	BV_INLINE BvTextureView* GetTextureView(const u32 index) const override final { return m_SwapChainTextureViews[index]; }
	BV_INLINE BvSemaphoreVk* GetCurrentImageAcquiredSemaphore() const { return m_ImageAcquiredSemaphores[m_CurrImageIndex]; }
	BV_INLINE BvSemaphoreVk* GetCurrentRenderCompleteSemaphore() const { return m_RenderCompleteSemaphores[m_CurrImageIndex]; }

private:
	void CreateSurface();
	void DestroySurface();
	void Resize();
	void AcquireImage();
	void CreateSynchronizationResources();
	void DestroySynchronizationResources();

private:
	const BvRenderDeviceVk & m_Device;
	BvCommandQueueVk & m_CommandQueue;

	VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
	VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;

	BvVector<BvSwapChainTextureVk *> m_SwapChainTextures;
	BvVector<BvTextureViewVk *> m_SwapChainTextureViews;

	BvVector<BvSemaphoreVk *> m_ImageAcquiredSemaphores;
	BvVector<BvSemaphoreVk *> m_RenderCompleteSemaphores;
	u32 m_CurrSemaphoreIndex = 0;

	u32 m_PresentationQueueIndex = 0;
	u32 m_CurrImageIndex = 0;
};