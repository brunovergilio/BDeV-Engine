#include "BDeV/Core/System/BvPlatformHeaders.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BDeV/Core/Utils/BvText.h"
#include "BDeV/Core/System/Memory/BvMemory.h"
#include <cstdio>
#include <intrin.h>
#include <utility>
#include <comdef.h>


constexpr u32 kMaxMessageSize = 2_kb;


class BvConsoleHelper
{
public:
	static BvConsoleHelper* GetInstance()
	{
		static BvConsoleHelper console;
		return &console;
	}

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

		if (!SetConsoleOutputCP(CP_UTF8))
		{
			return;
		}
		// For some reason, SetConsoleOutputCP is returning error 187 on my machine,
		// even though the function returns TRUE. I'm resetting it to 0.
		SetLastError(0);
	}

	~BvConsoleHelper() {}
};


void ConsoleHelper()
{
	static BvConsoleHelper console;
}


void BvConsole::Print(const char* pFormat, ...)
{
	ConsoleHelper();
	va_list argList;
	va_start(argList, pFormat);
	vprintf(pFormat, argList);
	va_end(argList);
}


void BvConsole::Print(const BvColorI& textColor, const char* pFormat, ...)
{
	ConsoleHelper();
	printf("\033[38;2;%d;%d;%dm", textColor.m_Red, textColor.m_Green, textColor.m_Blue);
	va_list argList;
	va_start(argList, pFormat);
	vprintf(pFormat, argList);
	va_end(argList);
	printf("\033[0m");
}


void BvConsole::Print(const BvColorI& textColor, const BvColorI& backGroundColor, const char* pFormat, ...)
{
	ConsoleHelper();
	printf("\033[38;2;%d;%d;%dm", textColor.m_Red, textColor.m_Green, textColor.m_Blue);
	printf("\033[48;2;%d;%d;%dm", backGroundColor.m_Red, backGroundColor.m_Green, backGroundColor.m_Blue);
	va_list argList;
	va_start(argList, pFormat);
	vprintf(pFormat, argList);
	va_end(argList);
	printf("\033[0m");
}


char* GetMessageBuffer()
{
	static thread_local char errorMessage[kMaxMessageSize]{};
	return errorMessage;
}


void BvDebug::Print(const char* pFormat, ...)
{
	auto pErrorMessage = GetMessageBuffer();
	va_list args;
	va_start(args, pFormat);
	u32 charsWritten = std::min(u32(vsnprintf(pErrorMessage, kMaxMessageSize, pFormat, args)) + 1, kMaxMessageSize);
	va_end(args);

	wchar_t errorMessageW[kMaxMessageSize];
	BvTextUtilities::ConvertUTF8CharToWideChar(pErrorMessage, charsWritten, errorMessageW, kMaxMessageSize);
	OutputDebugStringW(errorMessageW);
}


void BvDebug::Break()
{
#if BV_DEBUG
	__debugbreak();
#endif
}


void BvDebug::Assert(const char* pCondition, const BvSourceInfo& sourceInfo, const char* pFormat, ...)
{
#if BV_DEBUG
	auto pErrorMessage = GetMessageBuffer();
	va_list args;
	va_start(args, pFormat);
	auto charsWritten = std::min(u32(vsnprintf(pErrorMessage, kMaxMessageSize, pFormat, args)), kMaxMessageSize - 1);
	va_end(args);

	if (u32 remaining = kMaxMessageSize - (1 + charsWritten))
	{
		charsWritten += std::min(u32(snprintf(pErrorMessage + charsWritten, remaining,
			"\nSource: %s in %s [%d]", sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line)) + 1, remaining);
	}

	wchar_t errorMessageW[kMaxMessageSize];
	BvTextUtilities::ConvertUTF8CharToWideChar(pErrorMessage, charsWritten, errorMessageW, kMaxMessageSize);

	auto result = MessageBoxW(nullptr, errorMessageW, L"BDeV Assertion Error", MB_ABORTRETRYIGNORE | MB_ICONERROR);
	if (result == IDABORT)
	{
		exit(kI32Max);
	}
	else if (result == IDRETRY)
	{
		Break();
		return;
	}
	else if (result == IDIGNORE)
	{
		return;
	}
#endif
}


namespace Internal
{
	void ReportFatalError(const char* pMessage)
	{
		wchar_t errorMessageW[kMaxMessageSize]{};
		BvTextUtilities::ConvertUTF8CharToWideChar(pMessage, 0, errorMessageW, kMaxMessageSize);

		MessageBoxW(nullptr, errorMessageW, L"Fatal error", MB_OK);
		exit(kI32Max);
	}

	const char* GetWindowsErrorMessage(u32 errorCode)
	{
		if (errorCode == kU32Max)
		{
			errorCode = GetLastError();
		}
		char* pErrorMessage = GetMessageBuffer();
		wchar_t* pErrorMessageW = nullptr;
		u32 charsWritten = snprintf(pErrorMessage, kMaxMessageSize, "Win32 Error [%u]", errorCode);

		// We need UTF-8 encoding, but Windows only offers UTF-16, so we convert it back to UTF-8
		if (u32 errorMessageLength = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr,
			errorCode, 0, (LPWSTR)&pErrorMessageW, 0, nullptr))
		{
			charsWritten += snprintf(pErrorMessage + charsWritten, kMaxMessageSize - charsWritten, " - ");
			BvTextUtilities::ConvertWideCharToUTF8Char(pErrorMessageW, errorMessageLength + 1,
				pErrorMessage + charsWritten, kMaxMessageSize - charsWritten);
			LocalFree(pErrorMessageW);
		}

		return pErrorMessage;
	}
}