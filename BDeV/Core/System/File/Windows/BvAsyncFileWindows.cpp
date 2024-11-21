#include "BDeV/Core/System/File/BvAsyncFile.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BDeV/Core/System/Threading/BvSync.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Container/BvText.h"
#include "BDeV/Core/System/Memory/BvMemoryArea.h"
#include "BDeV/Core/System/Memory/BvMemory.h"


struct AsyncFileData : OVERLAPPED
{
	HANDLE m_hFile;
	void* m_pBuffer;
	u32 m_TotalBytesRequested;
	u32 m_TotalBytesProcessed;
	bool m_IsReadOp;
	bool m_IsDone;
};


namespace Internal
{
	BOOL ReadAsync(AsyncFileData& data, u32 bytesToRead)
	{
		return ReadFile(data.m_hFile, data.m_pBuffer, bytesToRead, nullptr, &data);
	}


	BOOL WriteAsync(AsyncFileData& data, u32 bytesToWrite)
	{
		return WriteFile(data.m_hFile, data.m_pBuffer, bytesToWrite, nullptr, &data);
	}
}


AsyncFileRequest::AsyncFileRequest()
{
}


AsyncFileRequest::AsyncFileRequest(AsyncFileRequest&& rhs) noexcept
{
	*this = std::move(rhs);
}


AsyncFileRequest& AsyncFileRequest::operator=(AsyncFileRequest&& rhs) noexcept
{
	if (this != &rhs)
	{
		std::swap(m_pIOData, rhs.m_pIOData);
	}

	return *this;
}


AsyncFileRequest::~AsyncFileRequest()
{
	Release();
}


bool AsyncFileRequest::IsComplete()
{
	if (!m_pIOData)
	{
		return true;
	}

	GetResult(false);

	return m_pIOData->m_IsDone;
}


u32 AsyncFileRequest::GetResult(bool wait)
{
	if (!m_pIOData)
	{
		return 0;
	}
	else if (m_pIOData->m_IsDone)
	{
		return m_pIOData->m_TotalBytesProcessed;
	}

	DWORD result = 0;
	bool done = false;
	do
	{
		if (GetOverlappedResult(m_pIOData->m_hFile, m_pIOData, &result, wait))
		{
			m_pIOData->m_TotalBytesProcessed += result;
			if (m_pIOData->m_TotalBytesProcessed < m_pIOData->m_TotalBytesRequested)
			{
				// Issue another operation for the remaining bytes
				m_pIOData->Pointer = reinterpret_cast<PVOID>(u64(m_pIOData->Pointer) + result);

				BOOL status = m_pIOData->m_IsReadOp ? Internal::ReadAsync(*m_pIOData, m_pIOData->m_TotalBytesRequested - m_pIOData->m_TotalBytesProcessed)
					: Internal::WriteAsync(*m_pIOData, m_pIOData->m_TotalBytesRequested - m_pIOData->m_TotalBytesProcessed);
				if (!status)
				{
					auto error = GetLastError();
					if (error != ERROR_IO_PENDING/* && error != ERROR_HANDLE_EOF*/)
					{
						// TODO: Handle Error
						m_pIOData->m_IsDone = true;

						done = true;
					}
					else
					{
						done = !wait;
					}
				}
				else
				{
					done = false;
				}
			}
			else
			{
				m_pIOData->m_IsDone = true;
				done = true;
			}
		}
		else
		{
			auto error = GetLastError();
			if (error != ERROR_IO_INCOMPLETE)
			{
				if (error != ERROR_HANDLE_EOF && error != ERROR_OPERATION_ABORTED)
				{
					// TODO: Handle error
				}
				m_pIOData->m_IsDone = true;
			}
			done = true;
		}
	} while (!done);

	return m_pIOData->m_TotalBytesProcessed;
}


void AsyncFileRequest::Cancel()
{
	if (!m_pIOData)
	{
		return;
	}

	if (!CancelIoEx(m_pIOData->m_hFile, m_pIOData))
	{
		auto error = GetLastError();
		if (error != ERROR_NOT_FOUND && error != ERROR_OPERATION_ABORTED)
		{
			// TODO: Handle error
		}
	}
	GetResult();
}


void AsyncFileRequest::Release()
{
	if (m_pIOData)
	{
		BV_DELETE(m_pIOData);
		m_pIOData = nullptr;
	}
}


BvAsyncFile::BvAsyncFile()
{
}


