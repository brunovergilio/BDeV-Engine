#pragma once


#include "BvRenderCommon.h"
#include "BvRenderDeviceObject.h"


BV_OBJECT_DEFINE_ID(BvQuery, "82e3f062-7670-4e70-9b8f-3d6698815cee");
class BvQuery : public IBvRenderDeviceObject
{
public:
	virtual QueryType GetQueryType() const = 0;
	virtual bool GetResult(void* pData, u64 size) = 0;

protected:
	BvQuery() {}
	virtual ~BvQuery() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvQuery);