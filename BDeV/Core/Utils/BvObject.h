#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/Memory/BvMemoryArena.h"
#include "BDeV/Core/System/Memory/BvMemory.h"
#include "BDeV/Core/Utils/BvUUID.h"
#include <atomic>


class BvControlBlockBase
{
public:
	BvControlBlockBase() = default;
	virtual ~BvControlBlockBase() = default;

	virtual void Destroy() noexcept = 0;
	virtual void DestroySelf() noexcept = 0;

	u32 IncSRef() noexcept
	{
		return ++m_SRefs;
	}

	u32 IncWRef() noexcept
	{
		return ++m_WRefs;
	}

	u32 DecSRef() noexcept
	{
		auto count = --m_SRefs;
		if (count == 0)
		{
			Destroy();
			DecWRef();
		}

		return count;
	}

	u32 DecWRef() noexcept
	{
		auto count = --m_WRefs;
		if (count == 0)
		{
			DestroySelf();
		}

		return count;
	}

private:
	std::atomic<u32> m_SRefs = 1;
	std::atomic<u32> m_WRefs = 1;
};


template<typename T, typename A = IBvMemoryArena>
class BvControlBlock : public BvControlBlockBase
{
public:
	BvControlBlock(A* pArena = nullptr) : m_pArena(pArena) {}
	~BvControlBlock() {}

	void SetObject(T* pObj)
	{
		m_pObj = pObj;
	}

	void Destroy() noexcept override
	{
		if (m_pArena)
		{
			BV_MDELETE(*m_pArena, m_pObj);
		}
		else
		{
			BV_DELETE(m_pObj);
		}
	}

	void DestroySelf() noexcept override
	{
		if (m_pArena)
		{
			BV_MDELETE(*m_pArena, this);
		}
		else
		{
			BV_DELETE(this);
		}
	}

protected:
	A* m_pArena = nullptr;
	T* m_pObj = nullptr;
};


#define BV_IBVOBJECT_DEFINE_IID(objType, uuid) namespace Internal \
{ \
	constexpr BvUUID objType##_UUID = MakeUUIDv4(uuid); \
}
#define BV_IBVOBJECT_IID(objType) Internal::objType##_UUID

#define BV_IBVOBJECT_FOR_EACH(id, first, ...) || first::::QueryInterface(id, ppInterface) \
	IF_ELSE(HAS_ARGS(__VA_ARGS__))			\
	(										\
		DEFER2(_BV_IBVOBJECT_FOR_EACH)()(id, __VA_ARGS__)	\
	)										\
	(										\
	)
#define _BV_IBVOBJECT_FOR_EACH() BV_IBVOBJECT_FOR_EACH

#define BV_IBVOBJECT_IMPL_INTERFACE(objType, baseType, ...) \
bool QueryInterface(const BvUUID& id, IBvObject** ppInterface) override \
{ \
	if (!ppInterface) \
	{ \
		return false; \
	} \
	if (id == BV_IBVOBJECT_IID(objType)) \
	{ \
		*ppInterface = this; \
		this->AddRef(); \
		return true; \
	} \
	return baseType::QueryInterface(id, ppInterface) __VA_OPT__ ( EVAL ( BV_IBVOBJECT_FOR_EACH(id, __VA_ARGS__) ) ); \
}

// Base COM-like object
BV_IBVOBJECT_DEFINE_IID(IBvObject, "00000000-0000-0000-0000-000000000000")
class IBvObject
{
	BV_NOCOPYMOVE(IBvObject);

public:
	virtual u32 AddRef() = 0;
	virtual u32 Release() = 0;
	virtual bool QueryInterface(const BvUUID& id, IBvObject** ppInterface) = 0;

protected:
	IBvObject() {}
	virtual ~IBvObject() {}
};


// This is a ref-counted object type, derived from the base interface,
// which implements AddRef() and Release() for base classes.
class BvRefCounted : public IBvObject
{
	BV_NOCOPYMOVE(BvRefCounted);

public:
	void SetControlBlock(BvControlBlockBase* pControlBlock)
	{
		m_pControlBlock = pControlBlock;
	}

	u32 AddRef() override final
	{
		return m_pControlBlock->IncSRef();
	}

	u32 Release() override final
	{
		return m_pControlBlock->DecSRef();
	}

protected:
	BvRefCounted() {}
	~BvRefCounted() {}

protected:
	BvControlBlockBase* m_pControlBlock = nullptr;
};


// Base type for classes implementing the IBvObject interface. Any classes using this COM-like
// classes must derive from this class and be created with either BV_NEW or BV_MNEW.
class BvObjectBase : public BvRefCounted
{
	BV_NOCOPYMOVE(BvObjectBase);

public:
	virtual bool QueryInterface(const BvUUID& id, IBvObject** ppInterface)
	{
		if (!ppInterface)
		{
			return false;
		}

		*ppInterface = nullptr;
		if (id == BV_IBVOBJECT_IID(IBvObject))
		{
			*ppInterface = this;
			(*ppInterface)->AddRef();

			return true;
		}

		return false;
	}

protected:
	BvObjectBase() {}
	virtual ~BvObjectBase() {}
};


