#include "BvText.h"
#include <string>


template<typename CharT>
size_t ConvertASCIIToCharT(const char* pSrc, size_t srcCharCount, CharT* pDst, size_t maxDstSize)
{
	if (!pSrc)
	{
		return 0;
	}

	if (!srcCharCount)
	{
		srcCharCount = std::char_traits<char>::length(pSrc) + 1;
	}

	if (!pDst || !maxDstSize)
	{
		return srcCharCount;
	}

	size_t maxChars = std::min(srcCharCount, maxDstSize);
	for (size_t i = 0; i < maxChars; ++i)
	{
		pDst[i] = CharT(pSrc[i]);
	}

	return maxChars;
};


template<typename CharT>
size_t ConvertCharTToASCII(const CharT* pSrc, size_t srcCharCount, char* pDst, size_t maxDstSize)
{
	if (!pSrc)
	{
		return 0;
	}

	if (!srcCharCount)
	{
		srcCharCount = std::char_traits<CharT>::length(pSrc) + 1;
	}

	if (!pDst || !maxDstSize)
	{
		return srcCharCount;
	}

	size_t maxChars = std::min(srcCharCount, maxDstSize);
	for (size_t i = 0; i < maxChars; ++i)
	{
		if (pSrc[i] > 127)
		{
			return i;
		}
		pDst[i] = char(pSrc[i]);
	}

	return maxChars;
};


template<typename UTF8CharT, typename CharT, typename = typename std::enable_if_t<!std::is_same_v<UTF8CharT, CharT>>>
size_t ConvertUTF8ToUTFT(const UTF8CharT* pSrc, size_t srcCharCount, CharT* pDst, size_t maxDstSize)
{
	static_assert(sizeof(CharT) == 1 || sizeof(CharT) == 2 || sizeof(CharT) == 4, "Invalid UTF char size");

	if (!pSrc)
	{
		return 0;
	}

	if (!srcCharCount)
	{
		srcCharCount = std::char_traits<UTF8CharT>::length(pSrc) + 1;
	}

	size_t byteCount = 0;
	for (size_t i = 0; i < srcCharCount;)
	{
		u8 ch = u8(pSrc[i]);
		u32 codePoint = 0;

		// Determine the number of bytes in the UTF-8 sequence
		if ((ch & 0x80) == 0)
		{
			if constexpr (sizeof(CharT) == 1)
			{
				if (pDst && maxDstSize)
				{
					if (byteCount >= maxDstSize)
					{
						break;
					}
					pDst[byteCount] = CharT(pSrc[i]);
				}
				byteCount += 1;
				++i;
				continue;
			}
			else
			{
				// 1-byte sequence (ASCII)
				codePoint = u32(pSrc[i++]);
			}
		}
		else if ((ch & 0xE0) == 0xC0)
		{
			if constexpr (sizeof(CharT) == 1)
			{
				if (pDst && maxDstSize)
				{
					if (byteCount + 1 >= maxDstSize)
					{
						break;
					}
					pDst[byteCount] = CharT(pSrc[i]);
					pDst[byteCount + 1] = CharT(pSrc[i + 1]);
				}
				byteCount += 2;
				++i;
				continue;
			}
			else
			{
				// 2-byte sequence
				codePoint = u32(pSrc[i++]) & 0x1F;
				codePoint = (codePoint << 6) | (u32(pSrc[i++]) & 0x3F);
			}
		}
		else if ((ch & 0xF0) == 0xE0)
		{
			if constexpr (sizeof(CharT) == 1)
			{
				if (pDst && maxDstSize)
				{
					if (byteCount + 2 >= maxDstSize)
					{
						break;
					}
					pDst[byteCount] = CharT(pSrc[i]);
					pDst[byteCount + 1] = CharT(pSrc[i + 1]);
					pDst[byteCount + 2] = CharT(pSrc[i + 2]);
				}
				byteCount += 3;
				++i;
				continue;
			}
			else
			{
				// 3-byte sequence
				codePoint = u32(pSrc[i++]) & 0x0F;
				codePoint = (codePoint << 6) | (u32(pSrc[i++]) & 0x3F);
				codePoint = (codePoint << 6) | (u32(pSrc[i++]) & 0x3F);
			}
		}
		else if ((ch & 0xF8) == 0xF0)
		{
			if constexpr (sizeof(CharT) == 1)
			{
				if (pDst && maxDstSize)
				{
					if (byteCount + 3 >= maxDstSize)
					{
						break;
					}
					pDst[byteCount] = CharT(pSrc[i]);
					pDst[byteCount + 1] = CharT(pSrc[i + 1]);
					pDst[byteCount + 2] = CharT(pSrc[i + 2]);
					pDst[byteCount + 3] = CharT(pSrc[i + 3]);
				}
				byteCount += 4;
				++i;
				continue;
			}
			else
			{
				// 4-byte sequence
				codePoint = u32(pSrc[i++]) & 0x07;
				codePoint = (codePoint << 6) | (u32(pSrc[i++]) & 0x3F);
				codePoint = (codePoint << 6) | (u32(pSrc[i++]) & 0x3F);
				codePoint = (codePoint << 6) | (u32(pSrc[i++]) & 0x3F);
			}
		}
		else
		{
			// Invalid UTF-8 sequence
			return byteCount;
		}

		if constexpr (sizeof(CharT) == 2)
		{
			if (codePoint <= 0xFFFF)
			{
				if (pDst && maxDstSize)
				{
					if (byteCount >= maxDstSize)
					{
						break;
					}
					pDst[byteCount] = CharT(codePoint);
				}
				++byteCount;
			}
			else
			{
				// Convert to surrogate pair
				codePoint -= 0x10000;
				if (pDst && maxDstSize)
				{
					if (byteCount + 1 >= maxDstSize)
					{
						break;
					}
					pDst[byteCount] = CharT(0xD800 | ((codePoint >> 10) & 0x3FF));
					pDst[byteCount + 1] = CharT(0xDC00 | (codePoint & 0x3FF));
				}
				byteCount += 2;

				return 0;
			}
		}
		else if constexpr (sizeof(CharT) == 4)
		{
			if (pDst && maxDstSize)
			{
				if (byteCount >= maxDstSize)
				{
					break;
				}
				pDst[byteCount] = CharT(codePoint);
			}
			++byteCount;
		}
	}

	return byteCount;
}


