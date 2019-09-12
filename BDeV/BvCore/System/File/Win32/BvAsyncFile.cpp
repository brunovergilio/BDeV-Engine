#include "BvAsyncFile.h"
#include "BvCore/BvDebug.h"
#include "BvCore/System/File/FileSystem.h"


#if (BV_PLATFORM == BV_PLATFORM_WIN32)


AsyncFileRequest::AsyncFileRequest()
{
	m_pIOData = new AsyncFileData();
	m_pIOData->m_InUse.Increment();
}


AsyncFileRequest::AsyncFileRequest(const AsyncFileRequest & rhs)
	: m_pIOData(rhs.m_pIOData), m_hFile(rhs.m_hFile), m_BytesTransfered(rhs.m_BytesTransfered), m_Result(rhs.m_Result)
{
	if (m_pIOData)
	{
		m_pIOData->m_InUse.Increment();
	}
}


AsyncFileRequest & AsyncFileRequest::operator =(const AsyncFileRequest & rhs)
{
	if (this != &rhs)
	{
		m_pIOData = rhs.m_pIOData;
		m_hFile = rhs.m_hFile;
		m_BytesTransfered = rhs.m_BytesTransfered;
		m_Result = rhs.m_Result;

		if (m_pIOData)
		{
			m_pIOData->m_InUse.Increment();
		}
	}

	return *this;
}


AsyncFileRequest::AsyncFileRequest(AsyncFileRequest && rhs)
{
	*this = std::move(rhs);
}


AsyncFileRequest & AsyncFileRequest::operator =(AsyncFileRequest && rhs)
{
	m_pIOData = rhs.m_pIOData;
	m_hFile = rhs.m_hFile;
	m_BytesTransfered = rhs.m_BytesTransfered;
	m_Result = rhs.m_Result;

	rhs.m_pIOData = nullptr;
	rhs.m_hFile = INVALID_HANDLE_VALUE;
	rhs.m_BytesTransfered = 0;
	rhs.m_Result = EnumVal(BvFileResult::kError);

	return *this;
}


AsyncFileRequest::~AsyncFileRequest()
{
	if (m_pIOData)
	{
		if (m_pIOData->m_InUse.Decrement() == 0)
		{
			delete m_pIOData;
		}
	}
}


u32 AsyncFileRequest::GetResult(const bool waitForIt)
{
	if ((m_Result == EnumVal(BvFileResult::kInProgress)))
	{
		if (GetOverlappedResult(m_hFile, &m_pIOData->m_OverlappedIO, reinterpret_cast<LPDWORD>(&m_BytesTransfered), waitForIt))
		{
			m_Result = EnumVal(BvFileResult::kDoneAsync);
		}
		else
		{
			i32 error = GetLastError();
			if (error == ERROR_IO_INCOMPLETE)
			{
				m_Result = EnumVal(BvFileResult::kInProgress);
			}
			else if (error == ERROR_HANDLE_EOF)
			{
				m_Result = BvFileResult::kDoneAsync | BvFileResult::kEOF;
			}
			else
			{
				m_Result = EnumVal(BvFileResult::kError);
			}
		}
	}

	return m_Result;
}


BvAsyncFile::BvAsyncFile(const char * const pFilename, const BvFileAccess mode)
{
	Open(pFilename, mode);
}


BvAsyncFile::BvAsyncFile(BvAsyncFile && rhs)
{
	BvAssert(this != &rhs);
	*this = std::move(rhs);
}


BvAsyncFile & BvAsyncFile::operator =(BvAsyncFile && rhs)
{
	BvAssert(this != &rhs);
	m_hFile = rhs.m_hFile;
	rhs.m_hFile = INVALID_HANDLE_VALUE;

	return *this;
}


BvAsyncFile::~BvAsyncFile()
{
	Close();
}


