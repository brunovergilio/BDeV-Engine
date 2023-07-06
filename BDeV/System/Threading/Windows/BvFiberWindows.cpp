#include "BDeV/System/Threading/BvFiber.h"
#include "BDeV/System/Debug/BvDebug.h"
#include <utility>
#include <Windows.h>


BvFiber::BvFiber()
{
}


BvFiber::BvFiber(BvFiber && rhs) noexcept
{
	*this = std::move(rhs);
}


BvFiber & BvFiber::operator =(BvFiber && rhs) noexcept
{
	if (this != &rhs)
	{
		std::swap(m_pFiberData, rhs.m_pFiberData);
	}

	return *this;
}


BvFiber::BvFiber(FiberFunction pFunction, void * const pData, const size_t stackSize)
{
	Create(pFunction, pData, stackSize);
}


BvFiber::~BvFiber()
{
	Destroy();
}


void BvFiber::Switch(const BvFiber & fiber) const
{
	BvAssert(fiber.m_pFiberData->m_pFiber != nullptr, "Fiber is nullptr");
	SwitchToFiber(fiber.m_pFiberData->m_pFiber);
}


void* BvFiber::GetFiber() const
{
	return m_pFiberData->m_pFiber;
}


void BvFiber::Create(FiberFunction pFunction, void* const pData, const size_t stackSize)
{
	m_pFiberData = new FiberData();
	m_pFiberData->m_pFunction = pFunction;
	m_pFiberData->m_pData = pData;
	
	// If we don't specify a function that means we're converting a thread to a fiber
	if (!pFunction)
	{
		return;
	}

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
	m_pFiberData->m_pFiber = CreateFiberEx(stackSize > 0 ? stackSize - 1 : 0, stackSize, FIBER_FLAG_FLOAT_SWITCH, FiberEntryPoint, m_pFiberData);

	BvAssert(m_pFiberData->m_pFiber != nullptr, "Couldn't create Fiber");
}


void BvFiber::Destroy()
{
	if (m_pFiberData)
	{
		if (m_pFiberData->m_pFiber && m_pFiberData->m_pFunction)
		{
			DeleteFiber(m_pFiberData->m_pFiber);
		}
		delete m_pFiberData;
		m_pFiberData = nullptr;
	}
}


void BvFiber::FiberEntryPoint(void * pData)
{
	FiberData * pFiber = reinterpret_cast<FiberData *>(pData);
	pFiber->m_pFunction(pFiber->m_pData);
}