#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/File/BvFileCommon.h"
#include "BDeV/Core/System/BvPlatformHeaders.h"


class BvFile
{
	BV_NOCOPY(BvFile);

public:
	friend class BvFileSystem;

	BvFile();
	BvFile(const char* pFilename, BvFileAccessMode mode = BvFileAccessMode::kReadWrite, BvFileAction action = BvFileAction::kOpenOrCreate);
	BvFile(BvFile&& rhs) noexcept;
	BvFile& operator=(BvFile&& rhs) noexcept;
	~BvFile();

	bool Open(const char* pFilename, BvFileAccessMode mode = BvFileAccessMode::kReadWrite, BvFileAction action = BvFileAction::kOpenOrCreate);

	bool Read(void* const pBuffer, u32 bufferSize, u32* pBytesProcessed = nullptr);
	bool Write(const void* pBuffer, u32 bufferSize, u32* pBytesProcessed = nullptr);

	template<typename Type> bool ReadT(Type& value) { return Read(&value, sizeof(Type)); }
	template<typename Type> bool WriteT(const Type& value) { return Write(&value, sizeof(Type)); }

	BvFile& SkipBytes(const i64 offset);
	BvFile& GoToStart();
	BvFile& GoToEnd();

	u64 GetFilePos() const;
	u64 GetSize() const;

	void Close();
	void Flush();
	bool GetInfo(BvFileInfo& fileInfo);
	bool IsValid() const;

	OSFileHandle GetHandle() const { return m_hFile; }

private:
	OSFileHandle m_hFile = kNullOSFileHandle;
};