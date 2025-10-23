#include "BvNetworkWindows.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"



namespace Internal
{
	BvWinsockInitializer::BvWinsockInitializer()
	{
		WSAData data;
		if (WSAStartup(WINSOCK_VERSION, &data))
		{
			BV_SYS_FATAL(BV_SOCKET_ERROR_CODE);
		}
	}


	BvWinsockInitializer::~BvWinsockInitializer()
	{
		if (WSACleanup())
		{
			BV_SYS_FATAL(BV_SOCKET_ERROR_CODE);
		}
	}


	const BvWinsockInitializer& BvWinsockInitializer::GetInstance()
	{
		static BvWinsockInitializer instance;
		return instance;
	}
}