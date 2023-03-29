#include "BDeV/System/Debug/BvDebug.h"
#include <intrin.h>
#include <Windows.h>
#include <cstdio>


constexpr ConsoleColor kDefaultTextColor = ConsoleColor::kWhite;
constexpr ConsoleColor kDefaultBackgroundColor = ConsoleColor::kBlack;
ConsoleColor g_CurrentTextColor = kDefaultTextColor;
ConsoleColor g_CurrentBackgroundColor = kDefaultBackgroundColor;


// Forward declarations
int VDPrintF(const char* format, va_list argList);
void SetColor(ConsoleColor textColor, ConsoleColor backgroundColor);


BV_API int PrintF(const char* pFormat, ...)
{
	SetColor(kDefaultTextColor, kDefaultBackgroundColor);
	va_list argList;
	va_start(argList, pFormat);
	auto charsWritten = vprintf(pFormat, argList);
	va_end(argList);

	return charsWritten;
}


int PrintF(ConsoleColor textColor, const char* pFormat, ...)
{
	SetColor(textColor, kDefaultBackgroundColor);
	va_list argList;
	va_start(argList, pFormat);
	auto charsWritten = vprintf(pFormat, argList);
	va_end(argList);

	return charsWritten;
}


int PrintF(ConsoleColor textColor, ConsoleColor backGroundColor, const char* pFormat, ...)
{
	SetColor(textColor, backGroundColor);
	va_list argList;
	va_start(argList, pFormat);
	auto charsWritten = vprintf(pFormat, argList);
	va_end(argList);

	return charsWritten;
}


int DPrintF(const char* pFormat, ...)
{
	va_list argList;
	va_start(argList, pFormat);
	auto charsWritten = VDPrintF(pFormat, argList);
	va_end(argList);

	return charsWritten;
}


void RaiseError(const char* pMessage, const char* pTitle)
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


u32 GetOSError(char* pErrorMessage, u32 bufferSize)
{
	auto errorCode = GetLastError();
	if (pErrorMessage)
	{
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, errorCode, 0, pErrorMessage, bufferSize, nullptr);
	}

	return errorCode;
}


void MakeDebugBreak()
{
	__debugbreak();
}


int VDPrintF(const char* format, va_list argList)
{
	static char buffer[kDebugMsgLen];
	int charsWritten = vsnprintf(buffer, kDebugMsgLen, format, argList);
	OutputDebugStringA(buffer);
	return charsWritten;
}


void SetColor(ConsoleColor textColor, ConsoleColor backgroundColor)
{
	if (textColor == g_CurrentTextColor && backgroundColor == g_CurrentBackgroundColor)
	{
		return;
	}

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), textColor | (backgroundColor << 16));

	g_CurrentTextColor = textColor;
	g_CurrentBackgroundColor = backgroundColor;
}