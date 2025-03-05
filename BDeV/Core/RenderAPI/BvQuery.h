#pragma once


#include "BvRenderCommon.h"
#include "BvRenderDeviceObject.h"


BV_OBJECT_DEFINE_ID(IBvQuery, "82e3f062-7670-4e70-9b8f-3d6698815cee");
class IBvQuery : public IBvRenderDeviceObject
{
public:
	virtual QueryType GetQueryType() const = 0;
	virtual bool GetResult(void* pData, u64 size) = 0;

protected:
	IBvQuery() {}
	~IBvQuery() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvQuery);