#pragma once


// BvVector
// Container class, with variable-sized memory. Once memory has been allocated for a specific instance, the specified amount becomes
// the maximum size for that instance. If the size needs to be increased, memory is reallocated again.
// The class can reserve and/or resize its underlying container


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Container/BvIterator.h"
#include "BDeV/Core/System/Debug/BvDebug.h"
#include "BDeV/Core/System/Memory/BvMemoryArena.h"
#include "BDeV/Core/System/Memory/BvMemory.h"


template<typename Type, typename MemoryArenaType = IBvMemoryArena>
class BvVector
{
public:
	using Iterator = RandomIterator<Type>;
	using ConstIterator = RandomIterator<const Type>;
	using ReverseIterator = RandomReverseIterator<Type>;
	using ConstReverseIterator = RandomReverseIterator<const Type>;

	BvVector(); // Default
	BvVector(MemoryArenaType* pAllocator); // Allocator
	explicit BvVector(const size_t size, const Type & val = Type(), MemoryArenaType* pAllocator = nullptr); // Fill
	explicit BvVector(Iterator start, Iterator end, MemoryArenaType* pAllocator = nullptr); // Range
	BvVector(std::initializer_list<Type> list, MemoryArenaType* pAllocator = nullptr); // Initializer List
	BvVector(const BvVector & rhs); // Copy
	BvVector(BvVector && rhs) noexcept; // Move

	BvVector& operator =(const BvVector & rhs); // Copy Assignment
	BvVector& operator =(BvVector && rhs) noexcept; // Move Assignment
	BvVector& operator =(std::initializer_list<Type> list); // Copy Assignment

	~BvVector();

	// Allocator
	MemoryArenaType* GetAllocator() const;
	void SetAllocator(MemoryArenaType* pAllocator);

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
	void PushBack(const Type & value);
	void PushBack(Type && value);
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
	void EraseAndSwapWithLast(size_t index);
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
	MemoryArenaType* m_pAllocator = nullptr;
	size_t m_Size = 0;
	size_t m_Capacity = 0;
};


template<typename Type, typename MemoryArenaType>
inline BvVector<Type, MemoryArenaType>::BvVector()
{
}


template<typename Type, typename MemoryArenaType>
inline BvVector<Type, MemoryArenaType>::BvVector(MemoryArenaType* pAllocator)
	: m_pAllocator(pAllocator)
{
}


template<typename Type, typename MemoryArenaType>
inline BvVector<Type, MemoryArenaType>::BvVector(const size_t size, const Type & val, MemoryArenaType* pAllocator)
	: m_pAllocator(pAllocator)
{
	Assign(size, val);
}


template<typename Type, typename MemoryArenaType>
inline BvVector<Type, MemoryArenaType>::BvVector(Iterator start, Iterator end, MemoryArenaType* pAllocator)
	: m_pAllocator(pAllocator)
{
	Assign(start, end);
}


template<typename Type, typename MemoryArenaType>
inline BvVector<Type, MemoryArenaType>::BvVector(std::initializer_list<Type> list, MemoryArenaType* pAllocator)
	: m_pAllocator(pAllocator)
{
	Assign(list);
}


template<typename Type, typename MemoryArenaType>
inline BvVector<Type, MemoryArenaType>::BvVector(const BvVector & rhs)
	: m_pAllocator(rhs.m_pAllocator)
{
	Grow(rhs.m_Size);

	m_Size = rhs.m_Size;
	for (auto i = 0; i < m_Size; i++)
	{
		new (&m_pData[i]) Type(rhs.m_pData[i]);
	}
}


template<typename Type, typename MemoryArenaType>
inline BvVector<Type, MemoryArenaType>::BvVector(BvVector && rhs) noexcept
{
	*this = std::move(rhs);
}


template<typename Type, typename MemoryArenaType>
inline BvVector<Type, MemoryArenaType>& BvVector<Type, MemoryArenaType>::operator=(const BvVector & rhs)
{
	if (this != &rhs)
	{
		Destroy();
		SetAllocator(rhs.m_pAllocator);
		Grow(rhs.m_Size);

		m_Size = rhs.m_Size;
		for (auto i = 0; i < m_Size; i++)
		{
			new (&m_pData[i]) Type(rhs.m_pData[i]);
		}
	}

	return *this;
}


template<typename Type, typename MemoryArenaType>
inline BvVector<Type, MemoryArenaType>& BvVector<Type, MemoryArenaType>::operator=(BvVector && rhs) noexcept
{
	if (this != &rhs)
	{
		std::swap(m_pData, rhs.m_pData);
		std::swap(m_pAllocator, rhs.m_pAllocator);
		std::swap(m_Size, rhs.m_Size);
		std::swap(m_Capacity, rhs.m_Capacity);
	}

	return *this;
}


template<typename Type, typename MemoryArenaType>
inline BvVector<Type, MemoryArenaType>& BvVector<Type, MemoryArenaType>::operator=(std::initializer_list<Type> list)
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


