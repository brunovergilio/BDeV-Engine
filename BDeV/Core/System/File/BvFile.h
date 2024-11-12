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
	BvFile(const char* const pFilename, BvFileAccessMode mode = BvFileAccessMode::kReadWrite, BvFileAction action = BvFileAction::kOpenOrCreate);
	BvFile(BvFile&& rhs) noexcept;
	BvFile& operator=(BvFile&& rhs) noexcept;
	~BvFile();

	bool Open(const char* const pFilename, BvFileAccessMode mode = BvFileAccessMode::kReadWrite, BvFileAction action = BvFileAction::kOpenOrCreate);

	u32 Read(void* const pBuffer, const u32 bufferSize);
	u32 Write(const void* const pBuffer, const u32 bufferSize);

	template<typename Type> u32 ReadT(Type& value) { return Read(&value, sizeof(Type)); }
	template<typename Type> u32 WriteT(const Type& value) { return Write(&value, sizeof(Type)); }

	BvFile& SkipBytes(const i64 offset);
	BvFile& GoToStart();
	BvFile& GoToEnd();

	u64 GetFilePos() const;
	u64 GetSize() const;

	void Close();
	void Flush();
	void GetInfo(BvFileInfo& fileInfo);
	bool IsValid() const;

	OSFileHandle GetHandle() const { return m_hFile; }

private:
	OSFileHandle m_hFile = kNullOSFileHandle;
};