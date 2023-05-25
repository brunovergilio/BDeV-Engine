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
		std::swap(m_pFiber, rhs.m_pFiber);
		std::swap(m_pFunction, rhs.m_pFunction);
		std::swap(m_pData, rhs.m_pData);
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
	BvAssert(fiber.m_pFiber != nullptr, "Fiber is nullptr");
	SwitchToFiber(fiber.m_pFiber);
}


void BvFiber::Create(FiberFunction pFunction, void* const pData, const size_t stackSize)
{
	BvAssert(pFunction != nullptr, "Fiber's callback routine is nullptr");

	m_pFunction = pFunction;
	m_pData = pData;

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
	m_pFiber = CreateFiberEx(stackSize > 0 ? stackSize - 1 : 0, stackSize, FIBER_FLAG_FLOAT_SWITCH, FiberEntryPoint, this);

	BvAssert(m_pFiber != nullptr, "Couldn't create Fiber");
}


void BvFiber::Destroy()
{
	if (m_pFiber && m_pFunction)
	{
		DeleteFiber(m_pFiber);
	}
}


void BvFiber::FiberEntryPoint(void * pData)
{
	BvFiber * pFiber = reinterpret_cast<BvFiber *>(pData);
	pFiber->m_pFunction(pFiber->m_pData);
}