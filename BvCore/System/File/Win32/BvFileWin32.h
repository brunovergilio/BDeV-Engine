#pragma once


#include "BvCore/Utils/BvUtils.h"
#include <Windows.h>


enum class BvFileAccess : u8
{
	kRead		= BvBit(0),
	kWrite		= BvBit(1),
	kReadWrite	= kRead | kWrite,
};


class BvFile
{
public:
	friend class BvFileSystem;

	BvFile();
	BvFile(BvFile && rhs) noexcept;
	BvFile & operator =(BvFile && rhs) noexcept;
	~BvFile();

	BvFile(const BvFile &) = delete;
	BvFile & operator =(const BvFile &) = delete;

	u32 Read(void * const pBuffer, const u32 bufferSize);
	u32 Write(const void * const pBuffer, const u32 bufferSize);

	template<typename Type> u32 ReadT(Type & value) { return Read(&value, sizeof(Type)); }
	template<typename Type> u32 WriteT(const Type & value) { return Write(&value, sizeof(Type)); }

	BvFile & SkipBytes(const i64 offset);
	BvFile & GoToStart();
	BvFile & GoToEnd();

	const u64 GetFilePos() const;
	const u64 GetSize() const;

	const bool IsValid() const;

	HANDLE GetHandle() const { return m_hFile; }

private:
	BvFile(HANDLE hFile)
		: m_hFile(hFile) {}

private:
	HANDLE m_hFile;
};