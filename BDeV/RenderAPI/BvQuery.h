#pragma once


#include "BvCommon.h"


class BvQuery
{
public:
	virtual QueryType GetQueryType() const = 0;
	virtual u64 GetResult() const = 0;

protected:
	BvQuery() {}
	virtual ~BvQuery() {}
};