#pragma once


#include "BvCore/BvDefines.h"
#include "BvCore/System/Threading/BvSync.h"
#include "BvCore/Utils/BvEnum.h"
#include "BvCore/System/File/AsyncIO.h"


class BvFile
{
public:
	friend class FileSystem;

	BvFile() {};
	BvFile(const char * const pFilename, const BvFileAccess mode = BvFileAccess::kReadAndWrite);
	BvFile(BvFile && rhs);
	BvFile & operator =(BvFile && rhs);
	~BvFile();

	BvFile(const BvFile &) = delete;
	BvFile & operator =(const BvFile &) = delete;

	// Basic operations
	BvFileResult Open(const char * const pFilename, const BvFileAccess mode = BvFileAccess::kReadAndWrite);
	void Close();

	// Synchronous IO
	BvFileResultT Read(void * const pBuffer, const u32 bufferSize, u32 * const pBytesRead = nullptr);
	BvFileResultT Write(const void * const pBuffer, const u32 bufferSize, u32 * const pBytesWritten = nullptr);

	AsyncIOResult ReadAsync(void * const pBuffer, const u32 bufferSize, pFnAsyncIOCallback pCallback = nullptr, void * const pData = nullptr);
	AsyncIOResult WriteAsync(void * const pBuffer, const u32 bufferSize, pFnAsyncIOCallback pCallback = nullptr, void * const pData = nullptr);

	BvFileResult Seek(const i64 offset, const BvFileSeek seekFrom);
	BvFileResult SkipBytes(const i64 offset);
	BvFileResult GoToStart();
	BvFileResult GoToEnd();

	const u64 GetFilePos() const;
	const u64 GetSize() const;

	const bool IsValid() const;

private:
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	BvFile(HANDLE hFile)
		: m_hFile(hFile) {}
#endif

private:
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	HANDLE m_hFile = INVALID_HANDLE_VALUE;
#endif
};