template<typename Type, typename MemoryArenaType>
inline BvVector<Type, MemoryArenaType>::~BvVector()
{
	Destroy();
}


template<typename Type, typename MemoryArenaType>
inline MemoryArenaType* BvVector<Type, MemoryArenaType>::GetAllocator() const
{
	return m_pAllocator;
}


template<typename Type, typename MemoryArenaType>
inline void BvVector<Type, MemoryArenaType>::SetAllocator(MemoryArenaType* pAllocator)
{
	if (m_pAllocator == pAllocator)
	{
		return;
	}

	if (m_Capacity > 0)
	{
		Type* pNewData = pAllocator ? BvMNewN(*pAllocator, Type, m_Capacity) : BvNewN(Type, m_Capacity);
		for (auto i = 0u; i < m_Size; i++)
		{
			new (&pNewData[i]) Type(std::move(m_pData[i]));
		}

		Clear();
		m_pAllocator ? BvMDeleteN(*m_pAllocator, m_pData) : BvDeleteN(m_pData);
		m_pData = pNewData;
	}

	m_pAllocator = pAllocator;
}


template<typename Type, typename MemoryArenaType>
inline const size_t BvVector<Type, MemoryArenaType>::Size() const
{
	return m_Size;
}

template<typename Type, typename MemoryArenaType>
inline const size_t BvVector<Type, MemoryArenaType>::Capacity() const
{
	return m_Capacity;
}

template<typename Type, typename MemoryArenaType>
inline const bool BvVector<Type, MemoryArenaType>::Empty() const
{
	return m_Size == 0;
}

template<typename Type, typename MemoryArenaType>
inline void BvVector<Type, MemoryArenaType>::Resize(const size_t size, const Type & value)
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

template<typename Type, typename MemoryArenaType>
inline void BvVector<Type, MemoryArenaType>::Reserve(const size_t size)
{
	Grow(size);
}

template<typename Type, typename MemoryArenaType>
inline Type & BvVector<Type, MemoryArenaType>::operator[](const size_t index)
{
	BvAssert(m_Size > 0 && index < m_Size, "Index out of bounds");
	return m_pData[index];
}

template<typename Type, typename MemoryArenaType>
inline const Type & BvVector<Type, MemoryArenaType>::operator[](const size_t index) const
{
	BvAssert(m_Size > 0 && index < m_Size, "Index out of bounds");
	return m_pData[index];
}

template<typename Type, typename MemoryArenaType>
inline Type & BvVector<Type, MemoryArenaType>::At(const size_t index)
{
	BvAssert(m_Size > 0 && index < m_Size, "Index out of bounds");
	return m_pData[index];
}

template<typename Type, typename MemoryArenaType>
inline const Type & BvVector<Type, MemoryArenaType>::At(const size_t index) const
{
	BvAssert(m_Size > 0 && index < m_Size, "Index out of bounds");
	return m_pData[index];
}

template<typename Type, typename MemoryArenaType>
inline Type & BvVector<Type, MemoryArenaType>::Front()
{
	BvAssert(m_Size > 0, "Vector is empty");
	return m_pData[0];
}

template<typename Type, typename MemoryArenaType>
inline const Type & BvVector<Type, MemoryArenaType>::Front() const
{
	BvAssert(m_Size > 0, "Vector is empty");
	return m_pData[0];
}

template<typename Type, typename MemoryArenaType>
inline Type & BvVector<Type, MemoryArenaType>::Back()
{
	BvAssert(m_Size > 0, "Vector is empty");
	return m_pData[m_Size - 1];
}

template<typename Type, typename MemoryArenaType>
inline const Type & BvVector<Type, MemoryArenaType>::Back() const
{
	BvAssert(m_Size > 0, "Vector is empty");
	return m_pData[m_Size - 1];
}

template<typename Type, typename MemoryArenaType>
inline Type * BvVector<Type, MemoryArenaType>::Data()
{
	return m_pData;
}

template<typename Type, typename MemoryArenaType>
inline const Type * const BvVector<Type, MemoryArenaType>::Data() const
{
	return m_pData;
}

template<typename Type, typename MemoryArenaType>
inline void BvVector<Type, MemoryArenaType>::Assign(Iterator start, Iterator end)
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

template<typename Type, typename MemoryArenaType>
inline void BvVector<Type, MemoryArenaType>::Assign(const size_t size, const Type & val)
{
	Clear();

	Grow(size);

	m_Size = size;
	for (u32 i = 0; i < m_Size; i++)
	{
		new (&m_pData[i]) Type(val);
	}
}

template<typename Type, typename MemoryArenaType>
inline void BvVector<Type, MemoryArenaType>::Assign(std::initializer_list<Type> list)
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

template<typename Type, typename MemoryArenaType>
inline void BvVector<Type, MemoryArenaType>::PushBack(const Type & value)
{
	EmplaceBack(value);
}

template<typename Type, typename MemoryArenaType>
inline void BvVector<Type, MemoryArenaType>::PushBack(Type && value)
{
	EmplaceBack(std::move(value));
}

