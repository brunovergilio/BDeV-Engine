#pragma once


#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Utils/BvHash.h"
#include "BDeV/Core/Container/BvVector.h"
#include <cstdarg>
#include <string_view>
#include <locale>


namespace Internal
{
	template<typename CharT>
	struct IsCharType
	{
		static constexpr bool value = false;
	};


#define BV_IS_CHAR_TYPE(type) template<> struct IsCharType<type> { static constexpr bool value = true; };

	BV_IS_CHAR_TYPE(char);
	BV_IS_CHAR_TYPE(wchar_t);
	BV_IS_CHAR_TYPE(char8_t);
	BV_IS_CHAR_TYPE(char16_t);
	BV_IS_CHAR_TYPE(char32_t);
#undef BV_IS_CHAR_TYPE

	template<typename CharT>
	inline constexpr bool IsCharTypeV = IsCharType<CharT>::value;
}


template<typename CharT, typename MemoryArenaType = IBvMemoryArena>
class BvStringT
{
public:
	static_assert(Internal::IsCharTypeV<CharT>, "Not a valid string character type");

	static constexpr u32 kInvalidIndex = kU32Max;

	BvStringT();
	BvStringT(MemoryArenaType* pAllocator);
	explicit BvStringT(const u32 size, const CharT c = CharT(), MemoryArenaType* pAllocator = nullptr);
	explicit BvStringT(const BvStringT & str, const u32 start, const u32 count, MemoryArenaType* pAllocator = nullptr);
	BvStringT(const CharT * const pStr, MemoryArenaType* pAllocator = nullptr);
	explicit BvStringT(const CharT * const pStr, const u32 start, const u32 count, MemoryArenaType* pAllocator = nullptr);
	BvStringT(const CharT c, MemoryArenaType* pAllocator = nullptr);
	BvStringT(const BvStringT & rhs);
	BvStringT(BvStringT && rhs) noexcept;

	BvStringT& operator =(const BvStringT & rhs);
	BvStringT& operator =(BvStringT && rhs) noexcept;
	BvStringT& operator =(const CharT * const pStr);
	BvStringT& operator =(const CharT c);

	~BvStringT();

	// Allocator
	MemoryArenaType* GetAllocator() const;
	void SetAllocator(MemoryArenaType* pAllocator);

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

	void Replace(const BvStringT & srcStr, const BvStringT& dstStr);
	void Replace(const CharT * const pSrcStr, const CharT * const pDstStr);
	void Replace(const CharT * const pSrcStr, const u32 srcSize, const CharT * const pDstStr, const u32 dstSize);

	const u32 Read(BvStringT& dstStr, const CharT delim, const u32 startIndex = 0) const;
	const u32 Read(BvStringT& dstStr, const CharT* const pDelim, const u32 size, const u32 startIndex = 0) const;
	void Format(const CharT* format, ...);

	void Erase(const u32 start, const u32 count);

	void Resize(u32 size, CharT c = CharT());
	void Clear();

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

	u32 Find(const CharT c, const u32 offset = 0) const;
	u32 Find(const BvStringT & str, const u32 offset = 0) const;
	u32 Find(const CharT * const pStr, const u32 offset = 0) const;
	u32 Find(const CharT * const pStr, const u32 offset, const u32 size) const;

	u32 RFind(const CharT c, const u32 offset = kInvalidIndex) const;
	u32 RFind(const BvStringT & str, const u32 offset = kInvalidIndex) const;
	u32 RFind(const CharT * const pStr, const u32 offset = kInvalidIndex) const;
	u32 RFind(const CharT * const pStr, const u32 size, const u32 offset) const;

	u32 FindFirstOf(const CharT c, const u32 offset = 0) const;
	u32 FindFirstOf(const BvStringT& str, const u32 offset = 0) const;
	u32 FindFirstOf(const CharT* const pStr, const u32 offset = 0) const;
	u32 FindFirstOf(const CharT* const pStr, const u32 offset, const u32 size) const;

	u32 FindFirstNotOf(const CharT c, const u32 offset = 0) const;
	u32 FindFirstNotOf(const BvStringT& str, const u32 offset = 0) const;
	u32 FindFirstNotOf(const CharT* const pStr, const u32 offset = 0) const;
	u32 FindFirstNotOf(const CharT* const pStr, const u32 offset, const u32 size) const;

	u32 FindLastOf(const CharT c, const u32 offset = kInvalidIndex) const;
	u32 FindLastOf(const BvStringT& str, const u32 offset = kInvalidIndex) const;
	u32 FindLastOf(const CharT* const pStr, const u32 offset = kInvalidIndex) const;
	u32 FindLastOf(const CharT* const pStr, const u32 offset, const u32 size) const;

