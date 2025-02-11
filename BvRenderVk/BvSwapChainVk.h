#pragma once


#include "BDeV/Core/RenderAPI/BvSwapChain.h"
#include "BDeV/Core/System/Threading/BvSync.h"
#include "BvSemaphoreVk.h"
#include "BvTextureViewVk.h"


class BvTextureVk;
class BvTextureViewVk;
class BvCommandQueueVk;
class BvCommandContextVk;
class BvGPUFenceVk;


BV_OBJECT_DEFINE_ID(BvSwapChainVk, "68fe17c5-c20d-4c33-83dc-22ac819ab867");
class BvSwapChainVk final : public BvSwapChain
{
public:
	BvSwapChainVk(BvRenderDeviceVk* pDevice, BvWindow* pWindow, const SwapChainDesc& swapChainParams, BvCommandContext* pContext);
	~BvSwapChainVk();

	void AcquireImage();
	void Present(bool vSync) override;
	void SetCurrentFence(BvGPUFenceVk* pFence, u64 value);

	BvRenderDevice* GetDevice() override;
	BV_INLINE u32 GetCurrentImageIndex() const override { return m_CurrImageIndex; }
	BV_INLINE BvTextureView* GetTextureView(u32 index) const override { return m_SwapChainTextureViews[index]; }
	BV_INLINE const BvSemaphoreVk* GetCurrentImageAcquiredSemaphore() const { return &m_ImageAcquiredSemaphores[m_CurrImageIndex]; }
	BV_INLINE const BvSemaphoreVk* GetCurrentRenderCompleteSemaphore() const { return &m_RenderCompleteSemaphores[m_CurrImageIndex]; }
	BV_INLINE bool IsValid() const { return m_Swapchain != VK_NULL_HANDLE; }

	BV_OBJECT_IMPL_INTERFACE(BvSwapChainVk, BvSwapChain, IBvRenderDeviceObject);

private:
	bool Create();
	void Destroy();

	void CreateSurface();
	void DestroySurface();
	void Resize();
	void CreateSynchronizationResources();
	void DestroySynchronizationResources();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	BvCommandQueueVk* m_pCommandQueue = nullptr;
	BvCommandContextVk* m_pCommandContext = nullptr;

	VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
	VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;

	BvVector<BvTextureVk*> m_SwapChainTextures;
	BvVector<BvTextureViewVk*> m_SwapChainTextureViews;

	BvVector<BvSemaphoreVk> m_ImageAcquiredSemaphores;
	BvVector<BvSemaphoreVk> m_RenderCompleteSemaphores;
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
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvSwapChainVk);


BV_CREATE_CAST_TO_VK(BvSwapChain)