#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Container/BvIterator.h"
#include "BDeV/System/Debug/BvDebug.h"


template<typename Type>
class BvQueue
{
public:
	BvQueue() {}

	BvQueue(const BvQueue& rhs)

		: m_pData(rhs.m_pData), m_Size(rhs.m_Size), m_Capacity(rhs.m_Capacity), m_Front(rhs.m_Front)
	{}

	BvQueue(BvQueue&& rhs) noexcept
	{
		*this = std::move(rhs);
	}

	BvQueue(std::initializer_list<Type> list)
	{
		for (auto&& item : list)
		{
			Enqueue(item);
		}
	}

	BvQueue& operator =(const BvQueue& rhs)
	{
		if (this != &rhs)
		{
			m_pData = rhs.m_pData;
			m_Size = rhs.m_Size;
			m_Capacity = rhs.m_Capacity;
			m_Front = rhs.m_Front;
		}

		return *this;
	}

	BvQueue& operator =(BvQueue&& rhs) noexcept
	{
		if (this != &rhs)
		{
			m_pData = rhs.m_pData; rhs.m_pData = nullptr;
			m_Size = rhs.m_Size; rhs.m_Size = 0;
			m_Capacity = rhs.m_Capacity; rhs.m_Capacity = 0;
			m_Front = rhs.m_Front; rhs.m_Front = 0;
		}

		return *this;
	}

	BvQueue& operator =(std::initializer_list<Type> list)
	{
		for (auto&& item : list)
		{
			Enqueue(item);
		}

		return *this;
	}

	void PushBack(const Type& value)
	{
		EmplaceBack(value);
	}

	void PushBack(Type&& value)
	{
		EmplaceBack(std::move(value));
	}

	template<typename... Args>
	Type& EmplaceBack(Args&&... args)
	{
		if (m_Size == m_Capacity)
		{
			Reserve(m_Capacity + 1 + u32(m_Capacity * 1.5f));
		}

		auto backIndex = (m_Front + m_Size++) % m_Capacity;
		new (&m_pData[backIndex]) Type(std::forward<Args>(args)...);

		return m_pData[backIndex];
	}

	void PushFront(const Type& value)
	{
		EmplaceFront(value);
	}

	void PushFront(Type&& value)
	{
		EmplaceFront(std::move(value));
	}

	template<typename... Args>
	Type& EmplaceFront(Args&&... args)
	{
		if (m_Size == m_Capacity)
		{
			Reserve(m_Capacity + 1 + u32(m_Capacity * 1.5f));
		}

		m_Front = (m_Front - 1 + m_Capacity) % m_Capacity;
		new (&m_pData[m_Front]) Type(std::forward<Args>(args)...);
		++m_Size;

		return m_pData[m_Front];
	}

	void PopFront()
	{
		if (IsEmpty())
		{
			return;
		}

		m_pData[m_Front].~Type();
		m_Front = (m_Front + 1) % m_Capacity;
		--m_Size;
	}

	bool PopFront(Type& value)
	{
		if (IsEmpty())
		{
			return false;
		}

		value = std::move(m_pData[m_Front]);
		m_Front = (m_Front + 1) % m_Capacity;
		--m_Size;

		return true;
	}

	void PopBack()
	{
		if (IsEmpty())
		{
			return;
		}

		auto backIndex = (m_Front + --m_Size) % m_Capacity;
		m_pData[backIndex].~Type();
	}

	bool PopBack(Type& value)
	{
		if (IsEmpty())
		{
			return false;
		}

		auto backIndex = (m_Front + --m_Size) % m_Capacity;
		value = std::move(m_pData[backIndex]);

		return true;
	}

	Type& Front()
	{
		BvAssert(!IsEmpty(), "Queue is empty!");
		return m_pData[m_Front];
	}

	const Type& Front() const
	{
		BvAssert(!IsEmpty(), "Queue is empty!");
		return m_pData[m_Front];
	}

	Type& Back()
	{
		BvAssert(!IsEmpty(), "Queue is empty!");
		return m_pData[(m_Front + (m_Size - 1)) % m_Capacity];
	}

	const Type& Back() const
	{
		BvAssert(!IsEmpty(), "Queue is empty!");
		return m_pData[(m_Front + (m_Size - 1)) % m_Capacity];
	}

	u32 Size() const
	{
		return m_Size;
	}

	u32 Capacity() const
	{
		return m_Capacity;
	}

	bool IsEmpty() const
	{
		return m_Size == 0;
	}

	void Reserve(u32 newCapacity)
	{
		if (newCapacity <= m_Capacity)
		{
			return;
		}

		auto pTmpData = new Type[newCapacity];

		if constexpr (std::is_fundamental_v<Type>)
		{
			auto newIndex = 0u;
			for (auto oldIndex = m_Front; newIndex < m_Size; oldIndex = ((oldIndex + 1) % m_Capacity))
			{
				new (&pTmpData[newIndex++]) Type(std::move(m_pData[oldIndex]));
			}
		}

		delete[] m_pData;
		m_Capacity = newCapacity;
		m_pData = pTmpData;
		m_Front = 0;
	}

private:
	Type* m_pData = nullptr;
	u32 m_Size = 0;
	u32 m_Capacity = 0;
	u32 m_Front = 0;
};