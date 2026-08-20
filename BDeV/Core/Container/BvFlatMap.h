#pragma once


#include "BDeV/Core/Container/BvVector.h"

template<typename K, typename T, typename C>
class BvFlatMap;


template<typename KeyIter, typename MappedIter, typename MappedConstIter>
struct FlatMapIteratorProvider;


template<typename KeyIter, typename MappedIter, typename MappedConstIter>
struct FlatMapIteratorProvider
{
	class BvFlatMapIterator
	{
	public:
		using iterator_category = std::random_access_iterator_tag;
		using iterator_concept = std::random_access_iterator_tag;
		using difference_type = ptrdiff_t;
		using value_type = std::pair<std::iter_value_t<KeyIter>, std::iter_value_t<MappedIter>>;
		using reference = std::pair<std::iter_reference_t<KeyIter>, std::iter_reference_t<MappedIter>>;

		BvFlatMapIterator() = default;

	private:
		template<typename K, typename T, typename C>
		friend class BvFlatMap;

		template<typename KeyIter, typename MappedIter, typename MappedConstIter>
		friend struct FlatMapIteratorProvider;

		explicit BvFlatMapIterator(KeyIter keyIt, MappedIter mappedIt)
			noexcept(std::is_nothrow_move_constructible_v<KeyIter> && std::is_nothrow_move_constructible_v<MappedIter>)
				: m_KeyIt(std::move(keyIt)), m_MappedIt(std::move(mappedIt)) {}

		using const_iterator = FlatMapIteratorProvider<KeyIter, MappedConstIter, MappedConstIter>::BvFlatMapIterator;

		class ArrowProxy
		{
		public:
			[[nodiscard]] const reference* operator->() const noexcept
			{
				return std::addressof(m_Ref);
			}

		private:
			friend BvFlatMapIterator;

			explicit ArrowProxy(const BvFlatMapIterator& it) : m_Ref{ *it } {}

			reference m_Ref;
		};

	public:
		using pointer = ArrowProxy;

		[[nodiscard]] reference operator*() const
		{
			return reference{ *m_KeyIt, *m_MappedIt };
		}

		[[nodiscard]] friend std::pair<std::iter_rvalue_reference_t<KeyIter>, std::iter_rvalue_reference_t<MappedIter>> iter_move(const BvFlatMapIterator& it)
		{
			return { std::ranges::iter_move(it.m_KeyIt), std::ranges::iter_move(it.m_MappedIt) };
		}

		[[nodiscard]] pointer operator->() const
		{
			return pointer{ *this };
		}

		BvFlatMapIterator& operator++()
		{
			++m_KeyIt;
			++m_MappedIt;
			return *this;
		}

		BvFlatMapIterator operator++(int)
		{
			auto old = *this;
			++*this;
			return old;
		}

		[[nodiscard]] bool operator==(const BvFlatMapIterator& rhs) const
		{
			return m_KeyIt == rhs.m_KeyIt;
		}

		[[nodiscard]] auto operator<=>(const BvFlatMapIterator& rhs) const
		{
			return m_KeyIt <=> rhs.m_KeyIt;
		}

		BvFlatMapIterator& operator--()
		{
			--m_KeyIt;
			--m_MappedIt;
			return *this;
		}

		BvFlatMapIterator operator--(int) {
			auto old = *this;
			--*this;
			return old;
		}

		BvFlatMapIterator& operator+=(const difference_type off)
		{
			m_KeyIt += static_cast<std::iter_difference_t<KeyIter>>(off);
			m_MappedIt += static_cast<std::iter_difference_t<MappedIter>>(off);
			return *this;
		}

		BvFlatMapIterator& operator-=(const difference_type off)
		{
			m_KeyIt -= static_cast<std::iter_difference_t<KeyIter>>(off);
			m_MappedIt -= static_cast<std::iter_difference_t<MappedIter>>(off);
			return *this;
		}

		[[nodiscard]] BvFlatMapIterator operator+(const difference_type off) const
		{
			auto tmp = *this;
			tmp += off;
			return tmp;
		}

		[[nodiscard]] BvFlatMapIterator operator-(const difference_type off) const
		{
			auto tmp = *this;
			tmp -= off;
			return tmp;
		}

		[[nodiscard]] reference operator[](const difference_type off) const
		{
			return *(*this + off);
		}

		[[nodiscard]] difference_type operator-(const BvFlatMapIterator& rhs) const
		{
			return m_KeyIt - rhs.m_KeyIt;
		}

