#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Utils/BvUtils.h"


// 2 kb should be enough for a file path (can always increase it if needed)
constexpr size_t kMaxPathSize = 2048;


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


struct BvFileInfo
{
	u64 m_CreationTimestamp;
	u64 m_LastAccessTimestamp;
	u64 m_LastWriteTimestamp;
	u64 m_FileSize;
};