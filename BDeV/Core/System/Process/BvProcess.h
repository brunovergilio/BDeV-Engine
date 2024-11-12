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


class BvPipe
{
	BV_NOCOPY(BvPipe);

public:
	BvPipe();
	BvPipe(bool isWritePipe);
	BvPipe(BvPipe&& rhs);
	BvPipe& operator=(BvPipe&& rhs);
	~BvPipe();

private:
	void* m_pReadPipe = nullptr;
	void* m_pWritePipe = nullptr;
};


namespace BvProcess
{
	const BvSystemInfo& GetSystemInfo();
	void GetStackTrace(BvStackTrace& stackTrace, const u32 numFramesToSkip = 1, const u32 numFramesToRecord = 3);
	void Yield();
};