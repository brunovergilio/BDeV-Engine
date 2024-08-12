#pragma once


#include "BvRenderCommon.h"
#include "BDeV/Core/Container/BvString.h"


class BvTextureView;
class BvSemaphore;
class BvWindow;


struct SwapChainDesc
{
	u32 m_SwapChainImageCount = 3;
	Format m_Format = Format::kUnknown;
	bool m_VSync = false;
};


class BvSwapChain
{
public:
	virtual void Present(bool vSync = false) = 0;
	virtual u32 GetCurrentImageIndex() const = 0;
	virtual BvTextureView* GetTextureView(u32 index) const = 0;

	BV_INLINE BvTextureView* GetCurrentTextureView() const { return GetTextureView(GetCurrentImageIndex()); }
	BV_INLINE BvWindow* GetWindow() { return m_pWindow; }
	BV_INLINE const SwapChainDesc& GetDesc() const { return m_SwapChainDesc; }

protected:
	BvSwapChain(BvWindow* pWindow, const SwapChainDesc& swapChainDesc)
		: m_pWindow(pWindow), m_SwapChainDesc(swapChainDesc)
	{
	}
	virtual ~BvSwapChain() = 0 {}

protected:
	BvWindow* m_pWindow;
	SwapChainDesc m_SwapChainDesc;
};