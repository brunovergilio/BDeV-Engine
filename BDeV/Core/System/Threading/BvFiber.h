#pragma once


#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Utils/BvTask.h"
#include "BDeV/Core/System/Memory/BvMemory.h"


struct BvFiberImpl;


class BvFiber
{
	BV_NOCOPY(BvFiber);

	static constexpr u32 kDefaultFiberStackSize = 64_kb;
	static constexpr auto kTaskSize = 24;

public:
	using EntryPointType = BvMTask<kTaskSize>;

	BvFiber();
	BvFiber(std::nullptr_t);
	BvFiber(BvFiber&& rhs) noexcept;
	BvFiber& operator=(BvFiber&& rhs) noexcept;
	~BvFiber();
	
	template<class Fn, typename = typename std::enable_if_t<std::is_invocable_v<Fn> && !std::is_integral_v<Fn>>>
	BvFiber(Fn&& fn)
		: BvFiber(BV_DEFAULT_MEMORY_ARENA, kDefaultFiberStackSize, std::forward<Fn>(fn)) {}

	template<class Fn, typename = typename std::enable_if_t<std::is_invocable_v<Fn> && !std::is_integral_v<Fn>>>
	explicit BvFiber(u32 stackSize, Fn&& fn)
		: BvFiber(BV_DEFAULT_MEMORY_ARENA, stackSize, std::forward<Fn>(fn)) {}

	template<class Fn, typename = typename std::enable_if_t<std::is_invocable_v<Fn> && !std::is_integral_v<Fn>>>
	explicit BvFiber(IBvMemoryArena* pArena, u32 stackSize, Fn&& fn)
	{
		EntryPointType entryPoint(std::forward<Fn>(fn));
		Create(pArena, stackSize, entryPoint);
	}

	// Resumes execution on the current fiber
	void Resume();

	// Suspends this fiber and resumes execution on the current thread's fiber
	void Yield();

	// Retrieves the fiber's local storage pointer
	void* GetLocalData() const;

	// Sets the fiber's local storage pointer
	void SetLocalData(void* pData);

	// Returns the current fiber context
	static BvFiber GetCurrent();

	// Returns the current thread's fiber context
	static BvFiber GetCurrentThread();

	// Converts the thread into a fiber
	static BvFiber CreateForThread(IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA);

	// Releases the thread's fiber context data
	static void DestroyForThread();

	BV_INLINE operator bool() const { return m_pImpl != nullptr; }

private:
	BvFiber(BvFiberImpl* pImpl);

	void Create(IBvMemoryArena* pArena, size_t stackSize, EntryPointType& entryPoint);
	void Destroy();

private:
	BvFiberImpl* m_pImpl = nullptr;
	bool m_Owned = false;
};