#pragma once


// To-Do: Get rid of this and create a regular queue, and do external synchronization, just like with the other containers


#include "BvCore/BvDebug.h"
#include "BvCore/System/Threading/BvSync.h"
#include "BvCore/Utils/BvUtils.h"


constexpr u32 kMaxDefaultSize = 1024;


template<class Type, class LockType = BvNoLock>
class BvSafeQueue
{
public:
	BvSafeQueue(const u32 size = kMaxDefaultSize);
	~BvSafeQueue();

	void Enqueue(const Type & value);
	void Enqueue(const Type * pValues, const u32 count);
	bool Dequeue(Type & value);

private:
	Type * m_pData = nullptr;
	LockType m_Lock;
	u32 m_EnqueueIndex = 0;
	u32 m_DequeueIndex = 0;
	u32 m_Size = 0;
};


template<class Type, class LockType>
inline BvSafeQueue<Type, LockType>::BvSafeQueue(const u32 size)
	: m_pData(new Type[size]), m_Size(size)
{
	BvAssert((size & (size - 1)) == 0);
}


template<class Type, class LockType>
inline BvSafeQueue<Type, LockType>::~BvSafeQueue()
{
	BvDelete(m_pData);
}


template<class Type, class LockType>
inline void BvSafeQueue<Type, LockType>::Enqueue(const Type & value)
{
	BvScopedLock lock(m_Lock);

	m_pData[m_EnqueueIndex++ & (m_Size - 1)] = value;
	if (m_EnqueueIndex == m_Size) { m_EnqueueIndex = 0; }
}


template<class Type, class LockType>
inline void BvSafeQueue<Type, LockType>::Enqueue(const Type * pValues, const u32 count)
{
	BvAssert(count < m_Size);
	BvAssert(pValues != nullptr);

	BvScopedLock lock(m_Lock);

	for (auto i = 0U; i < count; i++)
	{
		m_pData[m_EnqueueIndex++ & (m_Size - 1)] = pValues[i];
	}
}


template<class Type, class LockType>
inline bool BvSafeQueue<Type, LockType>::Dequeue(Type & value)
{
	BvScopedLock lock(m_Lock);

	if (m_EnqueueIndex == m_DequeueIndex)
	{
		return false;
	}

	value = m_pData[m_DequeueIndex++ & (m_Size - 1)];

	if (m_DequeueIndex == m_Size) { m_DequeueIndex = 0; }

	return true;
}