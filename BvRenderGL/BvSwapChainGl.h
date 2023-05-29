#pragma once


#include "BDeV/RenderAPI/BvSwapChain.h"


class BvContextGl;


class BvSwapChainGl final : public BvSwapChain
{
public:
	BvSwapChainGl(BvWindow* pWindow, const SwapChainDesc& swapChainParams);
	~BvSwapChainGl();

	void Create();
	void Destroy();

	void Present(bool vSync) override final;

	BV_INLINE u32 GetCurrentImageIndex() const override final { return 0; }
	BV_INLINE BvTextureView* GetTextureView(const u32 index) const override final { return nullptr; }

private:
	BvContextGl* m_pContext = nullptr;
};