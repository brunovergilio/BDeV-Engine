#pragma once


// BvFixedVector
// Container class, with fixed-size stack memory.


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include <iterator>


template<typename T, size_t N>
class BvFixedVector
{
	static_assert(N > 0, "BvFixedVector requires a non-zero capacity");

public:
	// Types required by standard container interfaces
	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = value_type*;
	using const_pointer = const value_type*;

	// C++20 Random Access / Contiguous Iterators
	using iterator = value_type*;
	using const_iterator = const value_type*;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	BvFixedVector() // Default
	{}

	explicit BvFixedVector(size_type size, const value_type& val = value_type()) // Fill
	{
		Assign(size, val);
	}

	template<std::input_iterator It>
	explicit BvFixedVector(It start, It end) // Range
	{
		Assign(start, end);
	}

	explicit BvFixedVector(size_type size, const value_type* pElements) // Range
	{
		Assign(size, pElements);
	}

	BvFixedVector(std::initializer_list<value_type> list) // Initializer List
	{
		Assign(list);
	}

	template<size_t M>
	BvFixedVector(const BvFixedVector<value_type, M>& rhs) // Copy
	{
		auto count = std::min(N, M);
		for (auto i = 0; i < count; i++)
		{
			PushBack(rhs.m_pData[i]);
		}
	}

	BvFixedVector(BvFixedVector&& rhs) noexcept // Move
	{
		for (auto i = 0; i < rhs.m_Size; i++)
		{
			new (std::addressof(m_pData[i])) value_type(std::move(rhs.m_pData[i]));
		}

		rhs.Clear();
	}

	template<size_t M>
	BvFixedVector& operator=(const BvFixedVector<value_type, M>& rhs) // Copy Assignment
	{
		if constexpr (N == M)
		{
			if (this == &rhs)
			{
				return *this;
			}
		}

		Clear();

		auto count = std::min(N, M);
		for (auto i = 0; i < count; i++)
		{
			PushBack(rhs.m_pData[i]);
		}

		return *this;
	}

	BvFixedVector& operator=(BvFixedVector&& rhs) noexcept // Move Assignment
	{
		if (this != &rhs)
		{
			Clear();

			for (auto i = 0; i < rhs.m_Size; i++)
			{
				new (std::addressof(m_pData[i])) value_type(std::move(rhs.m_pData[i]));
			}

			rhs.Clear();
		}

		return *this;
	}

	BvFixedVector& operator=(std::initializer_list<value_type> list) // Copy Assignment
	{
		Clear();

		auto size = list.size();

		auto count = std::min(size, N);
		for (auto i = 0; i < count; i++)
		{
			PushBack(list[i]);
		}

		return *this;
	}

	~BvFixedVector()
	{
		Clear();
	}

	// Forward Iterators
	iterator begin() noexcept { return m_pData; }
	iterator end() noexcept { return m_pData + m_Size; }

	const_iterator begin() const noexcept { return m_pData; }
	const_iterator end() const noexcept { return m_pData + m_Size; }

	const_iterator cbegin() const noexcept { return m_pData; }
	const_iterator cend() const noexcept { return m_pData + m_Size; }

	// Reverse Iterators
	reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
	reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

	const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
	const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

