#pragma once


#include "BvCore/BvCore.h"
#include "BvCore/Container/BvString.h"
#include "BvCore/Container/BvVector.h"


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


const BvSystemInfo& GetSystemInfo();
void GetStackTrace(BvStackTrace& stackTrace, const u32 numFramesToSkip = 1, const u32 numFramesToRecord = 3);