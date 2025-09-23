#pragma once


// BvVector
// Container class, with variable-sized memory. Once memory has been allocated for a specific instance, the specified amount becomes
// the maximum size for that instance. If the size needs to be increased, memory is reallocated again.
// The class can reserve and/or resize its underlying container, but not shrink.


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Container/BvIterator.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BDeV/Core/System/Memory/BvMemory.h"


template<typename Type>
class BvVector
{
public:
	using Iterator = RandomIterator<Type>;
	using ConstIterator = RandomIterator<const Type>;
	using ReverseIterator = RandomReverseIterator<Type>;
	using ConstReverseIterator = RandomReverseIterator<const Type>;

	BvVector(); // Default
	explicit BvVector(IBvMemoryArena* pArena, size_t reserveSize = 0); // Allocator
	explicit BvVector(const size_t size, const Type & val = Type(), IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA); // Fill
	explicit BvVector(Iterator start, Iterator end, IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA); // Range
	BvVector(std::initializer_list<Type> list, IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA); // Initializer List
	BvVector(const BvVector & rhs); // Copy
	BvVector(BvVector && rhs) noexcept; // Move

	BvVector& operator =(const BvVector & rhs); // Copy Assignment
	BvVector& operator =(BvVector && rhs) noexcept; // Move Assignment
	BvVector& operator =(std::initializer_list<Type> list); // Copy Assignment

	~BvVector();

	// Allocator
	IBvMemoryArena* GetAllocator() const;
	void SetAllocator(IBvMemoryArena* pArena, size_t reserveSize = 0);

	// Iterator
	Iterator begin() { return Iterator(m_pData); }
	Iterator end() { return Iterator(m_pData + m_Size); }
	ConstIterator begin() const { return ConstIterator(m_pData); }
	ConstIterator end() const { return ConstIterator(m_pData + m_Size); }
	ConstIterator cbegin() const { return begin(); }
	ConstIterator cend() const { return end(); }

	// Reverse Iterator
	ReverseIterator rbegin() { return ReverseIterator(m_pData); }
	ReverseIterator rend() { return ReverseIterator(m_pData + m_Size); }
	ConstReverseIterator rbegin() const { return ConstReverseIterator(m_pData); }
	ConstReverseIterator rend() const { ConstReverseIterator(m_pData + m_Size); }
	ConstReverseIterator crbegin() const { return rbegin(); }
	ConstReverseIterator crend() const { return rend(); }

	// Capacity
	const size_t Size() const;
	const size_t Capacity() const;
	const bool Empty() const;
	void Resize(const size_t size, const Type & value = Type());
	void Reserve(const size_t size);

	// Element Access
	Type & operator [](const size_t index);
	const Type & operator [](const size_t index) const;
	Type & At(const size_t index);
	const Type & At(const size_t index) const;
	Type & Front();
	const Type & Front() const;
	Type & Back();
	const Type & Back() const;
	Type * Data();
	const Type * const Data() const;

	// Modifiers
	void Assign(Iterator start, Iterator end); // Range
	void Assign(const size_t size, const Type & val); // Fill
	void Assign(std::initializer_list<Type> list); // Initializer List
	Type& PushBack(const Type & value);
	Type& PushBack(Type && value);
	void PopBack();
	void PopFront();
	Iterator Insert(ConstIterator position, const Type & value);
	Iterator Insert(ConstIterator position, const size_t count, const Type & value);
	Iterator Insert(ConstIterator position, Iterator first, Iterator last);
	Iterator Insert(ConstIterator position, Type && value);
	Iterator Insert(ConstIterator position, std::initializer_list<Type> list);
	Iterator Erase(size_t index);
	Iterator Erase(size_t startIndex, size_t count);
	Iterator Erase(ConstIterator position);
	Iterator Erase(ConstIterator first, ConstIterator last);
	void EraseUnsorted(size_t index);
	void Clear();
	template <class... Args>
	Iterator Emplace(ConstIterator position, Args&&... args);
	template <class... Args>
	Type& EmplaceBack(Args&&... args);
	size_t Find(const Type& value) const;
	bool Contains(const Type& value) const;

private:
	void Grow(const size_t size);
	void Destroy();

private:
	Type* m_pData = nullptr;
	IBvMemoryArena* m_pArena = nullptr;
	size_t m_Size = 0;
	size_t m_Capacity = 0;
};


