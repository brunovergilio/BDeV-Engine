#pragma once


// BvFixedVector
// Container class, with fixed-sized stack memory.


#include "BvCore/BvDefines.h"
#include "BvCore/Container/BvIterator.h"
#include "BvCore/BvDebug.h"
#include <utility>


template<class Type, size_t N>
class BvFixedVector
{
public:
	using Iterator = RandomIterator<Type>;
	using ConstIterator = RandomIterator<const Type>;
	using ReverseIterator = RandomReverseIterator<Type>;
	using ConstReverseIterator = RandomReverseIterator<const Type>;

	BvFixedVector(); // Default
	explicit BvFixedVector(const size_t size, const Type & val); // Fill
	explicit BvFixedVector(Iterator start, Iterator end); // Range
	template<size_t X>
	BvFixedVector(const BvFixedVector<Type, X> & rhs); // Copy
	template<size_t X>
	BvFixedVector(BvFixedVector<Type, X> && rhs); // Move
	BvFixedVector(std::initializer_list<Type> list); // Initializer List

	template<size_t X>
	BvFixedVector<Type, N> & operator =(const BvFixedVector<Type, X> & rhs); // Copy Assignment
	template<size_t X>
	BvFixedVector<Type, N> & operator =(BvFixedVector<Type, X> && rhs); // Move Assignment
	BvFixedVector<Type, N> & operator =(std::initializer_list<Type> list); // Copy Assignment

	~BvFixedVector();

	Iterator begin() { return Iterator(m_pData); }
	Iterator end() { return Iterator(m_pData + m_Size); }
	ConstIterator cbegin() const { return ConstIterator(m_pData); }
	ConstIterator cend() const { return ConstIterator(m_pData + m_Size); }
	ReverseIterator rbegin() { return ReverseIterator(m_pData); }
	ReverseIterator rend() { return ReverseIterator(m_pData + m_Size); }
	ConstReverseIterator crbegin() const { return ConstReverseIterator(m_pData); }
	ConstReverseIterator crend() const { return ConstReverseIterator(m_pData + m_Size); }