template<typename UTF16CharT, typename CharT, typename = typename std::enable_if_t<!std::is_same_v<UTF16CharT, CharT>>>
size_t ConvertUTF16ToUTFT(const UTF16CharT* pSrc, size_t srcCharCount, CharT* pDst, size_t maxDstSize)
{
	static_assert(sizeof(CharT) == 1 || sizeof(CharT) == 2 || sizeof(CharT) == 4, "Invalid UTF char size");

	if (!pSrc)
	{
		return 0;
	}

	if (!srcCharCount)
	{
		srcCharCount = std::char_traits<UTF16CharT>::length(pSrc) + 1;
	}

	size_t byteCount = 0;
	for (size_t i = 0; i < srcCharCount; ++i)
	{
		u16 ch = u16(pSrc[i]);
		u32 codePoint;
		if (ch >= 0xD800 && ch <= 0xDBFF)
		{
			// High surrogate
			if (i + 1 < srcCharCount && u16(pSrc[i + 1]) >= 0xDC00 && u16(pSrc[i + 1]) <= 0xDFFF)
			{
				if constexpr (sizeof(CharT) == 2)
				{
					if (pDst && maxDstSize)
					{
						if (byteCount + 1 >= maxDstSize)
						{
							break;
						}
						pDst[byteCount] = CharT(pSrc[i]);
						pDst[byteCount + 1] = CharT(pSrc[i + 1]);
					}
					byteCount += 2;
					++i;
					continue;
				}
				else
				{
					// Low surrogate
					codePoint = ((u16(pSrc[i]) - 0xD800) << 10) + (u16(pSrc[i + 1]) - 0xDC00) + 0x10000;
					++i;
				}
			}
			else
			{
				// Invalid surrogate pair
				return byteCount;
			}
		}
		else if (ch >= 0xDC00 && ch <= 0xDFFF)
		{
			// Lone low surrogate
			return byteCount;
		}
		else
		{
			if constexpr (sizeof(CharT) == 2)
			{
				if (pDst && maxDstSize)
				{
					if (byteCount >= maxDstSize)
					{
						break;
					}
					pDst[byteCount] = CharT(pSrc[i]);
				}
				++byteCount;
				continue;
			}
			else
			{
				// Basic code point
				codePoint = ch;
			}
		}

		if constexpr (sizeof(CharT) == 1)
		{
			if (codePoint <= 0x7F)
			{
				if (pDst && maxDstSize)
				{
					if (byteCount >= maxDstSize)
					{
						break;
					}
					pDst[byteCount] = CharT(codePoint);
				}
				byteCount += 1;
			}
			else if (codePoint <= 0x7FF)
			{
				if (pDst && maxDstSize)
				{
					if (byteCount + 1 >= maxDstSize)
					{
						break;
					}
					pDst[byteCount] = CharT(0xC0 | (codePoint >> 6));
					pDst[byteCount + 1] = CharT(0x80 | (codePoint & 0x3F));
				}
				byteCount += 2;
			}
			else if (codePoint <= 0xFFFF)
			{
				if (pDst && maxDstSize)
				{
					if (byteCount + 2 >= maxDstSize)
					{
						break;
					}
					pDst[byteCount] = CharT(0xE0 | (codePoint >> 12));
					pDst[byteCount + 1] = CharT(0x80 | ((codePoint >> 6) & 0x3F));
					pDst[byteCount + 2] = CharT(0x80 | (codePoint & 0x3F));
				}
				byteCount += 3;
			}
			else if (codePoint <= 0x10FFFF)
			{
				if (pDst && maxDstSize)
				{
					if (byteCount + 3 >= maxDstSize)
					{
						break;
					}
					pDst[byteCount] = CharT(0xF0 | (codePoint >> 18));
					pDst[byteCount + 1] = CharT(0x80 | ((codePoint >> 12) & 0x3F));
					pDst[byteCount + 2] = CharT(0x80 | ((codePoint >> 6) & 0x3F));
					pDst[byteCount + 3] = CharT(0x80 | (codePoint & 0x3F));
				}
				byteCount += 4;
			}
			else
			{
				// Invalid code point
				return byteCount;
			}
		}
		else if constexpr (sizeof(CharT) == 4)
		{
			if (pDst && maxDstSize)
			{
				if (byteCount >= maxDstSize)
				{
					break;
				}
				pDst[byteCount] = CharT(codePoint);
			}
			++byteCount;
		}
	}

	return byteCount;
}


