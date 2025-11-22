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
	
	template<class Fn, typename = typename std::enable_if_t<std::is_invocable_v<Fn> && !std::is_integral_v<Fn>>>
	BvFiber(Fn&& fn)
		: BvFiber(0, std::forward<Fn>(fn)) {}

	template<class Fn, typename = typename std::enable_if_t<std::is_invocable_v<Fn> && !std::is_integral_v<Fn>>>
	BvFiber(u32 stackSize, Fn&& fn)
		: m_Task(std::forward<Fn>(fn))
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

#if BV_PLATFORM_WIN32
#if !defined(BV_USE_ASM_FIBERS)
	static void CALLBACK FiberEntryPoint(void* pData);
#else
	static void FiberEntryPoint(void* pData);
	static void FiberExitPoint();
#endif // !defined(BV_USE_ASM_FIBERS)
#endif // BV_PLATFORM_WIN32

private:
	static constexpr auto kTaskSize = 24;

	BvMTask<kTaskSize> m_Task;
	OSFiberHandle m_hFiber = kNullOSFiberHandle;
	bool m_IsThreadSetup = false;
};