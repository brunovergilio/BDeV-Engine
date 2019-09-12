#include "BvString.h"
#include "BvCore/System/Memory/BvMemory.h"
#include "BvCore/Utils/BvUtils.h"
#include "BvCore/Utils/Hash.h"


BvString::BvString()
{
}


BvString::BvString(const u32 size)
	: m_pStr(new char[size]), m_Size(0), m_Capacity(size)
{
	BvAssert(m_pStr != nullptr);
}


BvString::BvString(const BvString & rhs)
{
	Assign(rhs);
}


BvString & BvString::operator=(const BvString & rhs)
{
	if (this != &rhs)
	{
		Assign(rhs);
	}

	return *this;
}


BvString::BvString(BvString && rhs)
{
	*this = std::move(rhs);
}


BvString & BvString::operator=(BvString && rhs)
{
	if (this != &rhs)
	{
		m_pStr = rhs.m_pStr;
		m_Size = rhs.m_Size;
		m_Capacity = rhs.m_Capacity;

		rhs.m_pStr = nullptr;
		rhs.m_Size = 0;
		rhs.m_Capacity = 0;
	}

	return *this;
}


BvString & BvString::operator=(const char * const pStr)
{
	if (m_pStr != pStr)
	{
		Assign(pStr);
	}

	return *this;
}

BvString & BvString::operator=(const char c)
{
	Assign(c);

	return *this;
}


BvString::~BvString()
{
	BvDeleteArray(m_pStr);
	m_Size = 0;
	m_Capacity = 0;
}


BvString::BvString(const BvString & str, const u32 start, const u32 count)
{
	Assign(str, start, count);
}


BvString::BvString(const char * const pStr)
{
	Assign(pStr);
}


BvString::BvString(const char * const pStr, const u32 start, const u32 count)
{
	Assign(pStr, start, count);
}


BvString::BvString(const char c)
{
	Assign(c);
}


void BvString::Assign(const BvString & str)
{
	Assign(str.m_pStr, 0, str.m_Size);
}


void BvString::Assign(const char * const pStr)
{
	Assign(pStr, 0, static_cast<u32>(strlen(pStr)));
}


void BvString::Assign(const char c)
{
	Assign(&c, 0, 1);
}


void BvString::Assign(const BvString & str, const u32 start, const u32 count)
{
	Assign(str.m_pStr, start, count);
}


void BvString::Assign(const char * const pStr, const u32 start, const u32 count)
{
	BvAssert(m_pStr != pStr);

	if (m_Capacity <= count)
	{
		Resize(GetNextPowerOf2(count));
	}

	memcpy(m_pStr, pStr + start, count);
	m_pStr[count] = 0;
	m_Size = count;
}


void BvString::Insert(const BvString & str, const u32 where)
{
	Insert(str.m_pStr, 0, str.m_Size, where);
}


void BvString::Insert(const char * const pStr, const u32 where)
{
	Insert(pStr, 0, static_cast<u32>(strlen(pStr)), where);
}


void BvString::Insert(const char c, const u32 where)
{
	Insert(&c, 0, 1, where);
}


void BvString::Insert(const BvString & str, const u32 start, const u32 count, const u32 where)
{
	Insert(str.m_pStr, start, count, where);
}


void BvString::Insert(const char * const pStr, const u32 start, const u32 count, const u32 where)
{
	BvAssert(m_Size >= where);
	if (count == 0)
	{
		return;
	}

	u32 newSize = count + m_Size;
	if (m_Capacity <= newSize)
	{
		Resize(GetNextPowerOf2(newSize));
	}
	m_pStr[newSize] = 0;

	//u32 i;
	//for (i = newSize - 1; i >= where + count; i--)
	//{
	//	m_pStr[i] = m_pStr[i - count];
	//}
	memmove(m_pStr + where + count, m_pStr + where, m_Size - where);
	memcpy(m_pStr + where, pStr + start, count);
	//u32 addPos = start;
	//for (i = where; i < count; i++)
	//{
	//	m_pStr[where] = pStr[addPos++];
	//}
	m_Size = newSize;
}


void BvString::Replace(const BvString & srcStr, const BvString dstStr)
{
	Replace(srcStr.m_pStr, srcStr.m_Size, dstStr.m_pStr, dstStr.m_Size);
}


void BvString::Replace(const char * const pSrcStr, const char * const pDstStr)
{
	Replace(pSrcStr, static_cast<u32>(strlen(pSrcStr)), pDstStr, static_cast<u32>(strlen(pDstStr)));
}