template<typename Type>
inline BvVector<Type>::BvVector()
	: m_pArena(BV_DEFAULT_MEMORY_ARENA)
{
}


template<typename Type>
inline BvVector<Type>::BvVector(IBvMemoryArena* pArena, size_t reserveSize)
	: m_pArena(pArena)
{
	if (reserveSize)
	{
		Reserve(reserveSize);
	}
}


template<typename Type>
inline BvVector<Type>::BvVector(const size_t size, const Type & val, IBvMemoryArena* pArena)
	: m_pArena(pArena)
{
	Assign(size, val);
}


template<typename Type>
inline BvVector<Type>::BvVector(Iterator start, Iterator end, IBvMemoryArena* pArena)
	: m_pArena(pArena)
{
	Assign(start, end);
}


template<typename Type>
inline BvVector<Type>::BvVector(std::initializer_list<Type> list, IBvMemoryArena* pArena)
	: m_pArena(pArena)
{
	Assign(list);
}


template<typename Type>
inline BvVector<Type>::BvVector(const BvVector & rhs)
	: m_pArena(rhs.m_pArena)
{
	Grow(rhs.m_Size);

	m_Size = rhs.m_Size;
	for (auto i = 0; i < m_Size; i++)
	{
		new (&m_pData[i]) Type(rhs.m_pData[i]);
	}
}


template<typename Type>
inline BvVector<Type>::BvVector(BvVector && rhs) noexcept
{
	*this = std::move(rhs);
}


template<typename Type>
inline BvVector<Type>& BvVector<Type>::operator=(const BvVector & rhs)
{
	if (this != &rhs)
	{
		Destroy();
		SetAllocator(rhs.m_pArena);
		Grow(rhs.m_Size);

		m_Size = rhs.m_Size;
		for (auto i = 0; i < m_Size; i++)
		{
			new (&m_pData[i]) Type(rhs.m_pData[i]);
		}
	}

	return *this;
}


template<typename Type>
inline BvVector<Type>& BvVector<Type>::operator=(BvVector && rhs) noexcept
{
	if (this != &rhs)
	{
		std::swap(m_pData, rhs.m_pData);
		std::swap(m_pArena, rhs.m_pArena);
		std::swap(m_Size, rhs.m_Size);
		std::swap(m_Capacity, rhs.m_Capacity);
	}

	return *this;
}


template<typename Type>
inline BvVector<Type>& BvVector<Type>::operator=(std::initializer_list<Type> list)
{
	Clear();

	auto size = list.size();
	Grow(size);

	m_Size = size;
	for (auto i = 0; i < m_Size; i++)
	{
		new (&m_pData[i]) Type(list[i]);
	}

	return *this;
}


template<typename Type>
inline BvVector<Type>::~BvVector()
{
	Destroy();
}


template<typename Type>
inline IBvMemoryArena* BvVector<Type>::GetAllocator() const
{
	return m_pArena;
}


template<typename Type>
inline void BvVector<Type>::SetAllocator(IBvMemoryArena* pArena, size_t reserveSize)
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
	//		new (&pNewData[i]) Type(std::move(m_pData[i]));
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
inline const size_t BvVector<Type>::Size() const
{
	return m_Size;
}

template<typename Type>
inline const size_t BvVector<Type>::Capacity() const
{
	return m_Capacity;
}

template<typename Type>
inline const bool BvVector<Type>::Empty() const
{
	return m_Size == 0;
}

