#pragma once


#include "BDeV/Core/BvCore.h"


namespace Internal
{
	template<typename T>
	concept BvIterable = requires(T t)
	{
		{ t.begin() } -> std::input_or_output_iterator;
		{ t.end() } -> std::sentinel_for<decltype(t.begin())>;
	};

	constexpr u32 kInvalidUTFCharResult = 0XFFFD;

	template<typename It>
	u32 UTF8To32(It& curr, It end)
	{
		u32 len = 0;
		auto c = u8(*curr);
		if ((c & 0x80) == 0) { len = 1; }
		else if ((c & 0xE0) == 0xC0) { len = 2; }
		else if ((c & 0xF0) == 0xE0) { len = 3; }
		else if ((c & 0xF8) == 0xF0) { len = 4; }
		if (len == 0 || curr + len > end)
		{
			return kInvalidUTFCharResult;
		}

		constexpr u8 kFirstByteMasks[] = { 0x00, 0x7F, 0x1F, 0x0F, 0x07 };

		u32 cp = *curr++ & kFirstByteMasks[len];
		for (auto i = len - 1; i > 0; i--)
		{
			cp = (cp << 6) | (*curr++ & 0x3F);
		}

		return cp;
	}


	template<typename It>
	u32 UTF16To32(It& curr, It end)
	{
		// Check if it's a surrogate pair (0xD800 - 0xDFFF)
		if (*curr >= 0xD800 && *curr <= 0xDFFF && curr + 1 < end)
		{
			// High surrogate (0xD800-0xDBFF) + Low surrogate (0xDC00-0xDFFF)
			u32 high = *curr++;
			u32 low = *curr++;

			return u32(((high - 0xD800) << 10) + (low - 0xDC00) + 0x10000);
		}
		else if (*curr >= 0xDC00 && *curr <= 0xDFFF || curr == end)
		{
			return kInvalidUTFCharResult;
		}

		return u32(*curr++);
	}


	template<typename It>
	u32 UTF32To8(u32 codePoint, It& curr, It end)
	{
		u32 len = 0;
		if (codePoint <= 0x7F) { len = 1; }
		else if (codePoint <= 0x7FF) { len = 2; }
		else if (codePoint <= 0xFFFF) { len = 3; }
		else if (codePoint <= 0x10FFFF) { len = 4; }

		if (len == 0 || curr + len > end)
		{
			return kInvalidUTFCharResult;
		}

		constexpr u8 kFirstByteMasks[] = { 0x00, 0x7F, 0x1F, 0x0F, 0x07 };
		constexpr u8 kFirstByteValues[] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0 };

		*curr++ = u8(((codePoint >> ((len - 1) * 6)) & kFirstByteMasks[len]) | kFirstByteValues[len]);
		for (auto i = len - 1; i > 0; i--)
		{
			*curr++ = u8(((codePoint >> ((i - 1) * 6)) & 0x3F) | 0x80);
		}

