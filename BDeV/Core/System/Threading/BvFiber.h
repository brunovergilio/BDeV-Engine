#pragma once


#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Utils/BvDelegate.h"
#include "BDeV/Core/System/Memory/BvMemory.h"
#include "BDeV/Core/System/BvPlatformHeaders.h"


using FiberFunction = void(*)(void*);

class BvFiber
{
	BV_NOCOPY(BvFiber);

public:
	friend class BvThread;

	BvFiber();
	BvFiber(BvFiber&& rhs) noexcept;
	BvFiber& operator=(BvFiber&& rhs) noexcept;
	template<class Fn, class... Args,
		typename = typename std::enable_if_t<std::is_invocable_v<Fn, Args...> && !std::is_integral_v<Fn>>>
	BvFiber(Fn&& fn, Args&&... args)
		: BvFiber(0, std::forward<Fn>(fn), std::forward<Args>(args)...) {}

	template<class Fn, class... Args,
		typename = typename std::enable_if_t<std::is_invocable_v<Fn, Args...> && !std::is_integral_v<Fn>>>
	BvFiber(const u32 stackSize, Fn&& fn, Args&&... args)
		: m_pDelegate(new((void*)BV_NEW_ARRAY(u8, sizeof(BvDelegate<Fn, Args...>))) BvDelegate<Fn, Args...>(std::forward<Fn>(fn), std::forward<Args>(args)...))
	{
		Create(stackSize);
	}

	~BvFiber();

	void Switch(const BvFiber& fiber) const;

	OSFiberHandle GetFiber() const;

private:
	void Create(const size_t stackSize);
	void Destroy();

private:
	BvDelegateBase* m_pDelegate = nullptr;
	OSFiberHandle m_pFiber = nullptr;
};