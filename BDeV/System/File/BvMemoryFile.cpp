#include "BvMemoryFile.h"


BvMemoryFile::BvMemoryFile()
{
}


BvMemoryFile::BvMemoryFile(size_t size, i64 currPos)
	: m_pData(new u8[size]), m_Size(size), m_Pos(currPos)
{
}


BvMemoryFile::BvMemoryFile(const u8* const pData, size_t size, i64 currPos)
	: m_pData(new u8[size]), m_Size(size), m_Pos(currPos)
{
	memcpy(m_pData, pData, size);
}


BvMemoryFile::BvMemoryFile(BvMemoryFile&& rhs) noexcept
{
	*this = std::move(rhs);
}


BvMemoryFile& BvMemoryFile::operator=(BvMemoryFile&& rhs) noexcept
{
	if (this != &rhs)
	{
		m_pData = rhs.m_pData;
		m_Size = rhs.m_Size;
		m_Pos = rhs.m_Pos;

		rhs.m_pData = nullptr;
		rhs.m_Size = 0;
		rhs.m_Pos = 0;
	}

	return *this;
}


BvMemoryFile::~BvMemoryFile()
{
	delete[] m_pData;
}


u32 BvMemoryFile::Read(void* const pBuffer, const u32 bufferSize)
{
	if ((u64)m_Pos >= m_Size)
	{
		return 0;
	}

	auto totalSize = std::min((u64)bufferSize, m_Size - (u64)m_Pos);
	memcpy(pBuffer, m_pData + m_Pos, totalSize);
	m_Pos += (i64)totalSize;

	return (u32)totalSize;
}

u32 BvMemoryFile::Write(const void* const pBuffer, const u32 bufferSize)
{
	auto newSize = (u64)(m_Pos + bufferSize);
	if (newSize >= m_Size)
	{
		newSize = GetNextPowerOf2(newSize);
		auto pNewData = new u8[newSize];
		memcpy(pNewData, m_pData, m_Size);
		delete[] m_pData;
		m_pData = pNewData;
		m_Size = newSize;
	}

	memcpy(m_pData + m_Pos, pBuffer, bufferSize);
	m_Pos += bufferSize;

	return bufferSize;
}


BvMemoryFile& BvMemoryFile::SkipBytes(const i64 offset)
{
	auto newOffset = m_Pos + offset;
	if (newOffset < 0)
	{
		m_Pos = 0;
	}
	else if (newOffset > m_Size)
	{
		m_Pos = m_Size;
	}
	else
	{
		m_Pos = newOffset;
	}

	return *this;
}


BvMemoryFile& BvMemoryFile::GoToStart()
{
	m_Pos = 0;

	return *this;
}


BvMemoryFile& BvMemoryFile::GoToEnd()
{
	m_Pos = m_Size;

	return *this;
}


const u64 BvMemoryFile::GetFilePos() const
{
	return m_Pos;
}


const u64 BvMemoryFile::GetSize() const
{
	return m_Size;
}


const bool BvMemoryFile::IsValid() const
{
	return m_pData != nullptr;
}