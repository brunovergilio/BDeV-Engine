#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/Memory/BvMemory.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"


template<typename Type, typename MemoryArenaType = IBvMemoryArena>
class BvQueue
{
public:
	BvQueue();
	BvQueue(MemoryArenaType* pAllocator);
	BvQueue(std::initializer_list<Type> list, MemoryArenaType* pAllocator = nullptr);
	BvQueue(const BvQueue& rhs);
	BvQueue(BvQueue&& rhs) noexcept;

	BvQueue& operator =(const BvQueue& rhs);
	BvQueue& operator =(BvQueue&& rhs) noexcept;
	BvQueue& operator =(std::initializer_list<Type> list);

	~BvQueue();

	// Allocator
	MemoryArenaType* GetAllocator() const;
	void SetAllocator(MemoryArenaType* pAllocator);

	void PushBack(const Type& value);
	void PushBack(Type&& value);
	template<typename... Args>
	Type& EmplaceBack(Args&&... args);

	void PushFront(const Type& value);
	void PushFront(Type&& value);
	template<typename... Args>
	Type& EmplaceFront(Args&&... args);

	bool PopFront();
	bool PopFront(Type& value);

	bool PopBack();
	bool PopBack(Type& value);

	Type& Front();
	const Type& Front() const;

	Type& Back();
	const Type& Back() const;

	u32 Size() const;
	u32 Capacity() const;
	bool IsEmpty() const;

	void Clear();
	void Reserve(u32 size);

private:
	void Grow(u32 size);
	void Destroy();

private:
	Type* m_pData = nullptr;
	MemoryArenaType* m_pAllocator = nullptr;
	u32 m_Size = 0;
	u32 m_Capacity = 0;
	u32 m_Front = 0;
};


template<typename Type, typename MemoryArenaType>
BvQueue<Type, MemoryArenaType>::BvQueue()
{
}


template<typename Type, typename MemoryArenaType>
inline BvQueue<Type, MemoryArenaType>::BvQueue(MemoryArenaType* pAllocator)
	: m_pAllocator(pAllocator)
{
}


template<typename Type, typename MemoryArenaType>
BvQueue<Type, MemoryArenaType>::BvQueue(std::initializer_list<Type> list, MemoryArenaType* pAllocator)
	: m_pAllocator(pAllocator)
{
	Grow(list.size());

	m_Size = list.size();
	for (auto i = 0; i < m_Size; i++)
	{
		new (&m_pData[i]) Type(list[i]);
	}
}


template<typename Type, typename MemoryArenaType>
BvQueue<Type, MemoryArenaType>::BvQueue(const BvQueue& rhs)
	: m_pAllocator(rhs.m_pAllocator)
{
	Grow(rhs.m_Size);

	m_Size = rhs.m_Size;
	for (auto i = 0; i < m_Size; i++)
	{
		new (&m_pData[i]) Type(rhs.m_pData[(rhs.m_Front + i) % rhs.m_Capacity]);
	}
}


template<typename Type, typename MemoryArenaType>
BvQueue<Type, MemoryArenaType>::BvQueue(BvQueue&& rhs) noexcept
{
	*this = std::move(rhs);
}


template<typename Type, typename MemoryArenaType>
BvQueue<Type, MemoryArenaType>& BvQueue<Type, MemoryArenaType>::operator =(const BvQueue& rhs)
{
	if (this != &rhs)
	{
		Destroy();
		SetAllocator(rhs.m_pAllocator);
		Grow(rhs.m_Size);

		m_Size = rhs.m_Size;
		for (auto i = 0; i < m_Size; i++)
		{
			new (&m_pData[i]) Type(rhs.m_pData[(rhs.m_Front + i) % rhs.m_Capacity]);
		}
	}

	return *this;
}


template<typename Type, typename MemoryArenaType>
BvQueue<Type, MemoryArenaType>& BvQueue<Type, MemoryArenaType>::operator =(BvQueue&& rhs) noexcept
{
	if (this != &rhs)
	{
		std::swap(m_pData, rhs.m_pData);
		std::swap(m_pAllocator, rhs.m_pAllocator);
		std::swap(m_Size, rhs.m_Size);
		std::swap(m_Capacity, rhs.m_Capacity);
		std::swap(m_Front, rhs.m_Front);
	}

	return *this;
}


