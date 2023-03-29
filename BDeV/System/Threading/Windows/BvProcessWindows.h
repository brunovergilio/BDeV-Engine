#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Container/BvString.h"
#include "BDeV/Container/BvVector.h"


struct BvSystemInfo
{
	u32 m_NumCores;
	u32 m_NumLogicalProcessors;
	u32 m_MemPageSize;
};


struct BvStackTrace
{
	struct Frame
	{
		BvString m_Function;
		BvString m_Module;
		BvString m_File;
		u64 m_Address;
		u32 m_Line;
	};

	BvVector<Frame> frames;
};


BV_API const BvSystemInfo& GetSystemInfo();
BV_API void GetStackTrace(BvStackTrace& stackTrace, const u32 numFramesToSkip = 1, const u32 numFramesToRecord = 3);