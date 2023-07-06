#pragma once


#include "BDeV/RenderAPI/BvSwapChain.h"


class BvRenderDeviceGl;
class BvContextGl;


class BvSwapChainGl final : public BvSwapChain
{
public:
	BvSwapChainGl(const BvRenderDeviceGl& device, BvWindow* pWindow, const SwapChainDesc& swapChainParams);
	~BvSwapChainGl();

	void Create();
	void Destroy();

	void Present(bool vSync) override final;

	BV_INLINE u32 GetCurrentImageIndex() const override final { return 0; }
	BV_INLINE BvTextureView* GetTextureView(const u32 index) const override final { return nullptr; }

private:
	const BvRenderDeviceGl& m_Device;
	BvContextGl* m_pContext = nullptr;
};