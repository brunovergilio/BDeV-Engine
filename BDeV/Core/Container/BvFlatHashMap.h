#pragma once


// BvFlatHashTable
// Swiss table implementation, based on Google's abseil (https://github.com/abseil/abseil-cpp/blob/master/absl/container/flat_hash_map.h)


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BDeV/Core/System/Memory/BvMemory.h"
#include "BDeV/Core/Utils/BvHash.h"
#include <iterator>


namespace HashTableDetail
{
	using HashCtrl = i8;

	constexpr HashCtrl kEmpty = -128; // 0x80
	constexpr HashCtrl kDeleted = -2; // 0xFE
	constexpr HashCtrl kSentinel = -1; // 0xFF
	constexpr HashCtrl kTemporary = -3; // 0xFD

	BV_INLINE bool IsFull(HashCtrl ctrl) { return ctrl >= 0; }
	BV_INLINE bool IsEmpty(HashCtrl ctrl) { return ctrl == kEmpty; }
	BV_INLINE bool IsDeleted(HashCtrl ctrl) { return ctrl == kDeleted; }
	BV_INLINE bool IsEmptyOrDeleted(HashCtrl ctrl) { return IsEmpty(ctrl) || IsDeleted(ctrl); }
	BV_INLINE bool IsEnd(HashCtrl ctrl) { return ctrl == kSentinel; }

	inline std::pair<size_t, HashCtrl> SplitHash(size_t hash)
	{
		constexpr u8 kFingerprintMask = 0x7F;
		return { hash >> 7, static_cast<HashCtrl>(hash & kFingerprintMask) };
	}

#if BV_HAS_SSE2
	constexpr size_t kGroupSize = 16;
	
	class GroupMatcher
	{
	public:
		explicit GroupMatcher(const HashCtrl* pCtrl)
			: m_Ctrl(_mm_loadu_si128(reinterpret_cast<const __m128i*>(pCtrl)))
		{
		}

		u16 Match(HashCtrl h2) const
		{
			auto match = _mm_set1_epi8(h2);
			return static_cast<u16>(_mm_movemask_epi8(_mm_cmpeq_epi8(m_Ctrl, match)));
		}

		u16 MatchEmpty() const
		{
			auto match = _mm_set1_epi8(kEmpty);
			return static_cast<u16>(_mm_movemask_epi8(_mm_cmpeq_epi8(m_Ctrl, match)));
		}

		u16 MatchDeleted() const
		{
			auto match = _mm_set1_epi8(kDeleted);
			return static_cast<u16>(_mm_movemask_epi8(_mm_cmpeq_epi8(m_Ctrl, match)));
		}

		u16 MatchEmptyOrDeleted() const
		{
			return MatchEmpty() | MatchDeleted();
		}

		u16 MatchFull() const
		{
			return static_cast<u16>(_mm_movemask_epi8(m_Ctrl) ^ 0xFFFF);
		}

	private:
		__m128i m_Ctrl;
	};
#else
#error "Architecture not supported"
#endif

	class GroupWalker
	{
	public:
		explicit GroupWalker(size_t capacity, size_t h1)
			: m_Capacity(capacity), m_Offset(h1& (m_Capacity - 1)) {}

		void Next() { m_Offset = (m_Offset + kGroupSize) & (m_Capacity - 1); }

		size_t SlotIndex() const { return m_Offset; }

		template<std::integral T> size_t SlotIndex(T value) const
		{
			auto newOffset = m_Offset + value;

			BV_ASSERT(newOffset != m_Capacity, "This should never happen!");

			return (newOffset - size_t(newOffset > m_Capacity) * (m_Capacity + 1)) & (m_Capacity - 1);
		}

	private:
		size_t m_Capacity;
		size_t m_Offset;
	};

	// Backing memory layout
	struct TableData
	{
		size_t m_Capacity; // Maximum size
		size_t m_Size; // Current size
		size_t m_Deleted; // How many tombstones
		size_t m_GrowthLeft; // How much more can be added before a resize and rehash is needed => capacity * max_load - (size + deleted)
		// HashCtrl m_Ctrls[m_Capacity] - controls
		// HashCtrl m_Sentinel - for iterators to know the end (always kSentinel)
		// HashCtrl m_CtrlClones[kGroupSize - 1]; // To avoid overflow on SIMD types
		// padding bytes for slot type alignment
		// Slots[m_Capacity]
	};

	template<typename K>
	struct KeyTraits
	{
		static const K& Get(const K& value)
		{
			return value;
		}
	};

