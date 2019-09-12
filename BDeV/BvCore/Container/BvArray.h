#pragma once


// BvArray
// Container class, with fixed-sized stack memory.


#include "BvCore/BvDefines.h"
#include "BvCore/Container/BvIterator.h"
#include <utility>


template<class Type, size_t N>
class BvArray
{
public:
	using Iterator = RandomIterator<Type>;
	using ConstIterator = RandomIterator<const Type>;
	using ReverseIterator = RandomReverseIterator<Type>;
	using ConstReverseIterator = RandomReverseIterator<const Type>;

	BvArray(); // Default
	explicit BvArray(const size_t size, const Type & val); // Fill
	explicit BvArray(Iterator start, Iterator end); // Range
	template<size_t X>
	BvArray(const BvArray<Type, X> & rhs); // Copy
	template<size_t X>
	BvArray(BvArray<Type, X> && rhs); // Move
	BvArray(std::initializer_list<Type> list); // Initializer List

	template<size_t X>
	BvArray<Type, N> & operator =(const BvArray<Type, X> & rhs); // Copy Assignment
	template<size_t X>
	BvArray<Type, N> & operator =(BvArray<Type, X> && rhs); // Move Assignment
	BvArray<Type, N> & operator =(std::initializer_list<Type> list); // Copy Assignment

	~BvArray();

	// Iterator (BROKEN - HAVE TO FIX THESE)
	Iterator begin() const { return Iterator(m_pData); }
	Iterator end() const { return Iterator(m_pData + N); }
	ConstIterator cbegin() const { return ConstIterator(m_pData); }
	ConstIterator cend() const { return ConstIterator(m_pData + N); }
	ReverseIterator rbegin() const { return ReverseIterator(m_pData); }
	ReverseIterator rend() const { return ReverseIterator(m_pData + N); }
	ConstReverseIterator crbegin() const { return ConstReverseIterator(m_pData); }
	ConstReverseIterator crend() const { return ConstReverseIterator(m_pData + N); }

	// Capacity
	const size_t Size() const;
	const size_t Capacity() const;
	const bool Empty() const;

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
	Type m_pData[N]{};
	size_t m_Size = 0;
};

template<class Type, size_t N>
inline BvArray<Type, N>::BvArray()
{
}


template<class Type, size_t N>
inline BvArray<Type, N>::BvArray(const size_t size, const Type & val)
{
	Assign(size, val);
}

template<class Type, size_t N>
inline BvArray<Type, N>::BvArray(Iterator start, Iterator end)
{
	Assign(start, end);
}

template<class Type, size_t N>
template<size_t X>
inline BvArray<Type, N>::BvArray(const BvArray<Type, X> & rhs)
{
	m_Size = N < rhs.Size() ? N : rhs.Size();
	for (auto i = 0; i < m_Size; i++)
	{
		m_pData[i] = rhs[i];
	}
}

template<class Type, size_t N>
template<size_t X>
inline BvArray<Type, N>::BvArray(BvArray<Type, X> && rhs)
{
	*this = std::move(rhs);
}

template<class Type, size_t N>
inline BvArray<Type, N>::BvArray(std::initializer_list<Type> list)
{
	Assign(list);
}

template<class Type, size_t N>
template<size_t X>
inline BvArray<Type, N>& BvArray<Type, N>::operator=(const BvArray<Type, X> & rhs)
{
	if (m_pData != rhs.Data())
	{
		Clear();

		m_Size = N < rhs.Size() ? N : rhs.Size();
		for (auto i = 0; i < m_Size; i++)
		{
			m_pData[i] = rhs.m_pData[i];
		}
	}

	return *this;
}

template<class Type, size_t N>
template<size_t X>
inline BvArray<Type, N>& BvArray<Type, N>::operator=(BvArray<Type, X> && rhs)
{
	if (m_pData != rhs.Data())
	{
		auto size = N < rhs.Size() ? N : rhs.Size();
		memmove(m_pData, rhs.Data(), sizeof(Type) * size);
		m_Size = size;

		memset(&rhs, 0, sizeof(BvArray<Type, X>));
	}

	return *this;
}

template<class Type, size_t N>
inline BvArray<Type, N>& BvArray<Type, N>::operator=(std::initializer_list<Type> list)
{
	Clear();

	m_Size = N < list.size() ? N : list.size();
	for (auto i = 0; i < m_Size; i++)
	{
		m_pData[i] = list[i];
	}

	return *this;
}

template<class Type, size_t N>
inline BvArray<Type, N>::~BvArray()
{
	Clear();
}