		[[nodiscard]] friend BvFlatMapIterator operator+(const difference_type off, const BvFlatMapIterator& rhs) {
			return rhs + off;
		}

		[[nodiscard]] operator const_iterator() const
			requires (!std::is_same_v<MappedIter, MappedConstIter>)
		{
			return const_iterator{ m_KeyIt, m_MappedIt };
		}

	private:
		KeyIter m_KeyIt;
		MappedIter m_MappedIt;
	};
};


template<typename K, typename T, typename C = std::less<K>>
class BvFlatMap
{
public:
	using key_type = K;
	using mapped_type = T;
	using value_type = std::pair<key_type, mapped_type>;
	using key_compare = C;
	using reference = std::pair<const key_type&, mapped_type&>;
	using const_reference = std::pair<const key_type&, const mapped_type&>;
	using size_type = size_t;
	using difference_type = ptrdiff_t;
	using key_container_type = BvVector<key_type>;
	using mapped_container_type = BvVector<mapped_type>;

	using iterator = FlatMapIteratorProvider<typename key_container_type::const_iterator,
		typename mapped_container_type::iterator, typename mapped_container_type::const_iterator>::BvFlatMapIterator;

	using const_iterator = FlatMapIteratorProvider<typename key_container_type::const_iterator,
		typename mapped_container_type::const_iterator, typename mapped_container_type::const_iterator>::BvFlatMapIterator;

	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	BvFlatMap() // Default
		: m_Keys(BV_DEFAULT_MEMORY_ARENA), m_Values(BV_DEFAULT_MEMORY_ARENA)
	{}

	explicit BvFlatMap(IBvMemoryArena* pArena) // Allocator
		: m_Keys(pArena), m_Values(pArena)
	{}

	explicit BvFlatMap(size_type size, IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA) // Reserve
		: m_Keys(size, pArena), m_Values(size, pArena)
	{
	}

	explicit BvFlatMap(size_type size, const value_type* pElements, IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA) // Range
		: m_Keys(size, pArena), m_Values(size, pArena)
	{
		for (auto i = 0; i < size; i++)
		{
			Insert(pElements[i]);
		}
	}

	BvFlatMap(std::initializer_list<value_type> list, IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA) // Initializer List
		: m_Keys(pArena), m_Values(pArena)
	{
		for (auto i = 0; i < list.size(); i++)
		{
			Insert(list[i]);
		}
	}

	BvFlatMap(const BvFlatMap& rhs) // Copy
		: m_Keys(rhs.m_Keys), m_Values(rhs.m_Values)
	{
	}

	BvFlatMap(BvFlatMap&& rhs) noexcept // Move
		: m_Keys(std::move(rhs.m_Keys)), m_Values(std::move(rhs.m_Values))
	{
	}

	BvFlatMap& operator=(const BvFlatMap& rhs) // Copy Assignment
	{
		if (this != &rhs)
		{
			Clear(true);
			SetAllocator(rhs.m_Keys.m_pArena);
			Reserve(rhs.m_Keys.Size());

			for (auto i = 0; i < rhs.m_Keys.Size(); i++)
			{
				Emplace(rhs.m_Keys[i], rhs.m_Values[i]);
			}
		}

		return *this;
	}

	BvFlatMap& operator=(BvFlatMap&& rhs) noexcept // Move Assignment
	{
		if (this != &rhs)
		{
			Clear(true);

			m_Keys = std::move(rhs.m_Keys);
			m_Values = std::move(rhs.m_Values);
		}

		return *this;
	}

	BvFlatMap& operator=(std::initializer_list<value_type> list) // Copy Assignment
	{
		Clear();

		auto size = list.size();
		Reserve(size);

		for (auto i = 0; i < size; i++)
		{
			Emplace(list[i]);
		}

		return *this;
	}

	~BvFlatMap()
	{
		Clear(true);
	}

	// Allocator
	IBvMemoryArena* GetAllocator() const
	{
		return m_Keys.GetAllocator();
	}

	void SetAllocator(IBvMemoryArena* pArena)
	{
		m_Keys.SetAllocator(pArena);
		m_Values.SetAllocator(pArena);
	}

	// Forward Iterators
	iterator begin() noexcept { return iterator{ m_Keys.cbegin(), m_Values.begin() }; }
	iterator end() noexcept { return iterator{ m_Keys.cend(), m_Values.end() }; }