		return codePoint;
	}


	template<typename It>
	u32 UTF32To16(u32 codePoint, It& curr, It end)
	{
		if (codePoint <= 0xFFFF && curr < end)
		{
			// BMP character
			*curr++ = u16(codePoint);

			return codePoint;
		}
		else if (codePoint <= 0x10FFFF && curr + 1 < end)
		{
			// Supplementary Plane - Need surrogate pair
			codePoint -= 0x10000;
			*curr++ = u16((codePoint >> 10) + 0xD800);
			*curr++ = u16((codePoint & 0x3FF) + 0xDC00);

			return codePoint;
		}
		else
		{
			return kInvalidUTFCharResult;
		}
	}


	template<typename It8, typename It16>
	u32 UTF8To16(It8& srcCurr, It8 srcEnd, It16& dstCurr, It16 dstEnd)
	{
		return UTF32To16(UTF8To32(srcCurr, srcEnd), dstCurr, dstEnd);
	}


	template<typename It8, typename It16>
	u32 UTF16To8(It16& srcCurr, It16 srcEnd, It8& dstCurr, It8 dstEnd)
	{
		return UTF32To8(UTF16To32(srcCurr, srcEnd), dstCurr, dstEnd);
	}

	template<typename It>
	size_t UTF8ByteCount(It begin, It end)
	{
		using UnderlyingType = typename std::iterator_traits<It>::value_type;
		constexpr auto kTypeSize = sizeof(UnderlyingType);

		static_assert(kTypeSize == 1 || kTypeSize == 2 || kTypeSize == 4, "Src UTF Encoding must be 8/16/32 bit");

		if constexpr (kTypeSize == 1)
		{
			return size_t(end - begin);
		}

		size_t byteCount = 0;
		for (auto curr = begin; curr < end; curr++)
		{
			if constexpr (kTypeSize == 2)
			{
				u16 cp = *curr;
				if (cp < 0x80) { byteCount += 1; }
				else if (cp < 0x800) { byteCount += 2; }
				// If it's a high surrogate, it pairs with the next unit to form a 4-byte UTF-8 glyph
				else if (cp >= 0xD800 && cp <= 0xDBFF) { byteCount += 4; curr++; }
				else { byteCount += 3; }
			}
			else if constexpr (kTypeSize == 4)
			{
				u32 cp = *curr;
				if (cp < 0x80) { byteCount += 1; }
				else if (cp < 0x800) { byteCount += 2; }
				else if (cp < 0x10000) { byteCount += 3; }
				else { byteCount += 4; }
			}
		}

		return byteCount;
	}

	template<typename It>
	size_t UTF16ByteCount(It begin, It end)
	{
		using UnderlyingType = typename std::iterator_traits<It>::value_type;
		constexpr auto kTypeSize = sizeof(UnderlyingType);

		static_assert(kTypeSize == 1 || kTypeSize == 2 || kTypeSize == 4, "Src UTF Encoding must be 8/16/32 bit");

		if constexpr (kTypeSize == 2)
		{
			return size_t(end - begin) << 1;
		}

		size_t byteCount = 0;
		for (auto curr = begin; curr < end; curr++)
		{
			if constexpr (kTypeSize == 1)
			{
				u8 byte = *curr;
				if (byte < 0x80) { byteCount += 2; }
				else if (byte < 0xE0) { byteCount += 2; curr += 1; }
				else if (byte < 0xF0) { byteCount += 2; curr += 2; }
				else { byteCount += 4; curr += 3; } // Surrogate pair needed
			}
			else if constexpr (kTypeSize == 4)
			{
				// Branchless optimization: adds 4 if cp >= 0x10000, else adds 2
				u32 cp = *curr;
				byteCount += 2 + 2 * (cp >= 0x10000);
			}
		}

		return byteCount;
	}

	template<typename It>
	size_t UTF32ByteCount(It begin, It end)
	{
		using UnderlyingType = typename std::iterator_traits<It>::value_type;
		constexpr auto kTypeSize = sizeof(UnderlyingType);

		static_assert(kTypeSize == 1 || kTypeSize == 2 || kTypeSize == 4, "Src UTF Encoding must be 8/16/32 bit");

		if constexpr (kTypeSize == 4)
		{
			return size_t(end - begin) << 2;
		}

		size_t byteCount = 0;
		for (auto curr = begin; curr < end; curr++, byteCount += 4)
		{
			if constexpr (kTypeSize == 1)
			{
				u8 byte = *curr;
				if (byte < 0x80) {}
				else if (byte < 0xE0) { curr += 1; }
				else if (byte < 0xE0) { curr += 2; }
				else { curr += 3; }
			}
			else if constexpr (kTypeSize == 2)
			{
				u16 u = *curr;
				if (u >= 0xD800 && u <= 0xDBFF)
				{
					curr++; // Skip low surrogate, they combine into 1 code point
				}
			}
		}

		return byteCount;
	}

	template<typename T>
	size_t UTFCharSequenceCount(T ch)
	{
		constexpr auto kTypeSize = sizeof(T);

		static_assert(kTypeSize == 1 || kTypeSize == 2 || kTypeSize == 4, "Src UTF Encoding must be 8/16/32 bit");

		if constexpr (kTypeSize == 1)
		{
			if ((ch & 0x80) == 0) { return 1; }
			else if ((ch & 0xE0) == 0xC0) { return 2; }
			else if ((ch & 0xF0) == 0xE0) { return 3; }
			else if ((ch & 0xF8) == 0xF0) { return 4; }

			return 0;
		}
		else if constexpr (kTypeSize == 2)
		{
			// Check if it's a surrogate pair (0xD800 - 0xDFFF)
			if (ch >= 0xD800 && ch <= 0xDFFF)
			{
				// High surrogate (0xD800-0xDBFF) + Low surrogate (0xDC00-0xDFFF)
				return 2;
			}

			return size_t(!(ch >= 0xDC00 && ch <= 0xDFFF));
		}
		else if constexpr (kTypeSize == 4)
		{
			return size_t((ch <= 0x10FFFF) && (ch < 0xD800 || ch > 0xDFFF));
		}
		else
		{
			return 0;
		}
	}

	template<typename SrcIt, typename DstIt>
	bool ConvertUTFChar(SrcIt& srcCurr, SrcIt srcEnd, DstIt& dstCurr, DstIt dstEnd)
	{
		if (srcCurr >= srcEnd || dstCurr >= dstEnd)
		{
			return false;
		}

		using SrcUnderlyingType = typename std::iterator_traits<SrcIt>::value_type;
		using DstUnderlyingType = typename std::iterator_traits<DstIt>::value_type;
		constexpr auto kSrcTypeSize = sizeof(SrcUnderlyingType);
		constexpr auto kDstTypeSize = sizeof(DstUnderlyingType);

		static_assert(kSrcTypeSize == 1 || kSrcTypeSize == 2 || kSrcTypeSize == 4, "Src UTF Encoding must be 8/16/32 bit");
		static_assert(kDstTypeSize == 1 || kDstTypeSize == 2 || kDstTypeSize == 4, "Dst UTF Encoding must be 8/16/32 bit");
		static_assert(kSrcTypeSize != kDstTypeSize, "Src and Dst UTF Encodings must be different");

		if constexpr (kSrcTypeSize == 1 && kDstTypeSize == 2)
		{
			return Internal::UTF8To16(srcCurr, srcEnd, dstCurr, dstEnd) != kInvalidUTFCharResult;
		}
		else if constexpr (kSrcTypeSize == 1 && kDstTypeSize == 4)
		{
			DstUnderlyingType cp = Internal::UTF8To32(srcCurr, srcEnd);
			if (cp == kInvalidUTFCharResult)
			{
				return false;
			}

			*dstCurr++ = cp;
			return true;
		}
		else if constexpr (kSrcTypeSize == 2 && kDstTypeSize == 1)
		{
			return Internal::UTF16To8(srcCurr, srcEnd, dstCurr, dstEnd) != kInvalidUTFCharResult;
		}
		else if constexpr (kSrcTypeSize == 2 && kDstTypeSize == 4)
		{
			DstUnderlyingType cp = Internal::UTF16To32(srcCurr, srcEnd);
			if (cp == kInvalidUTFCharResult)
			{
				return false;
			}

			*dstCurr++ = cp;
			return true;
		}
		else if constexpr (kSrcTypeSize == 4 && kDstTypeSize == 1)
		{
			return Internal::UTF32To8(*srcCurr++, dstCurr, dstEnd) != kInvalidUTFCharResult;
		}
		else if constexpr (kSrcTypeSize == 4 && kDstTypeSize == 2)
		{
			return Internal::UTF32To16(*srcCurr++, dstCurr, dstEnd) != kInvalidUTFCharResult;
		}
		else
		{
			return false;
		}
	}
}