	u32 FindLastNotOf(const CharT c, const u32 offset = kInvalidIndex) const;
	u32 FindLastNotOf(const BvStringT& str, const u32 offset = kInvalidIndex) const;
	u32 FindLastNotOf(const CharT* const pStr, const u32 offset = kInvalidIndex) const;
	u32 FindLastNotOf(const CharT* const pStr, const u32 offset, const u32 size) const;

	bool StartsWith(CharT c) const;
	bool StartsWith(const BvStringT& str) const;
	bool StartsWith(const CharT* pStr) const;

	bool EndsWith(CharT c) const;
	bool EndsWith(const BvStringT& str) const;
	bool EndsWith(const CharT* pStr) const;

	u64 Hash() const;

	BV_INLINE bool Contains(const CharT c) const { return Find(c) != kInvalidIndex; }
	BV_INLINE bool Contains(const BvStringT & str) const { return Find(str) != kInvalidIndex; }
	BV_INLINE bool Contains(const CharT * const pStr) const { return Find(pStr) != kInvalidIndex; }

	BvStringT & operator +=(const BvStringT & str) { Append(str); return *this; }
	BvStringT & operator +=(const CharT * const pStr) { Append(pStr); return *this; }
	BvStringT & operator +=(const CharT c) { Append(c); return *this; }

	BvStringT operator +(const BvStringT & str);
	BvStringT operator +(const CharT * const pStr);
	BvStringT operator +(const CharT c);
	friend BvStringT operator +(const CharT * const pStr, const BvStringT & str);
	friend BvStringT operator +(const CharT c, const BvStringT & str);

	BV_INLINE const CharT & operator[](const u32 index) const { BV_ASSERT(index < m_Size, "Index out of bounds"); return m_pStr[index]; }
	BV_INLINE CharT & operator[](const u32 index) { BV_ASSERT(index < m_Size, "Index out of bounds"); return m_pStr[index]; }
	BV_INLINE const CharT At(const u32 index) const { BV_ASSERT(index < m_Size, "Index out of bounds"); return m_pStr[index]; }
	BV_INLINE const CharT Front() const { BV_ASSERT(m_Size > 0, "Index out of bounds"); return m_pStr[0]; }
	BV_INLINE const CharT Back() const { BV_ASSERT(m_Size > 0, "Index out of bounds"); return m_pStr[m_Size - 1]; }

	BV_INLINE const CharT* CStr() const { return m_pStr; }
	BV_INLINE const CharT* LCStr() const { return m_pStr ? m_pStr + m_Size : nullptr; }
	BV_INLINE u32 Size() const { return m_Size; }
	BV_INLINE u32 Capacity() const { return m_Capacity; }
	BV_INLINE bool Empty() const { return m_Size == 0; }
	BV_INLINE CharT GetFirstChar() const { return m_Size > 0 ? m_pStr[0] : CharT(); }
	BV_INLINE CharT GetLastChar() const { return m_Size > 0 ? m_pStr[m_Size - 1] : CharT(); }

	BV_INLINE operator const CharT * const() { return m_pStr; }

	BV_INLINE i32 Compare(const BvStringT & str) const { return Compare(str.m_pStr, str.m_Size); }
	BV_INLINE i32 Compare(const CharT* const pStr, u32 size) const;

	BV_INLINE i32 CompareNoCase(const BvStringT& str) const { return CompareNoCase(str.m_pStr, str.m_Size); }
	BV_INLINE i32 CompareNoCase(const CharT* const pStr, u32 size) const;
	 
	BV_INLINE bool operator ==(CharT c) const { return m_Size == 1 && c == m_pStr[0]; }
	BV_INLINE bool operator ==(const BvStringT & str) const { return Compare(str) == 0; }
	BV_INLINE bool operator ==(const CharT * const pStr) const { return Compare(pStr, std::char_traits<CharT>::length(pStr)) == 0; }
	 
	BV_INLINE bool operator !=(CharT c) const { return !(*this == c); }
	BV_INLINE bool operator !=(const BvStringT& str) const { return Compare(str) != 0; }
	BV_INLINE bool operator !=(const CharT* const pStr) const { return Compare(pStr, std::char_traits<CharT>::length(pStr)) != 0; }
	 
	BV_INLINE bool operator <(const BvStringT & str) const { return Compare(str) < 0; }
	BV_INLINE bool operator <(const CharT * const pStr) const { return Compare(pStr, std::char_traits<CharT>::length(pStr)) < 0; }
	 
	BV_INLINE bool operator <=(const BvStringT & str) const { return Compare(str) <= 0; }
	BV_INLINE bool operator <=(const CharT * const pStr) const { return Compare(pStr, std::char_traits<CharT>::length(pStr)) <= 0; }
	 
