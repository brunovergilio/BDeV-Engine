#pragma once


// This type of file should be used for real-asynchronous operations,
// using aligned sector-sized blocks for read/write operations
#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/File/BvFileCommon.h"
#include "BDeV/Core/System/BvPlatformHeaders.h"


//Feature						Windows					Linux						macOS
//Unbuffered I/O				FILE_FLAG_NO_BUFFERING	O_DIRECT					No exact equivalent; try F_NOCACHE with fcntl()
//Immediate Write - Through		FILE_FLAG_WRITE_THROUGH	O_SYNC / O_DSYNC			O_SYNC / O_DSYNC
//Asynchronous I/O Libraries	Built - in async API	libaio or POSIX AIO(aio_*)	POSIX AIO(aio_*)


class AsyncFileRequest
{
public:
	friend class BvAsyncFile;

	AsyncFileRequest();
	AsyncFileRequest(AsyncFileRequest&& rhs) noexcept;
	AsyncFileRequest& operator=(AsyncFileRequest&& rhs) noexcept;
	~AsyncFileRequest();

	bool IsComplete();
	u32 GetResult(bool wait = true);
	void Cancel();
	bool IsValid() const;

private:
	struct AsyncFileData* m_pIOData = nullptr;
};


class BvAsyncFile
{
	BV_NOCOPY(BvAsyncFile);

public:
	friend class BvFileSystem;

	BvAsyncFile();
	BvAsyncFile(const char* const pFilename, BvFileAccessMode mode = BvFileAccessMode::kReadWrite,
		BvFileAction action = BvFileAction::kOpenOrCreate, BvAsyncFileFlags asyncFlags = BvAsyncFileFlags::kNone);
	BvAsyncFile(BvAsyncFile&& rhs) noexcept;
	BvAsyncFile& operator=(BvAsyncFile&& rhs) noexcept;
	~BvAsyncFile();

	bool Open(const char* const pFilename, BvFileAccessMode mode = BvFileAccessMode::kReadWrite,
		BvFileAction action = BvFileAction::kOpenOrCreate, BvAsyncFileFlags asyncFlags = BvAsyncFileFlags::kNone);

	AsyncFileRequest Read(void* pBuffer, u32 bufferSize, u64 position = 0);
	AsyncFileRequest Write(const void* pBuffer, u32 bufferSize, u64 position = 0);

	template<typename Type> AsyncFileRequest ReadT(Type& value, u64 position = 0) { return Read(&value, sizeof(Type), position); }
	template<typename Type> AsyncFileRequest WriteT(const Type& value, u64 position = 0) { return Write(&value, sizeof(Type), position); }

	u64 GetSize() const;

	void Close();
	void Flush();
	bool GetInfo(BvFileInfo& fileInfo);
	bool IsValid() const;

	OSFileHandle GetHandle() const { return m_hFile; }

private:
	OSFileHandle m_hFile = kNullOSFileHandle;
};