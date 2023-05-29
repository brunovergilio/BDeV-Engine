#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Container/BvIterator.h"
#include "BDeV/Utils/BvHash.h"
#include "BDeV/System/Memory/BvMemoryCommon.h"
#include <algorithm>
#include <utility>


template<typename Key, typename Hash = BvHash<Key>, typename Comparer = std::equal_to<Key>>
class BvRobinSet
{
public:
	using KeyValue = Key;

	using Iterator = RobinIterator<KeyValue>;
	using ConstIterator = RobinIterator<KeyValue, true>;

	BvRobinSet(); // Default
	explicit BvRobinSet(IBvMemoryAllocator* pAllocator);
	explicit BvRobinSet(const size_t capacity, IBvMemoryAllocator* pAllocator = GetDefaultAllocator()); // Reserve
	BvRobinSet(std::initializer_list<KeyValue> list, IBvMemoryAllocator* pAllocator = GetDefaultAllocator()); // Initializer List
	BvRobinSet(const BvRobinSet& rhs); // Copy
	BvRobinSet(BvRobinSet&& rhs) noexcept; // Move

	BvRobinSet& operator =(const BvRobinSet& rhs); // Copy Assignment
	BvRobinSet& operator =(BvRobinSet&& rhs) noexcept; // Move Assignment
	BvRobinSet& operator =(std::initializer_list<KeyValue> list); // Initializer List Assignment

	~BvRobinSet();

	// Allocator
	IBvMemoryAllocator* GetAllocator() const;
	void SetAllocator(IBvMemoryAllocator* pAllocator);

	// Iterator
	Iterator begin() { return Iterator(m_pData, m_pData, m_pHashes, &m_Capacity); }
	Iterator end() { return Iterator(m_pData, m_pData + m_Capacity, m_pHashes + m_Capacity, &m_Capacity); }
	const ConstIterator begin() const { return ConstIterator(m_pData, m_pData, m_pHashes, &m_Capacity); }
	const ConstIterator end() const { return ConstIterator(m_pData, m_pData + m_Capacity, m_pHashes + m_Capacity, &m_Capacity); }
	ConstIterator cbegin() const { return begin(); }
	ConstIterator cend() const { return end(); }

	void ResizeAndRehash(const size_t size);

	std::pair<typename BvRobinSet<Key, Hash, Comparer>::Iterator, bool> Emplace(Key&& key);
	std::pair<typename BvRobinSet<Key, Hash, Comparer>::Iterator, bool> Emplace(const Key& key);
	template<typename ...Args>
	std::pair<typename BvRobinSet<Key, Hash, Comparer>::Iterator, bool> Emplace(Args&&... args);
	bool Erase(const Key& key);

	Iterator FindKey(const Key& key) const;
	bool HasKey(const Key& key) const;

	void Clear();

	BV_INLINE const size_t Size() const { return m_Size; }
	BV_INLINE const size_t Capacity() const { return m_Capacity; }
	BV_INLINE const bool Empty() const { return m_Size == 0; }

private:
	template<typename KeyType>
	size_t EmplaceInternal(KeyValue* const pData, size_t* const pHashes, KeyType&& key);
	void Destroy();
	BV_INLINE const size_t HashPos(const u32 hash) const { return hash % m_Capacity; };
	BV_INLINE const size_t Hash(const Key& key) const;

private:
	KeyValue* m_pData = nullptr;
	size_t* m_pHashes = nullptr;
	IBvMemoryAllocator* m_pAllocator = GetDefaultAllocator();
	size_t m_Size = 0;
	size_t m_Capacity = 0;
};


template<typename Key, typename Hash, typename Comparer>
inline BvRobinSet<Key, Hash, Comparer>::BvRobinSet()
	: m_pAllocator(GetDefaultAllocator())
{
}

template<typename Key, typename Hash, typename Comparer>
inline BvRobinSet<Key, Hash, Comparer>::BvRobinSet(IBvMemoryAllocator* pAllocator)
	: m_pAllocator(pAllocator)
{
}


template<typename Key, typename Hash, typename Comparer>
inline BvRobinSet<Key, Hash, Comparer>::BvRobinSet(const size_t capacity, IBvMemoryAllocator* pAllocator)
	: m_pAllocator(pAllocator)
{
	ResizeAndRehash(capacity);
}


