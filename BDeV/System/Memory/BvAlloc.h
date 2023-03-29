#pragma once


#if (BV_PLATFORM == BV_PLATFORM_WIN32)
#include "BDeV/System/Memory/Windows/BvAllocWindows.h"
#else
#error "Platform not yet supported"
#endif