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
	BV_ASSERT(*mem.pAsSizeTPtr == kFrontGuardValue, "Front guards not maching in address 0x%p", mem.pAsVoidPtr);
}


void BvSimpleBoundsChecker::CheckBackGuard(void* pMemory)
{
	MemType mem{ pMemory };
	BV_ASSERT(*mem.pAsSizeTPtr == kBackGuardValue, "Back guards not maching in address 0x%p", mem.pAsVoidPtr);
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
		BV_ASSERT(*guard == kFrontGuardValue, "Front guards not maching in address 0x%p", guard);
	}
	m_FrontGuards.EmplaceBack(mem.pAsSizeTPtr);
}


void BvExtendedBoundsChecker::GuardBack(void* pMemory)
{
	MemType mem{ pMemory };
	*mem.pAsSizeTPtr = kBackGuardValue;

	for (auto guard : m_BackGuards)
	{
		BV_ASSERT(*guard == kBackGuardValue, "Front guards not maching in address 0x%p", guard);
	}
	m_BackGuards.EmplaceBack(mem.pAsSizeTPtr);
}


void BvExtendedBoundsChecker::CheckFrontGuard(void* pMemory)
{
	MemType mem{ pMemory };
	u32 index = 0;
	for (auto i = 0u; i < m_FrontGuards.Size(); i++)
	{
		auto guard = m_FrontGuards[i];
		BV_ASSERT(*guard == kFrontGuardValue, "Front guards not maching in address 0x%p", guard);

		if (mem.pAsVoidPtr == pMemory)
		{
			index = i;
		}
	}

	m_FrontGuards.EraseUnsorted(index);
}


void BvExtendedBoundsChecker::CheckBackGuard(void* pMemory)
{
	MemType mem{ pMemory };
	u32 index = 0;
	for (auto i = 0u; i < m_BackGuards.Size(); i++)
	{
		auto guard = m_BackGuards[i];
		BV_ASSERT(*guard == kBackGuardValue, "Back guards not maching in address 0x%p", guard);

		if (mem.pAsVoidPtr == pMemory)
		{
			index = i;
		}
	}

	m_BackGuards.EraseUnsorted(index);
}