	BV_INLINE bool operator >(const BvStringT & str) const { return Compare(str) > 0; }
	BV_INLINE bool operator >(const CharT * const pStr) const { return Compare(pStr, std::char_traits<CharT>::length(pStr)) > 0; }
	 
	BV_INLINE bool operator >=(const BvStringT & str) const { return Compare(str) >= 0; }
	BV_INLINE bool operator >=(const CharT * const pStr) const { return Compare(pStr, std::char_traits<CharT>::length(pStr)) >= 0; }

private:
	void Grow(u32 size);
	void Destroy();

protected:
	CharT * m_pStr = nullptr;
	MemoryArenaType* m_pAllocator = nullptr;
	u32 m_Size = 0;
	u32 m_Capacity = 0;
};


template<typename CharT, typename MemoryArenaType>
struct std::hash<BvStringT<CharT, MemoryArenaType>>
{
	u64 operator()(const BvStringT<CharT, MemoryArenaType>& val)
	{
		return val.Hash();
	}
};


template<typename CharT, typename MemoryArenaType>
BvStringT<CharT, MemoryArenaType>::BvStringT()
{
}


template<typename CharT, typename MemoryArenaType>
inline BvStringT<CharT, MemoryArenaType>::BvStringT(MemoryArenaType* pAllocator)
	: m_pAllocator(pAllocator)
{
}


template<typename CharT, typename MemoryArenaType>
BvStringT<CharT, MemoryArenaType>::BvStringT(const u32 size, const CharT c, MemoryArenaType* pAllocator)
	: m_pAllocator(pAllocator)
{
	Resize(size, c);
}


template<typename CharT, typename MemoryArenaType>
BvStringT<CharT, MemoryArenaType>::BvStringT(const BvStringT& str, const u32 start, const u32 count, MemoryArenaType* pAllocator)
	: m_pAllocator(pAllocator)
{
	Assign(str, start, count);
}


template<typename CharT, typename MemoryArenaType>
BvStringT<CharT, MemoryArenaType>::BvStringT(const CharT* const pStr, MemoryArenaType* pAllocator)
	: m_pAllocator(pAllocator)
{
	Assign(pStr);
}


template<typename CharT, typename MemoryArenaType>
BvStringT<CharT, MemoryArenaType>::BvStringT(const CharT* const pStr, const u32 start, const u32 count, MemoryArenaType* pAllocator)
	: m_pAllocator(pAllocator)
{
	Assign(pStr, start, count);
}


template<typename CharT, typename MemoryArenaType>
BvStringT<CharT, MemoryArenaType>::BvStringT(const CharT c, MemoryArenaType* pAllocator)
	: m_pAllocator(pAllocator)
{
	Assign(c);
}


template<typename CharT, typename MemoryArenaType>
BvStringT<CharT, MemoryArenaType>::BvStringT(const BvStringT<CharT, MemoryArenaType>& rhs)
	: m_pAllocator(rhs.m_pAllocator)
{
	Assign(rhs);
}


template<typename CharT, typename MemoryArenaType>
BvStringT<CharT, MemoryArenaType>::BvStringT(BvStringT&& rhs) noexcept
{
	*this = std::move(rhs);
}


template<typename CharT, typename MemoryArenaType>
BvStringT<CharT, MemoryArenaType>& BvStringT<CharT, MemoryArenaType>::operator=(const BvStringT& rhs)
{
	if (this != &rhs)
	{
		Destroy();

		Assign(rhs);
	}

	return *this;
}


template<typename CharT, typename MemoryArenaType>
BvStringT<CharT, MemoryArenaType>& BvStringT<CharT, MemoryArenaType>::operator=(BvStringT&& rhs) noexcept
{
	if (this != &rhs)
	{
		std::swap(m_pStr, rhs.m_pStr);
		std::swap(m_Size, rhs.m_Size);
		std::swap(m_Capacity, rhs.m_Capacity);
		std::swap(m_pAllocator, rhs.m_pAllocator);
	}

	return *this;
}


template<typename CharT, typename MemoryArenaType>
BvStringT<CharT, MemoryArenaType>& BvStringT<CharT, MemoryArenaType>::operator=(const CharT* const pStr)
{
	if (m_pStr != pStr)
	{
		Destroy();

		Assign(pStr);
	}

	return *this;
}

template<typename CharT, typename MemoryArenaType>
BvStringT<CharT, MemoryArenaType>& BvStringT<CharT, MemoryArenaType>::operator=(const CharT c)
{
	Destroy();

	Assign(c);

	return *this;
}


template<typename CharT, typename MemoryArenaType>
BvStringT<CharT, MemoryArenaType>::~BvStringT()
{
	Destroy();
}


