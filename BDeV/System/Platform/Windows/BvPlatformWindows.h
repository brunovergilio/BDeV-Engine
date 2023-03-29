#pragma once


#include "BDeV/System/Platform/BvPlatform.h"
#include "BDeV/Container/BvRobinMap.h"
#include <Windows.h>


#if defined(CreateWindow)
#undef CreateWindow
#endif


class BvPlatformWindows
{
	BV_NOCOPYMOVE(BvPlatformWindows);

public:
	static LRESULT WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};