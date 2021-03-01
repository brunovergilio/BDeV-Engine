#pragma once


#include "BvCore/BvCore.h"


#if (BV_PLATFORM == BV_PLATFORM_WIN32)
#include "BvCore/System/Library/Win32/BvSharedLibWin32.h"
#else
#error "Platform not yet supported"
#endif