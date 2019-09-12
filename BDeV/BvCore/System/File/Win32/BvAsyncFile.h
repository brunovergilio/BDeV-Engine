#pragma once


// This type of file should be used for real-asynchronous operations,
// using aligned sector-sized blocks for read/write operations

#include "BvCore/BvDefines.h"
#include "BvCore/System/Threading/BvSync.h"
#include "BvCore/Utils/BvEnum.h"
#include "BvCore/System/File/BvFileDefs.h"


#if (BV_PLATFORM == BV_PLATFORM_WIN32)


class AsyncFileRequest
{
public:
	friend class BvAsyncFile;

	AsyncFileRequest();
	AsyncFileRequest(const AsyncFileRequest &);
	AsyncFileRequest & operator =(const AsyncFileRequest &);
	AsyncFileRequest(AsyncFileRequest && rhs);
	AsyncFileRequest & operator =(AsyncFileRequest && rhs);
	~AsyncFileRequest();

	BvFileResultT GetResult(const bool waitForIt = true);
	BV_INLINE u64 GetBytesTransfered() const { return m_BytesTransfered; }

private:
	struct AsyncFileData
	{
		BvEvent m_Signal;
		OVERLAPPED m_OverlappedIO{};
		BvAtomic m_InUse;

		AsyncFileData()
			: m_Signal(BvEvent(true)) {}
	};

	AsyncFileData * m_pIOData = nullptr;
	HANDLE m_hFile = INVALID_HANDLE_VALUE;
	u32 m_BytesTransfered = 0;
	BvFileResultT m_Result = EnumVal(BvFileResult::kOk);
};


class BvAsyncFile
{
public:
	friend class FileSystem;

	BvAsyncFile() {};
	BvAsyncFile(const char * const pFilename, const BvFileAccess mode = BvFileAccess::kReadAndWrite);
	BvAsyncFile(BvAsyncFile && rhs);
	BvAsyncFile & operator =(BvAsyncFile && rhs);
	~BvAsyncFile();

	BvAsyncFile(const BvAsyncFile &) = delete;
	BvAsyncFile & operator =(const BvAsyncFile &) = delete;

	// Basic operations
	BvFileResult Open(const char * const pFilename, const BvFileAccess mode = BvFileAccess::kReadAndWrite);
	void Close();

	// Asynchronous IO
	AsyncFileRequest Read(void * const pBuffer, const u32 bufferLength, const u64 position = 0);
	AsyncFileRequest Write(const void * const pBuffer, const u32 bufferLength, const u64 position = 0);

	const u64 GetSize() const;

	BV_INLINE const bool IsValid() const { return m_hFile != INVALID_HANDLE_VALUE; }

private:
	BvAsyncFile(HANDLE hFile)
		: m_hFile(hFile) {}

private:
	HANDLE m_hFile = INVALID_HANDLE_VALUE;
};


#else
#error "Platform not yet supported"
#endif // #if (BV_PLATFORM == BV_PLATFORM_WIN32)