	template<typename K, typename V>
	struct KeyTraits<std::pair<const K, V>>
	{
		static const K& Get(const std::pair<const K, V>& value)
		{
			return value.first;
		}
	};

	template <typename T>
	concept HasIsTransparent = requires
	{
		typename T::is_transparent;
	};

	template<typename Hasher, typename KeyCompare>
	concept HashTableSupportsTransparent = HasIsTransparent<Hasher> && HasIsTransparent<KeyCompare>;

	template<typename K, typename V, typename H, typename C>
	class BvFlatHashTable;

	template<typename T, bool IsConst>
	class Iterator
	{
	public:
		template<typename K, typename V, typename H, typename C>
		friend class BvFlatHashTable;

		using iterator_category = std::forward_iterator_tag;
		using iterator_concept = std::forward_iterator_tag;

		using value_type = std::conditional_t<(IsConst), const T, T>;
		using reference = std::conditional_t<(IsConst), const T&, T&>;
		using pointer = std::remove_reference_t<reference>*;
		using difference_type = ptrdiff_t;
		using control_type = HashCtrl;

		reference operator*() const
		{
			BV_ASSERT(IsFull(*m_pCtrls), "Empty slot");
			return *m_pSlots;
		}

		pointer operator->() const
		{
			return std::addressof(*m_pSlots);
		}

		Iterator& operator++()
		{
			BV_ASSERT(IsFull(*m_pCtrls), "Empty slot");
			m_pCtrls++;
			m_pSlots++;
			SkipEmptyOrDeleted();
			return *this;
		}

		Iterator operator++(int)
		{
			auto copy = *this;
			++(*this);
			return copy;
		}

		friend bool operator==(const Iterator&, const Iterator&) = default;

	private:
		Iterator(control_type* pCtrls, value_type* pSlots)
			: m_pCtrls(pCtrls), m_pSlots(pSlots)
		{
			BV_ASSERT(m_pCtrls && m_pSlots, "Invalid data");
			SkipEmptyOrDeleted();
		}

		Iterator(control_type* pCtrls, value_type* pSlots, i32 noAdvance)
			: m_pCtrls(pCtrls), m_pSlots(pSlots)
		{
			BV_ASSERT(m_pCtrls && m_pSlots, "Invalid data");
		}

		void SkipEmptyOrDeleted()
		{
			[[unlikely]]
			if (IsEnd(*m_pCtrls))
			{
				return;
			}

			u16 matches = 0;
			size_t groupIndex = 0;
			do
			{
				GroupMatcher group(m_pCtrls + groupIndex);
				// Should hit, even if it's just the sentinel
				matches = ~group.MatchEmptyOrDeleted();
				groupIndex += kGroupSize;
			} while (!matches);

			auto offset = std::countr_zero(matches);
			m_pCtrls += offset;
			m_pSlots += offset;
		}

		control_type* m_pCtrls = nullptr;
		value_type* m_pSlots = nullptr;
	};

	struct FindResult
	{
		size_t m_Index;
		i8 m_H2;
		bool m_Found;
	};

	template<typename K, typename V, typename H = BvHash<K>, typename C = std::equal_to<K>>
	class BvFlatHashTable
	{
	protected:
		using mapped_type = V;
		using control_type = HashCtrl;

	public:
		using key_type = K;
		using value_type = std::conditional_t<(!std::is_same_v<V, void>), std::pair<const key_type, mapped_type>, key_type>;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using hasher = H;
		using key_equal = C;
		using reference = value_type&;
		using const_reference = const value_type&;
		using iterator = Iterator<value_type, (std::is_same_v<V, void>)>;
		using const_iterator = Iterator<value_type, true>;
		using key_traits = KeyTraits<value_type>;

		struct Table
		{
			TableData* m_pData;
			control_type* m_pCtrls;
			value_type* m_pSlots;
		};

	public:
		BvFlatHashTable()
			: m_pArena(BV_DEFAULT_MEMORY_ARENA)
		{}

		explicit BvFlatHashTable(IBvMemoryArena* pArena)
			: m_pArena(pArena)
		{}

		explicit BvFlatHashTable(size_type size, IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA)
			: m_pArena(pArena)
		{
			Resize(size);
		}

		explicit BvFlatHashTable(size_type size, const value_type* pElements, IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA)
			: m_pArena(pArena)
		{
			Resize(IdealCapacity(size));
			for (auto i = 0; i < size; i++)
			{
				Insert(pElements[i]);
			}
		}

