#pragma once


// BvVector
// Container class, with variable-sized memory. Once memory has been allocated for a specific instance, the specified amount becomes
// the maximum size for that instance. If the size needs to be increased, memory is reallocated again.
// The class can reserve and/or resize its underlying container


#include "BDeV/BvCore.h"
#include "BDeV/Container/BvIterator.h"
#include "BDeV/System/Debug/BvDebug.h"
#include "BDeV/System/Memory/BvMemoryCommon.h"


template<class Type>
class BvVector
{
public:
	using Iterator = RandomIterator<Type>;
	using ConstIterator = RandomIterator<const Type>;
	using ReverseIterator = RandomReverseIterator<Type>;
	using ConstReverseIterator = RandomReverseIterator<const Type>;

	BvVector(); // Default
	BvVector(IBvMemoryAllocator* pAllocator); // Allocator
	explicit BvVector(const size_t size, const Type & val = Type(), IBvMemoryAllocator* pAllocator = nullptr); // Fill
	explicit BvVector(Iterator start, Iterator end, IBvMemoryAllocator* pAllocator = nullptr); // Range
	BvVector(std::initializer_list<Type> list, IBvMemoryAllocator* pAllocator = nullptr); // Initializer List
	BvVector(const BvVector & rhs); // Copy
	BvVector(BvVector && rhs) noexcept; // Move

	BvVector& operator =(const BvVector & rhs); // Copy Assignment
	BvVector& operator =(BvVector && rhs) noexcept; // Move Assignment
	BvVector& operator =(std::initializer_list<Type> list); // Copy Assignment

	~BvVector();

	// Allocator
	IBvMemoryAllocator* GetAllocator() const;
	void SetAllocator(IBvMemoryAllocator* pAllocator);

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
	Iterator Erase(ConstIterator position);
	Iterator Erase(ConstIterator first, ConstIterator last);
	void RemoveIndex(size_t index);
	void EraseAndSwapWithLast(size_t index);
	void Clear();
	template <class... Args>
	Iterator Emplace(ConstIterator position, Args&&... args);
	template <class... Args>
	Type& EmplaceBack(Args&&... args);
	size_t Find(const Type& value) const;

private:
	void Grow(const size_t size);
	void Destroy();

private:
	Type* m_pData = nullptr;
	IBvMemoryAllocator* m_pAllocator = nullptr;
	size_t m_Size = 0;
	size_t m_Capacity = 0;
};


template<class Type>
inline BvVector<Type>::BvVector()
{
}


template<class Type>
inline BvVector<Type>::BvVector(IBvMemoryAllocator* pAllocator)
	: m_pAllocator(pAllocator)
{
}


template<class Type>
inline BvVector<Type>::BvVector(const size_t size, const Type & val, IBvMemoryAllocator* pAllocator)
	: m_pAllocator(pAllocator)
{
	Assign(size, val);
}


template<class Type>
inline BvVector<Type>::BvVector(Iterator start, Iterator end, IBvMemoryAllocator* pAllocator)
	: m_pAllocator(pAllocator)
{
	Assign(start, end);
}


template<class Type>
inline BvVector<Type>::BvVector(std::initializer_list<Type> list, IBvMemoryAllocator* pAllocator)
	: m_pAllocator(pAllocator)
{
	Assign(list);
}


template<class Type>
inline BvVector<Type>::BvVector(const BvVector & rhs)
	: m_pAllocator(rhs.m_pAllocator)
{
	Grow(rhs.m_Size);

	m_Size = rhs.m_Size;
	for (auto i = 0; i < m_Size; i++)
	{
		new (&m_pData[i]) Type(rhs.m_pData[i]);
	}
}


template<class Type>
inline BvVector<Type>::BvVector(BvVector && rhs) noexcept
{
	*this = std::move(rhs);
}


template<class Type>
inline BvVector<Type>& BvVector<Type>::operator=(const BvVector & rhs)
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


template<class Type>
inline BvVector<Type>& BvVector<Type>::operator=(BvVector && rhs) noexcept
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


template<class Type>
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


template<class Type>
inline BvVector<Type>::~BvVector()
{
	Destroy();
}


template<class Type>
inline IBvMemoryAllocator* BvVector<Type>::GetAllocator() const
{
	return m_pAllocator;
}


template<class Type>
inline void BvVector<Type>::SetAllocator(IBvMemoryAllocator* pAllocator)
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


template<class Type>
inline const size_t BvVector<Type>::Size() const
{
	return m_Size;
}

template<class Type>
inline const size_t BvVector<Type>::Capacity() const
{
	return m_Capacity;
}

template<class Type>
inline const bool BvVector<Type>::Empty() const
{
	return m_Size == 0;
}

template<class Type>
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

template<class Type>
inline void BvVector<Type>::Reserve(const size_t size)
{
	Grow(size);
}

