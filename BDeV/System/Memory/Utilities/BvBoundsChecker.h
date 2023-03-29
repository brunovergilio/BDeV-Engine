#pragma once


#include "BDeV/System/Memory/BvMemoryCommon.h"
#include "BDeV/Container/BvVector.h"


class BV_API BvNoBoundsChecker
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


class BV_API BvSimpleBoundsChecker
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


class BV_API BvExtendedBoundsChecker
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