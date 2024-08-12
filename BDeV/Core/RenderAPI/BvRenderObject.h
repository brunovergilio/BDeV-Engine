#pragma once


#pragma once


#include "BDeV/Core/Utils/BvObject.h"
#include "BDeV/Core/System/Memory/BvMemoryArena.h"


class IBvRenderObject : public IBvObject
{
public:
	BV_IBVOBJECT_ID("39079ac6-6521-4e4e-a587-d01abeda36e5");

	BV_IMPLEMENT_REF_COUNTER(m_RefCount, m_pArena);
	BV_IMPLEMENT_QUERY_INTERFACE(IBvRenderObject, IBvObject);

	virtual void SetName(const char* pName) = 0;

protected:
	IBvRenderObject(IBvMemoryArena* pArena)
		: m_pArena(pArena), m_RefCount(1) {}
	~IBvRenderObject() {}

protected:
	IBvMemoryArena* m_pArena = nullptr;
	std::atomic<i32> m_RefCount;
};


class BvRenderDevice;


class IBvRenderDeviceChild : public IBvRenderObject
{
public:
	BV_IBVOBJECT_ID("fe918846-7562-4fd5-bc89-99fd90e646e8");

	BV_IMPLEMENT_QUERY_INTERFACE_CALL_BASE(IBvRenderDeviceChild, IBvRenderObject);

	virtual BvRenderDevice* GetDevice() const = 0;

protected:
	IBvRenderDeviceChild(IBvMemoryArena* pArena)
		: IBvRenderObject(pArena) {}
	~IBvRenderDeviceChild() {}
};