// Basic factory class to simplify the creation of IBvObject-based objects
class BvObjectCreator
{
public:
	template<typename T, typename... Args>
	static T* Create(Args&&... args)
	{
		auto pCB = BV_NEW(BvControlBlock<T>)();
		auto pNewObj = BV_NEW(T)(std::forward<Args>(args)...);
		pCB->SetObject(pNewObj);
		pNewObj->SetControlBlock(pCB);

		return pNewObj;
	}

	template<typename T, typename A, typename... Args>
	static T* CreateManaged(A* pArena, Args&&... args)
	{
		auto pCB = BV_MNEW(*pArena, BvControlBlock<T>)(pArena);
		auto pNewObj = BV_MNEW(*pArena, T)(std::forward<Args>(args)...);
		pCB->SetObject(pNewObj);
		pNewObj->SetControlBlock(pCB);

		return pNewObj;
	}
};

#define BV_OBJECT_CREATE(Type, ...) BvObjectCreator::Create<Type>(__VA_ARGS__)
#define BV_OBJECT_MCREATE(pArena, Type, ...) BvObjectCreator::CreateManaged<Type>(pArena __VA_OPT__(,) __VA_ARGS__)


namespace Internal
{
	template<typename Type>
	class RemoveIBvObject : public Type
	{
	private:
		~RemoveIBvObject();
		u32 AddRef();
		u32 Release();
	};

	template<typename Type>
	struct IBvObjectReturnType
	{
		using ReturnType = RemoveIBvObject<Type>;
	};

	template<typename Type>
	using IBvObjectReturnTypeT = typename IBvObjectReturnType<Type>::ReturnType;
}


template<typename T>
class BvObjectHandle
{
public:
	BvObjectHandle()
	{
	}

	BvObjectHandle(T* pObj)
		: m_pObj(pObj)
	{
		InternalAddRef();
	}

	template<typename U>
	BvObjectHandle(U* pObj)
	{
		if (pObj)
		{
			T* pNewObj = nullptr;
			if (pObj->QueryInterface(T::GetId(), &pNewObj))
			{
				m_pObj = pNewObj;
			}
		}
	}

	BvObjectHandle(const BvObjectHandle& rhs)
		: m_pObj(rhs.m_pObj)
	{
		InternalAddRef();
	}

	BvObjectHandle(BvObjectHandle&& rhs)
	{
		// Can't use &rhs since the & operator is overloaded
		if (this != reinterpret_cast<BvObjectHandle*>(&reinterpret_cast<u8&>(rhs)))
		{
			Swap(rhs);
		}
	}

	BvObjectHandle& operator=(IBvObject* pObj)
	{
		if (m_pObj != pObj)
		{
			BvObjectHandle(pObj).Swap(*this);
		}

		return *this;
	}

	BvObjectHandle& operator=(const BvObjectHandle& rhs)
	{
		if (m_pObj != rhs.m_pObj)
		{
			BvObjectHandle(rhs).Swap(*this);
		}

		return *this;
	}

	BvObjectHandle& operator=(BvObjectHandle&& rhs)
	{
		// Can't use &rhs since the & operator is overloaded
		BvObjectHandle(static_cast<BvObjectHandle&&>(rhs)).Swap(*this);

		return *this;
	}

	~BvObjectHandle()
	{
		InternalRelease();
	}

#if BV_DEBUG
	BV_INLINE Internal::IBvObjectReturnTypeT<T>* operator->() const
	{
		return static_cast<Internal::IBvObjectReturnTypeT<T>*>(m_pObj);
	}
#else
	BV_INLINE T* operator->() const
	{
		return m_pObj;
	}
#endif

	BV_INLINE operator T*() const
	{
		return m_pObj;
	}

	BV_INLINE T** operator&()
	{
		return &m_pObj;
	}

	BV_INLINE u32 Reset()
	{
		return InternalRelease();
	}

	BV_INLINE void Attach(T* pObj)
	{
		if (m_pObj != pObj)
		{
			BvObjectHandle other(pObj);
			std::swap(m_pObj, other.m_pObj);
		}
	}

	BV_INLINE T* Detach()
	{
		auto pObj = m_pObj;
		m_pObj = nullptr;
		return pObj;
	}

	void Swap(BvObjectHandle& rhs)
	{
		std::swap(m_pObj, rhs.m_pObj);
	}

	void Swap(BvObjectHandle&& rhs)
	{
		std::swap(m_pObj, rhs.m_pObj);
	}

private:
	u32 InternalAddRef()
	{
		if (m_pObj)
		{
			return m_pObj->AddRef();
		}

		return 0;
	}

	u32 InternalRelease()
	{
		u32 refCount = 0;
		T* pObj = m_pObj;

		if (pObj)
		{
			m_pObj = nullptr;
			refCount = m_pObj->Release();
		}

		return refCount;
	}

private:
	T* m_pObj = nullptr;
};