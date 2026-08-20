#pragma once


// BvVector
// Container class, with variable-sized memory. Once memory has been allocated for a specific instance, the specified amount becomes
// the maximum size for that instance. If the size needs to be increased, memory is reallocated again.
// The class can reserve and/or resize its underlying container, but not shrink.


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BDeV/Core/System/Memory/BvMemory.h"
#include <iterator>


template<typename T>
class BvVector
{
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

	BvVector() // Default
		: m_pArena(BV_DEFAULT_MEMORY_ARENA)
	{}

	explicit BvVector(IBvMemoryArena* pArena) // Allocator
		: m_pArena(pArena)
	{}

	explicit BvVector(size_type size, IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA) // Reserve
		: m_pArena(pArena)
	{
		Reserve(size);
	}

	explicit BvVector(size_type size, const value_type& val, IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA) // Fill
		: m_pArena(pArena)
	{
		Resize(size, val);
	}

	template<std::input_iterator It>
	explicit BvVector(It start, It end, IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA) // Range
		: m_pArena(pArena)
	{
		Assign(start, end);
	}

	explicit BvVector(size_type size, const value_type* pElements, IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA) // Range
		: m_pArena(pArena)
	{
		Assign(size, pElements);
	}

	BvVector(std::initializer_list<value_type> list, IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA) // Initializer List
		: m_pArena(pArena)
	{
		Assign(list);
	}

	BvVector(const BvVector& rhs) // Copy
		: m_pArena(rhs.m_pArena)
	{
		Reserve(rhs.m_Size);

		for (auto i = 0; i < rhs.m_Size; i++)
		{
			PushBack(rhs.m_pData[i]);
		}
	}

	BvVector(BvVector&& rhs) noexcept // Move
		: m_pData(rhs.m_pData), m_pArena(rhs.m_pArena), m_Size(rhs.m_Size), m_Capacity(rhs.m_Capacity)
	{
		rhs.m_pData = nullptr;
		rhs.m_Size = 0;
		rhs.m_Capacity = 0;
	}

	BvVector& operator=(const BvVector& rhs) // Copy Assignment
	{
		if (this != &rhs)
		{
			Clear(true);
			SetAllocator(rhs.m_pArena);
			Reserve(rhs.m_Size);

			for (auto i = 0; i < rhs.m_Size; i++)
			{
				PushBack(rhs.m_pData[i]);
			}
		}

		return *this;
	}

	BvVector& operator=(BvVector&& rhs) noexcept // Move Assignment
	{
		if (this != &rhs)
		{
			Clear(true);

			m_pData = rhs.m_pData;
			m_pArena = rhs.m_pArena;
			m_Size = rhs.m_Size;
			m_Capacity = rhs.m_Capacity;

			rhs.m_pData = nullptr;
			rhs.m_Size = 0;
			rhs.m_Capacity = 0;
		}

		return *this;
	}

	BvVector& operator=(std::initializer_list<value_type> list) // Copy Assignment
	{
		Clear();

		auto size = list.size();
		Reserve(size);

		for (auto i = 0; i < size; i++)
		{
			PushBack(list[i]);
		}

		return *this;
	}

	~BvVector()
	{
		Clear(true);
	}

	// Allocator
	IBvMemoryArena* GetAllocator() const
	{
		return m_pArena;
	}

