#pragma once


#include "BDeV/Container/BvVector.h"
#include "BDeV/System/Window/BvWindow.h"


#if defined(CreateWindow)
#undef CreateWindow
#endif


class BV_API BvPlatform
{
	BV_NOCOPYMOVE(BvPlatform);

public:
	static void Initialize();
	static void Shutdown();

	static void Update();

	static class BvWindow* CreateWindow(const WindowDesc& windowDesc);
	static void DestroyWindow(BvWindow* pWindow);

private:
	BvPlatform() {}
	virtual ~BvPlatform() {}
};