		BvFlatHashTable(std::initializer_list<value_type> list, IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA)
			: m_pArena(pArena)
		{
			auto size = list.size();
			Resize(IdealCapacity(size));
			for (auto i = 0; i < size; i++)
			{
				Insert(list[i]);
			}
		}

		BvFlatHashTable(const BvFlatHashTable& rhs)
			: m_pArena(rhs.m_pArena)
		{
			if (auto pData = rhs.m_Table.m_pData)
			{
				Resize(pData->m_Capacity);
				for (auto it = rhs.begin(); it != rhs.end(); it++)
				{
					Insert(*it);
				}
			}
		}

		BvFlatHashTable(BvFlatHashTable&& rhs) noexcept
			: m_pArena(rhs.m_pArena), m_Table(rhs.m_Table)
		{
			rhs.m_Table = {};
		}

		BvFlatHashTable& operator=(const BvFlatHashTable& rhs) // Copy Assignment
		{
			if (this != &rhs)
			{
				Clear(true);
				SetAllocator(rhs.m_pArena);
				if (auto pData = rhs.m_Table.m_pData)
				{
					Resize(pData->m_Capacity);
					for (auto it = rhs.begin(); it != rhs.end(); it++)
					{
						Insert(*it);
					}
				}
			}

			return *this;
		}

		BvFlatHashTable& operator=(BvFlatHashTable&& rhs) noexcept // Move Assignment
		{
			if (this != &rhs)
			{
				Clear(true);

				m_Table = rhs.m_Table;
				m_pArena = rhs.m_pArena;

				rhs.m_Table = {};
			}

			return *this;
		}

		BvFlatHashTable& operator=(std::initializer_list<value_type> list) // Copy Assignment
		{
			Clear();

			auto size = list.size();
			Resize(IdealCapacity(size));
			for (auto i = 0; i < size; i++)
			{
				Insert(list[i]);
			}

			return *this;
		}

		~BvFlatHashTable()
		{
			Clear(true);
		}

		IBvMemoryArena* GetAllocator() const
		{
			return m_pArena;
		}

		void SetAllocator(IBvMemoryArena* pArena)
		{
			BV_ASSERT(pArena != nullptr, "Memory arena can't be nullptr");
			BV_ASSERT(m_Table.m_pData == nullptr, "Can't change allocators after allocations have been made");

			m_pArena = pArena;
		}

		void Clear(bool purge = false)
		{
			if (!m_Table.m_pData)
			{
				return;
			}

			if (m_Table.m_pData->m_Capacity)
			{
				for (auto i = 0; i < m_Table.m_pData->m_Capacity; i++)
				{
					if (m_Table.m_pCtrls[i] >= 0)
					{
						m_Table.m_pSlots[i].~value_type();
					}
				}
			}

			if (purge)
			{
				m_pArena->Free(m_Table.m_pData);
				m_Table = {};
			}
			else
			{
				std::fill_n(m_Table.m_pCtrls, m_Table.m_pData->m_Capacity + kGroupSize, kEmpty);
				m_Table.m_pCtrls[m_Table.m_pData->m_Capacity] = kSentinel;
				m_Table.m_pData->m_Size = m_Table.m_pData->m_Deleted = 0;
				m_Table.m_pData->m_GrowthLeft = m_Table.m_pData->m_Capacity;
			}
		}

		size_t Capacity() const { return m_Table.m_pData ? m_Table.m_pData->m_Capacity : 0; }
		size_t Size() const { return m_Table.m_pData ? m_Table.m_pData->m_Size : 0; }
		size_t Deleted() const { return m_Table.m_pData ? m_Table.m_pData->m_Deleted : 0; }
		size_t GrowthLeft() const { return m_Table.m_pData ? m_Table.m_pData->m_GrowthLeft : 0; }

		void Resize(size_t capacity)
		{
			// Always work with powers of 2, and a minimum size of 4
			constexpr size_t kMinSize = 4;
			capacity = std::max(capacity, kMinSize);
			if (capacity & (capacity - 1))
			{
				capacity = std::bit_ceil(capacity);
			}

			if (m_Table.m_pData && m_Table.m_pData->m_Capacity >= capacity)
			{
				return;
			}

			Table table = AllocateTable(capacity);

			if (m_Table.m_pData)
			{
				for (auto it = begin(); it != end(); it++)
				{
					InsertInternal(table, std::move(*it));
				}

				Clear(true);
			}

			m_Table = table;
		}