template<typename Type, typename MemoryArenaType>
BvQueue<Type, MemoryArenaType>& BvQueue<Type, MemoryArenaType>::operator =(std::initializer_list<Type> list)
{
	Clear();

	auto size = list.size();
	Grow(size);

	m_Size = size;
	for (auto i = 0; i < m_Size; i++)
	{
		new (&m_pData[(m_Front + i) % m_Capacity]) Type(list[i]);
	}

	return *this;
}


template<typename Type, typename MemoryArenaType>
inline BvQueue<Type, MemoryArenaType>::~BvQueue()
{
	Destroy();
}


template<typename Type, typename MemoryArenaType>
inline MemoryArenaType* BvQueue<Type, MemoryArenaType>::GetAllocator() const
{
	return m_pAllocator;
}


template<typename Type, typename MemoryArenaType>
inline void BvQueue<Type, MemoryArenaType>::SetAllocator(MemoryArenaType* pAllocator)
{
	if (m_pAllocator == pAllocator)
	{
		return;
	}

	if (m_Capacity > 0)
	{
		Type* pNewData = pAllocator ? BV_MNEW_ARRAY(*pAllocator, Type, m_Capacity) : BV_NEW_ARRAY(Type, m_Capacity);
		for (auto i = 0u; i < m_Size; i++)
		{
			new (&pNewData[i]) Type(std::move(m_pData[(m_Front + i) % m_Capacity]));
		}

		Clear();
		m_pAllocator ? BV_MDELETE_ARRAY(*m_pAllocator, m_pData) : BV_DELETE_ARRAY(m_pData);
		m_pData = pNewData;
	}

	m_pAllocator = pAllocator;
}


template<typename Type, typename MemoryArenaType>
void BvQueue<Type, MemoryArenaType>::PushBack(const Type& value)
{
	EmplaceBack(value);
}


template<typename Type, typename MemoryArenaType>
void BvQueue<Type, MemoryArenaType>::PushBack(Type&& value)
{
	EmplaceBack(std::move(value));
}


template<typename Type, typename MemoryArenaType>
template<typename... Args>
Type& BvQueue<Type, MemoryArenaType>::EmplaceBack(Args&&... args)
{
	if (m_Size == m_Capacity)
	{
		Grow(CalculateNewContainerSize(m_Capacity));
	}

	auto backIndex = (m_Front + m_Size++) % m_Capacity;
	new (&m_pData[backIndex]) Type(std::forward<Args>(args)...);

	return m_pData[backIndex];
}


template<typename Type, typename MemoryArenaType>
void BvQueue<Type, MemoryArenaType>::PushFront(const Type& value)
{
	EmplaceFront(value);
}


template<typename Type, typename MemoryArenaType>
void BvQueue<Type, MemoryArenaType>::PushFront(Type&& value)
{
	EmplaceFront(std::move(value));
}


template<typename Type, typename MemoryArenaType>
template<typename... Args>
Type& BvQueue<Type, MemoryArenaType>::EmplaceFront(Args&&... args)
{
	if (m_Size == m_Capacity)
	{
		Grow(CalculateNewContainerSize(m_Capacity));
	}

	m_Front = (m_Front - 1 + m_Capacity) % m_Capacity;
	new (&m_pData[m_Front]) Type(std::forward<Args>(args)...);
	++m_Size;

	return m_pData[m_Front];
}


template<typename Type, typename MemoryArenaType>
bool BvQueue<Type, MemoryArenaType>::PopFront()
{
	if (IsEmpty())
	{
		return false;
	}

	if constexpr (!std::is_trivially_destructible_v<Type>)
	{
		m_pData[m_Front].~Type();
	}
	m_Front = (m_Front + 1) % m_Capacity;
	--m_Size;

	return true;
}


template<typename Type, typename MemoryArenaType>
bool BvQueue<Type, MemoryArenaType>::PopFront(Type& value)
{
	if (IsEmpty())
	{
		return false;
	}

	value = std::move(m_pData[m_Front]);
	m_Front = (m_Front + 1) % m_Capacity;
	--m_Size;

	return true;
}


