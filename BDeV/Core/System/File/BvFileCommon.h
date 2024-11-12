#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Utils/BvUtils.h"


enum class BvFileAccessMode : u8
{
	kRead = BvBit(0),
	kWrite = BvBit(1),
	kReadWrite = kRead | kWrite,
};


enum class BvFileAction : u8
{
	kOpen,
	kCreate,
	kOpenOrCreate
};


enum class BvAsyncFileFlags : u8
{
	kNone = 0,
	kNoBuffering = BvBit(0), // Requires that buffers for IO operations be aligned to the disk's physical sector size
	kImmediateFlush = BvBit(1) // Flushes data to disk as soon as a write operation finishes
};
BV_USE_ENUM_CLASS_OPERATORS(BvAsyncFileFlags);


struct BvFileInfo
{
	u64 m_CreationTimestamp;
	u64 m_LastAccessTimestamp;
	u64 m_LastWriteTimestamp;
	u64 m_FileSize;
};