		std::pair<iterator, bool> Insert(const value_type& keyValue)
		{
			PrepareForInsert();

			return InsertInternal(m_Table, keyValue);
		}

		std::pair<iterator, bool> Insert(value_type&& keyValue)
		{
			PrepareForInsert();

			return InsertInternal(m_Table, std::move(keyValue));
		}

		template<typename KV>
			requires (!std::same_as<std::remove_cvref_t<KV>, value_type>&& std::constructible_from<value_type, KV>)
		std::pair<iterator, bool> Insert(KV&& keyValue)
		{
			PrepareForInsert();

			return InsertInternal(m_Table, std::forward<KV>(keyValue));
		}

		iterator Erase(const key_type& key)
		{
			auto index = FindIndex(key);
			if (index >= m_Table.m_pData->m_Capacity)
			{
				return end();
			}

			EraseInternal(index);

			return iterator{ m_Table.m_pCtrls + index + 1, m_Table.m_pSlots + index + 1 };
		}

		template<typename NK>
			requires (!std::same_as<std::remove_cvref_t<NK>, key_type>)
		iterator Erase(const NK& key)
		{
			auto index = FindIndex(key);
			if (index >= m_Table.m_pData->m_Capacity)
			{
				return end();
			}

			EraseInternal(index);

			return iterator{ m_Table.m_pCtrls + index + 1, m_Table.m_pSlots + index + 1 };
		}

		iterator Erase(iterator pos)
		{
			difference_type index = std::distance(m_Table.m_pCtrls, pos.m_pCtrls);
			if (index >= m_Table.m_pData->m_Capacity)
			{
				return end();
			}

			EraseInternal(index);

			return iterator{ m_Table.m_pCtrls + index + 1, m_Table.m_pSlots + index + 1 };
		}

		iterator Erase(const_iterator pos)
			requires(!std::is_same_v<iterator, const_iterator>)
		{
			difference_type index = std::distance(m_Table.m_pCtrls, pos.m_pCtrls);
			if (index >= m_Table.m_pData->m_Capacity)
			{
				return end();
			}

			EraseInternal(index);

			return iterator{ m_Table.m_pCtrls + index + 1, m_Table.m_pSlots + index + 1 };
		}

		iterator Find(const key_type& key)
		{
			auto index = FindIndex(key);
			if (index >= m_Table.m_pData->m_Capacity)
			{
				return end();
			}

			return iterator{ m_Table.m_pCtrls + index, m_Table.m_pSlots + index };
		}

		const_iterator Find(const key_type& key) const
		{
			auto index = FindIndex(key);
			if (index >= m_Table.m_pData->m_Capacity)
			{
				return end();
			}

			return const_iterator{ m_Table.m_pCtrls + index, m_Table.m_pSlots + index };
		}

		template<typename NK>
			requires (!std::same_as<std::remove_cvref_t<NK>, key_type>)
		iterator Find(const NK& key)
		{
			auto index = FindIndex(key);
			if (index >= m_Table.m_pData->m_Capacity)
			{
				return end();
			}

			return iterator{ m_Table.m_pCtrls + index, m_Table.m_pSlots + index };
		}

		template<typename NK>
			requires (!std::same_as<std::remove_cvref_t<NK>, key_type>)
		const_iterator Find(const NK& key) const
		{
			auto index = FindIndex(key);
			if (index >= m_Table.m_pData->m_Capacity)
			{
				return end();
			}

			return const_iterator{ m_Table.m_pCtrls + index, m_Table.m_pSlots + index };
		}

		// Forward Iterators
		iterator begin() noexcept { return iterator{ m_Table.m_pCtrls, m_Table.m_pSlots }; }
		iterator end() noexcept { return iterator{ m_Table.m_pCtrls + m_Table.m_pData->m_Capacity, m_Table.m_pSlots + m_Table.m_pData->m_Capacity, {} }; }

		const_iterator begin() const noexcept { return const_iterator{ m_Table.m_pCtrls, m_Table.m_pSlots }; }
		const_iterator end() const noexcept { return const_iterator{ m_Table.m_pCtrls + m_Table.m_pData->m_Capacity, m_Table.m_pSlots + m_Table.m_pData->m_Capacity, {} }; }

		const_iterator cbegin() const noexcept { return const_iterator{ m_Table.m_pCtrls, m_Table.m_pSlots }; }
		const_iterator cend() const noexcept { return const_iterator{ m_Table.m_pCtrls + m_Table.m_pData->m_Capacity, m_Table.m_pSlots + m_Table.m_pData->m_Capacity, {} }; }

