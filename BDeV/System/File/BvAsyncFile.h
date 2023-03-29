#pragma once


#include "BDeV/BvCore.h"


#if (BV_PLATFORM == BV_PLATFORM_WIN32)
#include "BDeV/System/File/Windows/BvAsyncFileWindows.h"
#else
#error "Platform not yet supported"
#endif