#pragma once


#include "BvRender/BvCommon.h"
#include "BvCore/Container/BvString.h"
#include "BvCore/System/Window/BvNativeWindow.h"


class BvTextureView;
class BvSemaphore;


struct SwapChainDesc
{
	u32 m_SwapChainImageCount = 3;
	Format m_Format = Format::kUndefined;
	bool m_VSync = false;
};


constexpr u32 kMaxWaitSemaphores = 8;


class BvSwapChain
{
public:
	virtual void Present(bool vSync = false) = 0;
	virtual u32 GetCurrentImageIndex() const = 0;
	virtual BvTextureView * GetTextureView(const u32 index) const = 0;

	BV_INLINE BvNativeWindow & GetWindow() const { return m_Window; }
	BV_INLINE const SwapChainDesc & GetDesc() const { return m_SwapChainDesc; }

protected:
	BvSwapChain(BvNativeWindow & window, const SwapChainDesc & swapChainDesc)
		: m_Window(window), m_SwapChainDesc(swapChainDesc) {}
	virtual ~BvSwapChain() = 0 {}

protected:
	BvNativeWindow & m_Window;
	SwapChainDesc m_SwapChainDesc;
};