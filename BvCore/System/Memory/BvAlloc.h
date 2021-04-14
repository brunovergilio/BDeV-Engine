#pragma once


#if (BV_PLATFORM == BV_PLATFORM_WIN32)
#include "BvCore/System/Memory/Win32/BvAllocWin32.h"
#else
#error "Platform not yet supported"
#endif