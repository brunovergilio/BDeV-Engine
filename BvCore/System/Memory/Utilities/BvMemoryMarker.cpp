#include "BvMemoryMarker.h"


constexpr u8 kAllocationMarkValue = 0xCD;
constexpr u8 kDeallocationMarkValue = 0xDD;


BvMemoryMarker::BvMemoryMarker()
{
}


BvMemoryMarker::~BvMemoryMarker()
{
}


void BvMemoryMarker::MarkAllocation(void* pMem, size_t size)
{
	memset(pMem, kAllocationMarkValue, size);
}


void BvMemoryMarker::MarkDeallocation(void* pMem, size_t size)
{
	memset(pMem, kDeallocationMarkValue, size);
}