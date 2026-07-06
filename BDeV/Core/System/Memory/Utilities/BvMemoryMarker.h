#pragma once


#include "BDeV/Core/System/Memory/BvMemory.h"


class BvMemoryMarker
{
	BV_NOCOPYMOVE(BvMemoryMarker);
public:
	BvMemoryMarker();
	~BvMemoryMarker();

	void MarkAllocation(void* pMem, size_t size);
	void MarkDeallocation(void* pMem, size_t size);
};