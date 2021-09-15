#pragma once


#include "BvCore/Utils/BvUtils.h"
#include <Windows.h>


class BvFiber
{
	BV_NOCOPY(BvFiber);

public:
	using  FiberFunction = void(*)(void*);

	BvFiber();
	BvFiber(BvFiber&& rhs);
	BvFiber& operator=(BvFiber&& rhs);
	explicit BvFiber(const bool isFiberFromThread, FiberFunction pFunction = nullptr, void* const pData = nullptr, const size_t stackSize = 0);
	~BvFiber();

	void CreateFromThread();
	void Create(FiberFunction pFunction, void* const pData = nullptr, const size_t stackSize = 0);
	void Destroy();

	void Switch(BvFiber& fiber);

	BV_INLINE void* GetFiber() const { return m_pFiber; }

private:
	static void CALLBACK FiberEntryPoint(void* pData);

private:
	FiberFunction m_pFunction = nullptr;
	void* m_pData = nullptr;
	void* m_pFiber = nullptr;
	bool m_IsFiberFromThread = false;
};