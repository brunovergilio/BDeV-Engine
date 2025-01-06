#pragma once


#include "BDeV/Core/BvCore.h"


#if (BV_PLATFORM == BV_PLATFORM_WIN32)
#include <Windows.h>

#ifdef CreateFile
#undef CreateFile
#endif

#ifdef DeleteFile
#undef DeleteFile
#endif

#ifdef CreateDirectory
#undef CreateDirectory
#endif

#ifdef RemoveDirectory
#undef RemoveDirectory
#endif

#ifdef GetFileAttributes
#undef GetFileAttributes
#endif

#ifdef CreateSemaphore
#undef CreateSemaphore
#endif

#ifdef CreateWindow
#undef CreateWindow
#endif

#ifdef Yield
#undef Yield
#endif

using OSFileHandle = HANDLE;
const OSFileHandle kNullOSFileHandle = INVALID_HANDLE_VALUE;

using OSThreadHandle = HANDLE;
constexpr OSThreadHandle kNullOSThreadHandle = nullptr;

#if !defined(BV_USE_OS_FIBERS)
namespace Internal
{
	struct OSFiberHandle
	{
		void* m_pMemory;
		void* m_pContext;
	};
}
using OSFiberHandle = Internal::OSFiberHandle;
#else
using OSFiberHandle = void*;
#endif
constexpr OSFiberHandle kNullOSFiberHandle = {};

using OSSharedLibHandle = HMODULE;
constexpr OSSharedLibHandle kNullOSSharedLibHandle = nullptr;

using OSWindowHandle = HWND;
constexpr OSWindowHandle kNullOSWindowHandle = nullptr;

using OSMonitorHandle = HMONITOR;
constexpr OSMonitorHandle kNullOSMonitorHandle = nullptr;

#endif