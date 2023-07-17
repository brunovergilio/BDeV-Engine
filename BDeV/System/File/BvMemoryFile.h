#pragma once


#include "BDeV/Utils/BvUtils.h"


class BvMemoryFile
{
public:
	BvMemoryFile();
	BvMemoryFile(size_t size, i64 currPos = 0);
	BvMemoryFile(const u8* const pData, size_t size, i64 currPos = 0);
	BvMemoryFile(BvMemoryFile&& rhs) noexcept;
	BvMemoryFile& operator =(BvMemoryFile&& rhs) noexcept;
	~BvMemoryFile();

	BvMemoryFile(const BvMemoryFile&) = delete;
	BvMemoryFile& operator =(const BvMemoryFile&) = delete;

	u32 Read(void* const pBuffer, const u32 bufferSize);
	u32 Write(const void* const pBuffer, const u32 bufferSize);

	template<typename Type> u32 ReadT(Type& value) { return Read(&value, sizeof(Type)); }
	template<typename Type> u32 WriteT(const Type& value) { return Write(&value, sizeof(Type)); }

	BvMemoryFile& SkipBytes(const i64 offset);
	BvMemoryFile& GoToStart();
	BvMemoryFile& GoToEnd();

	const u64 GetFilePos() const;
	const u64 GetSize() const;

	const bool IsValid() const;

private:
	u8* m_pData = nullptr;
	u64 m_Size = 0;
	i64 m_Pos = 0;
};