	const_iterator begin() const noexcept { return iterator{ m_Keys.cbegin(), m_Values.cbegin() }; }
	const_iterator end() const noexcept { return iterator{ m_Keys.cend(), m_Values.cend() }; }

	const_iterator cbegin() const noexcept { return iterator{ m_Keys.cbegin(), m_Values.cbegin() }; }
	const_iterator cend() const noexcept { return iterator{ m_Keys.cend(), m_Values.cend() }; }

	// Reverse Iterators
	reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
	reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

	const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
	const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

	const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }
	const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }

	size_type Size() const { return m_Keys.Size(); }
	size_type Capacity() const { return m_Keys.Capacity(); }
	bool Empty() const { return m_Keys.Empty(); }

	void Reserve(size_type size)
	{
		m_Keys.Reserve(size);
		m_Values.Reserve(size);
	}

	template<class... Args>
	std::pair<iterator, bool> Emplace(const key_type& key, Args&&... args)
	{
		key_compare comp;
		auto keyIt = std::upper_bound(m_Keys.begin(), m_Keys.end(), key, comp);
		auto index = keyIt - m_Keys.begin();
		if (keyIt != m_Keys.begin() && !comp(*(keyIt - 1), key))
		{
			return { begin() + (index - 1), false };
		}

		m_Keys.Emplace(index, key);
		m_Values.Emplace(index, std::forward<Args>(args)...);

		return { begin() + index, true };
	}

	template<class... Args>
	std::pair<iterator, bool> Emplace(key_type&& key, Args&&... args)
	{
		key_compare comp;
		auto keyIt = std::upper_bound(m_Keys.begin(), m_Keys.end(), key, comp);
		auto index = keyIt - m_Keys.begin();
		if (keyIt != m_Keys.begin() && !comp(*(keyIt - 1), key))
		{
			return { begin() + (index - 1), false };
		}

		m_Keys.Emplace(index, std::forward<key_type>(key));
		m_Values.Emplace(index, std::forward<Args>(args)...);

		return { begin() + index, true };
	}

	std::pair<iterator, bool> Insert(const value_type& keyVal)
	{
		key_compare comp;
		auto keyIt = std::upper_bound(m_Keys.begin(), m_Keys.end(), keyVal.first, comp);
		auto index = keyIt - m_Keys.begin();
		if (keyIt != m_Keys.begin() && !comp(*(keyIt - 1), keyVal.first))
		{
			return { begin() + (index - 1), false };
		}

		m_Keys.Emplace(index, keyVal.first);
		m_Values.Emplace(index, keyVal.second);

		return { begin() + index, true };
	}

	std::pair<iterator, bool> Insert(value_type&& keyVal)
	{
		key_compare comp;
		auto keyIt = std::upper_bound(m_Keys.begin(), m_Keys.end(), keyVal.first, comp);
		auto index = keyIt - m_Keys.begin();
		if (keyIt != m_Keys.begin() && !comp(*(keyIt - 1), keyVal.first))
		{
			return { begin() + (index - 1), false };
		}

		m_Keys.Emplace(index, std::forward<key_type>(keyVal.first));
		m_Values.Emplace(index, std::forward<mapped_type>(keyVal.second));

		return { begin() + index, true };
	}

	template<typename NV>
	std::pair<iterator, bool> InsertOrAssign(const key_type& key, NV&& value)
	{
		auto result = Emplace(key, std::forward<NV>(value));
		if (!result.second)
		{
			result.first->second = std::forward<NV>(value);
		}

		return result;
	}

	template<typename NV>
	std::pair<iterator, bool> InsertOrAssign(key_type&& key, NV&& value)
	{
		auto result = Emplace(std::forward<key_type>(key), std::forward<NV>(value));
		if (!result.second)
		{
			result.first->second = std::forward<NV>(value);
		}

		return result;
	}

	template<typename NK, typename NV>
	std::pair<iterator, bool> InsertOrAssign(NK&& key, NV&& value)
	{
		auto result = Emplace(std::forward<NK>(key), std::forward<NV>(value));
		if (!result.second)
		{
			result.first->second = std::forward<NV>(value);
		}

		return result;
	}

	bool Erase(const key_type& key)
	{
		key_compare comp;
		auto keyIt = std::upper_bound(m_Keys.begin(), m_Keys.end(), key, comp);
		if (keyIt != m_Keys.begin() && !comp(*(keyIt - 1), key))
		{
			auto index = (keyIt - 1) - m_Keys.begin();
			m_Keys.Erase(index);
			m_Values.Erase(index);
			
			return true;
		}

		return false;
	}

	void Clear(bool purge = false)
	{
		m_Keys.Clear(purge);
		m_Values.Clear(purge);
	}

	iterator LowerBound(const key_type& key)
	{
		auto it = std::lower_bound(m_Keys.begin(), m_Keys.end(), key, key_compare());
		if (it == m_Keys.end())
		{
			return end();
		}

		auto index = it - m_Keys.begin();
		return begin() + index;
	}

	const_iterator LowerBound(const key_type& key) const
	{
		auto it = std::lower_bound(m_Keys.cbegin(), m_Keys.cend(), key, key_compare());
		if (it == m_Keys.cend())
		{
			return cend();
		}

		auto index = it - m_Keys.cbegin();
		return cbegin() + index;
	}

	iterator UpperBound(const key_type& key)
	{
		auto it = std::upper_bound(m_Keys.begin(), m_Keys.end(), key, key_compare());
		if (it == m_Keys.end())
		{
			return end();
		}

		auto index = it - m_Keys.begin();
		return begin() + index;
	}

	const_iterator UpperBound(const key_type& key) const
	{
		auto it = std::upper_bound(m_Keys.cbegin(), m_Keys.cend(), key, key_compare());
		if (it == m_Keys.cend())
		{
			return cend();
		}

		auto index = it - m_Keys.cbegin();
		return cbegin() + index;
	}

	iterator Find(const key_type& key)
	{
		key_compare comp;
		auto it = std::lower_bound(m_Keys.begin(), m_Keys.end(), key, comp);
		if (it != m_Keys.end() && !comp(*it, key))
		{
			auto index = it - m_Keys.begin();
			return begin() + index;
		}

		return end();
	}

	const_iterator Find(const key_type& key) const
	{
		key_compare comp;
		auto it = std::lower_bound(m_Keys.cbegin(), m_Keys.cend(), key, comp);
		if (it != m_Keys.cend() && !comp(*it, key))
		{
			auto index = it - m_Keys.cbegin();
			return cbegin() + index;
		}

		return cend();
	}

	key_container_type& Keys() { return m_Keys; }
	const key_container_type& Keys() const { return m_Keys; }
	mapped_container_type& Values() { return m_Values; }
	const mapped_container_type& Values() const { return m_Values; }

	size_type size() const { return Size(); }
	size_type capacity() const { return Capacity(); }
	bool empty() const { return Empty(); }
	void reserve(size_type size) { return Reserve(size); }
	template<class... Args>
	std::pair<iterator, bool> emplace(const key_type& key, Args&&... args) { return Emplace(key, std::forward<Args>(args)...); }
	template<class... Args>
	std::pair<iterator, bool> emplace(key_type&& key, Args&&... args) { return Emplace(std::forward<key_type>(key), std::forward<Args>(args)...); }
	std::pair<iterator, bool> insert(const value_type& keyVal) { return Insert(keyVal); }
	std::pair<iterator, bool> insert(value_type&& keyVal) { return Insert(std::forward<value_type>(keyVal)); }
	template<typename NV>
	std::pair<iterator, bool> insert_or_assign(const key_type& key, NV&& value) { return InsertOrAssign(key, std::forward<NV>(value)); }
	template<typename NV>
	std::pair<iterator, bool> insert_or_assign(key_type&& key, NV&& value) { return InsertOrAssign(std::forward<key_type>(key), std::forward<NV>(value)); }
	template<typename NK, typename NV>
	std::pair<iterator, bool> insert_or_assign(NK&& key, NV&& value) { return InsertOrAssign(std::forward<NK>(key), std::forward<NV>(value)); }
	bool erase(const key_type& key) { return Erase(key); }
	void clear(bool purge = false) { return Clear(purge); }
	iterator lower_bound(const key_type& key) { return LowerBound(key); }
	const_iterator lower_bound(const key_type& key) const { return LowerBound(key); }
	iterator upper_bound(const key_type& key) { return UpperBound(key); }
	const_iterator upper_bound(const key_type& key) const { return UpperBound(key); }
	iterator find(const key_type& key) { return Find(key); }
	const_iterator find(const key_type& key) const { return Find(key); }
	key_container_type& keys() { return Keys(); }
	const key_container_type& keys() const { return Keys(); }
	mapped_container_type& values() { return Values(); }
	const mapped_container_type& values() const { return Values(); }

private:
	key_container_type m_Keys;
	mapped_container_type m_Values;
};