template<class Type>
inline Type & BvVector<Type>::operator[](const size_t index)
{
	BvAssert(m_Size > 0 && index < m_Size, "Index out of bounds");
	return m_pData[index];
}

template<class Type>
inline const Type & BvVector<Type>::operator[](const size_t index) const
{
	BvAssert(m_Size > 0 && index < m_Size, "Index out of bounds");
	return m_pData[index];
}

template<class Type>
inline Type & BvVector<Type>::At(const size_t index)
{
	BvAssert(m_Size > 0 && index < m_Size, "Index out of bounds");
	return m_pData[index];
}

template<class Type>
inline const Type & BvVector<Type>::At(const size_t index) const
{
	BvAssert(m_Size > 0 && index < m_Size, "Index out of bounds");
	return m_pData[index];
}

template<class Type>
inline Type & BvVector<Type>::Front()
{
	BvAssert(m_Size > 0, "Vector is empty");
	return m_pData[0];
}

template<class Type>
inline const Type & BvVector<Type>::Front() const
{
	BvAssert(m_Size > 0, "Vector is empty");
	return m_pData[0];
}

template<class Type>
inline Type & BvVector<Type>::Back()
{
	BvAssert(m_Size > 0, "Vector is empty");
	return m_pData[m_Size - 1];
}

template<class Type>
inline const Type & BvVector<Type>::Back() const
{
	BvAssert(m_Size > 0, "Vector is empty");
	return m_pData[m_Size - 1];
}

template<class Type>
inline Type * BvVector<Type>::Data()
{
	return m_pData;
}

template<class Type>
inline const Type * const BvVector<Type>::Data() const
{
	return m_pData;
}

template<class Type>
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

template<class Type>
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

template<class Type>
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

template<class Type>
inline void BvVector<Type>::PushBack(const Type & value)
{
	EmplaceBack(value);
}

template<class Type>
inline void BvVector<Type>::PushBack(Type && value)
{
	EmplaceBack(std::move(value));
}

template<class Type>
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


template<class Type>
inline void BvVector<Type>::PopFront()
{
	Erase(cbegin());
}


template<class Type>
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

template<class Type>
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

template<class Type>
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

template<class Type>
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

template<class Type>
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


template<class Type>
inline typename BvVector<Type>::Iterator BvVector<Type>::Erase(size_t index)
{
	return Erase(cbegin() + index);
}


template<class Type>
inline typename BvVector<Type>::Iterator BvVector<Type>::Erase(ConstIterator position)
{
	auto pos = position - ConstIterator(m_pData);
	if (pos >= m_Size)
	{
		return Iterator(m_pData + m_Size);
	}

	if constexpr (!std::is_trivially_destructible_v<Type>)
	{
		m_pData[pos].~Type();
	}

	if (pos < m_Size - 1)
	{
		for (auto i = pos; i < m_Size - 1; i++)
		{
			m_pData[i] = std::move(m_pData[i + 1]);
		}
	}

	m_Size--;

	return Iterator(m_pData + pos);
}

template<class Type>
inline typename BvVector<Type>::Iterator BvVector<Type>::Erase(ConstIterator first, ConstIterator last)
{
	auto pos = first - ConstIterator(m_pData);
	auto count = last - first;
	if (pos >= m_Size)
	{
		return Iterator(m_pData + m_Size);
	}

	if constexpr (!std::is_trivially_destructible_v<Type>)
	{
		for (auto i = pos; i < pos + count; i++)
		{
			m_pData[pos].~Type();
		}
	}

	if (pos < m_Size - 1)
	{
		for (auto i = pos + count; i < m_Size; i++)
		{
			m_pData[i - count] = std::move(m_pData[i]);
		}
	}

	m_Size -= count;

	return Iterator(m_pData + pos);
}

template<class Type>
inline void BvVector<Type>::RemoveIndex(size_t index)
{
	BvAssert(m_Size > 0 && index < m_Size, "Index out of bounds");
	Erase(cbegin() + index);
}

template<class Type>
inline void BvVector<Type>::EraseAndSwapWithLast(size_t index)
{
	BvAssert(m_Size > 0 && index < m_Size, "Index out of bounds");
	if (index < m_Size - 1)
	{
		std::swap(m_pData[index], m_pData[m_Size - 1]);
	}
	PopBack();
}

template<class Type>
inline void BvVector<Type>::Clear()
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

template<class Type>
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

template<class Type>
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

template<class Type>
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

template<class Type>
inline void BvVector<Type>::Grow(const size_t size)
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

template<class Type>
inline void BvVector<Type>::Destroy()
{
	Clear();

	if (m_pData)
	{
		m_pAllocator ? BvMDeleteN(*m_pAllocator, m_pData) : BvDeleteN(m_pData);
		m_pData = nullptr;
	}

	m_Capacity = 0;
}