template<typename Type>
inline void BvVector<Type>::Resize(const size_t size, const Type & value)
{
	if (size > m_Size)
	{
		Grow(size);

		for (auto i = m_Size; i < size; i++)
		{
			new (&m_pData[i]) Type(value);
		}

		m_Size = size;
	}
	else if (size < m_Size)
	{
		if constexpr (!std::is_trivially_destructible_v<Type>)
		{
			for (auto i = m_Size; i > size; i--)
			{
				m_pData[i - 1].~Type();
			}
		}

		m_Size = size;
	}
}

template<typename Type>
inline void BvVector<Type>::Reserve(const size_t size)
{
	Grow(size);
}

template<typename Type>
inline Type & BvVector<Type>::operator[](const size_t index)
{
	BV_ASSERT(m_Size > 0 && index < m_Size, "Index out of bounds");
	return m_pData[index];
}

template<typename Type>
inline const Type & BvVector<Type>::operator[](const size_t index) const
{
	BV_ASSERT(m_Size > 0 && index < m_Size, "Index out of bounds");
	return m_pData[index];
}

template<typename Type>
inline Type & BvVector<Type>::At(const size_t index)
{
	BV_ASSERT(m_Size > 0 && index < m_Size, "Index out of bounds");
	return m_pData[index];
}

template<typename Type>
inline const Type & BvVector<Type>::At(const size_t index) const
{
	BV_ASSERT(m_Size > 0 && index < m_Size, "Index out of bounds");
	return m_pData[index];
}

template<typename Type>
inline Type & BvVector<Type>::Front()
{
	BV_ASSERT(m_Size > 0, "Vector is empty");
	return m_pData[0];
}

template<typename Type>
inline const Type & BvVector<Type>::Front() const
{
	BV_ASSERT(m_Size > 0, "Vector is empty");
	return m_pData[0];
}

template<typename Type>
inline Type & BvVector<Type>::Back()
{
	BV_ASSERT(m_Size > 0, "Vector is empty");
	return m_pData[m_Size - 1];
}

template<typename Type>
inline const Type & BvVector<Type>::Back() const
{
	BV_ASSERT(m_Size > 0, "Vector is empty");
	return m_pData[m_Size - 1];
}

template<typename Type>
inline Type * BvVector<Type>::Data()
{
	return m_pData;
}

template<typename Type>
inline const Type * const BvVector<Type>::Data() const
{
	return m_pData;
}

template<typename Type>
inline void BvVector<Type>::Assign(Iterator start, Iterator end)
{
	Clear();

	auto size = end - start;
	Grow(size);

	m_Size = size;
	for (auto it = start; it != end; it++)
	{
		new (&m_pData[m_Size++]) Type(*it);
	}
}

template<typename Type>
inline void BvVector<Type>::Assign(const size_t size, const Type & val)
{
	Clear();

	Grow(size);

	m_Size = size;
	for (u32 i = 0; i < m_Size; i++)
	{
		new (&m_pData[i]) Type(val);
	}
}

template<typename Type>
inline void BvVector<Type>::Assign(std::initializer_list<Type> list)
{
	Clear();

	auto size = list.size();
	Grow(size);

	m_Size = size;
	for (auto i = 0; i < m_Size; i++)
	{
		new (&m_pData[i]) Type(list[i]);
	}
}

template<typename Type>
inline Type& BvVector<Type>::PushBack(const Type & value)
{
	return EmplaceBack(value);
}

template<typename Type>
inline Type& BvVector<Type>::PushBack(Type && value)
{
	return EmplaceBack(std::move(value));
}

template<typename Type>
inline void BvVector<Type>::PopBack()
{
	if (m_Size > 0)
	{
		if constexpr (!std::is_trivially_destructible_v<Type>)
		{
			m_pData[m_Size - 1].~Type();
		}
		--m_Size;
	}
}


template<typename Type>
inline void BvVector<Type>::PopFront()
{
	Erase(cbegin());
}


template<typename Type>
inline typename BvVector<Type>::Iterator BvVector<Type>::Insert(ConstIterator position, const Type & value)
{
	auto pos = position - ConstIterator(m_pData);
	if (pos > m_Size)
	{
		pos = m_Size;
	}

	if (m_Size == m_Capacity)
	{
		Grow(m_Capacity + 1);
	}

	if (pos < m_Size)
	{
		for (auto i = m_Size; i > pos; i--)
		{
			m_pData[i] = std::move(m_pData[i - 1]);
		}
	}

	new (&m_pData[pos]) Type(value);
	m_Size++;

	return Iterator(m_pData + pos);
}

