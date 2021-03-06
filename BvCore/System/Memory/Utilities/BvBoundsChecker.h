#pragma once


#include "BvCore/System/Memory/BvMemoryCommon.h"
#include "BvCore/Container/BvVector.h"


class BvNoBoundsChecker
{
public:
	BvNoBoundsChecker() {}
	~BvNoBoundsChecker() {}

	BV_INLINE void GuardFront(void*) {}
	BV_INLINE void GuardBack(void*) {}
	BV_INLINE void CheckFrontGuard(void*) {}
	BV_INLINE void CheckBackGuard(void*) {}

public:
	static constexpr size_t kFrontGuardSize = 0;
	static constexpr size_t kBackGuardSize = 0;
};


class BvSimpleBoundsChecker
{
public:
	BvSimpleBoundsChecker();
	~BvSimpleBoundsChecker();

	void GuardFront(void* pMemory);
	void GuardBack(void* pMemory);
	void CheckFrontGuard(void* pMemory);
	void CheckBackGuard(void* pMemory);

public:
	static constexpr size_t kFrontGuardSize = 4;
	static constexpr size_t kBackGuardSize = 4;
};


class BvExtendedBoundsChecker
{
public:
	BvExtendedBoundsChecker();
	~BvExtendedBoundsChecker();

	void GuardFront(void* pMemory);
	void GuardBack(void* pMemory);
	void CheckFrontGuard(void* pMemory);
	void CheckBackGuard(void* pMemory);

public:
	static constexpr size_t kFrontGuardSize = 4;
	static constexpr size_t kBackGuardSize = 4;

private:
	BvVector<u32*> m_FrontGuards;
	BvVector<u32*> m_BackGuards;
};