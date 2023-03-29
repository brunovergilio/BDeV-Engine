#pragma once


#include "BDeV/System/Debug/BvDebug.h"
#include "BDeV/Utils/BvUtils.h"
#include "BDeV/Utils/Hash.h"
#include "BDeV/Container/BvVector.h"
#include <cstdarg>
#include <string>


template<typename CharT>
class CharHelper
{
public:
#define BV_CHAR_TYPE_PREFIX(c)												\
	if constexpr (std::is_same_v<CharT, char>) { return c; }				\
	else if constexpr (std::is_same_v<CharT, wchar_t>) { return L##c; }		\
	else if constexpr (std::is_same_v<CharT, char8_t>) { return u8##c; }	\
	else if constexpr (std::is_same_v<CharT, char16_t>) { return u##c; }	\
	else if constexpr (std::is_same_v<CharT, char32_t>) { return U##c; }	\
	else { return 0; }														\

	static constexpr CharT NewLine() { BV_CHAR_TYPE_PREFIX('\n') }
	static constexpr CharT Tab() { BV_CHAR_TYPE_PREFIX('\t') }
	static constexpr CharT CarriageReturn() { BV_CHAR_TYPE_PREFIX('\n') }
#undef BV_CHAR_TYPE_PREFIX
};


template<typename CharT>
class BvStringT
{
public:
	static constexpr u32 kInvalidIndex = kU32Max;

	BvStringT();
	explicit BvStringT(const u32 size, const CharT c = 0);
	BvStringT(const BvStringT & rhs);
	BvStringT & operator =(const BvStringT & rhs);
	BvStringT(BvStringT && rhs) noexcept;
	BvStringT & operator =(BvStringT && rhs) noexcept;
	BvStringT & operator =(const CharT * const pStr);
	BvStringT & operator =(const CharT c);
	~BvStringT();

	explicit BvStringT(const BvStringT & str, const u32 start, const u32 count);
	BvStringT(const CharT * const pStr);
	explicit BvStringT(const CharT * const pStr, const u32 start, const u32 count);
	BvStringT(const CharT c);

	void Assign(const BvStringT & str);
	void Assign(const CharT * const pStr);
	void Assign(const CharT c);
	void Assign(const BvStringT & str, const u32 start, const u32 count);
	void Assign(const CharT * const pStr, const u32 start, const u32 count);

	void Insert(const BvStringT & str, const u32 where = 0);
	void Insert(const CharT * const pStr, const u32 where = 0);
	void Insert(const CharT c, const u32 where = 0);
	void Insert(const BvStringT & str, const u32 start, const u32 count, const u32 where = 0);
	void Insert(const CharT * const pStr, const u32 start, const u32 count, const u32 where = 0);

	void Append(const BvStringT & str) { Insert(str, m_Size); }
	void Append(const CharT * const pStr) { Insert(pStr, m_Size); }
	void Append(const CharT c) { Insert(c, m_Size); }
	void Append(const BvStringT & str, const u32 start, const u32 count) { Insert(str, start, count, m_Size); }
	void Append(const CharT * const pStr, const u32 start, const u32 count) { Insert(pStr, start, count, m_Size); }

	void Replace(const BvStringT & srcStr, const BvStringT dstStr);
	void Replace(const CharT * const pSrcStr, const CharT * const pDstStr);
	void Replace(const CharT * const pSrcStr, const u32 srcSize, const CharT * const pDstStr, const u32 dstSize);

	const u32 ReadLine(BvStringT& dstStr, const u32 offset = 0) const;
	const u32 Read(BvStringT& dstStr, const CharT delim, const u32 startIndex = 0) const;
	const u32 Read(BvStringT& dstStr, const CharT* const pDelim, const u32 size, const u32 startIndex = 0) const;
	void Format(const CharT * const format, ...);

	void Erase(const u32 start, const u32 count);

	void Resize(const u32 size, const CharT c = 0);
	void Clear();

	void Swap(BvStringT & str);
	void Copy(BvStringT & str) const;
	void Copy(BvStringT & str, const u32 start, const u32 count) const;
	BvStringT Substr(const u32 start, const u32 count) const;
	void Split(BvVector<BvStringT>& result, CharT delimiter, const CharT* pSymbols = nullptr) const;
	void Split(BvVector<BvStringT>& result, const CharT* pDelimiters, const CharT* pSymbols = nullptr) const;
	void Split(BvVector<BvStringT>& result, const CharT* pDelimiters, u32 delimeterCount, const CharT* = nullptr, u32 symbolCount = 0) const;

	bool StartsWith(const CharT c, const u32 offset = 0) const;
	bool StartsWith(const BvStringT& str, const u32 offset = 0) const;
	bool StartsWith(const CharT* const pStr, const u32 offset = 0) const;
	bool StartsWith(const CharT* const pStr, const u32 offset, const u32 size) const;

	const u32 Find(const CharT c, const u32 offset = 0) const;
	const u32 Find(const BvStringT & str, const u32 offset = 0) const;
	const u32 Find(const CharT * const pStr, const u32 offset = 0) const;
	const u32 Find(const CharT * const pStr, const u32 offset, const u32 size) const;

	const u32 RFind(const CharT c, const u32 offset = kInvalidIndex) const;
	const u32 RFind(const BvStringT & str, const u32 offset = kInvalidIndex) const;
	const u32 RFind(const CharT * const pStr, const u32 offset = kInvalidIndex) const;
	const u32 RFind(const CharT * const pStr, const u32 size, const u32 offset) const;

	const u32 FindFirstOf(const CharT c, const u32 offset = 0) const;
	const u32 FindFirstOf(const BvStringT& str, const u32 offset = 0) const;
	const u32 FindFirstOf(const CharT* const pStr, const u32 offset = 0) const;
	const u32 FindFirstOf(const CharT* const pStr, const u32 offset, const u32 size) const;

	const u32 FindFirstNotOf(const CharT c, const u32 offset = 0) const;
	const u32 FindFirstNotOf(const BvStringT& str, const u32 offset = 0) const;
	const u32 FindFirstNotOf(const CharT* const pStr, const u32 offset = 0) const;
	const u32 FindFirstNotOf(const CharT* const pStr, const u32 offset, const u32 size) const;

	const u32 FindLastOf(const CharT c, const u32 offset = kInvalidIndex) const;
	const u32 FindLastOf(const BvStringT& str, const u32 offset = kInvalidIndex) const;
	const u32 FindLastOf(const CharT* const pStr, const u32 offset = kInvalidIndex) const;
	const u32 FindLastOf(const CharT* const pStr, const u32 offset, const u32 size) const;

	const u32 FindLastNotOf(const CharT c, const u32 offset = kInvalidIndex) const;
	const u32 FindLastNotOf(const BvStringT& str, const u32 offset = kInvalidIndex) const;
	const u32 FindLastNotOf(const CharT* const pStr, const u32 offset = kInvalidIndex) const;
	const u32 FindLastNotOf(const CharT* const pStr, const u32 offset, const u32 size) const;

	const u64 Hash() const;

	BV_INLINE const bool Contains(const CharT c) const { return Find(c) != kInvalidIndex; }
	BV_INLINE const bool Contains(const BvStringT & str) const { return Find(str) != kInvalidIndex; }
	BV_INLINE const bool Contains(const CharT * const pStr) const { return Find(pStr) != kInvalidIndex; }

	BvStringT & operator +=(const BvStringT & str) { Append(str); return *this; }
	BvStringT & operator +=(const CharT * const pStr) { Append(pStr); return *this; }
	BvStringT & operator +=(const CharT c) { Append(c); return *this; }

	BvStringT operator +(const BvStringT & str);
	BvStringT operator +(const CharT * const pStr);
	BvStringT operator +(const CharT c);
	friend BvStringT operator +(const CharT * const pStr, const BvStringT & str);
	friend BvStringT operator +(const CharT c, const BvStringT & str);

	BV_INLINE const CharT & operator[](const u32 index) const { BvAssert(index < m_Size, "Index out of bounds"); return m_pStr[index]; }
	BV_INLINE CharT & operator[](const u32 index) { BvAssert(index < m_Size, "Index out of bounds"); return m_pStr[index]; }
	BV_INLINE const CharT At(const u32 index) const { BvAssert(index < m_Size, "Index out of bounds"); return m_pStr[index]; }
	BV_INLINE const CharT Front() const { BvAssert(m_Size > 0, "Index out of bounds"); return m_pStr[0]; }
	BV_INLINE const CharT Back() const { BvAssert(m_Size > 0, "Index out of bounds"); return m_pStr[m_Size - 1]; }

	BV_INLINE const CharT * const CStr() const { return m_pStr; }
	BV_INLINE const u32 Size() const { return m_Size; }
	BV_INLINE const u32 Capacity() const { return m_Capacity; }
	BV_INLINE const bool Empty() const { return m_Size == 0; }
	BV_INLINE CharT GetFirstChar() const { return m_Size > 0 ? m_pStr[0] : 0; }
	BV_INLINE CharT GetLastChar() const { return m_Size > 0 ? m_pStr[m_Size - 1] : 0; }

	BV_INLINE operator const CharT * const() { return m_pStr; }

	const i32 Compare(const BvStringT & str) const { return Compare(str.m_pStr, str.m_Size); }
	const i32 Compare(const CharT* const pStr, size_t size) const;

	const bool operator ==(CharT c) const { return m_Size == 1 && c == m_pStr[0]; }
	const bool operator ==(const BvStringT & str) const { return Compare(str) == 0; }
	const bool operator ==(const CharT * const pStr) const { return Compare(pStr) == 0; }

	const bool operator !=(CharT c) const { return !(*this == c); }
	const bool operator !=(const BvStringT& str) const { return Compare(str) != 0; }
	const bool operator !=(const CharT* const pStr) const { return Compare(pStr) != 0; }

	const bool operator <(const BvStringT & str) const { return Compare(str) < 0; }
	const bool operator <(const CharT * const pStr) const { return Compare(pStr) < 0; }

	const bool operator <=(const BvStringT & str) const { return Compare(str) <= 0; }
	const bool operator <=(const CharT * const pStr) const { return Compare(pStr) <= 0; }

	const bool operator >(const BvStringT & str) const { return Compare(str) > 0; }
	const bool operator >(const CharT * const pStr) const { return Compare(pStr) > 0; }

	const bool operator >=(const BvStringT & str) const { return Compare(str) >= 0; }
	const bool operator >=(const CharT * const pStr) const { return Compare(pStr) >= 0; }

private:
	void Destroy();

protected:
	CharT * m_pStr = nullptr;
	u32 m_Size = 0;
	u32 m_Capacity = 0;
};


template<typename CharT>
struct std::hash<BvStringT<CharT>>
{
	u64 operator()(const BvStringT<CharT>& val)
	{
		return val.Hash();
	}
};


template<typename CharT>
BvStringT<CharT>::BvStringT()
{
}


template<typename CharT>
BvStringT<CharT>::BvStringT(const u32 size, const CharT c)
{
	Resize(size, c);
}


template<typename CharT>
BvStringT<CharT>::BvStringT(const BvStringT<CharT>& rhs)
{
	Assign(rhs);
}


template<typename CharT>
BvStringT<CharT>& BvStringT<CharT>::operator=(const BvStringT& rhs)
{
	if (this != &rhs)
	{
		Destroy();

		Assign(rhs);
	}

	return *this;
}


template<typename CharT>
BvStringT<CharT>::BvStringT(BvStringT&& rhs) noexcept
{
	*this = std::move(rhs);
}


template<typename CharT>
BvStringT<CharT>& BvStringT<CharT>::operator=(BvStringT&& rhs) noexcept
{
	if (this != &rhs)
	{
		Destroy();

		m_pStr = rhs.m_pStr;
		m_Size = rhs.m_Size;
		m_Capacity = rhs.m_Capacity;

		rhs.m_pStr = nullptr;
		rhs.m_Size = 0;
		rhs.m_Capacity = 0;
	}

	return *this;
}


template<typename CharT>
BvStringT<CharT>& BvStringT<CharT>::operator=(const CharT* const pStr)
{
	if (m_pStr != pStr)
	{
		Destroy();

		Assign(pStr);
	}

	return *this;
}

template<typename CharT>
BvStringT<CharT>& BvStringT<CharT>::operator=(const CharT c)
{
	Destroy();

	Assign(c);

	return *this;
}


template<typename CharT>
BvStringT<CharT>::~BvStringT()
{
	Destroy();
}


template<typename CharT>
BvStringT<CharT>::BvStringT(const BvStringT& str, const u32 start, const u32 count)
{
	Assign(str, start, count);
}


template<typename CharT>
BvStringT<CharT>::BvStringT(const CharT* const pStr)
{
	Assign(pStr);
}


template<typename CharT>
BvStringT<CharT>::BvStringT(const CharT* const pStr, const u32 start, const u32 count)
{
	Assign(pStr, start, count);
}


template<typename CharT>
BvStringT<CharT>::BvStringT(const CharT c)
{
	Assign(c);
}


template<typename CharT>
void BvStringT<CharT>::Assign(const BvStringT& str)
{
	Assign(str.m_pStr, 0, str.m_Size);
}


template<typename CharT>
void BvStringT<CharT>::Assign(const CharT* const pStr)
{
	Assign(pStr, 0, pStr ? static_cast<u32>(std::char_traits<CharT>::length(pStr)) : 0);
}


template<typename CharT>
void BvStringT<CharT>::Assign(const CharT c)
{
	Assign(&c, 0, 1);
}


template<typename CharT>
void BvStringT<CharT>::Assign(const BvStringT& str, const u32 start, const u32 count)
{
	Assign(str.m_pStr, start, count);
}


template<typename CharT>
void BvStringT<CharT>::Assign(const CharT* const pStr, const u32 start, const u32 count)
{
	if (count == 0 || !pStr)
	{
		return;
	}

	BvAssert(m_pStr != pStr, "Same underlying string pointer");

	if (m_Capacity <= count)
	{
		Resize(GetNextPowerOf2(count));
	}

	std::char_traits<CharT>::copy(m_pStr, pStr + start, count);
	m_pStr[count] = 0;
	m_Size = count;
}


template<typename CharT>
void BvStringT<CharT>::Insert(const BvStringT& str, const u32 where)
{
	Insert(str.m_pStr, 0, str.m_Size, where);
}


template<typename CharT>
void BvStringT<CharT>::Insert(const CharT* const pStr, const u32 where)
{
	Insert(pStr, 0, static_cast<u32>(std::char_traits<CharT>::length(pStr)), where);
}


template<typename CharT>
void BvStringT<CharT>::Insert(const CharT c, const u32 where)
{
	Insert(&c, 0, 1, where);
}


template<typename CharT>
void BvStringT<CharT>::Insert(const BvStringT& str, const u32 start, const u32 count, const u32 where)
{
	Insert(str.m_pStr, start, count, where);
}


template<typename CharT>
void BvStringT<CharT>::Insert(const CharT* const pStr, const u32 start, const u32 count, const u32 where)
{
	BvAssert(m_Size >= where, "Position past the string's size");
	if (count == 0)
	{
		return;
	}

	u32 oldSize = m_Size;
	u32 newSize = count + m_Size;
	if (m_Capacity <= newSize)
	{
		Resize(GetNextPowerOf2(newSize));
	}
	m_pStr[newSize] = 0;

	if (oldSize > 0)
	{
		std::char_traits<CharT>::move(m_pStr + where + count, m_pStr + where, oldSize - where);
	}
	std::char_traits<CharT>::copy(m_pStr + where, pStr + start, count);
	m_Size = newSize;
}


template<typename CharT>
void BvStringT<CharT>::Replace(const BvStringT& srcStr, const BvStringT dstStr)
{
	Replace(srcStr.m_pStr, srcStr.m_Size, dstStr.m_pStr, dstStr.m_Size);
}


template<typename CharT>
void BvStringT<CharT>::Replace(const CharT* const pSrcStr, const CharT* const pDstStr)
{
	Replace(pSrcStr, static_cast<u32>(std::char_traits<CharT>::length(pSrcStr)), pDstStr,
		static_cast<u32>(std::char_traits<CharT>::length(pDstStr)));
}


template<typename CharT>
void BvStringT<CharT>::Replace(const CharT* const pSrcStr, const u32 srcSize, const CharT* const pDstStr, const u32 dstSize)
{
	u32 index = Find(pSrcStr, srcSize);
	if (index == kInvalidIndex)
	{
		return;
	}

	Erase(index, srcSize);
	Insert(pDstStr, 0, dstSize, index);
}


template<typename CharT>
const u32 BvStringT<CharT>::ReadLine(BvStringT& dstStr, const u32 offset) const
{
	dstStr.Clear();
	if (offset >= m_Size)
	{
		return kInvalidIndex;
	}

	u32 i = offset;
	while (i < m_Size && m_pStr[i] != CharHelper<CharT>::NewLine())
	{
		i++;
	}

	u32 count = i - offset;
	if (count > 0)
	{
		dstStr.Assign(m_pStr, offset, count);
	}

	return offset + count + 1;
}


template<typename CharT>
inline const u32 BvStringT<CharT>::Read(BvStringT& dstStr, const CharT delim, const u32 offset) const
{
	return Read(dstStr, &delim, 1, offset);
}


template<typename CharT>
inline const u32 BvStringT<CharT>::Read(BvStringT& dstStr, const CharT* const pDelim, const u32 size, const u32 offset) const
{
	dstStr.Clear();
	if (offset >= m_Size)
	{
		return kInvalidIndex;
	}

	u32 i = offset;
	bool found = false;
	while (i < m_Size && !found)
	{
		for (auto j = 0; j < size; j++)
		{
			if (m_pStr[i] = pDelim[j])
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			++i;
		}
	}

	u32 count = i - offset;
	if (count > 0)
	{
		dstStr.Assign(m_pStr, offset, count);
	}

	return offset + count + 1;
}


template<typename CharT>
void BvStringT<CharT>::Format(const CharT* const format, ...)
{
	u32 size = 0;
	{
		va_list args;
		va_start(args, format);
		size = vsnprintf(nullptr, 0, format, args) + 1;
		va_end(args);

		if (size == 1)
		{
			return;
		}
	}

	if (size >= m_Capacity)
	{
		Resize(GetNextPowerOf2(size));
	}

	va_list args;
	va_start(args, format);
	vsnprintf(m_pStr, size, format, args);
	va_end(args);

	m_pStr[size] = 0;
}


template<typename CharT>
void BvStringT<CharT>::Erase(const u32 start, const u32 count)
{
	BvAssert(start < m_Size&& count > 0, "Erasing past the string's size");
	u32 end = start + count;

	u32 removed = count;
	if (end < m_Size)
	{
		std::char_traits<CharT>::move(m_pStr + start, m_pStr + end, m_Size - end);
	}
	else
	{
		removed = m_Size - start;
	}

	m_Size -= removed;
	m_pStr[m_Size] = 0;
}

#if (BV_PLATFORM == BV_PLATFORM_WIN32 && BV_COMPILER == BV_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable:6386) // Buffer overrun
#endif

template<typename CharT>
void BvStringT<CharT>::Resize(const u32 size, const CharT c)
{
	BvAssert(size > 0, "String resizing needs a valid size");
	if (size <= m_Size)
	{
		return;
	}

	if (size >= m_Capacity)
	{
		CharT* pNewStr = new CharT[size + 1];
		if (m_Size > 0)
		{
			std::char_traits<CharT>::copy(pNewStr, m_pStr, m_Size);
		}
		pNewStr[m_Size] = 0;
		delete[] m_pStr;
		m_pStr = pNewStr;
		m_Capacity = size + 1;
	}

	for (auto i = m_Size; i < size; i++)
	{
		m_pStr[i] = c;
	}
	m_pStr[size] = 0;
	m_Size = size;
}

#if (BV_PLATFORM == BV_PLATFORM_WIN32 && BV_COMPILER == BV_COMPILER_MSVC)
#pragma warning(pop)
#endif


template<typename CharT>
void BvStringT<CharT>::Clear()
{
	std::char_traits<CharT>::assign(m_pStr, m_Size, 0);
	m_Size = 0;
}


template<typename CharT>
void BvStringT<CharT>::Swap(BvStringT& str)
{
	if (m_Size >= str.m_Capacity)
	{
		str.Resize(m_Capacity);
	}
	if (str.m_Size >= m_Capacity)
	{
		Resize(str.m_Capacity);
	}

	CharT* pDst, * pSrc;
	u32 min, max;
	if (m_Size > str.m_Size)
	{
		pDst = str.m_pStr;
		pSrc = m_pStr;
		min = str.m_Size;
		max = m_Size;
	}
	else
	{
		pSrc = str.m_pStr;
		pDst = m_pStr;
		min = m_Size;
		max = str.m_Size;
	}

	CharT tmp;
	u32 i = 0;
	for (; i < min; i++)
	{
		tmp = m_pStr[i];
		m_pStr[i] = str.m_pStr[i];
		str.m_pStr[i] = tmp;
	}

	for (; i < max; i++)
	{
		pDst[i] = pSrc[i];
	}
	pDst[i] = 0;
	pSrc[min] = 0;

	{
		u32 tmpSize = m_Size;
		m_Size = str.m_Size;
		str.m_Size = tmpSize;
	}
}


template<typename CharT>
void BvStringT<CharT>::Copy(BvStringT& str) const
{
	str.Assign(*this);
}


template<typename CharT>
void BvStringT<CharT>::Copy(BvStringT& str, const u32 start, const u32 count) const
{
	str.Assign(*this, start, count);
}


template<typename CharT>
BvStringT<CharT> BvStringT<CharT>::Substr(const u32 start, const u32 count) const
{
	return BvStringT(m_pStr, start, count);
}


template<typename CharT>
void BvStringT<CharT>::Split(BvVector<BvStringT>& strings, CharT delimiter, const CharT* pSymbols) const
{
	Split(strings, &delimiter, 1, pSymbols, pSymbols ? std::char_traits<CharT>::length(pSymbols) : 0);
}


template<typename CharT>
void BvStringT<CharT>::Split(BvVector<BvStringT>& strings, const CharT* pDelimiters, const CharT* pSymbols) const
{
	Split(strings, pDelimiters, std::char_traits<CharT>::length(pDelimiters), pSymbols, pSymbols ? std::char_traits<CharT>::length(pSymbols) : 0);
}


template<typename CharT>
void BvStringT<CharT>::Split(BvVector<BvStringT>& result, const CharT* pDelimiters, u32 delimeterCount, const CharT* pSymbols, u32 symbolCount) const
{
	auto startIndex = 0u;
	bool symbolFound = false;
	for (auto i = 0u; i < m_Size; i++)
	{
		symbolFound = false;
		for (auto symbolIndex = 0; symbolIndex < symbolCount; symbolIndex++)
		{
			if (m_pStr[i] == pSymbols[symbolIndex])
			{
				auto count = i - startIndex;
				if (count > 0)
				{
					result.EmplaceBack(std::move(Substr(startIndex, count)));
				}
				result.EmplaceBack(m_pStr[i]);
				startIndex = i + 1;

				break;
			}
		}
		if (symbolFound)
		{
			continue;
		}

		for (auto delimeterIndex = 0; delimeterIndex < delimeterCount; delimeterIndex++)
		{
			if (m_pStr[i] == pDelimiters[delimeterIndex])
			{
				auto count = i - startIndex;
				if (count > 0)
				{
					result.EmplaceBack(std::move(Substr(startIndex, count)));
				}
				startIndex = i + 1;

				break;
			}
		}
	}

	if (startIndex < m_Size)
	{
		result.EmplaceBack(std::move(Substr(startIndex, m_Size - startIndex)));
	}
}


template<typename CharT>
bool BvStringT<CharT>::StartsWith(const CharT c, const u32 offset) const
{
	return StartsWith(&c, offset, 1);
}


template<typename CharT>
bool BvStringT<CharT>::StartsWith(const BvStringT& str, const u32 offset) const
{
	return StartsWith(str.CStr(), offset, str.Size());
}


template<typename CharT>
bool BvStringT<CharT>::StartsWith(const CharT* const pStr, const u32 offset) const
{
	return StartsWith(pStr, offset, static_cast<u32>(std::char_traits<CharT>::length(pStr)));
}


template<typename CharT>
bool BvStringT<CharT>::StartsWith(const CharT* const pStr, const u32 offset, const u32 size) const
{
	return Find(pStr, offset, size) == offset;
}


template<typename CharT>
const u32 BvStringT<CharT>::Find(const CharT c, const u32 offset) const
{
	return Find(&c, offset, 1);
}


template<typename CharT>
const u32 BvStringT<CharT>::Find(const BvStringT& str, const u32 offset) const
{
	return Find(str.m_pStr, offset, str.m_Size);
}


template<typename CharT>
const u32 BvStringT<CharT>::Find(const CharT* const pStr, const u32 offset) const
{
	return Find(pStr, offset, static_cast<u32>(std::char_traits<CharT>::length(pStr)));
}


template<typename CharT>
const u32 BvStringT<CharT>::Find(const CharT* const pStr, const u32 offset, const u32 size) const
{
	if (size > m_Size || offset >= m_Size || m_Size - offset < size)
	{
		return kInvalidIndex;
	}

	u32 j;
	bool found;
	for (u32 i = offset; m_Size - i >= size; i++)
	{
		found = true;
		for (j = 0; j < size; j++)
		{
			if (m_pStr[j + i] != pStr[j])
			{
				found = false;
				break;
			}
		}

		if (found)
		{
			return i;
		}
	}

	return kInvalidIndex;
}


template<typename CharT>
const u32 BvStringT<CharT>::RFind(const CharT c, const u32 offset) const
{
	return RFind(&c, offset, 1);
}


template<typename CharT>
const u32 BvStringT<CharT>::RFind(const BvStringT& str, const u32 offset) const
{
	return RFind(str.m_pStr, offset, str.m_Size);
}


template<typename CharT>
const u32 BvStringT<CharT>::RFind(const CharT* const pStr, const u32 offset) const
{
	return RFind(pStr, offset, static_cast<u32>(std::char_traits<CharT>::length(pStr)));
}


template<typename CharT>
const u32 BvStringT<CharT>::RFind(const CharT* const pStr, const u32 offset, const u32 size) const
{
	if (m_Size == 0)
	{
		return kInvalidIndex;
	}

	const u32 endOffset = (offset == kInvalidIndex || offset >= m_Size) ? m_Size - 1 : offset;
	if (size > m_Size || (m_Size - endOffset < size))
	{
		return kInvalidIndex;
	}
	
	u32 j;
	bool found;
	for (u32 i = endOffset + 2 - size; i > 0; i--)
	{
		found = true;
		for (j = 0; j < size; j++)
		{
			if (m_pStr[j + i - 1] != pStr[j])
			{
				found = false;
				break;
			}
		}

		if (found)
		{
			return i - 1;
		}
	}

	return kInvalidIndex;
}


template<typename CharT>
inline const u32 BvStringT<CharT>::FindFirstOf(const CharT c, const u32 offset) const
{
	return FindFirstOf(&c, offset, 1);
}


template<typename CharT>
inline const u32 BvStringT<CharT>::FindFirstOf(const BvStringT& str, const u32 offset) const
{
	return FindFirstOf(str.m_pStr, offset, str.m_Size);
}


template<typename CharT>
inline const u32 BvStringT<CharT>::FindFirstOf(const CharT* const pStr, const u32 offset) const
{
	return FindFirstOf(pStr, offset, static_cast<u32>(std::char_traits<CharT>::length(pStr)));
}


template<typename CharT>
inline const u32 BvStringT<CharT>::FindFirstOf(const CharT* const pStr, const u32 offset, const u32 size) const
{
	if (offset >= m_Size)
	{
		return kInvalidIndex;
	}

	u32 j;
	for (u32 i = offset; i < m_Size; i++)
	{
		for (j = 0; j < size; j++)
		{
			if (m_pStr[i] == pStr[j])
			{
				return i;
			}
		}
	}

	return kInvalidIndex;
}


template<typename CharT>
inline const u32 BvStringT<CharT>::FindFirstNotOf(const CharT c, const u32 offset) const
{
	return FindFirstNotOf(&c, offset, 1);
}


template<typename CharT>
inline const u32 BvStringT<CharT>::FindFirstNotOf(const BvStringT& str, const u32 offset) const
{
	return FindFirstNotOf(str.m_pStr, offset, str.m_Size);
}


template<typename CharT>
inline const u32 BvStringT<CharT>::FindFirstNotOf(const CharT* const pStr, const u32 offset) const
{
	return FindFirstNotOf(pStr, offset, static_cast<u32>(std::char_traits<CharT>::length(pStr)));
}


template<typename CharT>
inline const u32 BvStringT<CharT>::FindFirstNotOf(const CharT* const pStr, const u32 offset, const u32 size) const
{
	if (m_Size == 0 || offset >= m_Size)
	{
		return kInvalidIndex;
	}

	u32 j;
	bool found;
	for (u32 i = offset; i < m_Size; i++)
	{
		found = false;
		for (j = 0; j < size; j++)
		{
			if (m_pStr[i] == pStr[j])
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			return i;
		}
	}

	return kInvalidIndex;
}


template<typename CharT>
const u32 BvStringT<CharT>::FindLastOf(const CharT c, const u32 offset) const
{
	return FindLastOf(&c, offset, 1);
}


template<typename CharT>
const u32 BvStringT<CharT>::FindLastOf(const BvStringT& str, const u32 offset) const
{
	return FindLastOf(str.m_pStr, offset, str.m_Size);
}


template<typename CharT>
const u32 BvStringT<CharT>::FindLastOf(const CharT* const pStr, const u32 offset) const
{
	return FindLastOf(pStr, offset, static_cast<u32>(std::char_traits<CharT>::length(pStr)));
}


template<typename CharT>
const u32 BvStringT<CharT>::FindLastOf(const CharT* const pStr, const u32 offset, const u32 size) const
{
	if (m_Size == 0)
	{
		return kInvalidIndex;
	}

	const u32 endOffset = (offset == kInvalidIndex || offset >= m_Size) ? m_Size - 1 : offset;

	bool found = true;
	u32 j;
	for (u32 i = endOffset + 1; i > size - 1; i--)
	{
		found = true;
		for (j = size; j > 0; j--)
		{
			if (m_pStr[i - 1] != pStr[j - 1])
			{
				found = false;
				break;
			}
		}

		if (found)
		{
			return i - 1;
		}
	}

	return kInvalidIndex;
}


template<typename CharT>
const u32 BvStringT<CharT>::FindLastNotOf(const CharT c, const u32 offset) const
{
	return FindLastNotOf(&c, offset, 1);
}


template<typename CharT>
const u32 BvStringT<CharT>::FindLastNotOf(const BvStringT& str, const u32 offset) const
{
	return FindLastNotOf(str.m_pStr, offset, str.m_Size);
}


template<typename CharT>
const u32 BvStringT<CharT>::FindLastNotOf(const CharT* const pStr, const u32 offset) const
{
	return FindLastNotOf(pStr, offset, static_cast<u32>(std::char_traits<CharT>::length(pStr)));
}


template<typename CharT>
const u32 BvStringT<CharT>::FindLastNotOf(const CharT* const pStr, const u32 offset, const u32 size) const
{
	if (m_Size == 0)
	{
		return kInvalidIndex;
	}

	const u32 endOffset = (offset == kInvalidIndex || offset >= m_Size) ? m_Size - 1 : offset;

	u32 j;
	bool found;
	for (u32 i = endOffset + 1; i > 0; i--)
	{
		found = false;
		for (j = 0; j < size; j++)
		{
			if (m_pStr[i - 1] == pStr[j])
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			return i - 1;
		}
	}

	return kInvalidIndex;
}


template<typename CharT>
const u64 BvStringT<CharT>::Hash() const
{
	return FNV1a64(m_pStr, m_Size);
}


template<typename CharT>
BvStringT<CharT> BvStringT<CharT>::operator +(const BvStringT& str)
{
	BvStringT newStr = *this;
	newStr.Append(str);

	return newStr;
}


template<typename CharT>
BvStringT<CharT> BvStringT<CharT>::operator +(const CharT c)
{
	BvStringT newStr = *this;
	newStr.Append(c);

	return newStr;
}


template<typename CharT>
BvStringT<CharT> BvStringT<CharT>::operator +(const CharT* const pStr)
{
	BvStringT newStr = *this;
	newStr.Append(pStr);

	return newStr;
}


template<typename CharT>
BvStringT<CharT> operator +(const CharT* const pStr, const BvStringT<CharT>& str)
{
	BvStringT newStr(pStr);
	newStr.Append(str);

	return newStr;
}


template<typename CharT>
BvStringT<CharT> operator+(const CharT c, const BvStringT<CharT>& str)
{
	BvStringT newStr(str);
	newStr.Insert(c);

	return newStr;
}


template<typename CharT>
const i32 BvStringT<CharT>::Compare(const CharT* const pStr, size_t size) const
{
	std::basic_string_view view1(m_pStr, m_Size), view2(pStr, size);

	return view1.compare(view2);
}


template<typename CharT>
inline void BvStringT<CharT>::Destroy()
{
	if (m_pStr)
	{
		delete[] m_pStr;
		m_pStr = nullptr;
		m_Size = 0;
		m_Capacity = 0;
	}
}


// Typedefs for convenience
using BvString = BvStringT<char>;
using BvWString = BvStringT<wchar_t>;
using BvString8 = BvStringT<char8_t>;
using BvString16 = BvStringT<char16_t>;
using BvString32 = BvStringT<char32_t>;