template<typename UTF32CharT, typename CharT, typename = typename std::enable_if_t<!std::is_same_v<UTF32CharT, CharT>>>
size_t ConvertUTF32ToUTFT(const UTF32CharT* pSrc, size_t srcCharCount, CharT* pDst, size_t maxDstSize)
{
	static_assert(sizeof(CharT) == 1 || sizeof(CharT) == 2 || sizeof(CharT) == 4, "Invalid UTF char size");

	if (!pSrc)
	{
		return 0;
	}

	if (!srcCharCount)
	{
		srcCharCount = std::char_traits<UTF32CharT>::length(pSrc) + 1;
	}

	size_t byteCount = 0;
	for (size_t i = 0; i < srcCharCount; ++i)
	{
		u32 codePoint = u32(pSrc[i]);
		if constexpr (sizeof(CharT) == 4)
		{
			if (pDst && maxDstSize)
			{
				if (byteCount >= maxDstSize)
				{
					break;
				}
				pDst[byteCount] = CharT(pSrc[i]);
			}
			++byteCount;
		}
		else if constexpr (sizeof(CharT) == 1)
		{
			if (codePoint <= 0x7F)
			{
				if (pDst && maxDstSize)
				{
					if (byteCount >= maxDstSize)
					{
						break;
					}
					pDst[byteCount] = CharT(codePoint);
				}
				byteCount += 1;
			}
			else if (codePoint <= 0x7FF)
			{
				if (pDst && maxDstSize)
				{
					if (byteCount + 1 >= maxDstSize)
					{
						break;
					}
					pDst[byteCount] = CharT(0xC0 | (codePoint >> 6));
					pDst[byteCount + 1] = CharT(0x80 | (codePoint & 0x3F));
				}
				byteCount += 2;
			}
			else if (codePoint <= 0xFFFF)
			{
				if (pDst && maxDstSize)
				{
					if (byteCount + 2 >= maxDstSize)
					{
						break;
					}
					pDst[byteCount] = CharT(0xE0 | (codePoint >> 12));
					pDst[byteCount + 1] = CharT(0x80 | ((codePoint >> 6) & 0x3F));
					pDst[byteCount + 2] = CharT(0x80 | (codePoint & 0x3F));
				}
				byteCount += 3;
			}
			else if (codePoint <= 0x10FFFF)
			{
				if (pDst && maxDstSize)
				{
					if (byteCount + 3 >= maxDstSize)
					{
						break;
					}
					pDst[byteCount] = CharT(0xF0 | (codePoint >> 18));
					pDst[byteCount + 1] = CharT(0x80 | ((codePoint >> 12) & 0x3F));
					pDst[byteCount + 2] = CharT(0x80 | ((codePoint >> 6) & 0x3F));
					pDst[byteCount + 3] = CharT(0x80 | (codePoint & 0x3F));
				}
				byteCount += 4;
			}
			else
			{
				// Invalid code point
				return byteCount;
			}
		}
		else
		{
			if (codePoint <= 0xFFFF)
			{
				if (pDst && maxDstSize)
				{
					if (byteCount >= maxDstSize)
					{
						break;
					}
					pDst[byteCount] = CharT(codePoint);
				}
				++byteCount;
			}
			else if (codePoint <= 0x10FFFF)
			{
				if (pDst && maxDstSize)
				{
					if (byteCount + 1 >= maxDstSize)
					{
						break;
					}
					pDst[byteCount] = CharT((codePoint >> 10) + 0xD800);
					pDst[byteCount + 1] = CharT((codePoint & 0x3FF) + 0xDC00);
				}
				byteCount += 2;
			}
			else
			{
				// Invalid code point
				return byteCount;
			}
		}
	}

	return byteCount;
}


