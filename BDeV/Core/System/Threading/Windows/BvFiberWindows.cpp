#include "BDeV/Core/System/Threading/BvFiber.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BDeV/Core/System/BvPlatformHeaders.h"
#include "BDeV/Core/System/Memory/BvMemory.h"
#include <utility>


struct alignas(kCacheLineSize) BvFiberImpl
{
	BvFiber::EntryPointType m_Task; // Entry Point
	IBvMemoryArena* m_pArena = nullptr; // Memory Arena
	void* m_pMemory = nullptr; // Allocated memory block
	OSFiberHandle m_hFiber = nullptr; // Fiber Context
	void* m_pFLS = nullptr; // Fiber Local Storage pointer
	bool m_IsThreadFiber = false; // Determines if this fiber context is from a thread
};


namespace Internal
{
	thread_local BvFiberImpl* g_pThreadFiber = nullptr;
	thread_local BvFiberImpl* g_pCurrentFiber = nullptr;
}


BvFiberImpl* GetThreadFiberTLS()
{
	return Internal::g_pThreadFiber;
}


void SetThreadFiberTLS(BvFiberImpl* pFiber)
{
	Internal::g_pThreadFiber = pFiber;
}


BvFiberImpl* GetCurrentFiberTLS()
{
	return Internal::g_pCurrentFiber;
}


void SetCurrentFiberTLS(BvFiberImpl* pFiber)
{
	Internal::g_pCurrentFiber = pFiber;
}


#if !defined(BV_USE_ASM_FIBERS)
void CALLBACK FiberEntryPoint(void* pData)
{
	BvFiberImpl* pFiber = reinterpret_cast<BvFiberImpl*>(pData);
	pFiber->m_Task();

	BV_ASSERT(nullptr, "A fiber should never hit this point");
	exit(0);
}
#else
struct BvTransferParams
{
	OSFiberHandle m_hPrevFiber = nullptr;
	void* m_pData = nullptr;
};


extern "C"
{
	// Switches to the callstack pointed to by 'fiber', along with any parameters in 'pData'.
	// Returns the previous fiber context pointer, optionally with custom data
	BvTransferParams jump_fcontext(const OSFiberHandle fiber, void* pData);

	// Prepares memory from 'pStack' with 'size' bytes to be used as a fiber context,
	// along with a pre-entry point
	OSFiberHandle make_fcontext(void* pStack, size_t size, void (*pFn)(BvTransferParams));
}


void UpdatePrevFiberContext(const BvTransferParams& result)
{
	auto pPrevFiber = reinterpret_cast<BvFiberImpl*>(result.m_pData);
	pPrevFiber->m_hFiber = result.m_hPrevFiber;
}


void FiberEntryPoint(BvTransferParams params)
{
	auto pFiber = reinterpret_cast<BvFiberImpl*>(params.m_pData);
	
	// Jump back to the previous fiber context used to create
	// this fiber, only happens during creation
	auto result = jump_fcontext(params.m_hPrevFiber, nullptr);

	// Upon returning, update the previous fiber's context
	UpdatePrevFiberContext(result);

	// Run main entry point
	pFiber->m_Task();

	BV_ASSERT(nullptr, "A fiber should never hit this point");
	exit(0);
}
#endif


BvFiber::BvFiber()
{
}


BvFiber::BvFiber(std::nullptr_t)
{
}


BvFiber::BvFiber(BvFiber&& rhs) noexcept
	: m_pImpl(rhs.m_pImpl), m_Owned(rhs.m_Owned)
{
}


BvFiber& BvFiber::operator=(BvFiber&& rhs) noexcept
{
	if (this != &rhs)
	{
		Destroy();

		m_pImpl = rhs.m_pImpl;
		m_Owned = rhs.m_Owned;

		rhs.m_pImpl = nullptr;
		rhs.m_Owned = false;
	}

	return *this;
}


BvFiber::~BvFiber()
{
	Destroy();
}


void BvFiber::Resume()
{
	BV_ASSERT(m_pImpl != nullptr, "Fiber is nullptr");
	BV_ASSERT(GetThreadFiberTLS() != nullptr, "Thread is not a fiber");
	BV_ASSERT(GetCurrentFiberTLS() != m_pImpl, "Already in fiber's context");

#if 0
	printf("Switching Fibers -> %p => %p\n", GetCurrentFiberTLS(), m_pImpl);
	printf("Current Thread Fiber TLS: 0x%p\n", GetThreadFiberTLS());
	printf("Current Fiber TLS: 0x%p\n", GetCurrentFiberTLS());
#endif

	// Save the previous fiber
	auto pPrevFiber = GetCurrentFiberTLS();

	// Update TLS with the new fiber
	SetCurrentFiberTLS(m_pImpl);

#if !defined(BV_USE_ASM_FIBERS)
	::SwitchToFiber(m_pImpl->m_hFiber);
#else
	// Switch to the selected fiber, also passing the previous fiber as parameter
	auto result = jump_fcontext(m_pImpl->m_hFiber, pPrevFiber);

#if 0
	printf("Current Fiber TLS After: 0x%p\n", GetCurrentFiberTLS());
#endif

	// Upon returning, update the previous fiber's context
	UpdatePrevFiberContext(result);
#endif
}


void BvFiber::Yield()
{
	auto fiber = GetCurrentThread();
	fiber.Resume();
}


void* BvFiber::GetLocalData() const
{
	BV_ASSERT(m_pImpl != nullptr, "Fiber is nullptr");
	return m_pImpl->m_pFLS;
}


void BvFiber::SetLocalData(void* pData)
{
	BV_ASSERT(m_pImpl != nullptr, "Fiber is nullptr");
	m_pImpl->m_pFLS = pData;
}


