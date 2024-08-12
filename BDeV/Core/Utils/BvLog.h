#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Utils/BvUtils.h"


template<typename LogType>
class BvLog final
{
	BV_DEFAULTCOPYMOVE(BvLog);
public:
	BvLog() {}
	~BvLog() {}

	void PrintF(const char* pFormat, ...)
	{
	}

private:
	LogType m_Log;
};