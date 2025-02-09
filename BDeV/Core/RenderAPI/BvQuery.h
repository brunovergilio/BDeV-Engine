#pragma once


#include "BvRenderCommon.h"


class BvQuery : public IBvRenderDeviceChild
{
public:
	virtual QueryType GetQueryType() const = 0;
	virtual bool GetResult(void* pData, u64 size) = 0;

protected:
	BvQuery() {}
	virtual ~BvQuery() {}
};