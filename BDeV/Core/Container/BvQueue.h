#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/Memory/BvMemory.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"


template<typename Type>
class BvQueue
{
public:
	BvQueue();
	explicit BvQueue(IBvMemoryArena* pArena, size_t reserveSize = 0);
	BvQueue(std::initializer_list<Type> list, IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA);
	BvQueue(const BvQueue& rhs);
	BvQueue(BvQueue&& rhs) noexcept;

	BvQueue& operator =(const BvQueue& rhs);
	BvQueue& operator =(BvQueue&& rhs) noexcept;
	BvQueue& operator =(std::initializer_list<Type> list);

	~BvQueue();

	// Allocator
	IBvMemoryArena* GetAllocator() const;
	void SetAllocator(IBvMemoryArena* pArena, size_t reserveSize = 0);

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
	IBvMemoryArena* m_pArena = nullptr;
	u32 m_Size = 0;
	u32 m_Capacity = 0;
	u32 m_Front = 0;
};


template<typename Type>
BvQueue<Type>::BvQueue()
	: m_pArena(BV_DEFAULT_MEMORY_ARENA)
{
}


template<typename Type>
inline BvQueue<Type>::BvQueue(IBvMemoryArena* pArena, size_t reserveSize)
	: m_pArena(pArena)
{
	if (reserveSize)
	{
		Reserve(reserveSize);
	}
}


template<typename Type>
BvQueue<Type>::BvQueue(std::initializer_list<Type> list, IBvMemoryArena* pArena)
	: m_pArena(pArena)
{
	Grow(list.size());

	m_Size = list.size();
	for (auto i = 0; i < m_Size; i++)
	{
		new (&m_pData[i]) Type(list[i]);
	}
}


template<typename Type>
BvQueue<Type>::BvQueue(const BvQueue& rhs)
	: m_pArena(rhs.m_pArena)
{
	Grow(rhs.m_Size);

	m_Size = rhs.m_Size;
	for (auto i = 0; i < m_Size; i++)
	{
		new (&m_pData[i]) Type(rhs.m_pData[(rhs.m_Front + i) % rhs.m_Capacity]);
	}
}


template<typename Type>
BvQueue<Type>::BvQueue(BvQueue&& rhs) noexcept
{
	*this = std::move(rhs);
}


template<typename Type>
BvQueue<Type>& BvQueue<Type>::operator =(const BvQueue& rhs)
{
	if (this != &rhs)
	{
		Destroy();
		SetAllocator(rhs.m_pArena);
		Grow(rhs.m_Size);

		m_Size = rhs.m_Size;
		for (auto i = 0; i < m_Size; i++)
		{
			new (&m_pData[i]) Type(rhs.m_pData[(rhs.m_Front + i) % rhs.m_Capacity]);
		}
	}

	return *this;
}


template<typename Type>
BvQueue<Type>& BvQueue<Type>::operator =(BvQueue&& rhs) noexcept
{
	if (this != &rhs)
	{
		std::swap(m_pData, rhs.m_pData);
		std::swap(m_pArena, rhs.m_pArena);
		std::swap(m_Size, rhs.m_Size);
		std::swap(m_Capacity, rhs.m_Capacity);
		std::swap(m_Front, rhs.m_Front);
	}

	return *this;
}


template<typename Type>
BvQueue<Type>& BvQueue<Type>::operator =(std::initializer_list<Type> list)
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


template<typename Type>
inline BvQueue<Type>::~BvQueue()
{
	Destroy();
}


template<typename Type>
inline IBvMemoryArena* BvQueue<Type>::GetAllocator() const
{
	return m_pArena;
}


