#include "BvCore/Utils/Win32/BvDebugWin32.h"
#include <Windows.h>
#include <cstdio>


int VDPrintF(const char* format, va_list argList);


void RaiseError(const char* const pMessage, const char* pTitle)
{
	auto result = MessageBoxA(nullptr, pMessage, pTitle, MB_ABORTRETRYIGNORE | MB_ICONERROR);
	if (result == IDABORT)
	{
		exit(1);
	}
	if (result == IDRETRY)
	{
		BvDebugBreak();
		return;
	}
	if (result == IDIGNORE)
	{
		return;
	}
}


int DPrintF(const char* pFormat, ...)
{
	va_list argList;
	va_start(argList, pFormat);
	int charsWritten = VDPrintF(pFormat, argList);
	va_end(argList);
	return charsWritten;
}


int VDPrintF(const char* format, va_list argList)
{
	static char buffer[kDebugMsgLen];
	int charsWritten = vsnprintf(buffer, kDebugMsgLen, format, argList);
	OutputDebugString(buffer);
	return charsWritten;
}