#include "BDeV/System/Debug/BvDebug.h"
#include <intrin.h>
#include <Windows.h>
#include <cstdio>


constexpr ConsoleColor kDefaultTextColor = ConsoleColor::kWhite;
constexpr ConsoleColor kDefaultBackgroundColor = ConsoleColor::kBlack;
ConsoleColor g_CurrentTextColor = kDefaultTextColor;
ConsoleColor g_CurrentBackgroundColor = kDefaultBackgroundColor;


// Forward declarations
i32 VDPrintF(const char* format, va_list argList);
void SetColor(ConsoleColor textColor, ConsoleColor backgroundColor);


i32 PrintF(const char* pFormat, ...)
{
	SetColor(kDefaultTextColor, kDefaultBackgroundColor);
	va_list argList;
	va_start(argList, pFormat);
	auto charsWritten = vprintf(pFormat, argList);
	va_end(argList);

	return charsWritten;
}


i32 PrintF(ConsoleColor textColor, const char* pFormat, ...)
{
	SetColor(textColor, kDefaultBackgroundColor);
	va_list argList;
	va_start(argList, pFormat);
	auto charsWritten = vprintf(pFormat, argList);
	va_end(argList);

	return charsWritten;
}


i32 PrintF(ConsoleColor textColor, ConsoleColor backGroundColor, const char* pFormat, ...)
{
	SetColor(textColor, backGroundColor);
	va_list argList;
	va_start(argList, pFormat);
	auto charsWritten = vprintf(pFormat, argList);
	va_end(argList);

	return charsWritten;
}


i32 DPrintF(const char* pFormat, ...)
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
		DbgBreak();
		return;
	}
	if (result == IDIGNORE)
	{
		return;
	}
}


thread_local char g_OSErrorMessage[BvDebugConstants::kMaxDebugMsgLen];


u32 GetOSErrorCode()
{
	auto errorCode = GetLastError();
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, errorCode, 0, g_OSErrorMessage, BvDebugConstants::kMaxDebugMsgLen, nullptr);

	return errorCode;
}


const char* GetOSErrorMessage()
{
	return g_OSErrorMessage;
}


void DbgBreak()
{
#if BV_DEBUG
	__debugbreak();
#endif
}


i32 VDPrintF(const char* format, va_list argList)
{
	int charsWritten = vsnprintf(BvDebugConstants::s_ErrorMessage, BvDebugConstants::kMaxDebugMsgLen, format, argList);
	OutputDebugStringA(BvDebugConstants::s_ErrorMessage);
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