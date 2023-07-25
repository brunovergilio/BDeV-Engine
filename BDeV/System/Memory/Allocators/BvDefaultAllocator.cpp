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
	return BvHeapMemory::Alloc(size, alignment, alignmentOffset);
}


void BvDefaultAllocator::Free(void* ptr)
{
	BvHeapMemory::Free(ptr);
}


size_t BvDefaultAllocator::GetAllocationSize(void* pMem)
{
	BvAssert(pMem != nullptr, "Memory pointer can't be nullptr!");

#if BV_PLATFORM == BV_PLATFORM_WIN32
	return _msize(reinterpret_cast<void*>(reinterpret_cast<size_t*>(pMem)[-1]));
#else
#error "Platform not yet supported"
#endif
}