#pragma once


#include "BvCore/BvCore.h"


#if (BV_PLATFORM == BV_PLATFORM_WIN32)
#include "BvCore/System/Window/Win32/BvNativeWindowWin32.h"
#else
#error "Platform not yet supported"
#endif