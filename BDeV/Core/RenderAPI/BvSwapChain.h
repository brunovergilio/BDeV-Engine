#pragma once


#include "BvRenderCommon.h"
#include "BDeV/Core/Container/BvString.h"
#include "BDeV/Core/Utils/BvObject.h"


class IBvTextureView;
class BvSemaphore;
class BvWindow;


//BV_OBJECT_DEFINE_ID(IBvSwapChain, "239e8b95-158a-4313-8f79-f68369441372");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvSwapChain);
class IBvSwapChain : public BvRCObj
{
public:
	virtual void Present(bool vSync = false) = 0;
	virtual u32 GetCurrentImageIndex() const = 0;
	virtual IBvTextureView* GetTextureView(u32 index) const = 0;

	virtual IBvTextureView* GetCurrentTextureView() const = 0;
	virtual BvWindow* GetWindow() const = 0;
	virtual const SwapChainDesc& GetDesc() const = 0;

protected:
	IBvSwapChain() {}
	~IBvSwapChain() {}
};