template<typename CharT, typename MemoryArenaType>
MemoryArenaType* BvStringT<CharT, MemoryArenaType>::GetAllocator() const
{
	return m_pAllocator;
}


template<typename CharT, typename MemoryArenaType>
void BvStringT<CharT, MemoryArenaType>::SetAllocator(MemoryArenaType* pAllocator)
{
	if (m_pAllocator == pAllocator)
	{
		return;
	}

	if (m_Capacity > 0)
	{
		CharT* pNewStr = reinterpret_cast<CharT*>(pAllocator ? BV_MALLOC(*pAllocator, sizeof(CharT) * m_Capacity, alignof(CharT)) : BV_ALLOC(sizeof(CharT) * m_Capacity, alignof(CharT)));
		if (m_Size > 0)
		{
			std::char_traits<CharT>::copy(pNewStr, m_pStr, m_Size);
			pNewStr[m_Size] = CharT();
		}
		m_pAllocator ? BV_MFREE(*m_pAllocator, m_pStr) : BV_FREE(m_pStr);
		m_pStr = pNewStr;
	}

	m_pAllocator = pAllocator;
}


template<typename CharT, typename MemoryArenaType>
void BvStringT<CharT, MemoryArenaType>::Assign(const BvStringT& str)
{
	Assign(str.m_pStr, 0, str.m_Size);
}


template<typename CharT, typename MemoryArenaType>
void BvStringT<CharT, MemoryArenaType>::Assign(const CharT* const pStr)
{
	Assign(pStr, 0, pStr ? static_cast<u32>(std::char_traits<CharT>::length(pStr)) : 0);
}


template<typename CharT, typename MemoryArenaType>
void BvStringT<CharT, MemoryArenaType>::Assign(const CharT c)
{
	Assign(&c, 0, 1);
}


template<typename CharT, typename MemoryArenaType>
void BvStringT<CharT, MemoryArenaType>::Assign(const BvStringT& str, const u32 start, const u32 count)
{
	Assign(str.m_pStr, start, count);
}


template<typename CharT, typename MemoryArenaType>
void BvStringT<CharT, MemoryArenaType>::Assign(const CharT* const pStr, const u32 start, const u32 count)
{
	if (!pStr)
	{
		return;
	}

	BV_ASSERT(m_pStr != pStr, "Same underlying string pointer");

	if (m_Capacity <= count)
	{
		Grow(count);
	}

	std::char_traits<CharT>::copy(m_pStr, pStr + start, count);
	m_pStr[count] = CharT();
	m_Size = count;
}


template<typename CharT, typename MemoryArenaType>
void BvStringT<CharT, MemoryArenaType>::Insert(const BvStringT& str, const u32 where)
{
	Insert(str.m_pStr, 0, str.m_Size, where);
}


template<typename CharT, typename MemoryArenaType>
void BvStringT<CharT, MemoryArenaType>::Insert(const CharT* const pStr, const u32 where)
{
	Insert(pStr, 0, static_cast<u32>(std::char_traits<CharT>::length(pStr)), where);
}


template<typename CharT, typename MemoryArenaType>
void BvStringT<CharT, MemoryArenaType>::Insert(const CharT c, const u32 where)
{
	Insert(&c, 0, 1, where);
}


template<typename CharT, typename MemoryArenaType>
void BvStringT<CharT, MemoryArenaType>::Insert(const BvStringT& str, const u32 start, const u32 count, const u32 where)
{
	Insert(str.m_pStr, start, count, where);
}


template<typename CharT, typename MemoryArenaType>
void BvStringT<CharT, MemoryArenaType>::Insert(const CharT* const pStr, const u32 start, const u32 count, const u32 where)
{
	BV_ASSERT(m_Size >= where, "Position past the string's size");
	if (count == 0)
	{
		return;
	}

	u32 oldSize = m_Size;
	u32 newSize = count + m_Size;
	if (m_Capacity <= newSize)
	{
		u32 tmp = newSize;
		u32 newCapacity = tmp + (tmp >> 1) + (tmp <= 2);
		Grow(newCapacity);
	}
	m_pStr[newSize] = CharT();

	if (oldSize - where > 0)
	{
		std::char_traits<CharT>::move(m_pStr + where + count, m_pStr + where, oldSize - where);
	}
	std::char_traits<CharT>::copy(m_pStr + where, pStr + start, count);
	m_Size = newSize;
}


template<typename CharT, typename MemoryArenaType>
void BvStringT<CharT, MemoryArenaType>::Replace(const BvStringT& srcStr, const BvStringT& dstStr)
{
	Replace(srcStr.m_pStr, srcStr.m_Size, dstStr.m_pStr, dstStr.m_Size);
}


