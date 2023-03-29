#include "BvDefaultAllocator.h"
#include "BDeV/System/Memory/BvAlloc.h"


BvDefaultAllocator::BvDefaultAllocator()
{
}


BvDefaultAllocator::~BvDefaultAllocator()
{
}


void* BvDefaultAllocator::Allocate(size_t size, size_t alignment, size_t alignmentOffset)
{
	return BvMAlloc(size, alignment, alignmentOffset);
}


void BvDefaultAllocator::Free(void* ptr)
{
	BvFree(ptr);
}