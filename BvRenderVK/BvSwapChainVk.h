#pragma once


#include "BvRender/BvSwapChain.h"
#include "BvRenderDeviceVk.h"
#include "BvTextureViewVk.h"
#include "BvCore/System/Threading/BvSync.h"


class BvTextureVk;
class BvSwapChainTextureVk;
class BvFenceVk;
class BvSemaphoreVk;
class BvCommandQueueVk;


class BvSwapChainVk final : public BvSwapChain
{
public:
	BvSwapChainVk(const BvRenderDeviceVk & renderDevice, BvCommandQueueVk & commandQueue,
		BvNativeWindow & window, const SwapChainDesc & swapChainParams);
	~BvSwapChainVk();

	bool Create();
	void Destroy();

	void Present(bool vSync) override final;

	BvSemaphoreVk * RegisterSignalSemaphore();

	BV_INLINE u32 GetCurrentImageIndex() const override final { return m_CurrImageIndex; }
	BV_INLINE BvTextureView * GetTextureView(const u32 index) const override final { return m_SwapChainTextureViews[index]; }
	BV_INLINE BvSemaphoreVk * GetCurrentSemaphore() const { return m_ImageAcquiredSemaphores[m_CurrImageIndex]; }

private:
	void CreateSurface();
	void DestroySurface();
	void Resize();
	void AcquireImage();
	void CreateSynchronizationResources();
	void DestroySynchronizationResources();

private:
	struct WaitSemaphoreCount
	{
		WaitSemaphoreCount() {}
		WaitSemaphoreCount(const WaitSemaphoreCount& rhs) : count(rhs.count.load()) {}
		WaitSemaphoreCount& operator=(const WaitSemaphoreCount& rhs) { count = rhs.count.load(); return *this; }
		WaitSemaphoreCount(WaitSemaphoreCount&& rhs) noexcept { *this = std::move(rhs); }
		WaitSemaphoreCount& operator=(WaitSemaphoreCount&& rhs) noexcept { count = rhs.count.load(); return *this; }
		std::atomic<u32> count;
	};
	const BvRenderDeviceVk & m_Device;
	BvCommandQueueVk & m_CommandQueue;

	VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
	VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;

	BvVector<BvSwapChainTextureVk *> m_SwapChainTextures;
	BvVector<BvTextureViewVk *> m_SwapChainTextureViews;

	BvVector<BvSemaphoreVk *> m_ImageAcquiredSemaphores;
	BvVector<BvVector<BvSemaphoreVk *>> m_SignalSemaphores;
	// Using atomics in case I want to render to the swap chain from multiple command buffers from separate threads
	BvVector<WaitSemaphoreCount> m_WaitSemaphoreCount;
	BvVector<BvFenceVk *> m_ImageAcquiredFences;
	u32 m_CurrSemaphoreIndex = 0;

	u32 m_PresentationQueueIndex = 0;
	u32 m_CurrImageIndex = 0;
};