template<typename Type, typename MemoryArenaType>
bool BvQueue<Type, MemoryArenaType>::PopBack()
{
	if (IsEmpty())
	{
		return false;
	}

	auto backIndex = (m_Front + --m_Size) % m_Capacity;
	if constexpr (!std::is_trivially_destructible_v<Type>)
	{
		m_pData[backIndex].~Type();
	}

	return true;
}


template<typename Type, typename MemoryArenaType>
bool BvQueue<Type, MemoryArenaType>::PopBack(Type& value)
{
	if (IsEmpty())
	{
		return false;
	}

	auto backIndex = (m_Front + --m_Size) % m_Capacity;
	value = std::move(m_pData[backIndex]);
	if constexpr (!std::is_trivially_destructible_v<Type>)
	{
		m_pData[backIndex].~Type();
	}

	return true;
}


template<typename Type, typename MemoryArenaType>
Type& BvQueue<Type, MemoryArenaType>::Front()
{
	BvAssert(!IsEmpty(), "Queue is empty!");
	return m_pData[m_Front];
}


template<typename Type, typename MemoryArenaType>
const Type& BvQueue<Type, MemoryArenaType>::Front() const
{
	BvAssert(!IsEmpty(), "Queue is empty!");
	return m_pData[m_Front];
}


template<typename Type, typename MemoryArenaType>
Type& BvQueue<Type, MemoryArenaType>::Back()
{
	BvAssert(!IsEmpty(), "Queue is empty!");
	return m_pData[(m_Front + (m_Size - 1)) % m_Capacity];
}


template<typename Type, typename MemoryArenaType>
const Type& BvQueue<Type, MemoryArenaType>::Back() const
{
	BvAssert(!IsEmpty(), "Queue is empty!");
	return m_pData[(m_Front + (m_Size - 1)) % m_Capacity];
}


template<typename Type, typename MemoryArenaType>
u32 BvQueue<Type, MemoryArenaType>::Size() const
{
	return m_Size;
}


template<typename Type, typename MemoryArenaType>
u32 BvQueue<Type, MemoryArenaType>::Capacity() const
{
	return m_Capacity;
}


template<typename Type, typename MemoryArenaType>
bool BvQueue<Type, MemoryArenaType>::IsEmpty() const
{
	return m_Size == 0;
}


template<typename Type, typename MemoryArenaType>
inline void BvQueue<Type, MemoryArenaType>::Clear()
{
	if constexpr (!std::is_trivially_destructible_v<Type>)
	{
		while (m_Size > 0)
		{
			m_pData[(m_Front + m_Size-- - 1) % m_Capacity].~Type();
		}
	}
	else
	{
		m_Size = 0;
	}
}


template<typename Type, typename MemoryArenaType>
void BvQueue<Type, MemoryArenaType>::Reserve(u32 size)
{
	Grow(size);
}


template<typename Type, typename MemoryArenaType>
inline void BvQueue<Type, MemoryArenaType>::Grow(u32 size)
{
	if (size <= m_Capacity)
	{
		return;
	}

	Type* pNewData = m_pAllocator ? BV_MNEW_ARRAY(*m_pAllocator, Type, size) : BV_NEW_ARRAY(Type, size);
	for (auto i = 0u; i < m_Size; i++)
	{
		auto curr = (m_Front + i) % m_Capacity;
		new (&pNewData[i]) Type(std::move(m_pData[curr]));
		m_pData[curr].~Type();
	}

	m_Capacity = size;
	if (m_pData)
	{
		m_pAllocator ? BV_MDELETE_ARRAY(*m_pAllocator, m_pData) : BV_DELETE_ARRAY(m_pData);
	}
	m_pData = pNewData;
	m_Front = 0;
}


template<typename Type, typename MemoryArenaType>
inline void BvQueue<Type, MemoryArenaType>::Destroy()
{
	Clear();
	if (m_pData)
	{
		m_pAllocator ? BV_MDELETE_ARRAY(*m_pAllocator, m_pData) : BV_DELETE_ARRAY(m_pData);
	}
}