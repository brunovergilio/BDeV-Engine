#pragma once


#include "BvCore/BvCore.h"


#if (BV_PLATFORM == BV_PLATFORM_WIN32)
#include "BvCore/System/File/Win32/BvFileSystemWin32.h"
#else
#error "Platform not yet supported"
#endif