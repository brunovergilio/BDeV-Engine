#pragma once


#include "BDeV/Container/BvVector.h"
#include <functional>


template<class... Args>
class BV_API BvEvent
{
public:
	BvEvent()
	{
	}

	BvEvent(const BvEvent& rhs)
	{
		m_Subscribers = rhs.m_Subscribers;
	}

	BvEvent(BvEvent&& rhs) noexcept
	{
		*this = std::move(rhs);
	}

	BvEvent& operator=(const BvEvent& rhs)
	{
		if (this != &rhs)
		{
			m_Subscribers = rhs.m_Subscribers;
		}

		return *this;
	}

	BvEvent& operator=(BvEvent&& rhs) noexcept
	{
		if (this != &rhs)
		{
			m_Subscribers = std::move(rhs.m_Subscribers);
		}

		return *this;
	}
	
	~BvEvent()
	{
		Clear();
	}

	void Subscribe(std::function<void(Args...)> handler)
	{
		m_Subscribers.PushBack(std::move(handler));
	}

	BvEvent& operator+=(std::function<void(Args...)> handler)
	{
		m_Subscribers.PushBack(std::move(handler));

		return *this;
	}

	void Clear()
	{
		m_Subscribers.Clear();
	}

	void operator() (Args... args) const
	{
		for (auto&& subscriber : m_Subscribers)
		{
			subscriber(std::forward<Args>(args)...);
		}
	}

	operator bool() const
	{
		return m_Subscribers.Size() > 0;
	}

private:
	BvVector<std::function<void(Args...)>> m_Subscribers;
};