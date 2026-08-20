#pragma once


// BvFlatHashMap
// Swiss table implementation, based on Google's abseil (https://github.com/abseil/abseil-cpp/blob/master/absl/container/flat_hash_map.h)


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BDeV/Core/System/Memory/BvMemory.h"
#include "BDeV/Core/Utils/BvHash.h"
#include <iterator>


namespace Internal
{
	constexpr i8 kEmpty = -128; // 0x80
	constexpr i8 kDeleted = -2; // 0xFE
	constexpr i8 kSentinel = -1; // 0xFF
	constexpr i8 kTemporary = -3; // 0xFD

	struct HashParts
	{
		size_t m_H1;
		i8 m_H2;
	};

	constexpr i8 kFingerprintMask = 0x7F;
	inline HashParts SplitHash(size_t hash)
	{
		return { hash >> 7, static_cast<i8>(hash & kFingerprintMask) };
	}

#if BV_HAS_SSE2
	constexpr size_t kGroupSize = 16;
	
	class GroupMatcher
	{
	public:
		explicit GroupMatcher(const i8* pCtrl)
			: m_Ctrl(_mm_loadu_si128(reinterpret_cast<const __m128i*>(pCtrl)))
		{
		}

		u16 Match(i8 h2) const
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

		// Matches ANY writable slot (either truly empty OR a tombstone)
		u16 MatchWritable() const
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

	// Backing memory layout
	struct HashTableData
	{
		size_t m_Capacity; // Maximum size
		size_t m_Size; // Current size
		size_t m_Deleted; // How many tombstones
		size_t m_GrowthLeft; // How much more can be added before a resize and rehash is needed => capacity * max_load - (size + deleted)
		// i8 m_Ctrls[m_Capacity] - controls
		// i8 m_Sentinel - for iterators to know the end (always kSentinel)
		// i8 m_CtrlClones[kGroupSize - 1]; // To avoid overflow on SIMD types
		// padding bytes for slot type alignment
		// Slots[m_Capacity]
	};
}


template<typename K, typename V, typename H = BvHash<K>, typename C = std::equal_to<K>>
class BvFlatHashMap
{
public:
	using key_type = K;
	using mapped_type = V;
	using value_type = std::pair<const key_type, mapped_type>;
	using key_compare = C;
	//using reference = std::pair<const key_type&, mapped_type&>; // TODO: Figure these out
	//using const_reference = std::pair<const key_type&, const mapped_type&>;
	using size_type = size_t;
	using difference_type = ptrdiff_t;
	using control_type = i8;

	class Iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using iterator_concept = std::forward_iterator_tag;

		using value_type = std::pair<const key_type, mapped_type>;
		using difference_type = std::ptrdiff_t;
		using pointer = value_type*;
		using reference = value_type&;

		Iterator(control_type* pCtrls, value_type* pSlots)
			: m_pCtrls(pCtrls), m_pSlots(pSlots)
		{
		}

		reference operator*() const
		{
			return m_pSlots[m_Index];
		}

		pointer operator->() const
		{
			return std::addressof(m_pSlots[m_Index]);
		}

		Iterator& operator++()
		{
			Advance();
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
		void Advance()
		{
			++m_Index;

			while (m_Index < m_Capacity)
			{
				Internal::GroupMatcher group(m_pCtrls + m_Index);

				auto matches = group.MatchFull();

				// Don't allow the group to advance beyond the real table.
				const auto remaining = m_Capacity - m_Index;

				if (remaining < Internal::kGroupSize)
				{
					matches &= static_cast<u16>(
						(u32{ 1 } << remaining) - 1);
				}

				if (matches)
				{
					m_Index += std::countr_zero(matches);
					return;
				}

				m_Index += Internal::kGroupSize;
			}
		}

		control_type* m_pCtrls = nullptr;
		value_type* m_pSlots = nullptr;
		size_type m_Index = 0;
	};

private:
	struct Table
	{
		Internal::HashTableData* m_pData;
		control_type* m_pCtrls;
		value_type* m_pSlots;
	};

	struct FindResult
	{
		size_t m_Index;
		i8 m_H2;
		bool m_Found;
		//bool m_WasTombstone;
	};

	class GroupWalker
	{
	public:
		explicit GroupWalker(size_t capacity, size_t h1)
			: m_Capacity(capacity), m_Offset(h1 & (m_Capacity - 1)) {}

		void Next() { m_Offset = (m_Offset + Internal::kGroupSize) & (m_Capacity - 1); }

		template<std::integral T> size_t SlotIndex(T value) const
		{
			auto newOffset = m_Offset + value;

			BV_ASSERT(newOffset != m_Capacity, "This should never happen!");

			return (newOffset - size_t(newOffset > m_Capacity) * (m_Capacity + 1)) & (m_Capacity - 1);
		}
		size_t SlotIndex() const { return m_Offset; }

