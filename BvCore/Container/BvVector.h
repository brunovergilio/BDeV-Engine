#pragma once


// BvVector
// Container class, with variable-sized memory. Once memory has been allocated for a specific instance, the specified amount becomes
// the maximum size for that instance. Requested sizes are rounded to the next power of 2 (if they're not one already).
// The class can reserve and/or resize its underlying container


#include "BvCore/BvPlatform.h"
#include "BvCore/Container/BvIterator.h"
#include "BvCore/BvDebug.h"
#include "BvCore/System/Memory/BvMemory.h"


template<class Type>
class BvVector
{
public:
	using Iterator = RandomIterator<Type>;
	using ConstIterator = RandomIterator<const Type>;
	using ReverseIterator = RandomReverseIterator<Type>;
	using ConstReverseIterator = RandomReverseIterator<const Type>;

	BvVector(); // Default
	explicit BvVector(const size_t size, const Type & val = Type()); // Fill
	explicit BvVector(Iterator start, Iterator end); // Range
	BvVector(const BvVector & rhs); // Copy
	BvVector(BvVector && rhs) noexcept; // Move
	BvVector(std::initializer_list<Type> list); // Initializer List

	BvVector<Type> & operator =(const BvVector & rhs); // Copy Assignment
	BvVector<Type> & operator =(BvVector && rhs) noexcept; // Move Assignment
	BvVector<Type> & operator =(std::initializer_list<Type> list); // Copy Assignment

	~BvVector();

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
	Iterator Insert(ConstIterator position, const Type & value);
	Iterator Insert(ConstIterator position, const size_t count, const Type & value);
	Iterator Insert(ConstIterator position, Iterator first, Iterator last);
	Iterator Insert(ConstIterator position, Type && value);
	Iterator Insert(ConstIterator position, std::initializer_list<Type> list);
	Iterator Erase(ConstIterator position);
	Iterator Erase(ConstIterator first, ConstIterator last);
	void Clear();
	template <class... Args>
	Iterator Emplace(ConstIterator position, Args&&... args);
	template <class... Args>
	void EmplaceBack(Args&&... args);

private:
	void Grow(const size_t size);
	void Destroy();

private:
	Type * m_pData = nullptr;
	size_t m_Size = 0;
	size_t m_Capacity = 0;
};

template<class Type>
inline BvVector<Type>::BvVector()
{
}


template<class Type>
inline BvVector<Type>::BvVector(const size_t size, const Type & val)
{
	Assign(size, val);
}

template<class Type>
inline BvVector<Type>::BvVector(Iterator start, Iterator end)
{
	Assign(start, end);
}

template<class Type>
inline BvVector<Type>::BvVector(const BvVector & rhs)
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
inline BvVector<Type>::BvVector(std::initializer_list<Type> list)
{
	Assign(list);
}

template<class Type>
inline BvVector<Type>& BvVector<Type>::operator=(const BvVector & rhs)
{
	if (this != &rhs)
	{
		Clear();

		if (rhs.m_Size > m_Capacity)
		{
			Grow(rhs.m_Size);
		}

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
		Clear();

		m_pData = rhs.m_pData;
		m_Size = rhs.m_Size;
		m_Capacity = rhs.m_Capacity;

		rhs.m_pData = nullptr;
		rhs.m_Size = 0;
		rhs.m_Capacity = 0;
	}

	return *this;
}

template<class Type>
inline BvVector<Type>& BvVector<Type>::operator=(std::initializer_list<Type> list)
{
	Clear();

	auto size = list.size();
	if (size > m_Capacity)
	{
		Grow(size);
	}

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
		auto prevSize = m_Size;

		if (size > m_Capacity)
		{
			Grow(size);
		}

		m_Size = size;
		for (auto i = prevSize; i < m_Size; i++)
		{
			new (&m_pData[i]) Type(value);
		}
	}
	else
	{
		for (auto i = m_Size; i > size; i--)
		{
			m_pData[i - 1].~Type();
		}

		m_Size = size;
	}
}

template<class Type>
inline void BvVector<Type>::Reserve(const size_t size)
{
	if (size > m_Size && size > m_Capacity)
	{
		Grow(size);
	}
}

template<class Type>
inline Type & BvVector<Type>::operator[](const size_t index)
{
	BvAssert(m_Size > 0 && index < m_Size);
	return m_pData[index];
}

