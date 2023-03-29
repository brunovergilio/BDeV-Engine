#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Container/BvIterator.h"
#include "BDeV/Utils/Hash.h"
#include "BDeV/System/Memory/BvMemoryCommon.h"
#include <algorithm>
#include <utility>


template<typename Key, typename Value, typename Hash = std::hash<Key>, typename Comparer = std::equal_to<Key>>
class BvRobinMap
{
public:
	using KeyValue = std::pair<Key, Value>;

	using Iterator = RobinIterator<KeyValue>;
	using ConstIterator = RobinIterator<KeyValue, true>;

	BvRobinMap(); // Default
	explicit BvRobinMap(const size_t capacity); // Fill
	BvRobinMap(const BvRobinMap& rhs); // Copy
	BvRobinMap(BvRobinMap&& rhs) noexcept; // Move
	BvRobinMap(std::initializer_list<KeyValue> list); // Initializer List
	
	BvRobinMap& operator =(const BvRobinMap& rhs); // Copy Assignment
	BvRobinMap& operator =(BvRobinMap&& rhs) noexcept; // Move Assignment
	BvRobinMap& operator =(std::initializer_list<KeyValue> list); // Initializer List Assignment

	~BvRobinMap();

	// Iterator
	Iterator begin() { return Iterator(m_pData, m_pData, m_pHashes, &m_Capacity); }
	Iterator end() { return Iterator(m_pData, m_pData + m_Capacity, m_pHashes + m_Capacity, &m_Capacity); }
	const ConstIterator begin() const { return ConstIterator(m_pData, m_pData, m_pHashes, &m_Capacity); }
	const ConstIterator end() const { return ConstIterator(m_pData, m_pData + m_Capacity, m_pHashes + m_Capacity, &m_Capacity); }
	ConstIterator cbegin() const { return begin(); }
	ConstIterator cend() const { return end(); }

	void ResizeAndRehash(const size_t size);

	template<typename ...Args>
	std::pair<typename BvRobinMap<Key, Value, Hash, Comparer>::Iterator, bool> Emplace(Key && key, Args &&... args);
	template<typename ...Args>
	std::pair<typename BvRobinMap<Key, Value, Hash, Comparer>::Iterator, bool> Emplace(const Key & key, Args &&... args);
	template<typename ...Args>
	std::pair<typename BvRobinMap<Key, Value, Hash, Comparer>::Iterator, bool> EmplaceOrAssign(const Key & key, Args &&... args);
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
	const size_t InsertInternal(KeyType && key, Args &&... args);
	const size_t MoveObject(KeyValue * const pKeyValue, size_t hash, KeyValue * const pData, size_t * const pHashes);
	void Destroy();
	BV_INLINE const size_t HashPos(const u32 hash) const { return (m_Capacity - 1) & hash; };
	BV_INLINE const size_t Hash(const Key & key) const;

private:
	KeyValue * m_pData = nullptr;
	size_t * m_pHashes = nullptr;
	size_t m_Size = 0;
	size_t m_Capacity = 0;
};


template<typename Key, typename Value, typename Hash, typename Comparer>
inline BvRobinMap<Key, Value, Hash, Comparer>::BvRobinMap()
{
}


template<typename Key, typename Value, typename Hash, typename Comparer>
inline BvRobinMap<Key, Value, Hash, Comparer>::BvRobinMap(const size_t capacity)
{
	ResizeAndRehash(capacity);
}


template<typename Key, typename Value, typename Hash, typename Comparer>
inline BvRobinMap<Key, Value, Hash, Comparer>::BvRobinMap(const BvRobinMap& rhs)
{
	ResizeAndRehash(rhs.m_Capacity);

	for (auto it = rhs.cbegin(); it != rhs.cend(); it++)
	{
		Emplace(it->first, it->second);
	}
}


template<typename Key, typename Value, typename Hash, typename Comparer>
inline BvRobinMap<Key, Value, Hash, Comparer>::BvRobinMap(BvRobinMap&& rhs) noexcept
{
	*this = std::move(rhs);
}


template<typename Key, typename Value, typename Hash, typename Comparer>
inline BvRobinMap<Key, Value, Hash, Comparer>::BvRobinMap(std::initializer_list<KeyValue> list)
{
	ResizeAndRehash(list.size());

	for (auto it = list.begin(); it != list.end(); it++)
	{
		Emplace(it->first, it->second);
	}
}


