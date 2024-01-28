#pragma once


#include "BDeV/System/Threading/BvSync.h"
#include "BDeV/Container/BvVector.h"


template<typename Type, u32 SIZE>
class BvRingBuffer
{
public:
	BvRingBuffer() {}
	BvRingBuffer(const BvRingBuffer& rhs) {}
	~BvRingBuffer() {}

	bool Push(const Type& value)
	{
		auto currIndex = m_CurrIndex.load(std::memory_order::acquire);
		BvScopedLock lock(m_PushLock);
		u32 lastIndex = m_LastIndex.load(std::memory_order::acquire);
		u32 nextIndex = GetNextIndex(lastIndex);
		if (nextIndex != currIndex)
		{
			m_Queue[lastIndex] = value;
			m_LastIndex.store(nextIndex, std::memory_order::release);
			return true;
		}

		return false;
	}

	bool Pop(Type*& pValue)
	{
		auto lastIndex = m_LastIndex.load(std::memory_order::acquire);
		BvScopedLock lock(m_PopLock);
		auto currIndex = m_CurrIndex.load(std::memory_order::acquire);
		auto nextIndex = GetNextIndex(currIndex);
		if (currIndex != lastIndex)
		{
			pValue = &m_Queue[currIndex];
			m_CurrIndex.store(nextIndex, std::memory_order::release);
			return true;
		}

		return false;
	}

private:
	u32 GetNextIndex(u32 currIndex)
	{
		return (currIndex + 1u) % SIZE;
	}

private:
	Type m_Queue[SIZE];
	std::atomic<u32> m_LastIndex{};
	std::atomic<u32> m_CurrIndex{};
	BvAdaptiveMutex m_PushLock;
	BvAdaptiveMutex m_PopLock;
};