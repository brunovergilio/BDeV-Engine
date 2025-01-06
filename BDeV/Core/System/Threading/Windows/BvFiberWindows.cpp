#include "BDeV/Core/System/Threading/BvFiber.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include <utility>


#if defined(BV_USE_OS_FIBERS)
static BvFiber& GetThreadFiberInternal()
{
	static thread_local BvFiber threadFiber;

	return threadFiber;
}

void __stdcall FiberEntryPoint(void* pData)
{
	IBvTask* pTask = reinterpret_cast<IBvTask*>(pData);
	pTask->Run();
}
#else
// This structure is only used when creating the fiber, but
// it helps visualizing the elements and their offsets
struct FiberContext
{
	__m128 m_pXMMs[10]; // 0 - XMM 6-15
	u32 m_MMXControl; // 160
	u32 m_X87Control; // 164
	void* m_Pad; // 168
	void* m_pFiberStorage; // 176 - NT_TIB 0x20
	void* m_pDeallocationStack; // 184 - NT_TIB 0x1478
	void* m_pStackLimit; // 192 - NT_TIB 0x10
	void* m_pStackBase; // 200 - NT_TIB 0x8
	void* m_pR12; // 208
	void* m_pR13; // 216
	void* m_pR14; // 224
	void* m_pR15; // 232
	void* m_pRDI; // 240
	void* m_pRSI; // 248
	void* m_pRBX; // 256
	void* m_pRBP; // 264
};


namespace Internal
{
	static thread_local BvFiber* s_pTLSCurrentFiber = nullptr;

	static BV_NO_INLINE BvFiber* GetCurrentFiber()
	{
		_mm_mfence();
		return s_pTLSCurrentFiber;
	}

	static BV_NO_INLINE void SetCurrentFiber(BvFiber* pFiber)
	{
		_mm_mfence();
		s_pTLSCurrentFiber = pFiber;
	}
}


static BV_NO_INLINE BvFiber& GetThreadFiberInternal()
{
	static thread_local BvFiber threadFiber;

	return threadFiber;
}


extern "C"
{
	extern void BV_NO_INLINE InitFPControl(const void* pFiber);
	extern void BV_NO_INLINE StartFiberASM(const void* pFiber);
	extern void BV_NO_INLINE SwitchToFiberASM(void** pSrcFiber, const void* pDstFiber);
}


void FiberEntryPoint(void* pData, void* pParent)
{
	IBvTask* pTask = reinterpret_cast<IBvTask*>(pData);
	pTask->Run();
}


void FiberExitPoint()
{
	BV_ASSERT(nullptr, "A fiber should never hit this point");
	exit(0);
}
#endif


BvFiber::BvFiber()
{
}


BvFiber::BvFiber(BvFiber&& rhs) noexcept
{
	*this = std::move(rhs);
}


BvFiber& BvFiber::operator=(BvFiber&& rhs) noexcept
{
	if (this != &rhs)
	{
		std::swap(m_hFiber, rhs.m_hFiber);
		std::swap(m_pTask, rhs.m_pTask);
	}

	return *this;
}


BvFiber::~BvFiber()
{
	Destroy();
}


void BvFiber::Switch(BvFiber& fiber)
{
#if defined(BV_USE_OS_FIBERS)
	BV_ASSERT(fiber.m_hFiber != nullptr, "Fiber is nullptr");
	SwitchToFiber(fiber.m_hFiber);
#else
	Internal::SetCurrentFiber(&fiber);
	SwitchToFiberASM(&m_hFiber.m_pContext, fiber.m_hFiber.m_pContext);
#endif
}


BvFiber& BvFiber::CreateForThread()
{
#if defined(BV_USE_OS_FIBERS)
	auto& threadFiber = GetThreadFiberInternal();
	BV_ASSERT(threadFiber.m_hFiber == nullptr, "Fiber already converted / created");
	threadFiber.m_hFiber = ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);
	BV_ASSERT(threadFiber.m_hFiber != nullptr, "Couldn't convert Thread to Fiber");
