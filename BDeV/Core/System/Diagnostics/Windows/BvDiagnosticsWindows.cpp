#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BDeV/Core/Container/BvText.h"
#include "BDeV/Core/System/Memory/BvMemory.h"
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


void BvConsole::ConsoleHelper()
{
	static BvConsoleHelper console;
}


void BvConsole::Print(const char* pFormat, ...)
{
	va_list argList;
	va_start(argList, pFormat);
	auto charsWritten = vprintf(pFormat, argList);
	va_end(argList);
}


void BvConsole::Print(const BvColorI& textColor, const char* pFormat, ...)
{
	printf("\033[38;2;%d;%d;%dm", textColor.m_Red, textColor.m_Green, textColor.m_Blue);
	va_list argList;
	va_start(argList, pFormat);
	auto charsWritten = vprintf(pFormat, argList);
	va_end(argList);
	printf("\033[0m");
}


void BvConsole::Print(const BvColorI& textColor, const BvColorI& backGroundColor, const char* pFormat, ...)
{
	printf("\033[38;2;%d;%d;%dm", textColor.m_Red, textColor.m_Green, textColor.m_Blue);
	printf("\033[48;2;%d;%d;%dm", backGroundColor.m_Red, backGroundColor.m_Green, backGroundColor.m_Blue);
	va_list argList;
	va_start(argList, pFormat);
	auto charsWritten = vprintf(pFormat, argList);
	va_end(argList);
	printf("\033[0m");
}


class BvBufferHelper
{
public:
	BvBufferHelper()
	{
	}

	~BvBufferHelper()
	{
		if (m_pBuffer)
		{
			BV_FREE(m_pBuffer);
		}
	}

	void* GetBuffer(size_t sizeNeeded, size_t alignment = kDefaultAlignmentSize)
	{
		if (sizeNeeded > m_BufferSize)
		{
			if (m_pBuffer)
			{
				BV_FREE(m_pBuffer);
			}
			m_pBuffer = BV_ALLOC(sizeNeeded, alignment);
			m_BufferSize = sizeNeeded;
		}

		return m_pBuffer;
	}

	BV_INLINE void* GetBuffer()
	{
		return m_pBuffer;
	}

private:
	void* m_pBuffer = nullptr;
	size_t m_BufferSize = 0;
};


BV_INLINE BvBufferHelper& GetMessageBuffer()
{
	static thread_local BvBufferHelper bufferHelper;
	return bufferHelper;
}


void BvDebug::Print(const char* pFormat, ...)
{
#if BV_DEBUG
	va_list args1;
	va_list args2;
	va_start(args1, pFormat);
	va_copy(args2, args1);
	auto charsWritten = vsnprintf(nullptr, 0, pFormat, args1);
	va_end(args1);

	size_t utf8Size = RoundToNearestPowerOf2(charsWritten + 1, 2);
	size_t utf16Size = utf8Size * 2;
	void* pMem = GetMessageBuffer().GetBuffer(utf8Size + utf16Size);
	char* pErrorMessage = (char*)pMem + utf16Size;

	vsnprintf(pErrorMessage, utf8Size, pFormat, args2);
	va_end(args2);

	wchar_t* pErrorMessageW = (wchar_t*)pMem;
	BvTextUtilities::ConvertUTF8CharToWideChar(pErrorMessage, utf8Size, pErrorMessageW, utf16Size);
	OutputDebugStringW(pErrorMessageW);
#endif
}


void BvDebug::Break()
{
	__debugbreak();
}


namespace Internal
{
	BvAssert::BvAssert(const char* pCondition, const BvSourceInfo& sourceInfo, const char* pFormat, ...)
	{
#if BV_DEBUG
		va_list args1;
		va_list args2;
		va_start(args1, pFormat);
		va_copy(args2, args1);
		auto errorCharsNeeded = vsnprintf(nullptr, 0, pFormat, args1);
		auto sourceInfoCharsNeeded = snprintf(nullptr, 0, "\nSource: %s in %s [%d]", sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);
		va_end(args1);

		size_t utf8Size = errorCharsNeeded + sourceInfoCharsNeeded + 1;
		size_t utf16Size = utf8Size * 2;
		void* pMem = GetMessageBuffer().GetBuffer(utf8Size + utf16Size);
		char* pErrorMessage = (char*)pMem + utf16Size;

		vsnprintf(pErrorMessage, errorCharsNeeded + 1, pFormat, args2);
		va_end(args2);

		snprintf(pErrorMessage + errorCharsNeeded, sourceInfoCharsNeeded + 1, "\nSource: %s in %s [%d]", sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);

		wchar_t* pErrorMessageW = (wchar_t*)pMem;
		BvTextUtilities::ConvertUTF8CharToWideChar(pErrorMessage, utf8Size, pErrorMessageW, utf16Size);

		auto result = MessageBoxW(nullptr, pErrorMessageW, L"BDeV Assertion Error", MB_ABORTRETRYIGNORE | MB_ICONERROR);
		if (result == IDABORT)
		{
			exit(kI32Max);
		}
		else if (result == IDRETRY)
		{
			BvDebug::Break();
			return;
		}
		else if (result == IDIGNORE)
		{
			return;
		}
#endif
	}

