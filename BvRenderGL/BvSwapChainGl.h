#pragma once


#include "BDeV/Core/RenderAPI/BvSwapChain.h"
#include "BvContextGl.h"
#include "BvTextureGl.h"
#include "BvTextureViewGl.h"


class BvRenderDeviceGl;
class BvCommandContextGl;


class BvSwapChainGl final : public IBvSwapChain
{
public:
	BvSwapChainGl(BvRenderDeviceGl* pDevice, BvWindow* pWindow, const SwapChainDesc& swapChaindesc, BvCommandContextGl* pCommandContext);
	~BvSwapChainGl();

	void Create();
	void Destroy();

	void Present(bool vSync) override;

	BV_INLINE u32 GetCurrentImageIndex() const override { return 0; }
	BV_INLINE IBvTextureView* GetCurrentTextureView() const override { return m_pTextureView; }
	BV_INLINE IBvTextureView* GetTextureView(const u32 index) const override { return m_pTextureView; }
	BV_INLINE BvWindow* GetWindow() const override { return m_pWindow; }
	BV_INLINE const SwapChainDesc& GetDesc() const override { return m_SwapChainDesc; }
	void SetWindowMode(SwapChainMode mode, BvMonitor* pMonitor = nullptr) override;

private:
	BvRenderDeviceGl* m_pDevice = nullptr;
	BvWindow* m_pWindow = nullptr;
	BvContextGl m_Context;
	BvCommandContextGl* m_pCommandContext = nullptr;
	SwapChainDesc m_SwapChainDesc;
	BvTextureGl* m_pTexture = nullptr;
	BvTextureViewGl* m_pTextureView = nullptr;
};


BV_CREATE_CAST_TO_GL(BvSwapChain)