#else
	auto& fiber = GetCurrent();
	Internal::SetCurrentFiber(&fiber);
	return fiber;
#endif
}


void BvFiber::DestroyForThread()
{
#if defined(BV_USE_OS_FIBERS)
	auto& threadFiber = GetThreadFiberInternal();
	BV_ASSERT(threadFiber.m_hFiber != nullptr, "Thread not yet converted to Fiber");
	BOOL result = ConvertFiberToThread();
	BV_ASSERT(result, "Couldn't convert Fiber to Thread");
	threadFiber.m_hFiber = nullptr;
#endif
}


BvFiber& BvFiber::GetCurrent()
{
	return GetThreadFiberInternal();
}


void BvFiber::Create(size_t stackSize)
{
#if defined(BV_USE_OS_FIBERS)
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
	m_hFiber = CreateFiberEx(stackSize > 0 ? stackSize - 1 : 0, stackSize, FIBER_FLAG_FLOAT_SWITCH, FiberEntryPoint, m_pTask);

	BV_ASSERT(m_hFiber != nullptr, "Couldn't create Fiber");
#else
	if (stackSize == 0)
	{
		stackSize = 128_kb;
	}

	// On Windows, the stack has to be aligned to a 16-byte boundary
	constexpr size_t kStackAlignmentSize = 16;
	m_hFiber.m_pMemory = BV_ALLOC(stackSize, kStackAlignmentSize);
	// The stack starts at the highest memory address of our block, then grows downwards/backwards; it also has to be 16-byte aligned
	auto pAlignedStack = reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(m_hFiber.m_pMemory) + stackSize) & ~(kStackAlignmentSize - 1));

	// We need at least 32 bytes for shadow space. As per MSDN:
	// The parameter area is always at the bottom of the stack (even if alloca is used), so that it will always be adjacent to the
	// return address during any function call. It contains at least four entries, but always enough space to hold all the parameters
	// needed by any function that may be called. Note that space is always allocated for the register parameters, even if the parameters
	// themselves are never homed to the stack; a callee is guaranteed that space has been allocated for all its parameters.
	auto pStackTop = reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(pAlignedStack) - sizeof(void*) * 4);

	// Add the address to the Fiber's starting function
	*(--pStackTop) = reinterpret_cast<uintptr_t>(StartFiberASM);
	// Add 8 bytes for padding; we need this when we're preparing the stack on a fiber switch call
	*(--pStackTop) = 0;
	// Set up the context structure which will hold the initial values for the fiber
	auto pContext = reinterpret_cast<FiberContext*>(reinterpret_cast<uintptr_t>(pStackTop) - sizeof(FiberContext));
	ZeroMemory(pContext, sizeof(FiberContext));
	// The stack base is the beginning of our stack
	pContext->m_pStackBase = pAlignedStack;
	// The limit is our initially allocated pointer (stack grows downwards)
	pContext->m_pStackLimit = pContext->m_pDeallocationStack = m_hFiber.m_pMemory;
	// These 3 registers will store some basic information for when the fiber is first switched into
	pContext->m_pRDI = m_pTask;
	pContext->m_pRSI = FiberEntryPoint;
	pContext->m_pRBP = FiberExitPoint;
	// The MMX and x87 control/status words have to be stored in the context's stack before the first switch. I've gotten some
	// really strange crashes when I didn't do that, such as a MessageBox call within a Fiber's call stack crashing.
	InitFPControl(pContext);
	
	// Store the context address
	m_hFiber.m_pContext = pContext;
#endif
}


void BvFiber::Destroy()
{
#if defined(BV_USE_OS_FIBERS)
	if (m_hFiber && m_pTask)
	{
		DeleteFiber(m_hFiber);
	}
#else
	if (m_hFiber.m_pMemory)
	{
		BV_FREE(m_hFiber.m_pMemory);
	}
#endif

	if (m_pTask)
	{
		BV_DELETE_ARRAY((u8*)m_pTask);
	}
}