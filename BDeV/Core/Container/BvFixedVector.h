#pragma once


// BvFixedVector
// Container class, with fixed-size stack memory.


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Container/BvIterator.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
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
	explicit BvFixedVector(const size_t size, const Type & val = Type()); // Fill
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
	Iterator Erase(size_t index);
	Iterator Erase(size_t startIndex, size_t count);
	Iterator Erase(ConstIterator position);
	Iterator Erase(ConstIterator first, ConstIterator last);
	void Clear();
	template <class... Args>
	Iterator Emplace(ConstIterator position, Args&&... args);
	template <class... Args>
	Type& EmplaceBack(Args&&... args);

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
		new (&m_pData[i]) Type(rhs[i]);
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
			new (&m_pData[i]) Type(rhs.m_pData[i]);
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
		Clear();

		auto size = N < rhs.Size() ? N : rhs.Size();
		for (auto i = 0U; i < size; i++)
		{
			m_pData[i] = std::move(rhs.m_pData[i]);
		}
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
		new (&m_pData[i]) Type(list[i]);
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
			new (&m_pData[i]) Type(value);
		}
	}
	else
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


template<class Type, size_t N>
inline Type & BvFixedVector<Type, N>::operator[](const size_t index)
{
	BvAssert(index < N, "Index out of bounds");
	if (index >= m_Size)
	{
		m_Size = index + 1;
	}
	return m_pData[index];
}

template<class Type, size_t N>
inline const Type & BvFixedVector<Type, N>::operator[](const size_t index) const
{
	BvAssert(m_Size > 0 && index < m_Size, "Index out of bounds");
	return m_pData[index];
}

template<class Type, size_t N>
inline Type & BvFixedVector<Type, N>::At(const size_t index)
{
	BvAssert(m_Size > 0 && index < m_Size, "Index out of bounds");
	return m_pData[index];
}

template<class Type, size_t N>
inline const Type & BvFixedVector<Type, N>::At(const size_t index) const
{
	BvAssert(m_Size > 0 && index < m_Size, "Index out of bounds");
	return m_pData[index];
}

template<class Type, size_t N>
inline Type & BvFixedVector<Type, N>::Front()
{
	BvAssert(m_Size > 0, "Vector is empty");
	return m_pData[0];
}

template<class Type, size_t N>
inline const Type & BvFixedVector<Type, N>::Front() const
{
	BvAssert(m_Size > 0, "Vector is empty");
	return m_pData[0];
}

template<class Type, size_t N>
inline Type & BvFixedVector<Type, N>::Back()
{
	BvAssert(m_Size > 0, "Vector is empty");
	return m_pData[m_Size - 1];
}

template<class Type, size_t N>
inline const Type & BvFixedVector<Type, N>::Back() const
{
	BvAssert(m_Size > 0, "Vector is empty");
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
		new (&m_pData[m_Size++]) Type(*it);
	}
}

template<class Type, size_t N>
inline void BvFixedVector<Type, N>::Assign(const size_t size, const Type & val)
{
	Clear();

	m_Size = N < size ? N : size;
	for (u32 i = 0; i < m_Size; i++)
	{
		new (&m_pData[i]) Type(val);
	}
}

template<class Type, size_t N>
inline void BvFixedVector<Type, N>::Assign(std::initializer_list<Type> list)
{
	Clear();

	m_Size = N < list.size() ? N : list.size();
	for (auto i = 0; i < m_Size; i++)
	{
		new (&m_pData[i]) Type(list[i]);
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
		if constexpr (!std::is_trivially_destructible_v<Type>)
		{
			m_pData[m_Size - 1].~Type();
		}
		--m_Size;
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
		for (auto i = m_Size; i > pos; i--)
		{
			m_pData[i] = std::move(m_pData[i - 1]);
		}
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
		for (auto i = m_Size; i > pos; i--)
		{
			m_pData[i] = std::move(m_pData[i - 1]);
		}
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


template<class Type, size_t N>
inline typename BvFixedVector<Type, N>::Iterator BvFixedVector<Type, N>::Erase(size_t index)
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

template<class Type, size_t N>
inline typename BvFixedVector<Type, N>::Iterator BvFixedVector<Type, N>::Erase(size_t startIndex, size_t count)
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


template<class Type, size_t N>
inline typename BvFixedVector<Type, N>::Iterator BvFixedVector<Type, N>::Erase(ConstIterator position)
{
	auto index = position - ConstIterator(m_pData);
	return Erase(index);
}

template<class Type, size_t N>
inline typename BvFixedVector<Type, N>::Iterator BvFixedVector<Type, N>::Erase(ConstIterator first, ConstIterator last)
{
	auto index = first - ConstIterator(m_pData);
	auto count = last - first;

	return Erase(index, count);
}

template<class Type, size_t N>
inline void BvFixedVector<Type, N>::Clear()
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
		for (auto i = m_Size; i > pos; i--)
		{
			m_pData[i] = std::move(m_pData[i - 1]);
		}
	}

	new (&m_pData[pos]) Type(std::forward<Args>(args)...);
	m_Size++;

	return Iterator(m_pData + pos);
}

template<class Type, size_t N>
template<class ...Args>
inline Type& BvFixedVector<Type, N>::EmplaceBack(Args && ...args)
{
	if (m_Size == N)
	{
		return m_pData[m_Size - 1];
	}

	new (&m_pData[m_Size++]) Type(std::forward<Args>(args)...);

	return m_pData[m_Size - 1];
}