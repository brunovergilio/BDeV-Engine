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


struct BvFileInfo
{
	u64 m_CreationTimestamp;
	u64 m_LastAccessTimestamp;
	u64 m_LastWriteTimestamp;
	u64 m_FileSize;
};