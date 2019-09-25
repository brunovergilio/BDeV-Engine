#pragma once


#include "BvCore/Container/BvVector.h"
#include "BvCore/Utils/BvUtils.h"
#include <functional>

template<typename Type, typename... Args>
class Dispatcher
{
public:
	Dispatcher();
	~Dispatcher();

	template<typename ObjType>
	void Subscribe(ObjType * const pObj, void (ObjType::*pFunction)(const Type &, Args...));
	template<typename ObjType>
	void Unsubscribe(ObjType * const pObj);

	void Notify(const Type & dispatcherObj, Args... args);

private:
	struct Listener
	{
		Listener() = default;
		Listener(void * pObj, std::function<void(const Type &, Args...)> pFunction)
			: pObj(pObj), pFunction(pFunction) {}
		void * pObj = nullptr;
		std::function<void(const Type &, Args...)> pFunction = nullptr;
	};

	BvVector<Listener *> m_Listeners;
};


template<typename Type, typename... Args>
Dispatcher<Type, Args...>::Dispatcher()
{
}


template<typename Type, typename... Args>
Dispatcher<Type, Args...>::~Dispatcher()
{
	for (auto && listener : m_Listeners)
	{
		BvDelete(listener);
	}

	m_Listeners.Clear();
}


template<typename Type, typename... Args>
template<typename ObjType>
void Dispatcher<Type, Args...>::Subscribe(ObjType * const pObj, void (ObjType::*pFunction)(const Type &, Args...))
{
	m_Listeners.EmplaceBack(new Listener(pObj, [pObj, pFunction](const Type & type, Args... args)
	{
		(pObj->*pFunction)(type, args...);
	}));
}


template<typename Type, typename... Args>
template<typename ObjType>
void Dispatcher<Type, Args...>::Unsubscribe(ObjType * const pObj)
{
	size_t i;
	for (i = 0; i < m_Listeners.Size(); i++)
	{
		if (m_Listeners[i]->pObj == pObj)
		{
			break;
		}
	}

	if (i < m_Listeners.Size())
	{
		BvDelete(m_Listeners[i]);
		m_Listeners.Erase(m_Listeners.cbegin() + i);
	}
}


template<typename Type, typename ...Args>
inline void Dispatcher<Type, Args...>::Notify(const Type & dispatcherObj, Args... args)
{
	for (auto && callback : m_Listeners)
	{
		callback->pFunction(dispatcherObj, args...);
	}
}