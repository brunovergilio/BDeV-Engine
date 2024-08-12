#pragma once


#include "BDeV/Core/System/Memory/BvMemory.h"
#include "BDeV/Core/Container/BvVector.h"


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
	static constexpr size_t kFrontGuardSize = sizeof(size_t);
	static constexpr size_t kBackGuardSize = sizeof(size_t);
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
	static constexpr size_t kFrontGuardSize = sizeof(size_t);
	static constexpr size_t kBackGuardSize = sizeof(size_t);

private:
	BvVector<size_t*> m_FrontGuards;
	BvVector<size_t*> m_BackGuards;
};