struct BvUTFCharTraits
{
	// Returns the number of code points, where each code point can be one to four bytes
	template<typename It>
	static size_t CodePointLength(It begin, It end)
	{
		size_t len = 0;
		auto it = begin;
		for (; it < end && *it; len++)
		{
			auto cl = Internal::UTFCharSequenceCount(*it);
			if (cl == 0)
			{
				return 0;
			}

			it += cl;
		}

		return len;
	}


	// Returns the number of code points, where each code point can be one to four bytes
	template<Internal::BvIterable It>
	static size_t CodePointLength(It str)
	{
		return CodePointLength(str.begin(), str.end());
	}


	// Returns the number of elements necessary for converting a specific UTF-encoded string to another UTF encoding, based on the template type
	template<typename T, typename It>
	static size_t LengthFor(It begin, It end)
	{
		constexpr auto kTypeSize = sizeof(T);

		static_assert(kTypeSize == 1 || kTypeSize == 2 || kTypeSize == 4, "Src UTF Encoding must be 8/16/32 bit");

		if constexpr (kTypeSize == 1)
		{
			return Internal::UTF8ByteCount(begin, end);
		}
		else if constexpr (kTypeSize == 2)
		{
			return Internal::UTF16ByteCount(begin, end) >> 1;
		}
		else if constexpr (kTypeSize == 4)
		{
			return Internal::UTF32ByteCount(begin, end) >> 2;
		}
		else
		{
			return 0;
		}
	}


