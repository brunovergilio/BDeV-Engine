#pragma once


#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Utils/BvUUID.h"
#include "BDeV/Core/System/Memory/BvMemory.h"
#include "BDeV/Core/Container/BvRobinSet.h"


#define BV_ADD_BASE_OBJ(objName) public objName
#define FOR_EACH_ADD_BASE_OBJ(m, first, ...) m(first)	\
	__VA_OPT__(,) \
	IF_ELSE(HAS_ARGS(__VA_ARGS__))			\
	(										\
		DEFER2(_FOR_EACH_ADD_BASE_OBJ)()(m, __VA_ARGS__)	\
	)										\
	(										\
	)
#define _FOR_EACH_ADD_BASE_OBJ() FOR_EACH_ADD_BASE_OBJ
#define BV_CREATE_CLASS(objName, ...) class objName __VA_OPT__(: EVAL(FOR_EACH_ADD_BASE_OBJ(BV_ADD_BASE_OBJ, __VA_ARGS__)))
#define BV_CREATE_STRUCT(objName, ...) struct objName __VA_OPT__(: EVAL(FOR_EACH_ADD_BASE_OBJ(BV_ADD_BASE_OBJ, __VA_ARGS__)))


#define BV_QUERY_INTERFACE_HELPER(objName, first) objName == first::GetInterfaceId()
#define FOR_EACH_BV_QUERY_INTERFACE(m, objName, first, ...) m(objName, first)	\
	__VA_OPT__(|| ) \
	IF_ELSE(HAS_ARGS(__VA_ARGS__))			\
	(										\
		DEFER2(_FOR_EACH_BV_QUERY_INTERFACE)()(m, objName, __VA_ARGS__)	\
	)										\
	(										\
	)
#define _FOR_EACH_BV_QUERY_INTERFACE() FOR_EACH_BV_QUERY_INTERFACE
#define BV_QUERY_INTERFACE(objName, ...) __VA_OPT__(EVAL(FOR_EACH_BV_QUERY_INTERFACE(BV_QUERY_INTERFACE_HELPER, objName, __VA_ARGS__))) \


#define BV_IMPLEMENT_QUERY_INTERFACE(...)							\
bool QueryInterface(const BvUUID& id, void** ppInterface) override	\
{																	\
	if (BV_QUERY_INTERFACE(id, __VA_ARGS__))						\
	{																\
		*ppInterface = this;										\
		AddRef();													\
		return true;												\
	}																\
	return false;													\
}


#define BV_IMPLEMENT_QUERY_INTERFACE_CALL_BASE(obj, base)			\
bool QueryInterface(const BvUUID& id, void** ppInterface) override	\
{																	\
	if (id == obj::GetInterfaceId())								\
	{																\
		*ppInterface = this;										\
		AddRef();													\
		return true;												\
	}																\
	else															\
	{																\
		return base::QueryInterface(id, ppInterface);				\
	}																\
}


#define BV_IMPLEMENT_REF_COUNTER(refCount, pArena)			\
i32 AddRef() override final									\
{															\
	return ++refCount;										\
}															\
															\
i32 Release() override final								\
{															\
	i32 ref = --refCount;									\
	if (ref <= 0)											\
	{														\
		pArena ? BvMDelete(*pArena, this) : BvDelete(this);	\
		return 0;											\
	}														\
															\
	return ref;												\
}


#define BV_IBVOBJECT_ID(interfaceId) static constexpr BvUUID GetInterfaceId() { return MakeUUIDv4(interfaceId); }

class IBvObject
{
	BV_NOCOPYMOVE(IBvObject);

public:
	BV_IBVOBJECT_ID("00000000-0000-0000-0000-000000000000");

	virtual i32 AddRef() = 0;
	virtual i32 Release() = 0;
	virtual bool QueryInterface(const BvUUID& id, void** ppInterface) = 0;