template<typename Key, typename Value, typename Hash, typename Comparer>
inline BvRobinMap<Key, Value, Hash, Comparer>& BvRobinMap<Key, Value, Hash, Comparer>::operator=(const BvRobinMap& rhs)
{
	if (this->m_pHashes != rhs.m_pHashes)
	{
		Clear();

		for (auto it = rhs.cbegin(); it != rhs.cend(); it++)
		{
			Emplace(it->first, it->second);
		}
	}

	return *this;
}


template<typename Key, typename Value, typename Hash, typename Comparer>
inline BvRobinMap<Key, Value, Hash, Comparer>& BvRobinMap<Key, Value, Hash, Comparer>::operator=(BvRobinMap&& rhs) noexcept
{
	if (this->m_pHashes != rhs.m_pHashes)
	{
		std::swap(m_pData, rhs.m_pData);
		std::swap(m_pHashes, rhs.m_pHashes);
		std::swap(m_Size, rhs.m_Size);
		std::swap(m_Capacity, rhs.m_Capacity);
	}

	return *this;
}


template<typename Key, typename Value, typename Hash, typename Comparer>
inline BvRobinMap<Key, Value, Hash, Comparer>& BvRobinMap<Key, Value, Hash, Comparer>::operator=(std::initializer_list<KeyValue> list)
{
	Clear();

	ResizeAndRehash(list.size());

	for (auto it = list.begin(); it != list.end(); it++)
	{
		Emplace(it->first, it->second);
	}
}


template<typename Key, typename Value, typename Hash, typename Comparer>
inline BvRobinMap<Key, Value, Hash, Comparer>::~BvRobinMap()
{
	Destroy();
}


template<typename Key, typename Value, typename Hash, typename Comparer>
inline void BvRobinMap<Key, Value, Hash, Comparer>::ResizeAndRehash(const size_t size)
{
	if (size <= m_Capacity)
	{
		return;
	}

	m_Capacity = (size & (size - 1)) == 0 ? size : GetNextPowerOf2(size);

	KeyValue * pNewData = reinterpret_cast<KeyValue *>(BvMAlloc(sizeof(KeyValue) * m_Capacity, std::max(alignof(Key), alignof(Value))));
	memset(pNewData, 0, sizeof(KeyValue) * m_Capacity);
	size_t * pNewHashes = reinterpret_cast<size_t *>(BvMAlloc(sizeof(size_t) * m_Capacity));

	memset(pNewHashes, 0, sizeof(size_t) * m_Capacity);

	for (size_t i = 0; i < m_Size; i++)
	{
		if (m_pHashes[i] != 0)
		{
			MoveObject(&m_pData[i], m_pHashes[i], pNewData, pNewHashes);
		}
	}

	BvFree(m_pData);
	BvFree(m_pHashes);

	m_pData = pNewData;
	m_pHashes = pNewHashes;
}


template<typename Key, typename Value, typename Hash, typename Comparer>
template<typename... Args>
inline std::pair<typename BvRobinMap<Key, Value, Hash, Comparer>::Iterator, bool> BvRobinMap<Key, Value, Hash, Comparer>::Emplace(Key && key, Args &&... args)
{
	if (m_Size == m_Capacity)
	{
		ResizeAndRehash(GetNextPowerOf2(m_Capacity));
	}

	auto iter = FindKey(std::forward<Key>(key));
	if (iter != cend())
	{
		return std::make_pair(iter, false);
	}

	auto index = InsertInternal(std::forward<Key>(key), std::forward<Args>(args)...);
	return std::make_pair(Iterator(m_pData, m_pData + index, m_pHashes + index, &m_Size), true);
}


template<typename Key, typename Value, typename Hash, typename Comparer>
template<typename... Args>
inline std::pair<typename BvRobinMap<Key, Value, Hash, Comparer>::Iterator, bool> BvRobinMap<Key, Value, Hash, Comparer>::Emplace(const Key & key, Args &&... args)
{
	if (m_Size == m_Capacity)
	{
		ResizeAndRehash(GetNextPowerOf2(m_Capacity));
	}

	auto iter = FindKey(key);
	if (iter != cend())
	{
		return std::make_pair(iter, false);
	}

	auto index = InsertInternal(key, std::forward<Args>(args)...);
	return std::make_pair(Iterator(m_pData, m_pData + index, m_pHashes + index, &m_Size), true);
}