	void SetAllocator(IBvMemoryArena* pArena)
	{
		BV_ASSERT(pArena != nullptr, "Memory arena can't be nullptr");
		BV_ASSERT(m_pData == nullptr, "Can't change allocators after allocations have been made");

		m_pArena = pArena;
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
	size_type Capacity() const { return m_Capacity; }
	bool Empty() const { return m_Size == 0; }
	void Resize(size_type size, const value_type& value = value_type())
	{
		if (size > m_Size)
		{
			Reserve(size);

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

	void Reserve(size_type size)
	{
		if (size <= m_Capacity)
		{
			return;
		}

		value_type* pNewData = reinterpret_cast<value_type*>(m_pArena->Allocate(size * sizeof(value_type), alignof(value_type)));
		if constexpr (!IsPodV<value_type>)
		{
			for (auto i = 0u; i < m_Size; i++)
			{
				if constexpr (std::is_nothrow_move_constructible_v<value_type> || !std::is_copy_constructible_v<value_type>)
				{
					new (std::addressof(pNewData[i])) value_type(std::move(m_pData[i]));
				}
				else
				{
					new (std::addressof(pNewData[i])) value_type(m_pData[i]);
				}

				if constexpr (!std::is_trivially_destructible_v<value_type>)
				{
					m_pData[i].~value_type();
				}
			}
		}
		else
		{
			if (m_Size)
			{
				memcpy(pNewData, m_pData, m_Size * sizeof(value_type));
			}
		}

		m_Capacity = size;
		if (m_pData)
		{
			m_pArena->Free(m_pData);
		}
		m_pData = pNewData;
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

		for (; start != end; start++)
		{
			PushBack(*start);
		}
	}

	void Assign(size_type size, const value_type* pElements) // Range
	{
		BV_ASSERT(pElements, "Invalid pointer");

		Clear();

		Reserve(size);

		for (auto i = 0; i < size; ++i)
		{
			PushBack(pElements[i]);
		}
	}

	void Assign(size_type size, const value_type& val) // Fill
	{
		Clear();

		Reserve(size);

		for (u32 i = 0; i < size; i++)
		{
			PushBack(val);
		}
	}

	void Assign(std::initializer_list<value_type> list) // Initializer List
	{
		Clear();

		auto size = list.size();
		Reserve(size);

		for (auto i = 0; i < size; i++)
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

	void Clear(bool purge = false)
	{
		if constexpr (!std::is_trivially_destructible_v<value_type>)
		{
			for (auto i = 0; i < m_Size; i++)
			{
				m_pData[i].~value_type();
			}
		}
		m_Size = 0;

		if (purge && m_pData)
		{
			m_pArena->Free(m_pData);
			m_pData = nullptr;
			m_Capacity = 0;
		}
	}

	template <class... Args>
	reference EmplaceBack(Args&&... args)
	{
		if (m_Size == m_Capacity)
		{
			Reserve(CalculateNewContainerSize(m_Capacity));
		}

		auto pVal = new (std::addressof(m_pData[m_Size++])) value_type(std::forward<Args>(args)...);

		Internal::PropagateAllocator(*pVal, m_pArena);

		return *pVal;
	}

	template <class... Args>
	iterator Emplace(size_type index, Args&&... args)
	{
		if (index >= m_Size)
		{
			EmplaceBack(std::forward<Args>(args)...);
			return end() - 1;
		}

		if (m_Size == m_Capacity)
		{
			Reserve(CalculateNewContainerSize(m_Capacity));
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

		Internal::PropagateAllocator(m_pData[index], m_pArena);

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

	IBvMemoryArena* get_allocator() const { return GetAllocator(); }
	void set_allocator(IBvMemoryArena* pArena) { SetAllocator(pArena); }
	size_type size() const { return Size(); }
	size_type byte_size() const { return ByteSize(); }
	size_type capacity() const { return Capacity(); }
	bool empty() const { return Empty(); }
	void resize(size_type size, const value_type& value = value_type()) { Resize(size, value); }
	void reserve(size_type size) { Reserve(size); }
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
	void pop_back() { PopBack(); }
	void clear(bool purge = false) { Clear(purge); }
	template <class... Args>
	reference emplace_back(Args&&... args) { return EmplaceBack(std::forward<Args>(args)...); }
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

	friend bool operator==(const BvVector& lhs, const BvVector& rhs)
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
	value_type* m_pData = nullptr;
	IBvMemoryArena* m_pArena = nullptr;
	size_type m_Size = 0;
	size_type m_Capacity = 0;
};