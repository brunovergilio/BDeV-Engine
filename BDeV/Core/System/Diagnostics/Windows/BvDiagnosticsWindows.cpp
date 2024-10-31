#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include <intrin.h>
#include "BDeV/Core/System/Windows/BvWindowsHeader.h"
#include <cstdio>


class BvConsoleHelper
{
public:
	static BvConsoleHelper* GetInstance()
	{
		static BvConsoleHelper console;
		return &console;
	}

private:
	BvConsoleHelper()
	{
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		if (!hOut || hOut == INVALID_HANDLE_VALUE)
		{
			return;
		}

		DWORD dwMode = 0;
		if (!GetConsoleMode(hOut, &dwMode))
		{
			return;
		}

		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT;
		if (!SetConsoleMode(hOut, dwMode))
		{
			return;
		}
	}

	~BvConsoleHelper() {}
};


i32 BvConsole::PrintF(const char* pFormat, ...)
{
	BvConsoleHelper::GetInstance();

	va_list argList;
	va_start(argList, pFormat);
	auto charsWritten = vprintf(pFormat, argList);
	va_end(argList);

	return charsWritten;
}


i32 BvConsole::PrintF(const BvColorI& textColor, const char* pFormat, ...)
{
	BvConsoleHelper::GetInstance();

	printf("\033[38;2;%d;%d;%dm", textColor.m_Red, textColor.m_Green, textColor.m_Blue);
	va_list argList;
	va_start(argList, pFormat);
	auto charsWritten = vprintf(pFormat, argList);
	va_end(argList);
	printf("\033[0m");

	return charsWritten;
}


i32 BvConsole::PrintF(const BvColorI& textColor, const BvColorI& backGroundColor, const char* pFormat, ...)
{
	BvConsoleHelper::GetInstance();

	printf("\033[38;2;%d;%d;%dm", textColor.m_Red, textColor.m_Green, textColor.m_Blue);
	printf("\033[48;2;%d;%d;%dm", backGroundColor.m_Red, backGroundColor.m_Green, backGroundColor.m_Blue);
	va_list argList;
	va_start(argList, pFormat);
	auto charsWritten = vprintf(pFormat, argList);
	va_end(argList);
	printf("\033[0m");

	return charsWritten;
}


i32 BvDebug::PrintF(const char* pFormat, ...)
{
#if BV_DEBUG
	va_list argList;
	va_start(argList, pFormat);
	auto charsWritten = vsnprintf(nullptr, 0, pFormat, argList);
	va_end(argList);

	{
		char* pErrorMessage = (char*)BV_STACK_ALLOC(charsWritten + 1);
		va_start(argList, pFormat);
		vsnprintf(pErrorMessage, charsWritten + 1, pFormat, argList);
		va_end(argList);
		OutputDebugStringA(pErrorMessage);
	}

	return charsWritten;
#else
	return 0;
#endif
}


void BvDebug::Assert(const char* pCondition, const BvSourceInfo& sourceInfo, const char* pFormat, ...)
{
#if BV_DEBUG
	va_list argList;
	va_start(argList, pFormat);
	auto charsWritten = vsnprintf(nullptr, 0, pFormat, argList);
	va_end(argList);

	auto extraCharsWritten = snprintf(nullptr, 0, "\nSource: %s in %s [%d]", sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);

	i32 result = 0;
	{
		char* pErrorMessage = (char*)BV_STACK_ALLOC(charsWritten + extraCharsWritten + 1);

		va_start(argList, pFormat);
		vsnprintf(pErrorMessage, charsWritten + 1, pFormat, argList);
		va_end(argList);

		snprintf(pErrorMessage + charsWritten, extraCharsWritten + 1, "\nSource: %s in %s [%d]", sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);
		result = MessageBoxA(nullptr, pErrorMessage, "BDeV Assertion Error", MB_ABORTRETRYIGNORE | MB_ICONERROR);
	}

	if (result == IDABORT)
	{
		exit(kI32Max);
	}
	else if (result == IDRETRY)
	{
		__debugbreak();
		return;
	}
	else if (result == IDIGNORE)
	{
		return;
	}
#endif
}


void BvError::RaiseError(const BvSourceInfo& sourceInfo, const char* pFormat, ...)
{
	va_list argList;
	va_start(argList, pFormat);
	auto charsWritten = vsnprintf(nullptr, 0, pFormat, argList);
	va_end(argList);

	auto extraCharsWritten = snprintf(nullptr, 0, "\nSource: %s in %s [%d]", sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);

	{
		char* pErrorMessage = (char*)BV_STACK_ALLOC(charsWritten + extraCharsWritten + 1);

		va_start(argList, pFormat);
		vsnprintf(pErrorMessage, charsWritten + 1, pFormat, argList);
		va_end(argList);

		snprintf(pErrorMessage + charsWritten, extraCharsWritten + 1, "\nSource: %s in %s [%d]", sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);
		MessageBoxA(nullptr, pErrorMessage, "BDeV Error", MB_OK | MB_ICONERROR);
	}

	exit(kI32Max);
}


void BvError::RaiseOSError(const BvSourceInfo& sourceInfo)
{
	constexpr auto kMaxMsgSize = 2048;
	
	auto errorCode = GetLastError();
	char errorMessage[kMaxMsgSize];
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, errorCode, 0, errorMessage, kMaxMsgSize, nullptr);

	RaiseError(sourceInfo, "System Error: %d\nReason: %s", errorCode, errorMessage);
}