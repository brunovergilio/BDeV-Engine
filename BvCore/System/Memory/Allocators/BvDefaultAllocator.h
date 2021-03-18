#pragma once


#include "BvCore/System/Memory/BvMemory.h"


class BvDefaultAllocator
{
public:
	BvDefaultAllocator();
	~BvDefaultAllocator();

	void* Allocate(size_t size, size_t alignment, size_t offset = 0);
	void Free(void* ptr);
};