template<typename Type>
inline typename BvVector<Type>::Iterator BvVector<Type>::Insert(ConstIterator position, const size_t count, const Type & value)
{
	auto pos = position - ConstIterator(m_pData);
	if (pos > m_Size)
	{
		pos = m_Size;
	}

	if (m_Size + count > m_Capacity)
	{
		Grow(m_Size + count + 1);
	}

	if (pos < m_Size)
	{
		for (auto i = m_Size + count - 1; i > pos + count - 1; i--)
		{
			m_pData[i] = std::move(m_pData[i - count]);
		}
	}

	for (auto i = 0; i < count; i++)
	{
		new (&m_pData[pos + i]) Type(value);
	}
	m_Size += count;

	return Iterator(m_pData + pos);
}

template<typename Type>
inline typename BvVector<Type>::Iterator BvVector<Type>::Insert(ConstIterator position, Iterator first, Iterator last)
{
	auto pos = position - ConstIterator(m_pData);
	if (pos > m_Size)
	{
		pos = m_Size;
	}

	auto count = last - first;
	if (m_Size + count > m_Capacity)
	{
		Grow(m_Size + count + 1);
	}

	if (pos < m_Size)
	{
		for (auto i = m_Size + count - 1; i > pos + count - 1; i--)
		{
			m_pData[i] = std::move(m_pData[i - count]);
		}
	}

	auto it = first;
	for (auto i = 0; i < count; i++)
	{
		new (&m_pData[pos + i]) Type(*(it++));
	}

	m_Size += count;

	return Iterator(m_pData + pos);
}

template<typename Type>
inline typename BvVector<Type>::Iterator BvVector<Type>::Insert(ConstIterator position, Type && value)
{
	auto pos = position - ConstIterator(m_pData);
	if (pos > m_Size)
	{
		pos = m_Size;
	}

	if (m_Size == m_Capacity)
	{
		Grow(m_Capacity + 1);
	}

	if (pos < m_Size)
	{
		for (auto i = m_Size; i > pos; i--)
		{
			m_pData[i] = std::move(m_pData[i - 1]);
		}
	}

	new (&m_pData[pos]) Type(std::move(value));
	m_Size++;

	return Iterator(m_pData + pos);
}

template<typename Type>
inline typename BvVector<Type>::Iterator BvVector<Type>::Insert(ConstIterator position, std::initializer_list<Type> list)
{
	auto pos = position - ConstIterator(m_pData);
	if (pos > m_Size)
	{
		pos = m_Size;
	}

	auto count = list.size();
	if (m_Size + count > m_Capacity)
	{
		Grow(m_Size + count + 1);
	}

	if (pos < m_Size)
	{
		for (auto i = m_Size + count - 1; i > pos + count - 1; i--)
		{
			m_pData[i] = std::move(m_pData[i - count]);
		}
	}

	auto it = list.begin();
	for (auto i = 0; i < count; i++)
	{
		new (&m_pData[pos + i]) Type(*(it++));
	}

	m_Size += count;

	return Iterator(m_pData + pos);
}


template<typename Type>
inline typename BvVector<Type>::Iterator BvVector<Type>::Erase(size_t index)
{
	if (index >= m_Size)
	{
		return Iterator(m_pData + m_Size);
	}

	if constexpr (!std::is_trivially_destructible_v<Type>)
	{
		m_pData[index].~Type();
	}

	if (index < m_Size - 1)
	{
		for (auto i = index; i < m_Size - 1; i++)
		{
			m_pData[i] = std::move(m_pData[i + 1]);
		}
	}

	m_Size--;

	return Iterator(m_pData + index);
}