template<class Type, size_t N>
inline const size_t BvArray<Type, N>::Size() const
{
	return m_Size;
}

template<class Type, size_t N>
inline const size_t BvArray<Type, N>::Capacity() const
{
	return N;
}

template<class Type, size_t N>
inline const bool BvArray<Type, N>::Empty() const
{
	return m_Size == 0;
}


template<class Type, size_t N>
inline Type & BvArray<Type, N>::operator[](const size_t index)
{
	BvAssert(m_Size > 0 && index < m_Size);
	return m_pData[index];
}

template<class Type, size_t N>
inline const Type & BvArray<Type, N>::operator[](const size_t index) const
{
	BvAssert(m_Size > 0 && index < m_Size);
	return m_pData[index];
}

template<class Type, size_t N>
inline Type & BvArray<Type, N>::At(const size_t index)
{
	BvAssert(m_Size > 0 && index < m_Size);
	return m_pData[index];
}

template<class Type, size_t N>
inline const Type & BvArray<Type, N>::At(const size_t index) const
{
	BvAssert(m_Size > 0 && index < m_Size);
	return m_pData[index];
}

template<class Type, size_t N>
inline Type & BvArray<Type, N>::Front()
{
	BvAssert(m_Size > 0);
	return m_pData[0];
}

template<class Type, size_t N>
inline const Type & BvArray<Type, N>::Front() const
{
	BvAssert(m_Size > 0);
	return m_pData[0];
}

template<class Type, size_t N>
inline Type & BvArray<Type, N>::Back()
{
	BvAssert(m_Size > 0);
	return m_pData[m_Size - 1];
}

template<class Type, size_t N>
inline const Type & BvArray<Type, N>::Back() const
{
	BvAssert(m_Size > 0);
	return m_pData[m_Size - 1];
}

template<class Type, size_t N>
inline Type * BvArray<Type, N>::Data()
{
	return m_pData;
}

template<class Type, size_t N>
inline const Type * const BvArray<Type, N>::Data() const
{
	return m_pData;
}

template<class Type, size_t N>
inline void BvArray<Type, N>::Assign(Iterator start, Iterator end)
{
	Clear();

	auto size = end - start;

	m_Size = N < size ? N : size;
	size_t count = 0;
	for (auto it = start; count < m_Size && it != end; it++, count++)
	{
		m_pData[m_Size++] = *it;
	}
}

template<class Type, size_t N>
inline void BvArray<Type, N>::Assign(const size_t size, const Type & val)
{
	Clear();

	m_Size = N < size ? N : size;
	for (u32 i = 0; i < m_Size; i++)
	{
		m_pData[i] = val;
	}
}

template<class Type, size_t N>
inline void BvArray<Type, N>::Assign(std::initializer_list<Type> list)
{
	Clear();

	m_Size = N < list.size() ? N : list.size();
	for (auto i = 0; i < m_Size; i++)
	{
		m_pData[i] = list[i];
	}
}

template<class Type, size_t N>
inline void BvArray<Type, N>::PushBack(const Type & value)
{
	if (m_Size == N)
	{
		return;
	}

	new (&m_pData[m_Size++]) Type(value);
}

template<class Type, size_t N>
inline void BvArray<Type, N>::PushBack(Type && value)
{
	if (m_Size == N)
	{
		return;
	}

	new (&m_pData[m_Size++]) Type(std::move(value));
}

template<class Type, size_t N>
inline void BvArray<Type, N>::PopBack()
{
	if (m_Size > 0)
	{
		m_pData[m_Size-- - 1].~Type();
	}
}

template<class Type, size_t N>
inline typename BvArray<Type, N>::Iterator BvArray<Type, N>::Insert(ConstIterator position, const Type & value)
{
	auto pos = position - ConstIterator(m_pData);
	if (pos > m_Size)
	{
		return Iterator(m_pData + N);
	}

	if (m_Size == N)
	{
		return Iterator(m_pData + N);
	}

	if (pos < m_Size)
	{
		memmove(m_pData + pos + 1, m_pData + pos, sizeof(Type) * (m_Size - pos));
	}

	new (&m_pData[pos]) Type(value);
	m_Size++;

	return Iterator(m_pData + pos);
}

template<class Type, size_t N>
inline typename BvArray<Type, N>::Iterator BvArray<Type, N>::Insert(ConstIterator position, const size_t count, const Type & value)
{
	auto pos = position - ConstIterator(m_pData);
	if (pos > m_Size)
	{
		return Iterator(m_pData + N);
	}

	if (m_Size + count > N)
	{
		return Iterator(m_pData + N);
	}

	if (pos < m_Size)
	{
		memmove(m_pData + pos + count, m_pData + pos, sizeof(Type) * (m_Size - pos));
	}

	for (auto i = 0; i < count; i++)
	{
		new (&m_pData[pos + i]) Type(value);
	}
	m_Size += count;

	return Iterator(m_pData + pos);
}