	const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }
	const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }

	// Capacity
	size_type Size() const { return m_Size; }
	size_type ByteSize() const { return m_Size * sizeof(value_type); }
	size_type Capacity() const { return N; }
	bool Empty() const { return m_Size == 0; }
	void Resize(size_type size, const value_type& value = value_type())
	{
		if (size > N)
		{
			size = N;
		}

		if (size > m_Size)
		{
			for (auto i = m_Size; i < size; i++)
			{
				PushBack(value);
			}
		}
		else if (size < m_Size)
		{
			if constexpr (!std::is_trivially_destructible_v<value_type>)
			{
				for (auto i = m_Size; i > size; i--)
				{
					m_pData[i - 1].~value_type();
				}
			}

			m_Size = size;
		}
	}

	// Element Access
	reference operator[](size_type index)
	{
		BV_ASSERT(m_Size > 0 && index < m_Size, "Index out of bounds");
		return m_pData[index];
	}

	const_reference operator[](size_type index) const
	{
		BV_ASSERT(m_Size > 0 && index < m_Size, "Index out of bounds");
		return m_pData[index];
	}

	reference At(size_type index)
	{
		BV_ASSERT(m_Size > 0 && index < m_Size, "Index out of bounds");
		return m_pData[index];
	}

	const_reference At(size_type index) const
	{
		BV_ASSERT(m_Size > 0 && index < m_Size, "Index out of bounds");
		return m_pData[index];
	}

	reference Front()
	{
		BV_ASSERT(m_Size > 0, "Vector is empty");
		return m_pData[0];
	}

	const_reference Front() const
	{
		BV_ASSERT(m_Size > 0, "Vector is empty");
		return m_pData[0];
	}

	reference Back()
	{
		BV_ASSERT(m_Size > 0, "Vector is empty");
		return m_pData[m_Size - 1];
	}

	const_reference Back() const
	{
		BV_ASSERT(m_Size > 0, "Vector is empty");
		return m_pData[m_Size - 1];
	}

	pointer Data() { return m_pData; }
	const_pointer Data() const { return m_pData; }

	// Modifiers
	template<std::input_iterator It>
	void Assign(It start, It end) // Range
	{
		Clear();

		for (; start != end && m_Size < N; start++)
		{
			PushBack(*start);
		}
	}

	void Assign(size_type size, const value_type* pElements) // Range
	{
		BV_ASSERT(pElements, "Invalid pointer");

		Clear();

		for (auto i = 0; i < size && m_Size < N; ++i)
		{
			PushBack(pElements[i]);
		}
	}

	void Assign(size_type size, const value_type& val) // Fill
	{
		Clear();


		for (u32 i = 0; i < size && m_Size < N; i++)
		{
			PushBack(val);
		}
	}

	void Assign(std::initializer_list<value_type> list) // Initializer List
	{
		Clear();

		auto size = list.size();

		for (auto i = 0; i < size && m_Size < N; i++)
		{
			PushBack(list[i]);
		}
	}

	value_type& PushBack(const value_type& value)
	{
		return EmplaceBack(value);
	}

	value_type& PushBack(value_type&& value)
	{
		return EmplaceBack(std::move(value));
	}

	value_type* TryPushBack(const value_type& value)
	{
		if (m_Size == N)
		{
			return nullptr;
		}

		PushBack(value);
		return &back();
	}

	value_type* TryPushBack(value_type&& value)
	{
		if (m_Size == N)
		{
			return nullptr;
		}

		PushBack(std::move(value));
		return &back();
	}

	void PopBack()
	{
		if (m_Size > 0)
		{
			if constexpr (!std::is_trivially_destructible_v<value_type>)
			{
				m_pData[m_Size - 1].~value_type();
			}
			--m_Size;
		}
	}

	void Clear()
	{
		if constexpr (!std::is_trivially_destructible_v<value_type>)
		{
			for (auto i = 0; i < m_Size; i++)
			{
				m_pData[i].~value_type();
			}
		}
		m_Size = 0;
	}

	template <class... Args>
	reference EmplaceBack(Args&&... args)
	{
		BV_ASSERT(m_Size < N, "Already at maximum capacity");

		auto pVal = new (std::addressof(m_pData[m_Size++])) value_type(std::forward<Args>(args)...);

		return *pVal;
	}

	template <class... Args>
	value_type* TryEmplaceBack(Args&&... args)
	{
		if (m_Size == N)
		{
			return nullptr;
		}

		EmplaceBack(std::forward<Args>(args)...);
		return &back();
	}

	template <class... Args>
	iterator Emplace(size_type index, Args&&... args)
	{
		BV_ASSERT(m_Size < N, "Already at maximum capacity");

		if (index >= m_Size)
		{
			EmplaceBack(std::forward<Args>(args)...);
			return end() - 1;
		}

		// Move-construct the last element into the newly available slot.
		new (std::addressof(m_pData[m_Size])) value_type(std::move(m_pData[m_Size - 1]));

		// Shift elements backwards using construction/destruction rather
		// than move assignment.
		for (auto i = m_Size - 1; i > index; --i)
		{
			m_pData[i].~value_type();

			new (std::addressof(m_pData[i])) value_type(std::move(m_pData[i - 1]));
		}

		// The old object at index is now replaced by the inserted object.
		m_pData[index].~value_type();

		new (std::addressof(m_pData[index])) value_type(std::forward<Args>(args)...);

		++m_Size;

		return m_pData + index;
	}

	template <class... Args>
	iterator Emplace(const_iterator it, Args&&... args)
	{
		size_type index = std::distance(cbegin(), it);
		return Emplace(index, std::forward<Args>(args)...);
	}

	iterator Erase(size_type index)
	{
		if (index >= m_Size)
		{
			return end();
		}

		for (auto i = index; i < m_Size - 1; ++i)
		{
			m_pData[i].~value_type();

			new (std::addressof(m_pData[i])) value_type(std::move(m_pData[i + 1]));
		}

		m_pData[m_Size - 1].~value_type();

		m_Size--;

		return iterator(m_pData + index);
	}

	iterator Erase(size_type startIndex, size_type count)
	{
		if (startIndex >= m_Size)
		{
			return end();
		}

		if (count == 0)
		{
			return m_pData + startIndex;
		}

		if (count > m_Size - startIndex)
		{
			count = m_Size - startIndex;
		}

		auto remaining = m_Size - (startIndex + count);
		for (auto i = 0; i < remaining; i++)
		{
			m_pData[startIndex + i].~value_type();

			new (std::addressof(m_pData[startIndex + i])) value_type(std::move(m_pData[startIndex + count + i]));
		}

		for (auto i = startIndex + remaining; i < m_Size; i++)
		{
			m_pData[i].~value_type();
		}

		m_Size -= count;

		return iterator(m_pData + startIndex);
	}

	iterator Erase(const_iterator it)
	{
		size_type index = std::distance(cbegin(), it);
		return Erase(index);
	}

	iterator Erase(const_iterator first, const_iterator last)
	{
		size_type index = std::distance(cbegin(), first);
		size_type count = std::distance(first, last);
		return Erase(index, count);
	}

	template <class... Args>
	iterator EmplaceUnsorted(size_type index, Args&&... args)
	{
		if (index > m_Size)
		{
			index = m_Size;
		}

		EmplaceBack(std::forward<Args>(args)...);

		if (index < m_Size - 1)
		{
			value_type tmp(std::move(m_pData[index]));

			m_pData[index].~value_type();

			new (std::addressof(m_pData[index])) value_type(std::move(m_pData[m_Size - 1]));

			m_pData[m_Size - 1].~value_type();

			new (std::addressof(m_pData[m_Size - 1])) value_type(std::move(tmp));
		}

		return iterator(m_pData + index);
	}

	template <class... Args>
	iterator EmplaceUnsorted(const_iterator it, Args&&... args)
	{
		size_type index = std::distance(cbegin(), it);
		return EmplaceUnsorted(index, std::forward<Args>(args)...);
	}

	void EraseUnsorted(size_type index)
	{
		BV_ASSERT(index < m_Size, "Index out of bounds");
		if (index < m_Size - 1)
		{
			m_pData[index].~value_type();

			new (std::addressof(m_pData[index])) value_type(std::move(m_pData[m_Size - 1]));
		}

		m_pData[m_Size - 1].~value_type();
		--m_Size;
	}

	void EraseUnsorted(const_iterator it)
	{
		size_type index = std::distance(cbegin(), it);
		EraseUnsorted(index);
	}

	size_type size() const { return Size(); }
	size_type byte_size() const { return ByteSize(); }
	size_type capacity() const { return Capacity(); }
	bool empty() const { return Empty(); }
	void resize(size_type size, const value_type& value = value_type()) { Resize(size, value); }
	reference at(size_type index) { return At(index); }
	const_reference at(size_type index) const { return At(index); }
	reference front() { return Front(); }
	const_reference front() const { return Front(); }
	reference back() { return Back(); }
	const_reference back() const { return Back(); }
	pointer data() { return Data(); }
	const_pointer data() const { return Data(); }
	template<std::input_iterator It>
	void assign(It start, It end) { Assign(start, end); }
	void assign(size_type size, const value_type* pElements) { Assign(size, pElements); }
	void assign(size_type size, const value_type& val) { Assign(size, val); }
	void assign(std::initializer_list<value_type> list) { Assign(list); }
	value_type& push_back(const value_type& value) { return PushBack(value); }
	value_type& push_back(value_type&& value) { return PushBack(std::move(value)); }
	value_type* try_push_back(const value_type& value) { return TryPushBack(value); }
	value_type* try_push_back(value_type&& value) { return TryPushBack(std::move(value)); }
	void pop_back() { PopBack(); }
	void clear() { Clear(); }
	template <class... Args>
	reference emplace_back(Args&&... args) { return EmplaceBack(std::forward<Args>(args)...); }
	template <class... Args>
	value_type* try_emplace_back(Args&&... args) {return TryEmplaceBack(std::forward<Args>(args)...); }
	template <class... Args>
	iterator emplace(size_type index, Args&&... args) { return Emplace(index, std::forward<Args>(args)...); }
	template <class... Args>
	iterator emplace(const_iterator it, Args&&... args) { return Emplace(it, std::forward<Args>(args)...); }
	iterator erase(size_type index) { return Erase(index); }
	iterator erase(size_type startIndex, size_type count) { return Erase(startIndex, count); }
	iterator erase(const_iterator it) { return Erase(it); }
	iterator erase(const_iterator first, const_iterator last) { return Erase(first, last); }
	template <class... Args>
	iterator emplace_unsorted(size_type index, Args&&... args) { return EmplaceUnsorted(index, std::forward<Args>(args)...); }
	template <class... Args>
	iterator emplace_unsorted(const_iterator it, Args&&... args) { return EmplaceUnsorted(it, std::forward<Args>(args)...); }
	void erase_unsorted(size_type index) { EraseUnsorted(index); }
	void erase_unsorted(const_iterator it) { EraseUnsorted(it); }

	friend bool operator==(const BvFixedVector& lhs, const BvFixedVector& rhs)
	{
		if (lhs.m_Size != rhs.m_Size)
		{
			return false;
		}

		for (auto i = 0; i < lhs.m_Size; i++)
		{
			if (lhs[i] != rhs[i])
			{
				return false;
			}
		}

		return true;
	}

private:
	alignas(alignof(value_type)) u8 m_Mem[sizeof(value_type) * N]{};
	value_type* m_pData = reinterpret_cast<value_type*>(m_Mem);
	size_type m_Size = 0;
};