		IBvMemoryArena* get_allocator() const { return GetAllocator(); }
		void set_allocator(IBvMemoryArena* pArena) { SetAllocator(pArena); }
		void clear(bool purge = false) { Clear(purge); }
		size_t capacity() const { return Capacity(); }
		size_t size() const { return Size(); }
		size_t deleted() const { return Deleted(); }
		size_t growthLeft() const { return GrowthLeft(); }
		void resize(size_t capacity) { Resize(capacity); }
		std::pair<iterator, bool> insert(const value_type& keyValue) { return Insert(keyValue); }
		std::pair<iterator, bool> insert(value_type&& keyValue) { return Insert(std::move(keyValue)); }
		template<typename KV>
			requires (!std::same_as<std::remove_cvref_t<KV>, value_type>&& std::constructible_from<value_type, KV>)
		std::pair<iterator, bool> insert(KV&& keyValue) { return Insert(std::forward<KV>(keyValue)); }
		iterator erase(const key_type& key) { return Erase(key); }
		template<typename NK>
			requires (!std::same_as<std::remove_cvref_t<NK>, key_type>)
		iterator erase(const NK& key) { return Erase(key); }
		iterator erase(iterator pos) { return Erase(pos); }
		iterator erase(const_iterator pos) requires(!std::is_same_v<iterator, const_iterator>) { return Erase(pos); }
		iterator find(const key_type& key) { return Find(key); }
		const_iterator find(const key_type& key) const { return Find(key); }
		template<typename NK>
			requires (!std::same_as<std::remove_cvref_t<NK>, key_type>)
		iterator find(const NK& key) { return Find(key); }
		template<typename NK>
			requires (!std::same_as<std::remove_cvref_t<NK>, key_type>)
		const_iterator find(const NK& key) const { return Find(key); }

	protected:
		static constexpr size_t GetCtrlOffset()
		{
			return sizeof(TableData);
		}

		static constexpr size_t GetSlotOffset(size_t capacity)
		{
			return RoundToNearestMultipleP2(GetCtrlOffset() + sizeof(control_type) * (capacity + kGroupSize), alignof(value_type));
		}

		static constexpr size_t DetermineAllocationSize(size_t capacity)
		{
			// For the total size:
			// Metadata
			// 'control_type' objects
			// 'control_type' sentinel + 'control_type[kGroupSize - 1]' extra elements
			// Padding bytes to account for the alignment of a 'value_type' object
			// 'value_type' objects
			return GetSlotOffset(capacity) + capacity * sizeof(value_type);
		}

		// Returns the size the map should not exceed, given a capacity
		size_t MaxGrowthBudget() const
		{
			constexpr size_t kNum = 7;
			constexpr size_t kDenom = 8;
			return (m_Table.m_pData->m_Capacity * kNum) / kDenom;
		}

		// Returns the 'ideal' size for a given capacity
		size_t IdealCapacity(size_t capacity) const
		{
			constexpr size_t kNum = 8;
			constexpr size_t kDenom = 7;
			return (m_Table.m_pData->m_Capacity * kNum) / kDenom;
		}

		Table AllocateTable(size_t capacity) const
		{
			auto newSize = DetermineAllocationSize(capacity);
			auto pMem = reinterpret_cast<u8*>(m_pArena->Allocate(newSize));

			auto pNewHashTable = reinterpret_cast<TableData*>(pMem);
			pNewHashTable->m_Capacity = capacity;
			pNewHashTable->m_Size = 0;
			pNewHashTable->m_Deleted = 0;
			pNewHashTable->m_GrowthLeft = capacity;

			auto pNewCtrls = reinterpret_cast<control_type*>(pMem + GetCtrlOffset());
			std::fill_n(pNewCtrls, capacity + kGroupSize, kEmpty);
			pNewCtrls[capacity] = kSentinel;

			auto pNewSlots = reinterpret_cast<value_type*>(pMem + GetSlotOffset(capacity));

			return { pNewHashTable, pNewCtrls, pNewSlots };
		}

		static void SetControlValue(control_type* pCtrls, size_t capacity, size_t index, control_type val)
		{
			pCtrls[index] = val;

			// Mirror if one of the first [kGroupSize - 1] elements
			if (index < kGroupSize - 1)
			{
				pCtrls[capacity + 1 + index] = val;
			}
		}

