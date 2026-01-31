#pragma once


#include "BDeV/Core/RenderAPI/BvSwapChain.h"
#include "BDeV/Core/System/Threading/BvSync.h"
#include "BvCommonD3D12.h"
#include "BvTextureViewD3D12.h"


class BvRenderDeviceD3D12;
class BvTextureD3D12;
class BvTextureViewD3D12;
class BvCommandQueueD3D12;
class BvCommandContextD3D12;
class BvGPUFenceD3D12;


class BvSwapChainD3D12 final : public IBvSwapChain, public IBvResourceD3D12
{
public:
	BvSwapChainD3D12(BvRenderDeviceD3D12* pDevice, BvWindow* pWindow, const SwapChainDesc& swapChainParams, BvCommandContextD3D12* pContext,
		ComPtr<IDXGISwapChain3>& swapChain, BvVector<ID3D12Resource*>& textures, UINT presentFlags);
	~BvSwapChainD3D12();

	void AcquireImage();
	void Present(bool vSync) override;
	void SetWindowMode(SwapChainMode mode, BvMonitor* pMonitor = nullptr) override;

	BV_INLINE IBvTextureView* GetCurrentTextureView() const override { return GetTextureView(GetCurrentImageIndex()); }
	BV_INLINE BvWindow* GetWindow() const override { return m_pWindow; }
	BV_INLINE const SwapChainDesc& GetDesc() const override { return m_SwapChainDesc; }
	BV_INLINE u32 GetCurrentImageIndex() const override { return m_CurrImageIndex; }
	BV_INLINE IBvTextureView* GetTextureView(u32 index) const override { return m_SwapChainTextureViews[index]; }
	BV_INLINE bool IsValid() const { return m_SwapChain != nullptr; }

private:
	void CreateTextureResources(BvVector<ID3D12Resource*>& textures);
	void DestroyTextureResources();
	//std::pair<u32, u32> GetRefreshRate(IDXGIFactory2* pFactory2, DXGI_FORMAT format);
	void Destroy();

	void Resize();
	void SetTrueFullscreen(bool value);

private:
	BvWindow* m_pWindow = nullptr;
	SwapChainDesc m_SwapChainDesc;
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	BvCommandContextD3D12* m_pCommandContext = nullptr;

	ComPtr<IDXGISwapChain3> m_SwapChain;

	BvVector<BvTextureD3D12*> m_SwapChainTextures;
	BvVector<BvTextureViewD3D12*> m_SwapChainTextureViews;

	u32 m_CurrImageIndex = 0;
	UINT m_PresentFlags = 0;
};
BV_OBJECT_DEFINE_ID(BvSwapChainD3D12, "68fe17c5-c20d-4c33-83dc-22ac819ab867");
BV_CREATE_CAST_TO_D3D12(BvSwapChain)