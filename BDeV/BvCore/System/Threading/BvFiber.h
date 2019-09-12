#pragma once


#include "BvCore/BvDefines.h"


class BvFiber
{
public:
	typedef void(*FiberFunction)(void*);

	BvFiber();
	BvFiber(BvFiber && rhs);
	BvFiber & operator=(BvFiber && rhs);
	BvFiber(const BvFiber &) = delete;
	BvFiber & operator=(const BvFiber &) = delete;
	explicit BvFiber(FiberFunction pFunction, void * const pData = nullptr);
	~BvFiber();

	void CreateFromThread();
	void DestroyFromThread();

	void Create(const size_t stackSize = 0);
	void Create(FiberFunction pFunction, void * const pData, const size_t stackSize = 0);
	void Destroy();

	void Activate();

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
};