	// Returns the number of elements necessary for converting a specific UTF-encoded string to another UTF encoding, based on the template type
	template<typename T, Internal::BvIterable It>
	static size_t LengthFor(It str)
	{
		return LengthFor<T, decltype(std::begin(str))>(str.begin(), str.end());
	}


	// Converts a UTF code point to another UTF encoding
	template<typename SrcIt, typename DstIt>
	static DstIt GetChar(SrcIt srcBegin, SrcIt srcEnd, DstIt dstBegin, DstIt dstEnd)
	{
		Internal::ConvertUTFChar(srcBegin, srcEnd, dstBegin, dstEnd);
		
		return dstBegin;
	}


	// Converts a UTF code point to another UTF encoding
	template<Internal::BvIterable SrcIt, Internal::BvIterable DstIt>
	static auto GetChar(SrcIt src, DstIt dst)
	{
		return GetChar(src.begin(), src.end(), dst.begin(), dst.end());
	}


	// Converts a UTF code point to another UTF encoding
	template<typename SrcIt, Internal::BvIterable DstIt>
	static auto GetChar(SrcIt srcBegin, SrcIt srcEnd, DstIt dst)
	{
		return GetChar(srcBegin, srcEnd, dst.begin(), dst.end());
	}


	// Converts a UTF code point to another UTF encoding
	template<Internal::BvIterable SrcIt, typename DstIt>
	static auto GetChar(SrcIt src, DstIt dstBegin, DstIt dstEnd)
	{
		return GetChar(src.begin(), src.end(), dstBegin, dstEnd);
	}


	// Converts a UTF string to another UTF encoding
	template<typename SrcIt, typename DstIt>
	static DstIt GetStr(SrcIt srcBegin, SrcIt srcEnd, DstIt dstBegin, DstIt dstEnd)
	{
		while (Internal::ConvertUTFChar(srcBegin, srcEnd, dstBegin, dstEnd))
		{
		}

		return dstBegin;
	}


	// Converts a UTF string to another UTF encoding
	template<Internal::BvIterable SrcIt, Internal::BvIterable DstIt>
	static auto GetStr(SrcIt src, DstIt dst)
	{
		return GetStr(src.begin(), src.end(), dst.begin(), dst.end());
	}


	// Converts a UTF string to another UTF encoding
	template<typename SrcIt, Internal::BvIterable DstIt>
	static auto GetStr(SrcIt srcBegin, SrcIt srcEnd, DstIt dst)
	{
		return GetStr(srcBegin, srcEnd, dst.begin(), dst.end());
	}


	// Converts a UTF string to another UTF encoding
	template<Internal::BvIterable SrcIt, typename DstIt>
	static auto GetStr(SrcIt src, DstIt dstBegin, DstIt dstEnd)
	{
		return GetStr(src.begin(), src.end(), dstBegin, dstEnd);
	}
};