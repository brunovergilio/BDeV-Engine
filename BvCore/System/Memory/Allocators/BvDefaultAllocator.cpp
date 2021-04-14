#include "BvDefaultAllocator.h"
#include "BvCore/System/Memory/BvAlloc.h"


BvDefaultAllocator::BvDefaultAllocator()
{
}


BvDefaultAllocator::BvDefaultAllocator(void* pStart, void* pEnd)
{
}


BvDefaultAllocator::~BvDefaultAllocator()
{
}


void* BvDefaultAllocator::Allocate(size_t size, size_t alignment, size_t offset /*= 0*/)
{
	return BvMAlloc(size, alignment);
}


void BvDefaultAllocator::Free(void* ptr)
{
	BvFree(ptr);
}