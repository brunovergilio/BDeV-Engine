#pragma once


#include "BvCore/BvPlatform.h"
#include "BvCore/Utils/BvUtils.h"


class BvFiber
{
	BV_NOCOPY(BvFiber);

public:
	using  FiberFunction = void(*)(void*);

	BvFiber();
	BvFiber(BvFiber && rhs);
	BvFiber & operator=(BvFiber && rhs);
	explicit BvFiber(const bool isFiberFromThread, FiberFunction pFunction = nullptr, void * const pData = nullptr, const size_t stackSize = 0);
	~BvFiber();

	void CreateFromThread();
	void Create(FiberFunction pFunction, void * const pData = nullptr, const size_t stackSize = 0);
	void Destroy();

	void Switch(BvFiber & fiber);

private:
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	static void CALLBACK FiberEntryPoint(void * pData);
#endif

private:
	FiberFunction m_pFunction = nullptr;
	void * m_pData = nullptr;

#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	void * m_pFiber = nullptr;
#endif

	bool m_IsFiberFromThread = false;
};