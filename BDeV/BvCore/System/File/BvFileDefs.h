#pragma once


#include "BvCore/Utils/BvEnum.h"
#include "BvCore/BvDefines.h"
#include "BvCore/Utils/BvUtils.h"


enum class BvFileAccess
{
	kRead,
	kWrite,
	kReadAndWrite
};


enum class BvFileSeek
{
	kBegin,
	kCurrent,
	kEnd
};


enum BvFileResult : u32
{
	kOk = 0,
	kDoneSync = BvBit(0),
	kDoneAsync = BvBit(1),
	kInProgress = BvBit(2),
	kEOF = BvBit(3),
	kError = BvBit(4),
	kIOSuccess = (1 | 2 | 8) //(kDoneSync | kDoneAsync | kEOF)
};
BV_USE_ENUM_CLASS_OPERATORS(BvFileResult);