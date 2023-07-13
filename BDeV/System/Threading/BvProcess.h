#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Container/BvString.h"
#include "BDeV/Container/BvVector.h"


struct BvSystemInfo
{
	u32 m_NumCores;
	u32 m_NumLogicalProcessors;
	u32 m_PageSize;
	u32 m_LargePageSize;
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


class BV_API BvProcess final
{
public:
	static const BvSystemInfo& GetSystemInfo();
	static void GetStackTrace(BvStackTrace& stackTrace, const u32 numFramesToSkip = 1, const u32 numFramesToRecord = 3);
	static void YieldExecution();
};