#include "BDeV/Core/System/File/BvAsyncFile.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BDeV/Core/System/Threading/BvSync.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Utils/BvText.h"
#include "BDeV/Core/System/Memory/BvMemory.h"


BvAsyncFileRequest::BvAsyncFileRequest()
{
	m_AsyncFileData.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
	IsDone(true);
}


BvAsyncFileRequest::~BvAsyncFileRequest()
{
	CloseHandle(m_AsyncFileData.hEvent);
}


bool BvAsyncFileRequest::IsDone()
{
	return m_AsyncFileData.m_IsDone.load(std::memory_order::acquire) || WaitForSingleObject(m_AsyncFileData.hEvent, 0) == WAIT_OBJECT_0;
}


u32 BvAsyncFileRequest::GetResult(bool wait)
{
	if (m_AsyncFileData.m_IsDone.load(std::memory_order::acquire))
	{
		return m_AsyncFileData.m_TotalBytesProcessed.load(std::memory_order::relaxed);
	}

	DWORD waitResult = WaitForSingleObject(m_AsyncFileData.hEvent, wait ? INFINITE : 0);
	if (waitResult == WAIT_OBJECT_0)
	{
		bool exp = false;
		if (m_AsyncFileData.m_Flag.compare_exchange_strong(exp, true, std::memory_order::acquire))
		{
			DWORD bytesTransferred = 0;
			BOOL result = GetOverlappedResult(m_AsyncFileData.m_hFile, &m_AsyncFileData, &bytesTransferred, FALSE);
			m_AsyncFileData.m_TotalBytesProcessed.store(result * bytesTransferred, std::memory_order::relaxed);
			IsDone(true);
		}

		return m_AsyncFileData.m_TotalBytesProcessed.load(std::memory_order::relaxed);
	}
	else
	{
		// Operation is still pending
		return 0;
	}
}


void BvAsyncFileRequest::Cancel()
{
	if (!IsDone())
	{
		CancelIoEx(m_AsyncFileData.m_hFile, &m_AsyncFileData);
	}
}


void BvAsyncFileRequest::IsDone(bool done)
{
	m_AsyncFileData.m_IsDone.store(done, std::memory_order::release);
}


void BvAsyncFileRequest::Reset(HANDLE hFile, u64 position)
{
	auto hEvent = m_AsyncFileData.hEvent;
	// We use sizeof(OVERLAPPED) intentionally here
	ZeroMemory(&m_AsyncFileData, sizeof(OVERLAPPED));
	m_AsyncFileData.hEvent = hEvent;

	m_AsyncFileData.m_hFile = hFile;
	m_AsyncFileData.Pointer = reinterpret_cast<PVOID>(position);
	m_AsyncFileData.m_TotalBytesProcessed.store(0, std::memory_order::relaxed);
	m_AsyncFileData.m_Flag.store(false, std::memory_order::release);
	IsDone(false);
	ResetEvent(m_AsyncFileData.hEvent);
}


BvAsyncFile::BvAsyncFile()
{
}


BvAsyncFile::BvAsyncFile(const char* const pFilename, BvFileAccessMode mode, BvFileAction action, BvAsyncFileFlags asyncFlags)
{
	Open(pFilename, mode, action, asyncFlags);
}


BvAsyncFile::BvAsyncFile(BvAsyncFile && rhs) noexcept
	: m_hFile(rhs.m_hFile)
{
	rhs.m_hFile = INVALID_HANDLE_VALUE;
}


BvAsyncFile & BvAsyncFile::operator =(BvAsyncFile && rhs) noexcept
{
	if (this != &rhs)
	{
		Close();

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
		BV_SYS_ERROR();
		
		return false;
	}

	return true;
}


bool BvAsyncFile::Read(BvAsyncFileRequest& request, void* pBuffer, u32 bufferSize, u64 position)
{
	BV_ASSERT(m_hFile != INVALID_HANDLE_VALUE, "Invalid file handle");
	BV_ASSERT(pBuffer != nullptr, "Null buffer");
	BV_ASSERT(bufferSize > 0, "Invalid buffer size");
	BV_ASSERT(request.IsDone(), "Operation is still in use");

	request.Reset(m_hFile, position);

	BOOL status = ReadFile(m_hFile, pBuffer, bufferSize, nullptr, &request.m_AsyncFileData);
	if (!status)
	{
		auto error = GetLastError();
		if (error != ERROR_IO_PENDING && error != ERROR_HANDLE_EOF)
		{
			BV_SYS_ERROR();
			return false;
		}
	}

	return true;
}


bool BvAsyncFile::Write(BvAsyncFileRequest& request, const void* pBuffer, u32 bufferSize, u64 position)
{
	BV_ASSERT(m_hFile != INVALID_HANDLE_VALUE, "Invalid file handle");
	BV_ASSERT(pBuffer != nullptr, "Null buffer");
	BV_ASSERT(bufferSize > 0, "Invalid buffer size");
	BV_ASSERT(request.IsDone(), "Operation is still in use");

	request.Reset(m_hFile, position);

	BOOL status = WriteFile(m_hFile, pBuffer, bufferSize, nullptr, &request.m_AsyncFileData);
	if (!status)
	{
		auto error = GetLastError();
		if (error != ERROR_IO_PENDING && error != ERROR_HANDLE_EOF)
		{
			BV_SYS_ERROR();
			return false;
		}
	}

	return true;
}


u64 BvAsyncFile::GetSize() const
{
	BV_ASSERT(m_hFile != INVALID_HANDLE_VALUE, "Invalid file handle");

	i64 fileSize = 0;
	BOOL status = GetFileSizeEx(m_hFile, reinterpret_cast<PLARGE_INTEGER>(&fileSize));
	if (!status)
	{
		BV_SYS_ERROR();
	}

	return u64(fileSize);
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
		BV_SYS_ERROR();
	}
}


bool BvAsyncFile::GetInfo(BvFileInfo& fileInfo)
{
	BV_ASSERT(m_hFile != INVALID_HANDLE_VALUE, "Invalid file handle");

	BY_HANDLE_FILE_INFORMATION bhfi;
	if (!GetFileInformationByHandle(m_hFile, &bhfi))
	{
		BV_SYS_ERROR();
		return false;
	}

	fileInfo.m_CreationTimestamp = *reinterpret_cast<u64*>(&bhfi.ftCreationTime);
	fileInfo.m_LastAccessTimestamp = *reinterpret_cast<u64*>(&bhfi.ftLastAccessTime);
	fileInfo.m_LastWriteTimestamp = *reinterpret_cast<u64*>(&bhfi.ftLastWriteTime);
	fileInfo.m_FileSize = (static_cast<u64>(bhfi.nFileSizeHigh) << 32) | static_cast<u64>(bhfi.nFileSizeLow);

	return true;
}


bool BvAsyncFile::IsValid() const
{
	return m_hFile != INVALID_HANDLE_VALUE;
}