		void PrepareForInsert()
		{
			if (!m_Table.m_pData)
			{
				Resize(0);
			}

			// If we didn't exceed the "ideal" map size, we're good
			auto minEmptySlots = Capacity() - MaxGrowthBudget();
			if (m_Table.m_pData->m_GrowthLeft >= minEmptySlots)
			{
				return;
			}

			// Check if half of the table is occupied by tombstones
			if (m_Table.m_pData->m_Size < (m_Table.m_pData->m_Capacity >> 1))
			{
				// If so, do an in-place rehash
				RehashInPlace();
			}
			else
			{
				// Double the amount
				Resize(m_Table.m_pData->m_Capacity << 1);
			}
		}

		void RehashInPlace()
		{
			auto capacity = m_Table.m_pData->m_Capacity;
			for (auto i = 0; i < capacity; i++)
			{
				SetControlValue(m_Table.m_pCtrls, capacity, i, m_Table.m_pCtrls[i] >= 0 ? kTemporary : kEmpty);
			}

			for (auto i = 0; i < capacity; i++)
			{
				if (m_Table.m_pCtrls[i] != kTemporary)
				{
					continue;
				}

				auto& slot = m_Table.m_pSlots[i];
				value_type tmp(std::move(slot));
				slot.~value_type();
				SetControlValue(m_Table.m_pCtrls, capacity, i, kEmpty);

				auto hash = hasher()(key_traits::Get(tmp));
				auto [h1, h2] = SplitHash(hash);

				GroupWalker gw(capacity, h1);
				while (true)
				{
					GroupMatcher gm(m_Table.m_pCtrls + gw.SlotIndex());
					if (auto matches = gm.MatchEmpty())
					{
						const auto bit = std::countr_zero(matches);
						const auto index = gw.SlotIndex(bit);

						new (std::addressof(m_Table.m_pSlots[index])) value_type(std::move(tmp));

						SetControlValue(m_Table.m_pCtrls, capacity, index, h2);

						break;
					}

					gw.Next();
				}
			}

			m_Table.m_pData->m_GrowthLeft = m_Table.m_pData->m_Capacity - m_Table.m_pData->m_Size;
			m_Table.m_pData->m_Deleted = 0;
		}

		template<typename KV>
		static std::pair<iterator, bool> InsertInternal(Table& table, KV&& keyValue)
		{
			FindResult findResult = FindInsertIndex(table, key_traits::Get(keyValue));
			if (findResult.m_Found)
			{
				return { iterator{ table.m_pCtrls + findResult.m_Index, table.m_pSlots + findResult.m_Index }, false };
			}

			BV_ASSERT(table.m_pData->m_GrowthLeft > 0, "Table can't be full");

			// Create object
			new (std::addressof(table.m_pSlots[findResult.m_Index])) value_type(std::forward<KV>(keyValue));

			// Set control value
			SetControlValue(table.m_pCtrls, table.m_pData->m_Capacity, findResult.m_Index, findResult.m_H2);

			// Update table stats
			table.m_pData->m_Size++;
			table.m_pData->m_GrowthLeft--;

			return { iterator{ table.m_pCtrls + findResult.m_Index, table.m_pSlots + findResult.m_Index }, true };
		}

		void EraseInternal(size_type index)
		{
			// Destroy object
			m_Table.m_pSlots[index].~value_type();

			// Set control value
			SetControlValue(m_Table.m_pCtrls, m_Table.m_pData->m_Capacity, index, kDeleted);

			// Update table stats
			m_Table.m_pData->m_Deleted++;
			m_Table.m_pData->m_Size--;
		}

		template<typename NK>
		size_t FindIndex(const NK& key) const
		{
			auto hash = hasher()(key);
			auto [h1, h2] = SplitHash(hash);

			GroupWalker gw(m_Table.m_pData->m_Capacity, h1);

			while (true)
			{
				GroupMatcher gm(m_Table.m_pCtrls + gw.SlotIndex());

				auto matches = gm.Match(h2);
				while (matches)
				{
					const auto bit = std::countr_zero(matches);
					const auto index = gw.SlotIndex(bit);

					if (key_equal{}(key_traits::Get(m_Table.m_pSlots[index]), key))
					{
						return index;
					}

					matches &= matches - 1;
				}

				if (gm.MatchEmpty())
				{
					return m_Table.m_pData->m_Capacity;
				}

				gw.Next();
			}
		}