template<typename CharT>
bool IsValidASCIIStringT(const CharT* pStr)
{
	auto pWalker = pStr;
	while (*pWalker)
	{
		if (u32(*pWalker) > 127)
		{
			return false;
		}
	}

	return true;
}


template<typename CharT, typename = typename std::enable_if_t<(sizeof(CharT) == 1)>>
bool IsValidUTF8StringT(const CharT* pStr)
{
	auto bytes = 0u;
	auto pWalker = pStr;
	while (*pWalker)
	{
		auto ch = u8(*pWalker);
		if (bytes == 0)
		{
			if ((ch & 0x80) == 0x00)
			{
				// ASCII, 1 byte
				continue;
			}
			else if ((ch & 0xE0) == 0xC0)
			{
				// 110x xxxx, 2 bytes
				bytes = 1;
			}
			else if ((ch & 0xF0) == 0xE0)
			{
				// 1110 xxxx, 3 bytes
				bytes = 2;
			}
			else if ((ch & 0xF8) == 0xF0)
			{
				// 1111 0xxx, 4 bytes
				bytes = 3;
			}
			else
			{
				return false; // Invalid first byte of UTF-8 sequence
			}
		}
		else
		{
			if ((ch & 0xC0) != 0x80)
			{
				return false; // Invalid continuation byte
			}
			--bytes;
		}
	}

	return bytes == 0;
}


