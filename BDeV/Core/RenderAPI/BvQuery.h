#pragma once


#include "BvRenderCommon.h"


class BvQuery : public BvRenderDeviceChild
{
public:
	virtual QueryType GetQueryType() const = 0;
	virtual u64 GetResult() const = 0;

protected:
	BvQuery() {}
	virtual ~BvQuery() {}
};