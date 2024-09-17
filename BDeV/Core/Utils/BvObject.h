#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/Memory/BvMemoryArena.h"
#include "BDeV/Core/System/Memory/BvMemory.h"
#include <atomic>


#define BV_IBVOBJECT_FOR_EACH(id, first, ...) || id == first::GetId() 	\
	IF_ELSE(HAS_ARGS(__VA_ARGS__))			\
	(										\
		DEFER2(_BV_IBVOBJECT_FOR_EACH)()(id, __VA_ARGS__)	\
	)										\
	(										\
	)
#define _BV_IBVOBJECT_FOR_EACH() BV_IBVOBJECT_FOR_EACH

#define BV_IBVOBJECT_IMPL_INTERFACE_BASE(objType) static constexpr BvStringId GetId() { return ConstexprMurmurHash64A(#objType); }
#define BV_IBVOBJECT_IMPL_INTERFACE(objType, ...) static constexpr BvStringId GetId() { return ConstexprMurmurHash64A(#objType); } \
void* QueryInterface(BvStringId id) override { if (id == this->GetId() __VA_OPT__ ( EVAL(BV_IBVOBJECT_FOR_EACH(id, __VA_ARGS__)) ) || id == IBvObject::GetId() ) { this->AddRef(); return this; } return nullptr; }


// This is a ref-counted object type, similar to a COM object
// Any derived classes must be created with either
// BV_MNEW or BV_NEW
class IBvObject
{
	BV_NOCOPYMOVE(IBvObject);

public:
	static constexpr BvStringId GetId()
	{
		return ConstexprMurmurHash64A("IBvObject");
	}

	virtual u32 AddRef() = 0;
	virtual u32 Release() = 0;
	virtual void* QueryInterface(BvStringId id) = 0;

protected:
	IBvObject() {}
	virtual ~IBvObject() {}
};


#define BV_IBVOBJECT_CREATE_REFCOUNT_BASE(ObjName, BaseObjName)				\
class ObjName : public BaseObjName											\
{																			\
	BV_NOCOPYMOVE(ObjName);													\
																			\
public:																		\
	u32 AddRef() override final												\
	{																		\
		return ++m_RefCount;												\
	}																		\
																			\
	u32 Release() override final											\
	{																		\
		auto refCount = --m_RefCount;										\
		if (refCount == 0)													\
		{																	\
			if (m_pArena)													\
			{																\
				BV_MDELETE(*m_pArena, this);								\
			}																\
			else															\
			{																\
				BV_DELETE(this);											\
			}																\
		}																	\
																			\
		return refCount;													\
	}																		\
																			\
	ObjName() : m_RefCount(1u) {}											\
	ObjName(IBvMemoryArena* pArena) : m_pArena(pArena), m_RefCount(1u) {}	\
	virtual ~ObjName() {}													\
																			\
protected:																	\
	IBvMemoryArena* m_pArena = nullptr;										\
	std::atomic<u32> m_RefCount = 0;										\
};

#define BV_IBVOBJECT_CREATE_REFCOUNT_BASE_DEFAULT(ObjName) BV_IBVOBJECT_CREATE_REFCOUNT_BASE(ObjName, IBvObject)


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
			if (auto p = pObj->QueryInterface(T::GetId()))
			{
				m_pObj = static_cast<T*>(p);
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
		*this = std::move(rhs);
	}

	BvObjectHandle& operator=(IBvObject* pObj)
	{
		if (m_pObj != pObj)
		{
			BvObjectHandle other(pObj);
			std::swap(m_pObj, other.m_pObj);
		}

		return *this;
	}

	BvObjectHandle& operator=(const BvObjectHandle& rhs)
	{
		if (m_pObj != rhs.m_pObj)
		{
			BvObjectHandle other(rhs);
			std::swap(m_pObj, other.m_pObj);
		}

		return *this;
	}

	BvObjectHandle& operator=(BvObjectHandle&& rhs)
	{
		if (m_pObj != rhs.m_pObj)
		{
			BvObjectHandle other(rhs);
			std::swap(m_pObj, other.m_pObj);
		}

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
		if (m_pObj)
		{
			refCount = m_pObj->Release();
			m_pObj = nullptr;
		}

		return refCount;
	}

private:
	T* m_pObj = nullptr;
};