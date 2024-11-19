#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BDeV/Core/Container/BvText.h"
#include "BDeV/Core/System/BvPlatformHeaders.h"
#include <cstdio>
#include <intrin.h>


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

		SetConsoleOutputCP(CP_UTF8);
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
	va_list args1;
	va_list args2;
	va_start(args1, pFormat);
	va_copy(args2, args1);
	auto charsWritten = vsnprintf(nullptr, 0, pFormat, args1);
	va_end(args1);

	char* pErrorMessage = nullptr;
	wchar_t* pErrorMessageW = nullptr;
	size_t utf8Size = charsWritten + 1;
	size_t utf16Size = utf8Size * 2;
	void* pMem = BV_STACK_ALLOC(utf8Size + utf16Size);
	pErrorMessage = (char*)pMem + utf16Size;
	pErrorMessageW = (wchar_t*)pMem;

	vsnprintf(pErrorMessage, utf8Size, pFormat, args2);
	va_end(args2);

	BvTextUtilities::ConvertUTF8CharToWideChar(pErrorMessage, utf8Size, pErrorMessageW, utf16Size);
	OutputDebugStringW(pErrorMessageW);

	return charsWritten;
#else
	return 0;
#endif
}


void BvDebug::Assert(const char* pCondition, const BvSourceInfo& sourceInfo, const char* pFormat, ...)
{
#if BV_DEBUG
	va_list args1;
	va_list args2;
	va_start(args1, pFormat);
	va_copy(args2, args1);
	auto charsWritten = vsnprintf(nullptr, 0, pFormat, args1);
	va_end(args1);

	auto extraCharsWritten = snprintf(nullptr, 0, "\nSource: %s in %s [%d]", sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);

	char* pErrorMessage = nullptr;
	wchar_t* pErrorMessageW = nullptr;
	size_t utf8Size = charsWritten + extraCharsWritten + 1;
	size_t utf16Size = utf8Size * 2;
	void* pMem = BV_STACK_ALLOC(utf8Size + utf16Size);
	pErrorMessage = (char*)pMem + utf16Size;
	pErrorMessageW = (wchar_t*)pMem;

	vsnprintf(pErrorMessage, charsWritten + 1, pFormat, args2);
	va_end(args2);

	snprintf(pErrorMessage + charsWritten, extraCharsWritten + 1, "\nSource: %s in %s [%d]", sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);
	
	BvTextUtilities::ConvertUTF8CharToWideChar(pErrorMessage, utf8Size, pErrorMessageW, utf16Size);
	OutputDebugStringW(pErrorMessageW);

	auto result = MessageBoxW(nullptr, pErrorMessageW, L"BDeV Assertion Error", MB_ABORTRETRYIGNORE | MB_ICONERROR);
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
	va_list args1;
	va_list args2;
	va_start(args1, pFormat);
	va_copy(args2, args1);
	auto charsWritten = vsnprintf(nullptr, 0, pFormat, args1);
	va_end(args1);

	auto extraCharsWritten = snprintf(nullptr, 0, "\nSource: %s in %s [%d]", sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);

	char* pErrorMessage = nullptr;
	wchar_t* pErrorMessageW = nullptr;
	size_t utf8Size = charsWritten + extraCharsWritten + 1;
	size_t utf16Size = utf8Size * 2;
	void* pMem = BV_STACK_ALLOC(utf8Size + utf16Size);
	pErrorMessage = (char*)pMem + utf16Size;
	pErrorMessageW = (wchar_t*)pMem;

	vsnprintf(pErrorMessage, charsWritten + 1, pFormat, args2);
	va_end(args2);

	snprintf(pErrorMessage + charsWritten, extraCharsWritten + 1, "\nSource: %s in %s [%d]", sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);

	BvTextUtilities::ConvertUTF8CharToWideChar(pErrorMessage, utf8Size, pErrorMessageW, utf16Size);
	OutputDebugStringW(pErrorMessageW);

	MessageBoxW(nullptr, pErrorMessageW, L"BDeV Error", MB_OK | MB_ICONERROR);

	exit(kI32Max);
}


void BvError::RaiseOSError(const BvSourceInfo& sourceInfo)
{
	auto errorCode = GetLastError();
	wchar_t* pOSErrorMessageW = nullptr;
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr,
		errorCode, 0, (wchar_t*)&pOSErrorMessageW, 1024, nullptr);

	wchar_t functionName[128]{};
	BvTextUtilities::ConvertUTF8CharToWideChar(sourceInfo.m_pFunction, 0, functionName, 127);
	wchar_t fileName[256]{};
	BvTextUtilities::ConvertUTF8CharToWideChar(sourceInfo.m_pFile, 0, fileName, 255);

	auto charsWritten = _snwprintf(nullptr, 0, L"System Error: %d\nReason: %s\nSource: %s in %s [%d]",
		errorCode, pOSErrorMessageW, functionName, fileName, sourceInfo.m_Line);

	wchar_t* pErrorMessageW = nullptr;
	size_t utf16Size = charsWritten + 1;
	void* pMem = BV_STACK_ALLOC(utf16Size);
	pErrorMessageW = (wchar_t*)pMem;

	_snwprintf(pErrorMessageW, utf16Size, L"System Error: %d\nReason: %s\nSource: %s in %s [%d]",
		errorCode, pOSErrorMessageW, functionName, fileName, sourceInfo.m_Line);
	
	LocalFree(pOSErrorMessageW);

	MessageBoxW(nullptr, pErrorMessageW, L"BDeV Error", MB_OK | MB_ICONERROR);

	exit(kI32Max);
}