	private:
		size_t m_Capacity;
		size_t m_Offset;
	};

public:
	BvFlatHashMap()
		: m_pArena(BV_DEFAULT_MEMORY_ARENA)
	{}

	explicit BvFlatHashMap(IBvMemoryArena* pArena)
		: m_pArena(pArena)
	{}

	explicit BvFlatHashMap(size_type size, IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA)
		: m_pArena(pArena)
	{
		Resize(size);
	}

	explicit BvFlatHashMap(size_type size, const value_type* pElements, IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA)
		: m_pArena(pArena)
	{
		Resize(IdealCapacity(size));
		for (auto i = 0; i < size; i++)
		{
			// TODO: Insert elements
		}
	}

	BvFlatHashMap(std::initializer_list<value_type> list, IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA)
		: m_pArena(pArena)
	{
		auto size = list.size();
		Resize(IdealCapacity(size));
		for (auto i = 0; i < size; i++)
		{
			// TODO: Add elements
		}
	}

	BvFlatHashMap(const BvFlatHashMap& rhs)
		: m_pArena(rhs.m_pArena)
	{
		if (auto pData = rhs.m_Table.m_pData)
		{
			Resize(pData->m_Capacity);
			for (auto i = 0; i < pData->m_Capacity; i++)
			{
				// TODO: Add elements
			}
		}
	}

	BvFlatHashMap(BvFlatHashMap&& rhs) noexcept
		: m_pArena(rhs.m_pArena), m_Table(rhs.m_Table)
	{
		rhs.m_Table = {};
	}

	BvFlatHashMap& operator=(const BvFlatHashMap& rhs) // Copy Assignment
	{
		if (this != &rhs)
		{
			Clear(true);
			//SetAllocator(rhs.m_pArena);
			if (auto pData = rhs.m_Table.m_pData)
			{
				Resize(pData->m_Capacity);
				for (auto i = 0; i < pData->m_Capacity; i++)
				{
					// TODO: Add elements
				}
			}
		}

		return *this;
	}

	BvFlatHashMap& operator=(BvFlatHashMap&& rhs) noexcept // Move Assignment
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

	BvFlatHashMap& operator=(std::initializer_list<value_type> list) // Copy Assignment
	{
		Clear();

		auto size = list.size();
		Resize(IdealCapacity(size));
		for (auto i = 0; i < size; i++)
		{
			// TODO: Add elements
		}

		return *this;
	}

	~BvFlatHashMap()
	{
		Clear(true);
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
			std::fill_n(m_Table.m_pCtrls, m_Table.m_pData->m_Capacity + Internal::kGroupSize, Internal::kEmpty);
			m_Table.m_pCtrls[m_Table.m_pData->m_Capacity] = Internal::kSentinel;
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
			for (auto i = 0; i < m_Table.m_pData->m_Capacity; i++)
			{
				if (m_Table.m_pCtrls[i] >= 0)
				{
					EmplaceInternal(table, std::move(m_Table.m_pSlots[i].first), std::move(m_Table.m_pSlots[i].second));
				}
			}

			Clear(true);
		}

		m_Table = table;
	}

	// TODO: Return std::pair<iterator, bool> in the future
	template<typename... Args>
	std::pair<value_type*, bool> Emplace(const key_type& key, Args&&... args)
	{
		PrepareForInsert();

		return EmplaceInternal(m_Table, key, std::forward<Args>(args)...);
	}

	// TODO: Return std::pair<iterator, bool> in the future
	//std::pair<value_type*, bool> Insert(const value_type& keyValue)
	//{
	//	PrepareForInsert();
	//}

	// TODO: Return std::pair<iterator, bool> in the future
	//std::pair<value_type*, bool> Insert(value_type&& keyValue)
	//{
	//	PrepareForInsert();
	//}

	bool Erase(const key_type& key)
	{
		auto index = FindIndex(key);
		if (index == kInvalidPos)
		{
			return false;
		}

		// Destroy object
		m_Table.m_pSlots[index].~value_type();

		// Set control value
		SetControlValue(m_Table.m_pCtrls, m_Table.m_pData->m_Capacity, index, Internal::kDeleted);

		// Update table stats
		m_Table.m_pData->m_Deleted++;
		m_Table.m_pData->m_Size--;

		return true;
	}

	// TODO: Return iterator in the future
	value_type* Find(const key_type& key)
	{
		auto index = FindIndex(key);
		if (index == kInvalidPos)
		{
			return nullptr;
		}

		return std::addressof(m_Table.m_pSlots[index]);
	}

	// TODO: Return iterator in the future
	const value_type* Find(const key_type& key) const
	{
		auto index = FindIndex(key);
		if (index == kInvalidPos)
		{
			return nullptr;
		}

		return std::addressof(m_Table.m_pSlots[index]);
	}

private:
	static constexpr size_t GetCtrlOffset()
	{
		return sizeof(Internal::HashTableData);
	}