void BvString::Replace(const char * const pSrcStr, const u32 srcSize, const char * const pDstStr, const u32 dstSize)
{
	u32 index = Find(pSrcStr, srcSize);
	if (index == kInvalidIndex)
	{
		return;
	}

	Erase(index, srcSize);
	Insert(pDstStr, 0, dstSize, index);
}


const u32 BvString::ReadLine(BvString & dstStr, const u32 startIndex)
{
	if (m_Size == 0)
	{
		return 0;
	}

	u32 i = startIndex;
	while (i < m_Size && m_pStr[i] != '\n')
	{
		i++;
	}

	u32 count = i - startIndex;
	dstStr.Assign(m_pStr, startIndex, count);

	return count;
}


void BvString::Format(const char * const format, ...)
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


void BvString::Erase(const u32 start, const u32 count)
{
	BvAssert(start < m_Size && count > 0);
	u32 i = start;
	u32 max = (start + count) >= m_Size ? m_Size : start + count;
	for (; i < max - 1; i++)
	{
		m_pStr[i] = m_pStr[i + 1];
	}
	m_pStr[i] = 0;
	m_Size = i;
}


void BvString::Resize(const u32 size)
{
	BvAssert(size > 0);

	u32 copyLen = size > m_Size ? m_Size : size - 1;

	char * pNewStr = new char[size];

	memcpy(pNewStr, m_pStr, copyLen);
	pNewStr[copyLen] = 0;
	BvDelete(m_pStr);

	m_pStr = pNewStr;
	m_Size = copyLen;
	m_Capacity = size;
}


void BvString::Clear()
{
	memset(m_pStr, 0, m_Size);
	m_Size = 0;
}


void BvString::Swap(BvString & str)
{
	if (m_Size >= str.m_Capacity)
	{
		str.Resize(m_Capacity);
	}
	if (str.m_Size >= m_Capacity)
	{
		Resize(str.m_Capacity);
	}

	char *pDst, *pSrc;
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

	char tmp;
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


void BvString::Copy(BvString & str) const
{
	str.Assign(*this);
}


void BvString::Copy(BvString & str, const u32 start, const u32 count) const
{
	str.Assign(*this, start, count);
}


BvString BvString::Substr(const u32 start, const u32 count)
{
	return BvString(m_pStr, start, count);
}


const u32 BvString::Find(const char c) const
{
	return Find(&c, 1);
}


const u32 BvString::Find(const BvString & str) const
{
	return Find(str.m_pStr, str.m_Size);
}


const u32 BvString::Find(const char * const pStr) const
{
	return Find(pStr, static_cast<u32>(strlen(pStr)));
}


const u32 BvString::Find(const char * const pStr, const u32 size) const
{
	if (size <= m_Size)
	{
		u32 j;
		bool found;
		for (u32 i = 0; m_Size - i >= size; i++)
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
	}

	return kInvalidIndex;
}


const u32 BvString::RFind(const char c) const
{
	return RFind(&c, 1);
}


const u32 BvString::RFind(const BvString & str) const
{
	return RFind(str.m_pStr, str.m_Size);
}


const u32 BvString::RFind(const char * const pStr) const
{
	return RFind(pStr, static_cast<u32>(strlen(pStr)));
}


const u32 BvString::RFind(const char * const pStr, const u32 size) const
{
	if (size <= m_Size)
	{
		u32 j;
		bool found;
		for (u32 i = m_Size - size + 1; i > 0; i--)
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
	}

	return kInvalidIndex;
}


const u32 BvString::Hash32() const
{
	//u32 hash = 5381;
	//u32 c;

	//for (u32 i = 0; i < m_Size; i++)
	//{
	//	c = static_cast<u32>(m_pStr[i]);
	//	hash = ((hash << 5) + hash) + c;
	//}

	//return hash;

	return FNV1a32(m_pStr, m_Size);
}


BvString BvString::operator +(const BvString & str)
{
	BvString newStr = *this;
	newStr.Append(str);

	return newStr;
}


BvString BvString::operator +(const char c)
{
	BvString newStr = *this;
	newStr.Append(c);

	return newStr;
}


BvString BvString::operator +(const char * const pStr)
{
	BvString newStr = *this;
	newStr.Append(pStr);

	return newStr;
}


BvString operator +(const char * const pStr, const BvString & str)
{
	BvString newStr(pStr);
	newStr.Append(str);

	return newStr;
}


BvString operator+(const char c, const BvString & str)
{
	BvString newStr(str);
	newStr.Insert(c);

	return newStr;
}