template<typename Key, typename Value, typename Hash, typename Comparer>
template<typename ...Args>
inline std::pair<typename BvRobinMap<Key, Value, Hash, Comparer>::Iterator, bool> BvRobinMap<Key, Value, Hash, Comparer>::EmplaceOrAssign(const Key & key, Args &&... args)
{
	auto iter = FindKey(key);
	if (iter != cend())
	{
		auto index = iter.GetIndex();
		m_pData[index].second.~Value();
		new (&m_pData[index].second) Value(std::forward<Args>(args)...);
		return std::make_pair(Iterator(m_pData, m_pData + index, m_pHashes + index, &m_Size), false);
	}

	if (m_Size == m_Capacity)
	{
		ResizeAndRehash(GetNextPowerOf2(m_Capacity));
	}

	auto index = InsertInternal(key, std::forward<Args>(args)...);
	return std::make_pair(Iterator(m_pData, m_pData + index, m_pHashes + index, &m_Size), true);
}


template<typename Key, typename Value, typename Hash, typename Comparer>
inline bool BvRobinMap<Key, Value, Hash, Comparer>::Erase(const Key & key)
{
	auto iter = FindKey(key);
	if (iter == cend())
	{
		return false;
	}

	auto currIndex = iter.GetIndex();
	m_pData[currIndex].first.~Key();
	m_pData[currIndex].second.~Value();
	m_pHashes[currIndex] = 0;
	m_Size--;

	for (;;)
	{
		auto shiftIndex = (currIndex + 1) & (m_Capacity - 1);
		if (m_pHashes[shiftIndex] == 0)
		{
			break;
		}

		size_t desiredPos = HashPos(m_pHashes[shiftIndex]);
		if (desiredPos == shiftIndex)
		{
			break;
		}
		else
		{
			m_pData[currIndex].first = std::move(m_pData[shiftIndex].first);
			m_pData[currIndex].second = std::move(m_pData[shiftIndex].second);
			m_pHashes[currIndex] = m_pHashes[shiftIndex];
			m_pHashes[shiftIndex] = 0;
		}

		currIndex = shiftIndex;
	}

	return true;
}


template<typename Key, typename Value, typename Hash, typename Comparer>
RobinIterator<std::pair<Key, Value>> BvRobinMap<Key, Value, Hash, Comparer>::FindKey(const Key & key) const
{
	size_t hash = Hash(key);
	size_t pos = HashPos(hash);
	size_t dist = 0;

	for (; dist < m_Capacity;)
	{
		if (m_pHashes[pos] == 0)
		{
			// If there isn't an element with that key, return the end of the map
			return Iterator(m_pData, m_pData + m_Capacity, m_pHashes + m_Capacity, &m_Capacity);
		}

		if (m_pHashes[pos] == hash && Comparer()(m_pData[pos].first, key))
		{
			// If there's already an element with that key, return true and the index where it's stored
			return Iterator(m_pData, m_pData + pos, m_pHashes + pos, &m_Capacity);
		}

		size_t currElemDesiredPos = HashPos(m_pHashes[pos]);
		size_t currElemDist = (pos - currElemDesiredPos + m_Capacity) & (m_Capacity - 1); // Add capacity to make insertion / look-up circular
		if (dist > currElemDist)
		{
			// If there isn't an element with that key, return the end of the map
			return Iterator(m_pData, m_pData + m_Capacity, m_pHashes + m_Capacity, &m_Capacity);
		}

		pos = (pos + 1) & (m_Capacity - 1);
		dist++;
	}

	return Iterator(m_pData, m_pData + m_Capacity, m_pHashes + m_Capacity, &m_Capacity);
}


 template<typename Key, typename Value, typename Hash, typename Comparer>
 bool BvRobinMap<Key, Value, Hash, Comparer>::HasKey(const Key& key) const
 {
	 return FindKey(key) != cend();
 }


template<typename Key, typename Value, typename Hash, typename Comparer>
void BvRobinMap<Key, Value, Hash, Comparer>::Clear()
{
	for (size_t i = m_Size; i > 0; i--)
	{
		if (m_pHashes[i - 1])
		{
			m_pData[i - 1].first.~Key();
			m_pData[i - 1].second.~Value();
		}
	}

	m_Size = 0;
}


