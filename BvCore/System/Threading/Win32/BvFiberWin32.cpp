#include "BvCore/System/Threading/BvFiber.h"
#include "BvCore/BvDebug.h"
#include <utility>


#if (BV_PLATFORM == BV_PLATFORM_WIN32)


BvFiber::BvFiber()
{
}


BvFiber::BvFiber(BvFiber && rhs)
{
	*this = std::move(rhs);
}


BvFiber & BvFiber::operator =(BvFiber && rhs)
{
	if (this != &rhs)
	{
		m_pFiber = rhs.m_pFiber;
		m_pFunction = rhs.m_pFunction;
		m_pData = rhs.m_pData;
		m_IsFiberFromThread = rhs.m_IsFiberFromThread;

		rhs.m_pFiber = nullptr;
		rhs.m_pFunction = nullptr;
		rhs.m_pData = nullptr;
		rhs.m_IsFiberFromThread = false;
	}

	return *this;
}


BvFiber::BvFiber(const bool isFiberFromThread, FiberFunction pFunction, void * const pData, const size_t stackSize)
	: m_IsFiberFromThread(isFiberFromThread)
{
	if (isFiberFromThread)
	{
		CreateFromThread();
	}
	else
	{
		Create(pFunction, pData, stackSize);
	}
}


BvFiber::~BvFiber()
{
	Destroy();
}


void BvFiber::CreateFromThread()
{
	BvAssertMsg(m_pFiber == nullptr, "Fiber already converted / created");
	m_pFiber = ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);
	BvAssertMsg(m_pFiber != nullptr, "Couldn't convert Thread to Fiber");
	m_IsFiberFromThread = true;
}


void BvFiber::Create(FiberFunction pFunction, void * const pData, const size_t stackSize)
{
	BvAssertMsg(m_pFiber == nullptr, "Fiber already created / converted");
	BvAssertMsg(pFunction != nullptr, "Fiber's callback routine is nullptr");

	m_pFunction = pFunction;
	m_pData = pData;

	// Piece of information taken from https://github.com/google/marl/issues/12
	// The Win32 CreateFiber function has unexpected virtual memory size by default.When using CreateFiberEx,
	// default behavior is to round stack size to next multiple of 1MiB.With CreateFiber, for some reason it rounds to 2MiB,
	// which is even worse.
	//
	//	Now even if you use CreateFiberEx, there's still some weirdness. If dwStackCommitSize and dwStackReserveSize are equal,
	// the stack size will still round to 1MiB. The only way is to use it like this:
	//	CreateFiberEx(size - 1, size, FIBER_FLAG_FLOAT_SWITCH, cb, NULL);
	//
	// Note that even then, the stack can't be less than dwAllocationGranularity, which is typically 64KiB.
	// If this minimum stack size is OK for performance, then that's how fiber creation should be done on Windows.
	//
	//	If we need smaller values(16KiB is common in schedulers), then asm fibers should be used instead IMO.
	m_pFiber = CreateFiberEx(stackSize - 1, stackSize, FIBER_FLAG_FLOAT_SWITCH, FiberEntryPoint, this);

	BvAssertMsg(m_pFiber != nullptr, "Couldn't create Fiber");
}


void BvFiber::Destroy()
{
	if (m_pFiber)
	{
		if (m_IsFiberFromThread)
		{
			BOOL result = ConvertFiberToThread(); result;
			BvAssertMsg(result, "Couldn't convert Fiber to Thread");
			m_IsFiberFromThread = false;
		}
		else
		{
			DeleteFiber(m_pFiber);
		}
		m_pFiber = nullptr;
		m_pData = nullptr;
	}
}


void BvFiber::Switch(BvFiber & fiber)
{
	BvAssertMsg(fiber.m_pFiber != nullptr, "Fiber is nullptr");
	SwitchToFiber(fiber.m_pFiber);
}


void BvFiber::FiberEntryPoint(void * pData)
{
	BvFiber * pFiber = reinterpret_cast<BvFiber *>(pData);
	pFiber->m_pFunction(pFiber->m_pData);
}


#endif