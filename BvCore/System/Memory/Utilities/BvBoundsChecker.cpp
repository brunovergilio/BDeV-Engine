#include "BvBoundsChecker.h"


constexpr u32 kFrontGuardValue = 0x7A7A7A7A;
constexpr u32 kBackGuardValue = 0x7F7F7F7F;


// ===============================================
// Simple Bounds Checker
// ===============================================
BvSimpleBoundsChecker::BvSimpleBoundsChecker()
{
}


BvSimpleBoundsChecker::~BvSimpleBoundsChecker()
{
}


void BvSimpleBoundsChecker::GuardFront(void* pMemory)
{
	MemType mem{ pMemory };
	*mem.pAsUIntPtr = kFrontGuardValue;
}


void BvSimpleBoundsChecker::GuardBack(void* pMemory)
{
	MemType mem{ pMemory };
	*mem.pAsUIntPtr = kBackGuardValue;
}


void BvSimpleBoundsChecker::CheckFrontGuard(void* pMemory)
{
	MemType mem{ pMemory };
	if (*mem.pAsUIntPtr != kFrontGuardValue)
	{
		BV_ERROR("Front guards not maching in address 0x%p", mem.pAsVoidPtr);
	}
}


void BvSimpleBoundsChecker::CheckBackGuard(void* pMemory)
{
	MemType mem{ pMemory };
	if (*mem.pAsUIntPtr != kBackGuardValue)
	{
		BV_ERROR("Back guards not maching in address 0x%p", mem.pAsVoidPtr);
	}
}


// ===============================================
// Extended Bounds Checker
// ===============================================
BvExtendedBoundsChecker::BvExtendedBoundsChecker()
{
}


BvExtendedBoundsChecker::~BvExtendedBoundsChecker()
{
}


void BvExtendedBoundsChecker::GuardFront(void* pMemory)
{
	MemType mem{ pMemory };
	*mem.pAsUIntPtr = kFrontGuardValue;

	for (auto guard : m_FrontGuards)
	{
		BV_ERROR("Front guards not maching in address 0x%p", guard);
	}
}


void BvExtendedBoundsChecker::GuardBack(void* pMemory)
{
	MemType mem{ pMemory };
	*mem.pAsUIntPtr = kBackGuardValue;
	
	for (auto guard : m_BackGuards)
	{
		BV_ERROR("Back guards not maching in address 0x%p", guard);
	}
}


void BvExtendedBoundsChecker::CheckFrontGuard(void* pMemory)
{
	MemType mem{ pMemory };
	u32 index = 0;
	for (auto i = 0u; i < m_FrontGuards.Size(); i++)
	{
		if (*mem.pAsUIntPtr != kFrontGuardValue)
		{
			BV_ERROR("Front guards not maching in address 0x%p", mem.pAsVoidPtr);
		}

		if (mem.pAsVoidPtr == pMemory)
		{
			index = i;
		}
	}

	std::swap(m_FrontGuards[index], m_FrontGuards.Back());
	m_FrontGuards.PopBack();
}


void BvExtendedBoundsChecker::CheckBackGuard(void* pMemory)
{
	MemType mem{ pMemory };
	u32 index = 0;
	for (auto i = 0u; i < m_BackGuards.Size(); i++)
	{
		if (*mem.pAsUIntPtr != kBackGuardValue)
		{
			BV_ERROR("Back guards not maching in address 0x%p", mem.pAsVoidPtr);
		}

		if (mem.pAsVoidPtr == pMemory)
		{
			index = i;
		}
	}

	std::swap(m_BackGuards[index], m_BackGuards.Back());
	m_BackGuards.PopBack();
}