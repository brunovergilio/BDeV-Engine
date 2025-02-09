#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Container/BvIterator.h"
#include "BDeV/Core/Utils/BvHash.h"
#include "BDeV/Core/System/Memory/BvMemory.h"
#include "BDeV/Core/System/Memory/BvMemoryArena.h"
#include <algorithm>
#include <utility>


template<typename Key, typename Value, typename MemoryArenaType = IBvMemoryArena, typename Hash = std::hash<Key>, typename Comparer = std::equal_to<Key>>
class BvRobinMap
{
public:
	using KeyValue = std::pair<Key, Value>;

	using Iterator = RobinIterator<KeyValue>;
	using ConstIterator = RobinIterator<KeyValue, true>;

	BvRobinMap(); // Default
	explicit BvRobinMap(MemoryArenaType* pAllocator);
	explicit BvRobinMap(const size_t capacity, MemoryArenaType* pAllocator = nullptr); // Reserve
	BvRobinMap(std::initializer_list<KeyValue> list, MemoryArenaType* pAllocator = nullptr); // Initializer List
	BvRobinMap(const BvRobinMap& rhs); // Copy
	BvRobinMap(BvRobinMap&& rhs) noexcept; // Move
	
	BvRobinMap& operator =(const BvRobinMap& rhs); // Copy Assignment
	BvRobinMap& operator =(BvRobinMap&& rhs) noexcept; // Move Assignment
	BvRobinMap& operator =(std::initializer_list<KeyValue> list); // Initializer List Assignment

	~BvRobinMap();

	// Allocator
	MemoryArenaType* GetAllocator() const;
	void SetAllocator(MemoryArenaType* pAllocator);

	// Iterator
	Iterator begin() { return Iterator(m_pData, m_pData, m_pHashes, &m_Capacity); }
	Iterator end() { return Iterator(m_pData, m_pData + m_Capacity, m_pHashes + m_Capacity, &m_Capacity); }
	const ConstIterator begin() const { return ConstIterator(m_pData, m_pData, m_pHashes, &m_Capacity); }
	const ConstIterator end() const { return ConstIterator(m_pData, m_pData + m_Capacity, m_pHashes + m_Capacity, &m_Capacity); }
	ConstIterator cbegin() const { return begin(); }
	ConstIterator cend() const { return end(); }

	void ResizeAndRehash(const size_t size);

	template<typename ...Args>
	std::pair<typename BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::Iterator, bool> Emplace(Key && key, Args &&... args);
	template<typename ...Args>
	std::pair<typename BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::Iterator, bool> Emplace(const Key & key, Args &&... args);
	template<typename ...Args>
	std::pair<typename BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::Iterator, bool> EmplaceOrAssign(Key&& key, Args &&... args);
	template<typename ...Args>
	std::pair<typename BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::Iterator, bool> EmplaceOrAssign(const Key & key, Args &&... args);
	bool Erase(const Key & key);

	Iterator FindKey(const Key & key) const;
	bool HasKey(const Key& key) const;

	void Clear();

	Value & operator[] (const Key & key);
	Value & operator[] (Key && key);

	Value & At(const Key & key);
	const Value & At(const Key & key) const;