		template<typename NK>
		static FindResult FindInsertIndex(const Table& table, const NK& key)
		{
			auto hash = hasher()(key);
			auto [h1, h2] = SplitHash(hash);

			GroupWalker gw(table.m_pData->m_Capacity, h1);

			while (true)
			{
				GroupMatcher gm(table.m_pCtrls + gw.SlotIndex());

				auto matches = gm.Match(h2);
				while (matches)
				{
					const auto bit = std::countr_zero(matches);
					const auto index = gw.SlotIndex(bit);

					if (key_equal{}(key_traits::Get(table.m_pSlots[index]), key))
					{
						return { index, h2, true };
					}

					matches &= matches - 1;
				}

				auto emptyMatches = gm.MatchEmpty();
				if (emptyMatches)
				{
					return { gw.SlotIndex(std::countr_zero(emptyMatches)), h2, false };
				}

				gw.Next();
			}
		}

		static iterator GetIterator(Table& table, size_t index) { return iterator(table.m_pCtrls + index, table.m_pSlots + index, {}); }

	protected:
		IBvMemoryArena* m_pArena = BV_DEFAULT_MEMORY_ARENA;
		Table m_Table{};
	};
}



template<typename K, typename V, typename H = BvHash<K>, typename C = std::equal_to<K>>
class BvFlatHashMap : public HashTableDetail::BvFlatHashTable<K, V, H, C>
{
	using base_type = HashTableDetail::BvFlatHashTable<K, V, H, C>;

public:
	using key_type = typename base_type::key_type;
	using mapped_type = typename base_type::mapped_type;
	using value_type = typename base_type::value_type;
	using size_type = typename base_type::size_type;
	using difference_type = typename base_type::difference_type;
	using hasher = typename base_type::hasher;
	using key_equal = typename base_type::key_equal;
	using reference = typename base_type::reference;
	using const_reference = typename base_type::const_reference;
	using iterator = typename base_type::iterator;
	using const_iterator = typename base_type::const_iterator;
	using key_traits = HashTableDetail::KeyTraits<value_type>;

public:
	BvFlatHashMap()
		: base_type()
	{
	}

	explicit BvFlatHashMap(IBvMemoryArena* pArena)
		: base_type(pArena)
	{
	}

	explicit BvFlatHashMap(size_type size, IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA)
		: base_type(size, pArena)
	{
	}

	explicit BvFlatHashMap(size_type size, const value_type* pElements, IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA)
		: base_type(size, pElements, pArena)
	{
	}

	BvFlatHashMap(std::initializer_list<value_type> list, IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA)
		: base_type(list, pArena)
	{
	}

	BvFlatHashMap(const BvFlatHashMap& rhs) = default;
	BvFlatHashMap(BvFlatHashMap&& rhs) noexcept = default;
	BvFlatHashMap& operator=(const BvFlatHashMap& rhs) = default;
	BvFlatHashMap& operator=(BvFlatHashMap&& rhs) noexcept = default;

	BvFlatHashMap& operator=(std::initializer_list<value_type> list) // Copy Assignment
	{
		base_type::operator=(list);

		return *this;
	}

	~BvFlatHashMap() = default;

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
		requires (!std::same_as<std::remove_cvref_t<NK>, key_type>&& std::constructible_from<key_type, NK>)
	std::pair<iterator, bool> InsertOrAssign(NK&& key, NV&& value)
	{
		auto result = Emplace(std::forward<NK>(key), std::forward<NV>(value));
		if (!result.second)
		{
			result.first->second = std::forward<NV>(value);
		}

		return result;
	}

	template<typename... Args>
	std::pair<iterator, bool> Emplace(const key_type& key, Args&&... args)
	{
		this->PrepareForInsert();

		return EmplaceInternal(this->m_Table, key, std::forward<Args>(args)...);
	}

	template<typename... Args>
	std::pair<iterator, bool> Emplace(key_type&& key, Args&&... args)
	{
		this->PrepareForInsert();

		return EmplaceInternal(this->m_Table, std::move(key), std::forward<Args>(args)...);
	}

	template<typename NK, typename... Args>
		requires (!std::same_as<std::remove_cvref_t<NK>, key_type>&& std::constructible_from<key_type, NK>)
	std::pair<iterator, bool> Emplace(NK&& key, Args&&... args)
	{
		this->PrepareForInsert();

		return EmplaceInternal(this->m_Table, std::forward<NK>(key), std::forward<Args>(args)...);
	}

	mapped_type& operator[](const key_type& key)
	{
		auto result = Emplace(key);
		return result.first->second;
	}

	mapped_type& operator[](key_type&& key)
	{
		auto result = Emplace(std::move(key));
		return result.first->second;
	}