template<typename Key, typename Hash, typename Comparer>
inline BvRobinSet<Key, Hash, Comparer>::BvRobinSet(std::initializer_list<KeyValue> list, IBvMemoryAllocator* pAllocator)
	: m_pAllocator(pAllocator)
{
	ResizeAndRehash(list.size());

	for (auto it = list.begin(); it != list.end(); it++)
	{
		Emplace(it->first, it->second);
	}
}


template<typename Key, typename Hash, typename Comparer>
inline BvRobinSet<Key, Hash, Comparer>::BvRobinSet(const BvRobinSet& rhs)
{
	ResizeAndRehash(rhs.m_Capacity);

	for (auto it = rhs.cbegin(); it != rhs.cend(); it++)
	{
		Emplace(it->first, it->second);
	}
}


template<typename Key, typename Hash, typename Comparer>
inline BvRobinSet<Key, Hash, Comparer>::BvRobinSet(BvRobinSet&& rhs) noexcept
{
	*this = std::move(rhs);
}


template<typename Key, typename Hash, typename Comparer>
inline BvRobinSet<Key, Hash, Comparer>& BvRobinSet<Key, Hash, Comparer>::operator=(const BvRobinSet& rhs)
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


template<typename Key, typename Hash, typename Comparer>
inline BvRobinSet<Key, Hash, Comparer>& BvRobinSet<Key, Hash, Comparer>::operator=(BvRobinSet&& rhs) noexcept
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


template<typename Key, typename Hash, typename Comparer>
inline BvRobinSet<Key, Hash, Comparer>& BvRobinSet<Key, Hash, Comparer>::operator=(std::initializer_list<KeyValue> list)
{
	Clear();

	ResizeAndRehash(list.size());

	for (auto it = list.begin(); it != list.end(); it++)
	{
		Emplace(it->first, it->second);
	}
}


template<typename Key, typename Hash, typename Comparer>
inline BvRobinSet<Key, Hash, Comparer>::~BvRobinSet()
{
	Destroy();
}


template<typename Key, typename Hash, typename Comparer>
inline IBvMemoryAllocator* BvRobinSet<Key, Hash, Comparer>::GetAllocator() const
{
	return m_pAllocator;
}


template<typename Key, typename Hash, typename Comparer>
inline void BvRobinSet<Key, Hash, Comparer>::SetAllocator(IBvMemoryAllocator* pAllocator)
{
	if (m_pAllocator == pAllocator)
	{
		return;
	}

	if (m_Capacity > 0)
	{
		KeyValue* pNewData = reinterpret_cast<KeyValue*>(pAllocator->Allocate((sizeof(KeyValue) + sizeof(size_t)) * m_Capacity, std::max(alignof(KeyValue), alignof(size_t)), 0, BV_SOURCE_INFO));
		size_t* pNewHashes = reinterpret_cast<size_t*>(reinterpret_cast<char*>(pNewData) + (sizeof(KeyValue) * m_Capacity));
		memset(pNewHashes, 0, sizeof(size_t) * m_Capacity);
		if (m_Size > 0)
		{
			for (size_t i = 0; i < m_Capacity; i++)
			{
				if (m_pHashes[i] != 0)
				{
					// Because we're using the same capacity, the elements will hash to the same positions,
					// therefore there's no need to call EmplaceInternal()
					new (&pNewData[i]) Key(std::move(m_pData[i]));
					pNewHashes[i] = m_pHashes[i];
				}
			}
		}
		Clear();
		m_pAllocator->Free(m_pData, BV_SOURCE_INFO);

		m_pData = pNewData;
		m_pHashes = pNewHashes;
	}

	m_pAllocator = pAllocator;
}


template<typename Key, typename Hash, typename Comparer>
inline void BvRobinSet<Key, Hash, Comparer>::ResizeAndRehash(const size_t size)
{
	if (size <= m_Capacity)
	{
		return;
	}

	auto oldCapacity = m_Capacity;
	m_Capacity = size;

	KeyValue* pNewData = reinterpret_cast<KeyValue*>(BvMAlloc((sizeof(KeyValue) + sizeof(size_t)) * m_Capacity, std::max(alignof(KeyValue), alignof(size_t))));
	size_t* pNewHashes = reinterpret_cast<size_t*>(reinterpret_cast<char*>(pNewData) + (sizeof(KeyValue) * m_Capacity));
	memset(pNewHashes, 0, sizeof(size_t) * m_Capacity);

	for (size_t i = 0; i < oldCapacity; i++)
	{
		if (m_pHashes[i] != 0)
		{
			EmplaceInternal(pNewData, pNewHashes, std::move(m_pData[i]));
		}
	}

	if (m_pData)
	{
		m_pAllocator->Free(m_pData, BV_SOURCE_INFO);
	}

	m_pData = pNewData;
	m_pHashes = pNewHashes;
}