	BV_INLINE const size_t Size() const { return m_Size; }
	BV_INLINE const size_t Capacity() const { return m_Capacity; }
	BV_INLINE const bool Empty() const { return m_Size == 0; }

private:
	template<typename KeyType, typename ...Args>
	size_t EmplaceInternal(KeyValue* const pData, size_t* const pHashes, KeyType&& key, Args&&... args);
	void Destroy();
	BV_INLINE const size_t HashPos(const u32 hash) const { return hash % m_Capacity; };
	BV_INLINE const size_t Hash(const Key & key) const;

private:
	KeyValue * m_pData = nullptr;
	size_t * m_pHashes = nullptr;
	MemoryArenaType* m_pAllocator = nullptr;
	size_t m_Size = 0;
	size_t m_Capacity = 0;
};


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
inline BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::BvRobinMap()
{
}


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
inline BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::BvRobinMap(MemoryArenaType* pAllocator)
	: m_pAllocator(pAllocator)
{
}


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
inline BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::BvRobinMap(const size_t capacity, MemoryArenaType* pAllocator)
	: m_pAllocator(pAllocator)
{
	ResizeAndRehash(capacity);
}


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
inline BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::BvRobinMap(std::initializer_list<KeyValue> list, MemoryArenaType* pAllocator)
	: m_pAllocator(pAllocator)
{
	ResizeAndRehash(list.size());

	for (auto it = list.begin(); it != list.end(); it++)
	{
		Emplace(it->first, it->second);
	}
}


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
inline BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::BvRobinMap(const BvRobinMap& rhs)
{
	ResizeAndRehash(rhs.m_Capacity);

	for (auto it = rhs.cbegin(); it != rhs.cend(); it++)
	{
		Emplace(it->first, it->second);
	}
}


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
inline BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::BvRobinMap(BvRobinMap&& rhs) noexcept
{
	*this = std::move(rhs);
}


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
inline BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>& BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::operator=(const BvRobinMap& rhs)
{
	if (this->m_pHashes != rhs.m_pHashes)
	{
		Destroy();
		SetAllocator(rhs.m_pAllocator);
		ResizeAndRehash(rhs.m_Capacity);

		for (auto it = rhs.cbegin(); it != rhs.cend(); it++)
		{
			Emplace(it->first, it->second);
		}
	}

	return *this;
}


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
inline BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>& BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::operator=(BvRobinMap&& rhs) noexcept
{
	if (this->m_pHashes != rhs.m_pHashes)
	{
		std::swap(m_pData, rhs.m_pData);
		std::swap(m_pHashes, rhs.m_pHashes);
		std::swap(m_pAllocator, rhs.m_pAllocator);
		std::swap(m_Size, rhs.m_Size);
		std::swap(m_Capacity, rhs.m_Capacity);
	}

	return *this;
}


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
inline BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>& BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::operator=(std::initializer_list<KeyValue> list)
{
	Clear();

	ResizeAndRehash(list.size());

	for (auto it = list.begin(); it != list.end(); it++)
	{
		Emplace(it->first, it->second);
	}
}


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
inline BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::~BvRobinMap()
{
	Destroy();
}


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
inline MemoryArenaType* BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::GetAllocator() const
{
	return m_pAllocator;
}


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
inline void BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::SetAllocator(MemoryArenaType* pAllocator)
{
	if (m_pAllocator == pAllocator)
	{
		return;
	}

	if (m_Capacity > 0)
	{
		KeyValue* pNewData = pAllocator ? BV_MNEW_ARRAY(*pAllocator, KeyValue, m_Capacity) : BV_NEW_ARRAY(KeyValue, m_Capacity);
		size_t* pNewHashes = pAllocator ? BV_MNEW_ARRAY(*pAllocator, size_t, m_Capacity) : BV_NEW_ARRAY(size_t, m_Capacity);
		memset(pNewHashes, 0, sizeof(size_t) * m_Capacity);
		if (m_Size > 0)
		{
			for (size_t i = 0; i < m_Capacity; i++)
			{
				if (m_pHashes[i] != 0)
				{
					// Because we're using the same capacity, the elements will hash to the same positions,
					// therefore there's no need to call EmplaceInternal()
					new (&pNewData[i]) std::pair<Key, Value>(std::move(m_pData[i].first), std::move(m_pData[i].second));
					pNewHashes[i] = m_pHashes[i];
				}
			}
		}
		Clear();
		m_pAllocator ? BV_MDELETE_ARRAY(*m_pAllocator, m_pData) : BV_DELETE_ARRAY(m_pData);
		m_pAllocator ? BV_MDELETE_ARRAY(*m_pAllocator, m_pHashes) : BV_DELETE_ARRAY(m_pHashes);

		m_pData = pNewData;
		m_pHashes = pNewHashes;
	}

	m_pAllocator = pAllocator;
}


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
inline void BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::ResizeAndRehash(const size_t size)
{
	if (size <= m_Capacity)
	{
		return;
	}

	auto oldCapacity = m_Capacity;
	m_Capacity = size;

	KeyValue* pNewData = m_pAllocator ? BV_MNEW_ARRAY(*m_pAllocator, KeyValue, m_Capacity) : BV_NEW_ARRAY(KeyValue, m_Capacity);
	size_t* pNewHashes = m_pAllocator ? BV_MNEW_ARRAY(*m_pAllocator, size_t, m_Capacity) : BV_NEW_ARRAY(size_t, m_Capacity);
	memset(pNewHashes, 0, sizeof(size_t) * m_Capacity);

	for (size_t i = 0; i < oldCapacity; i++)
	{
		if (m_pHashes[i] != 0)
		{
			EmplaceInternal(pNewData, pNewHashes, std::move(m_pData[i].first), std::move(m_pData[i].second));
		}
	}

	if (m_pData)
	{
		m_pAllocator ? BV_MDELETE_ARRAY(*m_pAllocator, m_pData) : BV_DELETE_ARRAY(m_pData);
		m_pAllocator ? BV_MDELETE_ARRAY(*m_pAllocator, m_pHashes) : BV_DELETE_ARRAY(m_pHashes);
	}

	m_pData = pNewData;
	m_pHashes = pNewHashes;
}


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
template<typename... Args>
inline std::pair<typename BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::Iterator, bool> BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::Emplace(Key && key, Args &&... args)
{
	if (m_Size == m_Capacity)
	{
		ResizeAndRehash(CalculateNewContainerSize(m_Capacity));
	}

	auto iter = FindKey(key);
	if (iter != cend())
	{
		return std::make_pair(iter, false);
	}

	auto index = EmplaceInternal(m_pData, m_pHashes, std::forward<Key>(key), std::forward<Args>(args)...);
	++m_Size;

	return std::make_pair(Iterator(m_pData, m_pData + index, m_pHashes + index, &m_Size), true);
}


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
template<typename... Args>
inline std::pair<typename BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::Iterator, bool> BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::Emplace(const Key & key, Args &&... args)
{
	if (m_Size == m_Capacity)
	{
		ResizeAndRehash(CalculateNewContainerSize(m_Capacity));
	}

	auto iter = FindKey(key);
	if (iter != cend())
	{
		return std::make_pair(iter, false);
	}

	auto index = EmplaceInternal(m_pData, m_pHashes, key, std::forward<Args>(args)...);
	++m_Size;

	return std::make_pair(Iterator(m_pData, m_pData + index, m_pHashes + index, &m_Size), true);
}


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
template<typename ...Args>
inline std::pair<typename BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::Iterator, bool> BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::EmplaceOrAssign(Key&& key, Args &&... args)
{
	auto iter = FindKey(key);
	if (iter != cend())
	{
		auto index = iter.GetIndex();
		if constexpr (!std::is_trivially_destructible_v<Value>)
		{
			m_pData[index].second.~Value();
		}
		new (&m_pData[index].second) Value(std::forward<Args>(args)...);
		return std::make_pair(Iterator(m_pData, m_pData + index, m_pHashes + index, &m_Size), false);
	}

	if (m_Size == m_Capacity)
	{
		ResizeAndRehash(CalculateNewContainerSize(m_Capacity));
	}

	auto index = EmplaceInternal(m_pData, m_pHashes, std::forward<Key>(key), std::forward<Args>(args)...);
	++m_Size;

	return std::make_pair(Iterator(m_pData, m_pData + index, m_pHashes + index, &m_Size), true);
}


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
template<typename ...Args>
inline std::pair<typename BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::Iterator, bool> BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::EmplaceOrAssign(const Key & key, Args &&... args)
{
	auto iter = FindKey(key);
	if (iter != cend())
	{
		auto index = iter.GetIndex();
		if constexpr (!std::is_trivially_destructible_v<Value>)
		{
			m_pData[index].second.~Value();
		}
		new (&m_pData[index].second) Value(std::forward<Args>(args)...);
		return std::make_pair(Iterator(m_pData, m_pData + index, m_pHashes + index, &m_Size), false);
	}

	if (m_Size == m_Capacity)
	{
		ResizeAndRehash(CalculateNewContainerSize(m_Capacity));
	}

	auto index = EmplaceInternal(m_pData, m_pHashes, key, std::forward<Args>(args)...);
	++m_Size;

	return std::make_pair(Iterator(m_pData, m_pData + index, m_pHashes + index, &m_Size), true);
}


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
inline bool BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::Erase(const Key & key)
{
	auto iter = FindKey(key);
	if (iter == cend())
	{
		return false;
	}

	auto currIndex = iter.GetIndex();
	if constexpr (!std::is_trivially_destructible_v<Key>)
	{
		m_pData[currIndex].first.~Key();
	}
	if constexpr (!std::is_trivially_destructible_v<Value>)
	{
		m_pData[currIndex].second.~Value();
	}
	m_pHashes[currIndex] = 0;
	--m_Size;

	for (;;)
	{
		// We start from the current index
		auto shiftIndex = (currIndex + 1) % m_Capacity;
		// If there's no hash value, we're done
		if (m_pHashes[shiftIndex] == 0)
		{
			break;
		}

		size_t desiredPos = HashPos(m_pHashes[shiftIndex]);
		// If the current element's desired position is this one, we're done
		if (desiredPos == shiftIndex)
		{
			break;
		}
		else
		{
			// Otherwise we swap the elements and keep going
			std::swap(m_pData[currIndex], m_pData[shiftIndex]);
			std::swap(m_pHashes[currIndex], m_pHashes[shiftIndex]);
		}

		currIndex = shiftIndex;
	}

	return true;
}


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
RobinIterator<std::pair<Key, Value>, false> BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::FindKey(const Key & key) const
{
	if (m_Capacity == 0)
	{
		return Iterator(m_pData, m_pData + m_Capacity, m_pHashes + m_Capacity, &m_Capacity);
	}

	size_t hash = Hash(key);
	size_t pos = HashPos(hash);
	size_t dist = 0;

	for (; dist < m_Capacity;)
	{
		// If there isn't an element with that key, return the end of the map
		if (m_pHashes[pos] == 0)
		{
			return Iterator(m_pData, m_pData + m_Capacity, m_pHashes + m_Capacity, &m_Capacity);
		}

		// If there's already an element with that key, return true and the index where it's stored
		if (m_pHashes[pos] == hash && Comparer()(m_pData[pos].first, key))
		{
			return Iterator(m_pData, m_pData + pos, m_pHashes + pos, &m_Capacity);
		}

		size_t currElemDesiredPos = HashPos(m_pHashes[pos]);
		size_t currElemDist = (pos - currElemDesiredPos + m_Capacity) % m_Capacity; // Add capacity to make insertion / look-up circular
		// If there isn't an element with that key, return the end of the map
		if (dist > currElemDist)
		{
			return Iterator(m_pData, m_pData + m_Capacity, m_pHashes + m_Capacity, &m_Capacity);
		}

		pos = (pos + 1) % m_Capacity;
		dist++;
	}

	return Iterator(m_pData, m_pData + m_Capacity, m_pHashes + m_Capacity, &m_Capacity);
}


 template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
 bool BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::HasKey(const Key& key) const
 {
	 return FindKey(key) != cend();
 }


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
void BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::Clear()
{
	for (size_t i = 0; i < m_Capacity; i++)
	{
		if (m_pHashes[i])
		{
			if constexpr (!std::is_trivially_destructible_v<Key>)
			{
				m_pData[i].first.~Key();
			}
			if constexpr (!std::is_trivially_destructible_v<Value>)
			{
				m_pData[i].second.~Value();
			}
			m_pHashes[i] = 0;
		}
	}

	m_Size = 0;
}


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
inline Value & BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::operator[](const Key & key)
{
	auto iter = FindKey(key);
	if (iter == end())
	{
		auto result = Emplace(key, Value());
		return m_pData[result.first.GetIndex()].second;
	}

	return m_pData[iter.GetIndex()].second;
}


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
inline Value & BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::operator[](Key && key)
{
	auto iter = FindKey(key);
	if (iter == cend())
	{
		auto result = Emplace(std::forward<Key>(key), Value());
		return m_pData[result.first.GetIndex()].second;
	}

	return m_pData[iter.GetIndex()].second;
}


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
inline Value & BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::At(const Key & key)
{
	auto iter = FindKey(key);
	BV_ASSERT(iter != cend(), "Key not found");

	return m_pData[iter.GetIndex()].second;
}


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
inline const Value & BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::At(const Key & key) const
{
	auto iter = FindKey(key);
	BV_ASSERT(iter != cend(), "Key not found");

	return m_pData[iter.GetIndex()].second;
}


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
template<typename KeyType, typename ...Args>
inline size_t BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::EmplaceInternal(KeyValue* const pData, size_t* const pHashes, KeyType&& key, Args && ...args)
{
	size_t hash = Hash(key);
	size_t currPos = HashPos(hash);
	size_t distFromDesiredPos = 0;
	size_t newElemInsertedPosition = kU64Max;

	KeyValue newItem(std::piecewise_construct, std::forward_as_tuple(std::forward<KeyType>(key)), std::forward_as_tuple(std::forward<Args>(args)...));

	for (; distFromDesiredPos < m_Capacity; distFromDesiredPos++, currPos = (currPos + 1) % m_Capacity)
	{
		if (pHashes[currPos] == 0)
		{
			new (&pData[currPos]) KeyValue(std::move(newItem));
			pHashes[currPos] = hash;

			if (newElemInsertedPosition == kU64Max)
			{
				newElemInsertedPosition = currPos;
			}

			break;
		}

		size_t currElemDesiredPos = HashPos(pHashes[currPos]);
		size_t currElemDist = (currPos - currElemDesiredPos + m_Capacity) % m_Capacity; // Add capacity to make insertion / look-up circular
		if (distFromDesiredPos > currElemDist)
		{
			std::swap(pData[currPos], newItem);
			std::swap(pHashes[currPos], hash);

			if (newElemInsertedPosition == kU64Max)
			{
				newElemInsertedPosition = currPos;
			}

			distFromDesiredPos = currElemDist;
		}
	}

	return newElemInsertedPosition;
}


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
void BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::Destroy()
{
	Clear();

	m_Capacity = 0;

	if (m_pData)
	{
		m_pAllocator ? BV_MDELETE_ARRAY(*m_pAllocator, m_pData) : BV_DELETE_ARRAY(m_pData);
		m_pAllocator ? BV_MDELETE_ARRAY(*m_pAllocator, m_pHashes) : BV_DELETE_ARRAY(m_pHashes);
		m_pData = nullptr;
		m_pHashes = nullptr;
	}
}


template<typename Key, typename Value, typename MemoryArenaType, typename Hash, typename Comparer>
inline const size_t BvRobinMap<Key, Value, MemoryArenaType, Hash, Comparer>::Hash(const Key & key) const
{
	size_t hash = MurmurHash64A(&key, sizeof(Key));

	return hash != 0 ? hash : 1;
}