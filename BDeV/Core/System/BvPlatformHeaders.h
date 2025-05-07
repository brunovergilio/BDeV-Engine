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

#ifdef GetMessage
#undef GetMessage
#endif

using OSFileHandle = HANDLE;
using OSThreadHandle = HANDLE;
#if defined(BV_USE_ASM_FIBERS)
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
using OSSharedLibHandle = HMODULE;
using OSWindowHandle = HWND;
using OSMonitorHandle = HMONITOR;
#endif


const OSFileHandle kNullOSFileHandle = INVALID_HANDLE_VALUE;
constexpr OSThreadHandle kNullOSThreadHandle = nullptr;
constexpr OSFiberHandle kNullOSFiberHandle = {};
constexpr OSSharedLibHandle kNullOSSharedLibHandle = nullptr;
constexpr OSWindowHandle kNullOSWindowHandle = nullptr;
constexpr OSMonitorHandle kNullOSMonitorHandle = nullptr;