#include "BvDefaultAllocator.h"


BvDefaultAllocator::BvDefaultAllocator(void*, void*)
{
}


BvDefaultAllocator::~BvDefaultAllocator()
{
}


void* BvDefaultAllocator::Allocate(size_t size, size_t alignment /*= kDefaultAlignmentSize*/, size_t alignmentOffset /*= 0*/)
{
	return BvMemory::Allocate(size, alignment, alignmentOffset);
}


void BvDefaultAllocator::Free(void* pMem)
{
	return BvMemory::Free(pMem);
}


size_t BvDefaultAllocator::GetAllocationSize(void* pMem) const
{
	return BvMemory::GetAllocationSize(pMem);
}