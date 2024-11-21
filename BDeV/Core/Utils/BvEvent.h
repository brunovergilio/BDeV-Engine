#pragma once


#include "BDeV/Core/Container/BvVector.h"
#include <functional>


template<typename T>
class BvDelegate {};

template<typename R, typename... Args>
class BvDelegate<R(Args...)>
{
	using ObjType = void*;
	using FunctionType = R(*)(ObjType, Args&&...);
	using DataType = std::pair<ObjType, FunctionType>;

	template<R(*Fn)(Args...)>
	static BV_INLINE R FunctionStub(ObjType obj, Args&&... args)
	{
		return (Fn)(std::forward<Args>(args)...);
	}

	template<typename T, R(T::* Fn)(Args...)>
	static BV_INLINE R ClassFunctionStub(ObjType obj, Args&&... args)
	{
		return (static_cast<T*>(obj)->*Fn)(std::forward<Args>(args)...);
	}

	template<typename T, R(T::* Fn)(Args...) const>
	static BV_INLINE R ConstClassFunctionStub(ObjType obj, Args&&... args)
	{
		return (static_cast<const T*>(obj)->*Fn)(std::forward<Args>(args)...);
	}

	BV_NOCOPY(BvDelegate);

public:
	BvDelegate() {}
	BvDelegate(BvDelegate&& rhs) noexcept
	{
		*this = std::move(rhs);
	}
	BvDelegate& operator=(BvDelegate&& rhs) noexcept
	{
		if (this != &rhs)
		{
			std::swap(m_Data, rhs.m_Data);
		}

		return *this;
	}

	template<R(*Fn)(Args...)>
	void Bind()
	{
		m_Data.first = nullptr;
		m_Data.second = &FunctionStub<Fn>;
	}

	template<typename T, R(T::* Fn)(Args...)>
	void Bind(T* pObj)
	{
		m_Data.first = pObj;
		m_Data.second = &ClassFunctionStub<T, Fn>;
	}

	template<typename T, R(T::* Fn)(Args...) const>
	void Bind(const T* pObj)
	{
		m_Data.first = const_cast<T*>(pObj);
		m_Data.second = &ConstClassFunctionStub<T, Fn>;
	}

	R operator()(Args... args)
	{
		return m_Data.second(m_Data.first, std::forward<Args>(args)...);
	}

private:
	DataType m_Data;
};


template<typename... Args>
class BvEvent
{
	using HandlerType = BvDelegate<void(Args...)>;

public:
	BvEvent() {}
	BvEvent(u32 maxHandlers, IBvMemoryArena* pArena = nullptr)
		: m_Handlers(pArena)
	{
		m_Handlers.Reserve(maxHandlers);
	}
	BvEvent(BvEvent&& rhs) noexcept
	{
		*this = std::move(rhs);
	}
	BvEvent& operator=(BvEvent&& rhs) noexcept
	{
		if (this != &rhs)
		{
			std::swap(m_Handlers, rhs.m_Handlers);
		}

		return *this;
	}

	template<void(*Fn)(Args...)>
	void AddHandler()
	{
		if (m_Handlers.Size() == m_Handlers.Capacity())
		{
			return;
		}

		auto& handler = m_Handlers.EmplaceBack(HandlerType());
		handler.Bind<Fn>();
	}

	template<typename T, void(T::* Fn)(Args...)>
	void AddHandler(T* pObj)
	{
		if (m_Handlers.Size() == m_Handlers.Capacity())
		{
			return;
		}

		auto& handler = m_Handlers.EmplaceBack(HandlerType());
		handler.Bind<Fn>(pObj);
	}

	template<typename T, void(T::* Fn)(Args...) const>
	void AddHandler(const T* pObj)
	{
		if (m_Handlers.Size() == m_Handlers.Capacity())
		{
			return;
		}

		auto& handler = m_Handlers.EmplaceBack(HandlerType());
		handler.Bind<Fn>(pObj);
	}

	void operator()(Args... args)
	{
		for (auto& handler : m_Handlers)
		{
			handler(std::forward<Args>(args)...);
		}
	}

private:
	BvVector<HandlerType> m_Handlers;
};