template<typename CharT, typename MemoryArenaType>
void BvStringT<CharT, MemoryArenaType>::Replace(const CharT* const pSrcStr, const CharT* const pDstStr)
{
	Replace(pSrcStr, static_cast<u32>(std::char_traits<CharT>::length(pSrcStr)), pDstStr,
		static_cast<u32>(std::char_traits<CharT>::length(pDstStr)));
}


template<typename CharT, typename MemoryArenaType>
void BvStringT<CharT, MemoryArenaType>::Replace(const CharT* const pSrcStr, const u32 srcSize, const CharT* const pDstStr, const u32 dstSize)
{
	u32 index = Find(pSrcStr, srcSize);
	if (index == kInvalidIndex)
	{
		return;
	}

	Erase(index, srcSize);
	Insert(pDstStr, 0, dstSize, index);
}


template<typename CharT, typename MemoryArenaType>
inline const u32 BvStringT<CharT, MemoryArenaType>::Read(BvStringT& dstStr, const CharT delim, const u32 offset) const
{
	return Read(dstStr, &delim, 1, offset);
}


template<typename CharT, typename MemoryArenaType>
inline const u32 BvStringT<CharT, MemoryArenaType>::Read(BvStringT& dstStr, const CharT* const pDelim, const u32 size, const u32 offset) const
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
			if (m_pStr[i] == pDelim[j])
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


template<typename CharT, typename MemoryArenaType>
void BvStringT<CharT, MemoryArenaType>::Format(const CharT* format, ...)
{
	u32 size = 0;
	{
		va_list args;
		va_start(args, format);
		va_list argsCopy;
		va_copy(argsCopy, args);
		size = vsnprintf(nullptr, 0, format, argsCopy) + 1;
		va_end(argsCopy);
		va_end(args);

		if (size == 1)
		{
			return;
		}
	}

	if (size >= m_Capacity)
	{
		Resize(size);
	}

	va_list args;
	va_start(args, format);
	vsnprintf(m_pStr, size, format, args);
	va_end(args);

	m_pStr[size] = CharT();
}


template<typename CharT, typename MemoryArenaType>
void BvStringT<CharT, MemoryArenaType>::Erase(const u32 start, const u32 count)
{
	BV_ASSERT(start < m_Size && count > 0, "Erasing past the string's size");
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
	m_pStr[m_Size] = CharT();
}


#if (BV_PLATFORM == BV_PLATFORM_WIN32 && BV_COMPILER == BV_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable:6386) // Buffer overrun
#endif

template<typename CharT, typename MemoryArenaType>
void BvStringT<CharT, MemoryArenaType>::Resize(u32 size, CharT c)
{
	if (size > m_Size)
	{
		Grow(size);

		if (c != CharT())
		{
			for (auto i = m_Size; i < size; i++)
			{
				m_pStr[i] = c;
			}
			m_pStr[size] = CharT();
			m_Size = size;
		}
	}
	else if (size < m_Size)
	{
		m_pStr[size] = CharT();
		m_Size = size;
	}
}

#if (BV_PLATFORM == BV_PLATFORM_WIN32 && BV_COMPILER == BV_COMPILER_MSVC)
#pragma warning(pop)
#endif


template<typename CharT, typename MemoryArenaType>
void BvStringT<CharT, MemoryArenaType>::Clear()
{
	m_pStr[0] = CharT();
	m_Size = 0;
}


template<typename CharT, typename MemoryArenaType>
void BvStringT<CharT, MemoryArenaType>::Copy(BvStringT& str) const
{
	str.Assign(*this);
}


template<typename CharT, typename MemoryArenaType>
void BvStringT<CharT, MemoryArenaType>::Copy(BvStringT& str, const u32 start, const u32 count) const
{
	str.Assign(*this, start, count);
}


template<typename CharT, typename MemoryArenaType>
BvStringT<CharT, MemoryArenaType> BvStringT<CharT, MemoryArenaType>::Substr(const u32 start, const u32 count) const
{
	return BvStringT(m_pStr, start, count);
}


template<typename CharT, typename MemoryArenaType>
void BvStringT<CharT, MemoryArenaType>::Split(BvVector<BvStringT>& strings, CharT delimiter, const CharT* pSymbols) const
{
	Split(strings, &delimiter, 1, pSymbols, pSymbols ? std::char_traits<CharT>::length(pSymbols) : 0);
}


template<typename CharT, typename MemoryArenaType>
void BvStringT<CharT, MemoryArenaType>::Split(BvVector<BvStringT>& strings, const CharT* pDelimiters, const CharT* pSymbols) const
{
	Split(strings, pDelimiters, std::char_traits<CharT>::length(pDelimiters), pSymbols, pSymbols ? std::char_traits<CharT>::length(pSymbols) : 0);
}


