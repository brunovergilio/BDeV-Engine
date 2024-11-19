#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Container/BvString.h"
#include "BDeV/Core/Container/BvVector.h"


struct BvSystemInfo
{
	u32 m_NumCores;
	u32 m_NumLogicalProcessors;
	u32 m_L1CacheCount;
	u32 m_L2CacheCount;
	u32 m_L3CacheCount;
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


namespace BvSystem
{
	const BvSystemInfo& GetSystemInfo();
	void GetStackTrace(BvStackTrace& stackTrace, const u32 numFramesToSkip = 1, const u32 numFramesToRecord = 3);
};


namespace BvCPU
{
	void Yield();
}