#include "BvCore/System/File/BvFile.h"
#include "BvCore/BvDebug.h"


#if (BV_PLATFORM == BV_PLATFORM_WIN32)


BvFile::BvFile()
	: m_hFile(INVALID_HANDLE_VALUE)
{
}


BvFile::BvFile(BvFile && rhs)
{
	*this = std::move(rhs);
}


BvFile & BvFile::operator =(BvFile && rhs)
{
	if (this != &rhs)
	{
		m_hFile = rhs.m_hFile;
		rhs.m_hFile = INVALID_HANDLE_VALUE;
	}

	return *this;
}


BvFile::~BvFile()
{
}


u32 BvFile::Read(void * const pBuffer, const u32 bufferSize)
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE);
	BvAssert(pBuffer != nullptr);
	BvAssert(bufferSize > 0);

	u32 bytes = 0;
	if (!ReadFile(m_hFile, pBuffer, bufferSize, reinterpret_cast<LPDWORD>(&bytes), nullptr))
	{
		return 0;
	}

	return bytes;
}


u32 BvFile::Write(const void * const pBuffer, const u32 bufferSize)
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE);
	BvAssert(pBuffer != nullptr);
	BvAssert(bufferSize > 0);

	u32 bytes = 0;
	if (!WriteFile(m_hFile, pBuffer, bufferSize, reinterpret_cast<LPDWORD>(&bytes), nullptr))
	{
		return 0;
	}

	return bytes;
}


BvFile & BvFile::SkipBytes(const i64 offset)
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE);
	LARGE_INTEGER seekOffset; seekOffset.QuadPart = offset;
	SetFilePointerEx(m_hFile, seekOffset, nullptr, FILE_CURRENT);

	return *this;
}


BvFile & BvFile::GoToStart()
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE);
	LARGE_INTEGER seekOffset; seekOffset.QuadPart = 0ll;
	SetFilePointerEx(m_hFile, seekOffset, nullptr, FILE_BEGIN);

	return *this;
}


BvFile & BvFile::GoToEnd()
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE);
	LARGE_INTEGER seekOffset; seekOffset.QuadPart = 0ll;
	SetFilePointerEx(m_hFile, seekOffset, nullptr, FILE_END);

	return *this;
}


const u64 BvFile::GetFilePos() const
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE);
	u64 offset = 0; LARGE_INTEGER pos{};
	SetFilePointerEx(m_hFile, pos, reinterpret_cast<LARGE_INTEGER *>(&offset), FILE_CURRENT);

	return offset;
}


const u64 BvFile::GetSize() const
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE);

	u64 fileSize = 0;
	GetFileSizeEx(m_hFile, reinterpret_cast<PLARGE_INTEGER>(&fileSize));

	return fileSize;
}


const bool BvFile::IsValid() const
{
	return m_hFile != INVALID_HANDLE_VALUE;
}


#endif // #if (BV_PLATFORM == BV_PLATFORM_WIN32)