template<typename CharT, typename MemoryArenaType>
void BvStringT<CharT, MemoryArenaType>::Split(BvVector<BvStringT>& result, const CharT* pDelimiters, u32 delimeterCount, const CharT* pSymbols, u32 symbolCount) const
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


template<typename CharT, typename MemoryArenaType>
bool BvStringT<CharT, MemoryArenaType>::StartsWith(const CharT c, const u32 offset) const
{
	return StartsWith(&c, offset, 1);
}


template<typename CharT, typename MemoryArenaType>
bool BvStringT<CharT, MemoryArenaType>::StartsWith(const BvStringT& str, const u32 offset) const
{
	return StartsWith(str.CStr(), offset, str.Size());
}


template<typename CharT, typename MemoryArenaType>
bool BvStringT<CharT, MemoryArenaType>::StartsWith(const CharT* const pStr, const u32 offset) const
{
	return StartsWith(pStr, offset, static_cast<u32>(std::char_traits<CharT>::length(pStr)));
}


template<typename CharT, typename MemoryArenaType>
bool BvStringT<CharT, MemoryArenaType>::StartsWith(const CharT* const pStr, const u32 offset, const u32 size) const
{
	return Find(pStr, offset, size) == offset;
}


template<typename CharT, typename MemoryArenaType>
u32 BvStringT<CharT, MemoryArenaType>::Find(const CharT c, const u32 offset) const
{
	return Find(&c, offset, 1);
}


template<typename CharT, typename MemoryArenaType>
u32 BvStringT<CharT, MemoryArenaType>::Find(const BvStringT& str, const u32 offset) const
{
	return Find(str.m_pStr, offset, str.m_Size);
}


template<typename CharT, typename MemoryArenaType>
u32 BvStringT<CharT, MemoryArenaType>::Find(const CharT* const pStr, const u32 offset) const
{
	return Find(pStr, offset, static_cast<u32>(std::char_traits<CharT>::length(pStr)));
}


template<typename CharT, typename MemoryArenaType>
u32 BvStringT<CharT, MemoryArenaType>::Find(const CharT* const pStr, const u32 offset, const u32 size) const
{
	std::basic_string_view<CharT> view(m_pStr, m_Size);
	auto index = view.find(pStr, offset, size);
	if (index == std::basic_string_view<CharT>::npos)
	{
		return kInvalidIndex;
	}
	return u32(index);
}


template<typename CharT, typename MemoryArenaType>
u32 BvStringT<CharT, MemoryArenaType>::RFind(const CharT c, const u32 offset) const
{
	return RFind(&c, offset, 1);
}


template<typename CharT, typename MemoryArenaType>
u32 BvStringT<CharT, MemoryArenaType>::RFind(const BvStringT& str, const u32 offset) const
{
	return RFind(str.m_pStr, offset, str.m_Size);
}


template<typename CharT, typename MemoryArenaType>
u32 BvStringT<CharT, MemoryArenaType>::RFind(const CharT* const pStr, const u32 offset) const
{
	return RFind(pStr, offset, static_cast<u32>(std::char_traits<CharT>::length(pStr)));
}


template<typename CharT, typename MemoryArenaType>
u32 BvStringT<CharT, MemoryArenaType>::RFind(const CharT* const pStr, const u32 offset, const u32 size) const
{
	std::basic_string_view<CharT> view(m_pStr, m_Size);
	auto index = view.rfind(pStr, offset, size);
	if (index == std::basic_string_view<CharT>::npos)
	{
		return kInvalidIndex;
	}
	return u32(index);
}


template<typename CharT, typename MemoryArenaType>
u32 BvStringT<CharT, MemoryArenaType>::FindFirstOf(const CharT c, const u32 offset) const
{
	return FindFirstOf(&c, offset, 1);
}


template<typename CharT, typename MemoryArenaType>
u32 BvStringT<CharT, MemoryArenaType>::FindFirstOf(const BvStringT& str, const u32 offset) const
{
	return FindFirstOf(str.m_pStr, offset, str.m_Size);
}


template<typename CharT, typename MemoryArenaType>
u32 BvStringT<CharT, MemoryArenaType>::FindFirstOf(const CharT* const pStr, const u32 offset) const
{
	return FindFirstOf(pStr, offset, static_cast<u32>(std::char_traits<CharT>::length(pStr)));
}


template<typename CharT, typename MemoryArenaType>
u32 BvStringT<CharT, MemoryArenaType>::FindFirstOf(const CharT* const pStr, const u32 offset, const u32 size) const
{
	std::basic_string_view<CharT> view(m_pStr, m_Size);
	auto index = view.find_first_of(pStr, offset, size);
	if (index == std::basic_string_view<CharT>::npos)
	{
		return kInvalidIndex;
	}
	return u32(index);
}


