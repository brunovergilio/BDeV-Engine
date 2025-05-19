#pragma once


#include "BDeV/Core/RenderAPI/BvSwapChain.h"
#include "BDeV/Core/System/Threading/BvSync.h"
#include "BvTextureViewVk.h"


class BvRenderDeviceVk;
class BvTextureVk;
class BvTextureViewVk;
class BvCommandQueueVk;
class BvCommandContextVk;
class BvGPUFenceVk;


//BV_OBJECT_DEFINE_ID(IBvSwapChainVk, "68fe17c5-c20d-4c33-83dc-22ac819ab867");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvSwapChainVk);


class BvSwapChainVk final : public IBvSwapChain, public IBvResourceVk
{
	BV_VK_DEVICE_RES_DECL;

public:
	BvSwapChainVk(BvRenderDeviceVk* pDevice, BvWindow* pWindow, const SwapChainDesc& swapChainParams, BvCommandContextVk* pContext);
	~BvSwapChainVk();

	void AcquireImage();
	void Present(bool vSync) override;
	void SetWindowMode(SwapChainMode mode, BvMonitor* pMonitor = nullptr) override;
	void SetCurrentFence(BvGPUFenceVk* pFence, u64 value);

	BV_INLINE IBvTextureView* GetCurrentTextureView() const override { return GetTextureView(GetCurrentImageIndex()); }
	BV_INLINE BvWindow* GetWindow() const override { return m_pWindow; }
	BV_INLINE const SwapChainDesc& GetDesc() const override { return m_SwapChainDesc; }
	BV_INLINE u32 GetCurrentImageIndex() const override { return m_CurrImageIndex; }
	BV_INLINE IBvTextureView* GetTextureView(u32 index) const override { return m_SwapChainTextureViews[index]; }
	BV_INLINE VkSemaphore GetCurrentImageAcquiredSemaphore() const { return m_ImageAcquiredSemaphores[m_CurrImageIndex]; }
	BV_INLINE VkSemaphore GetCurrentRenderCompleteSemaphore() const { return m_RenderCompleteSemaphores[m_CurrImageIndex]; }
	BV_INLINE bool IsValid() const { return m_Swapchain != VK_NULL_HANDLE; }
	BV_INLINE bool IsReady() const { return m_IsReady; }

	//BV_OBJECT_IMPL_INTERFACE(IBvSwapChainVk, IBvSwapChain, IBvRenderDeviceObject);

private:
	bool Create();
	void Destroy();

	void CreateSurface();
	void DestroySurface();
	void Resize();
	void SetTrueFullscreen(bool value);
	void CreateSynchronizationResources();
	void DestroySynchronizationResources();

private:
	BvWindow* m_pWindow = nullptr;
	SwapChainDesc m_SwapChainDesc;
	BvRenderDeviceVk* m_pDevice = nullptr;
	BvCommandQueueVk* m_pCommandQueue = nullptr;
	BvCommandContextVk* m_pCommandContext = nullptr;

	VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
	VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;

	BvVector<BvTextureVk*> m_SwapChainTextures;
	BvVector<BvTextureViewVk*> m_SwapChainTextureViews;

	BvVector<VkSemaphore> m_ImageAcquiredSemaphores;
	BvVector<VkSemaphore> m_RenderCompleteSemaphores;
	u32 m_CurrSemaphoreIndex = 0;
	u32 m_CurrImageIndex = 0;

	struct FenceData
	{
		BvGPUFenceVk* m_pFence = nullptr;
		u64 m_Value = 0;
	};
	BvVector<FenceData> m_Fences;

	// Make sure only one image has been acquired per presentation / frame
	bool m_IsReady = false;
	bool m_FullscreenAcquired = false;
};


BV_CREATE_CAST_TO_VK(BvSwapChain)