template<class Type>
inline const Type & BvVector<Type>::operator[](const size_t index) const
{
	BvAssert(m_Size > 0 && index < m_Size);
	return m_pData[index];
}

template<class Type>
inline Type & BvVector<Type>::At(const size_t index)
{
	BvAssert(m_Size > 0 && index < m_Size);
	return m_pData[index];
}

template<class Type>
inline const Type & BvVector<Type>::At(const size_t index) const
{
	BvAssert(m_Size > 0 && index < m_Size);
	return m_pData[index];
}

template<class Type>
inline Type & BvVector<Type>::Front()
{
	BvAssert(m_Size > 0);
	return m_pData[0];
}

template<class Type>
inline const Type & BvVector<Type>::Front() const
{
	BvAssert(m_Size > 0);
	return m_pData[0];
}

template<class Type>
inline Type & BvVector<Type>::Back()
{
	BvAssert(m_Size > 0);
	return m_pData[m_Size - 1];
}

template<class Type>
inline const Type & BvVector<Type>::Back() const
{
	BvAssert(m_Size > 0);
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
	if (m_Size == m_Capacity)
	{
		Grow(m_Capacity + 1);
	}

	new (&m_pData[m_Size++]) Type(value);
}

template<class Type>
inline void BvVector<Type>::PushBack(Type && value)
{
	if (m_Size == m_Capacity)
	{
		Grow(m_Capacity + 1);
	}

	new (&m_pData[m_Size++]) Type(std::move(value));
}

template<class Type>
inline void BvVector<Type>::PopBack()
{
	if (m_Size > 0)
	{
		m_pData[--m_Size].~Type();
	}
}

template<class Type>
inline typename BvVector<Type>::Iterator BvVector<Type>::Insert(ConstIterator position, const Type & value)
{
	auto pos = position - ConstIterator(m_pData);
	if (pos > m_Size)
	{
		return Iterator(m_pData + m_Size);
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
		return Iterator(m_pData + m_Size);
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
		return Iterator(m_pData + m_Size);
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
		return Iterator(m_pData + m_Size);
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
		return Iterator(m_pData + m_Size);
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
inline typename BvVector<Type>::Iterator BvVector<Type>::Erase(ConstIterator position)
{
	auto pos = position - ConstIterator(m_pData);
	if (pos >= m_Size)
	{
		return Iterator(m_pData + m_Size);
	}

	m_pData[pos].~Type();

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

	m_pData[pos].~Type();

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
inline void BvVector<Type>::Clear()
{
	for (size_t i = m_Size; i > 0; i--)
	{
		m_pData[i - 1].~Type();
	}

	m_Size = 0;
}

template<class Type>
template<class ...Args>
inline typename BvVector<Type>::Iterator BvVector<Type>::Emplace(ConstIterator position, Args && ...args)
{
	auto pos = position - ConstIterator(m_pData);
	if (pos > m_Size)
	{
		return Iterator(m_pData + m_Size);
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

	new (&m_pData[pos]) Type(std::forward<Args>(args)...);
	m_Size++;

	return Iterator(m_pData + pos);
}

template<class Type>
template<class ...Args>
inline void BvVector<Type>::EmplaceBack(Args && ...args)
{
	if (m_Size == m_Capacity)
	{
		Grow(m_Capacity + 1);
	}

	new (&m_pData[m_Size++]) Type(std::forward<Args>(args)...);
}

template<class Type>
inline void BvVector<Type>::Grow(const size_t size)
{
	if (size == 0)
	{
		return;
	}
	else if (size == 1)
	{
		m_Capacity = size << 1;
	}
	else
	{
		m_Capacity = size + (size >> 1);
	}

	void * pNewData = BvAlloc(sizeof(Type) * m_Capacity, alignof(Type));
	if (m_Size > 0)
	{
		Type * pObjs = reinterpret_cast<Type *>(pNewData);
		for (auto i = 0U; i < m_Size; i++)
		{
			new (&pObjs[i]) Type(std::move(m_pData[i]));
		}
	}
	BvFree(m_pData);

	m_pData = reinterpret_cast<Type *>(pNewData);
}

template<class Type>
inline void BvVector<Type>::Destroy()
{
	Clear();

	m_Capacity = 0;

	if (m_pData)
	{
		BvFree(m_pData);
		m_pData = nullptr;
	}
}