template<typename CharT, typename MemoryArenaType>
u32 BvStringT<CharT, MemoryArenaType>::FindFirstNotOf(const CharT c, const u32 offset) const
{
	return FindFirstNotOf(&c, offset, 1);
}


template<typename CharT, typename MemoryArenaType>
u32 BvStringT<CharT, MemoryArenaType>::FindFirstNotOf(const BvStringT& str, const u32 offset) const
{
	return FindFirstNotOf(str.m_pStr, offset, str.m_Size);
}


template<typename CharT, typename MemoryArenaType>
u32 BvStringT<CharT, MemoryArenaType>::FindFirstNotOf(const CharT* const pStr, const u32 offset) const
{
	return FindFirstNotOf(pStr, offset, static_cast<u32>(std::char_traits<CharT>::length(pStr)));
}


template<typename CharT, typename MemoryArenaType>
u32 BvStringT<CharT, MemoryArenaType>::FindFirstNotOf(const CharT* const pStr, const u32 offset, const u32 size) const
{
	std::basic_string_view<CharT> view(m_pStr, m_Size);
	auto index = view.find_first_not_of(pStr, offset, size);
	if (index == std::basic_string_view<CharT>::npos)
	{
		return kInvalidIndex;
	}
	return u32(index);
}


template<typename CharT, typename MemoryArenaType>
u32 BvStringT<CharT, MemoryArenaType>::FindLastOf(const CharT c, const u32 offset) const
{
	return FindLastOf(&c, offset, 1);
}


template<typename CharT, typename MemoryArenaType>
u32 BvStringT<CharT, MemoryArenaType>::FindLastOf(const BvStringT& str, const u32 offset) const
{
	return FindLastOf(str.m_pStr, offset, str.m_Size);
}


template<typename CharT, typename MemoryArenaType>
u32 BvStringT<CharT, MemoryArenaType>::FindLastOf(const CharT* const pStr, const u32 offset) const
{
	return FindLastOf(pStr, offset, static_cast<u32>(std::char_traits<CharT>::length(pStr)));
}


template<typename CharT, typename MemoryArenaType>
u32 BvStringT<CharT, MemoryArenaType>::FindLastOf(const CharT* const pStr, const u32 offset, const u32 size) const
{
	std::basic_string_view<CharT> view(m_pStr, m_Size);
	auto index = view.find_last_of(pStr, offset, size);
	if (index == std::basic_string_view<CharT>::npos)
	{
		return kInvalidIndex;
	}
	return u32(index);
}


template<typename CharT, typename MemoryArenaType>
u32 BvStringT<CharT, MemoryArenaType>::FindLastNotOf(const CharT c, const u32 offset) const
{
	return FindLastNotOf(&c, offset, 1);
}


template<typename CharT, typename MemoryArenaType>
u32 BvStringT<CharT, MemoryArenaType>::FindLastNotOf(const BvStringT& str, const u32 offset) const
{
	return FindLastNotOf(str.m_pStr, offset, str.m_Size);
}


template<typename CharT, typename MemoryArenaType>
u32 BvStringT<CharT, MemoryArenaType>::FindLastNotOf(const CharT* const pStr, const u32 offset) const
{
	return FindLastNotOf(pStr, offset, static_cast<u32>(std::char_traits<CharT>::length(pStr)));
}


template<typename CharT, typename MemoryArenaType>
u32 BvStringT<CharT, MemoryArenaType>::FindLastNotOf(const CharT* const pStr, const u32 offset, const u32 size) const
{
	std::basic_string_view<CharT> view(m_pStr, m_Size);
	auto index = view.find_last_not_of(pStr, offset, size);
	if (index == std::basic_string_view<CharT>::npos)
	{
		return kInvalidIndex;
	}
	return u32(index);
}

template<typename CharT, typename MemoryArenaType>
bool BvStringT<CharT, MemoryArenaType>::StartsWith(CharT c) const
{
	std::basic_string_view<CharT> view(m_pStr, m_Size);
	return view.starts_with(c);
}

template<typename CharT, typename MemoryArenaType>
bool BvStringT<CharT, MemoryArenaType>::StartsWith(const BvStringT& str) const
{
	std::basic_string_view<CharT> view(m_pStr, m_Size);
	return view.starts_with(str.m_pStr);
}

template<typename CharT, typename MemoryArenaType>
bool BvStringT<CharT, MemoryArenaType>::StartsWith(const CharT* pStr) const
{
	std::basic_string_view<CharT> view(m_pStr, m_Size);
	return view.starts_with(pStr);
}

template<typename CharT, typename MemoryArenaType>
bool BvStringT<CharT, MemoryArenaType>::EndsWith(CharT c) const
{
	std::basic_string_view<CharT> view(m_pStr, m_Size);
	return view.ends_with(c);
}

