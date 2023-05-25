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
	static constexpr const char* const s_WindowClassName = "BDeVWindowClass";

	static void Initialize();
	static void Shutdown();

	static void ProcessOSEvents();

	static BvWindow* CreateWindow(const WindowDesc& windowDesc);
	static void DestroyWindow(BvWindow* pWindow);

private:
	BvPlatform() {}
	~BvPlatform() {}

#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	static LRESULT WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif
};