BvFileResult BvAsyncFile::Open(const char * const pFilename, const BvFileAccess mode)
{
	BvAssert(pFilename != nullptr);

	DWORD acccessMode = 0;
	DWORD shareMode = 0;
	DWORD createMode = 0;
	DWORD createFlags = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING;

	bool exists = FileSys.FileExists(pFilename);

	switch (mode)
	{
	case BvFileAccess::kRead:
		acccessMode = GENERIC_READ;
		shareMode = FILE_SHARE_READ;
		createMode = OPEN_EXISTING;
		break;
	case BvFileAccess::kWrite:
		acccessMode = GENERIC_WRITE;
		createMode = CREATE_ALWAYS;
		break;
	case BvFileAccess::kReadAndWrite:
		acccessMode = GENERIC_READ | GENERIC_WRITE;
		createMode = exists ? OPEN_EXISTING : CREATE_NEW;
		break;
	default:
		break;
	}

	m_hFile = CreateFile(pFilename, acccessMode, shareMode, nullptr, createMode, createFlags, nullptr);

	return m_hFile != INVALID_HANDLE_VALUE ? BvFileResult::kOk : BvFileResult::kError;
}


void BvAsyncFile::Close()
{
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}
}


AsyncFileRequest BvAsyncFile::Read(void * const pBuffer, const u32 bufferLength, const u64 position)
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE);
	BvAssert(pBuffer != nullptr);
	BvAssert(bufferLength > 0);


	AsyncFileRequest result;
	result.m_pIOData->m_OverlappedIO.Pointer = reinterpret_cast<void *>(position);
	result.m_hFile = m_hFile;

	BOOL status = ReadFile(m_hFile, pBuffer, bufferLength, nullptr, &result.m_pIOData->m_OverlappedIO);
	if (status)
	{
		result.m_pIOData->m_Signal.Set();
		GetOverlappedResult(m_hFile, &result.m_pIOData->m_OverlappedIO, reinterpret_cast<LPDWORD>(&result.m_BytesTransfered), FALSE);

		result.m_Result = BvFileResult::kOk | BvFileResult::kDoneSync;
		if (result.m_BytesTransfered == 0)
		{
			result.m_Result |= BvFileResult::kEOF;
		}
	}
	else
	{
		i32 error = GetLastError();
		if (error == ERROR_IO_PENDING)
		{
			result.m_Result = EnumVal(BvFileResult::kInProgress);
		}
		else
		{
			result.m_pIOData->m_InUse.Store(0);
			result.m_pIOData = nullptr;
			result.m_Result = EnumVal(BvFileResult::kError);
		}
	}

	return result;
}


AsyncFileRequest BvAsyncFile::Write(const void * const pBuffer, const u32 bufferLength, const u64 position)
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE);
	BvAssert(pBuffer != nullptr);
	BvAssert(bufferLength > 0);

	AsyncFileRequest result;
	result.m_pIOData->m_OverlappedIO.Pointer = reinterpret_cast<void *>(position);
	result.m_hFile = m_hFile;

	BOOL status = WriteFile(m_hFile, pBuffer, bufferLength, nullptr, &result.m_pIOData->m_OverlappedIO);
	if (status)
	{
		result.m_pIOData->m_Signal.Set();
		GetOverlappedResult(m_hFile, &result.m_pIOData->m_OverlappedIO, reinterpret_cast<LPDWORD>(&result.m_BytesTransfered), FALSE);

		result.m_Result = BvFileResult::kOk | BvFileResult::kDoneSync;
		if (result.m_BytesTransfered == 0)
		{
			result.m_Result |= BvFileResult::kEOF;
		}
	}
	else
	{
		i32 error = GetLastError();
		if (error == ERROR_IO_PENDING)
		{
			result.m_Result = EnumVal(BvFileResult::kInProgress);
		}
		else
		{
			result.m_pIOData->m_InUse.Store(0);
			result.m_pIOData = nullptr;
			result.m_Result = EnumVal(BvFileResult::kError);
		}
	}

	return result;
}


const u64 BvAsyncFile::GetSize() const
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE);

	u64 fileSize;
	GetFileSizeEx(m_hFile, reinterpret_cast<PLARGE_INTEGER>(&fileSize));

	return fileSize;
}


#else
#error "Platform not yet supported"
#endif // #if (BV_PLATFORM == BV_PLATFORM_WIN32)