#include "BvDefaultAllocator.h"

BvDefaultAllocator::BvDefaultAllocator()
{
}


BvDefaultAllocator::~BvDefaultAllocator()
{
}


void* BvDefaultAllocator::Allocate(size_t size, size_t alignment, size_t offset /*= 0*/)
{
	size += alignment + offset;
	return BvAlloc(size, alignment);
}


void BvDefaultAllocator::Free(void* ptr)
{
	BvFree(ptr);
}