	template<typename Type>
	bool QueryInterfaceT(Type** ppInterface)
	{
		return QueryInterface(Type::GetInterfaceId(), ppInterface);
	}

protected:
	IBvObject() {}
	virtual ~IBvObject() = 0 {}
};


namespace Internal
{
	template<typename Type>
	class BvObjectRemoveMethodHelper : public Type
	{
	private:
		~BvObjectRemoveMethodHelper();
		i32 AddRef();
		i32 Release();
	};

	template<typename Type>
	struct BvRemoveIBvObjectMethod
	{
		using type = BvObjectRemoveMethodHelper<Type>;
	};

	template<typename Type>
	using BvRemoveIBvObjectMethodT = typename BvRemoveIBvObjectMethod<Type>::type;
}


template <typename Type>
class BvObjectPtr
{
public:
	template<class OtherType> friend class IBvObjectPtr;

	BvObjectPtr() {}
	
	BvObjectPtr(Type* pObj)
		: m_pObj(pObj)
	{
		InternalAddRef();
	}

	BvObjectPtr(const BvObjectPtr& rhs)
		: m_pObj(rhs.m_pObj)
	{
		InternalAddRef();
	}

	BvObjectPtr(BvObjectPtr&& rhs)
		: m_pObj(rhs.m_pObj)
	{
		rhs.m_pObj = nullptr;
	}

	BvObjectPtr& operator=(Type* pObj)
	{
		if (m_pObj != pObj)
		{
			InternalRelease();
			m_pObj = pObj;
			InternalAddRef();
		}

		return *this;
	}

	BvObjectPtr& operator=(const BvObjectPtr& rhs)
	{
		if (this != &rhs)
		{
			InternalRelease();
			m_pObj = rhs.m_pObj;
			InternalAddRef();
		}

		return *this;
	}

	BvObjectPtr& operator=(BvObjectPtr&& rhs)
	{
		std::swap(m_pObj, rhs.m_pObj);

		return *this;
	}

	Type* Get() const
	{
		return m_pObj;
	}

	operator bool() const
	{
		return m_pObj != nullptr;
	}

#if BV_DEBUG
	Internal::BvRemoveIBvObjectMethodT<Type>* operator->() const
	{
		return static_cast<Internal::BvRemoveIBvObjectMethodT<Type>*>(m_pObj);
	}
#else
	Type* operator->() const
	{
		return m_pObj;
	}
#endif

	Type** operator&()
	{
		return &m_pObj;
	}

	const Type* const* operator&() const
	{
		return &m_pObj;
	}

	Type** GetAddressOf()
	{
		return &m_pObj;
	}

	const Type* const* GetAddressOf() const
	{
		return &m_pObj;
	}

	Type** ReleaseAndGetAddressOf()
	{
		InternalRelease();
		return &m_pObj;
	}

	u32 Reset()
	{
		return InternalRelease();
	}

	void Attach(Type* pObj)
	{
		if (m_pObj == pObj)
		{
			return;
		}

		if (m_pObj)
		{
			InternalRelease();
		}

		m_pObj = pObj;
	}

	Type* Detach()
	{
		auto pObj = m_pObj;
		m_pObj = nullptr;
		return pObj;
	}

	template<typename OtherType>
	bool As(BvObjectPtr<OtherType>& rhs)
	{
		if (auto pObj = m_pObj->QueryInterfaceT<OtherType>())
		{
			rhs.Attach(pObj);
			return true;
		}

		return false;
	}

	~BvObjectPtr()
	{
		InternalRelease();
	}

protected:
	void InternalAddRef() const
	{
		if (m_pObj != nullptr)
		{
			m_pObj->AddRef();
		}
	}

	u32 InternalRelease()
	{
		u32 ref = 0;

		if (m_pObj != nullptr)
		{
			Type* pTemp = m_pObj;
			m_pObj = nullptr;
			ref = pTemp->Release();
		}

		return ref;
	}

protected:
	Type* m_pObj = nullptr;
};