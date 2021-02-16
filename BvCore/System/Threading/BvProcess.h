#pragma once


#include "BvCore/BvPlatform.h"


struct BvSystemInfo
{
	u32 m_NumCores;
	u32 m_NumLogicalProcessors;
	u32 m_MemPageSize;
};


const BvSystemInfo & GetSystemInfo();