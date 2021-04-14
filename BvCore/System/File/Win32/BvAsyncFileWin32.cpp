#include "BvCore/System/File/BvAsyncFile.h"
#include "BvCore/Utils/BvDebug.h"
#include "BvCore/System/Threading/BvSync.h"
#include "BvCore/Utils/BvUtils.h"


#if (BV_PLATFORM == BV_PLATFORM_WIN32)


struct AsyncFileData
{
	BvEvent m_Signal;
	OVERLAPPED m_OverlappedIO{};
	HANDLE m_hFile{};
	std::atomic<u32> m_UseCount;

	AsyncFileData()
		: m_Signal(BvEvent(true)), m_UseCount(1) {}
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


u32 AsyncFileRequest::GetResult(const bool wait)
{
	BvAssert(m_pIOData != nullptr, "Invalid Async IO Data");
	auto result = 0;
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
	: m_hFile(INVALID_HANDLE_VALUE)
{
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
	GetFileSizeEx(m_hFile, reinterpret_cast<PLARGE_INTEGER>(&fileSize));

	return fileSize;
}


const bool BvAsyncFile::IsValid() const
{
	return m_hFile != INVALID_HANDLE_VALUE;
}


#else
#error "Platform not yet supported"
#endif // #if (BV_PLATFORM == BV_PLATFORM_WIN32)