	template<typename Q>
		requires (!std::same_as<std::remove_cvref_t<Q>, key_type>&& std::constructible_from<key_type, Q>)
	mapped_type& operator[](Q&& key)
	{
		auto result = Emplace(std::forward<Q>(key));
		return result.first->second;
	}

	template<typename NV>
	std::pair<iterator, bool> insert_or_assign(const key_type& key, NV&& value) { return InsertOrAssign(key, std::forward<NV>(value)); }
	template<typename NV>
	std::pair<iterator, bool> insert_or_assign(key_type&& key, NV&& value) { return InsertOrAssign(std::forward<key_type>(key), std::forward<NV>(value)); }
	template<typename NK, typename NV>
		requires (!std::same_as<std::remove_cvref_t<NK>, key_type>&& std::constructible_from<key_type, NK>)
	std::pair<iterator, bool> insert_or_assign(NK&& key, NV&& value) { return InsertOrAssign(std::forward<NK>(key), std::forward<NV>(value)); }
	template<typename... Args>
	std::pair<iterator, bool> emplace(const key_type& key, Args&&... args) { return Emplace(key, std::forward<Args>(args)...); }
	template<typename... Args>
	std::pair<iterator, bool> emplace(key_type&& key, Args&&... args) { return Emplace(std::move(key), std::forward<Args>(args)...); }
	template<typename NK, typename... Args>
		requires (!std::same_as<std::remove_cvref_t<NK>, key_type>&& std::constructible_from<key_type, NK>)
	std::pair<iterator, bool> emplace(NK&& key, Args&&... args) { return Emplace(std::forward<NK>(key), std::forward<Args>(args)...); }

private:
	template<typename Q, typename... Args>
	static std::pair<iterator, bool> EmplaceInternal(base_type::Table& table, Q&& key, Args&&... args)
	{
		HashTableDetail::FindResult findResult = base_type::FindInsertIndex(table, key);
		if (findResult.m_Found)
		{
			return { base_type::GetIterator(table, findResult.m_Index), false };
		}

		BV_ASSERT(table.m_pData->m_GrowthLeft > 0, "Table can't be full");

		// Create object
		new (std::addressof(table.m_pSlots[findResult.m_Index])) value_type(std::piecewise_construct,
			std::forward_as_tuple(std::forward<Q>(key)), std::forward_as_tuple(std::forward<Args>(args)...));

		// Set control value
		base_type::SetControlValue(table.m_pCtrls, table.m_pData->m_Capacity, findResult.m_Index, findResult.m_H2);

		// Update table stats
		table.m_pData->m_Size++;
		table.m_pData->m_GrowthLeft--;

		return { base_type::GetIterator(table, findResult.m_Index), true };
	}
};



template<typename K, typename H = BvHash<K>, typename C = std::equal_to<K>>
class BvFlatHashSet : public HashTableDetail::BvFlatHashTable<K, void, H, C>
{
	using base_type = HashTableDetail::BvFlatHashTable<K, void, H, C>;

public:
	using key_type = typename base_type::key_type;
	using value_type = typename base_type::value_type;
	using size_type = typename base_type::size_type;
	using difference_type = typename base_type::difference_type;
	using hasher = typename base_type::hasher;
	using key_equal = typename base_type::key_equal;
	using reference = typename base_type::reference;
	using const_reference = typename base_type::const_reference;
	using iterator = typename base_type::iterator;
	using const_iterator = typename base_type::const_iterator;

public:
	BvFlatHashSet()
		: base_type()
	{}

	explicit BvFlatHashSet(IBvMemoryArena* pArena)
		: base_type(pArena)
	{}

	explicit BvFlatHashSet(size_type size, IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA)
		: base_type(size, pArena)
	{}

	explicit BvFlatHashSet(size_type size, const value_type* pElements, IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA)
		: base_type(size, pElements, pArena)
	{}

	BvFlatHashSet(std::initializer_list<value_type> list, IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA)
		: base_type(list, pArena)
	{}

	BvFlatHashSet(const BvFlatHashSet& rhs) = default;
	BvFlatHashSet(BvFlatHashSet&& rhs) noexcept = default;
	BvFlatHashSet& operator=(const BvFlatHashSet& rhs) = default;
	BvFlatHashSet& operator=(BvFlatHashSet&& rhs) noexcept = default;

	BvFlatHashSet& operator=(std::initializer_list<value_type> list) // Copy Assignment
	{
		base_type::operator=(list);

		return *this;
	}

	~BvFlatHashSet() = default;
};