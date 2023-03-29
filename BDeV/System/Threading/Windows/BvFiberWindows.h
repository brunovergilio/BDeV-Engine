#pragma once


#include "BDeV/Utils/BvUtils.h"
#include <Windows.h>


class BV_API BvFiber
{
	BV_NOCOPY(BvFiber);

public:
	friend class BvThread;

	using FiberFunction = void(*)(void*);

	BvFiber();
	BvFiber(BvFiber&& rhs) noexcept;
	BvFiber& operator=(BvFiber&& rhs) noexcept;
	explicit BvFiber(FiberFunction pFunction, void* const pData = nullptr, const size_t stackSize = 0);
	~BvFiber();

	void Switch(const BvFiber& fiber) const;

	BV_INLINE void* GetFiber() const { return m_pFiber; }

private:
	void Create(FiberFunction pFunction, void* const pData = nullptr, const size_t stackSize = 0);
	void Destroy();

	static void CALLBACK FiberEntryPoint(void* pData);

private:
	FiberFunction m_pFunction = nullptr;
	void* m_pData = nullptr;
	void* m_pFiber = nullptr;
};