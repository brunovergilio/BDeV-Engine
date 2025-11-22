#pragma once


#include "BDeV/Core/BvCore.h"


#if BV_PLATFORM_WIN32
#include <Windows.h>
#include <WinSock2.h>
#include <MSWSock.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

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
using OSSocketHandle = SOCKET;


const OSFileHandle kNullOSFileHandle = INVALID_HANDLE_VALUE;
constexpr OSThreadHandle kNullOSThreadHandle = nullptr;
constexpr OSFiberHandle kNullOSFiberHandle = {};
constexpr OSSharedLibHandle kNullOSSharedLibHandle = nullptr;
constexpr OSWindowHandle kNullOSWindowHandle = nullptr;
constexpr OSMonitorHandle kNullOSMonitorHandle = nullptr;
const OSSocketHandle kNullOSSocketHandle = INVALID_SOCKET;

#define BV_SOCKET_ERROR_CODE WSAGetLastError()

#endif // #if BV_PLATFORM_WIN32