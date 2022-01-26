#pragma once


#include "BvCore/Utils/BvUtils.h"
#include <Windows.h>
#include "BvCore/System/File/BvFileCommon.h"


class BvFile
{
public:
	friend class BvFileSystem;

	BvFile();
	BvFile(const char* const pFilename, BvFileAccessMode mode = BvFileAccessMode::kReadWrite, BvFileAction action = BvFileAction::kOpenOrCreate);
	BvFile(const wchar_t* const pFilename, BvFileAccessMode mode = BvFileAccessMode::kReadWrite, BvFileAction action = BvFileAction::kOpenOrCreate);
	BvFile(BvFile && rhs) noexcept;
	BvFile & operator =(BvFile && rhs) noexcept;
	~BvFile();

	BvFile(const BvFile &) = delete;
	BvFile & operator =(const BvFile &) = delete;

	bool Open(const char* const pFilename, BvFileAccessMode mode = BvFileAccessMode::kReadWrite, BvFileAction action = BvFileAction::kOpenOrCreate);
	bool Open(const wchar_t* const pFilename, BvFileAccessMode mode = BvFileAccessMode::kReadWrite, BvFileAction action = BvFileAction::kOpenOrCreate);

	u32 Read(void * const pBuffer, const u32 bufferSize);
	u32 Write(const void * const pBuffer, const u32 bufferSize);

	template<typename Type> u32 ReadT(Type & value) { return Read(&value, sizeof(Type)); }
	template<typename Type> u32 WriteT(const Type & value) { return Write(&value, sizeof(Type)); }

	BvFile & SkipBytes(const i64 offset);
	BvFile & GoToStart();
	BvFile & GoToEnd();

	const u64 GetFilePos() const;
	const u64 GetSize() const;

	void Close();

	const bool IsValid() const;

	HANDLE GetHandle() const { return m_hFile; }

private:
	HANDLE m_hFile = INVALID_HANDLE_VALUE;
};