template<typename Key, typename Value, typename Hash, typename Comparer>
inline Value & BvRobinMap<Key, Value, Hash, Comparer>::operator[](const Key & key)
{
	auto iter = FindKey(key);
	if (iter == end())
	{
		auto result = Emplace(key, Value());
		return m_pData[result.first.GetIndex()].second;
	}

	return m_pData[iter.GetIndex()].second;
}


template<typename Key, typename Value, typename Hash, typename Comparer>
inline Value & BvRobinMap<Key, Value, Hash, Comparer>::operator[](Key && key)
{
	auto iter = FindKey(key);
	if (iter == cend())
	{
		auto result = Emplace(std::forward<Key>(key), Value());
		return m_pData[result.first.GetIndex()].second;
	}

	return m_pData[iter.GetIndex()].second;
}


template<typename Key, typename Value, typename Hash, typename Comparer>
inline Value & BvRobinMap<Key, Value, Hash, Comparer>::At(const Key & key)
{
	auto iter = FindKey(key);
	BvAssert(iter != cend(), "Key not found");

	return m_pData[iter.GetIndex()].second;
}


template<typename Key, typename Value, typename Hash, typename Comparer>
inline const Value & BvRobinMap<Key, Value, Hash, Comparer>::At(const Key & key) const
{
	auto iter = FindKey(key);
	BvAssert(iter != cend(), "Key not found");

	return m_pData[iter.GetIndex()].second;
}


template<typename Key, typename Value, typename Hash, typename Comparer>
template<typename KeyType, typename... Args>
const size_t BvRobinMap<Key, Value, Hash, Comparer>::InsertInternal(KeyType && key, Args &&... args)
{
	constexpr size_t pairSize = sizeof(KeyValue);
	char mem[pairSize];
	KeyValue * pNewPair = reinterpret_cast<KeyValue *>(mem);
	
	new (&pNewPair->first) Key(key);
	new (&pNewPair->second) Value(std::forward<Args>(args)...);
	size_t hash = Hash(key);

	m_Size++;
	return MoveObject(pNewPair, hash, m_pData, m_pHashes);
}


template<typename Key, typename Value, typename Hash, typename Comparer>
const size_t BvRobinMap<Key, Value, Hash, Comparer>::MoveObject(KeyValue * const pKeyValue, size_t hash, KeyValue * const pData, size_t * const pHashes)
{
	size_t pos = HashPos(hash);
	size_t dist = 0;

	constexpr size_t pairSize = sizeof(KeyValue);
	KeyValue tmpKeyValue;
	size_t tmpHash = 0;

	for (; dist < m_Capacity;)
	{
		if (pHashes[pos] == 0)
		{
			pData[pos].first = std::move(pKeyValue->first);
			pData[pos].second = std::move(pKeyValue->second);
			pHashes[pos] = hash;

			return pos;
		}

		size_t currElemDesiredPos = HashPos(pHashes[pos]);
		size_t currElemDist = (pos - currElemDesiredPos + m_Capacity) & (m_Capacity - 1); // Add capacity to make insertion / look-up circular
		if (dist > currElemDist)
		{
			tmpKeyValue.first = std::move(pData[pos].first);
			tmpKeyValue.second = std::move(pData[pos].second);

			pData[pos].first = std::move(pKeyValue->first);
			pData[pos].second = std::move(pKeyValue->second);

			pKeyValue->first = std::move(tmpKeyValue.first);
			pKeyValue->second = std::move(tmpKeyValue.second);

			tmpHash = pHashes[pos];
			pHashes[pos] = hash;
			hash = tmpHash;

			dist = currElemDist;
		}

		pos = (pos + 1) & (m_Capacity - 1);
		dist++;
	}

	BvAssert(0, "This code should be unreachable");

	return kU32Max;
}


template<typename Key, typename Value, typename Hash, typename Comparer>
void BvRobinMap<Key, Value, Hash, Comparer>::Destroy()
{
	Clear();

	m_Capacity = 0;

	if (m_pData)
	{
		BvFree(m_pData);
		m_pData = nullptr;
	}
	if (m_pHashes)
	{
		BvFree(m_pHashes);
		m_pHashes = nullptr;
	}
}


template<typename Key, typename Value, typename Hash, typename Comparer>
inline const size_t BvRobinMap<Key, Value, Hash, Comparer>::Hash(const Key & key) const
{
	size_t hash = FNV1a64(&key, sizeof(Key));

	return hash != 0 ? hash : 1;
}