template<typename CharT, typename MemoryArenaType>
bool BvStringT<CharT, MemoryArenaType>::EndsWith(const BvStringT& str) const
{
	std::basic_string_view<CharT> view(m_pStr, m_Size);
	return view.ends_with(str.m_pStr);
}

template<typename CharT, typename MemoryArenaType>
inline bool BvStringT<CharT, MemoryArenaType>::EndsWith(const CharT* pStr) const
{
	std::basic_string_view<CharT> view(m_pStr, m_Size);
	return view.ends_with(pStr);
}


template<typename CharT, typename MemoryArenaType>
u64 BvStringT<CharT, MemoryArenaType>::Hash() const
{
	return m_Size > 0 ? MurmurHash64A(m_pStr, m_Size) : 0;
}


template<typename CharT, typename MemoryArenaType>
BvStringT<CharT, MemoryArenaType> BvStringT<CharT, MemoryArenaType>::operator +(const BvStringT& str)
{
	BvStringT newStr = *this;
	newStr.Append(str);

	return newStr;
}


template<typename CharT, typename MemoryArenaType>
BvStringT<CharT, MemoryArenaType> BvStringT<CharT, MemoryArenaType>::operator +(const CharT c)
{
	BvStringT newStr = *this;
	newStr.Append(c);

	return newStr;
}


template<typename CharT, typename MemoryArenaType>
BvStringT<CharT, MemoryArenaType> BvStringT<CharT, MemoryArenaType>::operator +(const CharT* const pStr)
{
	BvStringT newStr = *this;
	newStr.Append(pStr);

	return newStr;
}


template<typename CharT, typename MemoryArenaType>
BvStringT<CharT, MemoryArenaType> operator +(const CharT* const pStr, const BvStringT<CharT, MemoryArenaType>& str)
{
	BvStringT newStr(pStr);
	newStr.Append(str);

	return newStr;
}


template<typename CharT, typename MemoryArenaType>
BvStringT<CharT, MemoryArenaType> operator+(const CharT c, const BvStringT<CharT, MemoryArenaType>& str)
{
	BvStringT newStr(str);
	newStr.Insert(c);

	return newStr;
}

template<typename CharT, typename MemoryArenaType>
i32 BvStringT<CharT, MemoryArenaType>::Compare(const CharT* const pStr, u32 size) const
{
	std::basic_string_view view1(m_pStr, m_Size), view2(pStr, size);

	return view1.compare(view2);
}


template<typename CharT, typename MemoryArenaType>
i32 BvStringT<CharT, MemoryArenaType>::CompareNoCase(const CharT* const pStr, u32 size) const
{
	auto pStr1 = m_pStr, pStr2 = pStr;
	auto count = std::min(m_Size, size);
	std::locale loc;
	for (; 0 < count; --count, ++pStr1, ++pStr2)
	{
		CharT c1 = std::tolower(*pStr1, loc), c2 = std::tolower(*pStr2, loc);
		if (c1 != c2)
		{
			return c1 < c2 ? -1 : 1;
		}
	}

	if (m_Size < size)
	{
		return -1;
	}

	if (m_Size > size)
	{
		return 1;
	}

	return 0;
}


template<typename CharT, typename MemoryArenaType>
inline void BvStringT<CharT, MemoryArenaType>::Grow(u32 size)
{
	// Need to account for the null-terminating character (so not <=)
	if (size < m_Capacity)
	{
		return;
	}

	CharT* pNewStr = reinterpret_cast<CharT*>(m_pAllocator ? BV_MALLOC(*m_pAllocator, sizeof(CharT) * (size + 1), alignof(CharT)) : BV_ALLOC(sizeof(CharT) * (size + 1), alignof(CharT)));
	if (m_Size > 0)
	{
		std::char_traits<CharT>::copy(pNewStr, m_pStr, m_Size);
	}
	pNewStr[m_Size] = CharT();
	m_Capacity = size + 1;
	if (m_pStr)
	{
		m_pAllocator ? BV_MFREE(*m_pAllocator, m_pStr) : BV_FREE(m_pStr);
	}
	m_pStr = pNewStr;
}


template<typename CharT, typename MemoryArenaType>
inline void BvStringT<CharT, MemoryArenaType>::Destroy()
{
	if (m_pStr)
	{
		m_pAllocator ? BV_MFREE(*m_pAllocator, m_pStr) : BV_FREE(m_pStr);
		m_pStr = nullptr;
	}

	m_Size = 0;
	m_Capacity = 0;
}


// Typedefs for convenience
using BvString = BvStringT<char>;
using BvWString = BvStringT<wchar_t>;
using BvString8 = BvStringT<char8_t>;
using BvString16 = BvStringT<char16_t>;
using BvString32 = BvStringT<char32_t>;