template<typename Type>
inline typename BvVector<Type>::Iterator BvVector<Type>::Erase(size_t startIndex, size_t count)
{
	if (startIndex >= m_Size)
	{
		return Iterator(m_pData + m_Size);
	}

	if (startIndex + count > m_Size)
	{
		count = m_Size - startIndex;
	}

	if constexpr (!std::is_trivially_destructible_v<Type>)
	{
		for (auto i = startIndex; i < startIndex + count; i++)
		{
			m_pData[startIndex].~Type();
		}
	}

	if (startIndex < m_Size - 1)
	{
		for (auto i = startIndex + count; i < m_Size; i++)
		{
			m_pData[i - count] = std::move(m_pData[i]);
		}
	}

	m_Size -= count;

	return Iterator(m_pData + startIndex);
}


template<typename Type>
inline typename BvVector<Type>::Iterator BvVector<Type>::Erase(ConstIterator position)
{
	auto index = position - ConstIterator(m_pData);
	return Erase(index);
}

template<typename Type>
inline typename BvVector<Type>::Iterator BvVector<Type>::Erase(ConstIterator first, ConstIterator last)
{
	auto index = first - ConstIterator(m_pData);
	auto count = last - first;

	return Erase(index, count);
}


template<typename Type>
inline void BvVector<Type>::EraseUnsorted(size_t index)
{
	BV_ASSERT(m_Size > 0 && index < m_Size, "Index out of bounds");
	if (index < m_Size - 1)
	{
		std::swap(m_pData[index], m_pData[m_Size - 1]);
	}
	PopBack();
}

template<typename Type>
inline void BvVector<Type>::Clear()
{
	Internal::DestructArray(m_pData, m_Size);

	m_Size = 0;
}

template<typename Type>
template<class ...Args>
inline typename BvVector<Type>::Iterator BvVector<Type>::Emplace(ConstIterator position, Args&& ...args)
{
	auto pos = position - ConstIterator(m_pData);
	if (pos > m_Size)
	{
		pos = m_Size;
	}

	if (m_Size == m_Capacity)
	{
		Grow(CalculateNewContainerSize(m_Capacity));
	}

	if (pos < m_Size)
	{
		for (auto i = m_Size; i > pos; i--)
		{
			m_pData[i] = std::move(m_pData[i - 1]);
		}
	}

	new (&m_pData[pos]) Type(std::forward<Args>(args)...);
	m_Size++;

	return Iterator(m_pData + pos);
}

template<typename Type>
template<class ...Args>
inline Type& BvVector<Type>::EmplaceBack(Args&& ...args)
{
	if (m_Size == m_Capacity)
	{
		Grow(m_Capacity + (m_Capacity >> 1) + (m_Capacity <= 2));
	}

	new (&m_pData[m_Size]) Type(std::forward<Args>(args)...);

	return m_pData[m_Size++];
}

template<typename Type>
size_t BvVector<Type>::Find(const Type& value) const
{
	for (auto i = 0; i < m_Size; i++)
	{
		if (value == m_pData[i])
		{
			return i;
		}
	}

	return kU64Max;
}

template<typename Type>
inline bool BvVector<Type>::Contains(const Type& value) const
{
	return Find(value) != kU64Max;
}

template<typename Type>
inline void BvVector<Type>::Grow(const size_t size)
{
	if (size <= m_Capacity)
	{
		return;
	}

	Type* pNewData = reinterpret_cast<Type*>(m_pArena->Allocate(size * sizeof(Type), alignof(Type)));
	for (auto i = 0u; i < m_Size; i++)
	{
		new (&pNewData[i]) Type(std::move(m_pData[i]));
		if constexpr (!std::is_trivially_destructible_v<Type>)
		{
			m_pData[i].~Type();
		}
	}

	m_Capacity = size;
	if (m_pData)
	{
		m_pArena->Free(m_pData);
	}
	m_pData = pNewData;
}

template<typename Type>
inline void BvVector<Type>::Destroy()
{
	Clear();

	if (m_pData)
	{
		m_pArena->Free(m_pData);
		m_pData = nullptr;
	}

	m_Capacity = 0;
}