	static constexpr size_t GetSlotOffset(size_t capacity)
	{
		return RoundToNearestMultipleP2(GetCtrlOffset() + sizeof(control_type) * (capacity + Internal::kGroupSize), alignof(value_type));
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

		auto pNewHashTable = reinterpret_cast<Internal::HashTableData*>(pMem);
		pNewHashTable->m_Capacity = capacity;
		pNewHashTable->m_Size = 0;
		pNewHashTable->m_Deleted = 0;
		pNewHashTable->m_GrowthLeft = capacity;

		auto pNewCtrls = reinterpret_cast<control_type*>(pMem + GetCtrlOffset());
		std::fill_n(pNewCtrls, capacity + Internal::kGroupSize, Internal::kEmpty);
		pNewCtrls[capacity] = Internal::kSentinel;

		auto pNewSlots = reinterpret_cast<value_type*>(pMem + GetSlotOffset(capacity));

		return { pNewHashTable, pNewCtrls, pNewSlots };
	}

	static void SetControlValue(control_type* pCtrls, size_t capacity, size_t index, control_type val)
	{
		pCtrls[index] = val;

		// Mirror if one of the first [Internal::kGroupSize - 1] elements
		if (index < Internal::kGroupSize - 1)
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
			SetControlValue(m_Table.m_pCtrls, capacity, i, m_Table.m_pCtrls[i] >= 0 ? Internal::kTemporary : Internal::kEmpty);
		}

		for (auto i = 0; i < capacity; i++)
		{
			if (m_Table.m_pCtrls[i] != Internal::kTemporary)
			{
				continue;
			}

			auto& slot = m_Table.m_pSlots[i];
			value_type tmp(std::move(slot));
			slot.~value_type();
			SetControlValue(m_Table.m_pCtrls, capacity, i, Internal::kEmpty);

			auto hash = H()(tmp.first);
			auto [h1, h2] = Internal::SplitHash(hash);

			GroupWalker gw(capacity, h1);
			while (true)
			{
				Internal::GroupMatcher gm(m_Table.m_pCtrls + gw.SlotIndex());
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

	template<typename C, typename... Args>
	static std::pair<value_type*, bool> EmplaceInternal(Table& table, C&& key, Args&&... args)
	{
		FindResult findResult = FindInsertIndex(table, key);
		if (findResult.m_Found)
		{
			return { &table.m_pSlots[findResult.m_Index], false };
		}

		BV_ASSERT(table.m_pData->m_GrowthLeft > 0, "Table can't be full");

		// Create object
		new (std::addressof(table.m_pSlots[findResult.m_Index])) value_type(std::piecewise_construct,
			std::forward_as_tuple(std::forward<C>(key)), std::forward_as_tuple(std::forward<Args>(args)...));

		// Set control value
		SetControlValue(table.m_pCtrls, table.m_pData->m_Capacity, findResult.m_Index, findResult.m_H2);

		// Update table stats
		table.m_pData->m_Size++;
		table.m_pData->m_GrowthLeft--;

		return { std::addressof(table.m_pSlots[findResult.m_Index]), true };
	}

	size_t FindIndex(const key_type& key) const
	{
		auto hash = H()(key);
		auto [h1, h2] = Internal::SplitHash(hash);
		
		GroupWalker gw(m_Table.m_pData->m_Capacity, h1);

		while (true)
		{
			Internal::GroupMatcher gm(m_Table.m_pCtrls + gw.SlotIndex());

			auto matches = gm.Match(h2);
			while (matches)
			{
				const auto bit = std::countr_zero(matches);
				const auto index = gw.SlotIndex(bit);

				if (m_Table.m_pSlots[index].first == key)
				{
					return index;
				}

				matches &= matches - 1;
			}

			if (gm.MatchEmpty())
			{
				return kInvalidPos;
			}

			gw.Next();
		}
	}

	static FindResult FindInsertIndex(const Table& table, const key_type& key)
	{
		auto hash = H()(key);
		auto [h1, h2] = Internal::SplitHash(hash);

		GroupWalker gw(table.m_pData->m_Capacity, h1);

		//auto firstDeleted = kInvalidPos;

		while (true)
		{
			Internal::GroupMatcher gm(table.m_pCtrls + gw.SlotIndex());

			auto matches = gm.Match(h2);
			while (matches)
			{
				const auto bit = std::countr_zero(matches);
				const auto index = gw.SlotIndex(bit);

				if (table.m_pSlots[index].first == key)
				{
					return { index, h2, true };
				}

				// Leaving this for the moment
				//auto deletedMatches = gm.MatchDeleted();
				//if (firstDeleted == kInvalidPos && deletedMatches)
				//{
				//	firstDeleted = groupStart + std::countr_zero(deletedMatches);
				//}

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

private:
	IBvMemoryArena* m_pArena = BV_DEFAULT_MEMORY_ARENA;
	Table m_Table{};
};