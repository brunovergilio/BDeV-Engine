#include "BvBoundsChecker.h"


constexpr size_t kFrontGuardValue = 0x7A7A7A7A7A7A7A7A;
constexpr size_t kBackGuardValue = 0x7F7F7F7F7F7F7F7F;


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
	*mem.pAsSizeTPtr = kFrontGuardValue;
}


void BvSimpleBoundsChecker::GuardBack(void* pMemory)
{
	MemType mem{ pMemory };
	*mem.pAsSizeTPtr = kBackGuardValue;
}


void BvSimpleBoundsChecker::CheckFrontGuard(void* pMemory)
{
	MemType mem{ pMemory };
	if (*mem.pAsSizeTPtr != kFrontGuardValue)
	{
		BV_ERROR("Front guards not maching in address 0x%p", mem.pAsVoidPtr);
	}
}


void BvSimpleBoundsChecker::CheckBackGuard(void* pMemory)
{
	MemType mem{ pMemory };
	if (*mem.pAsSizeTPtr != kBackGuardValue)
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
	*mem.pAsSizeTPtr = kFrontGuardValue;

	for (auto guard : m_FrontGuards)
	{
		BV_ERROR("Front guards not maching in address 0x%p", guard);
	}
}


void BvExtendedBoundsChecker::GuardBack(void* pMemory)
{
	MemType mem{ pMemory };
	*mem.pAsSizeTPtr = kBackGuardValue;
	
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
		if (*mem.pAsSizeTPtr != kFrontGuardValue)
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
		if (*mem.pAsSizeTPtr != kBackGuardValue)
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