	// Capacity
	const size_t Size() const;
	const size_t Capacity() const;
	const bool Empty() const;
	void Resize(const size_t size, const Type & value = Type());

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
inline BvFixedVector<Type, N>::BvFixedVector()
{
}


template<class Type, size_t N>
inline BvFixedVector<Type, N>::BvFixedVector(const size_t size, const Type & val)
{
	Assign(size, val);
}

template<class Type, size_t N>
inline BvFixedVector<Type, N>::BvFixedVector(Iterator start, Iterator end)
{
	Assign(start, end);
}

template<class Type, size_t N>
template<size_t X>
inline BvFixedVector<Type, N>::BvFixedVector(const BvFixedVector<Type, X> & rhs)
{
	m_Size = N < rhs.Size() ? N : rhs.Size();
	for (auto i = 0; i < m_Size; i++)
	{
		m_pData[i] = rhs[i];
	}
}

template<class Type, size_t N>
template<size_t X>
inline BvFixedVector<Type, N>::BvFixedVector(BvFixedVector<Type, X> && rhs)
{
	*this = std::move(rhs);
}

template<class Type, size_t N>
inline BvFixedVector<Type, N>::BvFixedVector(std::initializer_list<Type> list)
{
	Assign(list);
}

template<class Type, size_t N>
template<size_t X>
inline BvFixedVector<Type, N>& BvFixedVector<Type, N>::operator=(const BvFixedVector<Type, X> & rhs)
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
inline BvFixedVector<Type, N>& BvFixedVector<Type, N>::operator=(BvFixedVector<Type, X> && rhs)
{
	if (m_pData != rhs.Data())
	{
		auto size = N < rhs.Size() ? N : rhs.Size();
		memmove(m_pData, rhs.Data(), sizeof(Type) * size);
		m_Size = size;

		memset(&rhs, 0, sizeof(BvFixedVector<Type, X>));
	}

	return *this;
}

template<class Type, size_t N>
inline BvFixedVector<Type, N>& BvFixedVector<Type, N>::operator=(std::initializer_list<Type> list)
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
inline BvFixedVector<Type, N>::~BvFixedVector()
{
	Clear();
}

template<class Type, size_t N>
inline const size_t BvFixedVector<Type, N>::Size() const
{
	return m_Size;
}

template<class Type, size_t N>
inline const size_t BvFixedVector<Type, N>::Capacity() const
{
	return N;
}

template<class Type, size_t N>
inline const bool BvFixedVector<Type, N>::Empty() const
{
	return m_Size == 0;
}


template<class Type, size_t N>
inline void BvFixedVector<Type, N>::Resize(const size_t size, const Type & value)
{
	if (size > N)
	{
		return;
	}

	if (size > m_Size)
	{
		auto prevSize = m_Size;

		m_Size = size;
		for (auto i = prevSize; i < m_Size; i++)
		{
			m_pData[i] = value;
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


template<class Type, size_t N>
inline Type & BvFixedVector<Type, N>::operator[](const size_t index)
{
	BvAssert(m_Size > 0 && index < m_Size);
	return m_pData[index];
}

template<class Type, size_t N>
inline const Type & BvFixedVector<Type, N>::operator[](const size_t index) const
{
	BvAssert(m_Size > 0 && index < m_Size);
	return m_pData[index];
}

template<class Type, size_t N>
inline Type & BvFixedVector<Type, N>::At(const size_t index)
{
	BvAssert(m_Size > 0 && index < m_Size);
	return m_pData[index];
}

template<class Type, size_t N>
inline const Type & BvFixedVector<Type, N>::At(const size_t index) const
{
	BvAssert(m_Size > 0 && index < m_Size);
	return m_pData[index];
}

template<class Type, size_t N>
inline Type & BvFixedVector<Type, N>::Front()
{
	BvAssert(m_Size > 0);
	return m_pData[0];
}

template<class Type, size_t N>
inline const Type & BvFixedVector<Type, N>::Front() const
{
	BvAssert(m_Size > 0);
	return m_pData[0];
}

template<class Type, size_t N>
inline Type & BvFixedVector<Type, N>::Back()
{
	BvAssert(m_Size > 0);
	return m_pData[m_Size - 1];
}

template<class Type, size_t N>
inline const Type & BvFixedVector<Type, N>::Back() const
{
	BvAssert(m_Size > 0);
	return m_pData[m_Size - 1];
}

template<class Type, size_t N>
inline Type * BvFixedVector<Type, N>::Data()
{
	return m_pData;
}

template<class Type, size_t N>
inline const Type * const BvFixedVector<Type, N>::Data() const
{
	return m_pData;
}

template<class Type, size_t N>
inline void BvFixedVector<Type, N>::Assign(Iterator start, Iterator end)
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
inline void BvFixedVector<Type, N>::Assign(const size_t size, const Type & val)
{
	Clear();

	m_Size = N < size ? N : size;
	for (u32 i = 0; i < m_Size; i++)
	{
		m_pData[i] = val;
	}
}

template<class Type, size_t N>
inline void BvFixedVector<Type, N>::Assign(std::initializer_list<Type> list)
{
	Clear();

	m_Size = N < list.size() ? N : list.size();
	for (auto i = 0; i < m_Size; i++)
	{
		m_pData[i] = list[i];
	}
}

template<class Type, size_t N>
inline void BvFixedVector<Type, N>::PushBack(const Type & value)
{
	if (m_Size == N)
	{
		return;
	}

	new (&m_pData[m_Size++]) Type(value);
}

template<class Type, size_t N>
inline void BvFixedVector<Type, N>::PushBack(Type && value)
{
	if (m_Size == N)
	{
		return;
	}

	new (&m_pData[m_Size++]) Type(std::move(value));
}

template<class Type, size_t N>
inline void BvFixedVector<Type, N>::PopBack()
{
	if (m_Size > 0)
	{
		m_pData[m_Size-- - 1].~Type();
	}
}

template<class Type, size_t N>
inline typename BvFixedVector<Type, N>::Iterator BvFixedVector<Type, N>::Insert(ConstIterator position, const Type & value)
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
inline typename BvFixedVector<Type, N>::Iterator BvFixedVector<Type, N>::Insert(ConstIterator position, const size_t count, const Type & value)
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
inline typename BvFixedVector<Type, N>::Iterator BvFixedVector<Type, N>::Insert(ConstIterator position, Iterator first, Iterator last)
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
inline typename BvFixedVector<Type, N>::Iterator BvFixedVector<Type, N>::Insert(ConstIterator position, Type && value)
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
inline typename BvFixedVector<Type, N>::Iterator BvFixedVector<Type, N>::Insert(ConstIterator position, std::initializer_list<Type> list)
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
inline typename BvFixedVector<Type, N>::Iterator BvFixedVector<Type, N>::Erase(ConstIterator position)
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
inline typename BvFixedVector<Type, N>::Iterator BvFixedVector<Type, N>::Erase(ConstIterator first, ConstIterator last)
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
inline void BvFixedVector<Type, N>::Clear()
{
	for (size_t i = m_Size; i > 0; i--)
	{
		m_pData[i - 1].~Type();
	}

	m_Size = 0;
}

template<class Type, size_t N>
template<class ...Args>
inline typename BvFixedVector<Type, N>::Iterator BvFixedVector<Type, N>::Emplace(ConstIterator position, Args && ...args)
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
inline void BvFixedVector<Type, N>::EmplaceBack(Args && ...args)
{
	if (m_Size == N)
	{
		return;
	}

	new (&m_pData[m_Size++]) Type(std::forward<Args>(args)...);
}