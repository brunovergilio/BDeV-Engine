#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Utils/BvUtils.h"


namespace BvVirtualMemory
{
	void* Reserve(size_t size);
	void Commit(void* pAddress, size_t size);
	void* ReserveAndCommit(size_t size, bool useLargePage = false);
	void Decommit(void* pAddress, size_t size);
	void Release(void* pAddress);
};