template<typename CharT, typename = typename std::enable_if_t<(sizeof(CharT) == 2)>>
bool IsValidUTF16StringT(const CharT* pStr)
{
	auto pWalker = pStr;
	while (*pWalker)
	{
		u16 c = u16(*pWalker);

		// Check if it's a high surrogate
		if (c >= 0xD800 && c <= 0xDBFF)
		{
			// Ensure there's a following low surrogate
			if (pWalker[1])
			{
				u16 next = u16(pWalker[1]);
				if (next >= 0xDC00 && next <= 0xDFFF)
				{
					// It's a valid surrogate pair
					// Move to the next character after the pair
					++pWalker;
				}
				else
				{
					// Invalid sequence
					return false;
				}
			}
			else
			{
				// Invalid sequence: high surrogate without following low surrogate
				return false;
			}
		}
		else if (c >= 0xDC00 && c <= 0xDFFF)
		{
			// Invalid sequence: low surrogate without preceding high surrogate
			return false;
		}

		++pWalker;
	}

	return true;
}


template<typename CharT, typename = typename std::enable_if_t<(sizeof(CharT) == 4)>>
bool IsValidUTF32StringT(const CharT* pStr)
{
	auto pWalker = pStr;
	while (*pWalker)
	{
		u32 c = u32(*pWalker);
		// Ensure each code point is within the valid Unicode range (0x0000 to 0x10FFFF)
		// and not in the surrogate pair range (0xD800 to 0xDFFF)
		if (c > 0x10FFFF || (c >= 0xD800 && c <= 0xDFFF))
		{
			return false;
		}

		++pWalker;
	}

	return true;
}


bool BvTextUtilities::IsValidASCIIString(const char* pStr)
{
	auto pWalker = reinterpret_cast<const u8*>(pStr);
	while (*pWalker)
	{
		if (u8(*pWalker) > 127)
		{
			return false;
		}
	}

	return true;
}


bool BvTextUtilities::IsValidUTF8String(const u8* pStr)
{
	return IsValidUTF8StringT(pStr);
}


bool BvTextUtilities::IsValidUTF8String(const char* pStr)
{
	return IsValidUTF8StringT(pStr);
}


bool BvTextUtilities::IsValidUTF8String(const char8_t* pStr)
{
	return IsValidUTF8StringT(pStr);
}


bool BvTextUtilities::IsValidUTF16String(const u16* pStr)
{
	return IsValidUTF16StringT(pStr);
}


bool BvTextUtilities::IsValidUTF16String(const char16_t* pStr)
{
	return IsValidUTF16StringT(pStr);
}


bool BvTextUtilities::IsValidUTF32String(const u32* pStr)
{
	return IsValidUTF32StringT(pStr);
}


bool BvTextUtilities::IsValidUTF32String(const char32_t* pStr)
{
	return IsValidUTF32StringT(pStr);
}


size_t BvTextUtilities::ConvertASCIIToWideChar(const char* pSrc, size_t srcCharCount, wchar_t* pDst, size_t maxDstSize)
{
	return ConvertASCIIToCharT(pSrc, srcCharCount, pDst, maxDstSize);
}


size_t BvTextUtilities::ConvertASCIIToUTF8Char(const char* pSrc, size_t srcCharCount, char* pDst, size_t maxDstSize)
{
	return ConvertASCIIToCharT(pSrc, srcCharCount, pDst, maxDstSize);
}


size_t BvTextUtilities::ConvertASCIIToUTF8(const char* pSrc, size_t srcCharCount, char8_t* pDst, size_t maxDstSize)
{
	return ConvertASCIIToCharT(pSrc, srcCharCount, pDst, maxDstSize);
}


size_t BvTextUtilities::ConvertASCIIToUTF16(const char* pSrc, size_t srcCharCount, char16_t* pDst, size_t maxDstSize)
{
	return ConvertASCIIToCharT(pSrc, srcCharCount, pDst, maxDstSize);
}


size_t BvTextUtilities::ConvertASCIIToUTF32(const char* pSrc, size_t srcCharCount, char32_t* pDst, size_t maxDstSize)
{
	return ConvertASCIIToCharT(pSrc, srcCharCount, pDst, maxDstSize);
}


size_t BvTextUtilities::ConvertWideCharToASCII(const wchar_t* pSrc, size_t srcCharCount, char* pDst, size_t maxDstSize)
{
	return ConvertCharTToASCII(pSrc, srcCharCount, pDst, maxDstSize);
}