BvAsyncFile::BvAsyncFile(const char* const pFilename, BvFileAccessMode mode, BvFileAction action, BvAsyncFileFlags asyncFlags)
{
	Open(pFilename, mode, action, asyncFlags);
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


bool BvAsyncFile::Open(const char* const pFilename, BvFileAccessMode mode, BvFileAction action, BvAsyncFileFlags asyncFlags)
{
	BV_ASSERT(pFilename != nullptr, "Invalid filename");

	DWORD acccessMode = 0;
	DWORD createMode = 0;
	DWORD createFlags = FILE_FLAG_OVERLAPPED;
	if (EHasFlag(asyncFlags, BvAsyncFileFlags::kNoBuffering))
	{
		createFlags |= FILE_FLAG_NO_BUFFERING;
	}
	if (EHasFlag(asyncFlags, BvAsyncFileFlags::kImmediateFlush))
	{
		createFlags |= FILE_FLAG_WRITE_THROUGH;
	}

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

	{
		auto sizeNeeded = BvTextUtilities::ConvertUTF8CharToWideChar(pFilename, 0, nullptr, 0);
		wchar_t* pFilenameW = (wchar_t*)BV_STACK_ALLOC(sizeNeeded * sizeof(wchar_t));
		BvTextUtilities::ConvertUTF8CharToWideChar(pFilename, 0, pFilenameW, sizeNeeded);
		m_hFile = CreateFileW(pFilenameW, acccessMode, 0, nullptr, createMode, createFlags, nullptr);
	}

	if (m_hFile == INVALID_HANDLE_VALUE)
	{
		// TODO: Handle error
		
		return false;
	}

	return true;
}


AsyncFileRequest BvAsyncFile::Read(void * const pBuffer, const u32 bufferSize, const u64 position)
{
	BV_ASSERT(m_hFile != INVALID_HANDLE_VALUE, "Invalid file handle");
	BV_ASSERT(pBuffer != nullptr, "Null buffer");
	BV_ASSERT(bufferSize > 0, "Invalid buffer size");

	AsyncFileRequest request;
	request.m_pIOData = BV_NEW(AsyncFileData);
	ZeroMemory(request.m_pIOData, sizeof(AsyncFileData));
	request.m_pIOData->m_hFile = m_hFile;
	request.m_pIOData->Pointer = reinterpret_cast<void*>(position);
	request.m_pIOData->m_pBuffer = pBuffer;
	request.m_pIOData->m_TotalBytesRequested = bufferSize;
	//request.m_pIOData->m_TotalBytesProcessed = 0;
	request.m_pIOData->m_IsReadOp = true;
	//request.m_pIOData->m_IsDone = false;

	BOOL status = Internal::ReadAsync(*request.m_pIOData, bufferSize);
	if (!status)
	{
		auto error = GetLastError();
		if (error != ERROR_IO_PENDING && error != ERROR_HANDLE_EOF)
		{
			request.Release();
			// TODO: Handle Error
		}
	}

	return request;
}


AsyncFileRequest BvAsyncFile::Write(const void * const pBuffer, const u32 bufferSize, const u64 position)
{
	BV_ASSERT(m_hFile != INVALID_HANDLE_VALUE, "Invalid file handle");
	BV_ASSERT(pBuffer != nullptr, "Null buffer");
	BV_ASSERT(bufferSize > 0, "Invalid buffer size");

	AsyncFileRequest request;
	request.m_pIOData = BV_NEW(AsyncFileData);
	ZeroMemory(request.m_pIOData, sizeof(AsyncFileData));
	request.m_pIOData->m_hFile = m_hFile;
	request.m_pIOData->Pointer = reinterpret_cast<void*>(position);
	request.m_pIOData->m_pBuffer = const_cast<void* const>(pBuffer);
	request.m_pIOData->m_TotalBytesRequested = bufferSize;
	//request.m_pIOData->m_TotalBytesProcessed = 0;
	//request.m_pIOData->m_IsReadOp = false;
	//request.m_pIOData->m_IsDone = false;

	BOOL status = Internal::WriteAsync(*request.m_pIOData, bufferSize);
	if (!status)
	{
		auto error = GetLastError();
		if (error != ERROR_IO_PENDING && error != ERROR_HANDLE_EOF)
		{
			request.Release();
			// TODO: Handle Error
		}
	}

	return request;
}


u64 BvAsyncFile::GetSize() const
{
	BV_ASSERT(m_hFile != INVALID_HANDLE_VALUE, "Invalid file handle");

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


void BvAsyncFile::Flush()
{
	BV_ASSERT(m_hFile != INVALID_HANDLE_VALUE, "Invalid file handle");

	BOOL status = FlushFileBuffers(m_hFile);
	if (!status)
	{
		// TODO: Handle Error
	}
}


void BvAsyncFile::GetInfo(BvFileInfo& fileInfo)
{
	BV_ASSERT(m_hFile != INVALID_HANDLE_VALUE, "Invalid file handle");

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