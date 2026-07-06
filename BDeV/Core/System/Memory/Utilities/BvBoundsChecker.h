#pragma once


#include "BDeV/Core/Container/BvVector.h"


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