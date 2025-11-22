#pragma once


#include "BDeV/Core/System/BvPlatformHeaders.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"


enum class NetAddressFamily : u16
{
	kUnknown = u16(AF_UNSPEC),
	kIPv4 = u16(AF_INET),
	kIPv6 = u16(AF_INET6)
};


enum class SocketType : u8
{
	kUnknown,
	kTCP = u8(SOCK_STREAM),
	kUDP = u8(SOCK_DGRAM)
};


#if BV_PLATFORM_WIN32
#include "BDeV/Core/System/Network/Windows/BvNetworkWindows.h"
#define BV_NETWORK_INIT Internal::BvWinsockInitializer::GetInstance()
#else
#define BV_NETWORK_INIT
#endif