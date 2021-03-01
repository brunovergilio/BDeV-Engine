#pragma once


#include "BvCore/Utils/BvDebug.h"
#include <string.h>


class BvString
{
public:
	static constexpr u32 kInvalidIndex = kU32Max;

	BvString();
	explicit BvString(const u32 size);
	BvString(const BvString & rhs);
	BvString & operator =(const BvString & rhs);
	BvString(BvString && rhs);
	BvString & operator =(BvString && rhs);
	BvString & operator =(const char * const pStr);
	BvString & operator =(const char c);
	~BvString();

	explicit BvString(const BvString & str, const u32 start, const u32 count);
	BvString(const char * const pStr);
	explicit BvString(const char * const pStr, const u32 start, const u32 count);
	BvString(const char c);

	void Assign(const BvString & str);
	void Assign(const char * const pStr);
	void Assign(const char c);
	void Assign(const BvString & str, const u32 start, const u32 count);
	void Assign(const char * const pStr, const u32 start, const u32 count);

	void Insert(const BvString & str, const u32 where = 0);
	void Insert(const char * const pStr, const u32 where = 0);
	void Insert(const char c, const u32 where = 0);
	void Insert(const BvString & str, const u32 start, const u32 count, const u32 where = 0);
	void Insert(const char * const pStr, const u32 start, const u32 count, const u32 where = 0);

	void Append(const BvString & str) { Insert(str, m_Size); }
	void Append(const char * const pStr) { Insert(pStr, m_Size); }
	void Append(const char c) { Insert(c, m_Size); }
	void Append(const BvString & str, const u32 start, const u32 count) { Insert(str, start, count, m_Size); }
	void Append(const char * const pStr, const u32 start, const u32 count) { Insert(pStr, start, count, m_Size); }

	void Replace(const BvString & srcStr, const BvString dstStr);
	void Replace(const char * const pSrcStr, const char * const pDstStr);
	void Replace(const char * const pSrcStr, const u32 srcSize, const char * const pDstStr, const u32 dstSize);

	const u32 ReadLine(BvString & dstStr, const u32 startIndex = 0);
	void Format(const char * const format, ...);

	void Erase(const u32 start, const u32 count);

	void Resize(const u32 size);
	void Clear();

	void Swap(BvString & str);
	void Copy(BvString & str) const;
	void Copy(BvString & str, const u32 start, const u32 count) const;
	BvString Substr(const u32 start, const u32 count);

	const u32 Find(const char c) const;
	const u32 Find(const BvString & str) const;
	const u32 Find(const char * const pStr) const;
	const u32 Find(const char * const pStr, const u32 size) const;

	const u32 RFind(const char c) const;
	const u32 RFind(const BvString & str) const;
	const u32 RFind(const char * const pStr) const;
	const u32 RFind(const char * const pStr, const u32 size) const;

	BV_INLINE const bool Contains(const char c) const { return Find(c) != kInvalidIndex; }
	BV_INLINE const bool Contains(const BvString & str) const { return Find(str) != kInvalidIndex; }
	BV_INLINE const bool Contains(const char * const pStr) const { return Find(pStr) != kInvalidIndex; }

	const u32 Hash32() const;

	BvString & operator +=(const BvString & str) { Append(str); return *this; }
	BvString & operator +=(const char * const pStr) { Append(pStr); return *this; }
	BvString & operator +=(const char c) { Append(c); return *this; }

	BvString operator +(const BvString & str);
	BvString operator +(const char * const pStr);
	BvString operator +(const char c);
	friend BvString operator +(const char * const pStr, const BvString & str);
	friend BvString operator +(const char c, const BvString & str);

	BV_INLINE const char & operator[](const u32 index) const { BvAssertMsg(index < m_Size, "Index out of bounds"); return m_pStr[index]; }
	BV_INLINE char & operator[](const u32 index) { BvAssertMsg(index < m_Size, "Index out of bounds"); return m_pStr[index]; }
	BV_INLINE const char At(const u32 index) const { BvAssertMsg(index < m_Size, "Index out of bounds"); return m_pStr[index]; }
	BV_INLINE const char Front() const { BvAssertMsg(m_Size > 0, "Index out of bounds"); return m_pStr[0]; }
	BV_INLINE const char Back() const { BvAssertMsg(m_Size > 0, "Index out of bounds"); return m_pStr[m_Size - 1]; }

	BV_INLINE const char * const CStr() const { return m_pStr; }
	BV_INLINE const u32 Size() const { return m_Size; }
	BV_INLINE const u32 Capacity() const { return m_Capacity; }
	BV_INLINE const bool Empty() const { return m_Size == 0; }

	BV_INLINE operator const char * const() { return m_pStr; }

	const i32 Compare(const BvString & str) const { return strcmp(m_pStr, str.m_pStr); }
	const i32 Compare(const char * const pStr) const { return strcmp(m_pStr, pStr); }

	const bool operator ==(const BvString & str) const { return Compare(str) == 0; }
	const bool operator ==(const char * const pStr) const { return Compare(pStr) == 0; }

	const bool operator <(const BvString & str) const { return Compare(str) < 0; }
	const bool operator <(const char * const pStr) const { return Compare(pStr) < 0; }

	const bool operator <=(const BvString & str) const { return Compare(str) <= 0; }
	const bool operator <=(const char * const pStr) const { return Compare(pStr) <= 0; }

	const bool operator >(const BvString & str) const { return Compare(str) > 0; }
	const bool operator >(const char * const pStr) const { return Compare(pStr) > 0; }

	const bool operator >=(const BvString & str) const { return Compare(str) >= 0; }
	const bool operator >=(const char * const pStr) const { return Compare(pStr) >= 0; }

protected:
	char * m_pStr = nullptr;
	u32 m_Size = 0;
	u32 m_Capacity = 0;
};