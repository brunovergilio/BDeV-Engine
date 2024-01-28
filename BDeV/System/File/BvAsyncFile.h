#pragma once


// This type of file should be used for real-asynchronous operations,
// using aligned sector-sized blocks for read/write operations
#include "BDeV/BvCore.h"
#include "BDeV/System/File/BvFileCommon.h"


struct AsyncFileData;


class AsyncFileRequest
{
public:
	friend class BvAsyncFile;

	AsyncFileRequest();
	AsyncFileRequest(const AsyncFileRequest&);
	AsyncFileRequest& operator =(const AsyncFileRequest&);
	AsyncFileRequest(AsyncFileRequest&& rhs) noexcept;
	AsyncFileRequest& operator =(AsyncFileRequest&& rhs) noexcept;
	~AsyncFileRequest();

	i32 GetResult(const bool wait = true);
	void Cancel();

private:
	AsyncFileData* m_pIOData = nullptr;
};


class BvAsyncFile
{
public:
	friend class BvFileSystem;

	BvAsyncFile();
	BvAsyncFile(const char* const pFilename, BvFileAccessMode mode = BvFileAccessMode::kReadWrite, BvFileAction action = BvFileAction::kOpenOrCreate);
	BvAsyncFile(const wchar_t* const pFilename, BvFileAccessMode mode = BvFileAccessMode::kReadWrite, BvFileAction action = BvFileAction::kOpenOrCreate);
	BvAsyncFile(BvAsyncFile&& rhs) noexcept;
	BvAsyncFile& operator =(BvAsyncFile&& rhs) noexcept;
	~BvAsyncFile();

	BvAsyncFile(const BvAsyncFile&) = delete;
	BvAsyncFile& operator =(const BvAsyncFile&) = delete;

	bool Open(const char* const pFilename, BvFileAccessMode mode = BvFileAccessMode::kReadWrite, BvFileAction action = BvFileAction::kOpenOrCreate);
	bool Open(const wchar_t* const pFilename, BvFileAccessMode mode = BvFileAccessMode::kReadWrite, BvFileAction action = BvFileAction::kOpenOrCreate);

	AsyncFileRequest Read(void* const pBuffer, const u32 bufferSize, const u64 position = 0);
	AsyncFileRequest Write(const void* const pBuffer, const u32 bufferSize, const u64 position = 0);

	template<typename Type> AsyncFileRequest ReadT(Type& value, const u64 position = 0) { return Read(&value, sizeof(Type), position); }
	template<typename Type> AsyncFileRequest WriteT(const Type& value, const u64 position = 0) { return Write(&value, sizeof(Type), position); }

	const u64 GetSize() const;

	void Close();

	void GetFileInfo(BvFileInfo& fileInfo);

	bool IsValid() const;

#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	void* GetHandle() const { return m_hFile; }
#endif

private:
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	void* m_hFile = ((void*)-1ll);
#endif
};