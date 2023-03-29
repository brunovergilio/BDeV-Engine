#include "BDeV/System/File/BvAsyncFile.h"
#include "BDeV/System/Debug/BvDebug.h"
#include "BDeV/System/Threading/BvSync.h"
#include "BDeV/Utils/BvUtils.h"
#include "BDeV/System/File/BvFileUtils.h"


struct AsyncFileData
{
	BvSignal m_Signal;
	OVERLAPPED m_OverlappedIO{};
	HANDLE m_hFile{};
	std::atomic<u32> m_UseCount;

	AsyncFileData()
		: m_Signal(BvSignal(true)), m_UseCount(1) {}
	~AsyncFileData() { m_UseCount--; }
};


AsyncFileRequest::AsyncFileRequest()
{
}


AsyncFileRequest::AsyncFileRequest(const AsyncFileRequest & rhs)
	: m_pIOData(rhs.m_pIOData)
{
	if (m_pIOData)
	{
		m_pIOData->m_UseCount++;
	}
}


AsyncFileRequest & AsyncFileRequest::operator =(const AsyncFileRequest & rhs)
{
	if (this != &rhs)
	{
		m_pIOData = rhs.m_pIOData;

		if (m_pIOData)
		{
			m_pIOData->m_UseCount++;
		}
	}

	return *this;
}


AsyncFileRequest::AsyncFileRequest(AsyncFileRequest && rhs) noexcept
{
	*this = std::move(rhs);
}


AsyncFileRequest & AsyncFileRequest::operator =(AsyncFileRequest && rhs) noexcept
{
	if (this != &rhs)
	{
		m_pIOData = rhs.m_pIOData;
		rhs.m_pIOData = nullptr;
	}

	return *this;
}


AsyncFileRequest::~AsyncFileRequest()
{
	if (m_pIOData)
	{
		if (--m_pIOData->m_UseCount == 0)
		{
			delete m_pIOData;
		}
	}
}


i32 AsyncFileRequest::GetResult(const bool wait)
{
	BvAssert(m_pIOData != nullptr, "Invalid Async IO Data");
	i32 result = 0;
	if (GetOverlappedResult(m_pIOData->m_hFile, &m_pIOData->m_OverlappedIO, reinterpret_cast<LPDWORD>(&result), wait))
	{
		return result;
	}
	else
	{
		auto error = GetLastError();
		if (error != ERROR_IO_INCOMPLETE && error != ERROR_HANDLE_EOF)
		{
			// TODO: Handle error
		}
	}

	return result;
}


void AsyncFileRequest::Cancel()
{
	BvAssert(m_pIOData != nullptr, "Invalid Async IO Data");
	if (!CancelIoEx(m_pIOData->m_hFile, &m_pIOData->m_OverlappedIO))
	{
		// TODO: Handle error
	}
}


BvAsyncFile::BvAsyncFile()
{
}


BvAsyncFile::BvAsyncFile(const char* const pFilename, BvFileAccessMode mode, BvFileAction action)
{
	wchar_t widePath[kMaxPathSize]{};
	ConvertToWidePathWithPrefix(widePath, pFilename);
	Open(widePath, mode, action);
}


BvAsyncFile::BvAsyncFile(const wchar_t* const pFilename, BvFileAccessMode mode, BvFileAction action)
{
	Open(pFilename, mode, action);
}


BvAsyncFile::BvAsyncFile(BvAsyncFile && rhs) noexcept
{
	*this = std::move(rhs);
}


BvAsyncFile & BvAsyncFile::operator =(BvAsyncFile && rhs) noexcept
{
	if (this != &rhs)
	{
		m_hFile = rhs.m_hFile;
		rhs.m_hFile = INVALID_HANDLE_VALUE;
	}

	return *this;
}


BvAsyncFile::~BvAsyncFile()
{
	Close();
}


bool BvAsyncFile::Open(const char* const pFilename, BvFileAccessMode mode, BvFileAction action)
{
	wchar_t widePath[kMaxPathSize]{};
	ConvertToWidePathWithPrefix(widePath, pFilename);

	return Open(widePath, mode, action);
}


bool BvAsyncFile::Open(const wchar_t* const pFilename, BvFileAccessMode mode, BvFileAction action)
{
	BvAssert(pFilename != nullptr, "Invalid filename");

	DWORD acccessMode = 0;
	DWORD createMode = 0;
	DWORD createFlags = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING;

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


AsyncFileRequest BvAsyncFile::Read(void * const pBuffer, const u32 bufferSize, const u64 position)
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE, "Invalid file handle");
	BvAssert(pBuffer != nullptr, "Null buffer");
	BvAssert(bufferSize > 0, "Invalid buffer size");

	AsyncFileRequest request;
	request.m_pIOData = new AsyncFileData();
	request.m_pIOData->m_OverlappedIO.Pointer = reinterpret_cast<void *>(position);
	request.m_pIOData->m_hFile = m_hFile;

	BOOL status = ReadFile(m_hFile, pBuffer, bufferSize, nullptr, &request.m_pIOData->m_OverlappedIO);
	if (!status)
	{
		auto error = GetLastError();
		if (error != ERROR_IO_PENDING)
		{
			// TODO: Handle Error
		}
	}

	return request;
}


AsyncFileRequest BvAsyncFile::Write(const void * const pBuffer, const u32 bufferSize, const u64 position)
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE, "Invalid file handle");
	BvAssert(pBuffer != nullptr, "Null buffer");
	BvAssert(bufferSize > 0, "Invalid buffer size");

	AsyncFileRequest request;
	request.m_pIOData = new AsyncFileData();
	request.m_pIOData->m_OverlappedIO.Pointer = reinterpret_cast<void *>(position);
	request.m_pIOData->m_hFile = m_hFile;

	BOOL status = WriteFile(m_hFile, pBuffer, bufferSize, nullptr, &request.m_pIOData->m_OverlappedIO);
	if (!status)
	{
		auto error = GetLastError();
		if (error != ERROR_IO_PENDING)
		{
			// TODO: Handle Error
		}
	}

	return request;
}


const u64 BvAsyncFile::GetSize() const
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE, "Invalid file handle");

	u64 fileSize;
	BOOL status = GetFileSizeEx(m_hFile, reinterpret_cast<PLARGE_INTEGER>(&fileSize));
	if (!status)
	{
		// TODO: Handle Error
	}

	return fileSize;
}


void BvAsyncFile::Close()
{
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}
}


void BvAsyncFile::GetFileInfo(BvFileInfo& fileInfo)
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE, "Invalid file handle");

	BY_HANDLE_FILE_INFORMATION bhfi;
	GetFileInformationByHandle(m_hFile, &bhfi);

	fileInfo.m_CreationTimestamp = *reinterpret_cast<u64*>(&bhfi.ftCreationTime);
	fileInfo.m_LastAccessTimestamp = *reinterpret_cast<u64*>(&bhfi.ftLastAccessTime);
	fileInfo.m_LastWriteTimestamp = *reinterpret_cast<u64*>(&bhfi.ftLastWriteTime);
	fileInfo.m_FileSize = (static_cast<u64>(bhfi.nFileSizeHigh) << 32) | static_cast<u64>(bhfi.nFileSizeLow);
}


bool BvAsyncFile::IsValid() const
{
	return m_hFile != INVALID_HANDLE_VALUE;
}