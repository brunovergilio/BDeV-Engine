#pragma once


#include "BvCore/BvCore.h"
#include "BvCore/Utils/BvUtils.h"


// 2 kb should be enough for a file path (can always increase it if needed)
constexpr size_t kMaxPathSize = 2048;
constexpr size_t kMaxFilenameSize = 64;


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