template<class Type, size_t N>
inline typename BvArray<Type, N>::Iterator BvArray<Type, N>::Insert(ConstIterator position, Iterator first, Iterator last)
{
	auto pos = position - ConstIterator(m_pData);
	if (pos > m_Size)
	{
		return Iterator(m_pData + N);
	}

	auto count = last - first;
	if (m_Size + count > N)
	{
		return Iterator(m_pData + N);
	}

	if (pos < m_Size)
	{
		memmove(m_pData + pos + count, m_pData + pos, sizeof(Type) * (m_Size - pos));
	}

	auto it = first;
	for (auto i = 0; i < count; i++)
	{
		new (&m_pData[pos + i]) Type(*(it++));
	}

	m_Size += count;

	return Iterator(m_pData + pos);
}

template<class Type, size_t N>
inline typename BvArray<Type, N>::Iterator BvArray<Type, N>::Insert(ConstIterator position, Type && value)
{
	auto pos = position - ConstIterator(m_pData);
	if (pos > m_Size)
	{
		return Iterator(m_pData + N);
	}

	if (m_Size == N)
	{
		return Iterator(m_pData + N);
	}

	if (pos < m_Size)
	{
		memmove(m_pData + pos + 1, m_pData + pos, sizeof(Type) * (m_Size - pos));
	}

	new (&m_pData[pos]) Type(std::move(value));
	m_Size++;

	return Iterator(m_pData + pos);
}

template<class Type, size_t N>
inline typename BvArray<Type, N>::Iterator BvArray<Type, N>::Insert(ConstIterator position, std::initializer_list<Type> list)
{
	auto pos = position - ConstIterator(m_pData);
	if (pos > m_Size)
	{
		return Iterator(m_pData + N);
	}

	auto count = list.size();
	if (m_Size + count > N)
	{
		return Iterator(m_pData + N);
	}

	if (pos < m_Size)
	{
		memmove(m_pData + pos + count, m_pData + pos, sizeof(Type) * (m_Size - pos));
	}

	auto it = list.begin();
	for (auto i = 0; i < count; i++)
	{
		new (&m_pData[pos + i]) Type(*(it++));
	}

	m_Size += count;

	return Iterator(m_pData + pos);
}

template<class Type, size_t N>
inline typename BvArray<Type, N>::Iterator BvArray<Type, N>::Erase(ConstIterator position)
{
	auto pos = position - ConstIterator(m_pData);
	if (pos >= m_Size)
	{
		return Iterator(m_pData + m_Size);
	}

	m_pData[pos].~Type();

	if (pos < m_Size - 1)
	{
		memmove(m_pData + pos, m_pData + pos + 1, sizeof(Type) * (m_Size - pos - 1));
	}

	m_Size--;

	return Iterator(m_pData + pos);
}

template<class Type, size_t N>
inline typename BvArray<Type, N>::Iterator BvArray<Type, N>::Erase(ConstIterator first, ConstIterator last)
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
		memmove(m_pData + pos, m_pData + pos + count, sizeof(Type) * (m_Size - pos - count));
	}

	m_Size -= count;

	return Iterator(m_pData + pos);
}

template<class Type, size_t N>
inline void BvArray<Type, N>::Clear()
{
	for (size_t i = m_Size; i > 0; i--)
	{
		m_pData[i - 1].~Type();
	}

	m_Size = 0;
}

template<class Type, size_t N>
template<class ...Args>
inline typename BvArray<Type, N>::Iterator BvArray<Type, N>::Emplace(ConstIterator position, Args && ...args)
{
	auto pos = position - ConstIterator(m_pData);
	if (pos > m_Size)
	{
		return Iterator(m_pData + N);
	}

	if (m_Size == N)
	{
		return Iterator(m_pData + N);
	}

	if (pos < m_Size)
	{
		memmove(m_pData + pos + 1, m_pData + pos, sizeof(Type) * (m_Size - pos));
	}

	new (&m_pData[pos]) Type(std::forward<Args>(args)...);
	m_Size++;

	return Iterator(m_pData + pos);
}

template<class Type, size_t N>
template<class ...Args>
inline void BvArray<Type, N>::EmplaceBack(Args && ...args)
{
	if (m_Size == N)
	{
		return;
	}

	new (&m_pData[m_Size++]) Type(std::forward<Args>(args)...);
}