template<typename Type>
inline void BvQueue<Type>::SetAllocator(IBvMemoryArena* pArena, size_t reserveSize)
{
	BV_ASSERT(pArena != nullptr, "Memory arena can't be nullptr");
	BV_ASSERT(m_pData == nullptr, "Can't change allocators after allocations have been made");
	//if (m_pArena == pArena)
	//{
	//	return;
	//}

	//if (m_Capacity > 0)
	//{
	//	Type* pNewData = reinterpret_cast<Type*>(m_pArena ? BV_MALLOC(*m_pArena, m_Capacity * sizeof(Type), alignof(Type)) : BV_ALLOC(m_Capacity * sizeof(Type), alignof(Type)));
	//	for (auto i = 0u; i < m_Size; i++)
	//	{
	//		new (&pNewData[i]) Type(std::move(m_pData[(m_Front + i) % m_Capacity]));
	//	}

	//	Clear();
	//	m_pArena ? BV_MFREE(*m_pArena, m_pData) : BV_FREE(m_pData);
	//	m_pData = pNewData;
	//}

	m_pArena = pArena;
	if (reserveSize)
	{
		Reserve(reserveSize);
	}
}


template<typename Type>
void BvQueue<Type>::PushBack(const Type& value)
{
	EmplaceBack(value);
}


template<typename Type>
void BvQueue<Type>::PushBack(Type&& value)
{
	EmplaceBack(std::move(value));
}


template<typename Type>
template<typename... Args>
Type& BvQueue<Type>::EmplaceBack(Args&&... args)
{
	if (m_Size == m_Capacity)
	{
		Grow(CalculateNewContainerSize(m_Capacity));
	}

	auto backIndex = (m_Front + m_Size++) % m_Capacity;
	new (&m_pData[backIndex]) Type(std::forward<Args>(args)...);

	return m_pData[backIndex];
}


template<typename Type>
void BvQueue<Type>::PushFront(const Type& value)
{
	EmplaceFront(value);
}


template<typename Type>
void BvQueue<Type>::PushFront(Type&& value)
{
	EmplaceFront(std::move(value));
}


template<typename Type>
template<typename... Args>
Type& BvQueue<Type>::EmplaceFront(Args&&... args)
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


template<typename Type>
bool BvQueue<Type>::PopFront()
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


template<typename Type>
bool BvQueue<Type>::PopFront(Type& value)
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


template<typename Type>
bool BvQueue<Type>::PopBack()
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


template<typename Type>
bool BvQueue<Type>::PopBack(Type& value)
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


template<typename Type>
Type& BvQueue<Type>::Front()
{
	BV_ASSERT(!IsEmpty(), "Queue is empty!");
	return m_pData[m_Front];
}


template<typename Type>
const Type& BvQueue<Type>::Front() const
{
	BV_ASSERT(!IsEmpty(), "Queue is empty!");
	return m_pData[m_Front];
}


template<typename Type>
Type& BvQueue<Type>::Back()
{
	BV_ASSERT(!IsEmpty(), "Queue is empty!");
	return m_pData[(m_Front + (m_Size - 1)) % m_Capacity];
}


template<typename Type>
const Type& BvQueue<Type>::Back() const
{
	BV_ASSERT(!IsEmpty(), "Queue is empty!");
	return m_pData[(m_Front + (m_Size - 1)) % m_Capacity];
}


template<typename Type>
u32 BvQueue<Type>::Size() const
{
	return m_Size;
}


template<typename Type>
u32 BvQueue<Type>::Capacity() const
{
	return m_Capacity;
}


template<typename Type>
bool BvQueue<Type>::IsEmpty() const
{
	return m_Size == 0;
}


template<typename Type>
inline void BvQueue<Type>::Clear()
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


template<typename Type>
void BvQueue<Type>::Reserve(u32 size)
{
	Grow(size);
}


template<typename Type>
inline void BvQueue<Type>::Grow(u32 size)
{
	if (size <= m_Capacity)
	{
		return;
	}

	Type* pNewData = reinterpret_cast<Type*>(m_pArena->Allocate(size * sizeof(Type), alignof(Type)));
	for (auto i = 0u; i < m_Size; i++)
	{
		auto curr = (m_Front + i) % m_Capacity;
		new (&pNewData[i]) Type(std::move(m_pData[curr]));
		m_pData[curr].~Type();
	}

	m_Capacity = size;
	if (m_pData)
	{
		m_pArena->Free(m_pData);
	}
	m_pData = pNewData;
	m_Front = 0;
}


template<typename Type>
inline void BvQueue<Type>::Destroy()
{
	Clear();
	if (m_pData)
	{
		m_pArena->Free(m_pData);
	}
}