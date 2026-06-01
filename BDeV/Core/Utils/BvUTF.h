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


	template<typename It>
	u32 UTF8To32(It& curr, It end)
	{
		u32 len = 0;
		auto c = u8(*curr);
		if ((c & 0x80) == 0) { len = 1; }
		else if ((c & 0xE0) == 0xC0) { len = 2; }
		else if ((c & 0xF0) == 0xE0) { len = 3; }
		else if ((c & 0xF8) == 0xF0) { len = 4; }
		if (len == 0 || curr + len >= end)
		{
			curr = end;
			return 0xFFFD;
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
			curr = end;
			return 0xFFFD;
		}

		return u32(*curr++);
	}


	template<typename It>
	void UTF32To8(u32 codePoint, It& curr, It end)
	{
		u32 len = 0;
		if (codePoint <= 0x7F) { len = 1; }
		else if (codePoint <= 0x7FF) { len = 2; }
		else if (codePoint <= 0xFFFF) { len = 3; }
		else if (codePoint <= 0x10FFFF) { len = 4; }

		if (len == 0 || curr + len >= end)
		{
			curr = end;
			return;
		}

		constexpr u8 kFirstByteMasks[] = { 0x00, 0x7F, 0x1F, 0x0F, 0x07 };
		constexpr u8 kFirstByteValues[] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0 };

		*curr++ = u8(((codePoint >> ((len - 1) * 6)) & kFirstByteMasks[len]) | kFirstByteValues[len]);
		for (auto i = len - 1; i > 0; i--)
		{
			*curr++ = u8(((codePoint >> ((i - 1) * 6)) & 0x3F) | 0x80);
		}
	}


	template<typename It>
	void UTF32To16(u32 codePoint, It& curr, It end)
	{
		if (codePoint <= 0xFFFF && curr < end)
		{
			// BMP character
			*curr++ = u16(codePoint);
		}
		else if (codePoint <= 0x10FFFF && curr + 1 < end)
		{
			// Supplementary Plane - Need surrogate pair
			codePoint -= 0x10000;
			*curr++ = u16((codePoint >> 10) + 0xD800);
			*curr++ = u16((codePoint & 0x3FF) + 0xDC00);
		}
		else
		{
			curr = end;
		}
	}


	template<typename It8, typename It16>
	void UTF8To16(It8& srcCurr, It8 srcEnd, It16& dstCurr, It16 dstEnd)
	{
		UTF32To16(UTF8To32(srcCurr, srcEnd), dstCurr, dstEnd);
	}


	template<typename It8, typename It16>
	void UTF16To8(It16& srcCurr, It16 srcEnd, It8& dstCurr, It8 dstEnd)
	{
		UTF32To8(UTF16To32(srcCurr, srcEnd), dstCurr, dstEnd);
	}
}


struct UTFCharTraits
{
	template<typename It>
	static size_t CharLength(It curr)
	{
		using UnderlyingType = typename std::iterator_traits<It>::value_type;
		constexpr auto kTypeSize = sizeof(UnderlyingType);

		static_assert(kTypeSize == 1 || kTypeSize == 2 || kTypeSize == 4, "Src UTF Encoding must be 8/16/32 bit");

		if constexpr (kTypeSize == 1)
		{
			if ((*curr & 0x80) == 0) { return 1; }
			else if ((*curr & 0xE0) == 0xC0) { return 2; }
			else if ((*curr & 0xF0) == 0xE0) { return 3; }
			else if ((*curr & 0xF8) == 0xF0) { return 4; }

			return 0;
		}
		else if constexpr (kTypeSize == 2)
		{
			// Check if it's a surrogate pair (0xD800 - 0xDFFF)
			if (*curr >= 0xD800 && *curr <= 0xDFFF)
			{
				// High surrogate (0xD800-0xDBFF) + Low surrogate (0xDC00-0xDFFF)
				return 2;
			}

			return size_t(!(*curr >= 0xDC00 && *curr <= 0xDFFF));
		}
		else if constexpr (kTypeSize == 4)
		{
			return size_t((*curr <= 0x10FFFF) && (*curr < 0xD800 || *curr > 0xDFFF));
		}
		else
		{
			std::unreachable();
		}
	}
	
