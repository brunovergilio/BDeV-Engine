#pragma once


// This type of file should be used for real-asynchronous operations,
// using aligned sector-sized blocks for read/write operations

#include "BvCore/BvPlatform.h"


struct AsyncFileData;


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

	u32 GetResult(const bool wait = true);
	void Cancel();

private:
	AsyncFileData * m_pIOData = nullptr;
};


class BvAsyncFile
{
public:
	friend class BvFileSystem;

	BvAsyncFile();
	BvAsyncFile(BvAsyncFile && rhs);
	BvAsyncFile & operator =(BvAsyncFile && rhs);
	~BvAsyncFile();

	BvAsyncFile(const BvAsyncFile &) = delete;
	BvAsyncFile & operator =(const BvAsyncFile &) = delete;

	AsyncFileRequest Read(void * const pBuffer, const u32 bufferLength, const u64 position = 0);
	AsyncFileRequest Write(const void * const pBuffer, const u32 bufferLength, const u64 position = 0);

	template<typename Type> u32 ReadT(Type & value, const u64 position = 0) { return Read(&value, sizeof(Type), position); }
	template<typename Type> u32 WriteT(const Type & value, const u64 position = 0) { return Write(&value, sizeof(Type), position); }

	const u64 GetSize() const;

	const bool IsValid() const;

private:
	BvAsyncFile(FileHandle hFile)
		: m_hFile(hFile) {}

private:
	FileHandle m_hFile;
};