size_t BvTextUtilities::ConvertWideCharToUTF8Char(const wchar_t* pSrc, size_t srcCharCount, char* pDst, size_t maxDstSize)
{
	static_assert(sizeof(wchar_t) == 2 || sizeof(wchar_t) == 4, "Invalid wchar_t size");

	if constexpr (sizeof(wchar_t) == 2)
	{
		return ConvertUTF16ToUTFT(pSrc, srcCharCount, pDst, maxDstSize);
	}
	else
	{
		return ConvertUTF32ToUTFT(pSrc, srcCharCount, pDst, maxDstSize);
	}
}


size_t BvTextUtilities::ConvertWideCharToUTF8(const wchar_t* pSrc, size_t srcCharCount, char8_t* pDst, size_t maxDstSize)
{
	static_assert(sizeof(wchar_t) == 2 || sizeof(wchar_t) == 4, "Invalid wchar_t size");

	if constexpr (sizeof(wchar_t) == 2)
	{
		return ConvertUTF16ToUTFT(pSrc, srcCharCount, pDst, maxDstSize);
	}
	else
	{
		return ConvertUTF32ToUTFT(pSrc, srcCharCount, pDst, maxDstSize);
	}
}


size_t BvTextUtilities::ConvertWideCharToUTF16(const wchar_t* pSrc, size_t srcCharCount, char16_t* pDst, size_t maxDstSize)
{
	static_assert(sizeof(wchar_t) == 2 || sizeof(wchar_t) == 4, "Invalid wchar_t size");

	if constexpr (sizeof(wchar_t) == 2)
	{
		return ConvertUTF16ToUTFT(pSrc, srcCharCount, pDst, maxDstSize);
	}
	else
	{
		return ConvertUTF32ToUTFT(pSrc, srcCharCount, pDst, maxDstSize);
	}
}


size_t BvTextUtilities::ConvertWideCharToUTF32(const wchar_t* pSrc, size_t srcCharCount, char32_t* pDst, size_t maxDstSize)
{
	static_assert(sizeof(wchar_t) == 2 || sizeof(wchar_t) == 4, "Invalid wchar_t size");

	if constexpr (sizeof(wchar_t) == 2)
	{
		return ConvertUTF16ToUTFT(pSrc, srcCharCount, pDst, maxDstSize);
	}
	else
	{
		return ConvertUTF32ToUTFT(pSrc, srcCharCount, pDst, maxDstSize);
	}
}


size_t BvTextUtilities::ConvertUTF8CharToASCII(const char* pSrc, size_t srcCharCount, char* pDst, size_t maxDstSize)
{
	return ConvertCharTToASCII(pSrc, srcCharCount, pDst, maxDstSize);
}


size_t BvTextUtilities::ConvertUTF8CharToWideChar(const char* pSrc, size_t srcCharCount, wchar_t* pDst, size_t maxDstSize)
{
	return ConvertUTF8ToUTFT(pSrc, srcCharCount, pDst, maxDstSize);
}


size_t BvTextUtilities::ConvertUTF8CharToUTF8(const char* pSrc, size_t srcCharCount, char8_t* pDst, size_t maxDstSize)
{
	return ConvertUTF8ToUTFT(pSrc, srcCharCount, pDst, maxDstSize);
}


size_t BvTextUtilities::ConvertUTF8CharToUTF16(const char* pSrc, size_t srcCharCount, char16_t* pDst, size_t maxDstSize)
{
	return ConvertUTF8ToUTFT(pSrc, srcCharCount, pDst, maxDstSize);
}


size_t BvTextUtilities::ConvertUTF8CharToUTF32(const char* pSrc, size_t srcCharCount, char32_t* pDst, size_t maxDstSize)
{
	return ConvertUTF8ToUTFT(pSrc, srcCharCount, pDst, maxDstSize);
}


size_t BvTextUtilities::ConvertUTF8ToASCII(const char8_t* pSrc, size_t srcCharCount, char* pDst, size_t maxDstSize)
{
	return ConvertCharTToASCII(pSrc, srcCharCount, pDst, maxDstSize);
}


