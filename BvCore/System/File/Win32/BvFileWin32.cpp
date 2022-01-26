#include "BvCore/System/File/Win32/BvFileWin32.h"
#include "BvCore/Utils/BvDebug.h"
#include "BvCore/System/File/Win32/BvFileUtilsWin32.h"


BvFile::BvFile()
{
}


BvFile::BvFile(const char* const pFilename, BvFileAccessMode mode, BvFileAction action)
{
	wchar_t widePath[kMaxPathSize]{};
	Internal::ConvertToWidePathWithPrefix(widePath, pFilename);
	Open(widePath, mode, action);
}


BvFile::BvFile(const wchar_t* const pFilename, const BvFileAccessMode mode, BvFileAction action)
{
	Open(pFilename, mode, action);
}


BvFile::BvFile(BvFile && rhs) noexcept
{
	*this = std::move(rhs);
}


BvFile & BvFile::operator =(BvFile && rhs) noexcept
{
	if (this != &rhs)
	{
		auto hTmp = m_hFile;
		m_hFile = rhs.m_hFile;
		rhs.m_hFile = hTmp;
	}

	return *this;
}


BvFile::~BvFile()
{
	Close();
}


bool BvFile::Open(const char* const pFilename, BvFileAccessMode mode, BvFileAction action)
{
	wchar_t widePath[kMaxPathSize]{};
	Internal::ConvertToWidePathWithPrefix(widePath, pFilename);

	return Open(widePath, mode, action);
}


bool BvFile::Open(const wchar_t* const pFilename, BvFileAccessMode mode, BvFileAction action)
{
	BvAssert(pFilename != nullptr, "Invalid filename");

	DWORD acccessMode = 0;
	DWORD createMode = 0;
	DWORD createFlags = FILE_ATTRIBUTE_NORMAL;

	switch (mode)
	{
	case BvFileAccessMode::kRead: acccessMode = GENERIC_READ; break;
	case BvFileAccessMode::kWrite: acccessMode = GENERIC_WRITE; break;
	case BvFileAccessMode::kReadWrite: acccessMode = GENERIC_READ | GENERIC_WRITE; break;
	}

	switch (action)
	{
	case BvFileAction::kOpen: createMode = OPEN_EXISTING; break;
	case BvFileAction::kCreate: createMode = CREATE_ALWAYS; break;
	case BvFileAction::kOpenOrCreate: createMode = OPEN_ALWAYS; break;
	}

	m_hFile = CreateFileW(pFilename, acccessMode, 0, nullptr, createMode, createFlags, nullptr);
	if (m_hFile == INVALID_HANDLE_VALUE)
	{
		// TODO: Handle error
		return false;
	}

	return true;
}


u32 BvFile::Read(void * const pBuffer, const u32 bufferSize)
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE, "Invalid file handle");
	BvAssert(pBuffer != nullptr, "Null buffer");
	BvAssert(bufferSize > 0, "Invalid buffer size");

	u32 bytes = 0;
	if (!ReadFile(m_hFile, pBuffer, bufferSize, reinterpret_cast<LPDWORD>(&bytes), nullptr))
	{
		return 0;
	}

	return bytes;
}


u32 BvFile::Write(const void * const pBuffer, const u32 bufferSize)
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE, "Invalid file handle");
	BvAssert(pBuffer != nullptr, "Null buffer");
	BvAssert(bufferSize > 0, "Invalid buffer size");

	u32 bytes = 0;
	if (!WriteFile(m_hFile, pBuffer, bufferSize, reinterpret_cast<LPDWORD>(&bytes), nullptr))
	{
		return 0;
	}

	return bytes;
}


BvFile & BvFile::SkipBytes(const i64 offset)
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE, "Invalid file handle");
	LARGE_INTEGER seekOffset; seekOffset.QuadPart = offset;
	SetFilePointerEx(m_hFile, seekOffset, nullptr, FILE_CURRENT);

	return *this;
}


BvFile & BvFile::GoToStart()
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE, "Invalid file handle");
	LARGE_INTEGER seekOffset; seekOffset.QuadPart = 0ll;
	SetFilePointerEx(m_hFile, seekOffset, nullptr, FILE_BEGIN);

	return *this;
}


BvFile & BvFile::GoToEnd()
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE, "Invalid file handle");
	LARGE_INTEGER seekOffset; seekOffset.QuadPart = 0ll;
	SetFilePointerEx(m_hFile, seekOffset, nullptr, FILE_END);

	return *this;
}


const u64 BvFile::GetFilePos() const
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE, "Invalid file handle");
	u64 offset = 0; LARGE_INTEGER pos{};
	SetFilePointerEx(m_hFile, pos, reinterpret_cast<LARGE_INTEGER *>(&offset), FILE_CURRENT);

	return offset;
}


const u64 BvFile::GetSize() const
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE, "Invalid file handle");

	u64 fileSize = 0;
	BOOL status = GetFileSizeEx(m_hFile, reinterpret_cast<PLARGE_INTEGER>(&fileSize));
	if (!status)
	{
		// TODO: Handle Error
	}

	return fileSize;
}


void BvFile::Close()
{
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}
}


const bool BvFile::IsValid() const
{
	return m_hFile != INVALID_HANDLE_VALUE;
}