	template<typename It>
	static size_t Length(It curr, It end)
	{
		size_t len = 0;
		auto it = curr;
		for (; it < end && *it; len++)
		{
			auto cl = CharLength(it);
			if (cl == 0)
			{
				return 0;
			}

			it += cl;
		}

		return len;
	}

	template<Internal::BvIterable It>
	static size_t Length(It str)
	{
		return Length(str.begin(), str.end());
	}


	template<typename SrcIt, typename DstIt>
	static size_t GetStr(SrcIt srcBegin, SrcIt srcEnd, DstIt dstBegin, DstIt dstEnd)
	{
		auto srcCurr = srcBegin;
		auto dstCurr = dstBegin;
		while (srcCurr < srcEnd && dstCurr < dstEnd)
		{
			ConvertChar(srcCurr, srcEnd, dstCurr, dstEnd);
		}

		return size_t(dstCurr - dstBegin);
	}

	template<Internal::BvIterable SrcIt, Internal::BvIterable DstIt>
	static size_t GetStr(SrcIt src, DstIt dst)
	{
		return GetStr(src.begin(), src.end(), dst.begin(), dst.end());
	}

	template<typename SrcIt, typename DstIt>
	static void GetChar(SrcIt srcBegin, SrcIt srcEnd, DstIt dstBegin, DstIt dstEnd)
	{
		auto srcCurr = srcBegin;
		auto dstCurr = dstBegin;
		if (srcCurr < srcEnd && dstCurr < dstEnd)
		{
			ConvertChar(srcCurr, srcEnd, dstCurr, dstEnd);
		}
	}

private:
	template<typename SrcIt, typename DstIt>
	static void ConvertChar(SrcIt& srcCurr, SrcIt srcEnd, DstIt& dstCurr, DstIt dstEnd)
	{
		BV_ASSERT(srcCurr < srcEnd && dstCurr < dstEnd, "Invalid iterators");

		using SrcUnderlyingType = typename std::iterator_traits<SrcIt>::value_type;
		using DstUnderlyingType = typename std::iterator_traits<DstIt>::value_type;
		constexpr auto kSrcTypeSize = sizeof(SrcUnderlyingType);
		constexpr auto kDstTypeSize = sizeof(DstUnderlyingType);

		static_assert(kSrcTypeSize == 1 || kSrcTypeSize == 2 || kSrcTypeSize == 4, "Src UTF Encoding must be 8/16/32 bit");
		static_assert(kDstTypeSize == 1 || kDstTypeSize == 2 || kDstTypeSize == 4, "Dst UTF Encoding must be 8/16/32 bit");
		static_assert(kSrcTypeSize != kDstTypeSize, "Src and Dst UTF Encodings must be different");

		if constexpr (kSrcTypeSize == 1 && kDstTypeSize == 2)
		{
			Internal::UTF8To16(srcCurr, srcEnd, dstCurr, dstEnd);
		}
		else if constexpr (kSrcTypeSize == 1 && kDstTypeSize == 4)
		{
			*dstCurr++ = Internal::UTF8To32(srcCurr, srcEnd);
		}
		else if constexpr (kSrcTypeSize == 2 && kDstTypeSize == 1)
		{
			Internal::UTF16To8(srcCurr, srcEnd, dstCurr, dstEnd);
		}
		else if constexpr (kSrcTypeSize == 2 && kDstTypeSize == 4)
		{
			*dstCurr++ = Internal::UTF16To32(srcCurr, srcEnd);
		}
		else if constexpr (kSrcTypeSize == 4 && kDstTypeSize == 1)
		{
			Internal::UTF32To8(*srcCurr++, dstCurr, dstEnd);
		}
		else if constexpr (kSrcTypeSize == 4 && kDstTypeSize == 2)
		{
			Internal::UTF32To16(*srcCurr++, dstCurr, dstEnd);
		}
		else
		{
			std::unreachable();
		}
	}
};