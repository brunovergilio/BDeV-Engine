#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/File/BvFileCommon.h"


class BvFile
{
public:
	friend class BvFileSystem;

	BvFile();
	BvFile(const char* const pFilename, BvFileAccessMode mode = BvFileAccessMode::kReadWrite, BvFileAction action = BvFileAction::kOpenOrCreate);
	BvFile(BvFile&& rhs) noexcept;
	BvFile& operator =(BvFile&& rhs) noexcept;
	~BvFile();

	BvFile(const BvFile&) = delete;
	BvFile& operator =(const BvFile&) = delete;

	bool Open(const char* const pFilename, BvFileAccessMode mode = BvFileAccessMode::kReadWrite, BvFileAction action = BvFileAction::kOpenOrCreate);

	//static void ReadAllText(const char* const pFilename);
	//static void ReadAllText(const wchar_t* const pFilename);

	i32 Read(void* const pBuffer, const u32 bufferSize);
	i32 Write(const void* const pBuffer, const u32 bufferSize);

	template<typename Type> i32 ReadT(Type& value) { return Read(&value, sizeof(Type)); }
	template<typename Type> i32 WriteT(const Type& value) { return Write(&value, sizeof(Type)); }

	BvFile& SkipBytes(const i64 offset);
	BvFile& GoToStart();
	BvFile& GoToEnd();

	const i64 GetFilePos() const;
	const u64 GetSize() const;

	void Close();

	void Flush();

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