template<typename Key, typename Hash, typename Comparer>
inline std::pair<typename BvRobinSet<Key, Hash, Comparer>::Iterator, bool> BvRobinSet<Key, Hash, Comparer>::Emplace(Key&& key)
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

	auto index = EmplaceInternal(m_pData, m_pHashes, std::forward<Key>(key));
	++m_Size;

	return std::make_pair(Iterator(m_pData, m_pData + index, m_pHashes + index, &m_Size), true);
}


template<typename Key, typename Hash, typename Comparer>
inline std::pair<typename BvRobinSet<Key, Hash, Comparer>::Iterator, bool> BvRobinSet<Key, Hash, Comparer>::Emplace(const Key& key)
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

	auto index = EmplaceInternal(m_pData, m_pHashes, key);
	++m_Size;

	return std::make_pair(Iterator(m_pData, m_pData + index, m_pHashes + index, &m_Size), true);
}


template<typename Key, typename Hash, typename Comparer>
template<typename ...Args>
inline std::pair<typename BvRobinSet<Key, Hash, Comparer>::Iterator, bool> BvRobinSet<Key, Hash, Comparer>::Emplace(Args&&... args)
{
	if (m_Size == m_Capacity)
	{
		ResizeAndRehash(CalculateNewContainerSize(m_Capacity));
	}

	Key key(std::forward<Args>(args)...);
	auto iter = FindKey(key);
	if (iter != cend())
	{
		return std::make_pair(iter, false);
	}

	auto index = EmplaceInternal(m_pData, m_pHashes, std::move(key));
	++m_Size;

	return std::make_pair(Iterator(m_pData, m_pData + index, m_pHashes + index, &m_Size), true);
}


template<typename Key, typename Hash, typename Comparer>
inline bool BvRobinSet<Key, Hash, Comparer>::Erase(const Key& key)
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


template<typename Key, typename Hash, typename Comparer>
BvRobinSet<Key, Hash, Comparer>::Iterator BvRobinSet<Key, Hash, Comparer>::FindKey(const Key& key) const
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
		if (m_pHashes[pos] == hash && Comparer()(m_pData[pos], key))
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


template<typename Key, typename Hash, typename Comparer>
bool BvRobinSet<Key, Hash, Comparer>::HasKey(const Key& key) const
{
	return FindKey(key) != cend();
}


template<typename Key, typename Hash, typename Comparer>
void BvRobinSet<Key, Hash, Comparer>::Clear()
{
	for (size_t i = 0; i < m_Capacity; i++)
	{
		if (m_pHashes[i])
		{
			m_pData[i].~Key();
		}
	}

	m_Size = 0;
}


template<typename Key, typename Hash, typename Comparer>
template<typename KeyType>
inline size_t BvRobinSet<Key, Hash, Comparer>::EmplaceInternal(KeyValue* const pData, size_t* const pHashes, KeyType&& key)
{
	size_t hash = Hash(key);
	size_t currPos = HashPos(hash);
	size_t distFromDesiredPos = 0;
	size_t newElemInsertedPosition = kU64Max;

	for (; distFromDesiredPos < m_Capacity; distFromDesiredPos++, currPos = (currPos + 1) % m_Capacity)
	{
		if (pHashes[currPos] == 0)
		{
			new (&pData[currPos]) KeyValue(std::move(key));
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
			std::swap(pData[currPos], key);
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


template<typename Key, typename Hash, typename Comparer>
void BvRobinSet<Key, Hash, Comparer>::Destroy()
{
	Clear();

	m_Capacity = 0;

	if (m_pData)
	{
		m_pAllocator->Free(m_pData, BV_SOURCE_INFO);
		m_pData = nullptr;
	}
}


template<typename Key, typename Hash, typename Comparer>
inline const size_t BvRobinSet<Key, Hash, Comparer>::Hash(const Key& key) const
{
	size_t hash = MurmurHash64A(&key, sizeof(Key));

	return hash != 0 ? hash : 1;
}