#pragma once


#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Utils/BvTask.h"
#include "BDeV/Core/System/Memory/BvMemory.h"
#include "BDeV/Core/System/BvPlatformHeaders.h"


class BvFiber
{
	BV_NOCOPY(BvFiber);

public:
	friend class BvThread;

	BvFiber();
	BvFiber(BvFiber&& rhs) noexcept;
	BvFiber& operator=(BvFiber&& rhs) noexcept;
	~BvFiber();
	
	template<class Fn, class... Args,
		typename = typename std::enable_if_t<std::is_invocable_v<Fn, Args...> && !std::is_integral_v<Fn>>>
	BvFiber(Fn&& fn, Args&&... args)
		: BvFiber(0, std::forward<Fn>(fn), std::forward<Args>(args)...) {}

	template<class Fn, class... Args,
		typename = typename std::enable_if_t<std::is_invocable_v<Fn, Args...> && !std::is_integral_v<Fn>>>
	BvFiber(u32 stackSize, Fn&& fn, Args&&... args)
		: m_pTask(new((void*)BV_NEW_ARRAY(u8, sizeof(Internal::BvTaskT<Fn, Args...>))) Internal::BvTaskT<Fn, Args...>(std::forward<Fn>(fn), std::forward<Args>(args)...))
	{
		Create(stackSize);
	}

	void Switch(BvFiber& fiber);
	static BvFiber* GetCurrent();

private:
	static BvFiber& GetThreadFiber();
	static BvFiber& CreateForThread();
	static void DestroyForThread();

	void Create(size_t stackSize);
	void Destroy();

private:
	OSFiberHandle m_hFiber = kNullOSFiberHandle;
	IBvTask* m_pTask = nullptr;
	bool m_IsThreadSetup = false;
};