size_t BvTextUtilities::ConvertUTF8ToWideChar(const char8_t* pSrc, size_t srcCharCount, wchar_t* pDst, size_t maxDstSize)
{
	return ConvertUTF8ToUTFT(pSrc, srcCharCount, pDst, maxDstSize);
}


size_t BvTextUtilities::ConvertUTF8ToUTF8Char(const char8_t* pSrc, size_t srcCharCount, char* pDst, size_t maxDstSize)
{
	return ConvertUTF8ToUTFT(pSrc, srcCharCount, pDst, maxDstSize);
}


size_t BvTextUtilities::ConvertUTF8ToUTF16(const char8_t* pSrc, size_t srcCharCount, char16_t* pDst, size_t maxDstSize)
{
	return ConvertUTF8ToUTFT(pSrc, srcCharCount, pDst, maxDstSize);
}


size_t BvTextUtilities::ConvertUTF8ToUTF32(const char8_t* pSrc, size_t srcCharCount, char32_t* pDst, size_t maxDstSize)
{
	return ConvertUTF8ToUTFT(pSrc, srcCharCount, pDst, maxDstSize);
}


size_t BvTextUtilities::ConvertUTF16ToASCII(const char16_t* pSrc, size_t srcCharCount, char* pDst, size_t maxDstSize)
{
	return ConvertCharTToASCII(pSrc, srcCharCount, pDst, maxDstSize);
}


size_t BvTextUtilities::ConvertUTF16ToWideChar(const char16_t* pSrc, size_t srcCharCount, wchar_t* pDst, size_t maxDstSize)
{
	return ConvertUTF16ToUTFT(pSrc, srcCharCount, pDst, maxDstSize);
}


size_t BvTextUtilities::ConvertUTF16ToUTF8Char(const char16_t* pSrc, size_t srcCharCount, char* pDst, size_t maxDstSize)
{
	return ConvertUTF16ToUTFT(pSrc, srcCharCount, pDst, maxDstSize);
}


size_t BvTextUtilities::ConvertUTF16ToUTF8(const char16_t* pSrc, size_t srcCharCount, char8_t* pDst, size_t maxDstSize)
{
	return ConvertUTF16ToUTFT(pSrc, srcCharCount, pDst, maxDstSize);
}


size_t BvTextUtilities::ConvertUTF16ToUTF32(const char16_t* pSrc, size_t srcCharCount, char32_t* pDst, size_t maxDstSize)
{
	return ConvertUTF16ToUTFT(pSrc, srcCharCount, pDst, maxDstSize);
}


size_t BvTextUtilities::ConvertUTF32ToASCII(const char32_t* pSrc, size_t srcCharCount, char* pDst, size_t maxDstSize)
{
	return ConvertCharTToASCII(pSrc, srcCharCount, pDst, maxDstSize);
}


size_t BvTextUtilities::ConvertUTF32ToWideChar(const char32_t* pSrc, size_t srcCharCount, wchar_t* pDst, size_t maxDstSize)
{
	return ConvertUTF32ToUTFT(pSrc, srcCharCount, pDst, maxDstSize);
}


size_t BvTextUtilities::ConvertUTF32ToUTF8Char(const char32_t* pSrc, size_t srcCharCount, char* pDst, size_t maxDstSize)
{
	return ConvertUTF32ToUTFT(pSrc, srcCharCount, pDst, maxDstSize);
}


size_t BvTextUtilities::ConvertUTF32ToUTF8(const char32_t* pSrc, size_t srcCharCount, char8_t* pDst, size_t maxDstSize)
{
	return ConvertUTF32ToUTFT(pSrc, srcCharCount, pDst, maxDstSize);
}


size_t BvTextUtilities::ConvertUTF32ToUTF16(const char32_t* pSrc, size_t srcCharCount, char16_t* pDst, size_t maxDstSize)
{
	return ConvertUTF32ToUTFT(pSrc, srcCharCount, pDst, maxDstSize);
}