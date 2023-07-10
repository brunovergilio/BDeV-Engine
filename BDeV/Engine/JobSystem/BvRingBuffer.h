#pragma once


#include "BDeV/System/Threading/BvSync.h"
#include "BDeV/Container/BvVector.h"


template<typename Type>
class BvRingBuffer
{
public:
	BvRingBuffer() {}
	BvRingBuffer(const BvRingBuffer& rhs) {}
	~BvRingBuffer() {}

	void Resize(u32 size)
	{
		m_Queue.Resize(size);
	}

	bool Push(const Type& value)
	{
		BvScopedLock lock(m_AddLock);
		u32 lastIndex = m_LastIndex.load();
		u32 nextIndex = GetNextIndex(lastIndex);
		if (nextIndex != m_CurrIndex.load())
		{
			m_Queue[lastIndex] = value;
			m_LastIndex.store(nextIndex);
			return true;
		}

		return false;
	}

	bool Pop(Type*& pValue)
	{
		auto lastIndex = m_LastIndex.load();
		auto currIndex = m_CurrIndex.load();
		auto nextIndex = GetNextIndex(currIndex);
		while (currIndex != lastIndex && !m_CurrIndex.compare_exchange_weak(currIndex, nextIndex))
		{
			nextIndex = GetNextIndex(currIndex);
		}

		if (currIndex != lastIndex)
		{
			pValue = &m_Queue[currIndex];
			return true;
		}

		return false;
	}

	BvVector& GetContainer() const { return m_Queue; }

private:
	u32 GetNextIndex(u32 currIndex)
	{
		return (currIndex + 1) % (u32)m_Queue.Size();
	}

private:
	BvVector<Type> m_Queue;
	std::atomic<u32> m_LastIndex{};
	std::atomic<u32> m_CurrIndex{};
	BvSpinlock m_AddLock;
};