#include "BvCore/System/Threading/BvFiber.h"
#include "BvCore/BvDebug.h"
#include <utility>


#if (BV_PLATFORM == BV_PLATFORM_WIN32)


BvThreadLocal u32 g_ThreadFiber = 0;


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

		rhs.m_pFiber = nullptr;
		rhs.m_pFunction = nullptr;
		rhs.m_pData = nullptr;
	}

	return *this;
}


BvFiber::BvFiber(FiberFunction pFunction, void * const pData)
	: m_pFunction(pFunction), m_pData(pData)
{
	BvAssertMsg(m_pFunction != nullptr, "Fiber's callback routine is nullptr");
}


BvFiber::~BvFiber()
{
	Destroy();
}


void BvFiber::CreateFromThread()
{
	BvAssertMsg(m_pFiber == nullptr, "Fiber already converted / created");
	m_pFiber = ConvertThreadToFiber(nullptr);
	BvAssertMsg(m_pFiber != nullptr, "Couldn't convert Thread to Fiber");
	m_pData = &g_ThreadFiber;
}


void BvFiber::DestroyFromThread()
{
	BOOL result = ConvertFiberToThread(); result;
	BvAssertMsg(result, "Couldn't convert Fiber to Thread");

	m_pData = nullptr;
	m_pFiber = nullptr;
}


void BvFiber::Create(const size_t stackSize)
{
	BvAssertMsg(m_pFiber == nullptr, "Fiber already created / converted");
	BvAssertMsg(m_pFunction != nullptr, "Fiber's callback routine is nullptr");

	m_pFiber = CreateFiber(stackSize, FiberEntryPoint, this);
	
	BvAssertMsg(m_pFiber != nullptr, "Couldn't create Fiber");
}


void BvFiber::Create(FiberFunction pFunction, void * const pData, const size_t stackSize)
{
	m_pFunction = pFunction;
	m_pData = pData;
	Create(stackSize);
}


void BvFiber::Destroy()
{
	if (m_pFiber && m_pData != &g_ThreadFiber)
	{
		DeleteFiber(m_pFiber);
		m_pFiber = nullptr;
		m_pData = nullptr;
	}
}


void BvFiber::Activate()
{
	BvAssertMsg(m_pFiber != nullptr, "Fiber is nullptr");
	SwitchToFiber(m_pFiber);
}


void BvFiber::FiberEntryPoint(void * pData)
{
	BvFiber * pFiber = reinterpret_cast<BvFiber *>(pData);
	pFiber->m_pFunction(pFiber->m_pData);
}


#endif