template<typename Type, typename MemoryArenaType>
inline void BvVector<Type, MemoryArenaType>::PopBack()
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


template<typename Type, typename MemoryArenaType>
inline void BvVector<Type, MemoryArenaType>::PopFront()
{
	Erase(cbegin());
}


template<typename Type, typename MemoryArenaType>
inline typename BvVector<Type, MemoryArenaType>::Iterator BvVector<Type, MemoryArenaType>::Insert(ConstIterator position, const Type & value)
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

template<typename Type, typename MemoryArenaType>
inline typename BvVector<Type, MemoryArenaType>::Iterator BvVector<Type, MemoryArenaType>::Insert(ConstIterator position, const size_t count, const Type & value)
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

template<typename Type, typename MemoryArenaType>
inline typename BvVector<Type, MemoryArenaType>::Iterator BvVector<Type, MemoryArenaType>::Insert(ConstIterator position, Iterator first, Iterator last)
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

template<typename Type, typename MemoryArenaType>
inline typename BvVector<Type, MemoryArenaType>::Iterator BvVector<Type, MemoryArenaType>::Insert(ConstIterator position, Type && value)
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

template<typename Type, typename MemoryArenaType>
inline typename BvVector<Type, MemoryArenaType>::Iterator BvVector<Type, MemoryArenaType>::Insert(ConstIterator position, std::initializer_list<Type> list)
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


template<typename Type, typename MemoryArenaType>
inline typename BvVector<Type, MemoryArenaType>::Iterator BvVector<Type, MemoryArenaType>::Erase(size_t index)
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

template<typename Type, typename MemoryArenaType>
inline typename BvVector<Type, MemoryArenaType>::Iterator BvVector<Type, MemoryArenaType>::Erase(size_t startIndex, size_t count)
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


template<typename Type, typename MemoryArenaType>
inline typename BvVector<Type, MemoryArenaType>::Iterator BvVector<Type, MemoryArenaType>::Erase(ConstIterator position)
{
	auto index = position - ConstIterator(m_pData);
	return Erase(index);
}

template<typename Type, typename MemoryArenaType>
inline typename BvVector<Type, MemoryArenaType>::Iterator BvVector<Type, MemoryArenaType>::Erase(ConstIterator first, ConstIterator last)
{
	auto index = first - ConstIterator(m_pData);
	auto count = last - first;

	return Erase(index, count);
}


template<typename Type, typename MemoryArenaType>
inline void BvVector<Type, MemoryArenaType>::EraseAndSwapWithLast(size_t index)
{
	BvAssert(m_Size > 0 && index < m_Size, "Index out of bounds");
	if (index < m_Size - 1)
	{
		std::swap(m_pData[index], m_pData[m_Size - 1]);
	}
	PopBack();
}

template<typename Type, typename MemoryArenaType>
inline void BvVector<Type, MemoryArenaType>::Clear()
{
	if constexpr (!std::is_trivially_destructible_v<Type>)
	{
		for (size_t i = m_Size; i > 0; i--)
		{
			m_pData[i - 1].~Type();
		}
	}

	m_Size = 0;
}

template<typename Type, typename MemoryArenaType>
template<class ...Args>
inline typename BvVector<Type, MemoryArenaType>::Iterator BvVector<Type, MemoryArenaType>::Emplace(ConstIterator position, Args&& ...args)
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

template<typename Type, typename MemoryArenaType>
template<class ...Args>
inline Type& BvVector<Type, MemoryArenaType>::EmplaceBack(Args&& ...args)
{
	if (m_Size == m_Capacity)
	{
		Grow(m_Capacity + (m_Capacity >> 1) + (m_Capacity <= 2));
	}

	new (&m_pData[m_Size]) Type(std::forward<Args>(args)...);

	return m_pData[m_Size++];
}

template<typename Type, typename MemoryArenaType>
size_t BvVector<Type, MemoryArenaType>::Find(const Type& value) const
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

template<typename Type, typename MemoryArenaType>
inline bool BvVector<Type, MemoryArenaType>::Contains(const Type& value) const
{
	return Find(value) != kU64Max;
}

template<typename Type, typename MemoryArenaType>
inline void BvVector<Type, MemoryArenaType>::Grow(const size_t size)
{
	if (size <= m_Capacity)
	{
		return;
	}

	Type* pNewData = m_pAllocator ? BvMNewN(*m_pAllocator, Type, size) : BvNewN(Type, size);
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
		m_pAllocator ? BvMDeleteN(*m_pAllocator, m_pData) : BvDeleteN(m_pData);
	}
	m_pData = pNewData;
}

template<typename Type, typename MemoryArenaType>
inline void BvVector<Type, MemoryArenaType>::Destroy()
{
	Clear();

	if (m_pData)
	{
		m_pAllocator ? BvMDeleteN(*m_pAllocator, m_pData) : BvDeleteN(m_pData);
		m_pData = nullptr;
	}

	m_Capacity = 0;
}