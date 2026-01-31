#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Utils/BvUUID.h"
#include "BDeV/Core/System/Memory/BvMemory.h"
#include <atomic>


namespace Internal
{
	template<typename T>
	constexpr const BvUUID GetObjectUUID() = delete;

	template<typename T>
	const BvUUID GetObjectUUIDHelper(T** ppObj)
	{
		return GetObjectUUID<T>();
	}

	class BvRCObjController;
}

// Creates a UUID for a specific object type
#define BV_OBJECT_DEFINE_ID(objType, uuid) namespace Internal \
{ \
	template<> constexpr const BvUUID GetObjectUUID<objType>() \
	{ \
		return MakeUUID(uuid); \
	} \
}

// Returns the UUID of a specific object type
#define BV_OBJECT_ID(objType) Internal::GetObjectUUID<objType>()

// Helper for methods that create UUID-based objects
#define BV_OBJ_ARGS(ppObj) Internal::GetObjectUUIDHelper(ppObj), reinterpret_cast<void**>(ppObj)


// Base class for refcounted objects that manage themselves
class BvRCObj
{
public:
	BV_INLINE void AddRef()
	{
		m_RefCount.fetch_add(1, std::memory_order_relaxed);
	}

	BV_INLINE void Release()
	{
		if (m_RefCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
		{
			SelfDestroy();
		}
	}

	template<BvMemoryArenaType A>
	BV_INLINE void SetMemoryArena(A* pArena)
	{
		BV_ASSERT(m_pArena == nullptr, "Can't change memory arena");
		m_pArena = pArena;
	}

protected:
	BvRCObj() {}
	template<BvMemoryArenaType A> BvRCObj(A* pArena) : m_pArena(pArena) {}
	virtual ~BvRCObj() = 0 {}

	virtual void SelfDestroy()
	{
		auto pObj = this;
		auto pArena = m_pArena;

		pObj->~BvRCObj();
		pArena->Free(pObj);
	}

private:
	IBvMemoryArena* m_pArena = nullptr;
	std::atomic<i32> m_RefCount = 1;
};


template<typename T>
concept BvRCType = std::is_base_of_v<BvRCObj, T>;


namespace Internal
{
	class BvRCObjController
	{
	public:
		template<BvRCType T, BvMemoryArenaType A, typename... Args>
		static T* Create(A& arena, const std::source_location& sourceInfo, Args&&... args)
		{
			T* pObj = BV_MNEW_SI(arena, sourceInfo, T)(std::forward<Args>(args)...);
			pObj->SetMemoryArena(&arena);
			return pObj;
		}

		template<BvRCType T, typename... Args>
		static T* Create(const std::source_location& sourceInfo, Args&&... args)
		{
			auto& arena = *BV_DEFAULT_MEMORY_ARENA;
			T* pObj = BV_MNEW_SI(arena, sourceInfo, T)(std::forward<Args>(args)...);
			pObj->SetMemoryArena(&arena);
			return pObj;
		}

		template<BvRCType T, BvMemoryArenaType A>
		static void* Allocate(A& arena, const std::source_location& sourceInfo)
		{
			return BV_MALLOC_SI(arena, sourceInfo, sizeof(T), alignof(T));
		}
	};
}


// These macros can be used to create BvRCObj instances with the default allocator or a custom one. They are meant to be assistance methods
// but are by no means mandatory or the only ways to create these types of objects. If needed, the BvRCObj::SelfDestroy() method can be
// overridden and custom implementation logic can be added to it
#define BV_RC_CREATE_IN_PLACE(arena, Type, ...) new(Internal::BvRCObjController::Allocate<Type>(arena, std::source_location::current())) Type(__VA_ARGS__);
#define BV_RC_CREATE(Type, ...) Internal::BvRCObjController::Create<Type>(std::source_location::current() __VA_OPT__(, __VA_ARGS__))
#define BV_RC_CREATE_CUSTOM(arena, Type, ...) Internal::BvRCObjController::Create<Type>(arena, std::source_location::current() __VA_OPT__(, __VA_ARGS__))


// Templated class for automatic management of BvRCObj types
template<typename T>
class BvRCRef
{
	template<typename U>
	friend class BvRCRef;

public:
	BvRCRef()
		: m_pObj(nullptr)
	{
	}
	BvRCRef(T* pObj, bool addRef = true)
		: m_pObj(pObj)
	{
		if (addRef)
		{
			InternalAddRef();
		}
	}
	BvRCRef(std::nullptr_t)
		: m_pObj(nullptr)
	{
	}
	BvRCRef(const BvRCRef& rhs)
		: m_pObj(rhs.m_pObj)
	{
		InternalAddRef();
	}
	BvRCRef(BvRCRef&& rhs) noexcept
		: m_pObj(rhs.m_pObj)
	{
		rhs.m_pObj = nullptr;
	}
	BvRCRef& operator=(T* pObj)
	{
		if (m_pObj != pObj)
		{
			InternalRelease();
			m_pObj = pObj;
			InternalAddRef();
		}
		return *this;
	}
	BvRCRef& operator=(std::nullptr_t)
	{
		InternalRelease();
		return *this;
	}
	BvRCRef& operator=(const BvRCRef& rhs)
	{
		if (m_pObj != rhs.m_pObj)
		{
			InternalRelease();
			m_pObj = rhs.m_pObj;
			InternalAddRef();
		}
		return *this;
	}
	BvRCRef& operator=(BvRCRef&& rhs) noexcept
	{
		if (this != reinterpret_cast<BvRCRef*>(&reinterpret_cast<u8&>(rhs)))
		{
			InternalRelease();
			m_pObj = rhs.m_pObj;
			rhs.m_pObj = nullptr;
		}
		return *this;
	}
	~BvRCRef()
	{
		InternalRelease();
	}
	void Reset()
	{
		InternalRelease();
	}
	void Attach(T* pObj)
	{
		InternalRelease();
		m_pObj = pObj;
	}
	T* Detach()
	{
		auto pObj = m_pObj;
		m_pObj = nullptr;
		return pObj;
	}
	T* operator->() const { return m_pObj; }
	operator bool() const { return m_pObj != nullptr; }
	operator T*() const { return m_pObj; }
	T** operator&() { return &m_pObj; }

private:
	void InternalAddRef()
	{
		if (m_pObj)
		{
			m_pObj->AddRef();
		}
	}

	void InternalRelease()
	{
		if (m_pObj)
		{
			m_pObj->Release();
			m_pObj = nullptr;
		}
	}

private:
	T* m_pObj;
};