#pragma once


#include "BDeV/Core/Container/BvVector.h"
#include <functional>


template<typename T>
class BvEvent {};

template<typename R, typename... Args>
class BvEvent<R(Args...)>
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

	BV_NOCOPY(BvEvent);

public:
	BvEvent() {}
	BvEvent(u32 maxHandlers, IBvMemoryArena* pArena = nullptr) : m_Handlers(maxHandlers, {}, pArena) {}
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

	template<R(*Fn)(Args...)>
	void AddHandler()
	{
		m_Handlers.PushBack({ nullptr, &FunctionStub<Fn> });
	}

	template<typename T, R(T::* Fn)(Args...)>
	void AddHandler(T* pObj)
	{
		m_Handlers.PushBack({ pObj, &ClassFunctionStub<T, Fn> });
	}

	template<typename T, R(T::* Fn)(Args...) const>
	void AddHandler(const T* pObj)
	{
		m_Handlers.PushBack({ const_cast<T*>(pObj), &ConstClassFunctionStub<T, Fn> });
	}

	void operator()(Args... args)
	{
		for (auto& handler : m_Handlers)
		{
			handler.second(handler.first, std::forward<Args>(args)...);
		}
	}

	template<typename = typename std::enable_if_t<!std::is_same_v<void, R>>>
	bool RunNext(u32& current, R* result, Args... args)
	{
		if (current < m_Handlers.Size())
		{
			auto& handler = m_Handlers[current++];
			*result = handler.second(handler.first, std::forward<Args>(args)...);
			return true;
		}

		return false;
	}

	template<typename = typename std::enable_if_t<std::is_same_v<void, R>>>
	bool RunNext(u32& current, Args... args)
	{
		if (current < m_Handlers.Size())
		{
			auto& handler = m_Handlers[current++];
			handler.second(handler.first, std::forward<Args>(args)...);
			return true;
		}

		return false;
	}

private:
	BvVector<DataType> m_Handlers;
};