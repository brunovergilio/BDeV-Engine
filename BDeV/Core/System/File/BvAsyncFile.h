#pragma once


// This type of file should be used for real-asynchronous operations,
// using aligned sector-sized blocks for read/write operations
#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/File/BvFileCommon.h"
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
#include "BDeV/Core/System/File/Windows/BvAsyncFileWindows.h"
#endif


//Feature						Windows					Linux						macOS
//Unbuffered I/O				FILE_FLAG_NO_BUFFERING	O_DIRECT					No exact equivalent; try F_NOCACHE with fcntl()
//Immediate Write - Through		FILE_FLAG_WRITE_THROUGH	O_SYNC / O_DSYNC			O_SYNC / O_DSYNC
//Asynchronous I/O Libraries	Built - in async API	libaio or POSIX AIO(aio_*)	POSIX AIO(aio_*)


class BvAsyncFile
{
	BV_NOCOPY(BvAsyncFile);

public:
	BvAsyncFile();
	BvAsyncFile(const char* const pFilename, BvFileAccessMode mode = BvFileAccessMode::kReadWrite,
		BvFileAction action = BvFileAction::kOpenOrCreate, BvAsyncFileFlags asyncFlags = BvAsyncFileFlags::kNone);
	BvAsyncFile(BvAsyncFile&& rhs) noexcept;
	BvAsyncFile& operator=(BvAsyncFile&& rhs) noexcept;
	~BvAsyncFile();

	bool Open(const char* const pFilename, BvFileAccessMode mode = BvFileAccessMode::kReadWrite,
		BvFileAction action = BvFileAction::kOpenOrCreate, BvAsyncFileFlags asyncFlags = BvAsyncFileFlags::kNone);

	bool Read(BvAsyncFileRequest& request, void* pBuffer, u32 bufferSize, u64 position = 0);
	bool Write(BvAsyncFileRequest& request, const void* pBuffer, u32 bufferSize, u64 position = 0);

	template<typename Type> bool ReadT(BvAsyncFileRequest& request, Type& value, u64 position = 0) { return Read(request, &value, sizeof(Type), position); }
	template<typename Type> bool WriteT(BvAsyncFileRequest& request, const Type& value, u64 position = 0) { return Write(request, &value, sizeof(Type), position); }

	u64 GetSize() const;

	void Close();
	void Flush();
	bool GetInfo(BvFileInfo& fileInfo);
	bool IsValid() const;

	OSFileHandle GetHandle() const { return m_hFile; }

private:
	OSFileHandle m_hFile = kNullOSFileHandle;
};