	BvFatalError::BvFatalError(const BvSourceInfo& sourceInfo, const char* pFormat, ...)
	{
		va_list args1;
		va_list args2;
		va_start(args1, pFormat);
		va_copy(args2, args1);
		auto errorCharsNeeded = vsnprintf(nullptr, 0, pFormat, args1);
		auto sourceInfoCharsNeeded = snprintf(nullptr, 0, "\nSource: %s in %s [%d]", sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);
		va_end(args1);

		size_t utf8Size = errorCharsNeeded + sourceInfoCharsNeeded + 1;
		size_t utf16Size = utf8Size * 2;
		void* pMem = GetMessageBuffer().GetBuffer(utf8Size + utf16Size);
		char* pErrorMessage = (char*)pMem + utf16Size;
		wchar_t* pErrorMessageW = (wchar_t*)pMem;

		vsnprintf(pErrorMessage, errorCharsNeeded + 1, pFormat, args2);
		va_end(args2);

		snprintf(pErrorMessage + errorCharsNeeded, sourceInfoCharsNeeded + 1, "\nSource: %s in %s [%d]", sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);

		BvTextUtilities::ConvertUTF8CharToWideChar(pErrorMessage, utf8Size, pErrorMessageW, utf16Size);

		MessageBoxW(nullptr, pErrorMessageW, L"BDeV Error", MB_OK | MB_ICONERROR);

		exit(kI32Max);
	}
}


struct BvErrorData
{
	i32 m_ErrorCode;
	BvBufferHelper m_ErrorMessage;
};


BV_INLINE BvErrorData& GetErrorData()
{
	static thread_local BvErrorData errorData;
	return errorData;
}


BvError::BvError()
{
}


void BvError::FromSTD()
{
	auto& errorData = GetErrorData();
	errorData.m_ErrorCode = errno;
	if (auto pError = strerror(errorData.m_ErrorCode))
	{
		auto sizeNeeded = strlen(pError) + 1;
		char* pErrorMessage = (char*)errorData.m_ErrorMessage.GetBuffer(sizeNeeded);
		strcpy(pErrorMessage, pError);
	}
}


void BvError::FromSystem()
{
	auto& errorData = GetErrorData();
	errorData.m_ErrorCode = i32(GetLastError());

	wchar_t* pErrorMessageW = nullptr;
	// We need UTF-8 encoding, but Windows only offers UTF-16, so we convert it back to UTF-8
	if (u32 bytesWritten = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr,
		errorData.m_ErrorCode, 0, (LPWSTR)&pErrorMessageW, 0, nullptr))
	{
		auto sizeNeeded = BvTextUtilities::ConvertWideCharToUTF8Char(pErrorMessageW, bytesWritten + 1, nullptr, 0);
		char* pErrorMessage = (char*)errorData.m_ErrorMessage.GetBuffer(sizeNeeded);
		BvTextUtilities::ConvertWideCharToUTF8Char(pErrorMessageW, bytesWritten + 1, pErrorMessage, sizeNeeded);
		LocalFree(pErrorMessageW);
	}
}


void BvError::MakeCustom(u32 errorCode, const char* pFormat, ...)
{
	auto& errorData = GetErrorData();
	errorData.m_ErrorCode = errorCode;

	va_list args1;
	va_list args2;
	va_start(args1, pFormat);
	va_copy(args2, args1);
	auto charsWritten = vsnprintf(nullptr, 0, pFormat, args1);
	va_end(args1);

	size_t sizeNeeded = charsWritten + 1;
	char* pErrorMessage = (char*)errorData.m_ErrorMessage.GetBuffer(sizeNeeded);
	vsnprintf(pErrorMessage, sizeNeeded, pFormat, args2);
	va_end(args2);
}


i32 BvError::GetErrorCode() const
{
	return GetErrorData().m_ErrorCode;
}


const char* BvError::GetErrorMessage() const
{
	return (const char*)GetErrorData().m_ErrorMessage.GetBuffer();
}