#pragma once


#include "BDeV/Core/RenderAPI/BvSwapChain.h"
#include "BDeV/Core/System/Threading/BvSync.h"
#include "BvSemaphoreVk.h"
#include "BvTextureViewVk.h"


class IBvRenderDeviceVk;
class IBvTextureVk;
class IBvTextureViewVk;
class BvCommandQueueVk;
class IBvCommandContextVk;
class IBvGPUFenceVk;


BV_OBJECT_DEFINE_ID(IBvSwapChainVk, "68fe17c5-c20d-4c33-83dc-22ac819ab867");
class IBvSwapChainVk : public IBvSwapChain
{
public:
	virtual void AcquireImage() = 0;
	virtual void SetCurrentFence(IBvGPUFenceVk* pFence, u64 value) = 0;
	virtual const BvSemaphoreVk* GetCurrentImageAcquiredSemaphore() const = 0;
	virtual const BvSemaphoreVk* GetCurrentRenderCompleteSemaphore() const = 0;
	virtual bool IsValid() const = 0;

protected:
	IBvSwapChainVk() {}
	~IBvSwapChainVk() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvSwapChainVk);


class BvSwapChainVk final : public IBvSwapChainVk
{
public:
	BvSwapChainVk(IBvRenderDeviceVk* pDevice, BvWindow* pWindow, const SwapChainDesc& swapChainParams, IBvCommandContext* pContext);
	~BvSwapChainVk();

	void AcquireImage() override;
	void Present(bool vSync) override;
	void SetCurrentFence(IBvGPUFenceVk* pFence, u64 value) override;

	IBvRenderDevice* GetDevice() override;
	BV_INLINE IBvTextureView* GetCurrentTextureView() const override { return GetTextureView(GetCurrentImageIndex()); }
	BV_INLINE BvWindow* GetWindow() const override { return m_pWindow; }
	BV_INLINE const SwapChainDesc& GetDesc() const override { return m_SwapChainDesc; }
	BV_INLINE u32 GetCurrentImageIndex() const override { return m_CurrImageIndex; }
	BV_INLINE IBvTextureView* GetTextureView(u32 index) const override { return m_SwapChainTextureViews[index]; }
	BV_INLINE const BvSemaphoreVk* GetCurrentImageAcquiredSemaphore() const override { return &m_ImageAcquiredSemaphores[m_CurrImageIndex]; }
	BV_INLINE const BvSemaphoreVk* GetCurrentRenderCompleteSemaphore() const override { return &m_RenderCompleteSemaphores[m_CurrImageIndex]; }
	BV_INLINE bool IsValid() const override { return m_Swapchain != VK_NULL_HANDLE; }

	BV_OBJECT_IMPL_INTERFACE(IBvSwapChainVk, IBvSwapChain, IBvRenderDeviceObject);

private:
	bool Create();
	void Destroy();

	void CreateSurface();
	void DestroySurface();
	void Resize();
	void CreateSynchronizationResources();
	void DestroySynchronizationResources();

private:
	BvWindow* m_pWindow = nullptr;
	SwapChainDesc m_SwapChainDesc;
	IBvRenderDeviceVk* m_pDevice = nullptr;
	BvCommandQueueVk* m_pCommandQueue = nullptr;
	IBvCommandContextVk* m_pCommandContext = nullptr;

	VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
	VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;

	BvVector<IBvTextureVk*> m_SwapChainTextures;
	BvVector<IBvTextureViewVk*> m_SwapChainTextureViews;

	BvVector<BvSemaphoreVk> m_ImageAcquiredSemaphores;
	BvVector<BvSemaphoreVk> m_RenderCompleteSemaphores;
	u32 m_CurrSemaphoreIndex = 0;
	u32 m_CurrImageIndex = 0;

	struct FenceData
	{
		IBvGPUFenceVk* m_pFence = nullptr;
		u64 m_Value = 0;
	};
	BvVector<FenceData> m_Fences;

	// Make sure only one image has been acquired per presentation / frame
	bool m_IsReady = false;
};


BV_CREATE_CAST_TO_VK(IBvSwapChain)