BvFiber BvFiber::GetCurrent()
{
	return GetCurrentFiberTLS();
}


BvFiber BvFiber::GetCurrentThread()
{
	return GetThreadFiberTLS();
}


BvFiber BvFiber::CreateForThread(IBvMemoryArena* pArena)
{
	BV_ASSERT(GetThreadFiberTLS() == nullptr, "Fiber already converted / created");
	auto pThreadFiber = BV_MNEW(*pArena, BvFiberImpl)();
	pThreadFiber->m_pArena = pArena;
	pThreadFiber->m_pMemory = pThreadFiber;
	pThreadFiber->m_IsThreadFiber = true;
	
#if !defined(BV_USE_ASM_FIBERS)
	pThreadFiber->m_hFiber = ::ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);
	BV_ASSERT(pThreadFiber->m_hFiber != nullptr, "Couldn't convert Thread to Fiber");
#endif

	// Update thread's fiber context
	SetThreadFiberTLS(pThreadFiber);
	// Also make it the current
	SetCurrentFiberTLS(pThreadFiber);

	return pThreadFiber;
}


void BvFiber::DestroyForThread()
{
	auto pThreadFiber = GetThreadFiberTLS();
	BV_ASSERT(pThreadFiber != nullptr && pThreadFiber == GetCurrentFiberTLS(), "Thread not yet converted to Fiber or not currently running");

#if !defined(BV_USE_ASM_FIBERS)
	BOOL result = ::ConvertFiberToThread();
	BV_ASSERT(result, "Couldn't convert Fiber back to Thread");
#endif

	BV_MDELETE(*pThreadFiber->m_pArena, pThreadFiber);
	
	// Unset TLS variables
	SetCurrentFiberTLS(nullptr);
	SetThreadFiberTLS(nullptr);
}


BvFiber::BvFiber(BvFiberImpl* pImpl)
	: m_pImpl(pImpl)
{
}


void BvFiber::Create(IBvMemoryArena* pArena, size_t stackSize, EntryPointType& entryPoint)
{
#if !defined(BV_USE_ASM_FIBERS)
	auto totalSize = sizeof(BvFiberImpl);

	m_pImpl = new(BV_MALLOC(*pArena, totalSize, alignof(BvFiberImpl))) BvFiberImpl();
	m_pImpl->m_Task = std::move(entryPoint);
	m_pImpl->m_pArena = pArena;
	m_pImpl->m_pMemory = m_pImpl;

	// Piece of information taken from https://github.com/google/marl/issues/12
	// The Win32 CreateFiber function has unexpected virtual memory size by default.When using CreateFiberEx,
	// default behavior is to round stack size to next multiple of 1MiB.With CreateFiber, for some reason it rounds to 2MiB,
	// which is even worse.
	//
	// Now even if you use CreateFiberEx, there's still some weirdness. If dwStackCommitSize and dwStackReserveSize are equal,
	// the stack size will still round to 1MiB. The only way is to use it like this:
	// CreateFiberEx(size - 1, size, FIBER_FLAG_FLOAT_SWITCH, cb, NULL);
	//
	// Note that even then, the stack can't be less than dwAllocationGranularity, which is typically 64KiB.
	// If this minimum stack size is OK for performance, then that's how fiber creation should be done on Windows.
	//
	// If we need smaller values(16KiB is common in schedulers), then asm fibers should be used instead IMO.
	m_pImpl->m_hFiber = CreateFiberEx(stackSize > 0 ? stackSize - 1 : 0, stackSize, FIBER_FLAG_FLOAT_SWITCH, FiberEntryPoint, this);
	BV_ASSERT(m_pImpl->m_hFiber != nullptr, "Couldn't create Fiber");
#else
	constexpr auto kAlignmentMask = alignof(BvFiberImpl) - 1;

	auto pMemory = BV_MALLOC(*pArena, stackSize, kDefaultAlignmentSize);

	// Stack memory grows backwards, from highest to lowest address, so we go to the end
	// of the memory block, minus sizeof(BvFiberImpl), then align it
	auto pStorage = reinterpret_cast<void*>(((reinterpret_cast<uintptr_t>(pMemory) + stackSize - sizeof(BvFiberImpl)) & ~(kAlignmentMask)));

	// Store our fiber data
	m_pImpl = new(pStorage) BvFiberImpl();
	m_pImpl->m_Task = std::move(entryPoint);
	m_pImpl->m_pArena = pArena;
	m_pImpl->m_pMemory = pMemory;

	// Skip 64 bytes (maybe should be 32 for Windows)
	auto pStackTop = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(pStorage) - 64);
	auto pStackBottom = pMemory;
	auto remainingStackSize = reinterpret_cast<uintptr_t>(pStackTop) - reinterpret_cast<uintptr_t>(pStackBottom);

	// Create context
	m_pImpl->m_hFiber = make_fcontext(pStackTop, remainingStackSize, FiberEntryPoint);

	// Do initial jump
	auto result = jump_fcontext(m_pImpl->m_hFiber, m_pImpl);

	// Update the new fiber's context - note that we don't update the thread's context
	// because we may not have converted the thread into a fiber yet
	m_pImpl->m_hFiber = result.m_hPrevFiber;
#endif

	m_Owned = true;
}


void BvFiber::Destroy()
{
	if (m_pImpl && m_Owned)
	{
#if !defined(BV_USE_ASM_FIBERS)
		DeleteFiber(m_pImpl->m_hFiber);
#endif

		m_pImpl->m_Task.~BvMTask();
		BV_MFREE(*m_pImpl->m_pArena, m_pImpl->m_pMemory);
	}
}