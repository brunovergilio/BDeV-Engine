#pragma once


#include "BDeV/Utils/BvUtils.h"


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

#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	static void __stdcall FiberEntryPoint(void* pData);
#endif

private:
	FiberFunction m_pFunction = nullptr;
	void* m_pData = nullptr;
	void* m_pFiber = nullptr;
};