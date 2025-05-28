#pragma once


#include "BDeV/Core/BvCore.h"


// Notes to self:
// * For conversions to and from wchar_t on the Windows platform, I'm already assuming UTF-16 is being used
// * On Linux and MacOS, sizeof(wchar_t) == 4, so ensure UTF-32 is being used for wchar_t on those platforms
// * Avoid ANSI strings at all times
class BvTextUtilities
{
public:
	static bool IsValidASCIIString(const char* pStr);
	
	static bool IsValidUTF8String(const u8* pStr);
	static bool IsValidUTF8String(const char* pStr);
	static bool IsValidUTF8String(const char8_t* pStr);
	
	static bool IsValidUTF16String(const u16* pStr);
	static bool IsValidUTF16String(const char16_t* pStr);
	
	static bool IsValidUTF32String(const u32* pStr);
	static bool IsValidUTF32String(const char32_t* pStr);

	static size_t ConvertASCIIToWideChar(const char* pSrc, size_t srcCharCount, wchar_t* pDst, size_t maxDstSize);
	static size_t ConvertASCIIToUTF8Char(const char* pSrc, size_t srcCharCount, char* pDst, size_t maxDstSize);
	static size_t ConvertASCIIToUTF8(const char* pSrc, size_t srcCharCount, char8_t* pDst, size_t maxDstSize);
	static size_t ConvertASCIIToUTF16(const char* pSrc, size_t srcCharCount, char16_t* pDst, size_t maxDstSize);
	static size_t ConvertASCIIToUTF32(const char* pSrc, size_t srcCharCount, char32_t* pDst, size_t maxDstSize);
	
	static size_t ConvertWideCharToASCII(const wchar_t* pSrc, size_t srcCharCount, char* pDst, size_t maxDstSize);
	static size_t ConvertWideCharToUTF8Char(const wchar_t* pSrc, size_t srcCharCount, char* pDst, size_t maxDstSize);
	static size_t ConvertWideCharToUTF8(const wchar_t* pSrc, size_t srcCharCount, char8_t* pDst, size_t maxDstSize);
	static size_t ConvertWideCharToUTF16(const wchar_t* pSrc, size_t srcCharCount, char16_t* pDst, size_t maxDstSize);
	static size_t ConvertWideCharToUTF32(const wchar_t* pSrc, size_t srcCharCount, char32_t* pDst, size_t maxDstSize);

	static size_t ConvertUTF8CharToASCII(const char* pSrc, size_t srcCharCount, char* pDst, size_t maxDstSize);
	static size_t ConvertUTF8CharToWideChar(const char* pSrc, size_t srcCharCount, wchar_t* pDst, size_t maxDstSize);
	static size_t ConvertUTF8CharToUTF8(const char* pSrc, size_t srcCharCount, char8_t* pDst, size_t maxDstSize);
	static size_t ConvertUTF8CharToUTF16(const char* pSrc, size_t srcCharCount, char16_t* pDst, size_t maxDstSize);
	static size_t ConvertUTF8CharToUTF32(const char* pSrc, size_t srcCharCount, char32_t* pDst, size_t maxDstSize);

	static size_t ConvertUTF8ToASCII(const char8_t* pSrc, size_t srcCharCount, char* pDst, size_t maxDstSize);
	static size_t ConvertUTF8ToWideChar(const char8_t* pSrc, size_t srcCharCount, wchar_t* pDst, size_t maxDstSize);
	static size_t ConvertUTF8ToUTF8Char(const char8_t* pSrc, size_t srcCharCount, char* pDst, size_t maxDstSize);
	static size_t ConvertUTF8ToUTF16(const char8_t* pSrc, size_t srcCharCount, char16_t* pDst, size_t maxDstSize);
	static size_t ConvertUTF8ToUTF32(const char8_t* pSrc, size_t srcCharCount, char32_t* pDst, size_t maxDstSize);
	
	static size_t ConvertUTF16ToASCII(const char16_t* pSrc, size_t srcCharCount, char* pDst, size_t maxDstSize);
	static size_t ConvertUTF16ToWideChar(const char16_t* pSrc, size_t srcCharCount, wchar_t* pDst, size_t maxDstSize);
	static size_t ConvertUTF16ToUTF8Char(const char16_t* pSrc, size_t srcCharCount, char* pDst, size_t maxDstSize);
	static size_t ConvertUTF16ToUTF8(const char16_t* pSrc, size_t srcCharCount, char8_t* pDst, size_t maxDstSize);
	static size_t ConvertUTF16ToUTF32(const char16_t* pSrc, size_t srcCharCount, char32_t* pDst, size_t maxDstSize);
	
	static size_t ConvertUTF32ToASCII(const char32_t* pSrc, size_t srcCharCount, char* pDst, size_t maxDstSize);
	static size_t ConvertUTF32ToWideChar(const char32_t* pSrc, size_t srcCharCount, wchar_t* pDst, size_t maxDstSize);
	static size_t ConvertUTF32ToUTF8Char(const char32_t* pSrc, size_t srcCharCount, char* pDst, size_t maxDstSize);
	static size_t ConvertUTF32ToUTF8(const char32_t* pSrc, size_t srcCharCount, char8_t* pDst, size_t maxDstSize);
	static size_t ConvertUTF32ToUTF16(const char32_t* pSrc, size_t srcCharCount, char16_t* pDst, size_t maxDstSize);
};