#pragma once


#include "BDeV/System/Memory/BvMemoryCommon.h"


class BvNoMemoryMarker
{
	BV_NOCOPYMOVE(BvNoMemoryMarker);
public:
	BvNoMemoryMarker() {}
	~BvNoMemoryMarker() {}

	BV_INLINE void MarkAllocation(void* pMem, size_t size) {}
	BV_INLINE void MarkDeallocation(void* pMem, size_t size) {}
};


class BvMemoryMarker
{
	BV_NOCOPYMOVE(BvMemoryMarker);
public:
	BvMemoryMarker();
	~BvMemoryMarker();

	void MarkAllocation(void* pMem, size_t size);
	void MarkDeallocation(void* pMem, size_t size);
};